#include <stdio.h>
#include <new>
int main()
{
	char arr[20];
	
	for(int i=0; i<20; i++)
		arr[i] = i;
		
	int* p = new(arr) int(20); 
	printf("%p,%p\n",&arr, p);
	
	for(int i=0; i<20; i++)
		printf("arr[i]:%d\n", arr[i]);
		
	return 0;
}

