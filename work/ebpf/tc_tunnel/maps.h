#ifndef __LIB_MAPS_H_
#define __LIB_MAPS_H_

#include <stdint.h>
#include <linux/bpf.h>
#include "common.h"

struct bpf_elf_map SEC("maps") tunnel_map = {
        .type           = BPF_MAP_TYPE_HASH,
        .size_key       = sizeof(struct tunnel_key),
        .size_value     = sizeof(struct tunnel_info),
        .pinning        = PIN_GLOBAL_NS,
        .max_elem       = 100,
};

#endif
