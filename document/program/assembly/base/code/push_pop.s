#push&pop
.section .data
output:
	.ascii "before push the %d value is %d\n"
values:
	.int 3, 7, 8, 1, 9, 2, 5, 10, 6, 4
output1:
	.ascii "after pop the %d value is %d\n"
.section .bss
	.lcomm  index 4
.section .text
.globl main
main:
movl $0, %edi
loop1:
movl values(, %edi, 4),  %eax
pushl %eax
inc %edi
movl %edi, index
pushl index
pushl $output
call printf
addl $12, %esp
cmpl $10, %edi
jne loop1


movl $0, %edi
loop_push:
pushl values(, %edi, 4)
inc %edi
cmpl $10, %edi
jne loop_push


movl $0, %edi
loop_pop:
popl values(, %edi, 4)
inc %edi
cmpl $10, %edi
jne loop_pop



movl $0, %edi
loop2:
movl values(, %edi, 4),  %eax
pushl %eax
inc %edi
movl %edi, index
pushl index
pushl $output1
call printf
addl $12, %esp
cmpl $10, %edi
jne loop2

movl $0, %ebx
movl $1, %eax
int $0x80
