#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

/*
cr7@cr7-virtual-machine:~/sgsn$ ./three_msleep 5000
^Cin handler
usleep:the diff time is 1337720
^Cin handler
^Cin handler
nanosleep:the diff time is 5001016
^Cin handler
^Cin handler
^Cin handler
select:the diff time is 5000999
*/

/*
sleep && usleep will affect by signal
select && nansleep can get off the flaw;
*/

int msleep_nanosleep(unsigned int msecond)
{
	struct timespec rqtp, rmtp;
	int ret;
	struct timeval tv_start, tv_end; 

	rqtp.tv_sec = msecond / 1000;
	msecond %= 1000;
	rqtp.tv_nsec = 1000000 * msecond;

	while((ret = nanosleep(&rqtp, &rqtp)) == -1 && errno == EINTR)
		continue;
	
	return (ret == 0) ? 0 : -1;
}


int msleep_select(unsigned int msecond)
{
	struct timeval tv;
	int ret;
	
	tv.tv_sec = msecond / 1000;
	msecond %= 1000;
	tv.tv_usec = 1000 * msecond;

	while((ret = select (0, NULL, NULL, NULL, &tv)) == -1 && errno == EINTR)
		continue;

	return (ret == 0) ? 0 : -1;
}



int msleep_usleep(unsigned int msecond)
{
	int ret;
	
	ret = usleep(1000*msecond);
	
	return (ret == 0) ? 0 : -1;
}

void handler(int signum)
{
	printf("in handler\n");
}

int main(int argc, char *argv[])
{
	struct timeval tv_start, tv_end; 
	int ret, time;

	struct sigaction sigact;
	
	if (argc != 2)
	{
		printf("usage:%s n(ms)\n", argv[0]);
		return -1; 
	}

	time = atoi(argv[1]);

	sigact.sa_handler = handler;
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);

	gettimeofday(&tv_start, NULL);
	ret = msleep_usleep(time); 
	gettimeofday(&tv_end, NULL);
	
	printf("usleep:the diff time is %ld\n", (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec));

	gettimeofday(&tv_start, NULL);
	ret = msleep_nanosleep(time);
	gettimeofday(&tv_end, NULL);
	assert(ret == 0);
	
	printf("nanosleep:the diff time is %ld\n", (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec));

	gettimeofday(&tv_start, NULL);
	ret = msleep_select(time);
	gettimeofday(&tv_end, NULL);
	assert(ret == 0);
	
	printf("select:the diff time is %ld\n", (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec));
}

