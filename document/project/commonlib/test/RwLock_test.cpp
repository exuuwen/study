#include <pthread.h>
#include <assert.h>
#include <iostream>

#include "RwLock.h"

using namespace std;

static unsigned int count = 0;
static RwLock mutex;

void *Run1(void *arg)
{
	int param = *(int*)(arg);
	cout << "thread 1 param " << param << endl;
	sleep(1);
	for(int i=0; i<param; i++)
	{
		{
			ReadLock rlock(mutex);	
			cout << "thread 1 time " << i << " read count is " << count << endl;
			sleep(5);
		}
		sleep(1);
	}
	return((void *)1);
}

void *Run2(void *arg)
{
	int param = *(int*)(arg);
	cout << "thread 2 param " << param << endl;
	sleep(1);
	for(int i=0; i<param; i++)
	{
		{
			ReadLock rlock(mutex);	
			cout << "thread 2 time " << i << " read count is " << count << endl;
			sleep(5);
		}
		sleep(1);
	}
	return((void *)2);
}

void *Run3(void *arg)
{
	int param = *(int*)(arg);
	cout << "thread 3 param " << param << endl;
	for(int i=0; i<param; i++)
	{
		{
			WriteLock wlock(mutex);	
			cout << "get write lock" << endl;
			count ++;
			sleep(5);
		}
		sleep(1);
	}
	return((void *)3);
}

int main()
{
	int param1 = 5;
	int param2 = 5;
	int param3 = 5;
	
	int ret;
	pthread_t id1;
	pthread_t id2;
	pthread_t id3;

	ret = pthread_create(&id1, NULL, Run1, &param1);
	assert(ret == 0);
	
	ret = pthread_create(&id2, NULL, Run2, &param2);
	assert(ret == 0);

	ret = pthread_create(&id3, NULL, Run3, &param3);
	assert(ret == 0);

  	pthread_join(id1, NULL);
  	pthread_join(id2, NULL);
	pthread_join(id2, NULL);

	cout << "finnal count:" << count << endl;
	
	return 0;
	
}
