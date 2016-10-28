#include <assert.h>
#include <stdio.h>

#include "ThreadPool.h"

using namespace std;

ThreadPool::ThreadPool(const string& name)
  : mutex_()
  , cond_(mutex_)
  , name_(name)
  , running_(false)
{
}

ThreadPool::~ThreadPool()
{
	if (running_)
	{
		stop();
	}
	
}

void ThreadPool::start(int numThreads)
{
	assert(threads_.empty());
	running_ = true;
	threads_.reserve(numThreads);
	for (int i = 0; i < numThreads; ++i)
	{
		char id[32];
		snprintf(id, sizeof id, "%d", i);
		threads_.push_back(new Thread(bind(&ThreadPool::runLoop, this), name_+" "+id));
		threads_[i]->start();
		threads_[i]->setAffinity(i);
	}
}

void ThreadPool::stop()
{
	//running_ = false;
	cond_.broadcast();

	int s = threads_.size();
	for(int i=0; i<s; i++)
	{
		Thread* thread = threads_.back();
		threads_.pop_back();
		thread->terminate();
		
		//printf("threadPool: 0x%lx delete ok\n", thread->threadID());
		delete thread;
	}
}

void ThreadPool::run(const Task& task)
{
	if (threads_.empty())
	{
		task();
	}
	else
	{
		MutexLock lock(mutex_);
		queue_.push_back(task);
		cond_.signal();
	}
}

ThreadPool::Task ThreadPool::take()
{
  	MutexLock lock(mutex_);
 	while (queue_.empty() && running_)
	{
		cond_.wait();
	}
	Task task;
	if(!queue_.empty())
	{
		task = queue_.front();
		queue_.pop_front();

		return task;
	}
	
	return NULL;
	
}

void ThreadPool::runLoop()
{
	while (running_)
	{
		Task task(take());
		if (task)
		{
			task();
		}
	}
}

