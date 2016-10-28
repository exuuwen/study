#include <stdio.h>
double get_area(double,int);
int main()
{
	int width = 10;
	double len = 3.222;
	double result;
	
	result = get_area(len, width);
	printf("result:%g\n", result);
	
	return 0;
}
