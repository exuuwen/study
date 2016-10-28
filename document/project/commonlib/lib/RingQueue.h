#ifndef _H_RING_QUEUE_H_
#define _H_RING_QUEUQ_H_
#include <stdio.h>
#include <assert.h>

#include "MutexLock.h"
#include "Base.h"

template <class T>
class RingQueue : Noncopyable
{
public:
	RingQueue(int size = 100);
	~RingQueue();
	
	bool push(const T &msg);
	bool pop(T &msg);
	
	bool isEmpty() const;
	bool isFull() const;
	int  len() const;
	void clear();
	
private:
	int size_;
	T   *list_;	
        //Tell the compiler that don't do any optimization to put these two 
        //variables into registers, it must be in memory for multithreading
	//volatile int m_nFront;
	//volatile int m_nRear;	
	int front_;
	int rear_;	
	mutable Mutex mutex_;
};

//////////////////////////////////////////////////////////////////
template <class T>
RingQueue<T>::RingQueue(int size) : mutex_()
{
	front_ = 0;
	rear_ = 0;
	size_ = size + 1;
	list_ = new T[size_];
	if (list_ == NULL)
	{
		printf("memory error!");
		assert(0);	
	}
}

template <class T>
RingQueue<T>::~RingQueue()
{
	if (list_ != NULL)
	{
		delete[] list_;
		list_ = NULL;	
	}
}

template <class T>
bool RingQueue<T>::push(const T &msg)
{
	if (isFull())
	{
		return false;	
	}
	
	MutexLock lock(mutex_);
	
	list_[rear_] = msg;
	
	rear_ = (rear_ + 1) % size_;
	
	return true;
}

template <class T>
bool RingQueue<T>::pop(T &msg)
{
	if (isEmpty())
	{
		return false;
	}

	MutexLock lock(mutex_);
	
	msg = list_[front_];
	
	front_ = (front_ + 1) % size_;
	
	return true;	
}

template <class T>
bool RingQueue<T>::isEmpty() const
{
	MutexLock lock(mutex_);

	if (front_ == rear_)
	{
		return true;	
	}	
	else
	{
		return false;
	}
}

template <class T>
bool RingQueue<T>::isFull() const
{
	MutexLock lock(mutex_);

	if (((rear_ + 1) % size_) == front_)
	{
		return true;	
	}	
	else
	{
		return false;	
	}
}

template <class T>
int RingQueue<T>::len() const
{
	MutexLock lock(mutex_);

	return ((rear_ - front_ + size_) % size_);
}

template <class T>
void RingQueue<T>::clear()
{
	MutexLock lock(mutex_);

	front_ = 0;
	rear_ = 0;
}

#endif
