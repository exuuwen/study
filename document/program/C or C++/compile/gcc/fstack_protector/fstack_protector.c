#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 -fstack-protector
           Emit extra code to check for buffer overflows, such as stack smashing attacks.  This is done by adding a guard variable to
           functions with vulnerable objects.  This includes functions that call "alloca", and functions with buffers larger than 8 bytes.
           The guards are initialized when a function is entered and then checked when the function exits.  If a guard check fails, an
           error message is printed and the program exits.

*/
 
int main(int argc, char * argv[])
{
    //more than 8
    char array[8];
 
    if (argc < 2) {
        fprintf(stderr, "Usage: %s DATA...\n", argv[0]);
        return 1;
    }

    printf("strlen %ld\n", strlen(argv[1]));
 
    memcpy(array, argv[1], strlen(argv[1]));
 
    return 0;
}

/*
 [root@10-19-61-167 ~]# gcc -Wall -O2  -fstack-protector fstack_protectot.c -o boom
 [root@10-19-61-167 ~]# gcc -Wall -O2  -fstack-protector-all fstack_protectot.c -o boom
 [root@10-19-61-167 ~]# gcc -Wall -O2  -fstack-protector-strong fstack_protectot.c -o boom
[root@10-19-61-167 ~]# ./boom AAAAAAAAA
strlen 9
*** stack smashing detected ***: ./boom terminated
======= Backtrace: =========
/lib64/libc.so.6(__fortify_fail+0x37)[0x7f2bb592e597]
/lib64/libc.so.6(__fortify_fail+0x0)[0x7f2bb592e560]
./boom[0x400667]
/lib64/libc.so.6(__libc_start_main+0xf5)[0x7f2bb5841b15]
./boom[0x400691]
======= Memory map: ========
00400000-00401000 r-xp 00000000 fd:01 34546363                           /root/boom
00600000-00601000 r--p 00000000 fd:01 34546363                           /root/boom
00601000-00602000 rw-p 00001000 fd:01 34546363                           /root/boom
00d9f000-00dc0000 rw-p 00000000 00:00 0                                  [heap]
7f2bb560a000-7f2bb561f000 r-xp 00000000 fd:01 33554569                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f2bb561f000-7f2bb581e000 ---p 00015000 fd:01 33554569                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f2bb581e000-7f2bb581f000 r--p 00014000 fd:01 33554569                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f2bb581f000-7f2bb5820000 rw-p 00015000 fd:01 33554569                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f2bb5820000-7f2bb59d7000 r-xp 00000000 fd:01 33607143                   /usr/lib64/libc-2.17.so
7f2bb59d7000-7f2bb5bd7000 ---p 001b7000 fd:01 33607143                   /usr/lib64/libc-2.17.so
7f2bb5bd7000-7f2bb5bdb000 r--p 001b7000 fd:01 33607143                   /usr/lib64/libc-2.17.so
7f2bb5bdb000-7f2bb5bdd000 rw-p 001bb000 fd:01 33607143                   /usr/lib64/libc-2.17.so
7f2bb5bdd000-7f2bb5be2000 rw-p 00000000 00:00 0 
7f2bb5be2000-7f2bb5c03000 r-xp 00000000 fd:01 34546357                   /usr/lib64/ld-2.17.so
7f2bb5dfa000-7f2bb5dfd000 rw-p 00000000 00:00 0 
7f2bb5e00000-7f2bb5e03000 rw-p 00000000 00:00 0 
7f2bb5e03000-7f2bb5e04000 r--p 00021000 fd:01 34546357                   /usr/lib64/ld-2.17.so
7f2bb5e04000-7f2bb5e05000 rw-p 00022000 fd:01 34546357                   /usr/lib64/ld-2.17.so
7f2bb5e05000-7f2bb5e06000 rw-p 00000000 00:00 0 
7ffc5edd0000-7ffc5edf1000 rw-p 00000000 00:00 0                          [stack]
7ffc5edfa000-7ffc5edfc000 r--p 00000000 00:00 0                          [vvar]
7ffc5edfc000-7ffc5edfe000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
Aborted (core dumped)
[root@10-19-61-167 ~]# gcc -Wall -O2  -fno-stack-protector fstack_protectot.c -o boom
[root@10-19-61-167 ~]# ./boom AAAAAAAAA
strlen 9
*/

