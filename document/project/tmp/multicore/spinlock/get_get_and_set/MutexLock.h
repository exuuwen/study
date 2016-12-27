#ifndef _MUTEX_LOCK_H_
#define _MUTEX_LOCK_H_

class Mutex {
public:
	Mutex() 
	{
    		locked = 0;
  	}
  	void lock();
  	void unlock();
private:
     	/*volatile*/ int locked;
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
