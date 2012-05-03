/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/sysmap.c
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
#include <linux/io.h>
#include <linux/device.h>
#include <mach/hardware.h>
#include <plat/csl/sysmap_types.h>
#include <mach/chal_sysmap_addr.h>

/* Needed to convert the base_address to virtual address if mapped */
unsigned int csl_sysmap_get_base_addr(SYSMAP_BASE_t block)
{
	if (block < SYSMAP_BASE_MAX)
	return (int) io_p2v(chal_sysmap_base_address[block]);

	return 0;
}
EXPORT_SYMBOL(csl_sysmap_get_base_addr);

static int __init sysmap_init(void)
{
	pr_debug("sysmap alive\n");
	return 0;
}

static void __exit sysmap_exit(void)
{
	pr_debug("exit sysmap driver\n");
}

arch_initcall(sysmap_init);
module_exit(sysmap_exit);

MODULE_AUTHOR("BROADCOM");
MODULE_DESCRIPTION("BROADCOM SYSMAP");
MODULE_LICENSE("GPL");
