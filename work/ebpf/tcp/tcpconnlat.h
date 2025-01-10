#ifndef __TCPCONNLAT_H
#define __TCPCONNLAT_H

#define TASK_COMM_LEN	16

struct tcpevent {
	union {
		__u32 saddr_v4;
		__u8 saddr_v6[16];
	};
	union {
		__u32 daddr_v4;
		__u8 daddr_v6[16];
	};
	char comm[TASK_COMM_LEN];
	__u64 delta_us;
	__u64 ts_us;
	__u32 tgid;
	int af;
	__u16 lport;
	__u16 dport;
};

struct config {
	__u32 targ_tgid;
	__u64 targ_min_us;
};


#endif /* __TCPCONNLAT_H_ */
