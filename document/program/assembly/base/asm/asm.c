#include <stdio.h>
/void func1
{
	unsigned short cs = 12;
    	unsigned short a = 1;
	unsigned short b = 8,c,d,e,f;
	//&:means use a single register.
    	asm ("movw $55,%1\n\t"
         :"=&r"(cs), "=&r"(a)	// output data must be used
	     :"r"(b));		// input data permits unused
																//a:5		  
    	printf("cs %d\n", cs);	//cs:1
	printf("a %d\n", a);	//a:55
	printf("b %d\n", b);	//b:8
}

void func2()
{
	unsigned short cs = 3;
    	unsigned short a = 1;
	//output must be get the value then can be used as a input
	//input can be used as output, but the real data will not be changed
	//&:means use a single register.
	//every output data should be used.
    	asm ("addw $55,%1\n\t"
	         "movw %1,%0\n\t"
                 :"=&r"(cs)
	         :"r"(a));		//for register output variable, first copy the variable to register from stack, after the operations copy it back
				// for register input variable, first copy the variable to register from stack, but not copy back, so the input variable will not be changed!						  
    	printf("cs %d\n",cs);	//cs:56
	printf("a %d\n",a);	//a:1
}
void func3()
{
	unsigned short cs = 3;
    	unsigned short a = 1;
	//input can be used as output, if it is a register variable the real data will not be changed, the memory variable will be changed!
    	asm ("movw $55,%1\n\t"
	         "movw %1,%0\n\t"
    	         :"=r"(cs)
	         :"m"(a));	//for memory output variable，operate the variable in address way
			//for memory input variable，operate the variable in address way, so the value will be changed!
												//a:55
			  
   	printf("cs %d\n",cs);     //cs:55
	printf("a %d\n",a);        //a:55
}

void func4()
{
    	unsigned short cs = 3;
    	unsigned short a = 1;
	unsigned short b =10,c,d,e,f;
	//output must be get the value then can be used as an input, or the value is bad!
	//input can be used as output, but the real data will not be changed
	//&:means use a single register.
	//every output data should be used.
    	asm ("movw $55,%0\n\t"
	         "addw %2,%1\n\t"	//for some implicit input operation, the dest register must be specified as input variable. So %1 is a.
                 :"=&r"(cs), "=r"(a)
	         :"r"(cs),"1"(a));	//so input cs and the output cs is not the same register! so cs:0x37, a:0x4	//if input list:"0"(cs),"1"(a))  cs:0x37 , a:0x38
					    					  
    	printf("0x%x\n",cs);
	printf("%x\n",a);
	
}

void func5()
{
    	unsigned short cs = 3;
    	unsigned short a = 1;
	unsigned short b,c,d,e,f;
	//output must be get the value then can be used as a input
	//input can be used as output, but the real data will not be changed
	//&:means use a single register.
	//every output data should be used.
   	asm ("movw $55,%0\n\t"
	         "addw %0,%1\n\t"
                 :"=&r"(cs), "=r"(a)
	         :"1"(a),"r"(b),"r"(c),"r"(d),"r"(e));	// "a" must can be input for addw, and must be in the same register
  						// no more register can be used, "=r"(a),"r"(b),"r"(c),"r"(d),"r"(e), "r"(f) will be wrong!
    	printf("%x\n",cs);
	printf("%x\n",a);	
}

