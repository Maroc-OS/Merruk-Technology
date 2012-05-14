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
*   @file   callctrl.h
*
*   @brief  This file defines the interface Internal call control functions.
*
*****************************************************************************/
#ifndef _CALLCTRL_H
#define _CALLCTRL_H



//Internal functions for using for STK to report to CC

Result_t CC_SetCurrentCallSimOrigined(Boolean simOrigined);

//Internal functions to update the call state and
//call action; they will be called directly by MSC
//before sending the messages to the upper layer.  

void CC_ReportCallReceive(	CallIndex_t		index,
							CallingParty_t*	inCallPartyPtr,
							CUGIndex_t		cug_index,
							Boolean			isInternationalCall);

void CC_ReportCallMeterVal(UInt8	callIndex,
						UInt32	callCCMUnit,
						UInt32	callDuration);

void CC_ReportDataCallBytes(UInt32 rxBytes,UInt32 txBytes);

void CC_ReportCallRelease(	CallIndex_t     index,
						Cause_t         cause);
					
Result_t CC_ReportCallActionResult( CallIndex_t index, 
								  CallAction_t action,
								  CallActionStatus_t result);

Result_t CC_ReportCallConnect(CallIndex_t		index, 
							CallStatus_t	status);

void CC_ReportCallConnectedID(CallDesc_t* inCcCtxPtr);

void CC_ReportMakeLinkRst(UInt8 ec_est_mode,UInt8 dc_mode);


void CC_ReportDataCallRelease(UInt8			callIndex,
						   MNATDSMsg_t*  mnatds_msg);

void CC_ReportDataCallConnect(UInt8 callIndex);

Result_t CC_MakeCall(	ClientInfo_t*	inClientInfoPtr,
						CCallType_t		callType, 
						UInt8*			callNum,
						void*			callParam,
						Boolean			isEmergency);

Result_t ProcessCall(	ClientInfo_t*	inClientInfoPtr,
						UInt8*			inDialStrPtr,
						CCallType_t		inCallType);

void ProcessApiClientCmd(	ClientInfo_t*	inClientInfoPtr,
							CCallType_t		inCallType,
							UInt8			inCallIndex,
							Result_t		inResult);

void CC_PostApiClientCmpInd(ClientInfo_t*	inClientInfoPtr,
							UInt8			inCallIndex,
							CCallType_t		inCallType,
							ClientCmd_t		inClientCmd,
							Result_t		inResult);

Boolean CC_ECTAllowedCondition( void );

//******************************************************************************
// Function Name:	CC_SetSpeechCodec
//
// Description:		This function takes the codec bitmap as input and updates the channel mode
//					used by the modem. The codec bitmap specifies AMR,EFR, GMS half/full rate speech
//					versions configured by the user.
//******************************************************************************


Result_t CC_SetSpeechCodec(	MNCC_Module_t	*inMnccCtxPtr,  
							MS_SpeechCodec_t inCodecBitmap);

//**************************************************************************************
/**
	Initialize default Call Config parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCurCallCfgPtr (in) Current Call Configuration
	@return		Void
	@note 
		Initialize default parameters for Call type, Call duration, Call meter etc.
		The difference between this and CC_InitCallControl is that this function
		does not initialize error correction and data compression module although
		those values belonging to call config are initalized.
		
		See CallConfig_t for parameters being initialized.
**/
void CcInitCallCfg(ClientInfo_t* inClientInfoPtr,
				   CallConfig_t* inCurCallCfgPtr);

//**************************************************************************************
/**
	Function to initialize Call Config parameters when using AT&F command
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCurCallCfgPtr (in) Current Call Configuration
	@return		Void
	@note
		Using this initialization means that some parameters will not be initialized.
		This is done specifically because typical DUNs issue a AT&F command just before 
		dialing. This will mean that settings such as CBST, HSCSD will go back to default.
		Calling this function will avoid this unpleasent situation.
**/
void CcInitCallCfgAmpF(	ClientInfo_t* inClientInfoPtr,
						CallConfig_t* inCurCallCfgPtr);

//**************************************************************************************
/**
	Initialize default Fax Call Config parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inFaxCfgPtr (in) Current Fax Configuration
	@return		Void
**/
void CcInitFaxConfig(	ClientInfo_t*	inClientInfoPtr,
						Fax_Param_t*	inFaxCfgPtr);

//**************************************************************************************
/**
	Initialize default Video Call Config parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCurCallCfgPtr (in) Current Call Configuration
	@return		Void
**/
void CcInitVideoCallCfg(ClientInfo_t*	inClientInfoPtr,
						CallConfig_t*	inCurCallCfgPtr);
#endif //_CALLCTRL_H


