#Makefile for IPC driver
EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/public/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_log/
EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX

obj-$(CONFIG_BRCM_FUSE_IPC_CIB) += \
	lnx_support.o \
	ipc_server_ccb.o \
	ipc_server.o \
	ipc_stubs.o \
	ipc_crashsupport.o \
	ipc_CIB/

