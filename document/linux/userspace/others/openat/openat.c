#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
int main()
{
    int dfd, ffd;
    dfd = open("/home/exuuwen/exuuwen/tmp", O_RDONLY);
    if(dfd < 1)
    {
        perror("open home");
        return -1;
    }
    ffd = openat(dfd, "test1.txt", O_CREAT | O_RDWR, 0775);
    if(ffd < 1)
    {
        perror("open new file");
        return -1;
    }

    close(dfd);
    close(ffd);

    return 0;
}
