#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

int     glob = 6;       /* external variable in initialized data */

int
main(void)
{
	int     var;        /* automatic variable on the stack */
	pid_t   pid;

	var = 88;
	printf("before vfork\n");   /* we don't flush stdio */
	if ((pid = vfork()) < 0)
	{
		perror("vfork error");
		exit(1);
	}
	else if (pid == 0)
	{      /* child */
		sleep(2);
		glob++;                 /* modify parent's variables */
		var++;
		_exit(0);               /* child terminates */
	}
	else
	{
		printf("pid = %d, glob = %d, var = %d\n", getpid(), glob, var);
	}
	
	exit(0);
}



