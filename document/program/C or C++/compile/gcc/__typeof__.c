#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/*__typeof__:define a type use "ture type(int char...)" or express of other variable*/
#define DEFINE_MY_TYPE(type, name) __thread __typeof__(type) my_var_##name

DEFINE_MY_TYPE(int, one); //It   is   equivalent   to  '__thread int  my_var_'; which is a thread variable.
int main()
{
	__typeof__(int *) x; //It   is   equivalent   to  'int  *x';

	__typeof__(int) a;//It   is   equivalent   to  'int  a';

	__typeof__(*x)  y;//It   is   equivalent   to  'int y';

	__typeof__(&a) b;//It   is   equivalent   to  'int  b';

	__typeof__(__typeof__(int *)[4])   z; //It   is   equivalent   to  'int  *z[4]';
	
	y = *x;
	b = &a;
	
	z[0] = x;
	z[1] = &a;

	return 0;
}



