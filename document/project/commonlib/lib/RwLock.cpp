#include <stdio.h>
#include <assert.h>

#include "RwLock.h"

RwLock::RwLock()
{
	
	if (pthread_rwlock_init(&rwlock_, NULL) != 0)
	{
		perror("PThreadRWlock create failed\n");
		assert(0);
	}
}

RwLock::~RwLock()
{

	pthread_rwlock_destroy(&rwlock_);
}

int RwLock::readLock()
{
	return pthread_rwlock_rdlock(&rwlock_);
}

int RwLock::writeLock()
{
	return pthread_rwlock_wrlock(&rwlock_);
}

int RwLock::unlock()
{
	return pthread_rwlock_unlock(&rwlock_);
}

int RwLock::tryReadLock()
{
	return pthread_rwlock_tryrdlock(&rwlock_);
}

int RwLock::tryWriteLock()
{
	return pthread_rwlock_trywrlock(&rwlock_);
}

