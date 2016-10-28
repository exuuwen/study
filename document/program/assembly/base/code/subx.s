#addx  signed & unsigned
#subx  signed & unsigned
.section .data
output:
	.ascii "the sub  value is %d\n"
.section .text
.globl main
main:
	movl $0, %eax
	movb $100, %al
	subb $10, %al
	movzx %al, %ebx

	pushl %ebx
	pushl $output
	call printf
	addl $8, %esp

	movl $0, %eax
	movb $2, %al
	subb $5, %al
	jc over
	movzx %al, %ebx
	pushl %ebx
	pushl $output
	call printf
	addl $8, %esp

over:
	movl $0, %eax
	movb $-100, %al
	subb $10, %al
	
	movsx %al, %ebx
	pushl %ebx
	pushl $output
	call printf
	addl $8, %esp

	movl $0, %eax
	movb $120, %al
	subb $-20, %al
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

	
