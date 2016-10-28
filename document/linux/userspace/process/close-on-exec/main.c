#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void)
{
	int fd, pid, ret;
	char buffer[20];

	fd = open("wo.txt", O_RDONLY /*| O_CLOEXEC*/);
	assert(fd > 0);

	int flag = fcntl(fd, F_GETFD);
	assert(flag != -1);
	flag |= FD_CLOEXEC;

	ret = fcntl(fd, F_SETFD, flag); // if don't set it, the test_exec can read the fd
	assert(ret == 0);

	printf("set FD_CLOEXEC flag success\n");

	pid = fork();
	if(pid == 0)
	{
		char child_buf[3];
		memset(child_buf, 0, sizeof(child_buf));
		ssize_t bytes = read(fd, child_buf, sizeof(child_buf) - 1);
		printf("child, bytes:%ld,%s\n", bytes, child_buf);

		
		char fd_str[5];
		memset(fd_str, 0, sizeof(fd_str));
		int dup_fd = fd;//dup(fd);  
		/*if dup a new fd, the test_exec can read it successfully
		the close-on-exec flag applies to file descriptors, not open file descriptions. the new fd from dup did not share the file descriptor flags */
		sprintf(fd_str, "%d", dup_fd);
		int ret = execl("./test_exec", "test_exec", fd_str, NULL);
		if(-1 == ret)
		{
			perror("ececl fail:");
			return -1;
		}

	}        
	else
	{
		waitpid(pid, NULL, 0);
		memset(buffer, 0, sizeof(buffer));
		ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
		printf("parent, bytes:%ld, %s\n", bytes, buffer);
	}

	return 0;
}

