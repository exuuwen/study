#ifndef _H_CONDITION_H_
#define _H_CONDITION_H_
#include <pthread.h>

#include "MutexLock.h"
#include "Base.h"

class Condition : Noncopyable
{
public:
	Condition(Mutex &mutex);
	~Condition();

	int wait();
	int waitTimeout(unsigned int ms);
	int signal();
	int broadcast();

private:
	pthread_cond_t	cond_;
	Mutex& mutex_; 
};


#endif


