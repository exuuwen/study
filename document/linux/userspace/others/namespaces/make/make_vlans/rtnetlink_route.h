#ifndef RTNETLINK_ROUTE_H
#define  RTNETLINK_ROUTE_H

int create_route(const char *gw_ip, const char *dst_ip, const char *ifname);

int delete_route(const char *gw_ip, const char *dst_ip, const char *ifname);

#endif
