#include "MutexLock.h"
#include <assert.h>
#include <stdio.h>

static __thread int slot;
void Mutex::lock() 
{
	slot = __sync_fetch_and_add(&tail, 1) % size;
	
        while (!flag[slot]);
}

void Mutex::unlock() 
{
	flag[slot] = 0;
	flag[(slot + 1) % size] = 1;
	
}


	
