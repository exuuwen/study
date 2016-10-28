#include <stdio.h>

#define RB_DEV_ALL_RAW "/dev/rb_0_raw"
int main()
{


    FILE* f=fopen(RB_DEV_ALL_RAW,"r");
    if (f) {
	fclose(f);
	printf("open okay\n");
    } else {
	printf("open fail\n");
	return 0;
    }

	return 0;
}
