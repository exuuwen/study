#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_

#include "MutexLock.h"
#include "Base.h"
/*              |--0->step_size--|step_size + 1->2*step_size|			      |(n-1)step_size-1->n*step_size|
		+------------------+----------------------- +--- -- -- -- -- -- -- -- +-----------------------------+
mempool_list:	+      order0      +         order1	    +	                      +    order(n-1) n=order_size  +
		+------------------+------------------------+--- -- -- -- -- -- -- -- +-----------------------------+
		    |
		    | first
		    v
		+------------------+  next  +------------------+	            +------------------+
order(x):	+ Mem_item1 + buf1 +------->+ Mem_item2 + buf2 +-- -- -- -- -- -- --+ Mem_itemn + bufn +
		+------------------+	    +------------------+	            +------------------+	
		
*/

class MemoryPool : Noncopyable
{
public:
	MemoryPool(unsigned int orderSize = 32, unsigned char stepPower = 6);
	~MemoryPool();
	void* allocate(unsigned int);
	int   deallocate(void* pool);
	void  reportStatus();
	
private:
	void* __allocate(unsigned int);
	int   __deallocate(void* pool);
	
	unsigned int stepSize_;
	unsigned char stepPower_;
	unsigned int orderSize_;
	mutable Mutex mutex_;
	
	struct Mem_item
	{
		unsigned int order;
		struct Mem_item *next;
		Mem_item()
		{
			order = 0xFFFFFFFF;
			next = NULL;
		}
	};
	
	struct Mem_list
	{
		struct Mem_item *first;
		unsigned int count;
		Mem_list()
		{
			first = NULL;
			count = 0;
		}
	};
	
	struct Mem_list  *memList_;
};

#endif
