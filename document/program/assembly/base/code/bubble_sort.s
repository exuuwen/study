#bubbles_sort  xchg
.section .data
output:
	.ascii "before sort the %d value is %d\n"
values:
	.int 3, 7, 8, 1, 9, 2, 5, 10, 6, 4
output1:
	.ascii "after sort the %d value is %d\n"
.section .bss
	.lcomm  index 4
.section .text
.globl main
main:
movl $0, %edi
loop1:
movl values(, %edi, 4),  %eax
pushl %eax
inc %edi
movl %edi, index
pushl index
pushl $output
call printf
addl $12, %esp
cmpl $10, %edi
jne loop1


movl $values, %esi
movl $9, %ecx
movl $9, %ebx
loop:
movl (%esi), %eax
cmp  %eax, 4(%esi)
jge skip
xchg  %eax, 4(%esi)
movl %eax, (%esi)
skip:
addl $4, %esi
dec %ebx
jnz  loop
dec %ecx
jz end

movl $values, %esi
movl %ecx, %ebx
jmp loop

end:
movl $0, %edi
loop2:
movl values(, %edi, 4),  %eax
pushl %eax
inc %edi
movl %edi, index
pushl index
pushl $output1
call printf
addl $12, %esp
cmpl $10, %edi
jne loop2

movl $0, %ebx
movl $1, %eax
int $0x80
