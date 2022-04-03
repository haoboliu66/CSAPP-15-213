
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
   0x0000000000401357 <+31>:	cmp    %eax,%r12d    # 比较length
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

# Phase_2


