/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/broadcom/dss/bcm215xx/reg_lcdc.h
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

/*****************************************************************************
*
*  reg_lcdc.h
*
*  PURPOSE:
*
*     This file contains definitions for the LCD controller registers
*
*  NOTES:
*
*****************************************************************************/
#ifndef __MACH_REG_LCDC_H
#define __MACH_REG_LCDC_H

#define REG_LCDC_CMDR		0x0000
#define REG_LCDC_DATR		0x0004
#define REG_LCDC_RREQ		0x0008
#define REG_LCDC_WTR		0x0010
#define REG_LCDC_RTR		0x0014
#define REG_LCDC_CR		0x0018
#define REG_LCDC_STATUS		0x001C
#define REG_LCDC_DBIBTR		0x0020
#define REG_LCDC_DBICTR		0x0024
#define REG_LCDC_TEDELAY	0x0028

/* physical address of the LCD data register */
#define REG_LCDC_DATR_PADDR	(BCM21553_IO_PHYSICAL + BCM21553_LCDC_BASE + REG_LCDC_DATR)

/*
 * LCD_WTR - LCD Write Timing Register
 */
#define REG_LCDC_WTR_SETUP_SHIFT	0
#define REG_LCDC_WTR_SETUP_MASK		(0xFF << REG_LCDC_WTR_SETUP_SHIFT)

#define REG_LCDC_WTR_PULSE_SHIFT	8
#define REG_LCDC_WTR_PULSE_MASK		(0xFF << REG_LCDC_WTR_PULSE_SHIFT)

#define REG_LCDC_WTR_HOLD_SHIFT		16
#define REG_LCDC_WTR_HOLD_MASK		(0xFF << REG_LCDC_WTR_HOLD_SHIFT)

#define REG_LCDC_WTR_VAL(setup, pulse, hold)	\
	(((setup) << REG_LCDC_WTR_SETUP_SHIFT)	\
	| ((pulse) << REG_LCDC_WTR_PULSE_SHIFT)	\
	| ((hold)  << REG_LCDC_WTR_HOLD_SHIFT))

/*
 * LCD_RTR - LCD Read Timing Register
 */
#define REG_LCDC_RTR_SETUP_SHIFT	0
#define REG_LCDC_RTR_SETUP_MASK		(0xFF << REG_LCDC_RTR_SETUP_SHIFT)

#define REG_LCDC_RTR_PULSE_SHIFT	8
#define REG_LCDC_RTR_PULSE_MASK		(0xFF << REG_LCDC_RTR_PULSE_SHIFT)

#define REG_LCDC_RTR_HOLD_SHIFT		16
#define REG_LCDC_RTR_HOLD_MASK		(0xFF << REG_LCDC_RTR_HOLD_SHIFT)

#define REG_LCDC_RTR_VAL(setup, pulse, hold)	\
	(((setup) << REG_LCDC_RTR_SETUP_SHIFT)	\
	| ((pulse) << REG_LCDC_RTR_PULSE_SHIFT)	\
	| ((hold)  << REG_LCDC_RTR_HOLD_SHIFT))

/*
 * LCD_CR - LCD Control Register
 */
#define REG_LCDC_CR_ENABLE_DMA			(1u << 31)
#define REG_LCDC_CR_ENABLE_8_BIT_INTF		(1u << 30)
#define REG_LCDC_CR_ENABLE_16_TO_18_EXPANSION	(1u << 29)
#define REG_LCDC_CR_ENABLE_DMA_BYTESWAP		(1u << 28)
#define REG_LCDC_CR_ENABLE_DMA_WORDSWAP		(1u << 27)
#define REG_LCDC_CR_SEL_LCD			(1u << 26)
#define REG_LCDC_CR_SEL_HANTRO_MODE		(1u << 25)
/* bit 24 reserved */
#define REG_LCDC_CR_DBI_B			(1u << 23)
#define REG_LCDC_CR_COLOR_ENDIAN		(1u << 22)	/* 1==big */
#define REG_LCDC_CR_INPUT_COLOR_MODE		(3u << 20)
#define REG_LCDC_CR_COLOR_MODE			(7u << 17)
#define REG_LCDC_CR_THCLK_CNT			(3u << 15)
#define REG_LCDC_CR_DBI_C			(1u << 14)
#define REG_LCDC_CR_DBI_C_TYPE			(1u << 13)	/* 1==4wire */
#define REG_LCDC_CR_NEW_FRAME			(1u << 12)
#define REG_LCDC_CR_EDGE_SEL			(1u << 11)
#define REG_LCDC_CR_TEVALID			(1u << 10)
#define REG_LCDC_CR_DBIC_SELECT			(1u << 9)

/*
 * LCDC_STATUS - LCD Status Register
 */
#define REG_LCDC_STATUS_FIFO_EMPTY		(1u << 31)
#define REG_LCDC_STATUS_FIFO_FULL		(1u << 30)
#define REG_LCDC_STATUS_FIFO_HALF		(1u << 29)
#define REG_LCDC_STATUS_FIFO_READ_PTR		(0xffu << 21)
#define REG_LCDC_STATUS_FIFO_WRITE_PTR		(0x3fu << 15)
#define REG_LCDC_STATUS_READ_READY		(1u << 14)
#define REG_LCDC_STATUS_LCD_BUSY		(1u << 13)
#define REG_LCDC_STATUS_RREQ			(1u << 12)
#define REG_LCDC_TE				(1u << 11)

#endif /* __MACH_REG_LCDC_H */
