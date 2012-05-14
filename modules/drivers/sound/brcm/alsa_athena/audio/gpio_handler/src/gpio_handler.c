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
*   @file   gpio_handler.c
*
*   @brief   
*     
*           Customer will implement this.
*
****************************************************************************/


#include "mobcom_types.h"
#include "audio_consts.h"
#include "gpio_handler.h"
#include "audio_vdriver_gpio.h"

//
// APIs 
//



// Start gpio 
void GPIO_init(AudioMode_t mode)
{
    if(mode == AUDIO_MODE_SPEAKERPHONE)	
        AUDDRV_GPIO_Start();
    else
        AUDDRV_GPIO_Stop();
    return;
}


//stop gpio
void GPIO_Deinit()
{
    AUDDRV_GPIO_Stop();
    return;
}
 

