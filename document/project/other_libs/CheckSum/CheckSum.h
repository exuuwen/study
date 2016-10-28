#ifndef IP_UDP_CHECKSUM_
#define IP_UDP_CHECKSUM_

#include <CommonLib.h>
unsigned short ip_sum_calc(unsigned short length, unsigned short *buf);
unsigned short udp_checksum(unsigned short *buffer, unsigned short size, unsigned int src_addr, unsigned int dest_addr);

#endif
