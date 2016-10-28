#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
// use 'top' command then press 1 can see each cpu's running situation

static int num_cpus;
static double waste_time(long n)
{
    double res = 0;
    long i = 0;
    while(i <n * 200000) {
        i++;
        res += sqrt(i);
    }
    return res;
}

void *function1(void *argc)
{
	pthread_t  thread_id;
	int ret, j, core;
	cpu_set_t cpuset;

	thread_id = pthread_self();
	printf("porgran pid:%u, the function1 pthread id is %lu\n",getpid(), thread_id);

	core = 0%num_cpus;

	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);

	ret = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);

	/* Check the actual affinity mask assigned to the thread */

	ret = pthread_getaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);

	printf("thread_id:%lu set the cup:\n", thread_id);
	for (j = 0; j < CPU_SETSIZE; j++)
	if (CPU_ISSET(j, &cpuset))
	   printf(" CPU %d\n", j);

	printf ("the function0 pthread id is %lu, bind core %d result: %f\n", thread_id, core, waste_time (5000));
	
	return 0;
}

void *function2(void *argc)
{
	pthread_t  thread_id;
	int ret, j, core;
	cpu_set_t cpuset;

	thread_id = pthread_self();
	printf("porgran pid:%u, the function1 pthread id is %lu\n",getpid(), thread_id);
	sleep(5);

	core = 1%num_cpus;

	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);

	ret = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);

	/* Check the actual affinity mask assigned to the thread */

	ret = pthread_getaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);

	printf("thread_id:%lu set the cup:\n", thread_id);
	for (j = 0; j < CPU_SETSIZE; j++)
	if (CPU_ISSET(j, &cpuset))
	   printf(" CPU %d\n", j);

	printf ("the function1 pthread id is %lu, bind core %d result: %f\n", thread_id, core, waste_time (5000));
	
	return 0;
}


int main()
{
	pthread_t  thread_id[2];
	int ret, j, core;
	pthread_t main_thread_id;
	cpu_set_t cpuset;
	main_thread_id = pthread_self();
	
	num_cpus = sysconf(_SC_NPROCESSORS_CONF);
	assert(num_cpus > 0);

	printf("cpu num:%d\n", num_cpus);
	
	printf("program pid:%u, mian_thread_id:%lu\n", getpid(), main_thread_id);

	ret = pthread_create(thread_id, NULL, function1, NULL);
	assert(ret == 0);

	ret = pthread_join(thread_id[0], NULL);
	assert(ret == 0);

	ret = pthread_create(thread_id + 1, NULL, function2, NULL);
	assert(ret == 0);
	ret = pthread_join(thread_id[1], NULL);
	assert(ret == 0);

	core = 2%num_cpus;
	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);

	ret = pthread_setaffinity_np(main_thread_id, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);

	/* Check the actual affinity mask assigned to the thread */

	ret = pthread_getaffinity_np(main_thread_id, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);

	for (j = 0; j < CPU_SETSIZE; j++)
	if (CPU_ISSET(j, &cpuset))
	   printf(" CPU %d\n", j);

	printf ("the main pthread id is %lu, bind core %d result: %f\n", main_thread_id, core, waste_time (5000));

	
	return 0;
}

/*
gcc -o cpu_affinity_pthread cpu_affinity_pthread.c -lpthread -lm
cr7@ubuntu:~/test$ ./cpu_affinity_pthread 
cpu num:2
program pid:2235, mian_thread_id:140090564720416
porgran pid:2235, the function1 pthread id is 140090553964288
thread_id:140090553964288 set the cup:
 CPU 0
the function0 pthread id is 140090553964288, bind core 0 result: 21081851083600.558594
porgran pid:2235, the function1 pthread id is 140090553964288
thread_id:140090553964288 set the cup:
 CPU 1
the function1 pthread id is 140090553964288, bind core 1 result: 21081851083600.558594
 CPU 0
the main pthread id is 140090564720416, bind core 0 result: 21081851083600.558594

*/



