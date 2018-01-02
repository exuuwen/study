#include <stdio.h>
#include <stdlib.h>

struct mm
{
	int b;
	char *c;
};

int a;
struct mm *m;

void func1(int *data)
{
	*data = 100;
}

void func2(int data, char *ch)
{
	m = (struct mm *)malloc(sizeof(struct mm));
	
	m->b = data;
	m->c = ch;
}

struct mm *s = 0xabcde;

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("%s: param\n", argv[0]);
		return -1;
	}		


	a = atoi(argv[1]);

	func1(&a);

	func2(a, "hello world");

	printf("a %d\n", a);
	printf("m->data %d, m->c %s\n", m->b, m->c);


	s->b = 0;
	
	

	return 0;	


}
