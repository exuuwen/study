#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

/*
•	Makes a call to pthread_exit
•	Responds to a cancellation request
•	Makes a call to pthread_cleanup_pop with a nonzero execute argument
*/

/*
arg1 = 1;
arg2 = 1;
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_cleanup_push 
hread 3 start
thread 3 push complete
cleanup: thread 3 second handler
cleanup: thread 3 first handler
thread 1 start
thread 1 push complete
thread 1 exit code 1
thread 2 start
thread 2 push complete
cleanup: thread 2 second handler
cleanup: thread 2 first handler
thread 2 exit code 2
*/

/*
arg1 = 0;
arg2 = 1;
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_cleanup_push 
hread 3 start
thread 3 push complete
cleanup: thread 3 second handler
cleanup: thread 3 first handler
thread 1 start
thread 1 push complete
cleanup: thread 1 second handler
thread 1 exit code 1
thread 2 start
thread 2 push complete
cleanup: thread 2 second handler
cleanup: thread 2 first handler
thread 2 exit code 2

*/
void
cleanup(void *arg)
{
	printf("cleanup: %s\n", (char *)arg);
}

void *
thr_fn1(void *arg)
{
	printf("thread 1 start\n");
	pthread_cleanup_push(cleanup, "thread 1 first handler");
	pthread_cleanup_push(cleanup, "thread 1 second handler");
	printf("thread 1 push complete\n");
	if (arg)
		return((void *)1);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(0);  //if para is 0, no execute 
	return((void *)1);
}

void *
thr_fn2(void *arg)
{
	sleep(2);
	printf("thread 2 start\n");
	pthread_cleanup_push(cleanup, "thread 2 first handler");
	pthread_cleanup_push(cleanup, "thread 2 second handler");
	printf("thread 2 push complete\n");
	if (arg)
		pthread_exit((void *)2);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	pthread_exit((void *)2);
}

void *
thr_fn3(void *arg)
{
	printf("thread 3 start\n");
	pthread_cleanup_push(cleanup, "thread 3 first handler");
	pthread_cleanup_push(cleanup, "thread 3 second handler");
	printf("thread 3 push complete\n");
	pthread_cancel(pthread_self());
	if (arg)
		pthread_exit((void *)2);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	pthread_exit((void *)2);
}

int
main(void)
{
	int         err;
	pthread_t   tid1, tid2, tid3;
	void        *tret;

	err = pthread_create(&tid1, NULL, thr_fn1, (void *)0);
	if (err != 0)
	{
		perror("can't create thread 1");
		exit(1);
	}

	err = pthread_create(&tid2, NULL, thr_fn2, (void *)1);
	if (err != 0)
	{
		perror("can't create thread 2");
		exit(1);
	}

	err = pthread_create(&tid3, NULL, thr_fn3, (void *)1);
	if (err != 0)
	{
		perror("can't create thread 3");
		exit(1);
	}

	

	err = pthread_join(tid3, &tret);
	if (err != 0)
	{
		perror("can't join with thread 3");
		exit(1);
	}
	printf("thread 2 exit code %d\n", (int)tret);

	err = pthread_join(tid1, &tret);
	if (err != 0)
	{
		perror("can't join with thread 1");
		exit(1);
	}
	printf("thread 1 exit code %d\n", (int)tret);

	err = pthread_join(tid2, &tret);
	if (err != 0)
	{
		perror("can't join with thread 2");
		exit(1);
	}
	printf("thread 2 exit code %d\n", (int)tret);
	exit(0);
}



