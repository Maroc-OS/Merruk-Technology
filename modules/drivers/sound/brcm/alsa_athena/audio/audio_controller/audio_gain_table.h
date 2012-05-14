/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/
/**
*
* @file   audio_gain_table.h
* @brief  Audio gain format translation table
*
******************************************************************************/


//=============================================================================
// Include directives
//=============================================================================

#ifndef __AUDIO_GAIN_TABLE_H__
#define __AUDIO_GAIN_TABLE_H__

#include "mobcom_types.h"
typedef struct
{
    AudioMode_t audioMode;
    UInt32  gainTotal;
    UInt32  gainPMU;
    UInt32  gainHW;
} AUDTABL_GainMapping_t;

#define NO_PMU_NEEDED   0x7FFFFFF
#define TOTAL_GAIN  0x7FFFFFF


/********************************************************************
*  @brief  read Slopgain value from Q14.1 gain 
*
*  @param  valueQ14_1  (in) gain in Q14.1 format 
*
*  @return HW slopgain register
*
****************************************************************************/
UInt16 AUDTABL_getQ14_1HWSlopGain(UInt16 valueQ14_1);


/********************************************************************
*  @brief  read Slopgain value from Q1.14 gain
*
*  @param  valueQ1_14	(in)  gain in Q1.14 format 
*
*  @return HW slopgain register
*
****************************************************************************/
UInt16 AUDTABL_getQ1_14HWSlopGain(UInt16 valueQ1_14);

/********************************************************************
*  @brief  return the gain distribution values which shows the PMU gain and
*  Audio HW gain
*
*  @param  gainQ31	(in)  gain in Q31.0 format 
*
*  @return Gain distribution values
*
****************************************************************************/
AUDTABL_GainMapping_t AUDTABL_getGainDistribution(AudioMode_t audioMode, UInt32 gainQ31);

#endif	//__AUDIO_GAIN_TABLE_H__

