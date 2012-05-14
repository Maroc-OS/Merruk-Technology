cmd_../modules/../modules/drivers/char/brcm/fuse_log/fifo.o := /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-gcc -Wp,-MD,../modules/../modules/drivers/char/brcm/fuse_log/.fifo.o.d  -nostdinc -isystem /root/CodeSourcery/Sourcery_G++_Lite/bin/../lib/gcc/arm-none-eabi/4.4.1/include -I/home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include -Iinclude  -I../modules/include -include include/generated/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-bcm215xx/include -Iarch/arm/plat-bcmap/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1136j-s -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -fno-inline-functions-called-once -funit-at-a-time -Wdeclaration-after-statement -fno-strict-overflow -fconserve-stack   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(fifo)"  -D"KBUILD_MODNAME=KBUILD_STR(fifo)" -D"DEBUG_HASH=17" -D"DEBUG_HASH2=61" -c -o ../modules/../modules/drivers/char/brcm/fuse_log/fifo.o ../modules/../modules/drivers/char/brcm/fuse_log/fifo.c

deps_../modules/../modules/drivers/char/brcm/fuse_log/fifo.o := \
  ../modules/../modules/drivers/char/brcm/fuse_log/fifo.c \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/posix_types.h \
  /root/CodeSourcery/Sourcery_G++_Lite/bin/../lib/gcc/arm-none-eabi/4.4.1/include/stdarg.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/string.h \
  ../modules/../modules/drivers/char/brcm/fuse_log/fifo.h \
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
  include/linux/linkage.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/linkage.h \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/bitops.h \
    $(wildcard include/config/smp.h) \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/system.h \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/arch/has/barriers.h) \
    $(wildcard include/config/arm/dma/mem/bufferable.h) \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
    $(wildcard include/config/cpu/32v6k.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  include/linux/typecheck.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/irqflags.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/hwcap.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/outercache.h \
    $(wildcard include/config/outer/cache/sync.h) \
    $(wildcard include/config/outer/cache.h) \
  include/asm-generic/cmpxchg-local.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/arch_hweight.h \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/lock.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/dynamic_debug.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  /home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  ../modules/../modules/drivers/char/brcm/fuse_log/output.h \

../modules/../modules/drivers/char/brcm/fuse_log/fifo.o: $(deps_../modules/../modules/drivers/char/brcm/fuse_log/fifo.o)

$(deps_../modules/../modules/drivers/char/brcm/fuse_log/fifo.o):
