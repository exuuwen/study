#include <stdio.h>

extern char __executable_start[];
extern char etext[], _etext[], __etext[];
extern char edata[], _edata[];
extern char end[], _end[];

int main()
{
	printf("estart:%x\n", __executable_start);
	printf("etext:%x %x %x\n", etext, _etext, __etext);
	printf("edate:%x %x\n", edata, _edata);
	printf("end:%x %x\n", end, _end);

	while(1);

	return 0;
}
