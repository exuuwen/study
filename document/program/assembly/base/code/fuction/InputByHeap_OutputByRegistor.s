#function
#input  by heap   output by registor
.section .data

.section .text
.globl main
main:
	
	pushl $19
	pushl $20
	call add
	addl $8, %esp

	movl %eax, %ebx
	movl $1, %eax
	int $0x80

.type add, @function
add:
	pushl %ebp
	movl %esp, %ebp

	subl $4, %esp #局部变量  not use 通过 -4(%ebp)获取

	movl 8(%ebp), %eax #偏移8是第一个参数 加4依次类推
	addl %eax, 12(%ebp) 

	movl 12(%ebp), %eax #eax为返回

	movl %ebp, %esp
	popl %ebp

	ret 
	
