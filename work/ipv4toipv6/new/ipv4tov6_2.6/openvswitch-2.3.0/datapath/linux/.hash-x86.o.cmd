cmd_/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/hash-x86.o := gcc -Wp,-MD,/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/.hash-x86.o.d  -nostdinc -isystem /usr/lib/gcc/x86_64-redhat-linux/4.4.7/include -I/root/ipv4tov6_2.6/openvswitch-2.3.0/include -I/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat -I/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include -Iinclude  -I/usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include -include include/linux/autoconf.h -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -m64 -mtune=generic -mno-red-zone -mcmodel=kernel -funit-at-a-time -maccumulate-outgoing-args -fstack-protector -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -Wframe-larger-than=2048 -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -pg -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -fconserve-stack -DVERSION=\"2.3.0\" -I/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/.. -I/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/.. -g -include /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/kcompat.h  -DMODULE -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(hash_x86)"  -D"KBUILD_MODNAME=KBUILD_STR(openvswitch)" -D"DEBUG_HASH=4" -D"DEBUG_HASH2=17" -c -o /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/.tmp_hash-x86.o /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/hash-x86.c

deps_/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/hash-x86.o := \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/hash-x86.c \
    $(wildcard include/config/x86.h) \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/kcompat.h \
  include/linux/version.h \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/linux/hash.h \
  include/linux/hash.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/types.h \
    $(wildcard include/config/x86/64.h) \
    $(wildcard include/config/highmem64g.h) \
  include/asm-generic/types.h \
    $(wildcard include/config/phys/addr/t/64bit.h) \
  include/asm-generic/int-ll64.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
    $(wildcard include/config/64bit.h) \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/asm/hash.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/processor.h \
    $(wildcard include/config/x86/vsmp.h) \
    $(wildcard include/config/x86/32.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/paravirt.h) \
    $(wildcard include/config/m386.h) \
    $(wildcard include/config/m486.h) \
    $(wildcard include/config/x86/debugctlmsr.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/processor-flags.h \
    $(wildcard include/config/vm86.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/vm86.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/ptrace.h \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/linux/compiler.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/linux/compiler-gcc.h \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/ptrace-abi.h \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/include/linux/types.h \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/linux/types.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
  include/linux/posix_types.h \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/linux/stddef.h \
  include/linux/stddef.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/posix_types.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/posix_types_64.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/segment.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/cache.h \
    $(wildcard include/config/x86/l1/cache/shift.h) \
  include/linux/linkage.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/linkage.h \
    $(wildcard include/config/x86/alignment/16.h) \
  include/linux/stringify.h \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/math_emu.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/sigcontext.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/current.h \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/asm/percpu.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/percpu.h \
    $(wildcard include/config/x86/64/smp.h) \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/linux/kernel.h \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /usr/lib/gcc/x86_64-redhat-linux/4.4.7/include/stdarg.h \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/bitops.h \
    $(wildcard include/config/x86/cmov.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/alternative.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/asm.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/cpufeature.h \
    $(wildcard include/config/x86/invlpg.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/required-features.h \
    $(wildcard include/config/x86/minimum/cpu/family.h) \
    $(wildcard include/config/math/emulation.h) \
    $(wildcard include/config/x86/pae.h) \
    $(wildcard include/config/x86/cmpxchg64.h) \
    $(wildcard include/config/x86/use/3dnow.h) \
    $(wildcard include/config/x86/p6/nop.h) \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/swab.h \
    $(wildcard include/config/x86/bswap.h) \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/minix.h \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/linux/log2.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/typecheck.h \
  include/linux/ratelimit.h \
  include/linux/param.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/param.h \
  include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/dynamic_debug.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  include/asm-generic/percpu.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/system.h \
    $(wildcard include/config/ia32/emulation.h) \
    $(wildcard include/config/x86/32/lazy/gs.h) \
    $(wildcard include/config/x86/ppro/fence.h) \
    $(wildcard include/config/x86/oostore.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/cmpxchg.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/cmpxchg_64.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/nops.h \
    $(wildcard include/config/mk7.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/irqflags.h \
    $(wildcard include/config/debug/lock/alloc.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/paravirt.h \
    $(wildcard include/config/transparent/hugepage.h) \
    $(wildcard include/config/paravirt/spinlocks.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/pgtable_types.h \
    $(wildcard include/config/kmemcheck.h) \
    $(wildcard include/config/compat/vdso.h) \
    $(wildcard include/config/proc/fs.h) \
  include/linux/const.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/page_types.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/page_64_types.h \
    $(wildcard include/config/physical/start.h) \
    $(wildcard include/config/physical/align.h) \
    $(wildcard include/config/flatmem.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/pgtable_64_types.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/paravirt_types.h \
    $(wildcard include/config/x86/local/apic.h) \
    $(wildcard include/config/paravirt/debug.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/desc_defs.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/kmap_types.h \
    $(wildcard include/config/debug/highmem.h) \
  include/asm-generic/kmap_types.h \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/linux/cpumask.h \
  include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
  include/linux/bitmap.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/string.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/string_64.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/page.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/page_64.h \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
    $(wildcard include/config/sparsemem.h) \
  include/asm-generic/getorder.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/msr.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/msr-index.h \
  include/linux/ioctl.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/cpumask.h \
  include/linux/personality.h \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/linux/math64.h \
  /usr/src/kernels/2.6.32-279.19.40.el6.ucloud.x86_64/arch/x86/include/asm/div64.h \
  include/asm-generic/div64.h \
  /root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/compat/include/linux/err.h \
  include/linux/err.h \

/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/hash-x86.o: $(deps_/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/hash-x86.o)

$(deps_/root/ipv4tov6_2.6/openvswitch-2.3.0/datapath/linux/hash-x86.o):
