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
*   @file   pchex_def.h
*
*   @brief  This file defines the extended capi api's related definition to establish 
*			data connection and data path for PS data.
*
****************************************************************************/
/**
********************************************************************************************/
#ifndef _PCHEX_DEF_H_
#define _PCHEX_DEF_H_
#ifdef __cplusplus
extern "C" {
#endif


/// Send/Receive Octet count
typedef struct {
	UInt32				dataSentSize;					///< Data sent size
	UInt32				dataRcvSize;					///< Data Receive Size
} PSDataByteCount_t;

/// UL Data Indication
typedef struct
{
	PCHCid_t 			cid;
	Boolean				bFree;
	UInt16				datalen;
	UInt8*				pData;
}PDP_ULData_Ind_t;


/// Flow control events
typedef enum
{
	PSDATA_FLOW_START,	///< Start flow control
	PSDATA_STOP			///< Stop flow control
} PCHEx_FlowCtrlEvent_t;


/// Flow control event callback function
typedef void		(*PCHEx_FlowCntrl_t)			(UInt8 cid, PCHEx_FlowCtrlEvent_t Event);
typedef void		(*PCHExApi_FlowCntrl_t)			(ClientInfo_t	*clientInfoPtr, UInt8 cid, PCHEx_FlowCtrlEvent_t Event);
typedef void (*PCHExApi_RouteSNDataCb_t) (
	ClientInfo_t			*clientInfoPtr,
	UInt8 				cid, 			
	MS_T_NPDU_PTR		*npdu_ptr, 
	UInt8 				rat);

typedef PCHActivateReason_t (*PCHEx_GetActivateReason_t)	(UInt8 cid);
typedef void (*PCHGPRS_TM_RouteSNDataCb_t) (UInt8 cid,UInt16 dataLen, UInt8 * pData);

/// Parameters to make the protocol config options with CHAP authentication type
typedef struct {
	UInt8* challengeData;	///<  challenge data
	UInt8 challengeLen;		///<  challenge data length
	UInt8 challengeId;		///<  challenge id
	UInt8* rspData;			///<  challenge response data(Final MD5 hash)
	UInt8 rspLen;			///<  challenge response length
	UInt8 rspId;			///<  challenge response id
	UInt8* usrNameData;		///<  user name
	UInt8 usrNameLen;		///<  username length
} PCHEx_ChapAuthType_t;



#ifdef __cplusplus
}
#endif

#endif //_PCHEX_DEF_H_
