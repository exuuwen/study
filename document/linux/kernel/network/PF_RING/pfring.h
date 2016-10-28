/*
 *
 * Definitions for packet ring
 *
 * 2004-10 Luca Deri <deri@ntop.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef __RING_H
#define __RING_H

#ifdef __KERNEL__
#include <linux/in6.h>
#include <asm/atomic.h>
#else
#include <netinet/in.h>
#define PAGE_SIZE 4096
#endif /* __KERNEL__ */

// Kernel use 64bits, while userland uses 32bits,
// in order to align, pack it to 32 bits order
#pragma pack(4)
#ifndef ETH_ALEN
#define ETH_ALEN  6
#endif

#define PF_RING               27
#define SOCK_RING             PF_RING

#define RESERVED_HDR_SPACE    48     // may have a longer header after processing

#define PF_RING_BUFFER_SIZE   40960000
#define PFRING_SNAP_LENGTH_DEFAULT  0XFFFF /* 64K */

// Versioning
#define RING_VERSION          1

#define PF_DEFRAG_RING 1234

// Setopt used value
#define SO_SET_PFRING_ACTI_SOCK       150
#define SO_SET_PFRING_DEACTI_SOCK     151
#define SO_SET_PFRING_BUFFER_SIZE     152
#define SO_SET_PFRING_SNAP_LENGTH     153

// Getopt used value
#define SO_GET_PFRING_LOG_LEVEL       160
#define SO_GET_PFRING_BUFFER_SIZE     161
#define SO_GET_PFRING_SNAP_LENGTH     162


typedef struct flowSlotInfo
{
    u_int32_t end_off, tot_mem;
    u_int32_t insert_off, forward_off, recycle_off;
    u_int64_t tot_lost, tot_insert;
    char padding64[64 - 36];
    /* <-- 64 bytes here, should be enough to avoid some L1 VIVT coherence issues (32 ~ 64bytes lines) */
    u_int16_t version;
    char padding128[128 - 66];
    /* <-- 128 bytes here, should be enough to avoid false sharing in most L2 (64 ~ 128bytes lines) */
    char padding4096[4096 - 128];
    /* <-- 4096 bytes here, to get a page aligned block writable by kernel side only */

    /* second page, managed by userland */
    u_int32_t read_off;
    u_int64_t tot_read;
    char padding8192[4096 - 12];
    /* <-- 8192 bytes here, to get a page aligned block writable by userland only */
} FlowSlotInfo;

/* 0=empty, 1=occupation, 2=ready to tx */
typedef enum {EMPTY=0, FULL_RX, FULL_TX, RECYCLABLE} SLOT_STATE;

typedef struct flowSlot
{
    u_int8_t     slot_state;    
    u_int32_t    slot_len;		/* the slot length including slot meta data and reserved space */
    u_int32_t    pkt_off;       /* unused */
    u_int16_t    snap_len;      /* the packet length in the slot */
    u_int16_t    pkt_len;  		/* the real packet length */
    u_int32_t    if_id;         /* reserved */
    struct sk_buff *skb;        /* pointer to orignal skb */
    unsigned char reserved_hdr_space[RESERVED_HDR_SPACE];
    unsigned char bucket[0];
} FlowSlot;

/* Debug level */
typedef enum
{
    PfNone=0,
    PfError,
    PfWarning,
    PfNotice,
    PfInfo,
    PfDebug,
    PfDebug2,
    PfDebug3
} PfTraceLevel;

#ifdef __KERNEL__

struct ring_sock
{
    struct sock        sk;       /* It MUST be the first element */
    struct packet_type prot_hook;
    spinlock_t         bind_lock;
};

/*
 * Ring socket infor
 */
struct ring_info
{
    u_int8_t index;            /* the index of this socket */
    u_int8_t active;

    char   dev_name[IFNAMSIZ];
    struct socket *sock;      /* the BSD socket it belongs to */
    u_int16_t pid;            /* owning pid info */

    unsigned int buffer_size;  /* ring size */
    unsigned int snap_length;  /* the max length copied to slot */
	atomic_t  tot_fwd_ok;       /* total packets forwarded sucessfully */
    atomic_t  tot_fwd_notok;    /* total packets failed to forward */
    atomic_t  tot_fwd_large;    /* total packets need to break into fragments before forwarding */
    bool was_empty;

    void * memory_start;
    FlowSlotInfo *slots_info; /* Points to ring_memory */
    char *slot_start;         /* Points to ring_memory+sizeof(FlowSlotInfo) */

    wait_queue_head_t waitqueue;
    rwlock_t buffer_lock;
};

#endif /* __KERNEL__  */

#pragma pack()
#endif /* __RING_H */

