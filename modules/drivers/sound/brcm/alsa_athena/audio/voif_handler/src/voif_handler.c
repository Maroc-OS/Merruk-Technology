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
*   @file   voif_handler.c
*
*   @brief  PCM data interface to DSP. 
*           It is used to hook with customer's downlink voice processing module. 
*           Customer will implement this.
*
****************************************************************************/


#include "mobcom_types.h"
#include "audio_consts.h"
#include "voif_handler.h"
#include "audio_vdriver_voif.h"

//
// APIs 
//

static void VOIF_CB_Fxn (Int16 * ulData, Int16 *dlData, UInt32 sampleCount, UInt8 isCall16K)
{
#if 0
    int count = 0;
    // Add the processing block here
    for(count = 0; count < sampleCount; count++)
    {
        ulData[count] = 0;
        dlData[count] = 0;
    }
#endif
    return;
}

// Start voif 
void VoIF_init(AudioMode_t mode)
{
    AUDDRV_VOIF_Start (VOIF_CB_Fxn);
    return;
}

void VoIF_Deinit()
{
    AUDDRV_VOIF_Stop ();
    return;
}
 

