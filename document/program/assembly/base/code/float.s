#小数float double 
#(gdb) x/gf &data2
#(gdb) x/f &data1
.section .data

output:
	.ascii "the movsx value is %f and the movzx value is %ld\n"
data2:
	.double 2122.34223
data1:
	.float  122.2
value:
	.float 1.2, -2.333, 7778.2, 3838.4
.section .bss
	.lcomm datal, 8
	.lcomm datas, 4
	.lcomm data, 16
.section .text
.globl main
main:

#fldx fstx
	fldl data2#会push
	fstl datal
	
	flds data1
	fstl datas #不pop
#package movups
	movups value, %xmm0
	movups %xmm0, data
	
	movl $1, %eax
	movl $0, %ebx
	int $0x80

	
