#loop func
.section .data
output:
	.ascii "the value is %d\n"
.section .text
.globl main
main:
	movl $10, %ecx  #利用ecx寄存器实现计数
	movl $0,  %eax
	
	jecxz done  #check ecx
loop1:
	addl %ecx, %eax
	loop loop1
done:
	pushl %eax
	pushl $output
	call printf
	addl $8, %esp
	
	movl $1, %eax	
	movl $0, %ebx
	int $0x80
	
