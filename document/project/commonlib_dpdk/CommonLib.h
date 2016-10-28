/*******************************************************************************
Description:

This file contains a lot of classes. This class is designed 
to implement the the basic library.

Author: Wen Xu

History:

11/23/2011 WX  Initial code.

*******************************************************************************/
#ifndef _COMMON_LIB_H_
#define _COMMON_LIB_H_
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <net/ethernet.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <asm/unistd.h>
#include <sys/mman.h>
#include <unistd.h>       
#include <sys/syscall.h>    
#include <sys/types.h>
#include <dirent.h>



// Many basic types definition
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef int            s32;
typedef unsigned char  byte;
typedef unsigned char  uchar;
typedef u8             u8bool;
typedef unsigned long long uint64_t;
typedef u32            uint32_t;

typedef uint64_t phys_addr_t; /**< Physical address definition. */


#define SUCCESS             0
#define FAIL                1

#define OK                  0
#define ERROR              -1





#endif

