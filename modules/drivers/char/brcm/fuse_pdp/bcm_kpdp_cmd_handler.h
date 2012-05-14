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

#ifndef _BCM_KPDP_CMD_HANDLER_H
#define _BCM_KPDP_CMD_HANDLER_H

#include "bcm_kpdp_common.h"
#include "bcm_kpdp.h"

// Common capi2 handler state
#define BCM_SendCAPI2Cmd       0xFFFC
#define BCM_RESPCAPI2Cmd       0xFFFD
#define BCM_FinishCAPI2Cmd     0xFFFE
#define BCM_ErrorCAPI2Cmd      0xFFFF

//pdp handler state
#define BCM_PDP_Verify 0x002

#define BCM_TID_INIT   0X00
#define BCM_TID_MAX    0XFFFFF

typedef struct
{
    struct list_head     list;
    KPDP_Response_t     result_info;
}KPDP_ResultQueue_t;


typedef struct // for command queue
{
    struct list_head list;
    struct mutex mutex;
    UInt32 cmd;
    struct work_struct commandq;
    struct workqueue_struct *cmd_wq;
    KPDP_Command_t *pdp_cmd;
} KPDP_CmdQueue_t;


typedef struct  // for capi2 info
{
    struct list_head list;
    UInt32 tid;
    UInt8 clientID;
    MsgType_t msgType;
    Result_t result;
    void *dataBuf;
    UInt32 dataLength;
    ResultDataBufHandle_t dataBufHandle;
} Kpdp_CAPI2Info_t;


typedef struct // for response queue
{
    spinlock_t lock;
    struct work_struct responseq;
    struct workqueue_struct *rsp_wq;
    Kpdp_CAPI2Info_t capi2_head;
} KPDP_RespWq_t;


typedef struct // for notify queue
{
    spinlock_t lock;
    struct work_struct notifyq;
    struct workqueue_struct *notify_wq;
    Kpdp_CAPI2Info_t capi2_head;
} KPDP_NotifyWq_t;


typedef struct kpdp_cmd // Command list
{
    struct list_head list;
    struct mutex mutex;
    UInt32 cmd;
    UInt32 tid;
    UInt32 handler_state;
    Result_t result;      //Response result, public/resultcode.h 
    KPDP_Command_t *pdp_cmd;
    void *bcm_pdp_rsp;
    UInt32 rsp_len;
    void (*capi2_handler)(void *pdp_cmd, Kpdp_CAPI2Info_t *capi2_rsp);
    void* cmdContext; // may be used by KPDP command handlers to store context information across CAPI2 calls to CP
} KPDP_CmdList_t;


typedef struct kpdp_capi2_handler_fn_t
{
    int    cmd;
    void   (*capi2_handler)(void *pdp_cmd, Kpdp_CAPI2Info_t *pdp_rsp);
    int    contextSize;
} kpdp_capi2_handler_fn_t;


typedef struct
{
    dev_t devnum;
    int kpdp_state;
    struct semaphore kpdp_sem;
    struct cdev cdev;
    void __iomem *apcp_shmem;
    struct timer_list timeout;
    wait_queue_head_t read_wait;
    int timeout_done;
    spinlock_t           recv_lock;
    struct file          *file;
    wait_queue_head_t    recv_wait;
    struct mutex         recv_mutex;
}KPDP_Param_t;


void KPDP_InitHandler(void);
void KPDP_CommandThread(struct work_struct *data);
void KPDP_ResponseHandler(struct work_struct *data);
void KPDP_NotifyHandler(struct work_struct *data);

void KPDP_SendNotify(int CmdID, void *rsp_data, UInt32 rsp_len);
void KPDP_ProcessNotification(Kpdp_CAPI2Info_t *entry);

void KPDP_Capi2HandleRespCbk(UInt32 tid, UInt8 clientID, MsgType_t msgType, Result_t result, void *dataBuf, UInt32 dataLength,ResultDataBufHandle_t dataBufHandle);
void KPDP_Capi2HandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData);
void KPDP_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel);

void KPDPSetClientID(UInt8 ClientID);
UInt32 KPDPGetClientID(void);
UInt32 KPDPGetNewTID(void);
UInt32 KPDPGetTID(void);

Boolean IsNeedToWait(unsigned long CmdID);

//protypes
void KPDP_SetupPdpHandler(void *pdp_cmd, Kpdp_CAPI2Info_t *capi2_rsp);
void KPDP_DeactivatePdpHandler(void *pdp_cmd, Kpdp_CAPI2Info_t *capi2_rsp);

#endif //_BCM_KPDP_CMD_HANDLER_H
