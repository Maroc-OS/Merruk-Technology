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
*   @file   phonectrl_api.h
*
*   @brief  This file defines the interface for Network Registration related API functions.
*
****************************************************************************/
/**
*
*   @defgroup   OldPhoneControlAPIGroup	Legacy Phone Control API
*   @ingroup    SystemGroup				
*
*   @brief      This group defines the legacy interfaces for network registration
*               and platform power up/down.  This group is no longer supported,
*				and is being supported for backwards compatibility.

	\n Use the link below to navigate back to the Phone Control Overview page. \n
    \li \if CAPI2
	\ref CAPI2PhoneCtrlOverview
	\endif
	\if CAPI
	\ref PhoneCtrlOverview
	\endif
****************************************************************************/

#ifndef _PHONECTRL_API_H_
#define _PHONECTRL_API_H_


Boolean 	PhoneCtrlApi_IsPowerDownInProgress(ClientInfo_t *clientInfoPtr);


void 		PhoneCtrlApi_ProcessReset(ClientInfo_t *clientInfoPtr);


/**
 * @addtogroup PhoneControlAPIGroup
 * @{
 */

//***************************************************************************************
/**
	This function de-registers the mobile from the network and
	powers down the system.

	@param		clientInfoPtr (in) Client information
**/
void				PhoneCtrlApi_ProcessPowerDownReq(ClientInfo_t *clientInfoPtr);


//***************************************************************************************
/**
	This function powers-up the platform with No RF activity.  In this state
	the system is functional but can not access the network.  It is typically
	used to allow access to run the mobile in a restricted environment such as
	an airplane.

	@param		clientInfoPtr (in) Client information
**/
void				PhoneCtrlApi_ProcessNoRfReq(ClientInfo_t *clientInfoPtr);



//***************************************************************************************
/**
	This function powers-up from No RF to calibration mode, or move from No RF to calibration mode

	@param		inClientInfoPtr (in) Client information
**/
void PhoneCtrlApi_ProcessNoRfToCalib(ClientInfo_t *inClientInfoPtr);



//***************************************************************************************
/**
	This function powers-up the platform.  The mobile will start to search for
	a network on which to camp and will broadcast events to registered clients.

	@param		clientInfoPtr (in) Client information
**/

void				PhoneCtrlApi_ProcessPowerUpReq(ClientInfo_t *clientInfoPtr);



/**
	Function to set the threshold parameters to control whether RSSI indication message MSG_RSSI_IND is 
	posted to clients. Once this function is called, the difference between the new and old RXLEV/RXQUAL 
	values (if current RAT is GSM) or RSCP/Ec/Io values (if current RAT is UMTS) must be larger or equal 
	to the threshold in order for the MSG_RSSI_IND message to be sent. 

    The caller can pass 0 to a threshold to disable the threshold checking. 

	@param clientInfoPtr		(in) Client information
	@param gsm_rxlev_thresold	(in) GSM RXLEV threshold. See section 8.1.4 of GSM 05.08 for RXLEV values. 
	@param gsm_rxqual_thresold	(in) GSM RXQUAL threshold. See Section 8.2.4 of GSM 05.08 for RXQUAL values. 
	@param umts_rscp_thresold	(in) UMTS RSCP threshold. See Section 9.1.1.3 of 3GPP 25.133 for RSCP values.
	@param umts_ecio_thresold	(in) UMTS Ec/Io threshold. See Section 9.1.2.3 of 3GPP 25.133 for Ec/Io values.

	@return		 None

**/
void	PhoneCtrlApi_SetRssiThreshold(ClientInfo_t *clientInfoPtr, UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold);


//**************************************************************************************
/**
	This function is used to check the system state value.

	@param      clientInfoPtr (in) Client information
	@return		SystemState_t

	Possible return values are ::SYSTEM_STATE_OFF, ::SYSTEM_STATE_ON, ::SYSTEM_STATE_ON_NO_RF, ::SYSTEM_STATE_OFF_IN_PROGRESS,
	::SYSTEM_STATE_OFF_CHARGING.
**/
SystemState_t PhoneCtrlApi_GetSystemState(ClientInfo_t *clientInfoPtr);

//**************************************************************************************
/**
	This function is used to set the system state value.

	@param      clientInfoPtr (in) Client information
	@param		state  Possible values are ::SYSTEM_STATE_OFF, ::SYSTEM_STATE_ON, ::SYSTEM_STATE_ON_NO_RF, ::SYSTEM_STATE_OFF_IN_PROGRESS,
	::SYSTEM_STATE_OFF_CHARGING.
**/

Result_t PhoneCtrlApi_SetSystemState(ClientInfo_t *clientInfoPtr, SystemState_t state);

//**************************************************************************************
/**
	This function returns the recieved signal level and signal quality 

	@param      clientInfoPtr (in) Client information
	@param		RxLev  signal level passed by reference to be filled in by the called function
	@param		RxQual rxquality passed by reference to be filled in by the called function
	
	@return		void
**/

void	PhoneCtrlApi_GetRxSignalInfo(ClientInfo_t *clientInfoPtr, UInt8 *RxLev, UInt8 *RxQual); 

/** @} */


UInt8 SYS_GetRIPPROCVersion( void );	

UInt8 PhoneCtrlApi_GetRIPPROCVersion( void );

Result_t PhoneCtrlApi_SetPowerDownTimer(ClientInfo_t *clientInfoPtr, UInt8 powerDownTimer);


//**************************************************************************************
/**
	This function is used to enable/disable paging status 

	@param		inClientInfoPtr (in) Client Information Pointer
	@param		status (in) value
	@return	 Result_t
**/
Result_t PhoneCtrlApi_SetPagingStatus(ClientInfo_t* inClientInfoPtr, UInt8 status);



#endif
