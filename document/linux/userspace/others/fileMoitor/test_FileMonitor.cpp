#include "FileMonitor.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		fprintf(stderr, "%s file_full_path\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	FileMonitor fm(argv[1]);

	bool modified;

	while(1)
	{
		modified = fm.hasBeenModified();
		if (modified == true)
			printf("changed\n");
		else
			printf("non changed\n");
		sleep(5);
	}

	return 0;
}

