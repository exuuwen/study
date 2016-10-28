/* example.c*/
#include <stdio.h>
#include <pthread.h>
#include<stdlib.h>
#include<time.h>


/*
NO PTHREAD_CREATE_DETACHED && pthread_detach
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_detachstate
This is the main pthread.
This is a pthread.
this pthread is over
This is the main pthread end.
*/

/*
PTHREAD_CREATE_DETACHED
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_detachstate
This is the main pthread.
This is the main pthread end.
*/

/*
pthread_detach
cr7@cr7-virtual-machine:~/test/APUE/threads$ ./pthread_detachstate
This is the main pthread.
This is the main pthread end.
*/
void *thread(void *arg)
{
	int i;
	
	printf("This is a pthread.\n");
	//pthread_detach(pthread_self());
	sleep(3);
	printf("this pthread is over\n");

	return (void*)0;
}

int main(void)
{
	int i,ret;
	pthread_t id;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);/*PTHREAD_CREATE_JOINABLE is defaut*/

	ret = pthread_create(&id, &attr,thread, NULL);
	if(ret != 0)
	{
		perror("Create pthread error!");
		exit (1);
	}
	printf("This is the main pthread.\n");

	sleep(1);// make sure thread detach him self, detach must be actted before the join

	ret = pthread_join(id, NULL);
	printf("join ret:%d\n", ret);

	printf("This is the main pthread end.\n");

	return (0);
}
