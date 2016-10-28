#include "MutexLock.h"
#include <assert.h>
#include <stdio.h>

void Mutex::lock() 
{
  	while( __sync_lock_test_and_set( &locked, 1));
}

void Mutex::unlock() 
{
	__sync_lock_release( &locked );
}


	
