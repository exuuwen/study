#ifndef READ_WRITE_LOCK_H
#define READ_WRITE_LOCK_H
#include <assert.h>

#include "Base.h"
#include "Condition.h"
#include "MutexLock.h"

class Rwlock : Noncopyable
{
public:
	Rwlock();
	~Rwlock();

	void rlock();
	void wlock();

	void runlock();
	void wunlock();
private:
	Mutex mutex_;
	Condition cond_;
	int readers_;
	bool writer_;
};


class ReadLock : Noncopyable
{
public:
        explicit ReadLock(Rwlock& mutex) : mutex_(mutex)
        {
                mutex_.rlock();
        }

        ~ReadLock()
        {
                mutex_.runlock();
        }

private:
        Rwlock& mutex_;
};

class WriteLock : Noncopyable
{
public:
        explicit WriteLock(Rwlock& mutex) : mutex_(mutex)
        {
                mutex_.wlock();
        }

        ~WriteLock()
        {
                mutex_.wunlock();
        }

private:
        Rwlock& mutex_;
};


#endif
