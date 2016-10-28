#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#include "MutexLock.h"
#include "Condition.h"

using namespace std;
Mutex mutex;
static Condition cond(mutex);
static Condition cond_all(mutex);
static int count;

void *Run1(void *arg)
{
	{
		MutexLock test(mutex);
		while(count != 1)
		{
			cout << "thread 1 wait cond" << endl;
			cond.wait();
		}
		count--;
	}
	cout << "thread 1 wake from cond" << endl;
	
	{	
		MutexLock test(mutex);
		while(count != 1)
		{
			cout << "thread 1 wait cond_all" << endl;
			cond_all.wait();
		}
	}
	cout << "thread 1 wake from cond_all" << endl;
	return((void *)1);
}

void *Run2(void *arg)
{	
	int ret;
	sleep(1);
	cout << "thread 2 wait cond_all 2000ms" << endl;
	{	
		MutexLock test(mutex);
		ret = cond_all.waitTimeout(2000);
	}

	if (ret != 0)
	{
		cout << "thread 2 wait cond_all 2000ms timeout" << endl;
	}
	else
	{
		cout << "thread 2 wake from cond_all 2000ms" << endl;
	}

	cout << "thread 2 wait cond_all"<< endl;
	{	
		MutexLock test(mutex);
		cond_all.wait();
	}

	cout << "thread 2 wake from cond_all" << endl;

	return((void *)2);
}

void *Run3(void *arg)
{
	sleep(1);
	cout << "send cond signal, count not ok" << endl;// wake it but thr1 will continue wait
	
	{	
		MutexLock test(mutex); 
		cond.signal();
	}

	sleep(2);
	
	cout << "send cond signal again, count ok" << endl;
	{	
		MutexLock test(mutex);
		count++;
		cond.signal();
	}
	
	sleep(1);
	cout << "send broadcast cond_all signal" << endl;
	{	
		MutexLock test(mutex);
		count++;
		cond_all.broadcast();
	}

	return((void *)3);
}

int main()
{	
	int ret;
	pthread_t id1;
	pthread_t id2;
	pthread_t id3;

	ret = pthread_create(&id1, NULL, Run1, NULL);
	assert(ret == 0);
	
	ret = pthread_create(&id2, NULL, Run2, NULL);
	assert(ret == 0);

	ret = pthread_create(&id3, NULL, Run3, NULL);
	assert(ret == 0);

  	pthread_join(id1, NULL);
  	pthread_join(id2, NULL);
	pthread_join(id2, NULL);	

	return 0;
	
}
