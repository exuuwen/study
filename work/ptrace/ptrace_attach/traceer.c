#include <stddef.h>   
#include <unistd.h>
#include <stdint.h>   
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main()
{   
	int i;
   	
	sleep(10); 
	for(i=0; i<10; ++i) 
	{
		printf("My counter: %d\n", i);
		sleep(1);
	}

    return 0;
}
