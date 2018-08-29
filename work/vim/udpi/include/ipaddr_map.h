#ifndef H_IP_MAP_H
#define H_IP_MAP_H

#include <stdint.h>

#define DB_DEF 0
#define DB_XJ 1

#if REGIONS == DB_XJ
#define UDPI_NUM_ADDR 441070
#else
#define UDPI_NUM_ADDR 16711679
#define UDPI_FIRST_IP 16777216
#define UDPI_LAST_IP 4294967039
#endif

struct addr_map
{
        uint32_t start;
        uint32_t end;
        uint8_t  isp;
        uint8_t  region;
};

#endif
