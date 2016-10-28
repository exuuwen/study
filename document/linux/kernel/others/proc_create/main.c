#include <stdio.h>
#include <sys/mman.h> 
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <time.h>
#include <sys/resource.h>


void main(){
	int proc_fd;
	///int proc_rpkfifo_size_fd;
	unsigned long int key;
	FILE *file;
	char proc_buf[30];
	char proc_buf1[30]="1221111";
	char proc_buf2[30];
	proc_fd = open("/proc/wx_procdir/wx_key",O_RDWR);
	//file=fopen("/proc/wx_procdir/wx_key","w");
	if(proc_fd != -1) {
		read(proc_fd,proc_buf,sizeof(proc_buf));
		sscanf(proc_buf,"%lx",&key);
		printf("the key is 0x%lx\n",key);
		
		if(write(proc_fd,proc_buf1,sizeof(proc_buf1))==-1)
		printf("write error\n");	
		//fprintf(proc_fd,"%lx",0x23451);
		//fflush(file);
		fdatasync(proc_fd);
		sleep(1);
		if(read(proc_fd,proc_buf2,sizeof(proc_buf2))==-1)
		printf("read error\n");
		sscanf(proc_buf2,"%lx",&key);
	
		printf("now the key is 0x%lx\n",key);
		close(proc_fd);
	}
	else{
		printf("open  proc failed\n");
		close(proc_fd);
	}
	
	
}
