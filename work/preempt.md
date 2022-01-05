/*
 * We put the hardirq and softirq counter into the preemption
 * counter. The bitmask has the following meaning:
 *
 * - bits 0-7 are the preemption count (max preemption depth: 256)
 * - bits 8-15 are the softirq count (max # of softirqs: 256)
 *
 * The hardirq count could in theory be the same as the number of
 * interrupts in the system, but we run all interrupt handlers with
 * interrupts disabled, so we cannot have nesting interrupts. Though
 * there are a few palaeontologic drivers which reenable interrupts in
 * the handler, so we need more than one bit here.
 *
 *         PREEMPT_MASK:        0x000000ff
 *         SOFTIRQ_MASK:        0x0000ff00
 *         HARDIRQ_MASK:        0x000f0000
 *             NMI_MASK:        0x00100000
 * PREEMPT_NEED_RESCHED:        0x80000000
 */     

#define hardirq_count() (preempt_count() & HARDIRQ_MASK)
#define softirq_count() (preempt_count() & SOFTIRQ_MASK)
#define irq_count()     (preempt_count() & (HARDIRQ_MASK | SOFTIRQ_MASK \
                                 | NMI_MASK))

/*
 * Are we doing bottom half or hardware interrupt processing?
 *
 * in_irq()       - We're in (hard) IRQ context
 * in_softirq()   - We have BH disabled, or are processing softirqs
 * in_interrupt() - We're in NMI,IRQ,SoftIRQ context or have BH disabled
 * in_serving_softirq() - We're in softirq context
 * in_nmi()       - We're in NMI context
 * in_task()      - We're in task context
 *      
 * Note: due to the BH disabled confusion: in_softirq(),in_interrupt() really
 *       should not be used in new code.
 */     
#define in_irq()                (hardirq_count())
#define in_softirq()            (softirq_count())
#define in_interrupt()          (irq_count())
#define in_serving_softirq()    (softirq_count() & SOFTIRQ_OFFSET)
#define in_nmi()                (preempt_count() & NMI_MASK)
#define in_task()               (!(preempt_count() & \
                                   (NMI_MASK | HARDIRQ_MASK | SOFTIRQ_OFFSET)))

local_bh_disable ---> preempt_count_add(SOFTIRQ_DISABLE_OFFSET)
local_bh_enable ---> preempt_count_sub(SOFTIRQ_DISABLE_OFFSET)

do_IRQ 
{
	irq_enter();
	irq_exit();
}

irq_enter 
{
	preempt_count_add(HARDIRQ_OFFSET);
}


irq_exit
{
	local_irq_disable(); //why

	preempt_count_sub(HARDIRQ_OFFSET);

	if (!in_interrupt() && local_softirq_pending())
		invoke_softirq();
}

invoke_softirq ---> __do_softirq
{
	__local_bh_disable_ip(_RET_IP_, SOFTIRQ_OFFSET); ---> preempt_count_add(SOFTIRQ_OFFSET)
	local_irq_enable();

	xxxxxxx handle_sofirq things

	local_irq_disable(); //why
	__local_bh_enable(SOFTIRQ_OFFSET); ----> preempt_count_sub(SOFTIRQ_OFFSET)
}




schedule point:
1. interrupt to kernel

retint_kernel:
#ifdef CONFIG_PREEMPT
        /* Interrupts are off */
        /* Check if we need preemption */
        btl     $9, EFLAGS(%rsp)                /* were interrupts off? */
        jnc     1f
0:      cmpl    $0, PER_CPU_VAR(__preempt_count)
        jnz     1f
        call    preempt_schedule_irq


2. 
#define preempt_enable() \
do { \
        barrier(); \
        if (unlikely(preempt_count_dec_and_test())) \
                __preempt_schedule(); \
} while (0)

The spin_unlock_(bh/irq), local_bh_enable 

