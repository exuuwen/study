#cmpsx
#小写转大写
#gdb:  x/s &value
.section .data
output1:
	.ascii "test12"
length1:
	.int  6
output2:
	.ascii "test1"
length2:
	.int  5

.section .text
.globl main
main:
	lea output1, %esi
	lea output2, %edi
	movl length1, %eax
	movl length2, %ecx
	cmpl %ecx, %eax
	ja longer
	xchg %eax, %ecx
longer:	
	cld
	repe cmpsb
	je equal
	jg greater
less:
	movl $1, %eax
	movl $255, %ebx
	int $0x80	
greater:
	movl $1, %eax
	movl $1, %ebx
	int $0x80
equal:
	movl length1, %eax
	movl length2, %ecx
	cmpl %ecx, %eax
	jg greater
	jl less
end:	

	movl $1, %eax
	movl $0, %ebx
	int $0x80
