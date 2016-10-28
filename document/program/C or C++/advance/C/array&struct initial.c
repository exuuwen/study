#include <fcntl.h>
#include <stdio.h>

struct test
{
	int data;
	char *m;
};



int main()
{
	int a = 0;
	int b = -1;
	
	/* array and struct initial */
	struct test array[10] = {
		[0 ... 9] = {
			.data = 10		
		},
		[2] = {
			.data = 3
		},
		[5] = {
			.data = 9
		},
		[8] = {
			.data = 8
		}
	};

	int i;
	for(i=0; i<10; i++)
	{
		printf("array[%d]:%d\n", i, array[i].data);
	}
	

	struct test tt;
	tt.data = 0;
	tt.m = NULL;
	
	
	printf("m member offset:%d\n", ((size_t) &((struct test *)0)->m) );
	// why
	return 0;
}


