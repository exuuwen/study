#ifndef _MUTEX_LOCK_H_
#define _MUTEX_LOCK_H_

#include <pthread.h>
#include "Base.h"

class Mutex : Noncopyable
{
friend class Condition;
public:
	Mutex(int type = PTHREAD_MUTEX_DEFAULT);
	virtual ~Mutex();

	int lock();
	int unlock();
	int trylock();
	int type();
private:
	pthread_mutex_t mutex_;
	pthread_mutexattr_t attr_;
	int type_;
};

class MutexLock : Noncopyable
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

class MutexTryLock : Noncopyable
{
public:
	explicit MutexTryLock(Mutex& mutex) : mutex_(mutex), locked(false)
	{
		if (mutex_.trylock() == 0)
			locked = true;
	}

	~MutexTryLock()
	{
		if (locked)
			mutex_.unlock();
	}

private:
	Mutex& mutex_;
	bool locked;
};

#endif
