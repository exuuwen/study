#include <iostream>
#include <assert.h>
#include <stdio.h>

#include "ThreadLocal.h"
#include "Thread.h"

using namespace std;

class Test
{
public:
	Test()
	{
		cout << "Test constructure tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;
	}
	~Test()
	{
		cout << "Test destructure tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;
	}

	const string& name() const { return name_; }
  	void set_name(const string& n) { name_ = n; }

private:
	string name_;
};

ThreadLocal<Test>  obj;

void thr_fn()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;
	sleep(1);
	cout << "thread first read name:" << obj.value().name() << endl;
	obj.value().set_name("thread one");
	cout << "thread set read name:" << obj.value().name() << endl;

	sleep(2);
	return ;
}

int main()
{
	Thread p(bind(&thr_fn), "thread1");
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name()  << endl;

	int ret = p.start();
	if(ret < 0)
	{
		printf("p start error\n");
		assert(0);
	}	
	
	obj.value().set_name("main one");
	cout << "main set instance name:" << obj.value().name() << endl;
	
	
	ret = p.join();
	if(ret < 0)
	{
		perror("p join error:");
		assert(0);
	}

	cout << "finnally main name:" <<obj.value().name() << endl;

	return 0;
}
