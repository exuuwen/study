/*汇编红函数
*/
#include <stdio.h>
#define GREAT(data1, data2, result) ({ \
	asm( \
	"cmp %1, %2\n\t" \
	"jge 0f\n\t"  \
	"movl %1, %0\n\t"  \
	"jmp 1f\n\t" \
	"0:\n\t"    \
	"movl %2, %0\n\t" \
	"1:\n\t"  \
	:"=r"(result)  \
	:"r"(data1), "r"(data2)); })

int main()
{
	int data1 = 10, data2 = 20;
	int result;
	
	GREAT(data1, data2, result);

	printf("the larger is %d\n", result);
}
