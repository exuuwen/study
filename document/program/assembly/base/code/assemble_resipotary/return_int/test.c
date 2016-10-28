#include <stdio.h>

int main()
{
	int a = 2;
	int b = 3;
	int result;

	result = addfunc(a, b);
	printf("result:%d\n", result);	

	result = addfunc(26, 29);
	printf("result:%d\n", result);
	return 0;
}
