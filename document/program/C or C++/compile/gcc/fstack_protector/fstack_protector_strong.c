#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
fstack-protector-all
           Like -fstack-protector except that all functions are protected.

-fstack-protector-strong
           Like -fstack-protector but includes additional functions to be protected --- those that have local array definitions, or have
           references to local frame addresses.
*/
 
struct no_chars {
    unsigned int len;
    unsigned int data;
};
 
int main(int argc, char * argv[])
{
    struct no_chars info = { };
 
    if (argc < 3) {
        fprintf(stderr, "Usage: %s LENGTH DATA...\n", argv[0]);
        return 1;
    }
 
    info.len = atoi(argv[1]);
    memcpy(&info.data, argv[2], info.len);
 
    return 0;
}

/*
[root@10-19-61-167 ~]# gcc -Wall -O2  -fstack-protector-strong fstack_protectot_strong.c -o boom
[root@10-19-61-167 ~]# gcc -Wall -O2  -fstack-protector-all fstack_protectot_strong.c -o boom
[root@10-19-61-167 ~]# ./boom 5 AAAAA
*** stack smashing detected ***: ./boom terminated
======= Backtrace: =========
/lib64/libc.so.6(__fortify_fail+0x37)[0x7f655afd9597]
/lib64/libc.so.6(__fortify_fail+0x0)[0x7f655afd9560]
./boom[0x40061d]
/lib64/libc.so.6(__libc_start_main+0xf5)[0x7f655aeecb15]
./boom[0x400649]
======= Memory map: ========
00400000-00401000 r-xp 00000000 fd:01 36647130                           /root/boom
00600000-00601000 r--p 00000000 fd:01 36647130                           /root/boom
00601000-00602000 rw-p 00001000 fd:01 36647130                           /root/boom
01e33000-01e54000 rw-p 00000000 00:00 0                                  [heap]
7f655acb5000-7f655acca000 r-xp 00000000 fd:01 33554569                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f655acca000-7f655aec9000 ---p 00015000 fd:01 33554569                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f655aec9000-7f655aeca000 r--p 00014000 fd:01 33554569                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f655aeca000-7f655aecb000 rw-p 00015000 fd:01 33554569                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f655aecb000-7f655b082000 r-xp 00000000 fd:01 33607143                   /usr/lib64/libc-2.17.so
7f655b082000-7f655b282000 ---p 001b7000 fd:01 33607143                   /usr/lib64/libc-2.17.so
7f655b282000-7f655b286000 r--p 001b7000 fd:01 33607143                   /usr/lib64/libc-2.17.so
7f655b286000-7f655b288000 rw-p 001bb000 fd:01 33607143                   /usr/lib64/libc-2.17.so
7f655b288000-7f655b28d000 rw-p 00000000 00:00 0 
7f655b28d000-7f655b2ae000 r-xp 00000000 fd:01 34546357                   /usr/lib64/ld-2.17.so
7f655b4a5000-7f655b4a8000 rw-p 00000000 00:00 0 
7f655b4ac000-7f655b4ae000 rw-p 00000000 00:00 0 
7f655b4ae000-7f655b4af000 r--p 00021000 fd:01 34546357                   /usr/lib64/ld-2.17.so
7f655b4af000-7f655b4b0000 rw-p 00022000 fd:01 34546357                   /usr/lib64/ld-2.17.so
7f655b4b0000-7f655b4b1000 rw-p 00000000 00:00 0 
7ffd354bb000-7ffd354dc000 rw-p 00000000 00:00 0                          [stack]
7ffd355f6000-7ffd355f8000 r--p 00000000 00:00 0                          [vvar]
7ffd355f8000-7ffd355fa000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
Aborted (core dumped)
[root@10-19-61-167 ~]# gcc -Wall -O2  -fno-stack-protector fstack_protectot_strong.c -o boom
[root@10-19-61-167 ~]# gcc -Wall -O2  -fstack-protector fstack_protectot_strong.c -o boom
[root@10-19-61-167 ~]# ./boom 5 AAAAA
[root@10-19-61-167 ~]# 
  */
