#gcc -o test hello_world_systemcall.s
#gcc -gstabs -o  test  hello_world_printf.s  #debug   break *func+num 设置断点   run   next  s  print /x $ecx
#must be main
.section .text
.globl main
main:




lea 0x11223344(%ebx), %esi
xor %rbx, %rbx

cmpl $0xaabbccdd, 0x11223344(%r8, %rbx, 1)

cmpw $0xaabb, 0x11223344(%r8, %rbx, 1)

cmpb $0xaa, 0x11223344(%r8, %rbx, 1)
jns 0x35
movl $1, %eax
movl $0, %ebx
int $0x80
