#!/bin/bash
gcc -shared -fPIC -o libfoo.so foo.c
gcc -shared -fPIC -o libbar.so bar.c
export LIBRARY_PATH=.
gcc -o main main.c -lbar -lfoo
export LD_LIBRARY_PATH=.
./main
