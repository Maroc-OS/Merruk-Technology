/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/broadcom/dss/bcm215xx/lcdc.h
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

#ifndef __BCM_LCDC_H
#define __BCM_LCDC_H

#include <linux/string.h>
#include <linux/module.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/interrupt.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/kernel_stat.h>
#include <linux/broadcom/bcm_sysctl.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/suspend.h>
#include <linux/kthread.h>
#include <plat/dma.h>
#include <linux/platform_device.h>
#include <plat/bcm_lcdc.h>

#include <asm/byteorder.h>
#include <asm/irq.h>
#include <linux/gpio.h>
#include <linux/broadcom/regaccess.h>
#if defined(CONFIG_BCM_IDLE_PROFILER_SUPPORT)
#include <linux/broadcom/idle_prof.h>
#endif
#include <linux/broadcom/cpu_sleep.h>
#include <asm/mach/irq.h>
#include <asm/io.h>

#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/hw.h>
#include <linux/broadcom/lcd.h>
#include <linux/broadcom/PowerManager.h>
#include <cfg_global.h>
#include <plat/syscfg.h>
#include <linux/regulator/consumer.h>

#include "lcd.h"
#include "reg_lcdc.h"
#include <plat/types.h>
#include <plat/osdal_os_driver.h>
#include <plat/csl/csl_lcd.h>
#include <plat/csl/csl_lcdc.h>

#if defined(CONFIG_PANEL_MAGNA_D51E5TA7601)
#include "../../displays/panel-magna-d51e5ta7601.h"
#elif defined(CONFIG_BCM_LCD_NT35582)
#include "../../displays/lcd_NT35582.h"
#elif defined(CONFIG_BCM_LCD_R61581)
#include "../../displays/lcd_R61581.h"
#elif defined(CONFIG_BCM_LCD_S6D04H0A01)
#include "../../displays/lcd_tiama_s6d04h0_a01.h"
#elif defined(CONFIG_BCM_LCD_S6D04H0A01_TASSVE)
#include "../../displays/lcd_tiama_s6d04h0_a01_tassve.h"
#elif defined(CONFIG_BCM_LCD_S6D04K1)
#include "../../displays/lcd_s6d04k1.h"
#elif defined(CONFIG_BCM_LCD_S6D04K1_LUISA_HW02)
#include "../../displays/lcd_s6d04k1_luisa_hw02.h"
#elif defined(CONFIG_BCM_LCD_ILI9341_BOE)
#include "../../displays/lcd_ili9341_boe.h"
#elif defined(CONFIG_BCM_LCD_S6D05A1X31_COOPERVE)
#include "../../displays/lcd_s6d05a1x31_cooperve.h"
#else
#include "../../displays/lcd_ili9341_rev05.h"
#endif

#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
#include <mach/bcm21553_cpufreq_gov.h>
#endif

#ifdef CONFIG_BRCM_KPANIC_UI_IND
#include "ap_start_dump_img.h"
#include "cp_start_dump_img.h"
#include "dump_end_img.h"
#endif

#endif /* __BCM_LCDC_H */
