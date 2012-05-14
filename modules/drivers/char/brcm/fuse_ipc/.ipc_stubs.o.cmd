cmd_../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_stubs.o := /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-gcc -Wp,-MD,../modules/../modules/drivers/char/brcm/fuse_ipc/.ipc_stubs.o.d  -nostdinc -isystem /root/CodeSourcery/Sourcery_G++_Lite/bin/../lib/gcc/arm-none-eabi/4.4.1/include -I/home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include -Iinclude  -I../modules/include -include include/generated/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-bcm215xx/include -Iarch/arm/plat-bcmap/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1136j-s -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -fno-inline-functions-called-once -funit-at-a-time -Wdeclaration-after-statement -fno-strict-overflow -fconserve-stack -Werror -I ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/public/ -I ../modules/drivers/char/brcm/fuse_log/ -DRPC_INCLUDED -DUNDER_LINUX   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(ipc_stubs)"  -D"KBUILD_MODNAME=KBUILD_STR(ipc_stubs)" -D"DEBUG_HASH=19" -D"DEBUG_HASH2=54" -c -o ../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_stubs.o ../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_stubs.c

deps_../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_stubs.o := \
  ../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_stubs.c \
  ../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_stubs.h \
  include/linux/broadcom/csl_types.h \
  arch/arm/plat-bcmap/include/plat/timer.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \

../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_stubs.o: $(deps_../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_stubs.o)

$(deps_../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_stubs.o):
