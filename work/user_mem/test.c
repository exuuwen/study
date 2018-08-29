#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
        char *pshare = (char *)calloc(1, 32);
        strcpy(pshare, "hahahhahah wx wx!");
        while(1) {
                sleep(1);
                printf("pid:%d user, info:%s, addr:%lu\n", getpid(), pshare, pshare);
        }
}

