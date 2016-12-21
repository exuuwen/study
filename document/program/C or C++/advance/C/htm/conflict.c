#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "htm.h"

unsigned long conflict[2], retry[2], capacity[2], debug[2], others[2], nested[2], explict[2], fail[2];
//#define DEBUG


unsigned long count = 0;

void do_inc(int id)
{
	int i;
	unsigned stat;

	//for (i=0; i<8; i++)
	{
		stat = _xbegin();
		if(stat == _XBEGIN_STARTED){
            		count++;
			_xend();
        	}
		else
		{
#ifdef DEBUG
			if (stat & _XABORT_CONFLICT){
				conflict[id] ++;
			}
			if (stat & _XABORT_CAPACITY){
				capacity[id] ++;
			}
			if (stat & _XABORT_DEBUG){
				debug[id] ++;
			}
			if (stat & _XABORT_RETRY){
				retry[id] ++;
			}
			if (stat & _XABORT_NESTED){
				nested[id] ++;
			}
			if (stat & _XABORT_EXPLICIT){
				explict[id] ++;
				i++;
			}
			if (stat == 0){
				others[id] ++;
			}
#endif
			__sync_add_and_fetch(&count, 1);
		}
	}

	return ;
}

static int times = 50000000;
void * Run1(void * para)
{
	int i = 0;

	while(i < times)
	{
		i++;
		//__sync_add_and_fetch(&count, 1);
		do_inc(0);
	}                        

	return 0;
}

void * Run2(void * para)
{
	int i = 0;
	while(i < times)
	{
		i++;
		//__sync_add_and_fetch(&count, 1);
		do_inc(1);
	}             
	return 0;        
}


int main()
{
	int ret;
	pthread_t id1;
	pthread_t id2;
	int i;

	ret = pthread_create(&id1, NULL, Run1, NULL);
	assert(ret == 0);

	cpu_set_t mask;
	int cpu = 0;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	if (pthread_setaffinity_np(id1, sizeof(cpu_set_t), &mask))
        {
		printf("set affinity for thread 0x%lx to cpu:%d failed\n",id1, cpu);
		return -1;
        }
	
	ret = pthread_create(&id2, NULL, Run2, NULL);
	assert(ret == 0);

	cpu = 1;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	if (pthread_setaffinity_np(id2, sizeof(cpu_set_t), &mask))
        {
		printf("set affinity for thread 0x%lx to cpu:%d failed\n",id2, cpu);
		return -1;
        }

	pthread_join(id1, NULL);
	pthread_join(id2, NULL);
	
#ifdef DEBUG
	for (i=0; i<2; i++)
		printf("coflict:%lu,retry:%lu,capacity:%lu,debug:%lu,other:%lu,nested:%lu,explict:%lu----fail:%lu\n", conflict[i], retry[i], capacity[i], debug[i], others[i], nested[i], explict[i], fail[i]);
#endif

	printf("last value in main thread count:%lu\n", count);  
	return 0;
}

