#ifndef H_LOG_H
#define H_LOG_H

#include <string.h>
#include <syslog.h>

#include <rte_log.h>
#include <rte_lcore.h>
#include <rte_branch_prediction.h>

#include "main.h"

RTE_DECLARE_PER_LCORE(struct trace_log, g_trace_log);

static inline void set_dpdk_log_level(uint32_t level) {
	rte_set_log_level(level + 1);
}

static inline void copy_dpdk_log_level(void) {
	uint32_t level = rte_get_log_level() - 1;
	struct trace_log log = RTE_PER_LCORE(g_trace_log);
	log.level = level;
	RTE_PER_LCORE(g_trace_log) = log;
}

static inline void set_local_log_level(uint32_t level) {
	struct trace_log log = RTE_PER_LCORE(g_trace_log);
	log.level = level;
	RTE_PER_LCORE(g_trace_log) = log;
}

static inline void enable_trace(struct trace_flow flow) {
	struct trace_log log = RTE_PER_LCORE(g_trace_log);
	log.is_trace_enabled = true;
	log.flow = flow;
	RTE_PER_LCORE(g_trace_log) = log;
}

static inline void disable_trace(void) {
	struct trace_log log = RTE_PER_LCORE(g_trace_log);
	log.is_trace_enabled = false;
	RTE_PER_LCORE(g_trace_log) = log;
}

static inline bool is_trace_enabled(void) {
	struct trace_log log = RTE_PER_LCORE(g_trace_log);
	return  log.is_trace_enabled;
}

static inline bool is_log_enabled(uint32_t level) {
	struct trace_log log = RTE_PER_LCORE(g_trace_log);

	if (unlikely(level <= log.level))
		return true;

	return false;
}

static inline bool is_flow_traced(struct trace_flow flow)
{
	if (is_trace_enabled())
	{
		struct trace_log log = RTE_PER_LCORE(g_trace_log);
		if (!memcmp(&log.flow, &flow, sizeof(struct trace_flow)))
		{
			return true;
		} 
		
	}

	return false;
}

static inline char const* basename(char const* path) {
	char const* filename = strrchr(path, '/');
	if (NULL == filename)
		return path;
	else
		return filename + 1;
}

#define UDPI_LOG_COPY_DPDK_LEVEL()       copy_dpdk_log_level()
#define UDPI_LOG_SET_LOCAL_LEVEL(l)       set_local_log_level(LOG_ ## l)
#define UDPI_LOG_SET_DPDK_LEVEL(l)       set_dpdk_log_level(LOG_ ## l)
#define UDPI_LOG_ENABLE_TRACE(flow)     enable_trace(flow)
#define UDPI_LOG_DISABLE_TRACE()    disable_trace()
#define UDPI_LOG_ENABLED(l)         unlikely(is_log_enabled(LOG_ ## l))
#define UDPI_TRACE_ENABLED()	    unlikely(is_trace_enabled())
#define UDPI_FLOW_TRACED(flow)	    unlikely(is_flow_traced(flow))

#define UDPI_LOG(l, format, ...)                                     \
  do {                                                               \
	if(UDPI_LOG_ENABLED(l)) {                                        \
      fprintf(stderr, "<%d>[C%u@%s:%d] " format "\n",               \
    		  LOG_ ## l, rte_lcore_id(),                             \
			  basename(__FILE__), __LINE__,                    \
			  ## __VA_ARGS__);                                       \
	}                                                                \
  } while(0)

#define UDPI_TRACE(format, ...)                                     \
  do {                                                               \
      fprintf(stderr, "[C%u@%s:%d] " format "\n",               \
    		  	  rte_lcore_id(),                             \
			  basename(__FILE__), __LINE__,                    \
			  ## __VA_ARGS__);                                       \
  } while(0)


#define UDPI_DEBUG( format, ...)  UDPI_LOG(DEBUG,   format, ## __VA_ARGS__)
#define UDPI_INFO(  format, ...)  UDPI_LOG(INFO,    format, ## __VA_ARGS__)
#define UDPI_NOTICE(format, ...)  UDPI_LOG(NOTICE,  format, ## __VA_ARGS__)
#define UDPI_WARN(  format, ...)  UDPI_LOG(WARNING, format, ## __VA_ARGS__)
#define UDPI_ERROR( format, ...)  UDPI_LOG(ERR,     format, ## __VA_ARGS__)

#define UDPI_COUNTER(format, ...)                                    \
  do {                                                               \
    fprintf(stderr, "<0>" format "\n", ## __VA_ARGS__);              \
  } while(0)

#define UDPI_PACKET_DUMP(pkt, format, ...)                           \
  do {                                                               \
	if(UDPI_TRACE_ENABLED()) {                                    \
	  char buf[8 * 1024];                                            \
	  FILE * file = fmemopen(buf, sizeof(buf), "w");                 \
	  rte_pktmbuf_dump(file, pkt, rte_pktmbuf_pkt_len(pkt));         \
	  fclose(file);                                                  \
	  UDPI_TRACE(format "\n%s", ## __VA_ARGS__, buf);                \
	}                                                                \
  } while(0)


#endif
