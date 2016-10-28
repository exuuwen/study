#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "Thread.h"
#include "ConcurrentQueue.h"

using namespace std;

struct test
{
	int a;
};

ConcurrentQueue<struct test> bq;

void thr_fn1()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;

	struct test t;
	for(int i=1; i<5; i++)
	{
		t.a = i;
		bq.put(t);
	}

	sleep(3);
	for(int i=1; i<5; i++)
	{
		t.a = i + 10;
		bq.put(t);
		sleep(1);
	}
	
}

void thr_fn2()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;
	//sleep(5);
	
	struct test m;
	while(1)
	{
		m = bq.take();
		cout << "test.a:" << m.a << endl; 
	}
}

int main()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;
	Thread p1(bind(&thr_fn1), "thread1");
	Thread p2(bind(&thr_fn2), "thread2");

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

	
	Thread::waitAllThreadsTerminate();

	return 0;
	
}
