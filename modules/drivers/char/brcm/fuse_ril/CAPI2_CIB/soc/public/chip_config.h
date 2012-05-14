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

#ifndef _CHIP_CONFIG_H_
#define _CHIP_CONFIG_H_

//
// This is intended to replace memmap.h over time when all code are modular and 
// based on RDB and cHAL instead of accessing chip register directly
//

// !< --- PreProcess --- >!
//FOR CHIP_CONFIG.INC


//CHIP ID
#define ML2000B					0
#define ML2010					1
#define ML2010B					2
#define ML2020					3
#define ML2020B					4
#define	ML2029					5
#define ML2010C					6
#define ML2021					7	//Development chip having HS DROM.
#define	BCM2132					8
#define	BCM2133					9
#define BCM2124					10
#define BCM2152					11
#define BCM2153					12
#define BCM213x1				13
#define BCM21551				14
#define BCM213x1v				15
#define ATHENA				    0x30
#define HERA				    0x31 // Temporary define for build purpose
#define RHEA				    0x32 // Temporary define for build purpose
#define SAMOA				    0x33 // Temporary define for build purpose

#if defined(_BCM21551_)
#define		CHIPID4ASM			BCM21551 // compile switch in ARM assembly code
#elif defined(_ATHENA_)
#define		CHIPID4ASM			ATHENA // compile switch in ARM assembly code
#elif defined(_HERA_)
#define		CHIPID4ASM			HERA
#elif defined(_RHEA_)
#define		CHIPID4ASM			RHEA
#elif defined(_SAMOA_)
#define		CHIPID4ASM			SAMOA
#endif

//--- BASE INFO
#define ROM_BASE			0x00000000
#define SDRAM_BASE			0x80000000
#define RAM_BASE			SDRAM_BASE

//#define MMU_TBL_DOMAIN     2
#if (defined(_BCM21551_) || defined(_ATHENA_) || defined(_HERA_) || defined(_RHEA_) || defined(_SAMOA_)) && defined(FUSE_COMMS_PROCESSOR)
#define ITCM_PAGE1_DOMAIN  3
#define ITCM_PAGE2_DOMAIN  4
#define ITCM_PAGE3_DOMAIN  5
#endif

#if defined(_HERA_) || defined(_RHEA_)
#define PUMR_REG				0x35003000
#else
#define PUMR_REG				0x08880018
#endif
#define MARM11CR_REG			0x08880058

// Mode for PUMR_REG (Power Up Mode Register), for platform internal use only
#define SEL_OFF					0x00
#define SEL_SOFT_DOWNLOAD		0x01	//UART Download through Bootrom
#define SEL_CALIBRATION			0x02
#define SEL_SECURITY_DOWNLOAD	0x04
#define SEL_CALL_TEST			0x08
#define SEL_USB_DOWNLOAD		0x10
#define SEL_BOOTLOADER_UART_DOWNLOAD	0x20	//UART Download through Bootloader 

// !< --- PreProcess --- >!

#endif

