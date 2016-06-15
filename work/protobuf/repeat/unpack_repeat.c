#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "repeat.pb-c.h"

#define MAX_MSG_SIZE 4096

int main(int argc, char **argv)
{
	uint8_t buf[MAX_MSG_SIZE]; // Input data container for bytes
	char c;
	size_t i = 0;
    
	while (fread(&c, 1, 1, stdin) != 0)
	{
		if (i >= MAX_MSG_SIZE)
		{
			fprintf(stderr,"message too long for allocated buffer\n");
			return 1;
		}

		buf[i++] = c;
	}	

	printf("buf len %d\n", i);
	DataList *msg = data_list__unpack(NULL, i, buf);

	for (i=0; i<msg->n_data_list; i++)
		printf("%d\n", msg->data_list[i]->data);

	data_list__free_unpacked(msg, NULL);	

	return 0;
}
