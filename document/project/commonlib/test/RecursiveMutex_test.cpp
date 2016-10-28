#include <iostream>
#include <pthread.h>
#include <assert.h>

#include "MutexLock.h"

using namespace std;

Mutex mutex(PTHREAD_MUTEX_RECURSIVE);

int count = 0;

void * Run1(void * para)
{
	int i = 0;
	
	sleep(2);
	MutexLock count_lock(mutex);
	cout << "run1 get first lock" << endl;
	sleep(3);
	{
		MutexLock count_lock1(mutex);
		cout << "run1 get second lock" << endl;
		sleep(3);
	}
	                       
	return 0;
}

void * Run2(void * para)
{
	MutexLock count_lock(mutex);
	cout << "run2 get lock" << endl;
	sleep(5);
	return 0;        
}

int main()
{
	int ret;
	pthread_t id1;
	pthread_t id2;

	ret = pthread_create(&id1, NULL, Run1, NULL);
	assert(ret == 0);
	
	ret = pthread_create(&id2, NULL, Run2, NULL);
	assert(ret == 0);

  	pthread_join(id2, NULL);
  	pthread_join(id1, NULL);
	
	cout << "last value in main thread count:" << count << endl;  
	return 0;
}

