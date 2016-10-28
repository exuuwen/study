#addx  signed & unsigned
.section .data
output:
	.ascii "the sum  value is %d\n"
.section .text
.globl main
main:
	movl $0, %eax
	movb $100, %al
	addb $10, %al

	pushl %eax
	pushl $output
	call printf
	addl $8, %esp

	movl $0, %eax
	movb $100, %al
	addb $200, %al
	jc over
	pushl %eax
	pushl $output
	call printf
	addl $8, %esp

over:
	movl $0, %eax
	movb $-100, %al
	addb $10, %al

	movsx %al, %ebx
	pushl %ebx
	pushl $output
	call printf
	addl $8, %esp

	movl $0, %eax
	movb $-100, %al
	addb $-100, %al
	jo end
	movsx %al, %ebx
	pushl %ebx
	pushl $output
	call printf
	addl $8, %esp
end:	
	movl $1, %eax
	movl $0, %ebx
	int $0x80

	
