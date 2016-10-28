#sbbx:unsigned   dec:better unsigned 
#mulx  imulx
.section .data
output:
	.ascii "the div  value is %d, remainder is %d\n"#must be qd
data1:
	.quad 8335
data2:
	.int 25
data3:
	.int -2235
data4:
	.short 25

.section .text
.globl main
main:
#unsigned
	movl data1, %eax   #64/32
	movl data1+4, %edx
	divl data2

	pushl %edx
	pushl %eax
	pushl $output
	call printf
	addl $12, %esp


#signed
	movw data3, %ax #32/16
	movw data3+2, %dx
	idivw  data4

	movsx %dx, %edx
	movsx %ax, %eax
	pushl %edx
	pushl %eax
	pushl $output
	call printf
	addl $12, %esp

	

	movl $1, %eax
	movl $0, %ebx
	int $0x80

	
