#ifndef _MUTEX_LOCK_H_
#define _MUTEX_LOCK_H_

#include <stdio.h>

class Mutex {
public:
	Mutex():tail(0), size(2) 
	{
		flag[0] = 1;
  	}
  	void lock();
  	void unlock();
private:
	int tail;
	int size;
	volatile long flag[2];
};

class MutexLock
{
public:
	explicit MutexLock(Mutex& mutex) : mutex_(mutex)
	{
		mutex_.lock();
	}

	~MutexLock()
	{
		mutex_.unlock();
	}

private:
	Mutex& mutex_;
};


#endif
