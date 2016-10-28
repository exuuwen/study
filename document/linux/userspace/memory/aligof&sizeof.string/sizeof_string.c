#include <stdio.h>

#define RB_UNIQUE_RAW_HEADER_HEAD "~~"

/* Note -1, the trailing null char is not a part of the header */
#define RB_RAW_HEADER_SIZE (sizeof(RB_UNIQUE_RAW_HEADER_HEAD)-1)

int main()
{
	printf("len:%d\n", RB_RAW_HEADER_SIZE);
	return 0;
}
