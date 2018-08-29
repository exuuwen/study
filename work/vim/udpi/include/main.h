#ifndef _MAIN_H_
#define _MAN_H_


#include <stdint.h>

#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ring.h>
#include <rte_ethdev.h>
#include <rte_ip.h>
#include <rte_hash.h>

#include "stats.h"
#include "dpi.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UDPI_MAX_PORTS
#define UDPI_MAX_PORTS 4
#endif
#ifndef UDPI_MBUF_ARRAY_SIZE
#define UDPI_MBUF_ARRAY_SIZE 256
#endif

#ifndef UDPI_MAX_WORKERS
#define UDPI_MAX_WORKERS 12
#endif

#ifndef UDPI_MAX_DUMPERS
#define UDPI_MAX_DUMPERS (UDPI_MAX_WORKERS)
#endif

#ifndef UDPI_MAX_DPDKID
#define UDPI_MAX_DPDKID 8
#endif

#ifndef UDPI_MAX_PACKET
#define UDPI_MAX_PACKET 4
#endif

#ifndef UDPI_MAX_QUEUES
#define UDPI_MAX_QUEUES 16
#endif

#ifndef UDPI_CPU_FREQ
#define UDPI_CPU_FREQ 2800  /*MHz*/
#endif

#define UPDI_FLUSH 0xFF

#define UDPI_METADATA_OFFSET(offset) (sizeof(struct rte_mbuf) + (offset))

#define true 1
#define false 0

enum udpi_core_type {
	UDPI_CORE_NONE = 0,
	UDPI_CORE_TASK = 1,
	UDPI_CORE_IPV4_RX = 2,
	UDPI_CORE_IPV4_DPI = 3,
	UDPI_CORE_STATS = 4,
	UDPI_CORE_PACKET = 5,
};

enum udpi_msg_req_type {
	UDPI_MSG_ADD_STATS = 0,
	UDPI_MSG_DEL_STATS = 1,
};

#define UDPI_PORT_INPUT  1
#define UDPI_PORT_OUTPUT 2

struct udpi_core_params {
	uint32_t core_id;
	uint32_t port_id;
	uint32_t id;
	enum udpi_core_type core_type;
	uint64_t core_handle_cycles;
	uint64_t core_total_cycles;
} __rte_cache_aligned;

struct udpi_port_params {
	uint32_t n_queues;
	uint32_t port_id;
} __rte_cache_aligned;

struct udpi_pkt_metadata {
	uint64_t tsc;
	uint32_t ip_src;
	uint32_t ip_dst;
	uint16_t port_src;
	uint16_t port_dst;
	uint8_t  proto;
	uint8_t region_type;
	uint8_t operator_type;
	uint8_t comp_type;
	uint8_t  port_type;
	uint8_t  reserved[3];
	uint16_t payload_len;
	uint16_t l4_offset;
	uint8_t line_num;
	uint8_t is_inline;
} __attribute__((__packed__));

struct udpi_overflow_counter {
	rte_atomic64_t eip;
	rte_atomic64_t flow;
	rte_atomic64_t notsyn;
	rte_atomic64_t cps;
	rte_atomic64_t packet;
	rte_atomic64_t packet_packet;
	rte_atomic64_t packet_mem;
	rte_atomic64_t packet_fail;
} __rte_cache_aligned;

struct udpi_params {
	/* CPU cores*/
	struct udpi_core_params cores[RTE_MAX_LCORE];
	uint32_t n_cores;

	/* Ports */
	struct udpi_port_params ports[UDPI_MAX_PORTS];
	uint32_t n_ports;

	uint32_t rsz_hwq_rx;
	int32_t  rsz_hwq_tx;
	uint32_t bsz_hwq_rd;
	uint32_t bsz_hwq_wr;

	uint32_t n_workers;
	uint32_t n_dumpers;
	uint32_t n_packets;
	int dumper_fds[RTE_MAX_LCORE];
	int packet_fds[RTE_MAX_LCORE];

	uint8_t volatile debug;

	uint32_t server_ip_be;
	uint32_t userver_ip_be;

	/* SW Queues (SWQs) */
	struct rte_ring **rings;
	struct rte_ring **stats_rings;
	struct rte_ring *packet_ring[UDPI_MAX_PACKET];
	uint32_t rsz_swq;
	uint32_t bsz_swq_rd;
	uint32_t bsz_swq_wr;

	/* Buffer pool */
	struct rte_mempool *pool[UDPI_MAX_QUEUES];
	//struct rte_mempool *indirect_pool;
	uint32_t pool_buffer_size;
	uint32_t pool_size;
	uint32_t pool_cache_size;

	/* Message buffer pool */
	struct rte_mempool *msg_pool;
	uint32_t msg_pool_buffer_size;
	uint32_t msg_pool_size;
	uint32_t msg_pool_cache_size;

	/* Hash params*/
	struct rte_hash_parameters eip_params;
	struct rte_hash_parameters flow_params;
	struct udpi_hash hash[UDPI_MAX_WORKERS];

	struct rte_hash_parameters addr_params;
	struct rte_hash *addr_hash[2];
	volatile uint8_t hash_id;
	struct rte_hash_parameters comp_addr_params;
	struct rte_hash *comp_addr_hash[2];
	volatile uint8_t comp_hash_id;

	/* Stats */
	struct udpi_stats stats;

	rte_atomic64_t eip_entries;

	rte_atomic64_t closed_flows[TCP_CONNTRACK_MAX];

	struct udpi_overflow_counter overflow;
	
	uint8_t dpdk_id;

	uint32_t line[UDPI_LINE_STATS];
	uint32_t line_src_judge[UDPI_LINE_STATS];//line_src_judge==1 src  else  dst
}__rte_cache_aligned;


extern struct udpi_params udpi;

struct udpi_msg_req {
	enum udpi_msg_req_type type;
	union {
		struct {
			time_t t;
		}add_stats;
		struct {
			time_t t;
		}del_stats;
	};	
};

struct udpi_mbuf_array {
	struct rte_mbuf *array[UDPI_MBUF_ARRAY_SIZE];
	uint32_t n_mbufs;
};

struct udpi_msg_resp {
	int32_t result;
};


void udpi_ping(void);
void udpi_init(void);
int32_t udpi_lcore_main_loop(__attribute__((unused)) void *arg);
void udpi_main_loop_ipv4_rx(void);
void udpi_main_loop_ipv4_dpi(void);
void udpi_main_loop_stats(void);
void udpi_main_loop_task(void);
void udpi_main_loop_packet(void);
void udpi_print_usage(char *prgname);
int32_t udpi_core_type_string_to_id(const char *string, enum udpi_core_type *id);
int32_t udpi_parse_args(int32_t argc, char **argv);
struct udpi_core_params * udpi_get_core_params(uint32_t core_id);
uint32_t udpi_get_first_core_id(enum udpi_core_type core_type);
struct rte_ring * udpi_get_ring_req(uint32_t core_id);
struct rte_ring * udpi_get_ring_resp(uint32_t core_id);

#ifdef __cplusplus
}

#endif
#endif /* _MAIN_H_ */
