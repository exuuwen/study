#cmpsx
#小写转大写
#gdb:  x/s &value
.section .data
output1:
	.ascii "a test12 - haha.\n"

.section .text
.globl main
main:
	leal output1, %edi
	movb $0, %al
	movl $0xffff, %ecx
	
	repne scasb
	jne notfound

	subl $0xffff, %ecx
	neg %ecx
	dec %ecx

	movl $1, %eax
	movl %ecx, %ebx
	int $0x80

notfound:	

	movl $1, %eax
	movl $0, %ebx
	int $0x80
