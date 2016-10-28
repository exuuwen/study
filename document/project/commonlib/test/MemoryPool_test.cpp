#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "MemoryPool.h"

#define ORDER 5
#define STEP 64

static void *tmp[ORDER];
static void *tps[1000];

int main()
{
	MemoryPool test_mempool(ORDER);
	
	for(unsigned int i=0; i<ORDER; i++)
	{
		for(unsigned int j=0; j<=i; j++)
		{
			tmp[j] = test_mempool.allocate(STEP*(i+1) - i);
			if(tmp[j] == NULL)
			{
				perror("allocate error");
				return -1;
			}	
		}
		for(unsigned int j=0; j<=i; j++)
		{
			int ret = test_mempool.deallocate(tmp[j]);
			if(ret == -1)
			{
				perror("deallocate error");
				return -1;
			}
		}
		printf("%d runs\n", i);
		test_mempool.reportStatus();	
	}

	void* tp = test_mempool.allocate(ORDER*STEP + 1);
	if(tp == NULL)
	{
		perror("super allocate error");
		return -1;
	}
	int ret = test_mempool.deallocate(tp);
	if(ret == -1)
	{
		perror("super deallocate error");
		return -1;
	}

	printf("super ORDER*STEP report\n");
	test_mempool.reportStatus();	

	
	for(unsigned int j=0; j<1000; j++)
	{
		tps[j] = test_mempool.allocate(STEP*4 - 1);
		if(tps[j] == NULL)
		{
			perror("last allocate error");
			return -1;
		}
	}

	for(unsigned int j=0; j<1000; j++)
	{
		int ret = test_mempool.deallocate(tps[j]);
		if(ret == -1)
		{
			perror("last deallocate error");
			return -1;
		}
	}
	
	return 0;
}


