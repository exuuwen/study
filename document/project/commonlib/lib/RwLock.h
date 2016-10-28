#ifndef _H_READ_WRITE_LOCK_H_
#define _H_READ_WRITE_LOCK_H_

#include <pthread.h>

#include "Base.h"

class RwLock : Noncopyable
{
public:
	RwLock();
	virtual ~RwLock();

	int readLock();
	int writeLock();
	int unlock();
	int tryReadLock();
	int tryWriteLock();
private:
	pthread_rwlock_t rwlock_;
};

class ReadLock : Noncopyable
{
public:
	explicit ReadLock(RwLock& mutex) : mutex_(mutex)
	{
		mutex_.readLock();
	}

	~ReadLock()
	{
		mutex_.unlock();
	}

private:
	RwLock& mutex_;
};

class WriteLock : Noncopyable
{
public:
	explicit WriteLock(RwLock& mutex) : mutex_(mutex)
	{
		mutex_.writeLock();
	}

	~WriteLock()
	{
		mutex_.unlock();
	}

private:
	RwLock& mutex_;
};


#endif



