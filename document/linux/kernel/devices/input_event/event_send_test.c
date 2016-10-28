#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include "dvb_event.h"
int main (int argc, char** argv)
{
  int fd;
  int ret;
  unsigned char input=0;

   input=atoi(argv[1]);
   fd = open ("/dev/mymisc", O_RDONLY);
   if (fd < 0)
    {
      printf ("open /dev/misc device error!\n");
      
      return -1;
    }
  //while(input < 8)
 
   sleep(3);
   printf("input data is %d\n", input);
   ret = ioctl(fd, SEND_EVENT, &input);
   if(ret < 0)
   perror("ioctl bad");
   //input++;
   	
 
 close(fd);
}
