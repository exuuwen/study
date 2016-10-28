
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>



int
main(void)
{
	char* name;
	name = getlogin();
	
	printf("name:%s\n", name);
	exit(0);
}
