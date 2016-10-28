#include <math.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
// use 'top' command then press 1 can see each cpu's running situation
double waste_time(long n)
{
    double res = 0;
    long i = 0;
    while(i <n * 200000) {
        i++;
        res += sqrt(i);
    }
    return res;
}

int main()
{
	cpu_set_t cpuset;
	int ret, j, num_cpus, core;
	pid_t pid;
	
	pid = getpid();
	num_cpus = sysconf(_SC_NPROCESSORS_CONF);
	assert(num_cpus > 0);

	printf("cpu num:%d\n", num_cpus);
	core = 0%num_cpus;

	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);

	ret = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);
	
	ret = sched_getaffinity(pid, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);

        for (j = 0; j < CPU_SETSIZE; j++)
        	if (CPU_ISSET(j, &cpuset))
        		printf("after set program bind on CPU %d\n", j);
	
	printf ("core %d result: %f\n", core, waste_time (5000));	

	core = 1%num_cpus;
	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);

	ret = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);
	
	ret = sched_getaffinity(pid, sizeof(cpu_set_t), &cpuset);
	assert(ret == 0);

        for (j = 0; j < CPU_SETSIZE; j++)
        	if (CPU_ISSET(j, &cpuset))
        		printf("after set program bind on CPU %d\n", j);
	
	printf ("core %d result: %f\n", core, waste_time (5000));
	
	return 0;
}
/*
cr7@ubuntu:~/test$ gcc -o cpu_affinity_process cpu_affinity_process.c -lm
cr7@ubuntu:~/test$ ./cpu_affinity_process 
cpu num:2
after set program bind on CPU 0
core 0 result: 21081851083600.558594
after set program bind on CPU 1
core 1 result: 21081851083600.558594
*/



