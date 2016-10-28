#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>        /* Definition of uint64_t */
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)


void  handle(union sigval v)
{
	time_t t;
	char p[32];
	time(&t);
	
	strftime(p, sizeof(p), "%T", localtime(&t));
	printf("%s thread 0x%x, val = %d, signal captured.\n", p, pthread_self(), v.sival_int);//must print it
	return;
}
 
int main(int argc, char *argv[])
{
	struct sigevent evp;
	struct itimerspec new_value;
	struct timespec now;
	timer_t timer;
	int ret;

	if (argc != 3) 
	{
		fprintf(stderr, "%s init-secs interval-secs  ##interval-secs!=0\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (clock_gettime(CLOCK_REALTIME, &now) == -1)
		handle_error("clock_gettime");

	/* Create a CLOCK_REALTIME absolute timer with initial
	expiration and interval as specified in command line */

	new_value.it_value.tv_sec = now.tv_sec + atoi(argv[1]);
	new_value.it_value.tv_nsec = now.tv_nsec;

	if(atoi(argv[2]) == 0)
	{
		fprintf(stderr, "%s init-secs interval-secs  ##interval-secs!=0\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	new_value.it_interval.tv_sec = atoi(argv[2]);
	new_value.it_interval.tv_nsec = 0;

	memset((void*)&evp, 0, sizeof(evp));
	evp.sigev_value.sival_ptr = &timer;
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = handle;
	evp.sigev_value.sival_int = 3;   //作为handle()的参数

	ret = timer_create(CLOCK_REALTIME, &evp, &timer);
	if(ret)
	{
		perror("timer_create");
		return -1;	
	}

	
	ret = timer_settime(timer, TIMER_ABSTIME, &new_value, NULL);
	if(ret)
	{
		perror("timer_settime");
		return -1;
	}	

	time_t t;
	char p[32];
	time(&t);
	
	strftime(p, sizeof(p), "%T", localtime(&t));
	printf("%s thread 0x%x\n", p, pthread_self());

	while(1);
}
