#include <iostream>
#include <stdio.h>
#include <assert.h>

#include "Thread.h"

using namespace std;

void thr_fn3()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << " main:" << CurrentThread::isMainThread() << endl;	
	sleep(10);
	printf("thread 3 exiting\n");
	//return((void *)3);
}

Thread *p3 = new Thread(bind(&thr_fn3), "thread3");
void thr_fn1(int data)
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << " main:" << CurrentThread::isMainThread() << endl;
	sleep(1);
	delete p3;	
	sleep(100);
	printf("thread 1 param %d\n", data);
	//return((void *)1);
}

int param = 10;
Thread *p1 = new Thread(bind(&thr_fn1, param), "thread1");

void thr_fn2()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << " main:" << CurrentThread::isMainThread() << endl;
	sleep(2);
	delete p1;	
	sleep(10);
	printf("thread 2 exiting\n");
	pthread_exit((void *)2);
}



Thread *p2 = new Thread(bind(&thr_fn2), "thread2");


int main()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << " main:" << CurrentThread::isMainThread() << endl;
	for(int i=0; i<=2; i++)
	{
		printf("%d max %d, min %d\n", i, sched_get_priority_max(i), sched_get_priority_min(i)); //get the priority of the policy
	}
	
	int ret = p1->start();
	if(ret < 0)
	{
		printf("p1 start error\n");
		assert(0);
	}
	ret = p2->start();
	if(ret < 0)
	{
		printf("p2 start error\n");
		assert(0);
	}
	ret = p3->start();
	if(ret < 0)
	{
		printf("p3 start error\n");
		assert(0);
	}

	
	Thread::waitAllThreadsTerminate();
	delete p2;

	return 0;
}
