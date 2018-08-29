#include "libtest.h"
#include <stdio.h>

int b = 10;
int c = 11;

void bar()
{
	printf("hahahah bar %d %d\n", b, c);
} 

void out()
{
	printf("hahahah out %d %d \n", b, c);
} 

int main()
{
	foo(1);
	return 0;
}
