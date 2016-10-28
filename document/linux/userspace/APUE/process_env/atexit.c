#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

static void my_exit1(void)
{
   printf("first exit handler\n");
}

static void my_exit2(void)
{
   printf("second exit handler\n");
}

static void my_exit3(void)
{
   printf("third exit handler\n");
}


int main(void)
{
	int ret;
	
	ret = atexit(my_exit3);
	assert(ret == 0);

	ret = atexit(my_exit2);
	assert(ret == 0);

	ret = atexit(my_exit1);
	assert(ret == 0);

	printf("main is done\n");

	exit(0);
}





