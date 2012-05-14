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
*   @file   phonebk_api_old.h
*
*   @brief  This file defines prototypes for the OLD Phonebook API functions.
*
*	Note:	The API functions in this file are obsoleted and will be removed in later release
*
****************************************************************************/

#ifndef _PHONEBK_API_OLD_H_
#define _PHONEBK_API_OLD_H_


/////////////////////////////////////////////////////////////////////////////////////////
//
//  The following are old Phonebook API functions which are obsolete, will be removed
//
void PBK_GetAlpha(char* number,	PBK_API_Name_t* alpha);
Boolean PBK_IsEmergencyCallNumber(char *phone_number);
Boolean PBK_IsPartialEmergencyCallNumber(char *phone_number);
Result_t PBK_SendInfoReq( UInt8 clientID, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendFindAlphaMatchMultipleReq( UInt8 clientID, PBK_Id_t pbk_id, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendFindAlphaMatchOneReq( UInt8 clientID, UInt8 numOfPbk, PBK_Id_t *pbkId, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, CallbackFunc_t* pbkAccessCb );
Boolean PBK_IsReady(void);
Result_t PBK_SendReadEntryReq( UInt8 clientID, PBK_Id_t pbk_id, 
					UInt16 start_index, UInt16 end_index, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendWriteEntryReq( UInt8 clientID, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
				char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendUpdateEntryReq( UInt8 clientID, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
	char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, 
	const USIM_PBK_EXT_DATA_t *usim_adn_ext_data, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendIsNumDiallableReq(UInt8 clientID, char *number, CallbackFunc_t* pbk_access_cb);
Boolean PBK_IsNumDiallable(const char *number, Boolean is_voice_call);
Boolean PBK_IsUssdDiallable(const UInt8 *data, UInt8 dcs, UInt8 len);
Result_t PBK_SendUsimAasInfoReq(UInt8 clientID, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimAasReadReq(UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimAasUpdateReq( UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, 
								   UInt8 alpha_size, const UInt8 *alpha, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendUsimGasInfoReq(UInt8 clientID, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimGasReadReq(UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimGasUpdateReq( UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, 
								   UInt8 alpha_size, const UInt8 *alpha, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendUsimHdkReadReq(UInt8 clientID, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimHdkUpdateReq( UInt8 clientID, const HDKString_t *hidden_key,
								    CallbackFunc_t* pbk_access_cb );
void PBK_SetFdnCheck(Boolean fdn_chk_on);
Boolean PBK_GetFdnCheck(void);
Boolean PBK_IsNumBarred(const char *number, Boolean is_voice_call);
// End of Old API definitions



#endif

