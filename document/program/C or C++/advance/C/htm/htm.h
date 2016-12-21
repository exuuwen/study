#ifndef HTM_H_H
#define HTM_H_H

#define _XBEGIN_STARTED      (~0u)
#define _XABORT_EXPLICIT     (1 << 0)
#define _XABORT_RETRY        (1 << 1)
#define _XABORT_CONFLICT     (1 << 2)
#define _XABORT_CAPACITY     (1 << 3)
#define _XABORT_DEBUG        (1 << 4)
#define _XABORT_NESTED       (1 << 5)
#define _XABORT_CODE(x)      (((x) >> 24) & 0xff)

static __attribute__((__always_inline__)) inline
unsigned int _xbegin(void)
{

	unsigned int ret = _XBEGIN_STARTED;

	asm volatile(".byte 0xc7,0xf8 ; .long 0" : "+a" (ret) :: "memory");
        
	return ret;
}

static __attribute__((__always_inline__)) inline
void _xend(void)
{
	asm volatile(".byte 0x0f,0x01,0xd5" ::: "memory");
}

#define _xabort(status) do { \
	asm volatile(".byte 0xc6,0xf8,%P0" :: "i" (status) : "memory"); \
} while (0)


#endif

