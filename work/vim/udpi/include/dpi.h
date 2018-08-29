#ifndef _UDPI_IPV4_DPI_H_
#define _UDPI_IPV4_DPI_H_

#include <rte_atomic.h>
#include <rte_hash.h>
#include <rte_spinlock.h>

#include <stdint.h>

#include "stats.h"
#include "ipaddr_map.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UDPI_MAX_TCP_RETRANSMISSION_WINDOW_SIZE 0x10000
#define UDPI_BASE_STEPS 16

/*
 * TCP State Values
 */
enum tcp_conntrack {
	TCP_CONNTRACK_NONE,
	TCP_CONNTRACK_SYN_SENT,
	TCP_CONNTRACK_SYN_RECV,
	TCP_CONNTRACK_ESTABLISHED,
	TCP_CONNTRACK_FIN_WAIT,
	TCP_CONNTRACK_CLOSE_WAIT,
	TCP_CONNTRACK_LAST_ACK,
	TCP_CONNTRACK_TIME_WAIT,
	TCP_CONNTRACK_CLOSE,
	TCP_CONNTRACK_LISTEN,	/* obsolete */
#define TCP_CONNTRACK_SYN_SENT2	TCP_CONNTRACK_LISTEN
	TCP_CONNTRACK_MAX,
	TCP_CONNTRACK_IGNORE
};

#define sNO TCP_CONNTRACK_NONE
#define sSS TCP_CONNTRACK_SYN_SENT
#define sSR TCP_CONNTRACK_SYN_RECV
#define sES TCP_CONNTRACK_ESTABLISHED
#define sFW TCP_CONNTRACK_FIN_WAIT
#define sCW TCP_CONNTRACK_CLOSE_WAIT
#define sLA TCP_CONNTRACK_LAST_ACK
#define sTW TCP_CONNTRACK_TIME_WAIT
#define sCL TCP_CONNTRACK_CLOSE
#define sS2 TCP_CONNTRACK_SYN_SENT2
#define sIV TCP_CONNTRACK_MAX
#define sIG TCP_CONNTRACK_IGNORE

/* What TCP flags are set from RST/SYN/FIN/ACK. */
enum tcp_bit_set {
	TCP_SYN_SET,
	TCP_SYNACK_SET,
	TCP_FIN_SET,
	TCP_ACK_SET,
	TCP_RST_SET,
	TCP_NONE_SET,
};

enum {
	TCP_DIR_ORIGIN = 0,
	TCP_DIR_REPLY,
	TCP_DIR_LAST,
};

struct flow_key {
	uint32_t upper_ip;
	uint32_t lower_ip;
	uint16_t upper_port;
	uint16_t lower_port;
	uint8_t  proto;
	uint8_t  diff;
}__attribute__((packed));

struct tcp_timestamps
{
	uint32_t last_time_stamps;
	uint32_t last_finished_stamps;
	uint64_t last_seen;	
};

struct flow_entry {
	struct flow_key flow;
	uint8_t region;
	uint8_t isp;
	uint64_t  last_seen;	
	struct eip_entry *eip;
	struct tcp_timestamps ts;
	uint32_t  next_tcp_seq_nr[2];
	uint64_t  last_http_request;	
	uint8_t validate;
	uint8_t state;
	uint8_t dir_origin;
	uint8_t state_ok;
};

struct eip_key {
	uint32_t eip;
}__attribute__((packed));

struct eip_entry {
	struct eip_key eip;
	struct udpi_eip_local_stats stats;
	uint64_t last_conn;
	uint64_t last_cps;
	uint32_t  token;
}__rte_cache_aligned;

struct udpi_hash {
	struct rte_hash *eip_hash;
	struct rte_hash *flow_hash;

	rte_atomic64_t eip_entries;

	struct eip_entry *eip_entry;
	struct flow_entry **flow_entry;
}__rte_cache_aligned;

#ifdef __cplusplus
}
#endif

#endif

