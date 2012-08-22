/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/include/mach/reg_emi.h
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

#ifndef __REG_BCM21553_EMI_H__
#define __REG_BCM21553_EMI_H__

#include <mach/io.h>
#include <mach/hardware.h>

#define HW_EMI_BASE                       IO_ADDRESS(BCM21553_EMI_BASE)

/* EMI module register mapping */
#define ADDR_EMI_PWR_DOWN_MODE            (HW_EMI_BASE + 0x18)
#define ADDR_EMI_REQUEST_MASK             (HW_EMI_BASE + 0x60)


#endif /*__REG_BCM21553_EMI_H__*/
