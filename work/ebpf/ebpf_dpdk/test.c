/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */

/*
 * To compile on x86:
 * clang -O2 -U __GNUC__ -target bpf -c t1.c
 *
 * To compile on ARM:
 * clang -O2 -I/usr/include/aarch64-linux-gnu/ -target bpf -c t1.c
 */

#include <stdint.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

union ct_addr {
    uint32_t ipv4;
    struct in6_addr ipv6;
};

struct ct_endpoint {
    union ct_addr addr;
    union {
        uint16_t port;
        struct {
            uint16_t icmp_id;
            uint8_t icmp_type;
            uint8_t icmp_code;
        };
    };
};

struct conn_key {
    struct ct_endpoint src;
    struct ct_endpoint dst;

    uint16_t dl_type;
    uint16_t zone;
    uint8_t nw_proto;
};

extern uint64_t conn_key_add_zone(struct conn_key *key, uint16_t zone);

uint64_t
entry(void *key)
{
	struct conn_key *ckey = (void *)key;
	int ret;

	if (ckey->src.addr.ipv4 != htonl(0x1010101))
		return 0;

	if (ckey->dst.addr.ipv4 != htonl(0x1010107))
		return 0;

	if (ckey->zone != 0)
		return 3;
	
	return conn_key_add_zone(ckey, 1);
}
