#ifndef _H_THREAD_POOL_H_
#define _H_THREAD_POOL_H_

#include <deque>
#include <vector>
#include <tr1/functional>

#include "Condition.h"
#include "MutexLock.h"
#include "Thread.h"
#include "Base.h"

using namespace std;
using namespace std::tr1;

class ThreadPool : Noncopyable
{
public:
	typedef function<void ()> Task;

	explicit ThreadPool(const string& name = string());
	~ThreadPool();

	void start(int num_threads);
	void stop();

	void run(const Task& f);
	
private:
	void runLoop();
	Task take();
	
	mutable Mutex mutex_;
	Condition cond_;
	string name_;
	vector<Thread*> threads_;
	deque<Task> queue_;
	bool running_;
};

#endif
