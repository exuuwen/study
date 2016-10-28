#gcc -o test  hello_world_printf.s
#gcc -gstabs -o  test  hello_world_printf.s  #debug   break *func+num 设置断点   run   next  s  print /x $ecx
#must be main
.section .data
output:
	.ascii "hello world printf:%d\n"
.section .text
.globl main
main:
pushl $2
pushl $output
call printf
addl $8, %esp
pushl $0
call exit
