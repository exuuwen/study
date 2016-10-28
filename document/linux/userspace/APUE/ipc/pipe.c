#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

/* streams read and block in default
when it is setten to non-block and there is no data in the pipe the reader will return Resource temporarily unavailable immediately
if there is no data in the pipe and the partner close pipes[1], the reader will return 0 immedately
if the partner close pipe[0], the writer write will incur SIGPIPE
*/
int main(int argc, char *argv[])
{
    pid_t  pid;
    int temp;
    int pipes[2];
    char buf_w[15];
    char buf_r[50];

    if (pipe2(pipes, /*O_NONBLOCK*/ 0) == -1)
    {
        perror("can not pipe:");
        exit(1);
    } 

    if ((pid = fork()) == -1)
    {
        perror("can not fork:");
        close(pipes[0]);
        close(pipes[1]);
        exit(1);
    }
    else if (pid > 0)
    {
        close(pipes[0]);
        printf("start to write the data to pipe\n");
        int i = 0;
	while (i < 10)
        {
            sprintf(buf_w, "%s %d", "hello work", i);
            printf("buf_w:%s\n", buf_w);
            if (write(pipes[1], buf_w, strlen(buf_w)) == -1)
            {
                perror("write error:");
                close(pipes[1]);
                exit(1);
            }
            i++;
        }

	wait();
        close(pipes[1]);
    }
    else if(pid == 0)
    {
        int n, i = 0;
        close(pipes[1]);
        sleep(1);
        printf("start to read the data from pipe\n");
	while (i < 10)
	{
	    if((n = read(pipes[0], buf_r, 49)) == -1)
            {
                perror("read error:");
                close(pipes[0]);
                _exit(1);
            }
		
            buf_r[n] = '\0';
            i++;
            printf("read %d data: %s\n", n, buf_r);		
            sleep(1);
        }

        close(pipes[0]);
    }	

    return  0;
}
