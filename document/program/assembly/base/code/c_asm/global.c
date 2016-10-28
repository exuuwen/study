/*asm and global var*/
#include <stdio.h>
int a = 10, b = 20;
int result;
int main()
{
	
	
	asm("movl a, %eax\n\t"
	"movl b, %ebx\n\t"
	"imull %ebx, %eax\n\t"
	"movl %eax, result\n\t"
	);
	
	printf("the result is %d\n", result);
	return 0;	
}
