#include "MutexLock.h"
#include <assert.h>
#include <stdio.h>

void Mutex::lock() 
{
	while (1)
	{
		while (locked){};
		if (!__sync_lock_test_and_set(&locked, 1))
    			return;
	}
}

void Mutex::unlock() 
{
  	__sync_lock_release( &locked );
}


	
