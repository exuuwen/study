#include <stdio.h>
#include <string.h>

int main()
{

	const char *first = strchr("hah/sss/www", '/');
	const char *last = strrchr("hah/sss/www", '/');

	const char *str = strstr("my.ucloud", ".uclou"); 

	if (first)
		printf("first: %s\n", first + 1);
	if (last)
		printf("last: %s\n", last + 1);

	if (str)
		printf("str: %s\n", str);
	return 0;
}

/*
first: sss/www
last: www
str: .ucloud
*/
