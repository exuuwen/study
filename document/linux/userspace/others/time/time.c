#include <bits/time.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
void main()
{
	struct timeval tv; 
	//tv = kmalloc(sizeof(struct timeval), GFP_KERNEL); 
	gettimeofday(&tv, 0);
	printf("the second now is %ld\n",tv.tv_sec);
	printf("the micro second is %ld\n",tv.tv_usec); 
	printf("%ld days from 19700101 by far\n",tv.tv_sec/(3600*24));
	printf("%ld years from 1970 by far\n",tv.tv_sec/(3600*24*365));
}
