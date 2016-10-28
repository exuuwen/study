#function
#input&output  globale value or register
.section .data
output1:
	.ascii "the data is %d\n"
.section .bss
	.lcomm input, 4
	.lcomm output, 4
.section .text
.globl main
main:
	
	movl $9, input
	call fuc

	movl $1, %eax
	movl output, %ebx
	int $0x80

.type fuc, @function
fuc:
	movl input, %edx
	addl $10, %edx
	movl %edx, output
	ret 
	
