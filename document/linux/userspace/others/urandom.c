#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
 
/* 从min和max中返回一个随机值 */
 
int random_number(int min, int max)
{
    static int dev_random_fd = -1;
    char *next_random_byte;
    int bytes_to_read;
    unsigned random_value;
     
    assert(max > min);
     
    if (dev_random_fd == -1)
    {
        dev_random_fd = open("/dev/urandom", O_RDONLY);
        assert(dev_random_fd != -1);
    }

    next_random_byte = (char *)&random_value;
    bytes_to_read = sizeof(random_value);

    /* 因为是从/dev/random中读取，read可能会被阻塞，一次读取可能只能得到一个字节，
     * 循环是为了让我们读取足够的字节数来填充random_value.
     */
    do
    {
        int bytes_read;
        bytes_read = read(dev_random_fd, next_random_byte, bytes_to_read);
        bytes_to_read -= bytes_read;
        next_random_byte += bytes_read;
    }while(bytes_to_read > 0);

    return min + (random_value % (max - min + 1));
}

int main()
{
    int i;

    for (i=0; i<10; i++)
    {
        printf("%d times get random bettwent 20 and 100, %d\n", i, random_number(20, 100));
    }

    return 0;
	
}
