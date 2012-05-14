/*********************************************************************
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
***************************************************************************/
/**
*
*   @file   dataacct_old_api.h
*
*   @brief  This file contains the interface function prototypes to the Data Account 
			Services.
*
****************************************************************************/
/**

*   @defgroup   DATAAccountGroup   Data Account Management
*   @ingroup    DATAGroup
*
*   @brief      This group defines the interfaces to create and manage 
				data accounts.
*
*	The Data Account Management API provides services to create and 
	manage circuit switch and packet switch accounts.  A data account
	contains all the information that is needed to establish a connection
	with the network.
*
****************************************************************************/

#ifndef _DATAACCT_OLD_API_H_
#define _DATAACCT_OLD_API_H_
/////////////////////////////////////////////////////////////////////////////////////////
//
//  The following are old Data Account Service API functions
//
Boolean			DATA_IsAcctIDValid(UInt8 acctID);
Result_t		DATA_CreateGPRSDataAcct(UInt8 acctID, GPRSContext_t *pGprsSetting);
Result_t		DATA_CreateCSDDataAcct(UInt8 acctID, CSDContext_t *pCsdSetting);
Result_t		DATA_DeleteDataAcct(UInt8 acctID);
Result_t		DATA_GetGPRSContext(UInt8 acctID, GPRSContext_t *pGprsContext);
Result_t		DATA_GetCSDContext(UInt8 acctID, CSDContext_t *pCsdContext);
Result_t		DATA_SetUsername(UInt8 acctID, UInt8 *username);
UInt8*			DATA_GetUsername(UInt8 acctID);
Result_t		DATA_SetPassword(UInt8 acctID, UInt8 *password);
UInt8*			DATA_GetPassword(UInt8 acctID);
Result_t		DATA_SetStaticIPAddr(UInt8 acctID, UInt8 *staticIPAddr);
const UInt8*	DATA_GetStaticIPAddr(UInt8 acctID);
Result_t		DATA_SetPrimaryDnsAddr(UInt8 acctID, UInt8 *priDnsAddr);
const UInt8*	DATA_GetPrimaryDnsAddr(UInt8 acctID);
Result_t		DATA_SetSecondDnsAddr(UInt8 acctID, UInt8 *sndDnsAddr);
const UInt8*	DATA_GetSecondDnsAddr(UInt8 acctID);
Result_t		DATA_SetDataCompression(UInt8 acctID, Boolean dataCompEnable);
Boolean			DATA_GetDataCompression(UInt8 acctID);
DataAccountType_t		DATA_GetAcctType(UInt8 acctID);
UInt8			DATA_GetEmptyAcctSlot(void);
UInt8			DATA_GetCidFromDataAcctID(UInt8 acctID);
UInt8			DATA_GetDataAcctIDFromCid(UInt8 cid);
UInt8			DATA_GetPrimaryCidFromDataAcctID(UInt8 acctID);
Boolean 		DATA_IsSecondaryDataAcct(UInt8 acctID);
UInt32			DATA_GetDataSentSize(UInt8 acctId);
UInt32			DATA_GetDataRcvSize(UInt8 acctId);
Result_t		DATA_SetGPRSPdpType(UInt8 acctID, UInt8 *pdpType);
const UInt8*	DATA_GetGPRSPdpType(UInt8 acctID);
Result_t		DATA_SetGPRSApn(UInt8 acctID, UInt8 *apn);
const UInt8*	DATA_GetGPRSApn(UInt8 acctID);
Result_t		DATA_SetAuthenMethod(UInt8 acctID, DataAuthenMethod_t authenMethod);
DataAuthenMethod_t		DATA_GetAuthenMethod(UInt8 acctID);
Result_t		DATA_SetGPRSHeaderCompression(UInt8 acctID, Boolean headerCompEnable);
Boolean			DATA_GetGPRSHeaderCompression(UInt8 acctID);
Result_t		DATA_SetGPRSQos(UInt8 acctID, PCHQosProfile_t qos);
const PCHQosProfile_t*	DATA_GetGPRSQos(UInt8 acctID);
Result_t		DATA_SetAcctLock(UInt8 acctID, Boolean acctLock);
Boolean			DATA_GetAcctLock(UInt8 acctID);
Result_t		DATA_SetGprsOnly(UInt8 acctID, Boolean gprsOnly);
Boolean			DATA_GetGprsOnly(UInt8 acctID);
Result_t		DATA_SetGPRSTft(UInt8 acctID, PCHTrafficFlowTemplate_t* pTft);
Result_t		DATA_GetGPRSTft(UInt8 acctID, PCHTrafficFlowTemplate_t* pTft);
Boolean			DATA_CheckTft(PCHTrafficFlowTemplate_t *pTft, Boolean isSecondary, UInt8 priCid);
UInt8*			DATA_GetChapChallenge(UInt8 acctID);
UInt8 			DATA_GetChapChallengeLen(UInt8 acctID);
UInt8*			DATA_GetChapResponse(UInt8 acctID);
UInt8 			DATA_GetChapResponseLen(UInt8 acctID);
UInt8*			DATA_GetChapUserId(UInt8 acctID);
UInt8 			DATA_GetChapUserIdLen(UInt8 acctID);
UInt8 			DATA_GetChapID(UInt8 acctID);
Result_t		DATA_SetCSDDialNumber(UInt8 acctID, UInt8 *dialNumber);
const UInt8*	DATA_GetCSDDialNumber(UInt8 acctID);
Result_t		DATA_SetCSDDialType(UInt8 acctID, CSDDialType_t dialType);
CSDDialType_t	DATA_GetCSDDialType(UInt8 acctID);
Result_t		DATA_SetCSDBaudRate(UInt8 acctID, CSDBaudRate_t baudRate);
CSDBaudRate_t	DATA_GetCSDBaudRate(UInt8 acctID);
Result_t		DATA_SetCSDSyncType(UInt8 acctID, CSDSyncType_t synctype);
CSDSyncType_t	DATA_GetCSDSyncType(UInt8 acctID);
Result_t		DATA_SetCSDErrorCorrection(UInt8 acctID, Boolean enable);
Boolean			DATA_GetCSDErrorCorrection(UInt8 acctID);
Result_t		DATA_SetCSDErrCorrectionType(UInt8 acctID, CSDErrCorrectionType_t errCorrectionType);
CSDErrCorrectionType_t	DATA_GetCSDErrCorrectionType(UInt8 acctID);
Result_t		DATA_SetCSDDataCompType(UInt8 acctID, CSDDataCompType_t dataCompType);
CSDDataCompType_t		DATA_GetCSDDataCompType(UInt8 acctID);
Result_t		DATA_SetCSDConnElement(UInt8 acctID, CSDConnElement_t connElement);
CSDConnElement_t		DATA_GetCSDConnElement(UInt8 acctID);
Result_t		DATA_CreateWLANDataAcct(UInt8 acctID, WLANData_t *pWlanSetting);
Result_t		DATA_SetWLANDataAcct(UInt8 acctID, WLANData_t *pWlanSetting);
Result_t		DATA_GetWLANSetting(UInt8 accId, WLANData_t *pWlanSetting);

// End of Old API definitions
/////////////////////////////////////////////////////////////////////////////////////////

#endif // _DATAACCT_OLD_API_H_
