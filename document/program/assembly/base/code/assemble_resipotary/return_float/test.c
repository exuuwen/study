#include <stdio.h>
float get_area(int); //must have this
int main()
{
	int ram = 10;
	float area ;

	area = get_area(ram);
	printf("result:%f\n", area);

	area = get_area(2);
	printf("result:%f\n", area);
	
	return 0;
}
