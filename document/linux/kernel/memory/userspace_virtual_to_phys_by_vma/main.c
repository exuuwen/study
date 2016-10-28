#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>/*getpagesize*/
#include <errno.h>
#include <stdlib.h>

struct ioctl_arg
{
        pid_t pid;
        unsigned long vaddr;
};
#define CHAR_DEV_PATH        "/dev/globalmem"
#define KMAP_IOC_MAGIC        'q'
#define KMAP_IOC_SET_PID_AND_VADDR        _IOW(KMAP_IOC_MAGIC, 1, char *)
int main(int argc, char **argv)
{
        int fd = -1;
        pid_t pid = 0;
        unsigned int vaddr = 0;
        int result = 0;
        int page_size =0;
        struct ioctl_arg arg;
        int retval = 0;

        memset(&arg, 0, sizeof(struct ioctl_arg));

        /*get page size*/
        //page_size = getpagesize();

        /*get pid*/
        pid = getpid();

        vaddr = (unsigned long)malloc(20);//
        memset((char *)vaddr, 0, 20);
        strcpy((char *)vaddr, "helddd wx hhjbjbssss ");

        /*assemble arg*/
        printf("The pid and the vaddr is:%d, %s pos is %lx\n", pid, vaddr,vaddr);
        arg.pid = pid;
        arg.vaddr = vaddr;

        /*open the device*/
        fd = open(CHAR_DEV_PATH, O_RDWR);

        if ( fd < 0 )
        {
                printf("error when open device file:%s\n", strerror(errno));
                retval = -1;
                goto error;
        }

        /*pass the arg to kernel*/
        result = ioctl(fd, KMAP_IOC_SET_PID_AND_VADDR, &arg);
        if ( result )
        {
                printf("error when ioctl:%s\n", strerror(errno));
		goto error;
        }

        printf("after ioctl we get the memory:%s... pos is %lx\n", (char *)vaddr,vaddr);

        error:
        if ( fd != -1 )
        {
                close(fd);
        }
        return retval;
}

