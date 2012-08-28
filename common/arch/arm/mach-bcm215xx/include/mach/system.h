/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/include/mach/system.h
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

#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#define		SOFTRST_SOFTRST		0x00000008

#include <plat/bcm_watchdog.h>
#include <plat/syscfg.h>
#include <asm/outercache.h>
#include <asm/cacheflush.h>

static inline void arch_idle(void)
{
	/*
	 * This should do all the clock switching
	 * and wait for interrupt tricks
	 */
	cpu_do_idle();
}

#define REBOOT_PREFIX		0x12345670
#define REBOOT_MODE_NONE	0
#define REBOOT_MODE_DOWNLOAD	1
#define REBOOT_MODE_UPLOAD	2
#define REBOOT_MODE_CHARGING	3
#define REBOOT_MODE_RECOVERY	4
#define REBOOT_MODE_ARM11_FOTA	5
#define REBOOT_MODE_CPREBOOT	6
#define REBOOT_MODE_RECOVERY_DONE	7

#define BCM215XXINFORM 0x28004000
#define BCM215XXINFORM3 0x28004010
static inline void arch_reset(char mode, const char *cmd)
{
	writel(0x12345678, BCM215XXINFORM3);	/* Don't enter lpm mode */

	if (!cmd) {
//		writel(REBOOT_PREFIX | REBOOT_MODE_NONE, BCM215XXINFORM3);
		panic("Illegal reset attemption");
	} else {
		if (!strcmp(cmd, "arm11_fota"))
			writel(REBOOT_PREFIX | REBOOT_MODE_ARM11_FOTA,
			       BCM215XXINFORM3);
		else if (!strcmp(cmd, "recovery"))
			writel(REBOOT_PREFIX | REBOOT_MODE_RECOVERY,
			       BCM215XXINFORM3);
		else if (!strcmp(cmd, "lpm_reboot"))
			writel(REBOOT_PREFIX | REBOOT_MODE_CHARGING,
			       BCM215XXINFORM3);		
		else if (!strcmp(cmd, "download"))
			writel(REBOOT_PREFIX | REBOOT_MODE_DOWNLOAD,
			       BCM215XXINFORM3);
		else if (!strcmp(cmd, "upload"))
			writel(REBOOT_PREFIX | REBOOT_MODE_UPLOAD,
			       BCM215XXINFORM3);
		else if (!strcmp(cmd, "recovery_done"))
			writel(REBOOT_PREFIX | REBOOT_MODE_RECOVERY_DONE,
			       BCM215XXINFORM3);
		else
			writel(REBOOT_PREFIX | REBOOT_MODE_NONE,
			       BCM215XXINFORM3);

		if (strcmp(cmd, "upload"))
			writel(0x00,BCM215XXINFORM);
	}

	flush_cache_all();
	
	outer_flush_all();
	outer_disable();
	outer_inv_all();

	
	writel(SOFTRST_SOFTRST, IO_ADDRESS(BCM21553_SOFTRST_BASE));
	for (;;)
	   ;	
}

#endif
