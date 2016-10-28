#ifndef _H_CONCURRENT_QUEUE_H_
#define _H_CONCURRENT_QUEUE_H_

#include <deque>
#include <assert.h>

#include "Base.h"
#include "Condition.h"
#include "MutexLock.h"

template<typename T>
class ConcurrentQueue :Noncopyable
{
public:
	ConcurrentQueue()
	: mutex_(),
	  notEmpty_(mutex_),
	  queue_()
	{
	}

	void put(const T& x)
	{
		MutexLock lock(mutex_);
		queue_.push_back(x);
		notEmpty_.signal();
	}

	T take()
	{
		MutexLock lock(mutex_);
		// always use a while-loop, due to spurious wakeup
		while (queue_.empty())
		{
			notEmpty_.wait();
		}
		assert(!queue_.empty());
		T front(queue_.front());
		queue_.pop_front();
		return front;
	}

	size_t size() const
	{
		MutexLock lock(mutex_);
		return queue_.size();
	}

private:
	mutable Mutex mutex_;
	Condition     notEmpty_;
	deque<T>      queue_;
};


#endif
