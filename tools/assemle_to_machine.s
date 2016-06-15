#gcc -o test hello_world_systemcall.s
#gcc -gstabs -o  test  hello_world_printf.s  #debug   break *func+num 设置断点   run   next  s  print /x $ecx
#must be main
.section .text
.globl main
main:

lea    0x2(%rax),%rdx

movq $0x7856341278563412, %rax
movabs $0x7856341278563412,%rax
callq *0x7856341
jmpq *0x7856341
#callq *0x785634133333
#jmpq *0x785634567833
jmpq  *%rax

callq *%rax
nop
