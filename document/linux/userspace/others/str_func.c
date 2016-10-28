#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(void)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char *token;
	int j;
	char *delim = " ";
	char *str1;

	const char *first = strchr("hah/sss/www", '/');
	const char *last = strrchr("hah/sss/www", '/');
	const char *str = strstr("my.ucloud", ".uclou"); 

	if (first)
		printf("first: %s\n", first + 1);
	if (last)
		printf("last: %s\n", last + 1);
	if (str)
		printf("str: %s\n", str);

	/*
	first: sss/www
	last: www
	str: .ucloud
	*/


	fp = fopen("/home/wxztt/file", "r");
	if (fp == NULL)
    	exit(EXIT_FAILURE);

	while ((read = getline(&line, &len, fp)) != -1) {
		uint32_t key;
		uint64_t comp_type;
		//printf("%s", line);
		for (j = 0, str1 = line; ; j++, str1 = NULL) {
			token = strtok(str1, delim);
			if (token == NULL)
				break;
			if (j == 0)
            {
                key = strtoul(token, NULL, 0);
				if (key > 1000)
				{
					printf("key fail %lu\n", key);
					break;
				}
            }
            else if (j == 1)
            {
                comp_type = strtoul(token, NULL, 0);
				if (comp_type > 10)
				{
					printf("comp_type fail %lu\n", comp_type);
					break;
				}
            }
		}

		if (j == 2)
		{
			printf("hahaha all ok %u %lu\n", key, comp_type);
		}
	}

	free(line);
	exit(EXIT_SUCCESS);
}

