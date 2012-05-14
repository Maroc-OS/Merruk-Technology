cmd_../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/src/ipc_queues.o := /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-gcc -Wp,-MD,../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/src/.ipc_queues.o.d  -nostdinc -isystem /root/CodeSourcery/Sourcery_G++_Lite/bin/../lib/gcc/arm-none-eabi/4.4.1/include -I/home/administrator/Bureau/Kernel_Dev/GT_S5360/common/arch/arm/include -Iinclude  -I../modules/include -include include/generated/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-bcm215xx/include -Iarch/arm/plat-bcmap/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1136j-s -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -fno-inline-functions-called-once -funit-at-a-time -Wdeclaration-after-statement -fno-strict-overflow -fconserve-stack -I ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/public/ -I ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/ap/ -I ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/ -I ../modules/drivers/char/brcm/fuse_ipc/ -I ../modules/drivers/char/brcm/fuse_log/ -DUNDER_LINUX -DFUSE_APPS_PROCESSOR -DFUSE_DUAL_PROCESSOR_ARCHITECTURE   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(ipc_queues)"  -D"KBUILD_MODNAME=KBUILD_STR(ipc_queues)" -D"DEBUG_HASH=41" -D"DEBUG_HASH2=22" -c -o ../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/src/ipc_queues.o ../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/src/ipc_queues.c

deps_../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/src/ipc_queues.o := \
  ../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/src/ipc_queues.c \
  include/linux/broadcom/csl_types.h \
  include/linux/broadcom/ipcproperties.h \
  ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/ipc_queues.h \
  ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/public/ipc_sharedmemory.h \
  include/linux/broadcom/ipcinterface.h \
    $(wildcard include/config/dpram.h) \
    $(wildcard include/config/brcm/cp/crash/dump.h) \
  ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/public/ipc_endpoints.h \
  ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/public/ipc_trace.h \

../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/src/ipc_queues.o: $(deps_../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/src/ipc_queues.o)

$(deps_../modules/../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/src/ipc_queues.o):
