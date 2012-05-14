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
*  @file  gpio_handler.h
*
*  @brief Template gpio handler API. Customer can add new API's and functions here.
*
*  @note
*****************************************************************************/
/**
*
* @defgroup Audio    Audio Component
*
* @brief    Updated by customer
*
* @ingroup  Audio Component
*****************************************************************************/


#ifndef	__AUDIO_GPIO_HANDLER_H__
#define	__AUDIO_GPIO_HANDLER_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

//*********************************************************************
/**
* Start the GPIO processing.
*
*	@param	channel: speaker type
*	@return	gpio
*	@note		
**************************************************************************/

void GPIO_init(AudioMode_t mode);

//*********************************************************************
/**
* Stop the GPIO processing.
*
*	@param	none
*	@return	void
*	@note		
**************************************************************************/
void GPIO_Deinit (void);



#ifdef __cplusplus
}
#endif



#endif // __AUDIO_GPIO_HANDLER_H__
