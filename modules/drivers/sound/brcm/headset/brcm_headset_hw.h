/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
* 
* 	@file	drivers/sound/brcm/headset/brcm_headset_hw.h
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

/* Header file for HEASET DRIVER */

#ifndef __BCM_HEADSET_HW__
#define __BCM_HEADSET_HW__

#define BCM_HEADSET_IOCTL_STATUS        0
#define BCM_HEADSET_IOCTL_BUTTON_STATUS 1

#define __CHIP_VERSION_H__

#define CHIP_VERSION(x,y) ((x)*100 + (y))

typedef unsigned long UInt32;

struct auxmic{ 
	volatile UInt32  AUXMIC_PRB_CYC;
	volatile UInt32  AUXMIC_MSR_DLY;
	volatile UInt32  AUXMIC_MSR_INTVL;
	volatile UInt32  AUXMIC_CMC;
	volatile UInt32  AUXMIC_MIC;
	volatile UInt32  AUXMIC_AUXEN;
	volatile UInt32  AUXMIC_MICINTH_ADJ;
	volatile UInt32  AUXMIC_MICINENTH_ADJ;
	volatile UInt32  AUXMIC_MICONTH_ADJ;
	volatile UInt32  AUXMIC_MICONENTH_ADJ;
	volatile UInt32  AUXMIC_F_PWRDWN;
	volatile UInt32  AUXMICSRVD;
	volatile UInt32  AUXMIC_MICINTH_DEF;
	volatile UInt32  AUXMIC_MICINENTH_DEF;
	volatile UInt32  AUXMIC_MICONTH_DEF;
	volatile UInt32  AUXMIC_MICONENTH_DEF;
	volatile UInt32  AUXMIC_MICINTH;
	volatile UInt32  AUXMIC_MICONTH;

}; 

extern int enable_headset_detection(struct auxmic *);

typedef unsigned char UInt8;
#define IMSI_DIGITS	15	///< Max. IMSI digits
typedef UInt8 IMSI_t[IMSI_DIGITS + 1];
#define GID_DIGITS	10	///< Max. GID1/GID2 file length
typedef UInt8 GID_DIGIT_t[GID_DIGITS];
/// SIM data needed to check SIMLOCK status
typedef struct
{
	IMSI_t imsi_string;	///< NULL terminated IMSI string
	GID_DIGIT_t	gid1;	///< GID1 data
	UInt8 gid1_len;		///< Number of bytes in "gid1" element, i.e. number of bytes in EF-GID1
	GID_DIGIT_t gid2;	///< GID2 data
	UInt8 gid2_len;		///< Number of bytes in "gid2" element, i.e. number of bytes in EF-GID2
} SIMLOCK_SIM_DATA_t;

#endif //  __BCM_HEADSET_HW__

