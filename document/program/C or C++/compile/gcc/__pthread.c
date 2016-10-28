#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static __thread int count;

void *function1(void *argc)
{
	printf("porgran pid:%u, the function1 pthread id is %lu, count:%d\n",getpid(), pthread_self(), count);
	count = 10;
	printf("porgran pid:%u, last the function1 pthread id is %lu, count:%d\n",getpid(), pthread_self(), count);

	return 0;
}

void *function2(void *argc)
{
	printf("porgran pid:%u, the function2 pthread id is %lu, count:%d\n", getpid(), pthread_self(), count);
	sleep(2);
	count = 100;
	printf("porgran pid:%u, last the function2 pthread id is %lu, count:%d\n", getpid(), pthread_self(), count);

	return 0;
}


int main()
{
	pthread_t  thread_id[2];
	int ret;
	pthread_t mian_thread_id;
	mian_thread_id = pthread_self();
	
	count = 2;
	
	printf("porgran pid:%u, mian_thread_id:%lu, count:%d\n", getpid(), mian_thread_id, count);

	ret = pthread_create(thread_id, NULL, function1, NULL);
	assert(ret == 0);

	ret = pthread_create(thread_id + 1, NULL, function2, NULL);
	assert(ret == 0);

	ret = pthread_join(thread_id[0], NULL);
	assert(ret == 0);
	ret = pthread_join(thread_id[1], NULL);
	assert(ret == 0);

	count = 1000;
	printf("porgran pid:%u, last mian_thread_id:%lu, count:%d\n", getpid(), mian_thread_id, count);
	return 0;
}



