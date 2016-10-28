#ifndef H_H_RTNETLINK_ADDR_H_H
#define H_H_RTNETLINK_ADDR_H_H

int create_addr( const char *ifname, const char *ip);

int delete_addr(const char *ifname, const char *ip);

#endif
