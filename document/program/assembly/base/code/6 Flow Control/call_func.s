#call func
.section .data
output:
	.ascii "the value is %d\n"
.section .text
.globl main
main:
	pushl $1
	pushl $output
	call printf
	addl $8, %esp
	call overhead
	pushl $3
	pushl $output
	call printf
	addl $8, %esp
	movl $1, %eax
	movl $0, %ebx
	int $0x80
overhead:
	pushl %ebp
	movl %esp, %ebp
	pushl $2
	pushl $output
	call printf
	addl $8, %esp
	movl %ebp, %esp
	popl %ebp
	ret
