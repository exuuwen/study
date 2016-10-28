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
The program set thread1 recieve user1, and thread2 recieve user2 from only of the three tid
if one thread mask a signal, it will be send to another thread
*/
/*
cr7@ubuntu:~/test/c++/ptr$ kill -n 12 17081
cr7@ubuntu:~/test/c++/ptr$ kill -n 12 17081
cr7@ubuntu:~/test/c++/ptr$ kill -n 12 17082
cr7@ubuntu:~/test/c++/ptr$ kill -n 12 17082
cr7@ubuntu:~/test/c++/ptr$ kill -n 12 17083
cr7@ubuntu:~/test/c++/ptr$ kill -n 12 17083
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17083
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17083
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17082
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17081
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17081
cr7@ubuntu:~/test/c++/ptr$ kill -n 10 17081
*/

/*
cr7@ubuntu:~/test/signal/pthread_signal$ ./recieve 
PID:17081
I am main process:3075630784.
I am thread 1:3075627840
I am thread 2:3067235136
SIG2:I am thread:3067235136
Thread 2  3067235136 wakeup
SIG2:I am thread:3067235136
SIG2:I am thread:3067235136
SIG2:I am thread:3067235136
SIG2:I am thread:3067235136
SIG2:I am thread:3067235136
SIG1:I am thread:3075627840
Thread 1  3075627840 wakeup
SIG1:I am thread:3075627840
SIG1:I am thread:3075627840
SIG1:I am thread:3075627840
SIG1:I am thread:3075627840
SIG1:I am thread:3075627840

*/

void sighandler(int sig) 
{
        printf("SIG1:I am thread:%lu\n", pthread_self());
}

void sighandler2(int sig) 
{
        printf("SIG2:I am thread:%lu\n", pthread_self());
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
                signal(SIGUSR1, sighandler);

		sigemptyset(&mask);
		if ( -1 == sigaddset(&mask, SIGUSR2))
		{
		        printf("add SIGINT to mask failed!\n");
		        return NULL;
		}
		ret = pthread_sigmask(SIG_BLOCK, &mask, NULL);  // thread mask
           	if (ret != 0)
		{
			printf("block SIGuser2 error\n");
			return NULL;
		}
	
	}
        else if(i == 2)
	{
                signal(SIGUSR2, sighandler2);
		
		if ( -1 == sigaddset(&mask, SIGUSR1))
		{
		        printf("add SIGINT to mask failed!\n");
		        return NULL;
		}
		ret = pthread_sigmask(SIG_BLOCK, &mask, NULL);
           	if (ret != 0)
		{
			printf("block SIGuser2 error\n");
			return NULL;
		}	
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

        i =1 ;
        ret = pthread_create(&tid1, NULL, mThread, &i);
	assert(ret == 0);

        sleep(1);
        i = 2;
        ret = pthread_create(&tid2, NULL, mThread, &i);
	assert(ret == 0);

     
        
        sigemptyset(&mask);
        if ( -1 == sigaddset(&mask, SIGUSR1))
	{
                printf("add SIGINT to mask failed!\n");
                return -1;
        }
	if ( -1 == sigaddset(&mask, SIGUSR2))
	{
                printf("add SIGINT to mask failed!\n");
                return -1;
        }
        if ( -1 == sigprocmask(SIG_BLOCK, &mask, NULL) ) // main mask
	{
                printf("sigprocmask failed!\n");
                return -1;
        }
        

        sleep(100);
        printf("Main process wake\n");
	while(1);
        sleep(3);
        return 0;
} 
