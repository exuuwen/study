#include <stdio.h>
#include <stdint.h>

uint8_t addr[2] = {0x12, 0x34};

union A {
	char ch;
	uint32_t i;
};

struct B {
	uint8_t  a:4,
			b:4;
};

int main()
{
	printf("0x%x\n", *(uint16_t *)addr);
	//for little endian should be 0x3412
	
	union A a;
	a.i = 1;
	
	printf("a.ch: %d\n", a.ch);
	//for little endian should be 0x1

	struct B b;
	b.a = 1;
	b.b = 2;

	printf("0x%x\n", *(uint8_t *)&b);
	/*00100001*/
}

/*
for 0x0a0b0c0d

Write Integer for Big Endian System
byte  addr       0         1       2        3
bit  offset  01234567 01234567 01234567 01234567
     binary  00001010 00001011 00001100 00001101
        hex     0a       0b      0c        0d

Write Integer for Little Endian System
byte  addr       0        1         2       3    
bit  offset   76543210 76543210 76543210 76543210
     binary   00001101 00001100 00001011 00001010
        hex      0d      0c        0b       0a   
*/
