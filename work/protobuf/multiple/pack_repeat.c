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

	StatsData **data;
	data = malloc(num * sizeof(*data));
	
	for (i=0; i<num; i++)
	{
		data[i] = malloc(sizeof(StatsData));
		stats_data__init(data[i]);
		data[i]->num = atoi(argv[i]);
	}

	RecordStatsRequest *body = malloc(sizeof(*body));
	record_stats_request__init(body);
	body->stats_data_list = data;
	body->n_stats_data_list = num;

	Head *head = malloc(sizeof(*head));
	head__init(head);
	head->version = 100;

	UMessage  msg = UMESSAGE__INIT;
	msg.body = body;
	msg.head = head;
	
	size_t len = umessage__get_packed_size(&msg);
	uint8_t *buf = malloc(len);
	umessage__pack(&msg, buf);

	
	fwrite (buf, len, 1, stdout);

	for (i=0; i<num; i++)
		free(data[i]);	

	free(data);

	return 0;
}
