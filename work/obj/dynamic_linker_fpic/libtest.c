#include "libtest.h"

#include <stdio.h>

static int a = 1000;
extern int b;
int c = 100;

static int *m1 = &a;
static int *m2 = &c;

extern void out();

void bar()
{
	/*a = 1;
	b = 2;	
	c = 3;*/
	printf("in bar %d %d %d %d %d\n", a, b, c, *m2, *m1);
}

static void bar_static()
{
	printf("static bar %d %d %d %d %d\n", a, b, c, *m2, *m1);
}

void foo(int i)
{
	bar();
	bar_static();
	out();
}
