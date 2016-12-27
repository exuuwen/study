#ifndef _MUTEX_LOCK_H_
#define _MUTEX_LOCK_H_

#include <pthread.h>

class Mutex
{
public:
	Mutex();
	virtual ~Mutex();

	int lock();
	int unlock();
	int type();
private:
	pthread_spinlock_t mutex_;
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
