#movsx  movzx 在调用函数里 不要用 eax ebx ecx edx 这些计数  会被函数调用用到
.section .data
unsigned_data:
	.int 56
signed_data:
	.int -56
output:
	.ascii "the  movsx value is %d and the movzx value is %d\n"
.section .text
.globl main
main:
#2 way  unsigned 
	movl $22, %eax
	movl unsigned_data, %eax
#3 way signed
	movl $-22, %eax
	mov  signed_data, %eax
	movw $0xf000, %ax

#movzx  movxs
	movl $0, %esi
	movl $0, %edi
	movl $0, %eax
	movw $0xf000, %ax
	
	movsx %ax, %esi
	movzx %ax, %edi
	pushl %edi
	pushl %esi
	pushl $output
	call printf
	addl $8, %esp
	
	movl $1, %eax
	movl $0, %ebx
	int $0x80

	
