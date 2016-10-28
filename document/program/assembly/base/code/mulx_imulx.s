#sbbx:unsigned   dec:better unsigned 
#mulx  imulx
.section .data
output:
	.ascii "the mul  value is %qd\n"#must be qd
data1:
	.int 123456
data2:
	.int 456789
put:
	.ascii "the mul  value is %d\n"#must be qd
.section .text
.globl main
main:
#unsigned
	movl data1, %eax #32 x 32
	mull data2

	pushl %edx
	pushl %eax
	pushl $output
	call printf
	addl $12, %esp

	movl $0, %eax # 8 x 8
	movb $10, %al
	movb $39, %bl
	mulb %bl
	
	movl $0, %ebx
	movzx %ax, %ebx
	pushl %ebx
	pushl $put
	call printf
	addl $8, %esp
#signed
	movl $0, %eax # 8 x 8
	movb $-100, %al
	movb $100, %bl
	imulb %bl

	movl $0, %ebx
	movsx %ax, %ebx
	pushl %ebx
	pushl $put
	call printf
	addl $8, %esp

	movl $1, %eax
	movl $0, %ebx
	int $0x80

	
