#include <stdio.h>
#include <assert.h>

int main(void)
{
	int ret;
	char *rval;
	char path[50];

	ret = chdir("tmp");
	assert(ret == 0);
	printf("chdir to /tmp succeeded\n");
	
	rval = getcwd(path, 50);
	assert(rval != NULL);

	printf("cwd = %s\n",path);

	return;
}



