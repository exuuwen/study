#define BPF_NO_PRESERVE_ACCESS_INDEX 
#define BPF_NO_GLOBAL_DATA  // compile with -g get btf, but not load to kernel with btf

#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_endian.h>
#include <bpf/bpf_core_read.h>
#include <tcpconnlat.h>

#define AF_INET    2
#define AF_INET6   10


#define _(P) ({typeof(P) val = 0; bpf_probe_read(&val, sizeof(val), &P); val;})

struct piddata {
	char comm[TASK_COMM_LEN];
	u64 ts;
	u32 tgid;
};

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, u32);
	__type(value, struct config);
	__uint(max_entries, 1);
} config_map SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, 4096);
	__type(key, struct sock *);
	__type(value, struct piddata);
} start SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
	__uint(key_size, sizeof(u32));
	__uint(value_size, sizeof(u32));
} events SEC(".maps");

static int trace_connect(struct sock *sk)
{
	u32 tgid = bpf_get_current_pid_tgid() >> 32;
	struct config *c;
	u32 index = 0;
	struct piddata piddata = {};

	c = bpf_map_lookup_elem(&config_map, &index);
	if (!c)
		return 0;

	if (c->targ_tgid && c->targ_tgid != tgid)
		return 0;

	bpf_get_current_comm(&piddata.comm, sizeof(piddata.comm));
	piddata.ts = bpf_ktime_get_ns();
	piddata.tgid = tgid;
	bpf_map_update_elem(&start, &sk, &piddata, 0);
	
	return 0;
}

static int handle_tcp_rcv_state_process(void *ctx, struct sock *sk)
{
	struct piddata *piddatap;
	struct tcpevent event = {};
	s64 delta;
	u64 ts;
	struct config *c;
	u32 index = 0;
	unsigned char state = 0;
	long ret;

	c = bpf_map_lookup_elem(&config_map, &index);
	if (!c)
		return 0;
	
	ret = bpf_probe_read(&state, sizeof(unsigned char), (const void *)&sk->__sk_common.skc_state);
	if (ret < 0)
		return 0;

	if (state != TCP_SYN_SENT)
		return 0;

	piddatap = bpf_map_lookup_elem(&start, &sk);
	if (!piddatap)
		return 0;

	ts = bpf_ktime_get_ns();
	delta = (s64)(ts - piddatap->ts);
	if (delta < 0)
		goto cleanup;

	event.delta_us = delta / 1000U;
	if (c->targ_min_us && event.delta_us < c->targ_min_us)
		goto cleanup;
	__builtin_memcpy(&event.comm, piddatap->comm,
			sizeof(event.comm));
	event.ts_us = ts / 1000;
	event.tgid = piddatap->tgid;
	event.lport = _(sk->__sk_common.skc_num);
	event.dport = _(sk->__sk_common.skc_dport);
	event.af = _(sk->__sk_common.skc_family);
	if (event.af == AF_INET) {
		event.saddr_v4 = _(sk->__sk_common.skc_rcv_saddr);
		event.daddr_v4 = _(sk->__sk_common.skc_daddr);
	} else {
		bpf_probe_read_kernel(event.saddr_v6, 16, sk->__sk_common.skc_v6_rcv_saddr.in6_u.u6_addr32);
		bpf_probe_read_kernel(event.daddr_v6, 16, sk->__sk_common.skc_v6_daddr.in6_u.u6_addr32);
	}
	bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
			&event, sizeof(event));

cleanup:
	bpf_map_delete_elem(&start, &sk);
	return 0;
}

SEC("kprobe/tcp_v4_connect")
int BPF_KPROBE(tcp_v4_connect, struct sock *sk)
{
	return trace_connect(sk);
}

SEC("kprobe/tcp_v6_connect")
int BPF_KPROBE(tcp_v6_connect, struct sock *sk)
{
	return trace_connect(sk);
}

SEC("kprobe/tcp_rcv_state_process")
int BPF_KPROBE(tcp_rcv_state_process, struct sock *sk)
{
	return handle_tcp_rcv_state_process(ctx, sk);
}


SEC("tracepoint/tcp/tcp_destroy_sock")
int tcp_destroy_sock(struct trace_event_raw_tcp_event_sk *ctx)
{
	const struct sock *sk = ctx->skaddr;

	bpf_map_delete_elem(&start, &sk);
	return 0;
}
char LICENSE[] SEC("license") = "GPL";
