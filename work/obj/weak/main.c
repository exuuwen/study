#include <stdio.h>

__attribute__((weak)) void foo();

extern int ext;

int data; /* unint weak one handle wit COMMON*/

int strong = 10;

__attribute__((weak)) int weak2 = 2;


int main()
{

	printf("ext:%d, weak2:%d , data:%d\n", ext, weak2, data);
	if(foo)
		foo();
	else
		printf("no foo\n");
	return 0;
}
