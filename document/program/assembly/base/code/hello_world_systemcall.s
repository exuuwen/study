#gcc -o test hello_world_systemcall.s
#gcc -gstabs -o  test  hello_world_printf.s  #debug   break *func+num 设置断点   run   next  s  print /x $ecx
#must be main
.section .data
output:
	.ascii "hello world control consloe\n"
.section .text
.globl main
main:
movl $4, %eax
movl $1, %ebx
movl $output, %ecx
movl $42, %edx
int $0x80
movl $1, %eax
movl $0, %ebx
int $0x80
