#include <stdio.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/stat.h>

int main()
{
    struct rlimit rlim;

    rlim.rlim_cur = 20;
    rlim.rlim_max = 20;
    if (setrlimit(RLIMIT_NOFILE, &rlim) == -1) 
    {
        perror("setrlimit:");
        return -1;
    }   

    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1) 
    {
        perror("setrlimit:");
        return -1;
    }   

    printf("rlim.rlim_cur:%ld, rlim.rlim_max:%ld\n", rlim.rlim_cur, rlim.rlim_max);

    int fd, cn = 0;
    while(1)
    {
        fd = open("a", O_CREAT | O_APPEND | O_RDWR, 0644);	
        if (fd < 0)
        {
            perror("open:");
            printf("open file nu: %d\n", cn);
            return -1;
        }
        cn++;
    }

    return 0;
}
