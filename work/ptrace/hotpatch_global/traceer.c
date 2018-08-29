#include <stddef.h>   
#include <unistd.h>
#include <stdint.h>   
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int opt = 100;

int main()
{   
	unsigned long i = 0;
   	
	while(1)
	{
		if(opt)
		{
			printf("My counter: %ld\n", i);
			sleep(1);
		}
		else
		{
			printf("You counter: %ld\n", i);
			sleep(1);
		}	

		i++;
	}

    return 0;
}
