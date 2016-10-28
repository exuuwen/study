#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

int main()
{
	struct tm *tblock;
	time_t times;
	times = time(NULL);

	printf("time is %s", ctime(&times));

 	tblock = localtime(&times); 
    	printf("Local time is: %s",asctime(tblock)); 

	tblock = gmtime(&times); 
	printf("gm time is: %s",asctime(tblock));
	return 0;
}
