#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

int main ()
{
  int keys_fd;
  int ret;
  struct input_event t;
  //int n=0;

  /*fd = open ("/dev/mymisc", O_RDONLY);
   if (fd < 0)
    {
      printf ("open /dev/misc device error!\n");
      
      return 0;
    }*/
  keys_fd = open ("/dev/input/event6", O_RDONLY);
  if (keys_fd < 0)
  {
      printf ("open /dev/input/event6 device error!\n");
      //close(fd);
      return 0;
  }
  printf("ok all is ok.................\n");
  
  while (1)
  {
	//printf("it is %d time...\n", n);
        //n++;
      if (read(keys_fd, &t, sizeof(t)) == sizeof(t))
      {
          if(t.type == EV_KEY && t.value == 1)
		printf("key :%d   value is :%d............\n", t.code, t.value);   	
      }
     else
     {
		printf("read error....\n");
		return -1;
     }
     
  }
  close (keys_fd);
  //close (fd);

  return 0;
}


























