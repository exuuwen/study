#include <stdio.h>

int g_count = 10;
int test(void)
{
   printf("[%s %s]: g_count=%d\n",__FILE__, __FUNCTION__, g_count); 

   return 0;
}

