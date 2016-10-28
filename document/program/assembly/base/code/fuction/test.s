#command line  必须按照_start方式
#cr7@cr7-virtual-machine:~/pro/program/assemle/fuction$ as -gbstabs -o test.o test.s
#cr7@cr7-virtual-machine:~/pro/program/assemle/fuction$ ld -dynamic-linker /lib/ld-linux.so.2  -o test  -lc test.o

.section .data
output:
	.asciz "there are %d param:\n"
output2:
	.asciz "%s\n"
.section .text
.globl _start
_start:
	movl (%esp), %ecx
	pushl %ecx
	pushl $output
	call printf
	addl $4, %esp
	popl %ecx

	movl %esp, %ebp

loop1:
	addl $4, %ebp
	pushl %ecx
	pushl (%ebp)
	pushl $output2
	call printf
	addl $8, %esp
	popl %ecx
	loop loop1
	
	

	

	movl $0, %ebx
	movl $1, %eax
	int $0x80


	
