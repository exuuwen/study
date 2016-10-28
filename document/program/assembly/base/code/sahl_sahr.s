#sal & sar & shr
#mulx  imulx
.section .data
output:
	.ascii "the sa(h)r  value is 0x%x\n "
datal:
	.short 1, 0x8001#-32767
output1:
	.ascii "cf is 1 \n "
datar:
	.int -256
output2:
	.ascii "the sal  value is 0x%x\n"
.section .text
.globl main
main:
#sal
	movw datal, %ax
	salw $2, %ax #the same as shll,都是移掉最高为到c中
	
	movsx %ax, %eax
	pushl %eax
	pushl $output2
	call printf
	addl $8, %esp 

	movw datal+2, %ax
	salw $1, %ax
	#jc ok
	
	movsx %ax, %eax
	pushl %eax
	pushl $output2
	call printf
	addl $8, %esp 
ok:
	pushl $output1
	call printf
	addl $4, %esp

	
#sar
	movl datar, %eax
	sarl $2, %eax

	pushl %eax
	pushl $output
	call printf
	addl $8, %esp 

#shr
	movl datar, %eax
	shrl $2, %eax

	pushl %eax
	pushl $output
	call printf
	addl $8, %esp 
	
	movl $1, %eax
	movl $0, %ebx
	int $0x80
