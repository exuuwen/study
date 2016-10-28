#include "x86_byteorder.h"
static volatile uint16_t u_16 = 0x1337;
static volatile uint32_t u_32 = 0xdeadbeefUL;
static volatile uint64_t u_64 = 0xdeadcafebabefaceULL;

/*
 * Byteorder functions
 * ===================
 *
 * - check that optimized byte swap functions are working for each
 *   size (16, 32, 64 bits)
 */

int
main(void)
{
	uint16_t res_u16;
	uint32_t res_u32;
	uint64_t res_u64;

	res_u16 = bswap16(u_16);
	printf("0x%x -> 0x%x\n", u_16, res_u16);
	if (res_u16 != 0x3713)
		return -1;

	res_u32 = bswap32(u_32);
	printf("0x%x -> 0x%x\n", u_32, res_u32);
	if (res_u32 != 0xefbeaddeUL)
		return -1;

	res_u64 = bswap64(u_64);
	printf("0x%llx -> 0x%llx\n", u_64, res_u64);
	if (res_u64 != 0xcefabebafecaaddeULL)
		return -1;

	return 0;
}
