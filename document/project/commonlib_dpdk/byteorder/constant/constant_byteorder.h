#ifndef _CONSTANT_BYTEORDER_H_
#define _CONSTANT_BYTEORDER_H_
#include "../../CommonLib.h"

/**
 * An internal function to swap bytes in a 16-bit value.
 *
 * It is used by rte_bswap16() when the value is constant.
 */
static inline uint16_t
bswap16(uint16_t x)
{
	return (uint16_t)(((x & 0x00ffU) << 8) |
		((x & 0xff00U) >> 8));
}

/**
 * An internal function to swap bytes in a 32-bit value.
 *
 * It is used by rte_bswap32() when the value is constant. 
 */
static inline uint32_t
bswap32(uint32_t x)
{
	return  ((x & 0x000000ffUL) << 24) |
		((x & 0x0000ff00UL) << 8) |
		((x & 0x00ff0000UL) >> 8) |
		((x & 0xff000000UL) >> 24);
}

/**
 * An internal function to swap bytes of a 64-bit value.
 *
 * It is used by rte_bswap64() when the value is constant. 
 */
static inline uint64_t
bswap64(uint64_t x)
{
	return  ((x & 0x00000000000000ffULL) << 56) |
		((x & 0x000000000000ff00ULL) << 40) |
		((x & 0x0000000000ff0000ULL) << 24) |
		((x & 0x00000000ff000000ULL) <<  8) |
		((x & 0x000000ff00000000ULL) >>  8) |
		((x & 0x0000ff0000000000ULL) >> 24) |
		((x & 0x00ff000000000000ULL) >> 40) |
		((x & 0xff00000000000000ULL) >> 56);
}



#endif /* _CONSTANT_BYTEORDER_H_ */
