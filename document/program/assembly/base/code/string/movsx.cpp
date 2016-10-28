#movsx test
#gdb:  x/s &value
.section .data

output:
	.ascii "this is a string test.\n"

.section .bss
	.lcomm value 23
.section .text
.globl main
main:
	leal output, %esi
	leal value,  %edi
	movl $23, %ecx
	
	#std   #back direction
	
	cld  #start direction

	rep movsb  
	

	
end:	

	movl $1, %eax
	movl $0, %ebx
	int $0x80
