#*******************************************************************************************
#Copyright 2010 Broadcom Corporation.  All rights reserved.
#
#Unless you and Broadcom execute a separate written software license agreement
#governing use of this software, this software is licensed to you under the
#terms of the GNU General Public License version 2, available at
#http://www.gnu.org/copyleft/gpl.html (the "GPL").
#
#Notwithstanding the above, under no circumstances may you combine this software
#in any way with any other Broadcom software provided under a license other than
#the GPL, without Broadcom's express prior written consent.
#*******************************************************************************************

EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DSTACK_wedge

EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/public/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ril/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_log/
include ../modules/drivers/char/brcm/fuse_ril/CAPI2_CIB/capi2_include_paths.gmk

obj-$(CONFIG_BRCM_CNTIN) = gps_cntin_driver.o
obj-$(CONFIG_BRCM_CNTIN) += gps_driver.o
