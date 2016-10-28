#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/if_vlan.h>
struct A
{
    char b;
    short a;
    double c;
    int d;
};

int main(int argc, char *argv[])
{
    printf("sizeof %ld, algin:%ld\n", sizeof(struct A), __alignof__(struct A));

    printf("sizeof %ld, algin:%ld\n", sizeof(long long), __alignof__(long long));
    return 0;
}
