#include "lib.h"

static int a = 100;
static void hah()
{
	a = 300;
}

int foo(int i)
{
	hah();
	return a + i;
}
