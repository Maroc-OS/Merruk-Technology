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
*   @file   plmnselect.h
*
*   @brief  This file contains all functions that are related to operator/PLMN selection.
*
****************************************************************************/

#ifndef _PLMNSELECT_H_
#define _PLMNSELECT_H_


typedef enum
{
	PLMN_SELECT_UNKNOWN			= 0,	
	PLMN_SELECT_AVAILABLE		= 1,	
	PLMN_SELECT_CURRENT			= 2,	
	PLMN_SELECT_FORBIDDEN		= 3,	
	PLMN_SELECT_STATUS_INVALID	= 4		
}PlmnSelectStatus_t;

//#define REGISTERED_NETWORK	0xFE
#define NO_MATCH			0xFF
#define SELECTED_PLMN		0xFD

#define MCC_FOR_AUTO_SELECT	0xFFFF
#define MNC_FOR_AUTO_SELECT	0xFF

// BRCM Internal Functions
Boolean IsFastBandNeededForAutoMode(void);
void		HandlePlmnListInd(InterTaskMsg_t* msg);


//*****************************************************************************
/**
* This function returns TRUE if both CS and PS services have invalid SIM/ME status
* for the network registration status.
*/
Boolean MS_IsInvalidSimMeStatus(ClientInfo_t *clientInfoPtr);


/**
* This function performs a PLMN detach-then-reattach procedure for STK Refresh.
* It should be called if the Refresh type is "Init With Full File Change 
* Notification" because the stack does not perform an IMSI detach-then-reattach
* procedure for the new IMSI. 
*
* Boolean loci_refreshed - TRUE if EF-LOCI is refreshed in SIM/USIM.
*
* @return Boolean TRUE if re-attach procedure is initiated; FALSE otherwise. 
*
*/
Boolean	MS_RefreshReAttach(Boolean loci_refreshed);
/**
 * @addtogroup PhoneControlAPIGroup
 * @{
 */




/** @} */

#endif

