#include <stdio.h>
#include "htm.h"

int main()
{
	int i = 0;
	unsigned long balance = 0;
	unsigned long conflict = 0, retry = 0, capacity = 0, debug = 0, others = 0, nested = 0, explict = 0;
	unsigned stat;

	for (i=0; i<1000000; i++)
	{
again:
		stat = _xbegin();
		if(stat == _XBEGIN_STARTED){
            		balance++;
			if (i == 1000)
			{
				//abort become explicit
				_xabort(2);
			}
			_xend();
        	}
		else
		{
			if (stat & _XABORT_CONFLICT){
				conflict ++;
			}
			if (stat & _XABORT_CAPACITY){
				capacity ++;
			}
			if (stat & _XABORT_DEBUG){
				debug ++;
			}
			if (stat & _XABORT_RETRY){
				retry ++;
			}
			if (stat & _XABORT_NESTED){
				nested ++;
			}
			if (stat & _XABORT_EXPLICIT){
				explict ++;
				i++;
			}
			if (stat == 0){
				others ++;
			}
			goto again;
		}
	}

	printf("balance:%lu,coflict:%lu,retry:%lu,capacity:%lu,debug:%lu,other:%lu,nested:%lu,explict:%lu\n", balance, conflict, retry, capacity, debug, others, nested, explict);
}
