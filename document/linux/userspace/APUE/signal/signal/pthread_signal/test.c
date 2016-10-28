#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

/*
all the thread share the same handler which is seted last time
*/
/*
cr7@ubuntu:~/test/signal/pthread_signal$ ./same_handle 
PID:17220
I am main process:3075651264.
I am thread 1:3075648320
I am thread 2:3067255616
SIG1:I am thread handler 2:3075651264
Main process wake
SIG1:I am thread handler 2:3075648320
Thread 1  3075648320 wakeup
SIG1:I am thread handler 2:3067255616
Thread 2  3067255616 wakeup
SIG1:I am thread handler 2:3067255616
SIG1:I am thread handler 2:3075648320
SIG1:I am thread handler 2:3075651264

*/

/*
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17220
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17221
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17222
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17222
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17221
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17220

*/
void sighandler1(int sig) 
{
        printf("SIG1:I am thread handler 1:%lu\n", pthread_self());
}

void sighandler2(int sig) 
{
        printf("SIG1:I am thread handler 2:%lu\n", pthread_self());
}

void sighandlerm(int sig) 
{
        printf("SIG1:I am thread handler main:%lu\n", pthread_self());
}


void *mThread(void *args) 
{
        int i = *(int*)args;
	int ret;
	sigset_t mask;

        pthread_detach(pthread_self());
        printf("I am thread %d:%lu\n", i, pthread_self());

        if(i == 1)
	{
                signal(SIGUSR1, sighandler1);
	
	}
        else if(i == 2)
	{
                signal(SIGUSR1, sighandler2);	// the last set
	}

        sleep(100);
        printf("Thread %d  %lu wakeup\n", i, pthread_self());
	while(1);
        return NULL;
}

int main(int argc, char **argv)
{
        pthread_t tid1, tid2;
        sigset_t mask;
        int i, ret;

        printf("PID:%d\n", getpid());

        printf("I am main process:%lu.\n", pthread_self());

	signal(SIGUSR1, sighandlerm);

        i =1 ;
        ret = pthread_create(&tid1, NULL, mThread, &i);
	assert(ret == 0);

        sleep(1);
        i = 2;
        ret = pthread_create(&tid2, NULL, mThread, &i);
	assert(ret == 0);
        

        sleep(100);
        printf("Main process wake\n");
	while(1);
        sleep(3);
        return 0;
} 
