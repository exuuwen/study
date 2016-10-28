#include <stdio.h>
#include <string.h>

#include "tun_tap.h"

void usage(const char* func)
{
    printf("%s create|delete tap|tun devname\n", func); 
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        usage(argv[0]);
        return 0;
    }

    uid_t uid = getuid();

    if (strcmp(argv[1], "create") == 0)
    {
        if (strcmp(argv[2], "tap") == 0)
           tap_create(argv[3], &uid, NULL);
        else if (strcmp(argv[2], "tun") == 0)
           tun_create(argv[3], &uid, NULL);
        else
           usage(argv[0]);
    }
    else if(strcmp(argv[1], "delete") == 0)
    {
        if (strcmp(argv[2], "tap") == 0)
           tap_delete(argv[3]);
        else if (strcmp(argv[2], "tun") == 0)
           tun_delete(argv[3]);
        else
           usage(argv[0]);
    }
    else
    {
        usage(argv[0]);
    }

    return 0; 
    
}

