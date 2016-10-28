#include <stdio.h>
#include <assert.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

#include "Thread.h"

namespace CurrentThread
{
	__thread const char* threadName = "main";
}

namespace
{
	__thread pid_t cachedTid = 0;

	pid_t gettid()
	{
		return static_cast<pid_t>(::syscall(SYS_gettid));
	}
}

pid_t CurrentThread::tid()
{
	if (cachedTid == 0)
	{
		cachedTid = gettid();
	}
	return cachedTid;
}

const char* CurrentThread::name()
{
	return threadName;
}

bool CurrentThread::isMainThread()
{
	return tid() == ::getpid();
}



Mutex Thread::mutex_;
list<Thread*> Thread::threads_;

Thread::Thread(const ThreadFunc& func, const string& name)
	: threadID_(0)
	, detachState_(false)
	, syscope_(false)
	, threadFunc_(func)
	, coreIndex_(-1)
	, name_(name)
	, tid_(0)
{
	int ret = pthread_attr_init(&threadAttr_);
	if(ret != 0)
	{
		perror("pthread_attr_init failed:");
		assert(0);
	}

	ret = pthread_attr_setdetachstate(&threadAttr_, PTHREAD_CREATE_JOINABLE);
	if(ret != 0)
	{
		perror("thread setdetachstate failed:");
		assert(0);
	}

	state_ = INIT;
}

Thread::~Thread()
{
	pthread_attr_destroy(&threadAttr_);
	
	if (state_ == RUNNING)
	{	
		int ret = terminate();	
		if(ret)
		{	
			printf("thread 0x%lx terminate failed:%s\n", threadID_, strerror(ret));
		}
	}

	{
		MutexLock lock(mutex_);
		threads_.remove(this);
	}

	printf("~thread ptr %p, id 0x%lx\n", this, threadID_);
}

pthread_t Thread::threadID() 
{
	return threadID_;
}

int Thread::start()
{
	int err;
	
	if(state_ != INIT)
	{
		printf("warning:thread 0x%lx already started\n", threadID_);
		return 0;
	}	

	err = pthread_create(&threadID_, &threadAttr_, &threadProc, this);	
	if(err == 0)
	{
		printf("thread ptr %p, id 0x%lx\n", this, threadID_);
		{
			MutexLock lock(mutex_);
			threads_.push_back(this);
		}
		state_ = RUNNING;
		return 0;
	}
	else
	{
		perror("create worker thread failed:");
		return -1;
	}
}

void Thread::run()
{
	tid_ = CurrentThread::tid();
	CurrentThread::threadName = name_.c_str();
	threadFunc_();

	state_ = TERMINATED;
}

pid_t Thread::tid()
{
	return tid_;
}

void * Thread::threadProc(void *param)
{
	Thread* thread = static_cast<Thread*>(param);

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	thread->run();

	return NULL;
}

int Thread::join()
{
	if (detachState_)
	{
		printf("Waring: thread 0x%lx join: Pthread is not joinable\n", threadID_);
		return -1;
	}
	else
	{
		int ret = pthread_join(threadID_, NULL);
		if(ret)
		{	
			printf("thread 0x%lx join failed. errno=%s\n", threadID_, strerror(ret));
			return -1;
		}
	}

	return 0;
}

int Thread::terminate()
{
	if (getThreadState() == RUNNING)
	{
		int ret = pthread_cancel(threadID_);	
		if(ret)
		{	
			printf("thread 0x%lx terminate failed::%s\n", threadID_, strerror(ret));
			return -1;
		}

		state_ = TERMINATED;
		return 0;
	}	
	else
	{
		//printf("Waring: thread 0x%lx Terminate: Pthread is not running\n", threadID_);
		return 0;
	}
}


int Thread::detach()
{
	if (getThreadState() == RUNNING)
	{
		if (detachState_)
		{
			return 0;
		}
		int ret = pthread_detach(threadID_);
		if (ret)
		{
			printf("thread 0x%lx pthread_detach failed:%s\n", threadID_, strerror(ret));
			return -1;
		}
		detachState_ = true;
		return 0;
	}
	else
	{
		printf("thread 0x%lx DetachePthread error:the pthread is not running\n", threadID_);
		return -1;	
	}
}


int Thread::setThreadAttr(bool detachable, bool syscope)
{
	if (state_ == INIT)
	{
		if (detachable)
		{		
			int ret = pthread_attr_setdetachstate(&threadAttr_, PTHREAD_CREATE_DETACHED);
			if(ret != 0)
			{
				printf("thread 0x%lx setdetachstate failed:%s\n", threadID_, strerror(ret));
				return -1;
			}

			detachState_ = true;
		}

		if (syscope)
		{		
			int ret = pthread_attr_setscope(&threadAttr_, PTHREAD_SCOPE_SYSTEM);
			if (ret != 0)
			{
				printf("thread 0x%lx setscope failed:%s\n", threadID_, strerror(ret));
				return -1;
			}

			syscope_ = true;
		}
		
		return 0;
	}
	else
	{
		printf("thread 0x%lx set_pthread_attr error:the pthread is started\n", threadID_);
		return -1;
	}
}	

//
// Make sure this function is called after Start() of the thread.
//
// policy: SCHED_OTHER (0), SCHED_FIFO (1), SCHED_RR (2).
// if policy is 0 ,the priority only can be 0. why? TODO we can not change the priority of SCHED_OTHER  .... pthread_setschedprio(3) can or not
// if policy is 1 or 2, then priority must be from 1 to 99. and must have root privelege
int Thread::setPriority(int p, int priority)
{
	if (getThreadState() == RUNNING)
	{
		struct sched_param param;  
		int policy, ret;

		if ((p == 0) && (priority == 0))
		{
			param.sched_priority = 0; 
			policy = 0;
			if ((ret = pthread_setschedparam(threadID_, policy, &param)) != 0)
			{
				printf("set priority %d for thread 0x%lx failed:%s\n", policy, threadID_, strerror(ret));
				return -1;
			}	
		}
		else if ((p <= 2) && (p >=1 ) && (priority >= 1) && (priority <= 99))
		{
			param.sched_priority = priority; 
			policy = p;
			if ((ret = pthread_setschedparam(threadID_, policy, &param)) != 0)
			{
				printf("set priority %d for thread: 0x%lx failed:%s", policy, threadID_, strerror(ret));
				return -1;
			}	
		}
		else
		{
			printf("thread 0x%lx invalid policy:%d and priority:%u specified\n", threadID_, p, priority);
			return -1;
		}	
	}
	else
	{
		printf("thread 0x%lx SetPriority error:the pthreadstate is not Running\n", threadID_);
		return -1;	
	}

	return 0;
}

int Thread::getPriority(int* policy, int* priority)
{
	if (getThreadState() == RUNNING)
	{
		struct sched_param param;  
		
		int ret = pthread_getschedparam(threadID_, policy, &param);	
		if (ret != 0)
		{
			printf("get priority for thread: 0x%lx failed:%s\n", threadID_, strerror(ret));
			return -1;
		}
		*priority = param.sched_priority;
		return 0;
	}
	else
	{
		printf("thread 0x%lx GetPriority error:the pthreadstate is not running\n", threadID_);
		return -1;	
	}
	
}

int Thread::setAffinity(unsigned char index)
{
	if (getThreadState() == RUNNING)
	{
		cpu_set_t mask, get;
		/////
		// gettid() system call is different with pthread_self() and getpid(), the value 
		// it gets is different with thread id. 
		// In a single-threaded process, the thread ID is equal to the process ID (PID, as 
		// returned by getpid()).  In a multithreaded process, all threads have the same PID, 
		// but each one has a unique TID.  	
		//
		int num = sysconf(_SC_NPROCESSORS_CONF);	

	    	// It is very good to know the return value, since on GTT Linux DP it
	    	// will return a surprising value: 0
		if (num <= 0)
		{
			printf("failed to get the number of processors on this board. ignore the affinity set\n");
			return -1;
		}
    	else
    	{
			printf("There are %d cores on this board\n", num);
    	}

		int cpu = index % num;
		CPU_ZERO(&mask);
		CPU_SET(cpu, &mask);
		if (pthread_setaffinity_np(threadID_, sizeof(cpu_set_t), &mask))
		{
			printf("set affinity for thread 0x%lx to cpu:%d failed\n", threadID_, cpu);
			return -1;
		}

		printf("set affinity for thread 0x%lx to cpu:%d successfully.\n", threadID_, cpu);
	
		// get the affinity
		CPU_ZERO(&get);
		if (pthread_getaffinity_np(threadID_, sizeof(cpu_set_t), &get) == -1)
		{
			printf("get cpu affinity for thread 0x%lx failed\n", threadID_);
			return -1;
		}
	
		for (int i = 0; i < num; i++)
		{
			if (CPU_ISSET(i, &get))
			{
				printf("thread: 0x%lx is running on cpu %d\n", threadID_, i);
				coreIndex_ = i;
				break;
			}
		}	
	}
	else
	{
		printf("thread 0x%lx SetAffinity error:the pthreadstate is not running\n", threadID_);
		return -1;
	}
	
	return 0;		
}

Thread::ThreadState Thread::getThreadState()
{
	if (state_ == RUNNING)
	{
		int err = pthread_kill(threadID_, 0); //check the Thread exits
		if (err != 0)
		{
			//printf("thread do not exits\n"); // in case a thread call pthread_exit or die in accident
			state_ = TERMINATED;
		}
	}
	
	return state_;
}

bool Thread::getDetachState()
{
	return detachState_;
}

bool Thread::getContentScope()
{
	return syscope_;
}

void Thread::waitAllThreadsTerminate()
{
back:	
	list<Thread*>::iterator iter;	
	
	{
		MutexLock lock(mutex_);
	
		iter = threads_.begin();
	}
	
	int zong = 0;
	while(1)
	{
		zong = 0;
		{
			MutexLock lock(mutex_);
			if (iter == threads_.end())
				break;
		}
		printf("waiting for ptr %p, id 0x%lx\n", (*iter), (*iter)->threadID());
		Thread* p = *iter;
		
		if(!(p->getDetachState()))
		{
			p->join();
		}

		{
			MutexLock lock(mutex_);
			int i = 0;
			list<Thread*>::iterator iter2;
			for(iter2 = threads_.begin(); threads_.end()!= iter2; iter2++)
			{
				Thread* p2 = *iter2;
				if (p2 == p)
				{
					zong = 1;
					printf("zong %d, p %p, id 0x%lx\n", i, p, p->threadID());		
				}	
				i++;
			}
		}
		if (!zong)
			goto back;
		
		{
			MutexLock lock(mutex_);
			iter++;
		}
	}
	
}




