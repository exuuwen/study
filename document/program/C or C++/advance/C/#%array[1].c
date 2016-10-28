#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define N 3

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
 
struct A
{
	int m;
	void *array[1];/*the same as [0]*/
}; 

void main()
{
	
        struct A *a;
	int i, j;
	
	printf("A:%lu, off:%lu\n", sizeof(struct A), offsetof(struct A, array));
	a = (struct A*)malloc(offsetof(struct A, array) + N * sizeof(void*));

	for(i=0; i<N; i++)
	{
		a->array[i] = malloc(10);
	} 

	for(i=0; i<N; i++)
	{
		memset((unsigned char*)(a->array[i]), i+1, 10);
	} 
	
	for(i=0; i<N; i++)
	{
		for(j=0; j<10; j++)
			printf("a->array[%d][%d]: %d ",i, j, *(unsigned char*)(a->array[i] + j));
		printf("\n");
	} 

	for(i=0; i<N; i++)
	{
		free(a->array[i]);
	} 

	free(a);

	return;
}
