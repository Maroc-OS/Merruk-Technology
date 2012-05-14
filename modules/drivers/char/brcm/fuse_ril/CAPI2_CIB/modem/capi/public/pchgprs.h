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
*   @file   pchgprs.h
*
*   @brief  This file contains definitions for PCH handler prototypes.
*
****************************************************************************/


#ifndef _PCH_PCHGPRS_H_
#define _PCH_PCHGPRS_H_


//******************************************************************************
// Function Prototypes
//******************************************************************************

void	initAllContextTblWithCtxtFromNVRAM( Boolean update_all );

void	initContextTblEntry(
			PCHContextTbl_t		*tbl_p
			);

Boolean	PCH_CheckTft(PCHTrafficFlowTemplate_t *pTft, Boolean isSecondary, UInt8 priCid);
void	pch_init_prot_struct(PCHContextTbl_t	*tbl_p, UInt8 cid);

void	mnActivateSecCb(
			ClientInfo_t				*clientInfoPtr,
			PCHResponseType_t			response,
			PCHNsapi_t					nsapi,
			PCHSapi_t					sapi,
			PCHQosProfile_t				*qos,
			Boolean						pfi_ind,
			UInt8						indicatedPFI,
			UInt8					radioPriority,
			PCHRejectCause_t			cause
			);

void	mnModifyCb(
			ClientInfo_t				*clientInfoPtr,
			PCHResponseType_t			response,
			PCHNsapi_t					nsapi,
			PCHSapi_t					sapi,
			PCHQosProfile_t				*qos,
			Boolean						pfi_ind,
			UInt8						indicatedPFI,
			PCHProtConfig_t				*protConfig,
			UInt8					radioPriority,
			PCHRejectCause_t				cause
			);


void			sendOneDeactivateReq(
						ClientInfo_t			*clientInfoPtr,
						PCHContextTbl_t			*tbl_p,
						PCHDeactivateReason_t	reason
						);


void			sendSnXidCnf(
						PCHNsapi_t			nsapi,
  						PCHXid_t			xid
						);
	
void 		sendPDPActivatePDU(
						UInt8	clientID,
						UInt8	cid,
						PCHActivateReason_t	reason,
						PCHProtConfig_t		*protConfig
						);
	
void			HandleActivateCnf(InterTaskMsg_t *InterMsg);
void			HandleDeactivateCnf(InterTaskMsg_t *InterMsg);
void			HandleSnXidCnf(InterTaskMsg_t *InterMsg);

void			pchActTimerEvent(TimerID_t Id);
void			pchDeactTimerEvent(TimerID_t Id);
void			pchSnXidTimerEvent(TimerID_t Id);
void			pchCgsendTimerEvent(TimerID_t Id); 

void			SendActivateTimeOut(UInt8 cid);
void			SendDeactivateTimeOut(UInt8 cid);
void			SendSnXidCnfTimeOut(UInt8 cid);
void			SendDataStateTimeOut(void);
void			SendCgsendTimeOut(UInt8 cid);


void			HandleActivateSecCnf(InterTaskMsg_t *InterMsg);
void			HandleModifyCnf(InterTaskMsg_t *InterMsg);

void			SendModifyTimeOut(UInt8 cid);

T_OS_PP_ID 		PDP_GetSNUpPoolId(UInt8 cid);
void 			PCH_ProcessAttachCnf(ClientInfo_t *clientInfoPtr, PCHAttachReason_t reason, PCHResponseType_t response, RegisterStatus_t status);

extern	PCHProtConfig_t PPPprotConfig;
extern	UInt8			*pDataStateBuf;
extern	Boolean			Is_Create_Pool;

void pchMapQosToQos99(PCHQosProfile_t	*pQos);
void pchMapQos99ToQos(PCHQosProfile_t	*pQos);

//***************************************************************************************
/**
	Function to get PCH context activate reason for the given cid
	@param		cid (in)  context id
	@return		PCHActivateReason_t  PCH context activate reason
	@note
**/
PCHActivateReason_t PDP_GetPCHContextActivateReason(UInt8 cid);

Boolean 		IsFirstAttachSent(void);

PDPDefaultContext_t*	PCH_GetPDPDefaultContextPtr(void);

#endif //_PCH_PCHGPRS_H_

