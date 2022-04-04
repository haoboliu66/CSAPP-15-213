
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

> `TEST` sets the zero flag, `ZF`, when the result of the AND operation is zero. If two operands are equal, their bitwise AND is zero when both are zero. > `TEST` also sets the sign flag, `SF`, when the most significant bit is set in the result, and the parity flag, `PF`, when the number of set bits is even.



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

- 两个string比较必须是完全相等, 才能返回0; 所以根据mov $0x402400,%esi, 很明显是函数传参; 因此我们只需要查看第二个传入的参数, 即以%esi地址开头的string是什么, 就是我们要的答案
  - 方法1: 进入strings_not_equal的stack frame内, 直接打印(gdb) print (char*)$rsi
  - 方法2: 在phase_1的 stack frame内, 根据调用strings_not_equal前的指令:  mov $0x402400,%esi, 就可以知道目标string的起始地址是0x402400, 就可以直接打印(gdb) print (char*) 0x402400
- 结果都是: **Border relations with Canada have never been better.**



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

```assembly
(gdb) disassemble phase_3
Dump of assembler code for function phase_3:
=> 0x0000000000400f43 <+0>:	sub    $0x18,%rsp
0x0000000000400f47 <+4>:	lea    0xc(%rsp),%rcx
0x0000000000400f4c <+9>:	lea    0x8(%rsp),%rdx
0x0000000000400f51 <+14>:	mov    $0x4025cf,%esi
0x0000000000400f56 <+19>:	mov    $0x0,%eax
0x0000000000400f5b <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
0x0000000000400f60 <+29>:	cmp    $0x1,%eax  # %eax必须大于1
0x0000000000400f63 <+32>:	jg     0x400f6a <phase_3+39>
0x0000000000400f65 <+34>:	callq  0x40143a <explode_bomb>
0x0000000000400f6a <+39>:	cmpl   $0x7,0x8(%rsp) # 0x8(%rsp)必须小于7
0x0000000000400f6f <+44>:	ja     0x400fad <phase_3+106>
0x0000000000400f71 <+46>:	mov    0x8(%rsp),%eax
0x0000000000400f75 <+50>:	jmpq   *0x402470(,%rax,8)
0x0000000000400f7c <+57>:	mov    $0xcf,%eax   # 207 => $eax
0x0000000000400f81 <+62>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f83 <+64>:	mov    $0x2c3,%eax
0x0000000000400f88 <+69>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f8a <+71>:	mov    $0x100,%eax
0x0000000000400f8f <+76>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f91 <+78>:	mov    $0x185,%eax
0x0000000000400f96 <+83>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f98 <+85>:	mov    $0xce,%eax
0x0000000000400f9d <+90>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f9f <+92>:	mov    $0x2aa,%eax
0x0000000000400fa4 <+97>:	jmp    0x400fbe <phase_3+123>
0x0000000000400fa6 <+99>:	mov    $0x147,%eax
0x0000000000400fab <+104>:	jmp    0x400fbe <phase_3+123>
0x0000000000400fad <+106>:	callq  0x40143a <explode_bomb>
0x0000000000400fb2 <+111>:	mov    $0x0,%eax
0x0000000000400fb7 <+116>:	jmp    0x400fbe <phase_3+123>
0x0000000000400fb9 <+118>:	mov    $0x137,%eax
0x0000000000400fbe <+123>:	cmp    0xc(%rsp),%eax
0x0000000000400fc2 <+127>:	je     0x400fc9 <phase_3+134>
0x0000000000400fc4 <+129>:	callq  0x40143a <explode_bomb>
0x0000000000400fc9 <+134>:	add    $0x18,%rsp
0x0000000000400fcd <+138>:	retq
```

0x0000000000400f6a <+39>:	cmpl   $0x7,0x8(%rsp)   意味着 0x8(%rsp)必须小于7
0x0000000000400f6f <+44>:	ja     0x400fad <phase_3+106>  ja意味着是unsigned value之间比较, 所以 0x8(%rsp)的范围是[0,7)


**phase_3 solution**:

0 207

1 311

2 to be continued

3 to be continued

4 to be continued

5 to be continued

6 to be continued



# Phase_4

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
source code could be the following:
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

假设数组内容是[a, b, c, d, e, f]
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


从<+95>开始到<+121>这段指令是一个循环逻辑:

%rsp就是第一个int的地址, 0x18(%rsp) 等于 %rsp + 24, 是数组越界之外的位置(数组长度是6, 偏移量最多是 $rsp + 20)

相当于%rsi所在的地址是越过了整个数组的下一个位置(offset 24), 

%rax是拿到了%r14的值, 也即%rsp的值, 所以%rax是从栈顶开始, 然后%rax每次加4, cmp  %rsi,%rax 一直比较, 直到%rax越过了%rsi的位置, 跳出循环时, %rax是%rsi + 4的地址

这段逻辑是用7减去数组的每一个值, 结果再放回原位置

```assembly
0x0000000000401153 <+95>:	lea    0x18(%rsp),%rsi
0x0000000000401158 <+100>:	mov    %r14,%rax
0x000000000040115b <+103>:	mov    $0x7,%ecx
0x0000000000401160 <+108>:	mov    %ecx,%edx
0x0000000000401162 <+110>:	sub    (%rax),%edx
0x0000000000401164 <+112>:	mov    %edx,(%rax)
0x0000000000401166 <+114>:	add    $0x4,%rax
0x000000000040116a <+118>:	cmp    %rsi,%rax
0x000000000040116d <+121>:	jne    0x401160 <phase_6+108>
```

假设: 原始数组2, 1, 6, 5, 4, 3 => 计算后的数组5, 6, 1, 2, 3, 4 (由低地址到高地址)


从<+123>开始到<+181>这段指令的逻辑:

```assembly
0x000000000040116f <+123>:	mov    $0x0,%esi
0x0000000000401174 <+128>:	jmp    0x401197 <phase_6+163>

0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx
0x000000000040117a <+134>:	add    $0x1,%eax
0x000000000040117d <+137>:	cmp    %ecx,%eax
0x000000000040117f <+139>:	jne    0x401176 <phase_6+130>
0x0000000000401181 <+141>:	jmp    0x401188 <phase_6+148>

0x0000000000401183 <+143>:	mov    $0x6032d0,%edx
0x0000000000401188 <+148>:	mov    %rdx,0x20(%rsp,%rsi,2)
0x000000000040118d <+153>:	add    $0x4,%rsi
0x0000000000401191 <+157>:	cmp    $0x18,%rsi
0x0000000000401195 <+161>:	je     0x4011ab <phase_6+183>

0x0000000000401197 <+163>:	mov    (%rsp,%rsi,1),%ecx
0x000000000040119a <+166>:	cmp    $0x1,%ecx
0x000000000040119d <+169>:	jle    0x401183 <phase_6+143>
0x000000000040119f <+171>:	mov    $0x1,%eax
0x00000000004011a4 <+176>:	mov    $0x6032d0,%edx
0x00000000004011a9 <+181>:	jmp    0x401176 <phase_6+130>
```

从 <+128> mov $0x0, %esi
   <+163> mov (%rsp,%rsi,1), %ecx
可以看出, %rsi从0开始, 在循环逻辑中每次%rsi加4, 基值是%rsp, 明显是数组的索引取值

当每个数字放入%ecx, 这个数字会和1进行比较
> 0x0000000000401197 <+163>:	mov    (%rsp,%rsi,1),%ecx
> 0x000000000040119a <+166>:	cmp    $0x1,%ecx
> 0x000000000040119d <+169>:	jle    0x401183 <phase_6+143>

- 如果 > 1, 访问内存位置0x6032d0, 把0x6032d0这个值本身或后面的某个地址放入%rsp的偏移位置处, 具体的地址取决于当前这个int的值和1的差值, 差值会决定从起始位置0x6032d0向后走几步, 然后拿到对应的数值

- 这个数字如果 <= 1, 把0x6032d0这个值直接存到%rsp对应偏移的位置

从 <+148> mov %rdx,0x20(%rsp,%rsi,2)
   <+153> add $0x4,%rsi
可以看出:
最后存入的位置取决于%rsi产生的offset, 也就是取决于当前这个数字位于数组的索引
最后存入的位置的base值是%rsp+32(decimal value of 0x20) , 数字之间的间隔是8
也就是说, 存入的地址分别是%rsp+32, %rsp+40, %rsp+48, %rsp+56, %rsp+64, %rsp+72


```c
# source code could be like the following
int phase_6(){
  int r;
  int arr[] = {1,2,3,4,5,6};
  int i = 0;
  for(; i < 6; i++){
    int cur = arr[i]; // mov  (%rsp,%rsi,1),%ecx
    if(cur > 1)  // cmp  $0x1,%ecx 
    	int m = 1; // mov  $0x1,%eax
      r = 76;  // mov  $0x6032d0,%edx;  jmp  0x401176 <phase_6+130>
      redo:
      &r = *(&r + 8); // mov  0x8(%rdx),%rdx  <+130>
      m += 1;  // add  $0x1,%eax
      if(m != cur){  // cmp %ecx,%eax
        goto redo;  // jne 0x401176 <phase_6+130>
      }
    	
  }else{
    r = 76; // mov $0x6032d0,%edx
		*(arr + i * 2) = r;  // mov %rdx,0x20(%rsp,%rsi,2) 
  }
  
}
```

使用 1 2 3 4 5 6 数组作为例子走出的结果: 从0x6032d0这个值为地址往后存着数值

每次放到%rdx的数据和其对应的于地址0x6032d0的offset分别是:
| offset | number   |
| ------ | -------- |
| 0      | 0x6032d0 |
| 1      | 0x6032e0 |
| 2      | 0x6032f0 |
| 3      | 0x603300 |
| 4      | 0x603310 |
| 5      | 0x603320 |

e.g. 如果是数组的第二个数字a, 对应的%rsi就是4, 这个数字就会被存到%rsp + 32 + 4 * 2的位置

把这几个值都放到了对应的offset的地址处, 就跳出了循环, 总共依次写入了6个地址值



后续两端的推理过程有点类似套娃, 画图解决的, 不赘述了...


接着: 下一段指令逻辑

推理过程在: https://www.processon.com/diagraming/5f383d11f346fb06dec524ab

大概过程是把%rsp + 32 到 %rsp + 72范围内

把高一个地址的值 存到 低地址存的值 + 8 这个地址上, 举例如图:

![](https://raw.githubusercontent.com/haoboliu66/PicBed/master/img/202204031251199.png)



![](https://raw.githubusercontent.com/haoboliu66/PicBed/master/img/202204031252088.png)



```assembly
0x00000000004011ab <+183>:	mov    0x20(%rsp),%rbx
0x00000000004011b0 <+188>:	lea    0x28(%rsp),%rax
0x00000000004011b5 <+193>:	lea    0x50(%rsp),%rsi
0x00000000004011ba <+198>:	mov    %rbx,%rcx   # %rsp + 32
0x00000000004011bd <+201>:	mov    (%rax),%rdx  # (%rsp + 40)
0x00000000004011c0 <+204>:	mov    %rdx,0x8(%rcx)
0x00000000004011c4 <+208>:	add    $0x8,%rax
0x00000000004011c8 <+212>:	cmp    %rsi,%rax
0x00000000004011cb <+215>:	je     0x4011d2 <phase_6+222>
0x00000000004011cd <+217>:	mov    %rdx,%rcx
0x00000000004011d0 <+220>:	jmp    0x4011bd <phase_6+201>
```



最后一段指令的逻辑:

推理过程在: https://www.processon.com/diagraming/5f383d11f346fb06dec524ab

```assembly
0x00000000004011d2 <+222>:	movq   $0x0,0x8(%rdx)
0x00000000004011da <+230>:	mov    $0x5,%ebp
0x00000000004011df <+235>:	mov    0x8(%rbx),%rax
0x00000000004011e3 <+239>:	mov    (%rax),%eax
0x00000000004011e5 <+241>:	cmp    %eax,(%rbx)
0x00000000004011e7 <+243>:	jge    0x4011ee <phase_6+250>
0x00000000004011e9 <+245>:	callq  0x40143a <explode_bomb>
0x00000000004011ee <+250>:	mov    0x8(%rbx),%rbx
0x00000000004011f2 <+254>:	sub    $0x1,%ebp
0x00000000004011f5 <+257>:	jne    0x4011df <phase_6+235>

0x00000000004011f7 <+259>:	add    $0x50,%rsp
0x00000000004011fb <+263>:	pop    %rbx
0x00000000004011fc <+264>:	pop    %rbp
0x00000000004011fd <+265>:	pop    %r12
0x00000000004011ff <+267>:	pop    %r13
0x0000000000401201 <+269>:	pop    %r14
0x0000000000401203 <+271>:	retq
```

**phase_6 solution: 4 3 2 1 6 5**



# Reference

## Lectures
https://www.cs.cmu.edu/afs/cs/academic/class/15213-f16/www/schedule.html

## Self-study lab 
http://csapp.cs.cmu.edu/3e/labs.html

## Common commands in gdb
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




