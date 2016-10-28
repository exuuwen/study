#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define DESCSIZE 11
#define HELPER_PID "HELPER_PID"
int main()
{
	char pid_env[DESCSIZE];
	pid_t pid;
	int ret;
	char *p;

	pid = getpid();

	ret = snprintf(pid_env, sizeof(pid_env), "%u", pid);
	assert((ret >= 0) && (ret < sizeof(pid_env)));

	ret = setenv(HELPER_PID, pid_env, 1);
	assert(ret == 0);

	p = getenv(HELPER_PID);
	assert(p != NULL);

	printf("HELPER_PID =%s\n", p);

	return 0;
}
