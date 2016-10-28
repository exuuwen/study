#include <stdio.h>
/*gcc -o test test.c  -DSTEST=\"hello\ world\" -DNUM=2*/

void function()
{
	printf("TEST:%s, NUM:%d\n", STEST, NUM);	
}

int main()
{
	function();
	return 0;
}

