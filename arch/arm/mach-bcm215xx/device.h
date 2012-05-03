/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/device.h
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
extern struct platform_device bcm21553_serial_device0;
extern struct platform_device bcm21553_serial_device1;
extern struct platform_device bcm21553_serial_device2;
extern struct plat_serial8250_port serial_platform_data0;
extern struct plat_serial8250_port serial_platform_data1;
extern struct plat_serial8250_port serial_platform_data2;
#endif

#if defined(CONFIG_BCM_WATCHDOG)
extern struct platform_device bcm_watchdog_device;
#endif

#if defined(CONFIG_I2C_BCM1160)
extern struct platform_device bcm21553_device_i2c1;
extern struct platform_device bcm21553_device_i2c2;
#if defined(CONFIG_ARCH_BCM21553_B0) || defined(CONFIG_ARCH_BCM21553_B1)
extern struct platform_device bcm21553_device_i2c3;
#endif
#endif
#ifdef CONFIG_MMC_BCM
extern struct platform_device bcm21553_sdhc_slot1;
#if !defined(CONFIG_MTD_ONENAND)
extern struct platform_device bcm21553_sdhc_slot2;
#endif
extern struct platform_device bcm21553_sdhc_slot3;
#endif

#if defined(CONFIG_BCM_AUXADC)
extern struct platform_device auxadc_device;
#endif

#if defined(CONFIG_KEYBOARD_BCM)
extern struct platform_device bcm215xx_kp_device;
#endif

#ifdef CONFIG_BCM_PWM
extern struct platform_device bcm_pwm_device;
#endif

#if defined(CONFIG_BRCM_HEADSET) || defined(CONFIG_BRCM_HEADSET_MODULE)
extern struct platform_device bcm_headset_device;
#endif
#ifdef CONFIG_SPI
extern struct platform_device bcm21xx_device_spi;
#endif
#if defined(CONFIG_BCM_OTP)
extern struct platform_device bcm_otp_device;
#endif

#ifdef CONFIG_BCM_I2SDAI
extern struct platform_device i2sdai_device;
#endif

#if defined(CONFIG_DMADEVICES) && defined(CONFIG_OSDAL_SUPPORT)
extern struct platform_device bcm21xx_dma_device;
#endif

#if defined(CONFIG_BCM215XX_DSS)
extern struct platform_device bcm215xx_lcdc_device;
#endif

#if defined(CONFIG_BCM_AVS)
extern struct platform_device bcm215xx_avs_device;

#if defined(CONFIG_BRCM_FUSE_SYSPARM)
extern void update_avs_sysparm(void);
#else
static void update_avs_sysparm(void)
{
}
#endif

#endif

#if defined(CONFIG_BCM_CPU_FREQ)
extern struct platform_device bcm21553_cpufreq_drv;
#endif

#if defined(CONFIG_CPU_FREQ_GOV_BCM21553)
extern struct platform_device  bcm21553_cpufreq_gov;
#endif

#if defined (CONFIG_BRCM_V3D)
extern struct platform_device v3d_device;
#endif

#endif /* __DEVICE_H_ */
