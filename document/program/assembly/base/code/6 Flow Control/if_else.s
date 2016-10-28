#if else
.section .data
output:
	.ascii "the max value is %d\n"
.section .text
.globl main
main:
	movl $30, %eax
	movl $20, %ebx
if:
	cmp %eax, %ebx
	jae  else
	pushl %eax
	jmp end
else:
	pushl %ebx
end:
	pushl $output
	call printf
	addl $8, %esp
	
	movl $1, %eax
	movl $0, %ebx
	int $0x80
	


	
