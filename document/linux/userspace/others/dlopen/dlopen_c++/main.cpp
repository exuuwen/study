#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char **argv)
{
        void* pdlhandle;
        char* pszerror;
	int *g_count, *count;

	typedef int (*test)();
        int (*mytest)();

        pdlhandle = dlopen("./libtest.so", RTLD_LAZY);
        pszerror = dlerror();
        if (0 != pszerror)
        {
            printf("%s", pszerror);
            return 1;
        }

        mytest = (test)dlsym(pdlhandle, "test");
        pszerror = dlerror();
        if (0 != pszerror)
        {
            printf("%s\n", pszerror);
            return 1;
        }

        g_count = (int*)dlsym(pdlhandle, "g_count");
        pszerror = dlerror();
        if (0 != pszerror)
        {
            printf("%s\n", pszerror);
            return 1;
        }

        count = (int*)dlsym(pdlhandle, "count");
        pszerror = dlerror();
        if (0 != pszerror)
        {
            printf("%s\n", pszerror);
            return 1;
        }

        mytest();   //defined in test.c (libtest.c)
        printf("[%s %s]: g_count=%d, count=%d\n", __FILE__, __FUNCTION__, *g_count, *count); 
	*g_count = 20;
	*count = 200;
        mytest();   //defined in test.c (libtest.c)

        dlclose(pdlhandle);
        return 0;
}

