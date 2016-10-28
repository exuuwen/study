#include <stdio.h>
/* We can add a number label and specify a direction for jump target. (f: forward, b:backward) */

int main()
{
	int a = 100;
    int b = 20;
    int result;
    
    asm("cmp %1, %2\n\t"
    	"jge 0f\n\t"		//jump to forward label 0
    	"movl %1, %0\n\t"
    	"jmp 1f\n\t"		//jump to forward label 1
    	"0:\n\t"
		"movl %2, %0\n\t"
		"1:\n\t"
		:"=r"(result)
		:"r"(a), "r"(b));
		
	printf("the large value is %d\n", result);
	return 0;
}
