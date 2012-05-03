/*******************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
 *
 *       @file   include/linux/broadcom/bcm_reset_utils.h
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
#ifndef _BRCM_RESET_UTILS_H_
#define _BRCM_RESET_UTILS_H_

typedef enum syscfg_reset_reason {
	POWER_ON_RESET	  = 0x000,
	AP_ONLY_BOOT 	  = 0x040,
	SOFT_RESET	  = 0x100,
	POWEROFF_CHARGING = 0x200,
} syscfg_reset_reason_t;

syscfg_reset_reason_t get_ap_boot_mode(void);

#endif /* _BRCM_RESET_UTILS_H_ */
