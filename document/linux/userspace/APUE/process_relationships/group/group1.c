#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>



int main(void)
{
	pid_t     pid;
	int ret;

	if ((pid = fork()) < 0) 
	{
		perror("fork error");
		exit(1);
	} 
	else if (pid == 0)  // child 
	{     			
		//ret = setpgid(pid, pid);
		//assert(ret == 0);
	}
	else 
	{
		sleep(2);               // parent 
	}

	while(1)
	{
		printf("pid %d process in the %d gp, %d session\n",getpid(), getpgrp(), getsid(0));
		sleep(3);
	}

	return 0;
}

