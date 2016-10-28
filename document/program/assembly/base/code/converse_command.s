#converse conmand  
.section .data
output:
	.ascii "the value is 0x%lx, after bswap value is 0x%lx\n"
value:
	.int 5
value1:
	.int 10
output1:
	.ascii "the svalue is 0x%lx, after cmpxchg value is 0x%lx\n"

.section .text
.globl main
main:
#bswap  
movl $0x12345678, %ebx
movl %ebx, %eax
bswap %ebx
pushl %ebx
pushl %eax
pushl $output
call printf
addl $8, %esp

#cmpxchg
movl value, %eax
movl value1, %ebx
cmpxchg %ebx, value
pushl value
pushl %eax
pushl $output1
call printf
addl $12, %esp



#xchg
movl $1, %ebx
movl $20, %eax
xchg %ebx, %eax
int $0x80
