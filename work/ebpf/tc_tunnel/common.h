#ifndef __LIB_KEY_H_
#define __LIB_KEY_H_

#include <stdint.h>

struct  tunnel_key {
        __u32 vni;
        __u32 ifindex;
        __u32 ip_dst;
};

struct  tunnel_info {
        __u32 vni;
        __u32 ifindex;
        __u32 tun_dst;
};


#endif
