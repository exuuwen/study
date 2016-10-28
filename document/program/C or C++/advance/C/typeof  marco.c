#include <fcntl.h>
#include <stdio.h>


#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; }


int main()
{
	int a = 0;
	int b = -1;
	

	int min = min(a, b);
	printf("min : %d\n", min);

	return 0;
}

/* 12: warning: comparison of distinct pointer types lacks a cast [enabled by default]  */
/* (void) (&_min1 == &_min2);		\ */

