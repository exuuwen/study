#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

/*
To clean up the lock state, we can establish fork 
handlers by calling the function pthread_atfork.
*/

/*
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_atfork 
thread started...
parent about to fork...
preparing locks...
parent unlocking locks...
parent returned from fork
cr7@cr7-virtual-machine:~/test/APUE/threads$ child unlocking locks...
child returned from fork
*/
static pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void
prepare(void)
{
	printf("preparing locks...\n");
	pthread_mutex_lock(&lock1);
	pthread_mutex_lock(&lock2);
}
void
parent(void)
{
	printf("parent unlocking locks...\n");
	pthread_mutex_unlock(&lock1);
	pthread_mutex_unlock(&lock2);
}

void
child(void)
{
	printf("child unlocking locks...\n");
	pthread_mutex_unlock(&lock1);
	pthread_mutex_unlock(&lock2);
}

void *
thr_fn(void *arg)
{
	printf("thread started...\n");
	pause();
	return(0);
}

int
main(void)
{
	int         err;
	pid_t       pid;
	pthread_t   tid;

	if ((err = pthread_atfork(prepare, parent, child)) != 0)
	{
		perror("pthread_atfork");
		exit(1);
	}
	err = pthread_create(&tid, NULL, thr_fn, 0);
	if (err != 0)
	{
		perror("pthread_create pthread 1");
		exit(1);
	}
	sleep(2);
	printf("parent about to fork...\n");
	if ((pid = fork()) < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (pid == 0) /* child */
		printf("child returned from fork\n");
	else        /* parent */
		printf("parent returned from fork\n");

	exit(0);
}



