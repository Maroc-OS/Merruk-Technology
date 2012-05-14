/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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

#ifndef _BCM_KRIL_CMD_HANDLER_H
#define _BCM_KRIL_CMD_HANDLER_H

#include "bcm_kril_common.h"
#include "bcm_kril.h"

// Common capi2 handler state
#define BCM_SendCAPI2Cmd       0xFFFC
#define BCM_RESPCAPI2Cmd       0xFFFD
#define BCM_FinishCAPI2Cmd     0xFFFE
#define BCM_ErrorCAPI2Cmd      0xFFFF

/*+20110418 BCOM PATCH FOR DebugScreen*/
#define BCM_MeasureReport 0xFFFA
/*-20110418 BCOM PATCH FOR DebugScreen*/


#define BCM_TID_INIT   0X00
#define BCM_TID_MAX    0XFFFFF
#define BCM_TID_SEND   0XFFFF
#define BCM_TID_RECV   0x0000
#define BCM_TID_NOTIFY 0X00

//---------------------------------------------------------
// RAT Defines
//---------------------------------------------------------
#define RAT_NOT_AVAILABLE  0
#define RAT_GSM            1
#define RAT_UMTS           2

#define RX_SIGNAL_INFO_UNKNOWN 0xFF  

#define INTERNATIONAL_CODE  '+'     ///< '+' at the beginning
#define TOA_International   145
#define TOA_Unknown         129

// for SIM
#define SIM_IMAGE_INSTANCE_SIZE         9
#define SIM_IMAGE_INSTANCE_COUNT_SIZE   1

// for SMS
#define SMS_FULL      0xFF

// For STK
#define STK_REFRESH              (0x01)
#define STK_MORETIME             (0x02)
#define STK_POLLINTERVAL         (0x03)
#define STK_POLLINGOFF           (0x04)
#define STK_EVENTLIST            (0x05)
#define STK_SETUPCALL            (0x10)
#define STK_SENDSS               (0x11)
#define STK_SENDUSSD             (0x12)
#define STK_SENDSMS              (0x13)
#define STK_SENDDTMF             (0x14)
#define STK_LAUNCHBROWSER        (0x15)
#define STK_PLAYTONE             (0x20)
#define STK_DISPLAYTEXT          (0x21)
#define STK_GETINKEY             (0x22)
#define STK_GETINPUT             (0x23)
#define STK_SELECTITEM           (0x24)
#define STK_SETUPMENU            (0x25)
#define STK_LOCALINFO            (0x26)
#define STK_SETUPIDLEMODETEXT    (0x28)
#define STK_RUNATCOMMAND         (0x34)
#define STK_LANGUAGENOTIFICATION (0x35)
#define STK_OPENCHANNEL          (0x40)
#define STK_CLOSECHANNEL         (0x41)
#define STK_RECEIVEDATA          (0x42)
#define STK_SENDDATA             (0x43)
#define STK_CHANNELSTATUS        (0x44)

/* BCD coded IMEI length: 7 bytes for 14 digits, but first nibble and last nibble are not used (set to 0) 
 * in our system parameter convention. So total 8 bytes. */
#define BCD_IMEI_LEN  8

typedef struct
{
    struct list_head     list;
    KRIL_Response_t     result_info;
}KRIL_ResultQueue_t;


typedef struct // for command queue
{
    struct list_head list;
    struct mutex mutex;
    UInt32 cmd;
    struct work_struct commandq;
    struct workqueue_struct *cmd_wq;
    KRIL_Command_t *ril_cmd;
} KRIL_CmdQueue_t;


typedef struct  // for capi2 info
{
    struct list_head list;
    UInt32 tid;
    UInt8 clientID;
    MsgType_t msgType;
    UInt32 DialogId;
    Result_t result;
    void *dataBuf;
    UInt32 dataLength;
    ResultDataBufHandle_t dataBufHandle;
} Kril_CAPI2Info_t;


typedef struct // for response queue
{
    spinlock_t lock;
    struct work_struct responseq;
    struct workqueue_struct *rsp_wq;
    Kril_CAPI2Info_t capi2_head;
} KRIL_RespWq_t;


typedef struct // for notify queue
{
    spinlock_t lock;
    struct work_struct notifyq;
    struct workqueue_struct *notify_wq;
    Kril_CAPI2Info_t capi2_head;
} KRIL_NotifyWq_t;


typedef struct kril_cmd // Command list
{
    struct list_head list;
    struct mutex mutex;
    UInt32 cmd;
    UInt32 tid;
    UInt32 handler_state;
    BRIL_Errno result;      //Response result
    KRIL_Command_t *ril_cmd;
    void *bcm_ril_rsp;
    UInt32 rsp_len;
    void (*capi2_handler)(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
    void* cmdContext; // may be used by KRIL command handlers to store context information across CAPI2 calls to CP
} KRIL_CmdList_t;


typedef struct kril_capi2_handler_fn_t
{
    int    cmd;
    void   (*capi2_handler)(void *ril_cmd, Kril_CAPI2Info_t *ril_rsp);
    int    contextSize;
} kril_capi2_handler_fn_t;


typedef struct
{
    dev_t devnum;
    int kril_state;
    struct semaphore kril_sem;
    struct cdev cdev;
    void __iomem *apcp_shmem;
    struct timer_list timeout;
    wait_queue_head_t read_wait;
    int timeout_done;
    spinlock_t           recv_lock;
    struct file          *file;
    wait_queue_head_t    recv_wait;
    struct mutex         recv_mutex;
}KRIL_Param_t;


// For cmd context structure
/**
Store the Call index
**/
typedef struct
{
    UInt8 ActiveIndex;
    Boolean activeMPTY;
    UInt8 WaitIndex;
    UInt8 HeldIndex;
    Boolean heldMPTY;
    UInt8 MptyIndex;
} KrilCallIndex_t;

/**
Store the DTMF information
**/
typedef struct
{
    UInt8 DTMFCallIndex;
    CCallState_t inCcCallState;
} KrilDTMFInfo_t;

/**
Store the call retry information
**/
typedef struct
{
    UInt8 total_retry_count;
    UInt8 current_retry_count;
    unsigned long retry_duration;
	VoiceCallParam_t call_param;
    char address[BCM_MAX_DIGITS+1];
} KrilCallRetryInfo_t;


/**
Selection path information for a SIM/USIM file
**/
typedef struct
{
    UInt8        path_len;
    const UInt16 *select_path;
} KrilSimDedFilePath_t;


void KRIL_InitHandler(void);
void KRIL_CommandThread(struct work_struct *data);
void KRIL_ResponseHandler(struct work_struct *data);
void KRIL_NotifyHandler(struct work_struct *data);

void ProcessNotification(Kril_CAPI2Info_t *entry);

void KRIL_Capi2HandleRespCbk(UInt32 tid, UInt8 clientID, MsgType_t msgType, UInt32 DialogId, Result_t result, void *dataBuf, UInt32 dataLength,ResultDataBufHandle_t dataBufHandle);
void KRIL_Capi2HandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData);
void KRIL_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel);

void SetClientID(UInt8 ClientID);
UInt32 GetClientID(void);
UInt32 GetNewTID(void);
UInt32 GetTID(void);

Boolean IsNeedToWait(unsigned long CmdID);
void SetBasicLoggingEnable(Boolean Enable);
Boolean IsBasicCapi2LoggingEnable(void);

// for call
void KRIL_SetIncomingCallIndex(UInt8 IncomingCallIndex);
UInt8 KRIL_GetIncomingCallIndex(void);
void KRIL_SetWaitingCallIndex(UInt8 IncomingCallIndex);
UInt8 KRIL_GetWaitingCallIndex(void);
void KRIL_SetCallType(int index, CCallType_t theCallType);
CCallType_t KRIL_GetCallType(int index);
void KRIL_SetCallState(int index, BRIL_CallState theCallState);
BRIL_CallState KRIL_GetCallState(int index);
void KRIL_ClearCallNumPresent(void);
void KRIL_SetCallNumPresent(int index, PresentationInd_t present, UInt8 c_num);
PresentationInd_t KRIL_GetCallNumPresent(int index);
void KRIL_SetInHoldCallHandler(Boolean CallHandler);
Boolean KRIL_GetInHoldCallHandler(void);
void KRIL_SetIsNeedMakeCall(Boolean MakeCall);
Boolean KRIL_GetIsNeedMakeCall(void);
void KRIL_SetHungupForegroundResumeBackgroundEndMPTY(UInt32 tid);
UInt32 KRIL_GetHungupForegroundResumeBackgroundEndMPTY(void);
void KRIL_SetLastCallFailCause(BRIL_LastCallFailCause inCause);
BRIL_LastCallFailCause KRIL_GetLastCallFailCause(void);
BRIL_LastCallFailCause KRIL_MNCauseToRilError(Cause_t inMNCause);

// for Network
void KRIL_SetInSetPrefNetworkTypeHandler( Boolean inHandler );
Boolean KRIL_GetInSetPrefNetworkTypeHandler( );
void KRIL_SetIsNeedSetPreferNetworkType(Boolean state);
Boolean KRIL_GetIsNeedSetPreferNetworkType();

Boolean KRIL_SetPreferredNetworkType(int PreferredNetworkType);
int KRIL_GetPreferredNetworkType(void);
Boolean KRIL_SetBandMode(int BandMode);
int KRIL_GetBandMode(void);
void KRIL_SetLocationUpdateStatus(int LocationUpdateStatus);
int KRIL_GetLocationUpdateStatus(void);
void KRIL_SetRestrictedState(int RestrictedState);
int KRIL_GetRestrictedState(void);

// for SS
void KRIL_SetCLIPValue(int LastCLIP);
int KRIL_GetCLIPValue(void);
void KRIL_SetInSsQueryHandler(Boolean inHandler);
Boolean KRIL_GetInSsQueryHandler(void);
void KRIL_SetServiceClassValue(int ss_class);
int KRIL_GetServiceClassValue(void);

// for SIM
void KRIL_SetSimAppType(SIM_APPL_TYPE_t simAppType);
SIM_APPL_TYPE_t KRIL_GetSimAppType(void);

// for SMS
Boolean QuerySMSinSIMHandle(KRIL_CmdList_t *listentry, Kril_CAPI2Info_t *entry);
void KRIL_SetSmsMti(SmsMti_t SmsMti);
SmsMti_t KRIL_GetSmsMti(void);
void SetIsRevClass2SMS(Boolean value);
Boolean GetIsRevClass2SMS(void);
void KRIL_SetInSendSMSHandler(Boolean SMSHandler);
Boolean KRIL_GetInSendSMSHandler(void);
void KRIL_SetMESMSAvailable(Boolean IsSMSMEAvailable);
Boolean KRIL_GetMESMSAvailable(void);
void KRIL_SetTotalSMSInSIM(UInt8 TotalSMSInSIM);
UInt8 KRIL_GetTotalSMSInSIM(void);
UInt8 CheckFreeSMSIndex(void);
void SetSMSMesgStatus(UInt8 Index, SIMSMSMesgStatus_t status);
SIMSMSMesgStatus_t GetSMSMesgStatus(UInt8 Index);
void KRIL_IncrementSendSMSNumber(void);
void KRIL_DecrementSendSMSNumber(void);
Int8 KRIL_GetSendSMSNumber(void);
void KRIL_SetInUpdateSMSInSIMHandler(Boolean SMSHandler);
Boolean KRIL_GetInUpdateSMSInSIMHandler(void);
void KRIL_IncrementUpdateSMSNumber(void);
void KRIL_DecrementUpdateSMSNumber(void);
Int8 KRIL_GetUpdateSMSNumber(void);

void KRIL_SetSMSToSIMTID(UInt32 Tid);
UInt32 KRIL_GetSMSToSIMTID(void);
void KRIL_SetServingCellTID(UInt32 tid);
UInt32 KRIL_GetServingCellTID(void);
void KRIL_SetInNeighborCellHandler( Boolean inHandler );
Boolean KRIL_GetInNeighborCellHandler();

void KRIL_SetInSetupPDPHandler( Boolean inHandler );
Boolean KRIL_GetInSetupPDPHandler();

void KRIL_SetNetworkSelectTID(UInt32 tid);
UInt32 KRIL_GetNetworkSelectTID(void);
void KRIL_SetInNetworkSelectHandler( Boolean inHandler );
Boolean KRIL_GetInNetworkSelectHandler();
void KRIL_SetInSetupPDPHandler( Boolean inHandler );
Boolean KRIL_GetInSetupPDPHandler();

/*+20110418 BCOM PATCH FOR DebugScreen*/
void KRIL_SetMeasureReportTID(UInt32 tid);
UInt32 KRIL_GetMeasureReportTID(void);
void KRIL_SetInMeasureReportHandler( Boolean inHandler );
Boolean KRIL_GetInMeasureReportHandler();
/*-20110418 BCOM PATCH FOR DebugScreen*/

void KRIL_SetHandling2GOnlyRequest( Boolean inHandler );
Boolean KRIL_GetHandling2GOnlyRequest( );

void HexDataToHexStr(char *HexString, const UInt8 *HexData, UInt16 length);
void RawDataPrintfun(UInt8* rawdata, UInt16 datalen, char* showstr);

void KRIL_CmdQueueWork(void);

void KRIL_SendNotify(int CmdID, void *rsp_data, UInt32 rsp_len);

// for error cause transform
BRIL_Errno RILErrorResult(Result_t err);
BRIL_Errno RILErrorSIMResult(SIMAccess_t err);
#endif //_BCM_KRIL_CMD_HANDLER_H
