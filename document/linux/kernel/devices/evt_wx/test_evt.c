#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

int main()
{
	int ret = 0;
	int fd = -1;
	struct input_event event;
	int len;
	
	if ((fd = open("/dev/input/event5", O_RDWR)) < 0) {
		ret = -1;
		printf("open /dev/input/event5 error\n");
		return ret;
	}
	
	while(1)
	{
		len = read(fd,(unsigned char*)&event,sizeof(struct input_event));
		printf("now read the key\n");
		if(len != sizeof(struct input_event))
			break;
		if(event.type != EV_KEY && event.type != EV_SYN)
			printf("event is %d!!!\n",event.type);
		else
		{
			if(event.type == EV_KEY)
			{

					printf("receive evt key_code is 0x%x, key_value s 0x%x\n",event.code,event.value);

			}
		}
	}
	

	close(fd);
	return ret;
}
