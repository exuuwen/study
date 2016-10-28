#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "ThreadPool.h"

using namespace std;

static void waste_time(long n)
{
	double res = 0;
	long i = 0;

	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;
	while(i <n * 200000) 
	{
		i++;
		res += sqrt(i);
	}
	//sleep(100);

}

int main()
{
	ThreadPool pool("MainThreadPool");
	pool.start(4);
	
	int time = 5000;
	sleep(1);
	for (int i=0; i<4; ++i)
	{
		pool.run(bind(&waste_time, time));
	}

	sleep(5);
	
	pool.stop();
	
	
	printf("wait\n");
	Thread::waitAllThreadsTerminate();
	printf("ok\n");
}
