/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/mach-bcm116x/device.h
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

#ifndef __DEVICE_H_
#define __DEVICE_H_

#include <linux/platform_device.h>

#if defined(CONFIG_SERIAL_8250) || defined(CONFIG_SERIAL_8250_MODULE)
extern struct platform_device bcm_serial_device0;
extern struct platform_device bcm_serial_device1;
#endif
#if defined(CONFIG_MMC_BCM)
extern struct platform_device bcm_sdhc_slot1;
extern struct platform_device bcm_sdhc_slot2;
#endif
#if defined(CONFIG_BCM_I2SDAI)
extern struct platform_device i2sdai_device;
extern struct platform_device i2s_device;
#endif
#if defined(CONFIG_BCM_WATCHDOG)
extern struct platform_device bcm_watchdog_device;
#endif
#if defined(CONFIG_I2C_BCM1160)
extern struct platform_device bcm_device_i2c1;
extern struct platform_device bcm_device_i2c2;
#endif
#if defined(CONFIG_BCM_AUXADC)
extern struct platform_device auxadc_device;
#endif
#if defined(CONFIG_BRCM_HEADSET) || defined(CONFIG_BRCM_HEADSET_MODULE)
extern struct platform_device bcm_headset_device;
#endif
#if defined(CONFIG_BCM_OTP)
extern struct platform_device bcm_otp_device;
#endif

#if defined(CONFIG_SPI)
extern struct platform_device bcm21xx_device_spi;
#endif
#endif /* __DEVICE_H_ */
