/*********************************************************************
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
***************************************************************************/
/**
*
*   @file   memconfig.h
*
*   @brief  This file contains memory configuration settings.
*
* 	@note 	This file should be customized for different hardware configurations.
*
****************************************************************************/

#ifndef _MEMCONFIG_H_
#define _MEMCONFIG_H_

#if CHIPVERSION < CHIP_VERSION(BCM2133,00) || defined(DSP2133_TEST)

//-------------------------- Default clock setting --------------------------------//
#define ACR_CLK_MASK		(SRM1WTH_MASK | EQCLK_MASK | MSPCLK_MASK | RIPCLK_MASK | MPCLK_MASK)
#define ACR_CLK_DEFAULT		(SRM1WTH_8    | EQCLK_78   | MSPCLK_78   | RIPCLK_78   | MPCLK_78)		// SRM1 8-bit data width needed by LCD

//------------------------ Flash and SRAM setting -----------------------------//
// Default memory setting
#define MEMCYCLE_MASK	(SRM0CLK_MASK | FSH0CLK_MASK | FSH1CLK_MASK | FSH2CLK_MASK)
#define MEMCYCLE_39		0x00000000											// MSP 39MHz (not used)
#define MEMCYCLE_52		0x00000000											// MSP 52MHz (not used)
#define MEMCYCLE_78		(SRM0CLK_7 | FSH0CLK_8 | 0 | 0)						// MSP 78MHz
#define MEMCYCLE_104	(SRM0CLK_4 | FSH0CLK_4 | 0 | 0)						// MSP 104MHz

// Flash page mode setting
#define FPCR0CYCLE_39		0x0000		// MSP 39MHz: disable page mode (not used)
#define FPCR1CYCLE_39		0x0000		// disable page mode
#define FPCR2CYCLE_39		0x0000		// disable page mode
#define FPCR0CYCLE_52		0x0000		// MSP 52MHz: disable page mode (not used)
#define FPCR1CYCLE_52		0x0000		// disable page mode
#define FPCR2CYCLE_52		0x0000		// disable page mode
#define FPCR0CYCLE_78		0x0000		// MSP 78MHz: disable page mode (not used)
#define FPCR1CYCLE_78		0x0000		// disable page mode
#define FPCR2CYCLE_78		0x0000		// disable page mode
#define FPCR0CYCLE_104		0x0000		// MSP 104MHz: disable page mode (not used)
#define FPCR1CYCLE_104		0x0000		// disable page mode
#define FPCR2CYCLE_104		0x0000		// disable page mode

#if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
// SRAM page mode setting
#define SPCR0CYCLE_39		0x0000		// MSP 39MHz: disable page mode
#define SPCR1CYCLE_39		0x0000		// disable page mode
#define SPCR0CYCLE_78		0x0000		// MSP 78MHz: disable page mode
#define SPCR1CYCLE_78		0x0000		// disable page mode
#define SPCR0CYCLE_104		0x0000		// MSP 104MHz: disable page mode
#define SPCR1CYCLE_104		0x0000		// disable page mode
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */

#else // #if CHIPVERSION < CHIP_VERSION(BCM2133,00)

#endif // #if CHIPVERSION < CHIP_VERSION(BCM2133,00)

#define FBA_BASE_8M			0x10		// Set flash memory base to 8Mbyte
#define FBA_BASE_16M		0x20		// Set flash memory base to 16Mbyte
#define FBA_BASE_24M		0x30		// Set flash memory base to 24Mbyte
#define FBA1_BASE			0x00		// set to 0x00 if flash memory bank 1 is not used
#define FBA2_BASE			0x00		// set to 0x00 if flash memory bank 2 is not used

#if !defined(CYGWIN_BUILD) && !defined(ROM_SIZE)

// ROM sizes hard-wired here for MKS builds - Cygwin defines it in platform.*
#if CHIPVERSION < CHIP_VERSION(BCM2132,00)
	#define ROM_SIZE        0x01000000      // 16 Mbytes
#else
	#define ROM_SIZE        0x04000000      // 128 Mbits (16 Mbytes) x 4
#endif // CHIPVERSION < CHIP_VERSION(BCM2132,00)


#endif

#if defined(INCL_BLUETOOTH) || defined(VPU_INCLUDED)
#define SRAM_SIZE			0x00400000			// 4M Bytes
#else
#if defined(_BCM2124_)
#define SRAM_SIZE			0x00300000			// 3M Bytes
#else // #if defined(_BCM2124_)
#define SRAM_SIZE			0x00200000			// 2M Bytes
#endif // #if defined(_BCM2124_)
#endif

#define SDRAM_SIZE			0x00400000			// 4M Bytes


#if !defined(CYGWIN_BUILD) && !defined(RAM_SIZE)

#ifdef	RAM_TYPE_SDRAM
#define RAM_SIZE			SDRAM_SIZE
#else
#define RAM_SIZE			SRAM_SIZE
#endif // NAND_SDRAM

#endif // CYGWIN_BUILD

#endif // _MEMCONFIG_H_

