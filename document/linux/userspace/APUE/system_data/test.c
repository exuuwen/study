#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>

/*
//the optimizations don't affect the global, static, and volatile variables; 
their values after the longjmp are the last values that they assumed. 
When we enable optimization, both autoval and regival go into registers
so the auto and register will not be stored.
$ gcc -o test testjmp.c               compile without any optimization
    $ ./test
    in f1():
    globval = 95, autoval = 96, regival = 97, volaval = 98, statval = 99
    after longjmp:
    globval = 95, autoval = 96, regival = 97, volaval = 98, statval = 99
    $ cc -O testjmp.c            compile with full optimization
$ gcc -o test testjmp.c -O2 
    in f1():
    globval = 95, autoval = 96, regival = 97, volaval = 98, statval = 99
    after longjmp:
    globval = 95, autoval = 2, regival = 3, volaval = 98, statval = 99
*/

static void f1(int, int, int, int);
static void f2(void);

static jmp_buf jmpbuffer;
static int     globval;

int
main(void)
{
     int             autoval;
     register int    regival;
     volatile int    volaval;
     static int      statval;

     globval = 1; autoval = 2; regival = 3; volaval = 4; statval = 5;

     if (setjmp(jmpbuffer) != 0) {
         printf("after longjmp:\n");
         printf("globval = %d, autoval = %d, regival = %d,"
             " volaval = %d, statval = %d\n",
             globval, autoval, regival, volaval, statval);
         exit(0);
     }

     /*
      * Change variables after setjmp, but before longjmp.
      */
     globval = 95; autoval = 96; regival = 97; volaval = 98;
     statval = 99;

     f1(autoval, regival, volaval, statval); /* never returns */
     exit(0);
}

static void
f1(int i, int j, int k, int l)
{
    printf("in f1():\n");
    printf("globval = %d, autoval = %d, regival = %d,"
        " volaval = %d, statval = %d\n", globval, i, j, k, l);
    f2();
}
static void
f2(void)
{
    longjmp(jmpbuffer, 1);
}









