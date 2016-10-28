#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

#include "MemoryPool.h"

MemoryPool::MemoryPool(unsigned int orderSize, unsigned char stepPower)
	: stepPower_(stepPower)
	, orderSize_(orderSize)
	, mutex_()
{
	stepSize_ = pow(2, stepPower_);
	memList_ = new struct Mem_list[orderSize_];
	if (memList_ == NULL)
	{
		assert(0);
	}
}

MemoryPool::~MemoryPool()
{
	unsigned int i;
	struct Mem_list *free_list;
	struct Mem_item *item; 

	printf("~Mempoll report\n");
	reportStatus();

	for(i=0; i<orderSize_; i++)
	{
		free_list = &memList_[i];
		while(free_list->first != NULL)
		{
			item = free_list->first;
			free_list->first = item->next;
			free((void*)item);
			free_list->count--;
		}
		assert(free_list->count == 0);
	}
	
	delete [] memList_;
	memList_ = NULL;
}

void* MemoryPool::__allocate(unsigned int size)
{
	char *p;
	unsigned int len;
	unsigned int index = size >> stepPower_;
	struct Mem_item *item;

	if(size == 0)
	{
		return NULL;
	}

	if(size % stepSize_ == 0)
	{
		index--;
	}

	if(index >= orderSize_)
	{
		len = size + sizeof(struct Mem_item);
		p = (char*)malloc(len);
		if(NULL == p)
		{
			perror("malloc error");
			return NULL;
		}
		item = (struct Mem_item*)p;
		item->order = 0xFFFFFFFF;
		item->next = NULL;

		return (void*)(p + sizeof(struct Mem_item));
	}
	
	struct Mem_list *free_list = &memList_[index];
	
	if(NULL == free_list->first)
	{
		len = (index + 1) * stepSize_ + sizeof(struct Mem_item);
		p = (char*)malloc(len);
		if(NULL == p)
		{
			perror("malloc error");
			return NULL;
		}
		item = (struct Mem_item*)p;
		item->order = index;
		item->next = NULL;
		
		return (void*)(p + sizeof(struct Mem_item));
	}
	
	p = (char*)(free_list->first);
	
	free_list->first = free_list->first->next;
	free_list->count--;
	
	return (void*)(p + sizeof(struct Mem_item));
}

int MemoryPool::__deallocate(void* p)
{
	struct Mem_item *item;

	item = (struct Mem_item*)((char*)p - sizeof(struct Mem_item));
	
	if(item->order == 0xFFFFFFFF)
	{
		free((void*)item);
	}
	else if(item->order < orderSize_)
	{
		struct Mem_list *free_list = &memList_[item->order];
		
		item->next = free_list->first ;
		free_list->first = item;
		free_list->count++;
	}
	else
	{
		printf("error item->order!");
		return -1;
	}
	
	return 0;
}
	
	
void* MemoryPool::allocate(unsigned int size)
{
	void *tmp;
	
	MutexLock lock(mutex_);
	tmp = __allocate(size);
	
	return tmp;
}


int MemoryPool::deallocate(void* p)
{
	int ret;
	
	MutexLock lock(mutex_);
	ret = __deallocate(p);
	
	return ret;
}

void MemoryPool::reportStatus(void)
{
	unsigned int count;
	
	MutexLock lock(mutex_);
	for(unsigned int i=0; i<orderSize_; i++)
	{
		struct Mem_list *free_list = &memList_[i];
		struct Mem_item *item = free_list->first;
		count = 0; 
		while(item)
		{
			count++;
			item = item->next;
		}
		
		printf("freelist %d free_list->count:%d vs count:%d\n", i, free_list->count, count);
		assert(count == free_list->count);
	}
}


		
