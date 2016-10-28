#include <stdio.h>  
#include <limits.h>

#define _STR(s)     #s 
#define STR(s)     _STR(s)
#define CONS(a,b)  (int)(a##e##b) 

int main() 
{ 
    printf(STR(INT_MAX));           // 输出字符串"hello" 
    printf("\n%d\n", CONS(3, 3));  // 3e2 输出:3000 

    return 0; 
} 
/*
1. 我们使用#把宏参数变为一个字符串,用##把两个宏参数贴合在一起. 
用法: 
#include<stdio.h>  

#define STR(s)     #s 
#define CONS(a,b)  (int)(a##e##b) 

int main() 
{ 
    printf(STR(hello));           // 输出字符串"hello" 
    printf("\n%d\n", CONS(3, 3));  // 3e2 输出:3000 
    return 0; 
} 

2. 需要注意的是凡宏定义里有用'#'或'##'的地方宏参数是不会再展开.
#include <stdio.h>  
#include <limits.h>

#define STR(s)     #s 

int main() 
{ 
    printf(STR(INT_MAX));         
    return 0; 
} 

#./main
INT_MAX //不会被扩展

解决方法
#include <stdio.h>  
#include <limits.h>

#define _STR(s)     #s 
#define STR(s)     _STR(s)
*/