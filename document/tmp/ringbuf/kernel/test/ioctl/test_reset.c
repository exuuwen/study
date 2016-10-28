#include <limits.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../../ringbuf_ioctl.h"

#define RB_DEV_ALL_RAW "/dev/rb_0_raw"

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		fprintf(stderr, "%s tag_name\n", argv[0]);
		return -1;
	}
		
	int fd_raw = open(RB_DEV_ALL_RAW, O_RDWR);
	if (fd_raw < 0)
	{
		perror("open:");
		return -1;
	}
	
	Tag tag;
	bzero(&tag, sizeof(Tag));
	
	strcpy(tag.tag_name, argv[1]);
	printf("tag:%s\n", tag.tag_name);

	int ret = ioctl(fd_raw, RB_IOCSETTAG, &tag);

	if (ret != 0)
	{
		perror("ioctl RB_IOCSETTAG:");
		return -1;
	}

	char dev[50];
	bzero(dev, sizeof(dev));
	sprintf(dev, "/dev/rb_%u_raw", tag.tag_no);
		
	printf("open dev name: %s\n", dev);

	int fd = open(dev, O_RDWR);
	if (fd < 0)
	{
		perror("open2:");
		return -1;
	}

	int i;
	char buf[200];
	bzero(buf, sizeof buf);

	for(i=0; i<2; i++)
	{
		sprintf(buf, "%s %d\n", tag.tag_name, i);
		ret = write(fd, buf, strlen(buf));	
		printf("write %d ok\n", ret);
		bzero(buf, sizeof buf);
		sleep(2);
	}

	close(fd);/* should close*/
	if (argc >= 3)
	{
		/*should fail*/
		ret = read(fd_raw, buf, sizeof(buf) - 1);
		printf("read %d: %s\n", ret, buf);
	}
		


	ret = ioctl(fd_raw, RB_IOCRESET, 0);
	if (ret != 0)
	{
		perror("ioctl RB_IOCRESET:");
		return -1;
	}

	bzero(buf, sizeof buf);

	scanf("%d", &i);

	for(i=0; i<2; i++)
	{
		sprintf(buf, "%s %d\n", tag.tag_name, i);
		ret = write(fd_raw, buf, strlen(buf));	
		printf("after reset write %d ok\n", ret);
		bzero(buf, sizeof buf);
		sleep(2);
	}

	ret = read(fd_raw, buf, sizeof(buf) - 1);
	printf("after reset read %d: %s\n", ret, buf);

	
	close(fd_raw);

	return 0;	
}
