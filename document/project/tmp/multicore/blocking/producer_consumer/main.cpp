#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <iostream>

#include "ProducerConsumer.h"

using namespace std;

struct test
{
        int a;
};

ProducerConsumer<struct test> pc(5);

void * Run1(void * para)
{
	int i = 0;
	while (i++ < 8)
	{
		struct test t;
		pc.deq(t);
		cout << "deq threadid 1. " << "test.a:" << t.a << endl;
	}

	return 0;
}

void * Run2(void * para)
{
	int i = 0;
        while (i++ < 12)
        {
                struct test t;
                pc.deq(t);
                cout << "deq threadid 2. " << "test.a:" << t.a << endl;
	}

        return 0;
}


void * Run3(void * para)
{
        int i = 0;
	sleep(2);
        while (i++ < 10)
        {
                struct test t;
		t.a = i;
                pc.enq(t);
                cout << "enq threadid 3. " << "test.a:" << t.a << endl;
        }

        return 0;
}



void * Run4(void * para)
{
        int i = 0;
	sleep(1);
        while (i++ < 10)
        {
                struct test t;
		t.a = i + 10;
                pc.enq(t);
                cout << "enq threadid 4. " << "test.a:" << t.a << endl;
        }
        
	return 0;
}


int main()
{
	int ret;
	pthread_t id1;
	pthread_t id2;
	pthread_t id3;
	pthread_t id4;

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
	
	ret = pthread_create(&id3, NULL, Run3, NULL);
	assert(ret == 0);

	cpu = 2;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	if (pthread_setaffinity_np(id3, sizeof(cpu_set_t), &mask))
        {
		printf("set affinity for thread 0x%lx to cpu:%d failed\n",id3, cpu);
		return -1;
        }

	ret = pthread_create(&id4, NULL, Run4, NULL);
	assert(ret == 0);
  	
	cpu = 3;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	if (pthread_setaffinity_np(id4, sizeof(cpu_set_t), &mask))
        {
		printf("set affinity for thread 0x%lx to cpu:%d failed\n",id4, cpu);
		return -1;
        }
	pthread_join(id1, NULL);
  	pthread_join(id2, NULL);
	pthread_join(id3, NULL);
  	pthread_join(id4, NULL);
	
	cout << "last value in main thread " << endl;  
	return 0;
}

