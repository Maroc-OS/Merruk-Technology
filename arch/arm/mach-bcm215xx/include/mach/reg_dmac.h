/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/include/mach/reg_dmac.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef __REG_BCM215XX_DMAC_H__
#define __REG_BCM215XX_DMAC_H__

#include <mach/io.h>
#include <mach/hardware.h>

#define HW_DMAC_BASE               IO_ADDRESS(BCM21553_DMAC_BASE)

/* DMAC register mapping */
#define ADDR_DMAC_CH7CONFIG        (HW_DMAC_BASE + 0x1F0)
#define ADDR_DMAC_CH8CONFIG        (HW_DMAC_BASE + 0x210)

/* Register bit defines */
#define DMAC_CH7CONFIG_E           0x01

#define DMAC_CH8CONFIG_E           0x01

#endif /*__REG_BCM215XX_DMAC_H__ */
