#include <linux/unistd.h>
#include <linux/bpf.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "common.h"

/*
   clang -O2 -Wall -target bpf -c tc-tunnel.c -o tc-tunnel.o
   gcc -O2 -o main main.c  -lbpf -lelf
   ip link add dev tun type gretap external
   tc qdisc add dev veth0 clsact
   tc qdisc add dev tun clsact
   tc filter add dev veth0 ingress bpf da obj tc-tunnel.o sec ingress_veth
   tc filter add dev tun ingress bpf da obj tc-tunnel.o sec ingress_gre
   ./main
*/

int main(void)
{
    const char *pinned_file = "/sys/fs/bpf/tc/globals/tunnel_map";
    struct tunnel_key key;
    struct tunnel_info info;
    int array_fd = -1;
    int ret = 0;

    array_fd = bpf_obj_get(pinned_file);
    if (array_fd < 0) {
        fprintf(stderr, "bpf_obj_get(%s): %s(%d)\n",
            pinned_file, strerror(errno), errno);
        goto out;
    }

    memset(&key, 0, sizeof(key));
    memset(&info, 0, sizeof(info));

    key.vni = 1000;
    key.ip_dst = 0xaca80007;
    info.ifindex = 8;

    ret = bpf_map_delete_elem(array_fd, &key);
    if (ret) {
        perror("bpf_map_delete_elem");
        goto out;
    }

    memset(&key, 0, sizeof(key));
    memset(&info, 0, sizeof(info));

    key.ifindex = 8;
    key.ip_dst = 0xaca80008;
    info.vni = 1000;
    info.tun_dst = 0xa133da7;
    info.ifindex = 12;

    ret = bpf_map_delete_elem(array_fd, &key);
    if (ret) {
        perror("bpf_map_delete_elem");
        goto out;
    }

out:
    if (array_fd != -1)
        close(array_fd);
    return ret;
}

