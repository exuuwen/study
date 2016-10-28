#include <stdio.h>

void MOVE(int count,char source,char dest,char temp)
{
	if(count==1)
		printf("move the %c to %c\n",source,dest);
	else 
	{
		MOVE(count-1,source,temp,dest);
		MOVE(1,source,dest,temp);
		MOVE(count-1,temp,dest,source);
	}
}

void main()
{
	int  count;
	count=3;
	MOVE(count,'A','B','C');

}

