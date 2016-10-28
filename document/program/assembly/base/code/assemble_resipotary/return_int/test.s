#assemble func
.section .text
.type addfunc, @function
.globl  addfunc
addfunc:
	pushl %ebp
	movl %esp, %ebp

	subl $4, %esp 

	movl 8(%ebp), %eax 
	addl %eax, 12(%ebp) 

	movl 12(%ebp), %eax 

	movl %ebp, %esp
	popl %ebp

	ret 
