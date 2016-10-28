#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

/*
pthread_once_t make all the pthreads only run one time
*/
/*
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_once_t 
thread 2 enter 
once_run in thread 2
thread 2 end
thread 1 enter
thread 1 end
thread 1 exit code 0
main thread exit
*/
static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_t tid1, tid2;
static void once_run(void)
{
	if(pthread_equal(pthread_self(), tid1))
		printf("once_run in thread 1\n");
	else if(pthread_equal(pthread_self(), tid2))
		printf("once_run in thread 2\n");
	else
		printf("error once run\n");
}
void *pthread1(void *arg)
{
	sleep(1);
	printf("thread 1 enter\n");
	pthread_once(&once, once_run);
	printf("thread 1 end\n");

	return (void*)0;
}
void *pthread2(void *arg)
{
	printf("thread 2 enter \n");
	pthread_once(&once, once_run);
	printf("thread 2 end\n");

	return (void*)0;
}
int main(void)
{
	int err;
	void *tret;

	err = pthread_create(&tid1,NULL, pthread1, NULL);
	if (err != 0)
	{
		perror("can't create thread 1:");
		exit(1);
	}

	err = pthread_create(&tid2,NULL, pthread2, NULL);
	if (err != 0)
	{
		perror("can't create thread 1:");
		exit(1);
	}
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

	printf("main thread exit\n");

	return 0;
}

