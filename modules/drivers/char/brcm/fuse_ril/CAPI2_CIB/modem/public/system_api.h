/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   system_api.h
*
*   @brief  This file contains definitions for Phone Control / System API
*
****************************************************************************/
/**
*
*   @defgroup   PhoneControlAPIGroup	Phone Control API
*   @ingroup    SystemGroup				System API
*
*   @brief      This group defines the interfaces for network registration
*               and platform power up/down.	

	\n Use the link below to navigate back to the Phone Control Overview page. \n
    \li \if CAPI2
	\ref CAPI2PhoneCtrlOverview
	\endif
	\if CAPI
	\ref PhoneCtrlOverview
	\endif
****************************************************************************/


#ifndef __SYSTEM_API_H__
#define __SYSTEM_API_H__



/**
 * @addtogroup OldPhoneControlAPIGroup
 * @{
 *
**/  

//******************************************************************************
/**
	Set Power Down Timer for data storage purpose. If the timer
	is already running as a result of calling SYS_ProcessPowerDownReq(),  
	the original timer will be destroyed and a new timer with the 
	new timeout value will be started.  

	@param powerDownTimer powerdown timer value
**/
Result_t	MS_SetPowerDownTimer(UInt8 powerDownTimer);  

//***************************************************************************************
/**
	This API allows the client to set RUA ready timer for negotiation.

	@param		inRuaReadyTimer (in) RUA ready timer in seconds. 

	@return		None
**/
void	MS_SetRuaReadyTimer(Ticks_t inRuaReadyTimer);  

/** @} */

void CAPI_InitRpc(void);

#endif	//	__SYSTEM_API_H__

