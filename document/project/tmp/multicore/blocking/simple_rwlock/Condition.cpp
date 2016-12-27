#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <assert.h>

#include "MutexLock.h"
#include "Condition.h"

Condition::Condition(Mutex& mutex) : mutex_(mutex)
{
	if (pthread_cond_init(&cond_, NULL) != 0)
	{
		perror("create cond error");
		assert(0);
	}
}

Condition::~Condition()
{
	pthread_cond_destroy(&cond_);
}

int Condition::wait()
{
	int ret;                                                                                                             
	                                                                                        
	ret = pthread_cond_wait(&cond_, &(mutex_.mutex_)); 

	return ret;
}

int Condition::waitTimeout(unsigned int ms)
{
	struct timeval now;
	
	gettimeofday(&now, NULL);

	struct timespec tm;
	tm.tv_sec = ms / 1000;
	ms %= 1000;
	tm.tv_nsec = ms * 1000000;

	tm.tv_sec += now.tv_sec;
	tm.tv_nsec += now.tv_usec * 1000;
	if (tm.tv_nsec > 1000000000)
	{
		tm.tv_sec += 1;
		tm.tv_nsec -= 1000000000;
	}

	int ret;

	                                                                                                            
	ret = pthread_cond_timedwait(&cond_, &(mutex_.mutex_), &tm);                                                         
	
	return ret;
}

int Condition::broadcast()
{
	int ret;
	                                                                                              
	ret = pthread_cond_broadcast(&cond_);

	return ret;
}

int Condition::signal()
{
	int ret;
	
	ret = pthread_cond_signal(&cond_);

	return ret;
}

