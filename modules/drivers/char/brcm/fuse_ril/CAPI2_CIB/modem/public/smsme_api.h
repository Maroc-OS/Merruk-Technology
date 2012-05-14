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
*   @file   smsme_api.h
*
*   @brief  This file contains definitions for ME storage API.
*
****************************************************************************/

#ifndef _SMSME_H_
#define _SMSME_H_
   
Boolean SMS_IsMeStorageEnabled(void);

UInt16 SMS_GetMaxMeCapacity(void);

UInt16 SMS_GetNextFreeSlot(void);

Result_t SMS_SetMeSmsStatus(UInt16 slotNumber, SIMSMSMesgStatus_t status);
Result_t SMS_GetMeSmsStatus(UInt16 slotNumber, SIMSMSMesgStatus_t* status);

Result_t SMS_StoreSmsToMe(UInt8* inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber);

Result_t SMS_RetrieveSmsFromMe(UInt16 slotNumber);

Result_t SMS_RemoveSmsFromMe(UInt16 slotNumber);

Result_t SMS_GetMeSmsBufferStatus(UInt16 *NbFree,
								  UInt16 *NbUsed);

Boolean SMS_IsSmsReplSupported(void);

UInt16 SMS_GetRecordNumberOfReplaceSMS(SmsStorage_t storageType,UInt8 tp_pid,UInt8* oaddress);

#endif  // _SMSME_H_

