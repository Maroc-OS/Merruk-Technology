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
#ifndef _MACH_SYSMAP_H_
#define _MACH_SYSMAP_H_

/* Needed to convert the base_address to virtual address if mapped */
unsigned int csl_sysmap_get_base_addr(unsigned int);

#endif /*_MACH_SYSMAP_H_*/
