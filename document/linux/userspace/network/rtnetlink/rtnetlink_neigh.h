#ifndef RTNETLINK_NEIGH_H
#define RTNETLINK_NEIGH_H

int create_neigh(const char *ip, unsigned char *mac, const char* ifname, int permanent);

int delete_neigh(const char *ip, const char* ifname);

#endif
