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
*   @file   phonebk_api_atc.h
*
*   @brief  This file defines prototypes for the Phonebook API functions. (used by ATC only)
*
****************************************************************************/
/**

*   @brief      This group defines the types and prototypes for the Phonebook API functions.
*	
****************************************************************************/

#ifndef _PHONEBK_API_ATC_H_
#define _PHONEBK_API_ATC_H_



//**************************************************************************************
/**	This function sends the request to update the USIM PBK Hidden key information.
	
	The hidden key shall be 4-8 digits according to Section 4.2.42 of 4GPP 31.102. 
	It is applicable for USIM only and it needs to be verified for an ADN entry
	whose status is marked as "hidden" before a user can access the entry. 
	See "is_hidden" element in "USIM_PBK_EXT_DATA_t" structure returned for an ADN entry in 3G USIM. 

	The client shall verify that the hidden key is provisioned in USIM before calling
	this function. The hidden key provision status is provided in "hidden_key_exist" 
	element in USIM_PBK_INFO_t structure in the response for PbkApi_SendInfoReq() function. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*hidden_key Hidden key information.
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		RESULT_OK 
	@note		A MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP message will be posted back
**/
Result_t PbkApi_SendUsimHdkUpdateReq( ClientInfo_t* inClientInfoPtr, const HDKString_t *hidden_key,
								    CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
	This function enables/disables the FDN check in phonebook. 

    By default FDN check in phonebook is on; It can be disabled by client for valid reasons such as:
	
	1. Client handles the STK Setup Call; STK Send SS/USSD; STK Send SMS and thus we must
	   disable FDN check when a call is set up or when SMS is sent because the request originates
	   from SIM and according to GSM 11.14 those requests must not be checked against FDN phonebook. 

    2. MMI Client already performs FDN check, there is no need to do it again. 
	
	When FDN check in phonebook is disabled, PbkApi_SendIsNumDiallableReq, PbkApi_IsNumDiallable() & 
	PbkApi_IsUssdDiallable() always return TRUE to indicate FDN check is passed. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		fdn_chk_on (in) TRUE to enable FDN check in phonebook; FALSE to disable it. 

	@return		None
	@note
**/
void PbkApi_SetFdnCheck(ClientInfo_t* inClientInfoPtr, Boolean fdn_chk_on);

/**
	This function returns the FDN check enabled/disabled status in phonebook. 

    By default FDN check in phonebook is on; It can be disabled by client for valid reasons such as:
	
	1. Client handles the STK Setup Call; STK Send SS/USSD; STK Send SMS and thus we must
	   disable FDN check when a call is set up or when SMS is sent because the request originates
	   from SIM and according to GSM 11.14 those requests must not be checked against FDN phonebook. 

    2. MMI Client already performs FDN check, there is no need to do it again. 
	
	When FDN check in phonebook is disabled, PbkApi_SendIsNumDiallableReq, PbkApi_IsNumDiallable() & 
	PbkApi_IsUssdDiallable() always return TRUE to indicate FDN check is passed. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@return		TRUE if FDN check is enabled; FALSE otherwise. 
	@note
**/
Boolean PbkApi_GetFdnCheck(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	This function requests to check whether the passed non-USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	For USSD number, use PbkApi_IsUssdDiallable() API instead. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*number (in) Pointer to NULL terminated number to be checked. 
	@param		*pbk_access_cb (in) Phonebook access callback

	@return		Result_t 
	@note
	The phonebook task returns the result in the MSG_CHK_PBK_DIALLED_NUM_RSP message
	by calling the passed callback function.
**/
Result_t PbkApi_SendIsNumDiallableReq(ClientInfo_t* inClientInfoPtr, char *number, CallbackFunc_t* pbk_access_cb);


//**************************************************************************************
/**
	This function checks whether the passed non-USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	For USSD number checking, use 
	PbkApi_IsUssdDiallable() API instead. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		number (in) Buffer that stores the NULL terminated number
	@param		is_voice_call (in) TRUE if the number is for voice call

	@return		TRUE if number is diallable; FALSE otherwise. 
	@note
	The result is returned synchronously. 
**/
Boolean PbkApi_IsNumDiallable(ClientInfo_t* inClientInfoPtr, const char *number, Boolean is_voice_call);


//**************************************************************************************
/**
	This function checks whether the passed USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	
	
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		data (in) Buffer that stores the USSD number
	@param		dcs (in) USSD DCS byte in CB DCS format
	@param		len (in) Number of bytes passed in "data". 

	@return		TRUE if USSD is diallable; FALSE otherwise. 
	@note
**/
Boolean PbkApi_IsUssdDiallable(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 dcs, UInt8 len);

//**************************************************************************************
/**
	This function checks whether the passed non-USSD number is barred according to the
	BDN setting and the BDN phonebook contents.

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		number (in) Buffer that stores the NULL terminated number
	@param		is_voice_call (in) TRUE if the number is for voice call

	@return		TRUE if number is barred; FALSE otherwise. 
	@note
	The result is returned synchronously. 
**/
Boolean PbkApi_IsNumBarred(ClientInfo_t* inClientInfoPtr, const char *number, Boolean is_voice_call);

#endif
