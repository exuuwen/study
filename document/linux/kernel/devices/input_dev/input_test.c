#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>

int main(void)
{
	int fd,i;
	int ret;
	unsigned char buf[4];
	fd_set set_r;
	struct timeval  timeout;
	
	fd = open("/dev/input_fifo", O_RDONLY);
	if(fd<0)
	{
		perror("open bad");
		return -1;
	}
	printf("open dev ok.........\n");
	//sleep(28);
	memset(buf, 0, sizeof(buf));
	
	while(1)
	{
		timeout.tv_sec=10;
		timeout.tv_usec=0;
		FD_ZERO(&set_r);
		FD_SET(fd, &set_r);
		ret=select(fd+1, &set_r, 0, 0, &timeout);
		printf("the select ret is %d\n", ret);
		if(ret == -1)
		{
			perror("select bad");
			return -1;
		}
		else if(ret == 0)
			printf("select timeout\n");
		else 
		{
			if(FD_ISSET(fd,&set_r))
			{
				ret = read(fd,buf,sizeof(buf));
				if(ret<0)
				{
					perror("read bad");
					return -1;
				}
				else 
				{
					printf("read ok....\n");
					for(i=0; i<sizeof(buf); i++)
					printf("buf[%d] is %d\n", i,buf[i]);
				}
			}
			else
			printf("select other thing why ?\n"); 
		}
		
	}
	close(fd);
}
