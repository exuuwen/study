#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "Thread.h"
#include "RingQueue.h"

using namespace std;

struct test
{
	int a;
};

RingQueue<struct test> rq(5);

void thr_fn1()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;

	struct test t;
	bool ret;
	for(int i=1; i<7; i++)
	{
		t.a = i;
		ret = rq.push(t);
		if(ret)
		{
			cout << "push t:" << t.a << " success" << endl;
		}
	}
	cout << "fun1 len:" << rq.len() << endl;
	sleep(3);
	for(int i=1; i<7; i++)
	{
		t.a = i + 10;
		ret = rq.push(t);
		if(ret)
		{
			cout << "push t:" << t.a << " success" << endl;
		}
		cout << "fun1 len:" << rq.len() << endl;
		sleep(1);
	}
}

void thr_fn2()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;
	sleep(5);
	
	struct test m;
	bool ret;
	cout << "fun2 len:" << rq.len() << endl;
	while(1)
	{
		ret = rq.pop(m);
		if(ret)
		{
			cout << "fun2 len:" << rq.len() << endl;
			cout << "test.a:" << m.a << endl; 
		}
		//sleep(1);
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
