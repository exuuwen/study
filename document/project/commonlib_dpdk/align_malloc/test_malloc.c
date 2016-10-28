#include "align_malloc.h"

int main()
{
	void * test_align = align_malloc(125, 32);
	void * test_old = malloc(125);
	printf("the test_align@0x%p and %ld\n", test_align, (unsigned long)test_align%32);
	printf("the test_old@0x%p and %ld\n", test_old, (unsigned long)test_old%32);
	align_free(test_align);
	free(test_old);
	return 0;
}
