#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

/*
A thread must block the signals it is waiting for before calling sigwait. It will not run the sigaction
*/

/*
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_signal 

recieve SIGUSR1
^C
interrupt
^C
interrupt
^C
interrupt
^C
interrupt
^\
quit so we out
thread 1 exit code 0
*/
static int         quitflag;   /* set nonzero by thread */
static sigset_t    mask;


void *thr_fn(void *arg)
{
	int err, signo;

	while(1)
	{
		err = sigwait(&mask, &signo);
		if (err != 0)
		{
			perror("sigwait error");
			exit(1);
		}
		switch (signo) 
		{
		case SIGINT:
		    printf("\ninterrupt\n");
		    break;

		case SIGQUIT:
		    printf("\nquit so we out\n");
		    return (void*)0;

		case SIGUSR1:
		    printf("\nrecieve SIGUSR1\n");
		    break;

		default:
		    printf("unexpected signal %d\n", signo);
		    exit(1);
		}
   	}
}

int main(void)
{
	int         err;
	sigset_t    oldmask;
	static pthread_t   tid;
	void *tret;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGUSR1);
	if ((err = pthread_sigmask(SIG_BLOCK, &mask, &oldmask)) != 0)
	if (err != 0)
	{
		perror("SIG_BLOCK error");
		exit(1);
	}

	err = pthread_create(&tid, NULL, thr_fn, 0);
	if (err != 0)
	{
		perror("can't create thread ");
		exit(1);
	}

	sleep(1);
	err = pthread_kill(tid, 0); //check the pthread exits
	if (err != 0)
	{
		perror("thread do not exits");
		exit(1);
	}
	
	err = pthread_kill(tid, SIGUSR1);
	if (err != 0)
	{
		perror("send  SIGUSR1 err");
		exit(1);
	}

	err = pthread_join(tid, &tret);
	if (err != 0)
	{
		perror("can't join with thread 1");
		exit(1);
	}
	printf("thread 1 exit code %d\n", (int)tret);

	/* reset signal mask which unblocks SIGQUIT */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
	{
		perror("SIG_SETMASK error");
		exit(1);
	}
	exit(0);
}



