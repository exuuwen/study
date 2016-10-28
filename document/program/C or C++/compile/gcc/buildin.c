#include <stdio.h>
#include <stdlib.h>

//gcc -O2 -o test buildin.c
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

int main()
{
	int c;

	if( __builtin_types_compatible_p(typeof(c), int))
		printf("it's a int type\n");
	else
		printf("it's not a int type\n"); 

	int a = 10;
	int *b = malloc(sizeof(int));
	int ret = -1;

	*b = ret;
	ret = __builtin_constant_p(a + *b);
	free(b);

	printf("ret:%d\n", ret);

	__builtin_prefetch (&c, 1, 1);
	__builtin_prefetch (&ret, 0, 1);
	c = ret;

	return 0;
}
