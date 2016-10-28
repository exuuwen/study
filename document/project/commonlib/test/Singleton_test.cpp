#include <iostream>
#include <assert.h>
#include <stdio.h>

#include "Singleton.h"
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
  std::string name_;
};

void thr_fn()
{
	cout << "tid:" << CurrentThread::tid() <<" name:" << CurrentThread::name() << endl;
	sleep(1);
	cout << "thread first read instance name:" << Singleton<Test>::instance().name() << endl;
	Singleton<Test>::instance().set_name("only one in p");
	cout << "thread set instance name:" << Singleton<Test>::instance().name() << endl;

	sleep(2);
	return;
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
	
	Singleton<Test>::instance().set_name("only one");
	cout << "main set instance name:" << Singleton<Test>::instance().name() << endl;
	
	ret = p.join();
	if(ret < 0)
	{
		printf("p join error\n");
		assert(0);
	}

	cout << "finnally instance name:" << Singleton<Test>::instance().name() << endl;

	return 0;
}
