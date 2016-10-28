.section .data
output:
	.ascii "the %d value is %d\n"
values:
	.int 3, 7, 8, 1, 9, 2, 5 ,10, 6, 4
.section .bss
	.lcomm  index 4
.section .text
.globl main
main:
movl $0, %edi
loop:
movl values(, %edi, 4),  %eax
pushl %eax
inc %edi
movl %edi, index
pushl index
pushl $output
call printf
addl $12, %esp
cmpl $10, %edi
jne loop

#change data
movl $values, %edi
movl $100, 4(%edi)

movl $1, %edi
movl values(, %edi, 4), %ebx
movl $1, %eax
int $0x80
