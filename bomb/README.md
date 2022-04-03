
Bomb Lab
=======

# Phase_1



```assembly
(gdb) disassemble
Dump of assembler code for function phase_1:
0x0000000000400ee0 <+0>:	sub    $0x8,%rsp
0x0000000000400ee4 <+4>:	mov    $0x402400,%esi
=> 0x0000000000400ee9 <+9>:	callq  0x401338 <strings_not_equal>
0x0000000000400eee <+14>:	test   %eax,%eax
0x0000000000400ef0 <+16>:	je     0x400ef7 <phase_1+23>
0x0000000000400ef2 <+18>:	callq  0x40143a <explode_bomb>
0x0000000000400ef7 <+23>:	add    $0x8,%rsp
0x0000000000400efb <+27>:	retq
```

> test %A %B   => A & B == 0

> test %eax %eax => if %eax == 0, then zero flag is set

`TEST` sets the zero flag, `ZF`, when the result of the AND operation is zero. If two operands are equal, their bitwise AND is zero when both are zero. `TEST` also sets the sign flag, `SF`, when the most significant bit is set in the result, and the parity flag, `PF`, when the number of set bits is even.



To make je at <+16> jump to <phase_1 + 23>, %eax has to be 0

```assembly
(gdb) disassemble
Dump of assembler code for function phase_1:
0x0000000000400ee0 <+0>:	sub    $0x8,%rsp
0x0000000000400ee4 <+4>:	mov    $0x402400,%esi
=> 0x0000000000400ee9 <+9>:	callq  0x401338 <strings_not_equal>
0x0000000000400eee <+14>:	test   %eax,%eax
0x0000000000400ef0 <+16>:	je     0x400ef7 <phase_1+23>
0x0000000000400ef2 <+18>:	callq  0x40143a <explode_bomb>
0x0000000000400ef7 <+23>:	add    $0x8,%rsp
0x0000000000400efb <+27>:	retq
```

%eax must be 0, meaning <strings_not_equal> must return 0

- 两个string比较必须是完全相等, 才能返回0; 所以我们只需要查看第二个传入的参数, 即以%rsi地址开头的string是什么, 就是我们要的答案
  - 方法1: 直接打印    (gdb) print (char*)$rsi
  - 方法2: 查看phase_1, 调用strings_not_equal前的指令:  mov  $0x402400,%esi, 就可以知道目标string的起始地址是0x402400, 就可以直接打印    (gdb) print (char*) 0x402400
- 结果都是: **Border relations with Canada have never been better.**

```assembly
(gdb) disassemble
Dump of assembler code for function strings_not_equal:
=> 0x0000000000401338 <+0>:	push   %r12
   0x000000000040133a <+2>:	push   %rbp
   0x000000000040133b <+3>:	push   %rbx
   0x000000000040133c <+4>:	mov    %rdi,%rbx
   0x000000000040133f <+7>:	mov    %rsi,%rbp
   0x0000000000401342 <+10>:	callq  0x40131b <string_length>
   0x0000000000401347 <+15>:	mov    %eax,%r12d
   0x000000000040134a <+18>:	mov    %rbp,%rdi
   0x000000000040134d <+21>:	callq  0x40131b <string_length>
   0x0000000000401352 <+26>:	mov    $0x1,%edx
   0x0000000000401357 <+31>:	cmp    %eax,%r12d    # compare length of the two strings 
   0x000000000040135a <+34>:	jne    0x40139b <strings_not_equal+99>
   0x000000000040135c <+36>:	movzbl (%rbx),%eax
   0x000000000040135f <+39>:	test   %al,%al
   0x0000000000401361 <+41>:	je     0x401388 <strings_not_equal+80>
   0x0000000000401363 <+43>:	cmp    0x0(%rbp),%al
   0x0000000000401366 <+46>:	je     0x401372 <strings_not_equal+58>
   0x0000000000401368 <+48>:	jmp    0x40138f <strings_not_equal+87>
   0x000000000040136a <+50>:	cmp    0x0(%rbp),%al
   0x000000000040136d <+53>:	nopl   (%rax)
   0x0000000000401370 <+56>:	jne    0x401396 <strings_not_equal+94>
   0x0000000000401372 <+58>:	add    $0x1,%rbx
   0x0000000000401376 <+62>:	add    $0x1,%rbp
   0x000000000040137a <+66>:	movzbl (%rbx),%eax
   0x000000000040137d <+69>:	test   %al,%al
   0x000000000040137f <+71>:	jne    0x40136a <strings_not_equal+50>
   0x0000000000401381 <+73>:	mov    $0x0,%edx
   0x0000000000401386 <+78>:	jmp    0x40139b <strings_not_equal+99>
   0x0000000000401388 <+80>:	mov    $0x0,%edx
   0x000000000040138d <+85>:	jmp    0x40139b <strings_not_equal+99>
   0x000000000040138f <+87>:	mov    $0x1,%edx
   0x0000000000401394 <+92>:	jmp    0x40139b <strings_not_equal+99>
   0x0000000000401396 <+94>:	mov    $0x1,%edx
   0x000000000040139b <+99>:	mov    %edx,%eax
   0x000000000040139d <+101>:	pop    %rbx
   0x000000000040139e <+102>:	pop    %rbp
   0x000000000040139f <+103>:	pop    %r12
   0x00000000004013a1 <+105>:	retq
```

**phase_1 solution: Border relations with Canada have never been better.**


# Phase_2

```assembly
(gdb) disassemble phase_2
Dump of assembler code for function phase_2:
=> 0x0000000000400efc <+0>:	push   %rbp
0x0000000000400efd <+1>:	push   %rbx
0x0000000000400efe <+2>:	sub    $0x28,%rsp
0x0000000000400f02 <+6>:	mov    %rsp,%rsi
0x0000000000400f05 <+9>:	callq  0x40145c <read_six_numbers>
0x0000000000400f0a <+14>:	cmpl   $0x1,(%rsp)      # (%rsp) 必须是1, 才能避免执行<explode_bomb>
0x0000000000400f0e <+18>:	je     0x400f30 <phase_2+52>
0x0000000000400f10 <+20>:	callq  0x40143a <explode_bomb>
0x0000000000400f15 <+25>:	jmp    0x400f30 <phase_2+52>
0x0000000000400f17 <+27>:	mov    -0x4(%rbx),%eax
0x0000000000400f1a <+30>:	add    %eax,%eax
0x0000000000400f1c <+32>:	cmp    %eax,(%rbx)
0x0000000000400f1e <+34>:	je     0x400f25 <phase_2+41>
0x0000000000400f20 <+36>:	callq  0x40143a <explode_bomb>
0x0000000000400f25 <+41>:	add    $0x4,%rbx
0x0000000000400f29 <+45>:	cmp    %rbp,%rbx
0x0000000000400f2c <+48>:	jne    0x400f17 <phase_2+27>
0x0000000000400f2e <+50>:	jmp    0x400f3c <phase_2+64>
0x0000000000400f30 <+52>:	lea    0x4(%rsp),%rbx
0x0000000000400f35 <+57>:	lea    0x18(%rsp),%rbp
0x0000000000400f3a <+62>:	jmp    0x400f17 <phase_2+27>
0x0000000000400f3c <+64>:	add    $0x28,%rsp
0x0000000000400f40 <+68>:	pop    %rbx
0x0000000000400f41 <+69>:	pop    %rbp
0x0000000000400f42 <+70>:	retq
```

0x0000000000400f0a <+14>:	cmpl   $0x1,(%rsp)      意味着 (%rsp) 必须是1, 才能避免<explode_bomb>

所以第一个值是1



0x0000000000400f17 <+27>:	mov    -0x4(%rbx),%eax  意味着 mem[rbx - 4]的值放到%eax, 然后

0x0000000000400f1a <+30>:	add    %eax,%eax    %eax自身翻倍

0x0000000000400f1c <+32>:	cmp    %eax,(%rbx)   %eax翻倍后 必须要等于 (%rbx)才不能不触发<explode_bomb>

此时 %eax的值来源于  -0x4(%rbx), 也即 %rsp的值, 也就是1, 翻倍后%eax的值是2, 必须等于%rbx所指的值,

所以 (%rbx) 的值是2



然后 %rbx的值会进行加4, 所以可以预见到的是, %rbx的地址一直向栈底方向走, 直到执行到指令

0x0000000000400f29 <+45>:	cmp    %rbp,%rbx  导致0x0000000000400f2c <+48>:	jne    0x400f17 <phase_2+27> 不再成立之前, 

%rbx一直都是每次加4, 然后进行%eax自身翻倍, 并比较%eax与%rbx的值


**phase_2 solution: 1 2 4 8 16 32**


# Phase_3


# Phase_4

## 

```assembly
(gdb) disassemble phase_4
Dump of assembler code for function phase_4:
=> 0x000000000040100c <+0>:	sub    $0x18,%rsp
0x0000000000401010 <+4>:	lea    0xc(%rsp),%rcx
0x0000000000401015 <+9>:	lea    0x8(%rsp),%rdx
0x000000000040101a <+14>:	mov    $0x4025cf,%esi	   # "%d %d"
0x000000000040101f <+19>:	mov    $0x0,%eax
0x0000000000401024 <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
0x0000000000401029 <+29>:	cmp    $0x2,%eax    # eax一定要是2
0x000000000040102c <+32>:	jne    0x401035 <phase_4+41>
0x000000000040102e <+34>:	cmpl   $0xe,0x8(%rsp)  # 14  0x8(%rsp)的范围是[0,14]
0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46>
0x0000000000401035 <+41>:	callq  0x40143a <explode_bomb>
0x000000000040103a <+46>:	mov    $0xe,%edx
0x000000000040103f <+51>:	mov    $0x0,%esi
0x0000000000401044 <+56>:	mov    0x8(%rsp),%edi
0x0000000000401048 <+60>:	callq  0x400fce <func4>
0x000000000040104d <+65>:	test   %eax,%eax
0x000000000040104f <+67>:	jne    0x401058 <phase_4+76>
0x0000000000401051 <+69>:	cmpl   $0x0,0xc(%rsp)
0x0000000000401056 <+74>:	je     0x40105d <phase_4+81>
0x0000000000401058 <+76>:	callq  0x40143a <explode_bomb>
0x000000000040105d <+81>:	add    $0x18,%rsp
0x0000000000401061 <+85>:	retq
```

0x000000000040102e <+34>:	cmpl   $0xe,0x8(%rsp)

0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46>   => jump below or equal,  意味着操作数是unsigned值

意味着 0x8(%rsp)的范围是[0,14]



```assembly
0x000000000040103a <+46>:	mov    $0xe,%edx
0x000000000040103f <+51>:	mov    $0x0,%esi
0x0000000000401044 <+56>:	mov    0x8(%rsp),%edi
0x0000000000401048 <+60>:	callq  0x400fce <func4>
```

%rdi, %rsi, %rdx分别为前三个参数, 值分别为0x8(%rsp), 0, 14, 传递给func4

```assembly
0x000000000040104d <+65>:	test   %eax,%eax
0x000000000040104f <+67>:	jne    0x401058 <phase_4+76>
```

func4的返回值必须是0, 才能保证jne不执行, explode_bomb不被调用

通过func4可以推理出phase_4的第一个参数0x8(%rsp)的值是7, 推理如下:

```assembly
(gdb) disassemble func4
Dump of assembler code for function func4:
0x0000000000400fce <+0>:	sub    $0x8,%rsp
0x0000000000400fd2 <+4>:	mov    %edx,%eax
0x0000000000400fd4 <+6>:	sub    %esi,%eax
0x0000000000400fd6 <+8>:	mov    %eax,%ecx
0x0000000000400fd8 <+10>:	shr    $0x1f,%ecx  # >>>31
0x0000000000400fdb <+13>:	add    %ecx,%eax
0x0000000000400fdd <+15>:	sar    %eax
0x0000000000400fdf <+17>:	lea    (%rax,%rsi,1),%ecx # %rax + %rsi
0x0000000000400fe2 <+20>:	cmp    %edi,%ecx   # 与%edi比较, %edi是传入的第一个参数, 即0x8(%rsp)
0x0000000000400fe4 <+22>:	jle    0x400ff2 <func4+36>
0x0000000000400fe6 <+24>:	lea    -0x1(%rcx),%edx
0x0000000000400fe9 <+27>:	callq  0x400fce <func4>
0x0000000000400fee <+32>:	add    %eax,%eax
0x0000000000400ff0 <+34>:	jmp    0x401007 <func4+57>
0x0000000000400ff2 <+36>:	mov    $0x0,%eax
0x0000000000400ff7 <+41>:	cmp    %edi,%ecx
0x0000000000400ff9 <+43>:	jge    0x401007 <func4+57>
0x0000000000400ffb <+45>:	lea    0x1(%rcx),%esi
0x0000000000400ffe <+48>:	callq  0x400fce <func4>
0x0000000000401003 <+53>:	lea    0x1(%rax,%rax,1),%eax
0x0000000000401007 <+57>:	add    $0x8,%rsp
0x000000000040100b <+61>:	retq
```

> **When shifting an unsigned value, the >> operator in C is a logical shift.** **When shifting a signed value, the >> operator is an arithmetic shift**.

```c
int func4(int x, int y, int z){
	int m = z;	// mov    %edx,%eax
  m = z - y;  // sub    %esi,%eax
  int n = m;      // mov    %eax,%ecx
  n = n >>> 31 // shr    $0x1f,%ecx
  m += n;      // add    %ecx,%eax
  m = m >> 1;			// sar    %eax
  n = m + y // lea    (%rax,%rsi,1),%ecx
  
  if(n > x){
    z = n - 1;	// lea    -0x1(%rcx),%edx
    int res = func4(x, y, z);
    res += res;
    return res;
  }else{  // n <= x
    m = 0;
    
    if(n >= x){
      return m;  // 此时可以满足func4的返回值是0
    }
  }
  
}
```

只要让让n == x, 即可满足 func4的返回值是0

所以根据func4开头的一堆运算, 可以得出n的值

```c
n =  ( ((z - y) >>> 31)  +  (z - y) )  >> 1   +    y
```

y = 0, z = 14, 计算得出 n = 7, 所以 x = 7



```assembly
0x0000000000401051 <+69>:	cmpl   $0x0,0xc(%rsp)
0x0000000000401056 <+74>:	je     0x40105d <phase_4+81>
0x0000000000401058 <+76>:	callq  0x40143a <explode_bomb>
0x000000000040105d <+81>:	add    $0x18,%rsp
```

接着0xc(%rsp) 是phase_4的第二个参数

很容易地可以通过  cmpl   $0x0,0xc(%rsp), 可知, 第二个参数必须是0, 才能直接跳到函数末尾返回, 所以第二个参数是0


**phase_4 solution: 7 0**


# Phase_5

## 

```assembly
(gdb) disassemble phase_5
Dump of assembler code for function phase_5:
0x0000000000401062 <+0>:	push   %rbx
0x0000000000401063 <+1>:	sub    $0x20,%rsp
0x0000000000401067 <+5>:	mov    %rdi,%rbx
0x000000000040106a <+8>:	mov    %fs:0x28,%rax
0x0000000000401073 <+17>:	mov    %rax,0x18(%rsp)
0x0000000000401078 <+22>:	xor    %eax,%eax
0x000000000040107a <+24>:	callq  0x40131b <string_length>
0x000000000040107f <+29>:	cmp    $0x6,%eax      # <string_length> 返回值必须是6
0x0000000000401082 <+32>:	je     0x4010d2 <phase_5+112>
0x0000000000401084 <+34>:	callq  0x40143a <explode_bomb>
0x0000000000401089 <+39>:	jmp    0x4010d2 <phase_5+112>
0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx
0x000000000040108f <+45>:	mov    %cl,(%rsp)
0x0000000000401092 <+48>:	mov    (%rsp),%rdx
0x0000000000401096 <+52>:	and    $0xf,%edx
0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx
0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
0x00000000004010a4 <+66>:	add    $0x1,%rax
0x00000000004010a8 <+70>:	cmp    $0x6,%rax
0x00000000004010ac <+74>:	jne    0x40108b <phase_5+41>
0x00000000004010ae <+76>:	movb   $0x0,0x16(%rsp)
0x00000000004010b3 <+81>:	mov    $0x40245e,%esi
0x00000000004010b8 <+86>:	lea    0x10(%rsp),%rdi
0x00000000004010bd <+91>:	callq  0x401338 <strings_not_equal>
0x00000000004010c2 <+96>:	test   %eax,%eax
0x00000000004010c4 <+98>:	je     0x4010d9 <phase_5+119>
0x00000000004010c6 <+100>:	callq  0x40143a <explode_bomb>
0x00000000004010cb <+105>:	nopl   0x0(%rax,%rax,1)
0x00000000004010d0 <+110>:	jmp    0x4010d9 <phase_5+119>
0x00000000004010d2 <+112>:	mov    $0x0,%eax
0x00000000004010d7 <+117>:	jmp    0x40108b <phase_5+41>
0x00000000004010d9 <+119>:	mov    0x18(%rsp),%rax
0x00000000004010de <+124>:	xor    %fs:0x28,%rax
0x00000000004010e7 <+133>:	je     0x4010ee <phase_5+140>
0x00000000004010e9 <+135>:	callq  0x400b30 <__stack_chk_fail@plt>
0x00000000004010ee <+140>:	add    $0x20,%rsp
0x00000000004010f2 <+144>:	pop    %rbx
0x00000000004010f3 <+145>:	retq
```



先看这个函数

**strings_not_equal**

0x000000000040135c <+36>:	movzbl (%rbx),%eax
0x000000000040135f <+39>:	test   %al,%al

首先, movzbl (%rbx),%eax 的作用是, 把%rbx所指的内存数据的最后一个byte进行zero extension放到%eax, 因为一个字符也就是1byte, 所以extension作用不大

- movzbl - zero extension
- movsbl - sign extension

然后, test %al, %al 如果想要跳出函数, %al的值必须是0, 才能执行下面的je指令, %al表示的是1个byte大小的范围, 正常情况下, 没有字符的值是0, 除非这个值是C语言中字符串的结束字符, 即null character满足条件。所以这个指令是判断是否字符串比较进行到了末尾

```assembly
(gdb) disassemble strings_not_equal
Dump of assembler code for function strings_not_equal:
0x0000000000401338 <+0>:	push   %r12
0x000000000040133a <+2>:	push   %rbp
0x000000000040133b <+3>:	push   %rbx
0x000000000040133c <+4>:	mov    %rdi,%rbx
0x000000000040133f <+7>:	mov    %rsi,%rbp
0x0000000000401342 <+10>:	callq  0x40131b <string_length>
0x0000000000401347 <+15>:	mov    %eax,%r12d
0x000000000040134a <+18>:	mov    %rbp,%rdi
0x000000000040134d <+21>:	callq  0x40131b <string_length>
0x0000000000401352 <+26>:	mov    $0x1,%edx
0x0000000000401357 <+31>:	cmp    %eax,%r12d
0x000000000040135a <+34>:	jne    0x40139b <strings_not_equal+99>  # 自此以上, 是判断2个字符串长度是否相等
0x000000000040135c <+36>:	movzbl (%rbx),%eax
0x000000000040135f <+39>:	test   %al,%al
0x0000000000401361 <+41>:	je     0x401388 <strings_not_equal+80>
0x0000000000401363 <+43>:	cmp    0x0(%rbp),%al
0x0000000000401366 <+46>:	je     0x401372 <strings_not_equal+58>
0x0000000000401368 <+48>:	jmp    0x40138f <strings_not_equal+87>
0x000000000040136a <+50>:	cmp    0x0(%rbp),%al
0x000000000040136d <+53>:	nopl   (%rax)
0x0000000000401370 <+56>:	jne    0x401396 <strings_not_equal+94>
0x0000000000401372 <+58>:	add    $0x1,%rbx
0x0000000000401376 <+62>:	add    $0x1,%rbp
0x000000000040137a <+66>:	movzbl (%rbx),%eax
0x000000000040137d <+69>:	test   %al,%al
0x000000000040137f <+71>:	jne    0x40136a <strings_not_equal+50>
0x0000000000401381 <+73>:	mov    $0x0,%edx
0x0000000000401386 <+78>:	jmp    0x40139b <strings_not_equal+99>
0x0000000000401388 <+80>:	mov    $0x0,%edx
0x000000000040138d <+85>:	jmp    0x40139b <strings_not_equal+99>
0x000000000040138f <+87>:	mov    $0x1,%edx
0x0000000000401394 <+92>:	jmp    0x40139b <strings_not_equal+99>
0x0000000000401396 <+94>:	mov    $0x1,%edx
0x000000000040139b <+99>:	mov    %edx,%eax
0x000000000040139d <+101>:	pop    %rbx
0x000000000040139e <+102>:	pop    %rbp
0x000000000040139f <+103>:	pop    %r12
0x00000000004013a1 <+105>:	retq
```



然后观察这一段指令可知: 进入这段指令最开始时, %rax的值为0, 然后每次加1, 这段指令末尾还会进行和6的比较, 即判断目前走过的长度, 很明显是一个遍历string的行为; 在遍历过程中, 会对每一个字符进行一定的处理, 处理后的字符要和0x40245e位置的字符串相等, 我们很容易可知0x40245e字符串内容为flyers, 所以我们要看这段指令是如何对每个字符进行操作的, 逆推回去即可得到最初的字符串

```assembly
0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx
0x000000000040108f <+45>:	mov    %cl,(%rsp)
0x0000000000401092 <+48>:	mov    (%rsp),%rdx
0x0000000000401096 <+52>:	and    $0xf,%edx
0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx
0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
0x00000000004010a4 <+66>:	add    $0x1,%rax
0x00000000004010a8 <+70>:	cmp    $0x6,%rax
0x00000000004010ac <+74>:	jne    0x40108b <phase_5+41>
```



这段指令需要的关注的点:

```assembly
0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx
0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
```

0x4024b0加上某个offset, 得到一个内存地址, 这个内存地址的数据movzbl后, 放到%edx

然后把%edx的低位1个byte取出放到对应%rsp一定偏移的内存地址中

所以我们不妨先看一下内存地址0x4024b0存的什么数据

```assembly
(gdb) x/s 0x4024b0
0x4024b0 <array.3449>:	"maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"
```

这样一看, 就很清楚了, 只要根据我们的输入的每个字符得到一个offset, 通过每个offset依次能访问到 f l y e r s这些字符就可以了

1, 先得到每个字符需要的offset:

| char | offset |
| ---- | ------ |
| f    | 9      |
| l    | 15     |
| y    | 14     |
| e    | 5      |
| r    | 6      |
| s    | 7      |

2, 看如何通过我们输入的字符, 变换得到这些offset


假设这个字符是a  =>  a & f => 1100001 & 00001111 => 00000001 得到了offset是1

 00001001 <= **1101001** & 00001111

 00001111 <= **1101111** & 00001111

 00001110 <= **1101110** & 00001111

 00000101 <= **1100101** & 00001111

 00000110 <= **1100110** & 00001111

 00000111 <= **1100111** & 00001111

所以可以计算出:

| offset | Result |
| ------ | ------ |
| 9      | i      |
| 15     | o      |
| 14     | n      |
| 5      | e      |
| 6      | f      |
| 7      | g      |


**phase_5 solution:  ionefg**



# phase_6

第一个关注点: 

break point在<+18> , 执行x/s $rdi, 可以看到我们的input字符串, 即read_six_numbers的入参

read_six_numbers是查验输入数据的值, 如果有6个%d, 返回值是6, 如果小于6个, 直接爆炸


```assembly
0x00000000004010fc <+8>:	sub    $0x50,%rsp
0x0000000000401100 <+12>:	mov    %rsp,%r13
0x0000000000401103 <+15>:	mov    %rsp,%rsi
0x0000000000401106 <+18>:	callq  0x40145c <read_six_numbers>
0x000000000040110b <+23>:	mov    %rsp,%r14
0x000000000040110e <+26>:	mov    $0x0,%r12d
0x0000000000401114 <+32>:	mov    %r13,%rbp
```

%r13 offset为0的数值存入%eax, 可知这是我们输入的6个数字中的第一个

这个数字减1后, 一定要 <= 5, 所以这个数字的范围是 [0, 6]

由jbe指令可知, 这6个数字是unsigned int, 范围是[0,6], 但是根据 当数字减1之后, 还要小于5, 可以推断出: 这些数字之中没有0, 

因为对于unsigned int 0 减1之后, 会变成0xffffffff, 肯定大于5, 会直接触发炸弹; 因此, 这6个输入数字就是1,2,3,4,5,6的某个排序

```assembly
0x0000000000401117 <+35>:	mov    0x0(%r13),%eax
0x000000000040111b <+39>:	sub    $0x1,%eax
0x000000000040111e <+42>:	cmp    $0x5,%eax
0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52>
0x0000000000401123 <+47>:	callq  0x40143a <explode_bomb>
```

下面这一段指令的逻辑是: 对数组进行一个嵌套循环, 判断的目的是保证: 

- bcdef不和a相等

- cdef不和b相等

- def不和c相等

- ef不和d相等

- f不和e相等

```assembly
0x0000000000401114 <+32>:	mov    %r13,%rbp
0x0000000000401117 <+35>:	mov    0x0(%r13),%eax
0x000000000040111b <+39>:	sub    $0x1,%eax
0x000000000040111e <+42>:	cmp    $0x5,%eax
0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52>
0x0000000000401123 <+47>:	callq  0x40143a <explode_bomb>
0x0000000000401128 <+52>:	add    $0x1,%r12d
0x000000000040112c <+56>:	cmp    $0x6,%r12d
0x0000000000401130 <+60>:	je     0x401153 <phase_6+95>
0x0000000000401132 <+62>:	mov    %r12d,%ebx
0x0000000000401135 <+65>:	movslq %ebx,%rax
0x0000000000401138 <+68>:	mov    (%rsp,%rax,4),%eax
0x000000000040113b <+71>:	cmp    %eax,0x0(%rbp)
0x000000000040113e <+74>:	jne    0x401145 <phase_6+81>
0x0000000000401140 <+76>:	callq  0x40143a <explode_bomb>
0x0000000000401145 <+81>:	add    $0x1,%ebx
0x0000000000401148 <+84>:	cmp    $0x5,%ebx
0x000000000040114b <+87>:	jle    0x401135 <phase_6+65>
0x000000000040114d <+89>:	add    $0x4,%r13
0x0000000000401151 <+93>:	jmp    0x401114 <phase_6+32>
```






# Common commands in gdb
## Reference
https://sourceware.org/gdb/onlinedocs/gdb/Output-Formats.html

layout reg

b func_name - set break point at function

b *address - set break point at address

info b - show all break points

delete - remove all break points


c - resume execution, until hitting the next break point

nexti / ni - Execute one machine instruction, but if it is a function call, proceed until the function returns.

finish/fin - return from current stack frame(return current function call)

return - drop current frame


**view data in memory at a certain address**

x/x $rsp - Print as integer in hexadecimal

x/d $rsp - Print as integer in signed decimal

x/s $rsp - Regard as a string


x/6wx $rsp - 查看rsp起始往栈底方向6个 words的数据, x表示用hexadecimal格式输出


打印带偏移的地址数据

(gdb) print *(int *) ($rsp+8)




