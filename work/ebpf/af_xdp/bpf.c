// SPDX-License-Identifier: GPL-2.0
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

#define trace_printk(fmt, ...)                                       \
	do {                                                         \
		char _fmt[] = fmt;                                   \
		bpf_trace_printk(_fmt, sizeof(_fmt), ##__VA_ARGS__); \
	} while (0)

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, int);
	__type(value, int);
	__uint(pinning, LIBBPF_PIN_BY_NAME);
	__uint(max_entries, 1);
} qidconf_map SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_XSKMAP);
	__type(key, int);
	__type(value, int);
	__uint(pinning, LIBBPF_PIN_BY_NAME);
	__uint(max_entries, 4);
} xsks_map SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
	__type(key, int);
	__type(value, unsigned int);
	__uint(pinning, LIBBPF_PIN_BY_NAME);
	__uint(max_entries, 1);
} rr_map SEC(".maps");

SEC("xdp")
int xdp_sock_prog(struct xdp_md *ctx)
{
	int *qidconf, key = 0, idx;
	unsigned int *rr;

	qidconf = bpf_map_lookup_elem(&qidconf_map, &key);
	if (!qidconf)
		return XDP_ABORTED;

	if (*qidconf != ctx->rx_queue_index)
		return XDP_PASS;

#if RR_LB /* NB! RR_LB is configured in xdpsock.h */
	rr = bpf_map_lookup_elem(&rr_map, &key);
	if (!rr)
		return XDP_ABORTED;

	*rr = (*rr + 1) & (MAX_SOCKS - 1);
	idx = *rr;
#else
	idx = 0;
#endif

	return bpf_redirect_map(&xsks_map, idx, 0);
}

char _license[] SEC("license") = "GPL";
