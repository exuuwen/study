#include <time.h>
#include <stdio.h>
#include <assert.h>

#define SEC        1
#define MILLISEC   1000         /* Numb. of millisecs. in a second. */
#define MICROSEC   1000000      /* Numb  of microsecs. in a second */
#define NANOSEC    1000000000   /* Numb. of nanosecs.  in a second */

long diff_time(struct timespec *tnew, struct timespec *told)
{
	long sec;
	long nsec;
	long msec;
	printf("new se:%ld; old se:%ld\n", tnew->tv_sec, told->tv_sec);
	if ((tnew->tv_sec < told->tv_sec) || ((tnew->tv_sec == told->tv_sec) && (tnew->tv_nsec < told->tv_nsec))) 
	{
		msec = -1;
	} 
	else 
	{
		sec = tnew->tv_sec - told->tv_sec;
		nsec =  tnew->tv_nsec - told->tv_nsec;
		msec = (sec * MILLISEC) + (nsec / MICROSEC);
	}

	return msec;
}
int get_current_time(struct timespec *tp)
{
	
	if (clock_gettime(CLOCK_REALTIME /*CLOCK_MONOTONIC*/, tp) != 0)
	{
		perror("clock_gettime:");
		return -1;
	}

	return 0;
}

int main()
{
	struct timespec new;
	struct timespec old;
	int ret;

	ret = get_current_time(&old);
	assert(ret == 0);
	sleep(2);
	ret = get_current_time(&new);
	assert(ret == 0);

	long time = diff_time(&new, &old);
	assert(ret != -1);
	printf("elapse time %ld\n", time);

	return 0;
}
/*
1. gcc compiler with  opt -lrt 
2. CLOCK_REALTIME: always start form 1970.01.01
3. CLOCK_MONOTONIC: the time start from today. If you want to compute the elapsed time between two events observed on the one machine without an intervening reboot, CLOCK_MONOTONIC is the best option
*/