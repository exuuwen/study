#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>/*getpagesize*/
#include <errno.h>
#include <stdlib.h>

#define CHAR_DEV_PATH        "/dev/test"
int static_one ;
int main(int argc, char **argv)
{
        int fd = -1;
        unsigned int vaddr = 0;
        int retval = 0;
	unsigned int vaddr2 = 0;
	
     

        vaddr = (unsigned long)malloc(1024*10*1024);
	vaddr2 = (unsigned long)malloc(1024*20*1024);
	printf("retval addr is 0x%lx\n", &retval);
	printf("oneaddr is 0x%lx\n",&static_one);
        printf("vaddr  is  0x%lx\n", vaddr);
	printf("vaddr2  is  0x%lx\n", vaddr2);
	printf("main_addr  is  0x%lx\n", main);
        /*open the device*/
        fd = open(CHAR_DEV_PATH, O_RDWR);

        if ( fd < 0 )
        {
                printf("error when open device file:%s\n", strerror(errno));
                retval = -1;
                goto error;
        }


        error:
        if ( fd != -1 )
        {
                close(fd);
        }
        return retval;
}

