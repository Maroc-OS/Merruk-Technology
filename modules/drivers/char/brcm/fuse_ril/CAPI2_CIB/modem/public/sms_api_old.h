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
*   @file   sms_api_old.h
*
*   @brief  This file defines the interface for old SMS API.
*
*	Note:	The API functions in this file are obsoleted and will be removed in later release
*
****************************************************************************/





#ifndef _SMS_API_OLD_H_
#define _SMS_API_OLD_H_
/////////////////////////////////////////////////////////////////////////////////////////
//
//  The following are old SMS API functions is obsoleted
//  will be move "sms_api_old.h"
//
void SMS_SendPDUAckToNetwork(UInt8 clientID, SmsMti_t mti, UInt16 rp_cause, UInt8* tpdu, UInt8 tpdu_len);
Boolean SMS_GetCBIgnoreDuplFlag(void);
Boolean SMS_GetSmsReadStatusChangeMode(void);
Boolean SMS_IsCBFilterSusp(void);
Boolean SMS_IsCachedDataReady(void);
Boolean SMS_IsVMIndEnabled(void);
Result_t SMS_AddCellBroadcastChnlReq(UInt8 clientID, UInt8* newCBMI, UInt8 cbmiLen);
Result_t SMS_AddCellBroadcastLangReq(UInt8 clientID, UInt8 newCBLang);
Result_t SMS_CBAllowAllChnlReq(UInt8 clientID, Boolean suspend_filtering);
Result_t SMS_ChangeSmsStatusReq(UInt8 clientId, SmsStorage_t storageType, UInt16 index);
Result_t SMS_DeleteSmsMsgByIndexReq(UInt8 clientID, SmsStorage_t storeType, UInt16 rec_no);
Result_t SMS_GetAllNewMsgDisplayPref(UInt8* mode, UInt8* mt, UInt8* bm, UInt8* ds, UInt8* bfr);
Result_t SMS_GetCBMI(SMS_CB_MSG_IDS_t* mids);
Result_t SMS_GetCbLanguage(MS_T_MN_CB_LANGUAGES* langs);
Result_t SMS_GetNumOfVmscNumber(UInt8* num);
Result_t SMS_GetSIMSMSCapacityExceededFlag(Boolean* flag);
Result_t SMS_GetSMSStorageStatus(SmsStorage_t storageType, UInt16 *NbFree, UInt16 *NbUsed);
Result_t SMS_GetSMSrvCenterNumber(SmsAddress_t *sca, UInt8 rec_no);
Result_t SMS_GetSmsStoredState (SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t* status);
Result_t SMS_GetSmsTxParams(SmsTxParam_t* params);
Result_t SMS_GetTxParamInTextMode( SmsTxTextModeParms_t* smsParms );
Result_t SMS_GetVMWaitingStatus(SmsVoicemailInd_t* vmInd); 
Result_t SMS_GetVmscNumber(SIM_MWI_TYPE_t vmsc_type, SmsAddress_t* vmsc);
Result_t SMS_IsSmsServiceAvail(void);
Result_t SMS_ListSmsMsgReq(UInt8 clientID, SmsStorage_t storeType, SIMSMSMesgStatus_t msgBox);
Result_t SMS_ReadSmsMsgReq(UInt8 clientID, SmsStorage_t storeType, UInt16 rec_no);
Result_t SMS_RemoveAllCBChnlFromSearchList(void);
Result_t SMS_RemoveCellBroadcastChnlReq(UInt8 clientID, UInt8* newCBMI);
Result_t SMS_RemoveCellBroadcastLangReq(UInt8 clientID, UInt8* cbLang);
Result_t SMS_RestoreSmsServiceProfile(UInt8 profileIndex);
Result_t SMS_RetrieveMaxCBChnlLength(UInt8* length);
Result_t SMS_SaveSmsServiceProfile(UInt8 profileIndex);
Result_t SMS_SendMERemovedStatusInd(MEAccess_t result, UInt16 slotNumber);
Result_t SMS_SendMERetrieveSmsDataInd(MEAccess_t result, UInt16 slotNumber, UInt8* inSms, UInt16 inLen, SIMSMSMesgStatus_t status);
Result_t SMS_SendMEStoredStatusInd(MEAccess_t result, UInt16 slotNumber);
Result_t SMS_SendSMSCommandPduReq(UInt8 clientID, UInt8 length, UInt8* inSmsCmdPdu);
Result_t SMS_SendSMSCommandTxtReq(UInt8 clientID, SmsCommand_t smsCmd, UInt8* inNum, UInt8* inCmdTxt, Sms_411Addr_t* sca);
Result_t SMS_SendSMSPduReq(UInt8 clientID, UInt8 length, UInt8* inSmsPdu, Sms_411Addr_t* sca);
Result_t SMS_SendSMSReq(UInt8 clientID, UInt8* inNum, UInt8* inSMS, SmsTxParam_t* params, UInt8* inSca);
Result_t SMS_SendSMSSrvCenterNumberUpdateReq(UInt8 clientID, SmsAddress_t* sca, UInt8 rec_no);
Result_t SMS_SendStoredSmsReq(UInt8 clientID, SmsStorage_t storageType, UInt16 index);
Result_t SMS_SetAllNewMsgDisplayPref(UInt8* mode, UInt8* mt, UInt8* bm, UInt8* ds, UInt8* bfr);
Result_t SMS_SetCBIgnoreDuplFlag(Boolean ignoreDupl);
Result_t SMS_SetCellBroadcastMsgTypeReq(UInt8 clientID, UInt8 mode, UInt8* chnlIDs, UInt8* codings);
Result_t SMS_SetMoSmsTpMr(UInt8 clientID, UInt8* tpMr);
Result_t SMS_SetNewMsgDisplayPref(NewMsgDisplayPref_t type, UInt8 mode);
Result_t SMS_SetSMSPrefStorage(SmsStorage_t storageType);
Result_t SMS_SetSmsTxParamCodingType(SmsCodingType_t* codingType);
Result_t SMS_SetSmsTxParamCompression(Boolean compression);
Result_t SMS_SetSmsTxParamProcId(UInt8 pid);
Result_t SMS_SetSmsTxParamRejDupl(Boolean rejDupl);
Result_t SMS_SetSmsTxParamReplyPath(Boolean replyPath);
Result_t SMS_SetSmsTxParamStatusRptReqFlag(Boolean srr);
Result_t SMS_SetSmsTxParamUserDataHdrInd(Boolean udhi);
Result_t SMS_SetSmsTxParamValidPeriod(UInt8 validatePeriod);
Result_t SMS_SetVMIndOnOff(Boolean on_off);
Result_t SMS_StartCellBroadcastWithChnlReq(UInt8 clientID, MS_MN_CB_MSG_IDS* cbmi, MS_MN_CB_LANGUAGES* lang);
Result_t SMS_StartMultiSmsTransferReq(UInt8 clientID);
Result_t SMS_StartReceivingCellBroadcastReq(UInt8 clientID);
Result_t SMS_StopMultiSmsTransferReq(UInt8 clientID);
Result_t SMS_StopReceivingCellBroadcastReq(UInt8 clientID);
Result_t SMS_UpdateVmscNumberReq(UInt8 clientID, SIM_MWI_TYPE_t vmsc_type, UInt8* vmscNum, UInt8 numType, UInt8* alpha, UInt8 alphaCoding, UInt8 alphaLen);
Result_t SMS_WriteSMSPduReq(UInt8 clientID, UInt8 length, UInt8* inSmsPdu, Sms_411Addr_t* sca, SmsStorage_t storageType);
Result_t SMS_WriteSMSPduToSIMReq(UInt8 clientID, UInt8 length, UInt8* inSmsPdu, SIMSMSMesgStatus_t smsState);
Result_t SMS_WriteSMSReq(UInt8 clientID, UInt8* inNum, UInt8* inSMS, SmsTxParam_t* params, UInt8* inSca, SmsStorage_t storageType);
SmsEnhancedVMInd_t* SMS_GetEnhancedVMInfoIEI(UInt8 clientID);
SmsStorage_t SMS_GetSMSPrefStorage(void);
UInt16 SMS_GetSmsMaxCapacity(SmsStorage_t storageType);
UInt8 SMS_GetLastTpMr(void);
UInt8 SMS_GetNewMsgDisplayPref(NewMsgDisplayPref_t type);
void SMS_SendAckToNetwork(SmsMti_t mti, SmsAckNetworkType_t ackType);
void SMS_SendMemAvailInd(void);
void SMS_SetSmsReadStatusChangeMode(Boolean mode);
void SMS_SetSmsStoredState (SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t status);
SMS_BEARER_PREFERENCE_t	GetSMSBearerPreference(void);
void SetSMSBearerPreference(SMS_BEARER_PREFERENCE_t inPref);
// End of Old API definitions

#endif //_SMS_API_OLD_H_
