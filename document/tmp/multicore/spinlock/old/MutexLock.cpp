#include "MutexLock.h"
#include <assert.h>
#include <stdio.h>

Mutex::Mutex()
{

	if (pthread_spin_init(&mutex_, 0) != 0)
	{
	    	perror("PThreadMutex create failed\n");
		assert(0);
	}
}

Mutex::~Mutex()
{
	pthread_spin_destroy(&mutex_);
}

int Mutex::lock()
{
	return pthread_spin_lock(&mutex_);
}

int Mutex::unlock()
{
	return pthread_spin_unlock(&mutex_);
}




	
