#assemble func
.section .data
data:
	.float  2.3
.section .text
.type get_area, @function
.globl  get_area
get_area:
	pushl %ebp
	movl %esp, %ebp
	
	fldpi
	filds 8(%ebp)
	fmul %st(0), %st(0)
	fmul %st(1), %st(0)	

	movl %ebp, %esp
	popl %ebp

	ret 
