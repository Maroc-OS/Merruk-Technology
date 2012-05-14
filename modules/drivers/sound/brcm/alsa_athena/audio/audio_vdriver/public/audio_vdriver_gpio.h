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
*  @file  audio_vdriver_gpio.h
*
*  @brief Audio API for gpio interface. Used for MIC as an example.
*
*  @note
*****************************************************************************/
/**
*
* @defgroup Audio    Audio Component
*
* @brief    This group defines the common APIs for gpio driver
*
* @ingroup  Audio Component
*****************************************************************************/


#ifndef	__AUDIO_VDRIVER_GPIO_H__
#define	__AUDIO_VDRIVER_GPIO_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif


#define AUDIO_DRIVER_GPIO_ENABLED // flag to turn on/off this object

#ifdef AUDIO_DRIVER_GPIO_ENABLED // real definitions


//*********************************************************************
/**
* Start the GPIO processing.
*
*	@param none	
*	@return	void
*	@note		
**************************************************************************/
void AUDDRV_GPIO_Start (void);


//*********************************************************************
/**
* Stop the GPIO processing.
*
*	@param	none
*	@return	void
*	@note		
**************************************************************************/
void AUDDRV_GPIO_Stop (void);



#endif // AUDIO_DRIVER_GPIO_ENABLED

#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VDRIVER_GPIO_H__
