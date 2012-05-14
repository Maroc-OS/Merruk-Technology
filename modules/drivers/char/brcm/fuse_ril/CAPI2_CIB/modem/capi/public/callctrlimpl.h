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
*   @file   callctrlimpl.h
*
*   @brief  The file contains definitions of call control implementation 
*			hiden from the call control API.
*
****************************************************************************/

#ifndef _CALLCTRLIMPL
#define _CALLCTRLIMPL



#define INVALID_MARK	102

///	Fax Class
typedef enum
{
        FCLASSSERVICEMODE_DATA = 0,			///< Fax class 0
        FCLASSSSERVICEMODE_FAX_2 = 2,		///< Fax class 2
        FCLASSSSERVICEMODE_FAX_2dot0 = 20	///< Fax class 2.0

} FClassServiceMode_t;


//Call structure to keep the data about a call
typedef struct{

	UInt8					callNum[MAX_DIGITS+2]; ///< (Note: 1 byte for null termination and the other for int code '+')
	PresentationInd_t		present;				///< Calling party presentation indicator
	CallingInfo_t           callingInfo;
	UInt8					callIndex;
	CCallType_t				callType;
	CCallState_t 			callState;
	Cause_t					callExitCause;
	Boolean					callMpty;
	Boolean					callConnectedIDPresent;
	UInt8					callConnectedID[MAX_DIGITS+2];///< (Note: 1 byte for null termination and the other for int code '+')

	UInt32					callStateTimeStamp;
	UInt32					callStartTimeStamp;
	Boolean					callSetupPhase;
	Boolean					callSimOrigined;

	UInt8					clientID;
	ClientInfo_t			clientInfo;
} CCallInfo_t;


///	Call Session
typedef enum{

	VOICECALL,		///< Voice Call session
	DATACALL,		///< Data Call	session
	FAXDATACALL,	///< Fax/Data Call
	MULTIPARTY,		///< Multiparty Voice Call
	IDLE_SESSION,	///< Idle session
	VIDEOCALL		///< Video Call

} CCallSession_t;


typedef struct{

	CCallSession_t	curCallSession;
	UInt8			curCallNum[MAX_DIGITS+2];///< (Note: 1 byte for null termination and the other for int code '+')
	UInt8           curMOSetupNum[MAX_DIGITS+2];///< (Note: 1 byte for null termination and the other for int code '+')
	UInt8			curCallNumType;//145 or 129
	UInt8			curCallIndex;
	CCallType_t		curCallType;
	//the index to the callinfo table 
	UInt8			curCallInteralID;

	UInt8           curMOSetupInteralID;

	//Next held call index
	UInt8			nextHeldCallIndex;
	//Next wait call index
	UInt8			nextWaitCallIndex;

	Cause_t			lastCallExitCause;
	UInt32			lastCallCCMUnit;
	UInt32			lastCallDuration;
	UInt32			lastDataCallRxBytes;
	UInt32			lastDataCallTxBytes;

} CCallCtrlCxt_t;


typedef struct{

	//For data call request parameter
	MNATDSParmSetupReq_t	dataCallReqParam;
	

} DataCallReqParam_t;

typedef struct{

	//For data call response parameter
	MNATDSParmSetupResp_t	dataCallRespParam;

}DataCallRespParam_t;


CCallInfo_t*	_CallInfoTable(void);
CCallCtrlCxt_t*	_CallCtrlCxt(void);

void		 _InitCallCtrlImpl(void);

UInt8 _AddOneNewCall(ClientInfo_t*	inClientInfoPtr,
					 UInt8*			callNum,
					 CCallType_t	callTy);

Boolean		 _CheckCallIndex(UInt8 callIndex);
Boolean		 _UpdateCallState(UInt8 callIndex,CCallState_t state);
Boolean		 _UpdateAllCallStates(CCallState_t state);
Boolean		 _UpdateCallExitCause(UInt8 callIndex,Cause_t cause);
Boolean		 _UpdateAllActiveCallStates(CCallState_t state);
Boolean		 _UpdateCallConnectTime(UInt8 callIndex);
Boolean		 _NewCurCallContext(UInt8 callIndex);
Boolean		 _CleanupCurCallContext(void);
Boolean		 _SetNextHeldCallIndex(UInt8 callIndex);
UInt8		 _GetNextHeldCallIndex(void);
Boolean		 _SetNextWaitCallIndex(UInt8 callIndex);
UInt8		 _GetNextWaitCallIndex(void);
UInt8        _GetNextBeginningCallIndex(void);
UInt8		 _GetNextActiveCallIndex(void);
Boolean		 _GetCallIndexInThisState(	UInt8*			inCallIndexPtr,
										CCallState_t	inCcCallState);
Boolean		 _DelOneCallByIndex(UInt8 callIndex);
Boolean		 _DelOneCallByPhoneNum(UInt8* callNum);
Boolean		 _DelOneCallByInternalID(UInt8 id);
Boolean		 _FinishCallSetupPhase(UInt8 callIndex);
Boolean		 _ThereIsCallInSetupPhase(void);
Boolean		 _ClearCallTable(void);
Boolean		 _GetAllCallStates(CCallStateList_t* stateList,UInt8* listSz);
Boolean		 _GetAllCallIndex(CCallIndexList_t* indexList,UInt8* listSz);
Boolean		 _GetAllCallIndexByState(CCallIndexList_t* indexList,UInt8* listSz,CCallState_t state);
Boolean		 _GetAllCallIndexInMPTY(CCallIndexList_t* indexList,UInt8* listSz,Boolean inMPTY);

Cause_t		 _GetCallExitCause(UInt8 callIndex);
CCallState_t _GetCallState(UInt8 callIndex);
CCallType_t	 _GetCallType(UInt8 callIndex);
Boolean		 _GetCallNumber(UInt8 callIndex,UInt8* phNum);
Boolean		 _SetCallNumber(UInt8 callIndex,UInt8* phNum);


CCallInfo_t* _Get_CallTableCtx(UInt8  callIndex);
Boolean		 _GetCallingInfo(UInt8 callIndex, CallingInfo_t* callingInfoPtr);
Boolean		 _SetCallInfoPresent(UInt8 callIndex, UInt8 inPresent);
Boolean		 _SetCallName(	UInt8  callIndex,
							UInt8  dataCodeScheme,
							UInt8  lengthInChar,
							UInt8  nameSize,
							UInt8* namePtr);
Boolean		 _GetCallPresent(UInt8 inCallIndex, PresentationInd_t* inPresentPtr);


UInt8		 _GetTypeAdd(UInt8 callIndex);
//Boolean		 _SetCallClientID(UInt8 callIndex,UInt8 clientID);
UInt8		 _GetCallClientID(UInt8 callIndex);
void		 _GetCallClientInfo(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);
UInt8		 _CallTableIdx(UInt8 callIndex);

void		 _InitFaxCall(void);


void Post_SpeechCall(ClientInfo_t* inClientInfoPtr,
					 CCParmSend_t* ccParamSendPtr);

void _UpdateVoiceCallStatus(CallDesc_t*		inCcCtxPtr,
							CallStatus_t	status);

void _UpdateDataCallStatus(	CallDesc_t*		inCcCtxPtr,
							CallStatus_t	status);

void _UpdateDataCallMoCiAlloc(	CallIndex_t	index, 
								CallType_t	call_type);

void  _ComposeDataCallParam(CallConfig_t*			cfg,
							TelephoneNumber_t*		phoneNumberPtr,
							MNATDSParmSetupReq_t*	mnCallParam);

void  _ComposeDataCallRespParam(CallConfig_t*			cfg,
								MNATDSParmSetupResp_t* mnDataCallResp);

Boolean	_SetCallClientInfo(	ClientInfo_t*	inClientInfoPtr,
							UInt8			callIndex);

#endif

