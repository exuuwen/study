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
in default the reader is block
if it is setten non-block, reader will return 0 immediately if no writer here
if there are n reader, only one can be the buf from the fifo, the others return 0
*/
int main(int argc, char *argv[])
{
    int fd;
    int read_byte = 0;
    char receiver_buf[100];  

    memset(receiver_buf, '\0', sizeof(receiver_buf));
    printf("open the fifo process %d\n", getpid());

    fd = open(FIFO, O_RDONLY);
    if (fd == -1)
    {
        perror("can not open");
        exit(1);
    }

    read_byte = read(fd, receiver_buf, sizeof(receiver_buf));
    if (read_byte == -1)
    {
        perror("can not read");
        exit(1);
    }
    
    printf("receive the %d data from fifo is %s\n", read_byte, receiver_buf);
    
    close(fd);

    exit(0);
}
