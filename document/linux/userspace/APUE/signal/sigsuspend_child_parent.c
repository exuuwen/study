#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include  <setjmp.h>

/*
1: no TELL_WAIT TELL_PARENT TELL_CHILD
cr7@cr7-virtual-machine:~/APUE/signal$ ./sigsuspend_child_parent 
outputou ftropm upatre nt
from child
2: avoid race condition
cr7@cr7-virtual-machine:~/APUE/signal$ gcc -o sigsuspend_child_parent sigsuspend_child_parent.c
cr7@cr7-virtual-machine:~/APUE/signal$ ./sigsuspend_child_parent 
output from parent
output from child

*/
static volatile sig_atomic_t sigflag; /* set nonzero by sig handler */
static sigset_t newmask, oldmask, zeromask;

static void
sig_usr(int signo)   /* one signal handler for SIGUSR1 and SIGUSR2 */
{
    sigflag = 1;
}

void
TELL_WAIT(void)
{
	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
	{
		perror("signal(SIGUSR1) error");
		exit(1);
	}
	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
	{
		perror("signal(SIGUSR2) error");
		exit(1);
	}
	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);

	/*
	* Block SIGUSR1 and SIGUSR2, and save current signal mask.
	*/
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
	{
		perror("SIG_BLOCK error");
		exit(1);
	}
}

void
TELL_PARENT(pid_t pid)
{
	kill(pid, SIGUSR2);              /* tell parent we're done */
}

void
WAIT_PARENT(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);   /* and wait for parent */
	sigflag = 0;

	/*
	* Reset signal mask to original value.
	*/
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
	{
		perror("wait parent SIG_SETMASK error");
		exit(1);
	}
}

void
TELL_CHILD(pid_t pid)
{
	kill(pid, SIGUSR1);             /* tell child we're done */
}

void
WAIT_CHILD(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);  /* and wait for child */
	sigflag = 0;

	/*
	* Reset signal mask to original value.
	*/
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
	{
		perror("wait child SIG_SETMASK error");
		exit(1);
	}
}

static void charatatime(char *);

int main(void)
{
	pid_t   pid;
	
	TELL_WAIT();

	if ((pid = fork()) < 0) 
	{
		perror("fork error");
		exit(1);
	} 
	else if (pid == 0) 
	{
		WAIT_PARENT();      /* parent goes first */
		charatatime("output from child\n");
	} 
	else 
	{
		charatatime("output from parent\n");
		TELL_CHILD(pid);

	}
	exit(0);
}

static void charatatime(char *str)
{
	char    *ptr;
	int     c;

	setbuf(stdout, NULL);           /* set unbuffered */
	for (ptr = str; (c = *ptr++) != 0; )
		putc(c, stdout);
}






