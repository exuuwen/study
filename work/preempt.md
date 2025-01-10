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


/**
 * DEFINE_IDTENTRY_IRQ - Emit code for device interrupt IDT entry points
 * @func:       Function name of the entry point
 *
 * The vector number is pushed by the low level entry stub and handed
 * to the function as error_code argument which needs to be truncated
 * to an u8 because the push is sign extending.
 *
 * irq_enter/exit_rcu() are invoked before the function body and the
 * KVM L1D flush request is set. Stack switching to the interrupt stack
 * has to be done in the function body if necessary.
 */

/*
 * common_interrupt() handles all normal device IRQ's (the special SMP
 * cross-CPU interrupts have their own entry points).
 */
DEFINE_IDTENTRY_IRQ(common_interrupt)
{
        struct pt_regs *old_regs = set_irq_regs(regs);

        if (unlikely(call_irq_handler(vector, regs)))
                apic_eoi();

        set_irq_regs(old_regs);
}


#define DEFINE_IDTENTRY_IRQ(func)                                       \
static void __##func(struct pt_regs *regs, u32 vector);                 \
                                                                        \
__visible noinstr void func(struct pt_regs *regs,                       \
                            unsigned long error_code)                   \
{                                                                       \
        irqentry_state_t state = irqentry_enter(regs);                  \
        u32 vector = (u32)(u8)error_code;                               \
                                                                        \
        instrumentation_begin();                                        \
        run_irq_on_irqstack_cond(__##func, regs, vector);               \
        instrumentation_end();                                          \
        irqentry_exit(regs, state);                                     \
}                                                                       \
    
---> translate to

DEFINE_IDTENTRY_IRQ(common_interrupt)
{
	irq_enter_rcu();
        handle_irq(desc, regs);
	irq_exit_rcu();
	irqentry_exit();
}


irq_enter_rcu 
{
	preempt_count_add(HARDIRQ_OFFSET);
}


irq_exit_rcu
{
	preempt_count_sub(HARDIRQ_OFFSET);

	if (!in_interrupt() && local_softirq_pending())
		invoke_softirq();
}

static void run_ksoftirqd(unsigned int cpu)
{
        ksoftirqd_run_begin(); //local_irq_disable
        if (local_softirq_pending()) {
                /*
                 * We can safely run softirq on inline stack, as we are not deep
                 * in the task stack here.
                 */
                handle_softirqs(true);
                ksoftirqd_run_end();
                cond_resched();
                return;
        }
        ksoftirqd_run_end();// local_irq_enable
}


invoke_softirq ---> run_ksoftirqd
{
	__local_bh_disable_ip(_RET_IP_, SOFTIRQ_OFFSET); ---> preempt_count_add(SOFTIRQ_OFFSET)
	local_irq_enable();

	xxxxxxx handle_sofirq things

	local_irq_disable(); 
	__local_bh_enable(SOFTIRQ_OFFSET); ----> preempt_count_sub(SOFTIRQ_OFFSET)
}

schedule point:
1. interrupt to kernel
	irqentry_exit();
        ----> if (user_mode(regs))
                 irqentry_exit_to_user_mode(regs);
              else
                 if (IS_ENABLED(CONFIG_PREEMPTION))
                      irqentry_exit_cond_resched(); 

/*
retint_kernel:
#ifdef CONFIG_PREEMPT
        /* Interrupts are off */
        /* Check if we need preemption */
        btl     $9, EFLAGS(%rsp)                /* were interrupts off? */
        jnc     1f
0:      cmpl    $0, PER_CPU_VAR(__preempt_count)
        jnz     1f
        call    preempt_schedule_irq
*/

2. 
#define preempt_enable() \
do { \
        barrier(); \
        if (unlikely(preempt_count_dec_and_test())) \
                __preempt_schedule(); \
} while (0)

The spin_unlock_(bh/irq), local_bh_enable 

