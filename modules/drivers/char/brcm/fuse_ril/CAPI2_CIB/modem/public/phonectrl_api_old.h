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
*   @file   phonectrl_api_old.h
*
*   @brief  This file defines the interface for OLD phone ctrl related API functions.
*
*	Note:	The API functions in this file are obsoleted and will be removed in later release
*
****************************************************************************/

#ifndef _PHONECTRL_API_OLD_H_
#define _PHONECTRL_API_OLD_H_
Boolean IsPowerDownInProgress(void);
void 		SYS_ProcessReset(void);
/**
 * @addtogroup OldPhoneControlAPIGroup
 * @{
 */
/**
	This function de-registers the mobile from the network and
	powers down the system.
**/
void				SYS_ProcessPowerDownReq(void);


/**
	This function powers-up the platform with No RF activity.  In this state
	the system is functional but can not access the network.  It is typically
	used to allow access to run the mobile in a restricted environment such as
	an airplane.
**/
void				SYS_ProcessNoRfReq(void);


/**
	This function powers-up the platform.  The mobile will start to search for
	a network on which to camp and will broadcast events to registered clients.

**/
void				SYS_ProcessPowerUpReq(void);


/**
	Function to set the threshold parameters to control whether RSSI indication message MSG_RSSI_IND is 
	posted to clients. Once this function is called, the difference between the new and old RXLEV/RXQUAL 
	values (if current RAT is GSM) or RSCP/Ec/Io values (if current RAT is UMTS) must be larger or equal 
	to the threshold in order for the MSG_RSSI_IND message to be sent. 

    The caller can pass 0 to a threshold to disable the threshold checking. 

	@param gsm_rxlev_thresold (in) GSM RXLEV threshold. See section 8.1.4 of GSM 05.08 for RXLEV values. 
	@param gsm_rxqual_thresold (in) GSM RXQUAL threshold. See Section 8.2.4 of GSM 05.08 for RXQUAL values. 
	@param umts_rscp_thresold (in) UMTS RSCP threshold. See Section 9.1.1.3 of 3GPP 25.133 for RSCP values.
	@param umts_ecio_thresold (in) UMTS Ec/Io threshold. See Section 9.1.2.3 of 3GPP 25.133 for Ec/Io values.

	@return		 None

**/
void SYS_SetRssiThreshold(UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold);

/**
	This function is used to check the system state value.

	@return		SystemState_t

	Possible return values are ::SYSTEM_STATE_OFF, ::SYSTEM_STATE_ON, ::SYSTEM_STATE_ON_NO_RF, ::SYSTEM_STATE_OFF_IN_PROGRESS,
	::SYSTEM_STATE_OFF_CHARGING.
**/
SystemState_t SYS_GetSystemState(void);

/**
	This function is used to set the system state value.

	@param		state  Possible values are ::SYSTEM_STATE_OFF, ::SYSTEM_STATE_ON, ::SYSTEM_STATE_ON_NO_RF, ::SYSTEM_STATE_OFF_IN_PROGRESS,
	::SYSTEM_STATE_OFF_CHARGING.
**/
Result_t	SYS_SetSystemState(SystemState_t state);
/**
	This function returns the recieved signal level and signal quality 

	@param		RxLev  signal level passed by reference to be filled in by the called function
	@param		RxQual rxquality passed by reference to be filled in by the called function
	
	@return		void
**/
void	SYS_GetRxSignalInfo(UInt8 *RxLev, UInt8 *RxQual); 

/** @} */

#endif
