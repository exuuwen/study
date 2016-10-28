#ifndef _H_SINGLE_QUEUE_H_
#define _H_SINGLE_QUEUE_H_
#include <stdio.h>
#include <assert.h>

#include "MutexLock.h"
#include "Base.h"

template <class T>
class SingleQueue : Noncopyable
{
public:
	SingleQueue();
	virtual ~SingleQueue();

	bool push(const T&);
	bool pop(T&);
	unsigned int size();

private:
	struct node
	{
		T data;
		struct node *next;
	};
	
	unsigned int size_;
	struct node *head_;
	struct node *tail_;	

	mutable Mutex mutex_;
};

//////////////////////////////////////////////////////////
template <class T>
SingleQueue<T>::SingleQueue()
{
	struct node *tmp = new struct node;
	if(tmp == NULL)
	{
		assert(0);
	}
	
	size_ = 0;
	tmp->next = NULL;
	tail_ = head_ = tmp;
}

template <class T>
SingleQueue<T>::~SingleQueue()
{
	T data;
	size_ = 0;

	while(pop(data));
}

template <class T>
bool SingleQueue<T>::push(const T& data)
{
	struct node *tmp;
	
	tmp = new struct node;
	if(tmp == NULL)
	{
		return false;
	}

	MutexLock lock(mutex_);

	tmp->data = data;
	tmp->next = NULL;

	tail_->next = tmp;
	tail_ = tmp;
	size_++;

	return true;
}

template <class T>
bool SingleQueue<T>::pop(T& data)
{
	MutexLock lock(mutex_);

	struct node *old = head_;
	struct node *tmp = old->next;

	if(tmp == NULL)
	{
		return false;
	}
	
	data = tmp->data;
	head_ = tmp;
	delete old;
	size_--;

	return true;
}

template <class T>
unsigned int SingleQueue<T>::size()
{
	return size_;
}

#endif
