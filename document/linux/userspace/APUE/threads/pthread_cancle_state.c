#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
/*
no pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_cancle_state 
thread 1 enter
thread 1 exit code -1
main thread exit
*/

/*
pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_cancle_state 
thread 1 enter
thread 1 exit code 0
main thread exit
*/
void *thr_fn1(void *arg)
{
	printf("thread 1 enter\n");
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);/*PTHREAD_CANCEL_ENABLE is the defaut value*/
	sleep(3);
	return((void *)0);
}

int main(void)
{
	int         err;
	void        *tret;
	pthread_t   tid1, tid2, tid3;

	err = pthread_create(&tid1, NULL, thr_fn1, NULL);
	if (err != 0)
	{
		perror("can't create thread 1");
		exit(1);
	}
	sleep(1);

	pthread_cancel(tid1);

	err = pthread_join(tid1, &tret);
	if (err != 0)
	{
		perror("can't join with thread 1");
		exit(1);
	}
	printf("thread 1 exit code %d\n", (int)tret);

	printf("main thread exit\n");
	
	return 0;
}
  
