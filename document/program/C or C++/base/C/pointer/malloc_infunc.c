#include <stdio.h>

char *GetMemory3(int num)
{
    char *p = (char *)malloc(sizeof(char) * num);
    return p;
}

void GetMemory2(char **p, int num)
{
    *p = (char *)malloc(sizeof(char) * num);
}
void Test2(void)
{
    char *str = NULL;
     GetMemory2(&str,100);  
    strcpy(str, "hello");
    printf("str is %s ......\n",str);
    free(str);  
}

void Test3(void)
{
    char *str = NULL;
    str = GetMemory3(100);  
    strcpy(str, "hello");
    printf("str is %s ......\n",str);
    free(str);  
}

int main()
{
	Test2();
	Test3();
	return 0;
}