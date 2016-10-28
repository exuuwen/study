/*jmp
1. 只能在头一个asm中jmp
2. 在意个文件中 标签只能用一次
*/
#include <stdio.h>

int main()
{
	int length = 25;
	int data1 = 10, data2 = 20;
	int result;
	
	asm(
	"cmp %1, %2\n\t"
	"jge 0f\n\t"
	"movl %1, %0\n\t"
	"jmp 1f\n\t"
	"0:\n\t"
	"movl %2, %0\n\t"
	"1:\n\t"
	:"=r"(result)
	:"r"(data1), "r"(data2)
	);

	printf("the larger is %d\n", result);
}
