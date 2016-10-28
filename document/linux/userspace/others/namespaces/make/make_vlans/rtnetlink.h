#ifndef H_H_RTNETLINK_H_H
#define H_H_RTNETLINK_H_H

#include <linux/rtnetlink.h>

typedef struct {
    struct nlmsghdr   n;
    struct ifinfomsg  ifi;		
    char              buf[512];
} rt_link_request_t;

typedef struct {
    struct nlmsghdr   n;
    struct ifaddrmsg  ifa;
    char              buf[512];
} rt_addr_request_t;

typedef struct {
    struct nlmsghdr   n;
    struct rtmsg      rt;
    char              buf[1024];
} rt_route_request_t;

typedef struct {
    struct nlmsghdr   n;
    struct ndmsg      nd;
    char              buf[1024];
} rt_neigh_request_t;


int rt_open(struct sockaddr_nl *loc_addr);

int rt_find_ack(struct nlmsghdr *hdr, 
		   struct sockaddr_nl *nladdr, 
		   struct msghdr *nlmsg, 
		   struct sockaddr_nl *loc_addr,		   
		   unsigned int *seq, int r);

int rt_send(int fd, unsigned int *seq, struct nlmsghdr *n, struct sockaddr_nl *loc_addr);

struct rtattr* rt_addAttr_data(struct nlmsghdr *n, int type, const void *data, int len);

struct rtattr * rt_addAttr_hdr(struct nlmsghdr *n, int type);

void rt_compAttr_hdr(struct nlmsghdr *n, struct rtattr * rta_head);

void rt_link_init(struct nlmsghdr *n, int cmd, unsigned short flags);

void rt_addr_init(struct nlmsghdr *n, int cmd, unsigned short flags);

void rt_route_init(struct nlmsghdr *n, int cmd, unsigned short flags);

#endif
