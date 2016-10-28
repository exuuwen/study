#include <stdio.h>

#define xchg(in, out)   asm("xchg%z1 %3,%0\n\t" : "=r"(in), "=r"(out) : "0"(in), "1"(out))  /*for register output variable, first copy the variable to register from stack, after the operations copy it back*/

//#define xchg(in, out)   asm("xchg%z1 %2,%0\n\t" : "=m"(in), "=r"(out) : "1"(out))   /* This is also ok! for memory output variable，operate the variable in address way, the implict input of first data can get from stack */

#define mxchg(m, out)   asm("xchg%z1 %2,%0\n\t" : "=m" (*(m)), "=r" (out) : "1" (out))

int main()
{
	int inputl = 0x11223344;
	int outputl = 0x44332211;

	xchg(inputl, outputl);

	printf("inputl:0x%x\n", inputl);
	printf("outputl:0x%x\n", outputl);

	short inputw = 0x1122;
	short outputw = 0x4433;

	xchg(inputw, outputw);

	printf("inputw:0x%x\n", inputw);
	printf("outputw:0x%x\n", outputw);

	char inputb = 0x11;
	char outputb = 0x44;

	xchg(inputb, outputb);

	printf("inputb:0x%x\n", inputb);
	printf("outputb:0x%x\n", outputb);

	int tmpl = 0x11223344;
	int *ml = &tmpl;
	int outl = 0x44332211;
	
	mxchg(ml, outl);

	printf("minputl:0x%x\n", tmpl);
	printf("moutputl:0x%x\n", outl);

	short tmpw = 0x1122;
	short *mw = &tmpw;
	short outw = 0x4433;
	
	mxchg(mw, outw);

	printf("minputw:0x%x\n", tmpw);
	printf("moutputw:0x%x\n", outw);

	short tmpb = 0x11;
	short *mb = &tmpb;
	short outb = 0x44;
	
	mxchg(mb, outb);

	printf("minputb:0x%x\n", tmpb);
	printf("moutputb:0x%x\n", outb);
	return 0;
	
}
