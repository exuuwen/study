#ifndef _H_PTHREAD_H_
#define _H_PTHREAD_H_
#include <iostream>
#include <string>
#include <pthread.h>

#include <list>
#include <tr1/functional>

#include "Base.h"
#include "MutexLock.h"

using namespace std::tr1;
using namespace std;

class Thread : public Noncopyable
{
public:
	enum ThreadState
	{
		INIT = 0,
		RUNNING,
		TERMINATED,
	};

	typedef function<void ()> ThreadFunc;

	Thread(const ThreadFunc&, const string& name = string());
	virtual ~Thread();

	int start();
	pid_t tid();
	int setPriority(int policy, int priority); 
	int getPriority(int* policy, int *priority); 
	pthread_t threadID();
	int detach();
	int terminate();
	int setThreadAttr(bool detachable = false, bool systemscope = false);
	bool getDetachState();
	bool getContentScope();
	int setAffinity(unsigned char index); // set cpu affinity for this thread basing on thread index
	int join();
	ThreadState getThreadState();

	static void waitAllThreadsTerminate();// usually called by main
private:
	static void * threadProc(void * param);
	void run();
	pthread_t threadID_;
	pthread_attr_t threadAttr_;
	bool detachState_;
	bool syscope_;
	ThreadFunc threadFunc_;
	int coreIndex_;
	string name_;
	pid_t tid_;
	ThreadState state_;

	static Mutex mutex_;
	static list<Thread*> threads_;
};

namespace CurrentThread
{
  pid_t tid();
  const char* name();
  bool isMainThread();
}

#endif

