#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "repeat.pb-c.h"

int main(int argc, char **argv)
{
	int num, i;

	if (argc < 2)
	{
		printf("./repeat num1 num2 num3...\n");
		return 0;
	}	

	argv++;
	argc--;

	num = argc;

	Data **msg;
	msg = malloc(num * sizeof(*msg));
	
	for (i=0; i<num; i++)
	{
		msg[i] = malloc(sizeof(Data));
		data__init(msg[i]);
		msg[i]->data = atoi(argv[i]);
	}

	DataList list = DATA_LIST__INIT;
	list.data_list = msg;
	list.n_data_list = num;

	size_t len = data_list__get_packed_size(&list);
	uint8_t *buf = malloc(len);
	data_list__pack(&list, buf);
	
	fwrite (buf, len, 1, stdout);

	for (i=0; i<num; i++)
		free(msg[i]);	

	free(msg);

	return 0;
}
