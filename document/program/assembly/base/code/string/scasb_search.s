#scabs
#gdb:  x/s &value
.section .data
output1:
	.ascii "test12 - haha.\n"
length1:
	.int  15
output2:
	.ascii "-"

.section .text
.globl main
main:
	lea output2, %esi
	lea output1, %edi
	lodsb
	movl length1, %ecx
	
	repne scasb
	jne notfound

	subl %ecx, length1

	movl $1, %eax
	movl length1, %ebx
	int $0x80

notfound:	

	movl $1, %eax
	movl $0, %ebx
	int $0x80
