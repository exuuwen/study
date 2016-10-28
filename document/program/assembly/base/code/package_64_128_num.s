#打包64位和128位数   
#寄存器 print /x $name
#data  x /n(cdx)(bdwg)  n:次数  2：字符，10进制，16进制   3：以byte double word 64位
.section .data

output:
	.ascii "the  movsx value is %ld and the movzx value is %ld\n"
datal2:
	.quad 0, 0
datal1:
	.quad 0
data2:
	.int -1, 0, 1, 2
data1:
	.int 1, 1
.section .text
.globl main
main:

	movq data1, %mm0
	movq %mm0, datal1

	movdqu data2, %xmm0
	movdqu %xmm0, datal2
	
	
	

	movl $1, %eax
	movl $0, %ebx
	int $0x80

	
