/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
* 
* 	@file	arch/arm/mach-bcm215xx/include/mach/usbctl.h
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

#if !defined( __USBCTL_H )
#define __USBTCL_H

/* ---- Include Files ---------------------------------------------------- */
#include <mach/hardware.h>

/* Bit definitions for Analog Configuration Register9 on baseband processor related to USB Device Mode */
#define ANACR9_USB_PPC_PWR_OFF_ANALOG_DRIVERS	0x00000001
#define ANACR9_USB_SELECT_DEVICE_MODE		0x00000002  //If this bit is 0 then it selects host mode
#define ANACR9_USB_SELECT_OTG_MODE		0x00000004  //If this bit is 0 then it selects standard USB 2.0 mode
#define ANACR9_USB_PLL_CAL_ENABLE	       	0x00000008
#define ANACR9_USB_PLL_POWER_ON		       	0x00000010 //If this bit is 0 then it turns PLL power down
#define ANACR9_USB_UTMI_SOFT_RESET_DISABLE      0x00000020 //If this bit is 0 then it enables UTMI soft reset
#define ANACR9_USB_UTMI_STOP_DIGITAL_CLOCKS     0x00000040 //If this bit is 0 then port comes out of reset in power down
#define ANACR9_USB_AFE_NON_DRVING			    0x00000080 //If this bit is 0 then USB host is connected.
#define ANACR9_USB_RESERVED_CLK_DURING_SUSP	0x40000000
#define ANACR9_USB_IDLE_ENABLE	              	0x00008000 //If 1 HSOTG PHY goes to sleep mode
#define ANACR9_PLL_SUSPEND_ENABLE		0x00000100 //0: USB suspend; 1: Normal mode

#define USB_CORE_WRAPPER_OTG_ON	  		0x00008000
#define USB_CORE_WRAPPER_PHY_CLK_DOMAIN_RESET  0x00000800
#define USB_CORE_WRAPPER_AHB_CLK_DOMAIN_RESET  0x00000400
#define USB_ON_IS_HCLK_EN      (1<<16)

#define MSK_CAPD_USB_PHY_CLK 0x00000004
#define USB_CORE_WRAPPER_AHB_CLK_USBON_SOURCE  0x00010000

#ifndef HW_IO_PHYS_TO_VIRT
#define HW_IO_PHYS_TO_VIRT(x)  ((x) - BCM21553_IO_PHYSICAL + BCM21553_IO_VIRTUAL)
#define HW_IO_VIRT_TO_PHYS(x)  ((x) - BCM21553_IO_VIRTUAL + BCM21553_IO_PHYSICAL)
#endif
#define HW_USBOTG_BASE     		HW_IO_PHYS_TO_VIRT( BCM21553_USB_HSOTG_BASE )   // USB On-the-go
#define HW_SYS_BASE	     		HW_IO_PHYS_TO_VIRT(BCM21553_SYSCFG_BASE)
#define HW_CLKPWR_BASE 		IO_ADDRESS(BCM21553_CLKPWR_BASE)

#define REG_USB_CTL                 						__REG32( HW_USBOTG_BASE + 0x80000 )
#define REG_SYS_ANACR9      							__REG32( HW_SYS_BASE + 0xa4 )
#define REG_SYSCFG_PERIPH_AHB_CLK_ENABLE			__REG32(HW_SYS_BASE + 0x108)

#define REG_CLKPWR_USBPLL_ENABLE        				__REG32(HW_CLKPWR_BASE + 0xA4)	/* 0 1bit   Power Up the 48MHz PLL */
#define REG_CLKPWR_USBPLL_OEN           				__REG32(HW_CLKPWR_BASE + 0xA8)	/* 0 1bit   Turn on the output gate of the 48MHz PLL */
#define REG_CLKPWR_CLK_ANALOG_PHASE_ENABLE		__REG32(HW_CLKPWR_BASE + 0x140) /* 01 bit Enable it */
#define REG_CLKPWR_CLK_USB_48_ENABLE				__REG32(HW_CLKPWR_BASE + 0x1d4)

#endif /* __USBTCL_H */