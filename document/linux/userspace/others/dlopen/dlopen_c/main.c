#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char **argv)
{
        void* pdlhandle;
        char* pszerror;
	int *g_count;

        int (*mytest)();

        pdlhandle = dlopen("./libtest.so", RTLD_LAZY);
        pszerror = dlerror();
        if (0 != pszerror)
        {
            printf("%s", pszerror);
            return 1;
        }

        mytest = dlsym(pdlhandle, "test");
        pszerror = dlerror();
        if (0 != pszerror)
        {
            printf("%s", pszerror);
            return 1;
        }

        g_count = dlsym(pdlhandle, "g_count");
        pszerror = dlerror();
        if (0 != pszerror)
        {
            printf("%s", pszerror);
            return 1;
        }

        mytest();   //defined in test.c (libtest.c)
        printf("[%s %s]: g_count=%d\n", __FILE__, __FUNCTION__, *g_count); 
	*g_count = 20;
        mytest();   //defined in test.c (libtest.c)

        dlclose(pdlhandle);
        return 0;
}

