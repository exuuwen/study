#ifndef UAKE_HEAD_H
#define UAKE_HEAD_H

#include <linux/in6.h>

#define MAX_UAKE_SIZE  512

struct uake_entry {
	struct in6_addr  ip6_addr;
	unsigned int   ip_addr;
};

struct uake_map {
	unsigned short count;
	struct uake_entry maps[MAX_UAKE_SIZE];
};

#define UAKE_MAP_UPDATE  _IOW('o', 1, struct uake_map)
#define UAKE_MAP_GET     _IOWR('o', 2, struct uake_entry)

#endif
