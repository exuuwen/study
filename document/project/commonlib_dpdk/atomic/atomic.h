#ifndef _X86_ATOMIC_H_
#define _X86_ATOMIC_H_
#include "../CommonLib.h"

#define MAX_LCORE 2
/*
 * This file provides an API for atomic operations on X86.
 */

#if MAX_LCORE == 1
#define	MPLOCKED /**< No need to insert MP lock prefix. */
#else
#define	MPLOCKED	"lock ; " /**< Insert MP lock prefix. */
#endif
/**
 * General memory barrier.
 *
 * Guarantees that the LOAD and STORE operations generated before the
 * barrier occur before LOAD and STORE operations generated after.
 */
#define	mb()  asm volatile(MPLOCKED "addl $0,(%%esp)" : : : "memory")

/**
 * Write memory barrier.
 *
 * Guarantees that the STORE operations generated before the barrier
 * occur before the STORE operations generated after.
 */
#define	wmb() asm volatile(MPLOCKED "addl $0,(%%esp)" : : : "memory")

/**
 * Read memory barrier.
 *
 * Guarantees that the LOAD operations generated before the barrier
 * occur before the STORE operations generated after.
 */
#define	rmb() asm volatile(MPLOCKED "addl $0,(%%esp)" : : : "memory")


/**
 * Atomic compare and set.
 *
 * (atomic) equivalent to:
 *   if (*dst == exp)
 *     *dst = src (all 32-bit words)
 *
 * @return
 *   Non-zero on success; 0 on failure.
 */
static inline int
atomic_cmpset(volatile uint32_t *dst, uint32_t exp, uint32_t src)
{
	uint8_t res;

	asm volatile(
		     "	" MPLOCKED "			"
		     "	cmpxchgl %[src],%[dst] ;	"
		     "       sete	%[res] ;	"
		     : [res] "=a" (res),	/* 0 */
		     [dst] "=m" (*dst)		/* 1 */
		     : [src] "r" (src),		/* 2 */
		     "a" (exp),			/* 3 */
		     "m" (*dst)			/* 4 */
		     : "memory");

	return res;
}

/**
 * The atomic counter structure.
 */
typedef struct {
	volatile int32_t cnt; /**< An internal counter value. */
} atomic_t;

/**
 * Initialize an atomic counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void
atomic_init(atomic_t *v)
{
	v->cnt = 0;
}

/**
 * Atomically read a 32-bit value from a counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   The value of the counter.
 */
static inline int32_t
atomic_read(atomic_t *v)
{
	return v->cnt;
}

/**
 * Atomically set a counter to a 32-bit value.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param new
 *   The new value for the counter.
 */
static inline void
atomic_set(atomic_t *v, int32_t new)
{
	v->cnt = new;
}

/**
 * Atomically add a 32-bit value to an atomic counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param inc
 *   The value to be added to the counter.
 */
static inline void
atomic_add(atomic_t *v, int32_t inc)
{
	asm volatile(MPLOCKED
		     "addl %[inc],%[cnt]"
		     : [cnt] "=m" (v->cnt) /* output (0) */
		     : [inc] "ir" (inc),   /* input (1) */
		       "m" (v->cnt)        /* input (2) */
		     );                    /* no clobber-list */
}

/**
 * Atomically subtract a 32-bit value from an atomic counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param dec
 *   The value to be substracted from the counter.
 */
static inline void
atomic_sub(atomic_t *v, int32_t dec)
{
	asm volatile(MPLOCKED
		     "subl %[dec],%[cnt]"
		     : [cnt] "=m" (v->cnt) /* output (0) */
		     : [dec] "ir" (dec),   /* input (1) */
		       "m" (v->cnt)        /* input (2) */
		     );                    /* no clobber-list */
}

/**
 * Atomically increment a counter by one.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void
atomic_inc(atomic_t *v)
{
	asm volatile(MPLOCKED
		     "incl %[cnt]"
		     : [cnt] "=m" (v->cnt) /* output (0) */
		     : "m" (v->cnt)        /* input (1) */
		     );                    /* no clobber-list */
}

/**
 * Atomically decrement a counter by one.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void
atomic_dec(atomic_t *v)
{
	asm volatile(MPLOCKED
		     "decl %[cnt]"
		     : [cnt] "=m" (v->cnt) /* output (0) */
		     : "m" (v->cnt)        /* input (1) */
		     );                    /* no clobber-list */
}

/**
 * Atomically add a 32-bit value to a counter and return the result.
 *
 * Atomically adds the 32-bits value (inc) to the atomic counter (v) and
 * returns the value of v after addition.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param inc
 *   The value to be added to the counter.
 * @return
 *   The value of v after the addition.
 */
static inline int32_t
atomic_add_return(atomic_t *v, int32_t inc)
{
	int32_t prev = inc;

	asm volatile(MPLOCKED
		     "xaddl %[prev], %[cnt]"
		     : [prev] "+r" (prev),    /* output (0) */
		       [cnt] "=m" (v->cnt)    /* output (1) */
		     : "m" (v->cnt)           /* input (2) */
		     );                       /* no clobber-list */
	return prev + inc;
}

/**
 * Atomically subtract a 32-bit value from a counter and return
 * the result.
 *
 * Atomically subtracts the 32-bit value (inc) from the atomic counter
 * (v) and returns the value of v after the substraction.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param dec
 *   The value to be substracted from the counter.
 * @return
 *   The value of v after the substraction.
 */
static inline int32_t
atomic_sub_return(atomic_t *v, int32_t dec)
{
	return atomic_add_return(v, -dec);
}


/**
 * Atomically increment a 32-bit counter by one and test.
 *
 * Atomically increments the atomic counter (v) by one and returns true if
 * the result is 0, or false in all other cases.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   True if the result after the increment operation is 0; false otherwise.
 */
static inline int atomic_inc_and_test(atomic_t *v)
{
	uint8_t ret;

	asm volatile(MPLOCKED
		     "incl %[cnt] ; "
		     "sete %[ret]"
		     : [cnt] "+m" (v->cnt),   /* output (0) */
		       [ret] "=qm" (ret)      /* output (1) */
		     :                        /* no input */
		     );                       /* no clobber-list */
	return ret != 0;
}

/**
 * Atomically decrement a 32-bit counter by one and test.
 *
 * Atomically decrements the atomic counter (v) by one and returns true if
 * the result is 0, or false in all other cases.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   True if the result after the decrement operation is 0; false otherwise.
 */
static inline int atomic_dec_and_test(atomic_t *v)
{
	uint8_t ret;

	asm volatile(MPLOCKED
		     "decl %[cnt] ; "
		     "sete %[ret]"
		     : [cnt] "+m" (v->cnt),   /* output (0) */
		       [ret] "=qm" (ret)      /* output (1) */
		     :                        /* no input */
		     );                       /* no clobber-list */
	return ret != 0;
}


/**
 * Atomically set a 32-bit counter to 0.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void atomic_clear(atomic_t *v)
{
	v->cnt = 0;
}



#endif /* _X86_ATOMIC_H_ */
