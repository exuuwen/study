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
		
	int fd = open(RB_DEV_ALL_RAW, 0);
	if (fd < 0)
	{
		perror("open:");
		return -1;
	}
	
	Tag tag;
	bzero(&tag, sizeof(Tag));
	
	strcpy(tag.tag_name, argv[1]);
	printf("tag:%s\n", tag.tag_name);

	int ret = ioctl(fd, RB_IOCSETTAG, &tag);
	close(fd);
	if (ret != 0)
	{
		perror("ioctl RB_IOCSETTAG:");
		return -1;
	}

	int tag_no = 0;
	char dev[50];
	bzero(dev, sizeof(dev));
	sprintf(dev, "/dev/rb_%u_raw", tag.tag_no);
		
	printf("dev name: %s\n", dev);

	fd = open(dev, O_RDWR);
	if (fd < 0)
	{
		perror("open2:");
		return -1;
	}

	int i;
	char buf[600];
	bzero(buf, sizeof buf);
	
	ret = read(fd, buf, sizeof(buf) - 1);
	printf("read %d: %s", ret, buf);
	bzero(buf, sizeof buf);

	if(scanf("%d", &i))
	{
		ret = read(fd, buf, sizeof(buf) - 1);
		printf("read %d: %s", ret, buf);
	}

	bzero(buf, sizeof buf);
	if(scanf("%d", &i))
	{
		ret = read(fd, buf, sizeof(buf) - 1);
		printf("read %d: %s\n", ret, buf);
	}
	
	close(fd);

	return 0;	
}
