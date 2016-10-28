/*asm and global var*/
#include <stdio.h>

int main()
{
	
	char input[30] = "this is a test message.\n";
	char output[30];
	int length = 25;
	int data1 = 10, data2 = 20;
	int result;

	asm(
	"imull %%ebx, %%ecx\n\t"
	"movl %%ecx, %%eax\n\t"
	:"=a"(result)
	:"b"(data1), "c"(data2)
	);
	printf("the result is %d\n", result);

	asm(
	"cld \n\t"
	"rep movsb\n\t"
	:
	:"S"(input), "D"(output), "c"(length)
	);
	
	printf("the output is %s", output);
		
	data1 = 20;
	asm(
	"imull %1, %2\n\t"
	"movl %2, %0\n\t"
	:"=r"(result)
	:"r"(data1), "r"(data2)
	);
	printf("the result is %d\n", result);
	
	data1 = 30;
	asm(
	"imull %1, %0\n\t"
	:"=r"(data2)
	:"r"(data1), "0"(data2)
	);
	printf("the result is %d\n", data2);
	
	asm(
	"imull %[value2], %[value1]\n\t"
	:[value1] "=r"(data2)
	:[value2] "r"(data1), "0"(data2)
	);
	printf("the result is %d\n", data2);

	data2 = 20;
	asm(
	"movl %1, %%eax\n\t"
	"imull %%eax, %0\n\t"
	:"=r"(data2)
	:"r"(data1), "0"(data2)
	:"%eax" /*告诉编译器在代码中使用了未初始化的寄存器*/
	);
	printf("the result is %d\n", data2);

	data1 = 10;
	data2 = 20;
	asm(
	"\n\t"
	:"=r"(data1), "=r"(data2)
	:"0"(data2),"1"(data1)     /*data1输入的寄存器作为data2输出的寄存器，data2输入的寄存器作为data1输出的寄存器 */
	);
	printf("the data1 is %d\n", data1);
	printf("the data2 is %d\n", data2);
	return 0;	
}
