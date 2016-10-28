#systemcall
#ret = write(fd, *buf, len)  syscall_num: 4
.section .data

output:
	.ascii "this is a message to console\n"
output_end:
	.equ len, output_end - output
.section .text
.globl main
main:
#not
	movl $4, %eax  #syscall_num
	movl $1, %ebx  #fd
	movl $output, %ecx  # buf
	movl $len, %edx  #len
	int $0x80
	


end:	
	movl %eax, %ebx  #%eax为ret
	movl $1, %eax
	
	int $0x80
