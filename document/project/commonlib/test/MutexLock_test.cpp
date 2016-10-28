#include <iostream>
#include <pthread.h>
#include <assert.h>

#include "MutexLock.h"

using namespace std;

Mutex mutex;

int count = 0;

void * Run1(void * para)
{
	int i = 0;
	while(i < 10000000)
	{
		i++;
		{
			MutexLock count_lock(mutex);
			count++; 
		}
	}                        
	cout << "last value in 1 thread count:" << count << endl;
	return 0;
}

void * Run2(void * para)
{
	int i = 0;
	while(i < 10000000)
	{
		i++;
		{
			MutexLock count_lock(mutex);
			count += 5;
		}
	}             
	cout << "last value in 1 thread count:" << count << endl;  
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

  	pthread_join(id1, NULL);
  	pthread_join(id2, NULL);
	
	cout << "last value in main thread count:" << count << endl;  
	return 0;
}

