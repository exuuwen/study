#include <stdio.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <sys/types.h>  
#include <sys/wait.h>  
#include <time.h>  
#include <signal.h>  
#include <sys/param.h>  
#include <sys/stat.h>  


void init_daemon(void)
{  
    pid_t pid;  
    int i;  
    pid = fork();  
    
    if(pid > 0)
    { 
        exit(0);  
    }
    else if(pid < 0 )
    {  
        perror("fork fail:");  
        exit(1);  
    }
       
    setsid();
    pid = fork();
    if(pid > 0)
    { 
        exit(0);  
    }
    else if(pid < 0 )
    {  
        perror("fork2 fail:");  
        exit(1);  
    }

    chdir("/tmp");
    umask(0);
    for(i=0; i<NOFILE; ++i)
        close(i);  

    return ;  
} 

int main(void)
{  
    FILE *fp;  
    time_t t;  
  
    init_daemon();  
    //daemon(0, 0);

    while(1)
    {  
        sleep(5);  
        if( (fp=fopen("log.txt", "a+")) >= 0)
        {
            t = time(0);  
            fprintf(fp, "damon test time:%s", asctime(localtime(&t)));  
            fclose(fp);  
            printf("hahaha\n");
        } 
         
    }  
    return 1;  
}  
