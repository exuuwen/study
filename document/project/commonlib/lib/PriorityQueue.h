#ifndef _PRIORITY_QUEUE_H_
#define  _PRIORITY_QUEUE_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>

#include "Base.h"
#include "MutexLock.h"

using namespace std;


template <class T>
class PriorityQueue : Noncopyable
{
public:
	PriorityQueue(unsigned int size = 10000, unsigned char band_no = 6);
	~PriorityQueue();

	bool putq(const T &data, unsigned char priority);
	bool getq(T& data);
	void clear();	
	bool isEmpty() const;		
	bool isFull() const;
	unsigned int len() const;
	
private:
	struct msgb 	/* used internally */
	{
		struct msgb * b_next;	/* next message on queue */
		T data;	
		unsigned char m_band;		/* message priority */
	};

	typedef struct band_info 
	{
		struct msgb *qb_first;		/* first queue message in this band */
		struct msgb *qb_last;		/* last queued message in this band */
	}band_info_t;

	void __putq(struct msgb *msg);
	void __getq(T& data);

	//band_info_t *q_bandp;       /* band's flow-control information */
	//u8 q_nband;		    /* number of priority bands */
	//u8 q_mband;       	    /* the maximum band having messages in it */ 
	//u32 q_msgs;		    /* count msgs on the queue */
	//u32 q_nsize;                /* how many messages can be in the queue at same time */

	band_info_t *band_;       /* band's flow-control information */
	unsigned char bandNum_;		    /* number of priority bands */
	unsigned char bandMax_;       	    /* the maximum band having messages in it */ 
	unsigned int count_;		    /* count msgs on the queue */
	unsigned int size_;                /* how many messages can be in the queue at same time */
	mutable Mutex mutex_;
};

template <class T>
PriorityQueue<T>::PriorityQueue(unsigned int size, unsigned char band_no)
{
	// init counter
	count_ = 0;
	size_ = size;
		
	// init the qband info	
	bandNum_ = band_no;	
	// + 1 band...
	band_ = new band_info_t[bandNum_ + 1];
	if (band_ == NULL)
	{
		printf("create band_info for priority queue failed\n");
		assert(0);	
	}

	for (int i = 0; i <= bandNum_; i++)
	{
		band_[i].qb_first = NULL;
		band_[i].qb_last = NULL;	
	}
	
	bandMax_ = 0;	
}

template <class T>
PriorityQueue<T>::~PriorityQueue()
{
	clear();
	delete [] band_;	
}

// lock and unlock methods will implement multiple lock mechinisms.
// 1, if queue is used in one process, multithread, pthread_mutex
// 2, if the queue is used in multi-process, shared memory, semphore
// 3, if the queue is used in process and interrupt context, kernel irq lock.
// Here we only implement case 1.

// Flush the queue, free all the messages.
//TODO
template <class T>
void PriorityQueue<T>::clear()
{
	T data;

	while(getq(data));		
}

//
// Put a message in the priority queue, used externally.
// Return values:
// 0 : success
//>1 : failed
template <class T>
bool PriorityQueue<T>::putq(const T &data, unsigned char priority)
{
	bool ret = true;

	if(isFull())
	{
		ret = false;
	}
	else
	{
		struct msgb *msg = (struct msgb*)malloc(sizeof(struct msgb));
		if(msg == NULL)
		{
			printf("create msg error!\n");
			ret = false;
		} 
		else
		{
			msg->b_next = NULL;
			msg->data = data;
			msg->m_band = priority;
			MutexLock lock(mutex_);
			__putq(msg);	
		}
	}
	
	return ret;
}

// internal use to put message
template <class T>
void PriorityQueue<T>::__putq(struct msgb *msg)
{
	band_info_t *qb;

	if (msg->m_band > bandNum_)
	{
		// The specified band is bigger than max allowed, we can put this message 
		// to the end of the maximum band to make sure it will be handled in fifo 
		// manner as soon as possible instead of discard it.
		// this is a small trick. such kind of message handling should not happen.
		// here is just a kind exception handling.
		
		//band_max_ = band_num_;
		//qb = &band_[band_max_];
		qb = &band_[0];
		if (qb->qb_first == NULL)
		{
			qb->qb_first = msg;
			qb->qb_last = msg;
		}
		else
		{
			qb->qb_last->b_next = msg;
			qb->qb_last = msg;
		}
		cout << "Warning: super the band_num" << endl;
		count_++;
		
		return;
	}
	
	if (msg->m_band > bandMax_)
	{
		bandMax_ = msg->m_band;	
	}

	qb = &band_[msg->m_band];
	
	if (qb->qb_first == NULL)
	{
		qb->qb_first = msg;
		qb->qb_last = msg;
	}
	else
	{
		qb->qb_last->b_next = msg;
		qb->qb_last = msg;
	}
	
	count_++;				
}

template <class T>
bool PriorityQueue<T>::getq(T& data)
{
	bool ret = true;

	if(isEmpty())
	{
		ret = false;
	}
	else
	{
		MutexLock lock(mutex_);		
		__getq(data);
	}
		
	return ret;	
}

template <class T>
void PriorityQueue<T>::__getq(T& data)
{
	band_info_t *qb = &band_[bandMax_];
	
	// get the first one on the band
	struct msgb * msg = qb->qb_first;

	assert(msg != NULL);
	
	struct msgb *b_next = msg->b_next;

	// update the band pointers
	qb->qb_first = b_next;
	if (qb->qb_last == msg)
	{
		// this message is the last on in the band
		qb->qb_last = NULL;
		qb->qb_first = NULL;
		
		// only at this case, 
		// find a band having msg and assign it to band_x
		if (bandMax_ > 0)
		{
			char found = 0;
			for (int i = bandMax_ - 1; i >= 0; i--)
			{
				if (band_[i].qb_first != NULL)
				{
					bandMax_ = i;
					found = 1;
					break;	
				}			
			}
			
			if (found == 0)
			{
				bandMax_ = 0;
			}
		}			
	}
	
	// Null the pointer before pop the message out.
	msg->b_next = NULL;
	data = msg->data;
	free(msg);
	
	count_--;
}

// to check if the queue is empty, return true or false
template <class T>
bool PriorityQueue<T>::isEmpty() const
{
	MutexLock lock(mutex_);
	if(count_ == 0)
	{
		assert(bandMax_ == 0);
	}
	return (count_ == 0);
}

// to check if the queue is full, return true or false
template <class T>
bool PriorityQueue<T>::isFull() const
{
	MutexLock lock(mutex_);
	return (count_ == size_);
}

// to get the current messages number in queue
template <class T>
unsigned int PriorityQueue<T>::len() const
{
	MutexLock lock(mutex_);
	return count_;
}



#endif

