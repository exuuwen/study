/*
 * template.h
 *
 *  Created on: Nov 25, 2010
 *      Author: dragon
 */

#ifndef TEMPLATE_H_
#define TEMPLATE_H_

#include <iostream>
using namespace std;

template <class T, class W>
int compares(const T &v1, const W &v2)
{
	//v1 = 0;
	//v2 = 0;
	if(v1 < v2)  return -1;
	if(v2 < v1)  return 1;
	return 0;
}



template <class T, size_t N>     //非类型参数
int array_init(  T (&v)[N])  //数组引用
{
	for(int i = 0; i < N; i++)
		v[i] = 0;
	return 0;
}
/////////////////////////////////////////////////////////////////////
template <class type> class Queue;
template <class type> class QueueItem;

template <class type>
ostream& operator << (ostream &os, const Queue<type> &q)
{
	os << "< ";
	QueueItem<type> *p;
	for(p = q.head; p; p = p->next)
	{
		//cout <<"a"<< endl;
		os << p->item << " ";
	}
	os << ">";

	return os;
}



template <class type>
class QueueItem
{
	friend class Queue<type>;
	friend ostream& operator << <type> (ostream &os, const Queue<type> &q);
	QueueItem(const type &t):item(t), next(NULL){}
	type item;
	QueueItem *next;
};


template <class type>
class Queue
{
public:
	friend ostream& operator << <type> (ostream &os, const Queue<type> &q);
	Queue():head(0), tail(0){}

	Queue(const Queue &q):head(0), tail(0)
	{
		copy_items(q);
	}

	template <class it>
	Queue(it beg, it end):head(0), tail(0)
	{
		copy_items(beg, end);
	}
	~Queue()
	{
		//cout << "in the ~queue" << endl;
		destory();
	}

	template <class iter> void assign(iter beg, iter end)
	{
		destory();
		copy_items(beg, end);
	}


	void push(const type &item)
	{
		QueueItem<type> *new_queue = new QueueItem<type>(item);
		//cout << "item is " << item <<endl;
		if(empty())
		{
			head = tail = new_queue;
		}
		else
		{
			tail -> next = new_queue;
			tail = new_queue;
		}
	}

	type pop()
	{
		//cout << "in the pop" << endl;
		QueueItem<type> *p = head;
		type temp = p->item;
		head = head -> next;
		delete p;
		return temp;
	}

	bool  empty() const
	{
		return head == 0;
	}

	Queue& operator = (const Queue &q)
	{
		//cout << "in oper =" << endl;
		destory();
		copy_items(q);

		return *this;
	}

private:
	QueueItem<type> *head;
	QueueItem<type> *tail;
	void destory()
	{
		while(!empty())
		{
			//cout << "in destroy" <<endl;
			pop();
		}
	}

	void copy_items(const Queue &q)
	{
		QueueItem<type> *p = q.head;
		for(; p; p = p->next)
		{
			push(p->item);
		}
	}
	template <class it>
	void copy_items(it beg, it end)
	{
		//it temp = beg;
		while(beg != end)
		{
			//cout << "temp is " << *temp << endl;
			push(*beg);
			beg++;
		}
	}
};
/////////////////////////////////////////////////////////
template <class type> class Handle;
//template <class type> class Data;

template <class type>
class Data
{
public:
	//friend class Handle< Data<type> >;
	Data(type d): data(d){}
	type& get_data()
	{
		return data;
	}
private:
	type data;
};

template <class T>
class Handle
{
public:
	Handle(T *p = 0):ptr(p), use(new size_t(1)){}

	Handle(const Handle& h):ptr(h.ptr),use(h.use){++*use;}

	T* operator->()
	{
		if(ptr)
			return ptr;
		else
		{
			cout << "error retrun *T" <<endl;
			return 0;
		}
	}

	T& operator*()
	{
		if(ptr)
			return *ptr;
		else
			cout << "error retrun T&" <<endl;
	}

	Handle& operator=(const Handle& h)
	{
		rem_ref();
		ptr = h.ptr;
		use = h.use;
		use++;
		return *this;
	}

	~Handle()
	{
		rem_ref();
	}

private:
	T* ptr;
	size_t *use;
	void rem_ref()
	{
		if(--*use == 0 )
		{
			delete use;
			delete ptr;
		}
	}
};


#endif /* TEMPLATE_H_ */
