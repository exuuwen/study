#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/capability.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
	int ret, fd;
	char buf[10];
	cap_t caps;
	pid_t ruid, euid, suid;
	/*caps = cap_get_proc();
	if(!caps)
	{
		perror("get proc");
		return EXIT_FAILURE;
	}

	printf("unrust: running with caps %s\n", cap_to_text(caps, NULL));

	printf("in exev the  child  pid is %d\n",getpid());
        printf("in exev the father's pid is  %d\n",getppid());
	getresuid(&ruid, &euid, &suid);
	fprintf(stderr,"regain Hi from the sandbox! I'm pid=%d, uid=%d, euid=%d, rudi=%d, suid=%d\n",
	getpid(), getuid(), geteuid(), ruid, suid);*/
	
	printf("in the untrust\n");

	fd = open("./test.txt", O_RDWR);
	assert(fd > 0);
	printf("open ok\n");	

	ret = read(fd, buf, 9);
	buf[9] = 0;
	printf("read ok:%s!\n", buf);

	
	close(fd);
	
	return 0;
}
