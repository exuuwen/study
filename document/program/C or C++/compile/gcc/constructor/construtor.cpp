#include <stdio.h>
#include <stdlib.h>
void at_exit()
{
	printf("in the at exit\n");
}

class A
{
public:
	A()
	{
		printf("in A constructor\n");
	}
	
	~A()
	{
		printf("in A  destructor\n");
	}
};

A a;

void __attribute__ ((constructor (101))) my_init1(void)
{
	printf("in my init1\n");
}

void __attribute__ ((destructor (101))) my_fini1(void)
{
	printf("in my fini1\n");
}

void __attribute__ ((constructor (102))) my_init2(void)
{
	printf("in my init2\n");
}

void __attribute__ ((destructor (102))) my_fini2(void)
{
	printf("in my fini2\n");
}

int main()
{
	printf("in main\n");
	atexit(at_exit);
	return 0;
}

