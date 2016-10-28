#include <iostream>
#include <pthread.h>
#include <assert.h>

#include "Atomic.h"

using namespace std;

AtomicInt32 i32;
int count = 0;

void * Run1(void * para)
{
	int i = 0;
	while(i < 10000000)
	{
			i++;
			i32.inc();
			count++; //wrong one;
	}                        
	cout << "i32 last value in 1 thread " << i32.read() << " count:" << count << endl;
	return 0;
 
}

void * Run2(void * para)
{
	int i = 0;
	while(i < 10000000)
	{
			i++;
			i32.add(5);
			count += 5;//wrong one
	}             
	cout << "i32 last value in 2 thread " << i32.read() << " count:" << count << endl;  

   	return 0; 
        
}

int main()
{
	int32_t ret32;
	AtomicInt32 a32 = i32;
	
	cout << "i32:" << i32.read() << endl;

	ret32 = i32.add(5);
	cout << "i32:" << ret32 << endl;
	ret32 = i32.addReturn(2);
	cout << "old i32:" << ret32 << " new i32:"<< i32.read() << endl;

	ret32 = i32.sub(2);
	cout << "i32:" << ret32 << endl;
	ret32 = i32.subReturn(3);
	cout << "old i32:" << ret32 << " new i32:"<< i32.read() << endl;

	ret32 = i32.dec();
	cout << "i32:" << ret32 << endl;
	ret32 = i32.decReturn();
	cout << "old i32:" << ret32 << " new i32:"<< i32.read() << endl;

	ret32 = i32.inc();
	cout << "i32:" << ret32 << endl;
	ret32 = i32.incReturn();
	cout << "old i32:" << ret32 << " new i32:"<< i32.read() << endl;

	ret32 = i32.set(100);
	cout << "old i32:" << ret32 << " new i32:"<< i32.read() << endl;

	pthread_t id1;
	pthread_t id2;
	
	int ret;
	ret = pthread_create(&id1, NULL, Run1, NULL);
	assert(ret == 0);
	
	ret = pthread_create(&id2, NULL, Run2, NULL);
	assert(ret == 0);

  	pthread_join(id1, NULL);
  	pthread_join(id2, NULL);
	
	cout << "finally i32 last value in main thread " << i32.read() << " count:" << count << endl;
	return 0;
}
