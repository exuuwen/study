#include <stdio.h>
#include <string.h>

double get_cpu_usage(void)
{
    double total;
    double usage;
    int user;
    int nice;
    int system;
    int idle;
    unsigned char cpu[8];
    unsigned char text[256];
    FILE *fp;
    fp = fopen("/proc/stat", "r");
        fread(text, 1, sizeof(text), fp);
    if (strstr(text, "cpu"))
    {
        sscanf(text, "%s %d %d %d %d", cpu, &user, &nice, &system, &idle);
    }
 
    printf("%d %d %d %d\n", user, nice, system, idle);
 
    fclose(fp);
    total = (user + nice + system + idle);
        usage = (user + nice + system )/total ;
    return usage;
}





