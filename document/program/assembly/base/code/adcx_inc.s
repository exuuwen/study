#adcx:both sign  unsigned  inc:better unsigned

.section .data
output:
	.ascii "the adc sum  value is %qd\n"#must be qd
data1:
	.quad 1234567890
data2:
	.quad 9876543210
value:
	.int 1, 2, 3
.section .text
.globl main
main:
	
	movl data1, %eax
	inc %eax  #register
	movl data1+4, %ebx #数组也可以这样比如value+4 也可以value（，index,4)
	movl data2, %ecx
	movl data2+4, %edx

	addl %ecx, %eax
	adcl %edx,  %ebx

	pushl %ebx
	pushl %eax
	pushl $output
	call printf
	addl $12, %esp

	



	movl $1, %eax
	movl $0, %ebx
	int $0x80

	
