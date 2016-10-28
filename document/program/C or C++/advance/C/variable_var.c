#include <stdio.h>



int func1(int data)
{
	printf("in func1:%d\n", data);
	return 0;
}

int func2(int a, int b)
{
	printf("in func2\n");
	if(a > b)
		return a;
	return b;
}
struct test
{
	int a;
	int (*func)(int a, ...);
};

int main()
{
	struct test test1, test2;
	int ret = -1;

	test1.a = 1;
	test1.func = func1;
	ret = test1.func(11);
	printf("ret:%d\n",ret);

	test2.a = 2;
	test2.func = func2;
	ret = test2.func(1, test2.a);
	printf("ret:%d\n",ret);
	
	return 0;
}
