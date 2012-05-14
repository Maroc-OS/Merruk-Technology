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
*   @file   audio_vdriver_voif.c
*
*   @brief  PCM data interface to DSP. 
*           It is used to hook with customer's downlink voice processing module.
*
****************************************************************************/


#include "mobcom_types.h"
#include "resultcode.h"
#include "msconsts.h"
#include "shared.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "csl_aud_drv.h"
#include "csl_apcmd.h"
#include "audio_vdriver.h"
#include "audio_vdriver_voif.h"
#include "dspcmd.h"
#include "log.h"
#include "ripisr_audio.h"


/**
*
* @addtogroup Audio
* @{
*/


#define VOIF_8K_SAMPLE_COUNT	160
#define VOIF_16K_SAMPLE_COUNT	320


typedef	struct AUDDRV_VOIF_t
{
	UInt8					isRunning;
	VOIF_CB					cb;
} AUDDRV_VOIF_t;


static AUDDRV_VOIF_t	voifDrv = { 0 };
static Boolean voif_enabled = 0;

//
// APIs 
//

// Start voif 
void AUDDRV_VOIF_Start (VOIF_CB voifCB)
{
    if (voifDrv.isRunning)
			return;
    voifDrv.cb = (VOIF_CB)voifCB;
    register_voif_request_handler(VOIF_ISR_Handler);

    VPRIPCMDQ_VOIFControl( 1 );
    voif_enabled = TRUE;
    voifDrv.isRunning = TRUE;
    Log_DebugPrintf(LOGID_AUDIO," AUDDRV_VOIF_Start end \r\n");
}

// Stop voif
void AUDDRV_VOIF_Stop (void)
{
    if (voifDrv.isRunning == FALSE)
        return;
    VPRIPCMDQ_VOIFControl( 0 );
    register_voif_request_handler(NULL);
    voifDrv.cb = NULL;
    voif_enabled = FALSE;
    voifDrv.isRunning = FALSE;
	Log_DebugPrintf(LOGID_AUDIO,"AUDDRV_VOIF_Stop end \r\n");
}

// handle interrupt of data ready
void VOIF_ISR_Handler (UInt32 bufferIndex, UInt32 samplingRate)
{
    UInt32 dlIndex;
    Int16	*ulBuf, *dlBuf;
	UInt32 sampleCount = VOIF_8K_SAMPLE_COUNT;
    SharedMem_t* pSharedMem = SHAREDMEM_GetDsp_SharedMemPtr();
    

    if (!voif_enabled)
        return;
    ulBuf = (Int16 *)&pSharedMem->shared_voif_UL_buffer[0];
    dlIndex = bufferIndex & 0x1;

    if (samplingRate) 
    {
        sampleCount = VOIF_16K_SAMPLE_COUNT;
    }
    else
    {  
        sampleCount = VOIF_8K_SAMPLE_COUNT;
    }
    //Log_DebugPrintf(LOGID_AUDIO,"VOIF_TaskEntry received VOIF_DATA_READY. dlIndex = %d isCall16K = %d \r\n", dlIndex, samplingRate);
    
    dlBuf = (Int16 *)&pSharedMem->shared_voif_DL_buffer[dlIndex * sampleCount];
    if (voifDrv.cb)
        	voifDrv.cb (ulBuf, dlBuf, sampleCount, (UInt8)samplingRate);

}
 

