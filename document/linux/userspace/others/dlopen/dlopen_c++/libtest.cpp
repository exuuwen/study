#include <stdio.h>

int g_count = 10;
namespace A
{
	extern "C" int count = 100;
	extern "C" int test(void)
	{
   		printf("[%s %s]: g_count=%d count=%d\n",__FILE__, __FUNCTION__, g_count, count); 
		return 0;
	}
}

