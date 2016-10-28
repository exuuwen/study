#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>

/*
cr7@cr7-virtual-machine:~/test/APUE/system_data$ ./test 1 2 3 4 -2 -1 0
No jump, value:1
No jump, value:2
No jump, value:3
No jump, value:4
jump from:-2
jump from:-1
No jump, value:0
*/

static jmp_buf jmpbuffer;


void jump_func(int jump)
{
	if(jump < 0)
		longjmp(jmpbuffer, jump);/* return jump*/

	printf("No jump, value:%d\n", jump); 
	/* rest of processing for this command */
}


int main(int argc, char *argv[])
{
	int ret;
	int jump;
	int i = 1;

	if(argc < 2)
	{
		printf("usage: ./jmp data ...\n");/*data < 0 jump*/
		exit(1);
	}

	ret = setjmp(jmpbuffer);
	if(ret != 0)
		printf("jump from:%d\n", ret);
	printf("i:%d\n", i);
	while(i<argc)
	{
		jump = atoi(argv[i]);
		i++;
		jump_func(jump);	
	}
	
	exit(0);
}







