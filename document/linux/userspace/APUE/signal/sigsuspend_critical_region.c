#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include  <setjmp.h>

/*
cr7@cr7-virtual-machine:~/APUE/signal$ ./sigsuspend_critical_region 
in to the critical region
^Cout of the critical region
in sig_int
cr7@cr7-virtual-machine:~/APUE/signal$ ./sigsuspend_critical_region 
in to the critical region
out of the critical region
^\^Cin sig_int
Quit
*/
static void sig_int(int);

int main(void)
{
	sigset_t    newmask, oldmask, waitmask;

	if (signal(SIGINT, sig_int) == SIG_ERR)
	{
		perror("signal(SIGINT) error");
		exit(1);	
	}	

	sigemptyset(&waitmask);
	sigaddset(&waitmask, SIGQUIT);

	sigemptyset(&newmask);
	sigaddset(&newmask, SIGINT);

	/*
	* Block SIGINT and save current signal mask.
	*/
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
	{
		perror("SIG_BLOCK error");
		exit(1);	
	}
	
	/* critical region of code */
	printf("in to the critical region\n");
	sleep(5);
	printf("out of the critical region\n");

	/*
	* Pause, allowing all signals except SIGUSR1.
	*/
	if (sigsuspend(&waitmask) != -1)
	{
		perror("sigsuspend error");
		exit(1);	
	}

	/*
	* Reset signal mask which unblocks SIGINT.
	*/
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
	{
		perror("SIG_SETMASK error");
		exit(1);	
	}

	exit(0);
}

static void sig_int(int signo)
{
	printf("in sig_int\n");
}



