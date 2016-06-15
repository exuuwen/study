#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdint.h>

#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ring.h>
#include <rte_ethdev.h>
#include <rte_ip.h>
#include <rte_hash.h>

#ifndef bool
#define bool char
#define true 1
#define false 0
#endif

struct trace_flow {
	uint32_t srcip;
	uint32_t dstip;
	uint16_t srcport;
	uint16_t dstport;
	uint8_t proto;
} __attribute__((__packed__));

struct trace_log {
	uint32_t level;
	bool is_trace_enabled;
	struct trace_flow flow;
};

#ifndef UDPI_MAX_PORTS
#define UDPI_MAX_PORTS 4
#endif
#ifndef UDPI_MBUF_ARRAY_SIZE
#define UDPI_MBUF_ARRAY_SIZE 256
#endif

#ifndef UDPI_MAX_QUEUES
#define UDPI_MAX_QUEUES 8
#endif

#ifndef UDPI_CPU_FREQ
#define UDPI_CPU_FREQ 2800  /*MHz*/
#endif

#define UPDI_FLUSH 0xFF


enum udpi_core_type {
	UDPI_CORE_NONE = 0,
	UDPI_CORE_TASK = 1,
	UDPI_CORE_IPV4_RX = 2,
};


enum udpi_port_type {
	UDPI_PORT_NONE = 0,
	UDPI_PORT_INPUT = 1,
	UDPI_PORT_OUTPUT = 2,
};

enum udpi_msg_req_type {
	UDPI_MSG_SET_TRACE = 0,
	UDPI_MSG_DISABLE_TRACE = 1,
	UDPI_MSG_SET_LOG_LEVEL = 2,
};

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
	uint32_t port_type;
} __rte_cache_aligned;

struct udpi_pkt_metadata {
	uint64_t tsc;
	uint32_t ip_src;
	uint32_t ip_dst;
	uint16_t port_src;
	uint16_t port_dst;
	uint8_t  proto;
	uint8_t reserved[3];
} __attribute__((__packed__));

struct udpi_params {
	/* CPU cores*/
	struct udpi_core_params cores[RTE_MAX_LCORE];
	uint32_t n_cores;

	uint32_t n_workers;

	uint8_t trace;

	/* Ports */
	struct udpi_port_params ports[UDPI_MAX_PORTS];
	uint32_t n_ports;

	struct rte_eth_conf port_conf;
	uint32_t rsz_hwq_rx;
	int32_t  rsz_hwq_tx;
	uint32_t bsz_hwq_rd;
	uint32_t bsz_hwq_wr;

	/* SW Queues (SWQs) */
	struct rte_ring **rings;
	struct rte_ring **stats_rings;
	uint32_t rsz_swq;
	uint32_t bsz_swq_rd;
	uint32_t bsz_swq_wr;

	/* Buffer pool */
	struct rte_mempool *pool;
	//struct rte_mempool *indirect_pool;
	uint32_t pool_buffer_size;
	uint32_t pool_size;
	uint32_t pool_cache_size;

	/* Message buffer pool */
	struct rte_mempool *msg_pool;
	uint32_t msg_pool_buffer_size;
	uint32_t msg_pool_size;
	uint32_t msg_pool_cache_size;

	struct rte_ring **msg_rings;
	
	struct trace_log log;
}__rte_cache_aligned;


extern struct udpi_params udpi;

struct udpi_msg_req {
	enum udpi_msg_req_type type;
	union {
		struct {
			uint8_t level;
		}set_log_level;
		struct {
			struct trace_flow flow;
		}set_trace;
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
void udpi_main_loop_task(void);
void udpi_main_loop_ipv4_rx(void);
void udpi_print_usage(char *prgname);
int32_t udpi_core_type_string_to_id(const char *string, enum udpi_core_type *id);
int32_t udpi_port_type_string_to_id(const char *string, enum udpi_port_type *id);
int32_t udpi_parse_args(int32_t argc, char **argv);
struct udpi_core_params * udpi_get_core_params(uint32_t core_id);
uint32_t udpi_get_first_core_id(enum udpi_core_type core_type);
struct rte_ring * udpi_get_ring_req(uint32_t core_id);
struct rte_ring * udpi_get_ring_resp(uint32_t core_id);

#endif /* _MAIN_H_ */
