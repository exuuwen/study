#include <stdio.h>

int A;

#include <stdio.h>
//���ص��Ǿֲ������ĵ�ַ���õ�ַλ�ڶ�̬��������ջ��
char *s1()
{
char p[]="Hello world1!";
printf("in s1 p=%p\n", p);
printf("in s1: string's address: %p\n", &("Hello world!"));
return p;
}
//���ص����ַ��������ĵ�ַ���õ�ַλ�ھ�̬������
char *s2()
{
char *q="Hello world2!";
printf("in s2 q=%p\n", q);
printf("in s2: string's address: %p\n", &("Hello world!"));
return q;
}
//���ص��Ǿ�̬�ֲ������ĵ�ַ���õ�ַλ�ھ�̬������
char *s3()
{
static char r[]="Hello world3!";
printf("in s3 r=%p\n", r);
printf("in s3: string's address: %p\n", &("Hello world!"));
return r;
}
int main()
{
char *t1, *t2, *t3;
t1=s1();
t2=s2();
t3=s3();
printf("in main:");
printf("p=%p, q=%p, r=%p\n", t1, t2, t3);
printf("%s\n", t1);
printf("%s\n", t2);
printf("%s\n", t3);
return 0;
}
