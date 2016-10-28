#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include  <setjmp.h>
/*
There is a problem in calling longjmp, however. When a signal is caught, 
the signal-catching function is entered with the current signal 
automatically being added to the signal mask of the process. 
This prevents subsequent occurrences of that signal from 
interrupting the signal handler. Linux however, do not save 
and restore the signal mask.
so sigsetjump can do this
*/
/*
cr7@cr7-virtual-machine:~/APUE/signal$ ./sigsetjump 
in alarm handleer
read timeout 5s
in alarm handleer
read timeout 5s
in alarm handleer
read timeout 5s
in alarm handleer
read timeout 5s
in alarm handleer
read timeout 5s

cr7@cr7-virtual-machine:~/APUE/signal$ ./alarm_blockread 
lllll
lllll
*/

static jmp_buf    env_alrm;

static void sig_alrm(int signo)
{
	printf("in alarm handleer\n");	
	siglongjmp(env_alrm, 1);
}

int main(void)
{
	int     n;
	char    line[10];
	int ret;
	struct sigaction sigact;
	static int times = 2;

	sigact.sa_handler = sig_alrm;
	sigact.sa_flags = 0;
	sigaction(SIGALRM, &sigact, NULL);

	if (sigsetjmp(env_alrm, 1) != 0)
	{
		printf("read timeout 5s\n");
		//exit(1);	
	}
	
	ret = alarm(times);
	if ((n = read(STDIN_FILENO, line, 10)) < 0)
		perror("read error");
	alarm(0);
	

	write(STDOUT_FILENO, line, n);
	exit(0);
}
