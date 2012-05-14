#Makefile for KPDP 
#EXTRA_CFLAGS := -Werror


# common definitons for all capi2 related include paths
include ../modules/drivers/char/brcm/fuse_ril/CAPI2_CIB/capi2_include_paths.gmk

EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_log/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/public/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc_CIB/arpc/inc/
#EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc_CIB/srpc/inc/
#EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ril/crypto/

# used to prevent redefinition of U_CHAR in various CIB header files
EXTRA_CFLAGS += -D_BSDTYPES_DEFINED -DTYPEDEF_U_CHAR -DTYPEDEF_U_SHORT
EXTRA_CFLAGS += -DFUSE_APPS_PROCESSOR -DFUSE_DUAL_PROCESSOR_ARCHITECTURE

EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -D_BCM2153_
EXTRA_CFLAGS += -DUMTS


obj-$(CONFIG_BRCM_FUSE_RIL_CIB) += \
	bcm_kpdp_main.o \
	bcm_kpdp_ioctl.o \
	bcm_kpdp_cmd_handler.o \
	bcm_kpdp_capi2_handler.o \
	bcm_kpdp_notify_handler.o \
	bcm_pdp_handler.o

