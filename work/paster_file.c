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
                uint32_t a = 0, b = 0, c = 0, d = 0;
                int ret = sscanf(token, "%u.%u.%u.%u", &a, &b, &c, &d); 
                if (ret == 0 || ret == EOF) 
                {
                    printf("sccanf error %s\n", token); 
                    break;
                } 
                key = (a<<24)|(b<<16)|(c<<8)|d;
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

