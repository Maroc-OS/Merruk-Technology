/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/bcm21553_cpuidle_drv.h
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

#ifndef BCM21553_CPUIDLE_DRV_H
#define BCM21553_CPUIDLE_DRV_H

/* CPU states */
enum {
	BCM21553_STATE_C0,	/* ARM: WFI */
	BCM21553_STATE_C1,	/* ARM: WFI,Pedestal */
	BCM21553_STATE_C2,	/* ARM: WFI,Pedestal,Dormant */
	BCM21553_MAX_CPU_STATES,
};

#endif /* BCM21553_CPUIDLE_DRV_H */
