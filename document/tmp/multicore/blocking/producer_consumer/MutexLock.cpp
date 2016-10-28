#include "MutexLock.h"
#include <assert.h>
#include <stdio.h>

Mutex::Mutex(int type)
{
	if (type != PTHREAD_MUTEX_NORMAL && type != PTHREAD_MUTEX_RECURSIVE && type != PTHREAD_MUTEX_ERRORCHECK)
	{
		type = PTHREAD_MUTEX_DEFAULT;
	}
	type_ = type;

	if (pthread_mutexattr_init(&attr_) != 0)
	{
	    	perror("mutexattr_init failed\n");
		assert(0);
	}
	if (pthread_mutexattr_settype(&attr_, type_) != 0)
	{
	    	perror("mutexattr_settype failed\n");
		assert(0);
	}

	if (pthread_mutex_init(&mutex_, &attr_) != 0)
	{
	    	perror("PThreadMutex create failed\n");
		assert(0);
	}
}

Mutex::~Mutex()
{
	pthread_mutexattr_destroy(&attr_);
	pthread_mutex_destroy(&mutex_);
}

int Mutex::lock()
{
	return pthread_mutex_lock(&mutex_);
}

int Mutex::unlock()
{
	return pthread_mutex_unlock(&mutex_);
}

int Mutex::trylock()
{
	return pthread_mutex_trylock(&mutex_);	
}

int Mutex::type()
{
	return type_;
}



	
