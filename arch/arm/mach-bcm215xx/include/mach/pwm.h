/*******************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
 *
 *       @file   arch/arm/mach-bcm215xx/include/mach/pwm.h
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 ******************************************************************************/

#ifndef _MACH_PWM_H_
#define _MACH_PWM_H_

/*
 * Maximum number of PWM channels supported by this platform
 */
#define MAX_PWM_DEVICE_ID        20

/*
 * PWM channel IDs
 */
enum bcm_pwm_ids {
	BCM_PWM_CHANNEL0,
	BCM_PWM_CHANNEL1,
	BCM_PWM_CHANNEL2,
	BCM_PWM_CHANNEL3,
	BCM_PWM_CHANNEL4,
	BCM_PWM_CHANNEL5,
	BCM59035_PWM_CHANNEL0,
	BCM59035_PWM_CHANNEL1,
};

#endif /* _MACH_PWM_H_ */
