/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/include/mach/reg_auxmic.h
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

#ifndef __REG_BCM21553_AUXMIC_H__
#define __REG_BCM21553_AUXMIC_H__

#include <mach/io.h>
#include <mach/hardware.h>

#define HW_AUXMIC_BASE                      IO_ADDRESS(BCM21553_AUXMIC_BASE)

/*
 * Auxiliary Microphone Detection Register Offsets
 */
#define AUXMIC_PRB_CYC			0x00
#define   AUXMIC_PRB_CYC_MS(x)		(ffs(x >> 3))
#define AUXMIC_MSR_DLY			0x04
#define   AUXMIC_MSR_DLY_MS(x)		(ffs(x >> 1))
#define AUXMIC_MSR_INTVL		0x08
#define   AUXMIC_MSR_INTVL_MS(x)	(ffs(x >> 2))
#define AUXMIC_CMC			0x0c
#define   AUXMIC_CMC_PROB_CYC_INF	0x1
#define AUXMIC_MIC			0x10
#define   AUXMIC_MIC_SPLT_MSR_INTR	0x1
#define AUXMIC_AUXEN			0x14
#define   AUXMIC_ENABLE_BIAS		0x1
#define AUXMIC_MICINTH_ADJ		0x18
#define   AUXMIC_MICINTH_ADJ_VAL(x)	((x) & 0x7f)
#define AUXMIC_MICINENTH_ADJ		0x1c
#define   AUXMIC_MICINENTH_ADJ_VAL(x)	((x) & 0x7f)
#define AUXMIC_MICONTH_ADJ		0x20
#define   AUXMIC_MICONTH_ADJ_VAL(x)	((x) & 0x7f)
#define AUXMIC_MICONENTH_ADJ		0x24
#define   AUXMIC_MICONENTH_ADJ_VAL(x)	((x) & 0x7f)
#define AUXMIC_F_PWRDWN			0x28
#define   AUXMIC_F_PWRDWN_ENABLE	0x1

#endif /* __REG_BCM21553_AUXMIC_H__ */
