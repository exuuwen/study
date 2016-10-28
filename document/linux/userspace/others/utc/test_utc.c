#include <time.h>
#include <stdio.h>
#include <sys/time.h>

#include "utc.h"

static char time_str[100];

int main()
{
	struct timespec time_now;
	struct timeval time_val;
	int ret;

	rt_create_utc();
	
	clock_gettime(CLOCK_REALTIME, &time_now);
	ret = rt_get_time(time_str, time_now.tv_sec);
	if (ret == 0)
		printf("utc time:%s\n", time_str);
	
	gettimeofday(&time_val, 0);
	ret = rt_get_time(time_str, time_val.tv_sec);
	if (ret == 0)
		printf("val utc time:%s\n", time_str);

	printf("set offset\n");
	rt_set_utc_offset(-(3600*8));
	sleep(2);

	clock_gettime(CLOCK_REALTIME, &time_now);
	ret = rt_get_time(time_str, time_now.tv_sec);
	if (ret == 0)
		printf("utc time:%s\n", time_str);

	gettimeofday(&time_val, 0);
	ret = rt_get_time(time_str, time_val.tv_sec);
	if (ret == 0)
		printf("val utc time:%s\n", time_str);

	rt_remove_utc();

	return 0;
}
/*
gcc -o utc utc.c test_utc.c -lrt
cr7@ubuntu:~/test/modules/utc/userspace$ ./utc 
utc time:2013-08-08 08:00:44
val utc time:2013-08-08 08:00:44
set offset
utc time:2013-08-08 16:00:46
val utc time:2013-08-08 16:00:46
*/
