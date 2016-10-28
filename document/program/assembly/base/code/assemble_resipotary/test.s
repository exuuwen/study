#assemble func
.section .text
.type get_area, @function
.globl  get_area
get_area:
	pushl %ebp
	movl %esp, %ebp

	
	fldl 8(%ebp)  #first is double at 8(len is 8)
	fimul 16(%ebp)	#second is int  at 16

	movl %ebp, %esp
	popl %ebp

	ret 
