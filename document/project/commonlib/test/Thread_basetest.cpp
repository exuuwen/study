#include <iostream>
#include <stdio.h>
#include <assert.h>

#include "Thread.h"

using namespace std;

void thr_fn1(int data)
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << " main:" << CurrentThread::isMainThread() << endl;
	sleep(3);
	printf("thread 1 param %d\n", data);
	//return((void *)1);
}

void thr_fn2()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << " main:" << CurrentThread::isMainThread() << endl;
	sleep(2);
	printf("thread 2 exiting\n");
	pthread_exit((void *)2);
}

void thr_fn3()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << " main:" << CurrentThread::isMainThread() << endl;
	sleep(2);
	printf("thread 3 exiting\n");
	//return((void *)3);
}

int main()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << " main:" << CurrentThread::isMainThread() << endl;
	for(int i=0; i<=2; i++)
	{
		printf("%d max %d, min %d\n", i, sched_get_priority_max(i), sched_get_priority_min(i)); //get the priority of the policy
	}
	int param = 10;
	Thread p1(bind(&thr_fn1, param), "thread1");
	Thread p2(bind(&thr_fn2), "thread2");
	Thread p3(bind(&thr_fn3), "thread3");
	p2.setThreadAttr(false, true);
	int ret = p1.start();
	if(ret < 0)
	{
		printf("p1 start error\n");
		assert(0);
	}
	ret = p2.start();
	if(ret < 0)
	{
		printf("p2 start error\n");
		assert(0);
	}
	ret = p3.start();
	if(ret < 0)
	{
		printf("p3 start error\n");
		assert(0);
	}
	ret = p3.start();
	if(ret < 0)
	{
		assert(0);
	}
	printf("p3 already start\n");
	sleep(1);

	ret = p1.setAffinity(0);
	if(ret < 0)
	{
		printf("p1 SetAffinity error\n");
		assert(0);
	} 
	ret = p2.setAffinity(1);
	if(ret < 0)
	{
		printf("p2 SetAffinity error\n");
		assert(0);
	} 
	ret = p3.setAffinity(2);
	if(ret < 0)
	{
		printf("p3 SetAffinity error\n");
		assert(0);
	}   
	ret = p2.terminate();
	if(ret < 0)
	{
		printf("p2 terminate error\n");
		assert(0);
	}
	
	ret = p1.setPriority(1, 99);  //should root
	if(ret < 0)
	{
		printf("p1 terminate error\n");
		assert(0);
	}
	int policy, level;
	ret = p3.getPriority(&policy, &level);
	if(ret < 0)
	{
		printf("p3 GetPriority error\n");
		assert(0);
	}
	printf("p3 policy %d, level %d\n", policy, level);

	ret = p1.getPriority(&policy, &level);
	if(ret < 0)
	{
		printf("p1 GetPriority error\n");
		assert(0);
	}
	printf("p1 policy %d, level %d\n", policy, level);

	ret = p1.detach();
	if(ret < 0)
	{
		printf("p1 detach error\n");
		assert(0);
	}
	printf("p 0x%lx tid %d, detache %d, syscope %d, state %d\n", p1.threadID(), p1.tid(), p1.getDetachState(), p1.getContentScope(), p1.getThreadState());
	printf("p 0x%lx tid %d, detache %d, syscope %d, state %d\n", p2.threadID(), p2.tid(), p2.getDetachState(), p2.getContentScope(), p2.getThreadState());
	printf("p 0x%lx tid %d, detache %d, syscope %d, state %d\n", p3.threadID(), p3.tid(), p3.getDetachState(), p3.getContentScope(), p3.getThreadState());

	Thread::waitAllThreadsTerminate();
	ret = p3.terminate();
	if(ret < 0)
	{
		assert(0);
	}
	
	printf("p3 already terminal\n");

	ret = p1.detach();
	if(ret < 0)
	{
		printf("p1 detach error\n");
		assert(0);
	}
	printf("p1 is not terminal and already detach\n");

	ret = p2.detach();
	if(ret == 0)
	{
		printf("p2 detach error\n");
		assert(0);
	}
	printf("p2 already terminal and can not detach\n");

	ret = p1.terminate();
	if(ret < 0)
	{
		printf("p1 terminate error\n");
		assert(0);
	}

	printf("p 0x%lx tid %d, detache %d, syscope %d, state %d\n", p1.threadID(), p1.tid(), p1.getDetachState(), p1.getContentScope(), p1.getThreadState());
	printf("p 0x%lx tid %d, detache %d, syscope %d, state %d\n", p2.threadID(), p2.tid(), p2.getDetachState(), p2.getContentScope(), p2.getThreadState());
	printf("p 0x%lx tid %d, detache %d, syscope %d, state %d\n", p3.threadID(), p3.tid(), p3.getDetachState(), p3.getContentScope(), p3.getThreadState());

	return 0;
}
