/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/bcm_fuse_memmap.h
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

#ifndef _BCM_FUSE_MEMMAP_H_
#define _BCM_FUSE_MEMMAP_H_

/* from nansdram_memmap.h */
#define CP_START_ADDR      (0x80300000)

#define CP_BOOT_BASE_PHYS (0xFFFF0000UL)
#define CP_BOOT_BASE_SIZE (0x40)

/* Shared memory resource shared between CP and CP
 * This memory information is required for Audio component
 */
#define DSP_SH_BASE	0x80000000 /* 320k DSP sharedmem */
#define DSP_SH_SIZE	0x00050000
#define CP_SH_BASE	DSP_SH_BASE
#define CP_SH_SIZE	0x00020000 /* 128k CP/DSP sharedmem */
#define AP_SH_BASE	(DSP_SH_BASE+CP_SH_SIZE)
#define AP_SH_SIZE	0x00010000 /* 64k  AP/DSP sharedmem */

#endif /* _BCM_FUSE_MEMMAP_H_ */
