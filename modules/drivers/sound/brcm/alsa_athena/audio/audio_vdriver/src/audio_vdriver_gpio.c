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
*   @file   audio_vdriver_gpio.c
*
*   @brief   
*           
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
#include "audio_vdriver_gpio.h"
#include "dspcmd.h"
#include "log.h"
//#include "ripisr_audio.h"

#include <linux/gpio.h>

#if defined(AUDIO_GPIO_TASSVE)
#define GPIO13_MIC_SELECT  13

//GPIO13_MIC_SELECT=1  main MIC
//GPIO13_MIC_SELECT=0  loud MIC
#define Main_MIC   1
#define Loud_MIC   0

/**
*
* @addtogroup Audio
* @{
*/


//
// APIs 
//


// Start gpio 
void AUDDRV_GPIO_Start ()
{
        gpio_request(GPIO13_MIC_SELECT, "Loud_MIC enable");
        gpio_direction_output(GPIO13_MIC_SELECT, Loud_MIC);

        gpio_free(GPIO13_MIC_SELECT);

        Log_DebugPrintf(LOGID_AUDIO," AUDIO_GPIO_TASSVE start \r\n");
}


// Stop gpio
void AUDDRV_GPIO_Stop (void)
{
        gpio_request(GPIO13_MIC_SELECT, "Main_MIC enable");
        gpio_direction_output(GPIO13_MIC_SELECT, Main_MIC);

        gpio_free(GPIO13_MIC_SELECT);

        Log_DebugPrintf(LOGID_AUDIO," AUDIO_GPIO_TASSVE end \r\n");
}

#else // default model

// Start gpio 
void AUDDRV_GPIO_Start ()
{
        Log_DebugPrintf(LOGID_AUDIO," AUDDRV_GPIO_Start end \r\n");
}

// Stop gpio
void AUDDRV_GPIO_Stop (void)
{
        Log_DebugPrintf(LOGID_AUDIO,"AUDDRV_GPIO_Stop end \r\n");
}
#endif

