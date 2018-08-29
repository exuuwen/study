gcc -fPIC -shared -o libtest.so libtest.c
gcc -c program.c
gcc program.o -o program ./libtest.so
