#for 在调用函数里 不要用 eax ebx ecx edx 这些计数  会被函数调用用到
.section .data
output:
	.ascii "the  value is %d\n"
.section .text
.globl main
main:
	movl $0, %esi
	movl $5, %edi
	jmp do
for:	
	dec %edi
	js end
do:	
	addl %edi, %esi
	pushl %esi
	pushl $output
	call printf
	addl $8, %esp
	jmp for
end:	
	movl $1, %eax
	movl $0, %ebx
	int $0x80

	
