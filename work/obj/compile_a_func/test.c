#include <stdio.h>
#define  PRCESSION
int main()
{
#ifdef  PRCESSION
	printf("hello world\n");
#else 
	printf("test procession\n");
#endif
	return 0;

}
