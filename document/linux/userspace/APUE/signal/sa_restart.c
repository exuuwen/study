#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>

/*SA_RESTART: the signal intercept the syscall, 
the the system call will be restart.
*/
/*
case: sigact.sa_flags = SA_RESTART;
cr7@cr7-virtual-machine:~/APUE/signal$ ./sa_restart 
signled:0     //type contrl + d means NULL
^Cin handler
^Cin handler
d
*/

/*
case: sigact.sa_flags = 0;
cr7@cr7-virtual-machine:~/APUE/signal$ ./sa_restart 
signled:0
^Cin handler
signled:1
*/

static int signaled = 0;

void handler(int signum)
{
	printf("in handler\n");
	signaled = 1;
}

int main()
{
	char ch;
	struct sigaction sigact;
	
	sigact.sa_handler = handler;
	sigact.sa_flags = SA_RESTART;
	//sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);

	while(read(STDIN_FILENO, &ch, 1) != 1 && printf("signled:%d\n", signaled) && !signaled);

	return 0;
}
