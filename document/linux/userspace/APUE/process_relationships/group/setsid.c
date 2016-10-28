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
	
	ret = setpgid(getpid(), 22270);
	assert(ret == 0);
	ret = setsid();
	assert(ret != -1);

	while(1)
	{
		printf("pid %d process in the %d gp, %d session\n",getpid(), getpgrp(), getsid(0));
		sleep(3);
	}
	
	return 0;
}

