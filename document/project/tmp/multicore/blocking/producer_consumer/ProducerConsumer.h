#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H
#include <assert.h>

#include "Base.h"
#include "Condition.h"
#include "MutexLock.h"

template <class T> 
class ProducerConsumer : Noncopyable
{
public:
	ProducerConsumer(int size);
	~ProducerConsumer();

	void enq(T& t);
	void deq(T& t);

private:
	int size_;
	int count_;
	int tail_;
	int head_;
	T *items_;	
	Mutex mutex_;
	Condition fullCond_;
	Condition emptyCond_;
};


///////////////////////////////////////////////
template <class T>
ProducerConsumer<T>::ProducerConsumer(int size)
	: size_(size)
	, count_(0)
	, tail_(0)
	, head_(0)
	, items_(NULL)
	, mutex_()
	, fullCond_(mutex_)
	, emptyCond_(mutex_)
{
	items_ = new T[size];
	assert(items_ != NULL);
	
}

template <class T>
ProducerConsumer<T>::~ProducerConsumer()
{
	delete [] items_;
}

template <class T>
void ProducerConsumer<T>::enq(T& t)
{
	mutex_.lock();
	if (count_ == size_)
	{
		fullCond_.wait();
	}

	items_[tail_++] = t;
	count_++;

	if (tail_ == size_)
	{
		tail_ = 0;
	}

	emptyCond_.signal();
	
	mutex_.unlock();
}

template <class T>
void ProducerConsumer<T>::deq(T& t)
{
	mutex_.lock();
	if (count_ == 0)
	{
		emptyCond_.wait();
	}

	t = items_[head_++];
	count_--;

	if (head_ == size_)
	{
		head_ = 0;
	}
	
	fullCond_.signal();

	mutex_.unlock();
}

#endif
