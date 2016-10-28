#ifndef _BRIDGE_H_H_
#define _BRIDGE_H_H_

int add_bridge(const char *brname);

int del_bridge(const char *brname);

int bridge_add_port(const char *ifname, const char *brname);

int bridge_del_port(const char *ifname, const char *brname);

#endif 
