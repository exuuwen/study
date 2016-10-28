#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
/*
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_private_data 
thread 2 enter
thread 2 returns ccccc
thread 1 enter 
thread 1 returns ababs
thread 1 exit code 0
thread 2 exit code 0
main thread exit
*/

static pthread_key_t key;
static pthread_t tid1, tid2;


void *pthread1(void *arg)
{
	char a[10]="ababs";	
		
	printf("thread 1 enter \n");
	pthread_setspecific(key, (void *)a);
	sleep(1);
	printf("thread 1 returns %s\n", (char*)pthread_getspecific(key));
	sleep(2);

	return (void*)0;
}

void *pthread2(void *arg)
{
	printf("thread 2 enter\n");
	char a[10]="ccccc";			
	//printf("thread %d enter\n",tid);
	pthread_setspecific(key, (void *)a);
	printf("thread 2 returns %s\n", (char*)pthread_getspecific(key));
	sleep(4);
	
	return (void*)0;
}

int main(void)
{
	void *tret;
	int err;

	err = pthread_key_create(&key, NULL);
	if (err != 0)
	{
		perror("can't key create");
		exit(1);
	}

	err = pthread_create(&tid1,NULL, pthread1,NULL);
	if (err != 0)
	{
		perror("can't create thread 1:");
		exit(1);
	}

	err = pthread_create(&tid2,NULL, pthread2,NULL);
	if (err != 0)
	{
		perror("can't create thread 2:");
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

	printf("thread 2 exit code %d\n", (int)tret);

	pthread_key_delete(key);

	printf("main thread exit\n");
	
	return 0;
}
