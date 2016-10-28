#lodsb   stosb  
#小写转大写
#gdb:  x/s &value
.section .data

output:
	.ascii "test FOR small TO big.\n"

.section .bss
	.lcomm value 23
.section .text
.globl main
main:
	
	pushl $output
	call printf
	addl $4, %esp	
	
	leal output, %esi
	leal output,  %edi
	cld  
	movl $23, %ecx
loop1:
	lodsb 
	cmpb $'a', %al
	jl skp
	cmp $'z', %al
	jg skp
	subb $0x20, %al
skp:
	stosb  
	loop loop1

	pushl $output
	call printf
	addl $4, %esp	
	

	
end:	

	movl $1, %eax
	movl $0, %ebx
	int $0x80
