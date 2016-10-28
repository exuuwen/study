#include <stdio.h>
#include <stdlib.h>
#include <getopt.h> //getopt_long()头文件位置

int main(int argc, char** argv)
{
    const char *optstring="n:v";
    int c,deb1,index;
    struct option opts[]={{"username",required_argument,NULL,'n'},
                          {"version",no_argument,NULL,'v'},
                          {"debug",no_argument,&deb1,1},
                          {"wx",no_argument,NULL,2},
                          {"ztt",no_argument,NULL,3},
                          {0,0,0,0}};
    while((c=getopt_long(argc,argv,optstring,opts,&index))!=-1)
    {
        
        switch(c)
        {
        case 'n'://-n 或者 --username 指定用户名
            printf("username is %s\n",optarg);
            break;
        case 'v'://-v 或者--version,输出版本号
            printf("version is 0.0.1 \n");
            break;
        case 0://flag不为NULL
            printf("debug is %d\n",deb1);
            break;
        case '?'://选项未定义
            printf("?\n");
            break;
        default:
            printf("c is %d\n",c);
            break;
        }
    }
    return 0;
}

