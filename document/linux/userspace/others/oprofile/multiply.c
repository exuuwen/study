#include <stdio.h>
int fast_multiply(x,  y)
{
    return x * y;
}
 
int slow_multiply(x, y)
{
    int i, j, z;
    for (i = 0, z = 0; i < x; i++)
        z = z + y;
    return z;
}
int test(x, y)
{
    int i, j, z;
    for (i = 0, z = 0; i < x; i++)
        z = z + y;
    return z;
}
 
int main(int argc, char *argv[])
{
    int i, j;
    int x, y, z;
    for (i = 0; i < 200; i ++) {
        for (j = 0; j <  300; j++) {
            x = fast_multiply(i, j);
            y = slow_multiply(i, j);
			z = test(i, j);
        }
    }
    printf("x=%d, y=%d, z=%d\n", x, y, z);
    return 0;
}
