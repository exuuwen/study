#ifndef _X86_BYTEORDER_H_
#define _X86_BYTEORDER_H_
#include "../../CommonLib.h"
/**
 * An architecture-optimized byte swap for a 16-bit value.
 *
 * Do not use this function directly. The preferred function is rte_bswap16().
 */
static inline uint16_t bswap16(uint16_t _x)
{
	register uint16_t x = _x;
	asm volatile ("xchgb %b[x1],%h[x2]"
		      : [x1] "=q" (x)
		      : [x2] "0" (x)
		      );
	return x;
}

/**
 * An architecture-optimized byte swap for a 32-bit value.
 *
 * Do not use this function directly. The preferred function is rte_bswap32().
 */
static inline uint32_t bswap32(uint32_t _x)
{
	register uint32_t x = _x;
	asm volatile ("bswap %[x]"
		      : [x] "+r" (x)
		      );
	return x;
}

/**
 * An architecture-optimized byte swap for a 64-bit value.
 *
  * Do not use this function directly. The preferred function is rte_bswap64().
 */
static inline uint64_t bswap64(uint64_t x)
{
	uint64_t ret = 0;
	ret |= ((uint64_t)bswap32(x & 0xffffffffUL) << 32);
	ret |= ((uint64_t)bswap32((x >> 32) & 0xffffffffUL));
	return ret;
}

#endif /* _X86_BYTEORDER_H_ */
