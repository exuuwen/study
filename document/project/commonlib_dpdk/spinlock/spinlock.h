/*-
Description:

This file is used to implement a spin lock in the user space. It is based on the 

x86 asm code with high performance.

This class only works under x86 HW platform.

The reason why this spin lock architecture dependent implementation working well

under Intel SMP platform, multicore, is memory barrier is used to lock the FSB, 

Front Serial Bus.

XCHG will trigger 'lock' signal in the bus, which is atomically.

AT&T sytax is used in the code, instead of Intel. The difference:

1, Register name

AT&T:  %eax 

Intel: eax

There is a prefix

2, integer

AT&T: movl $0 %eax

Intel: mov eax, 0x0

3, addressing

// embedded asm syntax include four parts, which is seperated by ":". 

// __asm__(asm template: output: input : destroy description part);

// asm template is mandatory, the other three parts are optinal.

Notes for the readability.

1, "=m" (s) stands for s is a memory variable. + stands it is a output write operator

2, "=r" (s) stands for s is a register variable.

3, "memory" tells GCC memory are changed, write the register, cache variable back to memory.

also it takes memory barrier functionality.

4, % is used as prefix of register, in order to keep % in the complied program, %% is used.

just like "\" in the c programming.

5, %0, %1 can up to 10 parameters, it can be input or output variables. You can name it a 

alias for readability. such as lock and oldval.

6, $0 stands for 0, $1 stands for 1.

Author: Tom, Zhang Jiangtao

History:

12/023/2011 WX  Initial code.

 */

#ifndef _X86_SPINLOCK_H_
#define _X86_SPINLOCK_H_
#include "../CommonLib.h"


/**
 * The spinlock_t type.
 */
typedef struct {
	volatile int locked; /**< lock status 0 = unlocked, 1 = locked */
} spinlock_t;



/**
 * Initialize the spinlock to an unlocked state.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
spinlock_init(spinlock_t *sl)
{
	sl->locked = 0;
}

/**
 * Take the spinlock.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
spinlock_lock(spinlock_t *sl)
{
	asm volatile ("mov $1, %%eax\n"     // put 1 into eax register
		      "1:\n"											// lable 1
		      "xchg %[locked], %%eax\n"		// atomically exchange memory variable s with eax register value
		      "test %%eax, %%eax\n"				// test eax
		      "jz 3f\n"										// jump to lable 3 if zero. end
		      "2:\n"											// lable 2
		      "cmp $0, %[locked]\n"				// s - 0 compare
		      "jnz 2b\n"									// jump to lable 2 if not zero
		      "jmp 1b\n"									// jump to lable 1
		      "3:\n"											// lable 3
		      : [locked] "=m" (sl->locked)// output memory value s
		      :														// input is empty
		      : "memory", "%eax");				// writethrough to memory

}

/**
 * Release the spinlock.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
spinlock_unlock (spinlock_t *sl)
{
	asm volatile ("xor %%eax, %%eax\n"	// exclusive or, assign it 0 back to eax.
		      "xchg %[locked], %%eax\n"		// exchange s with eax
		      : [locked] "=m" (sl->locked)// output value
		      :														// input value
		      : "memory", "%eax");				// write through memory
}

/**
 * Try to take the lock.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline int
spinlock_trylock (spinlock_t *sl)
{
	int oldval = 0;

	asm volatile ("mov $1, %%eax\n"			 // put 1 into eax
		      "xchg %[locked], %%eax\n"		 // exchange s with eax
		      "mov %%eax, %[oldval]"			 // move eax value to old value
		      : [locked] "=m" (sl->locked),// output value s
			[oldval] "=r" (oldval)					 // output value oldval
		      :														 // input is empty
		      : "memory", "%eax");				 // memory


	return (oldval == 0);
}

/**
 * Test if the lock is taken.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the lock is currently taken; 0 otherwise.
 */
static inline int spinlock_is_locked (spinlock_t *sl)
{
	return sl->locked;
}

#endif /* _X86_SPINLOCK_H_ */
