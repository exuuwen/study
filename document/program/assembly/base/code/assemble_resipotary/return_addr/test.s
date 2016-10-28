#assemble func
.section .data
output:
	.ascii  "hello world"
.section .text
.type get_string, @function
.globl  get_string
get_string:
	pushl %ebp
	movl %esp, %ebp

	movl $output, %eax 

	movl %ebp, %esp
	popl %ebp

	ret 
