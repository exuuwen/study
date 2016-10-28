#not xor and or test
#mulx  imulx
.section .data

data:
	.int 0xf0f0f0f0
output:
	.ascii "the value is 0x%x\n"


.section .text
.globl main
main:
#not
	movl data, %eax
	not %eax
	
	pushl %eax
	pushl $output
	call printf
	addl $8, %esp
#xor
	movl data, %eax
	xor $0xf0f0f0f, %eax
	
	pushl %eax
	pushl $output
	call printf
	addl $8, %esp

#and
	movl data, %eax
	and $0xffff, %eax
	
	pushl %eax
	pushl $output
	call printf
	addl $8, %esp

#and
	movl data, %eax
	or $0xffff, %eax
	
	pushl %eax
	pushl $output
	call printf
	addl $8, %esp
#test
	movl data, %eax
	test $0x1, %eax
	jz end

	pushl %eax
	pushl $output
	call printf
	addl $8, %esp
end:	

	movl $1, %eax
	movl $0, %ebx
	int $0x80
