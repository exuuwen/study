#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

#define FIFO "./my_t_fifo"

/*
in default the write is block
if it is setten non-block, it will return - with errno EXIO, if there is no reader
*/
int main(int argc, char *argv[])
{
    int fd;
    int write_byte = 0;
    int res;
    char sender_buf[20] = "hello world";  
	
    if (access(FIFO, F_OK) == -1)
    {
        res = mkfifo(FIFO, 0744);
        if (res != 0)
        {
            perror("can not make fifo");
            exit(1);		
        }		
    }
	
    printf("open the fifo use process %d\n", getpid());
	
    fd = open(FIFO, O_WRONLY | O_NONBLOCK);
    if (fd == -1)
    {
        perror("can not open");
        exit(1);
    }
    
    write_byte = write(fd, sender_buf, sizeof(sender_buf));
    if (write_byte == -1)
    {
        perror("can not write");
        exit(1);
    }
        
    printf("write %d data to fifo %s\n", write_byte, sender_buf);

    close(fd);

    exit(0);
}
