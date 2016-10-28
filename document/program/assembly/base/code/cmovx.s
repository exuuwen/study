#get the bigest value  conditon_movx
.section .data
output:
	.ascii "the max value is %d\n"
values:
	.int 3, 7, 8, 1, 9, 2, 5 , 10, 6, 4
.section .text
.globl main
main:
movl values, %ebx
movl $1, %edi
loop:
movl values(, %edi, 4), %eax
cmp %ebx, %eax
cmova %eax, %ebx
inc %edi
cmp $10, %edi
jne loop

pushl %ebx
pushl $output
call printf
addl $8, %esp

movl $0, %ebx
movl $1, %eax
int $0x80
