#/*******************************************************************************************
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
#*******************************************************************************************/
EXTRA_CFLAGS += -Wno-strict-prototypes 
ifeq ($(CONFIG_ARCH_BCM116X),y)
	obj-$(CONFIG_BRCM_XSCRIPT_DMA) += xscript_dma.o
endif

ifeq ($(CONFIG_ARCH_BCM215XX),y)
	        obj-$(CONFIG_BRCM_XSCRIPT_DMA) += xscript_dma_osdal.o
endif
