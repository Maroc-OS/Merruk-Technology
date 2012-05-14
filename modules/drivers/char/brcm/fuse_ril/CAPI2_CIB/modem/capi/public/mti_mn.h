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
*   @file   mti_mn.h
*
*   @brief  This file contains the prototypes for MTI MN functions.
*
*****************************************************************************/
#ifndef _MSC_MN_H_
#define _MSC_MN_H_


//******************************************************************************
// Callbacks
//******************************************************************************
typedef void (*CB_ActivateResp_t) (
	ClientInfo_t			*clientInfoPtr,
	PCHResponseType_t		response,
	PCHNsapi_t				nsapi,
	PCHSapi_t				sapi,
	PCHPDPType_t			pdpType,
	PCHPDPAddress_t			pdpAddress,
	PCHQosProfile_t			*qos,
	PCHProtConfig_t			*protConfig,
	UInt8					radioPriority,
	PCHPDPAddressIE_t		pdpAddressIE,
	PCHRejectCause_t		cause,
	PCHActivateReason_t		reason
	);  // i.e. mnActivateCb()

typedef void (*CB_ActivateSecResp_t) (
	ClientInfo_t				*clientInfoPtr,
	PCHResponseType_t			response,
	PCHNsapi_t					nsapi,
	PCHSapi_t					sapi,
	PCHQosProfile_t				*qos,
	Boolean						pfi_ind,
	UInt8						indicatedPFI,
	UInt8						radioPriority,
	PCHRejectCause_t			cause
	);  // i.e. mnActivateSecCb()

typedef void (*CB_ModifyResp_t) (
	ClientInfo_t			*clientInfoPtr,
	PCHResponseType_t		response,
	PCHNsapi_t				nsapi,
	PCHSapi_t				sapi,
	PCHQosProfile_t			*qos,
	Boolean					pfi_ind,
	UInt8					indicatedPFI,
	PCHProtConfig_t			*protConfig,
	UInt8					radioPriority,
	PCHRejectCause_t		cause
	);

typedef void (*CB_DeactivateResp_t) (
	ClientInfo_t			*clientInfoPtr,
	PCHResponseType_t		response,
	PCHNsapi_t				nsapi,
	PCHDeactivateReason_t	reason
	);

//******************************************************************************
// Function Prototypes
//******************************************************************************

void MN_Init( void );					// Initialize the call-control manager

void MN_Shutdown( void );				// Shut down routine

void MN_Run( void );					// Run routine

void MN_Register(void);

void MN_ReportSIMGeneralServiceStatus(	// Report SIM General Service Status change
	ClientInfo_t* inClientInfoPtr,
	Boolean	sim_general_service_ready
	);

void MN_SetCallActionReq(	ClientInfo_t*	inClientInfoPtr,
							CallIndex_t		inCallindex,
							CallAction_t	action);

void MN_SetDataCallMode(			// Callback to set the Data Call Mode
	DataCallMode_t mode
	);

void MN_ReportSIMACMUpdate(			// Report SIM ACM/ACMmax value update
	void
	);


		
////////////////////////////////////////////

void MN_ActivateReq( 
	ClientInfo_t			*clientInfoPtr,
	CB_ActivateResp_t		cb,	
	PCHNsapi_t				nsapi,
	PCHSapi_t				sapi,
	PCHPDPType_t			pdpType,
	PCHPDPAddress_t			pdpAddress,
	PCHQosProfile_t			*qos,
	PCHAPN_t				apn,
	PCHProtConfig_t			*protConfig,
	PCHActivateReason_t		reason,
	UInt8				cid
	);

void MN_ActivateSecReq( 
	ClientInfo_t				*clientInfoPtr,
	CB_ActivateSecResp_t		cb,	
	PCHNsapi_t					priNsapi,
	PCHNsapi_t					secNsapi,
	PCHSapi_t					sapi,
	PCHQosProfile_t				*qos,
	PCHTrafficFlowTemplate_t	*tft,
    PCHProtConfig_t			    *protConfig,
	Int32				        secTiPd, 
	// bMTPdp is TRUE for MT when the secondary PDP activation is initiated by the Network;
	// bMTPdp is FALSE for MO when secondary PDP activation is initiated by the MS.
	Boolean                     bMTPdp,
	UInt8				        cid
	);

void MN_ModifyReq( 
	ClientInfo_t			*clientInfoPtr,
	CB_ModifyResp_t			cb,	
	PCHNsapi_t				nsapi,
	PCHSapi_t				sapi,
	PCHQosProfile_t			*qos,
	PCHTrafficFlowTemplate_t	*tft,
	PCHProtConfig_t			*protConfig
	);

void MN_DeactivateReq(
	ClientInfo_t			*clientInfoPtr,
	CB_DeactivateResp_t		cb,
	PCHNsapi_t				nsapi,
	PCHDeactivateReason_t	reason
	);

void MN_DeactivateReq_QOS(
	ClientInfo_t			*clientInfoPtr,
	CB_DeactivateResp_t		cb,
	PCHNsapi_t				nsapi,
	PCHDeactivateReason_t	reason
	);

void MN_ActivateRsp(
	ClientInfo_t			*clientInfoPtr,
	PCHPDPAddress_t			pdpAddress,
	PCHRejectCause_t    	cause,
	PCHAPN_t			apn
	);

void MN_ActivateSecRsp(
	ClientInfo_t			*clientInfoPtr,
	Int32					secTiPd,
	PCHRejectCause_t		cause,
	PCHProtConfig_t 		*protConfig //ifdef PDP_2nd
	);

void MN_CheckQminRsp(ClientInfo_t	*clientInfoPtr,PCHNsapi_t	nsapi);

void MN_ReportSIMRemovedStatus(ClientInfo_t* clientInfoPtr);

Boolean MN_IsWaitingSwapingCalls(void);

CallIndex_t	MNAOC_FindCallIndex(CallTimeIndex_t ctime_idx);
//CallTimeIndex_t	MNAOC_FindCallTimeIndex(CallIndex_t c_idx);

void MN_HandleEccListInd(InterTaskMsg_t *InterMsg);

Boolean MNIsUserHangUpInProgress(void);

void MNReportCallMOReleaseCnf(	CallDesc_t*	inCcCtxPtr,
								Cause_t		cause);

void MN_ReportCCCallRelease(CallDesc_t* inCcCtxPtr, Cause_t inCause);

void MNReportCallRelease(	ClientInfo_t*	inClientInfoPtr,
							CallIndex_t		index,
							Cause_t			cause,
							Duration_t		duration,
							CallMeterUnit_t	CCM_units);

void MN_PostMsgToMn(void* inMsgPtr, MnSap_t inSap);

#endif

