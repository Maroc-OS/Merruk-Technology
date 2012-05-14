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
*   @file   pch_util.h
*
*   @brief  This file contains misc definitions for PCH (GPRS PDP Context Handler) module.
*
*
****************************************************************************/

#ifndef _PCH_UTIL_
#define _PCH_UTIL_




typedef struct
{
	PCHNsapi_t				nsapi;
	PCHQosProfile_t			qosProfile;
	PCHProtConfig_t			protConfig; 
    PCHTrafficFlowTemplate_t tft;
}Inter_CheckQoSMinInd_t;

typedef struct
{
	PCHNsapi_t				nsapi;
	UInt8					cause;
	Boolean					reActivateInd;
}Inter_DeactivateInd_t;

typedef struct
{
	PCHPDPType_t			pdpType;
	PCHPDPAddress_t			pdpAddress;
	PCHAPN_t				apn;
	PCHProtConfig_t			protConfig; 	
}Inter_ActivateInd_t;

typedef struct
{
	PCHNsapi_t					priNsapi;	
	PCHQosProfile_t 			qos;
	PCHTrafficFlowTemplate_t	tft;
	PCHProtConfig_t				protConfig; 
    Int32						secTiPd;
}Inter_ActivateSecInd_t;

typedef struct 
{
	PCHResponseType_t		response;
	PCHActivateReason_t		reason;
	PCHNsapi_t				nsapi;
	PCHSapi_t				sapi;
	PCHRejectCause_t		cause;
	PCHPDPType_t			pdpType;		//char[20]
	PCHPDPAddress_t			pdpAddress;		//char[20]
	PCHQosProfile_t			qos;			//UInt8 *5
	PCHProtConfig_t			protConfig;		//UInt8 + UInt8*PCH_PROT_CONFIG_OPTIONS_SIZE
	UInt8					radioPriority;
	PCHPDPAddressIE_t		pdpAddressIE;
}Inter_ActivateCnf_t;

typedef struct 
{
	PCHResponseType_t		response;
	PCHNsapi_t				nsapi;
	PCHSapi_t				sapi;
	PCHQosProfile_t			qos;
	Boolean					pfi_ind;
	UInt8					indicatedPFI;
	PCHRejectCause_t		cause;
	UInt8					radioPriority;
}Inter_ActivateSecCnf_t;

typedef struct 
{
	PCHResponseType_t		response;
	PCHNsapi_t				nsapi;
	PCHSapi_t				sapi;
	PCHQosProfile_t			qos;
	Boolean					pfi_ind;
	UInt8					indicatedPFI;
	PCHRejectCause_t		cause;
	UInt8					radioPriority;
}Inter_ModifyCnf_t;

typedef struct
{
	PCHResponseType_t		response;
	PCHNsapi_t				nsapi;
	PCHDeactivateReason_t	reason;
}Inter_DeactivateCnf_t;

typedef struct
{
	PCHNsapi_t				nsapi;
  	PCHXid_t				xid;
}Inter_SnXidCnf_t;



typedef struct
{
	UInt8					accessType;
	UInt8					cid;
	UInt8					priCid;
}ATCGACTSECData_t;



//******************************************************************************
// Internal Function Prototypes - Not available for MMI/ATC
//******************************************************************************
void			ProcessRegistInfo(
					ClientInfo_t			*clientInfoPtr,
					PchRespType_t			pchType,			  
					RegisterStatus_t		cs_status,
					UInt8					cs_cause,
					RegisterStatus_t		gprs_status,
					UInt8					gprs_cause,
					MMPlmnInfo_t			plmnInfo,
					MMNetworkInfo_t			networkInfo, 
					Boolean					plmn_search_ended
				);

//API prototypes

void			HandleCheckQoSMinInd(InterTaskMsg_t *InterMsg);
void			HandleDataStateTimer(InterTaskMsg_t *InterMsg);
void			HandleCgsendTimer(InterTaskMsg_t *InterMsg);
void			HandleDeactivateInd(InterTaskMsg_t *InterMsg);
void			HandleActivateInd(InterTaskMsg_t *InterMsg);
void 			HandleActivateSecInd(InterTaskMsg_t *InterMsg);
void			HandleModifyContextInd(InterTaskMsg_t *InterMsg);



void			RouteSNDataInd( T_NPDU_PTR	*npdu_ptr, UInt8 rat);
Boolean			foundTblPtrByCid(PCHCid_t cid, PCHContextTbl_t **tbl_p);
Boolean			foundTblPtrByNsapi(PCHNsapi_t nsapi, PCHContextTbl_t **tbl_p);
UInt8			getNsapiForCid(UInt8 cid, PCHActivateReason_t reason);
UInt8			cleanNsapiForCid(UInt8 cid);
UInt8			compareQOS( PCHQosProfile_t min, PCHQosProfile_t incoming);
Result_t		getPDPCauseFromSMCause(UInt8 cause);
PCHRejectCause_t getSMCauseFromPDPCause(Result_t cause);

void			sendOneActivateReq(
								ClientInfo_t 			*clientInfoPtr,
								PCHContextTbl_t		*pTbl,
								PCHProtConfig_t		*pProtConfig,
								PCHActivateReason_t	reason,
								UInt8			cid
								);	


void			startActTimer(UInt8 cid);
void			stopActTimer(UInt8 cid);
void			startModifyTimer(UInt8 cid);
void			stopModifyTimer(UInt8 cid);
void			startDeactTimer(UInt8 cid);
void			stopDeactTimer(UInt8 cid);
void			startSnXidTimer(UInt8 cid);
void			stopSnXidTimer(UInt8 cid);
void			startCgsendTimer(UInt8 cid, UInt32 duration);
void			stopCgsendTimer(UInt8 cid);


void			resetStoredActInfo(UInt8 cid);
void			resetStoredModifyInfo(UInt8 cid);
void			resetStoredDeactInfo(UInt8 cid);
void			resetStoredDataStateInfo(UInt8 cid);

void			reportDeactIndToSTK(UInt8 cid, Result_t cause, Boolean isReAct);


void pch_fakeActSecCnf(const UInt8* _P1,const UInt8* _P2,const UInt8* _P3,const UInt8* _P4,
					    const UInt8* _P5,const UInt8* _P6,const UInt8* _P7,const UInt8* _P8);

Result_t		PCH_ReadDecodedProtConfig(UInt8 cid, PCHDecodedProtConfig_t* out);



PCHCid_t		PDP_GetPPPCid(void); 
void 		PDP_SetPPPCid(PCHCid_t cid);

// Need to specify bNet2MS in PCHDecodedProtConfig_t{} when
// calling  PCHEx_ReadProtConfig(); 
// The default for bNet2MS is FALSE to indicate the direction is from MS to network.
Result_t PCHEx_ReadProtConfig( PCHProtConfig_t *ipcnfgl, PCHDecodedProtConfig_t* out );

void GenChapRequest(CHAP_ChallengeOptions_t* cc, char* chal_name);

void GenChapResponse(CHAP_ChallengeOptions_t*cc, 
					 CHAP_ResponseOptions_t *cr, 
					 char* secret, 
					 char* chal_name);

void UPAPGenWapAuthEx(PAP_CnfgOptions_t *po, char * user,  char * password);



typedef void (*PCHGPRS_RouteSNDataCb_Stack_t) (UInt8 cid, T_NPDU_PTR	*npdu_ptr, UInt8 rat);


extern			PCHGPRS_RouteSNDataCb_Stack_t		routeSNDataToModemCb;
extern			PCHGPRS_RouteSNDataCb_Stack_t		routeSNDataToIpRelayCb;
extern			PCHGPRS_RouteSNDataCb_Stack_t		routeSNDataToSTKCb;


void			PDP_RegisterRouteSNDataInd( 
					PCHGPRS_RouteSNDataCb_Stack_t		sendSNDataToModem,
					PCHGPRS_RouteSNDataCb_Stack_t		sendSNDataToIpRelay,
					PCHGPRS_RouteSNDataCb_Stack_t		sendSNDataToSTK );

void SetPktFilterDirection(
			PDPDefaultContext_t 	*pContext,	  // the target secondary PDP context
			PCHPacketFilter_T		*pPktFilter); // the packet filter to be modified

UInt8 SetTftOpcode(
	        PCHTrafficFlowTemplate_t 	*inOldTftPtr,
	        PCHTrafficFlowTemplate_t 	*inNewTftPtr );

void PrntTft(
			PCHTrafficFlowTemplate_t* 	inTftPtr); // the TFT for output

void PrntOneTftPacketFilter(
			PCHPacketFilter_T*			inPktFilterPtr); // the packet filter for output
	

void PrntOneTftParamList(
			PCHTFTParamList_t* 		    inParamListPtr); // the paramter list for output


#endif //_PCH_UTIL_

