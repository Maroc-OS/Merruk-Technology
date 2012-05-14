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
#include "bcm_kpdp_common.h"
#include "bcm_kpdp_capi2_handler.h"
#include "bcm_kpdp_cmd_handler.h"
#include "bcm_kpdp_ioctl.h"
#include "ipcinterface.h"

extern KPDP_RespWq_t gKpdpRespWq;
extern KPDP_NotifyWq_t gKpdpNotifyWq;
// wake lock
#ifdef CONFIG_HAS_WAKELOCK
extern struct wake_lock kpdp_rsp_wake_lock;
extern struct wake_lock kpdp_notify_wake_lock;
#endif
void KPDP_Capi2HandleRespCbk(UInt32 tid, UInt8 clientID, MsgType_t msgType, Result_t result, void *dataBuf, UInt32 dataLength, ResultDataBufHandle_t dataBufHandle)
{
    Kpdp_CAPI2Info_t *capi2_rsp = NULL;

    //KPDP_DEBUG(DBG_INFO, "tid:%lu msgtype:0x%x result:%d\n", tid, msgType, result);
    capi2_rsp = kmalloc(sizeof(Kpdp_CAPI2Info_t), GFP_KERNEL);
    if(!capi2_rsp)
    {
        KPDP_DEBUG(DBG_ERROR, "Unable to allocate CAPI2 Response memory\n");
        RPC_SYSFreeResultDataBuffer(dataBufHandle);
        return;
    }

    capi2_rsp->tid = tid;
    capi2_rsp->clientID = clientID;    
    capi2_rsp->msgType = msgType;
    capi2_rsp->result = result;
    capi2_rsp->dataBuf = dataBuf;
    capi2_rsp->dataLength = dataLength;
    capi2_rsp->dataBufHandle = dataBufHandle;

    if(capi2_rsp->tid == 0) //notify
    {
        unsigned long irql;
#ifdef CONFIG_HAS_WAKELOCK
        wake_lock(&kpdp_notify_wake_lock);
#endif
        spin_lock_irqsave(&gKpdpNotifyWq.lock, irql);
        list_add_tail(&capi2_rsp->list, &gKpdpNotifyWq.capi2_head.list); 
        spin_unlock_irqrestore(&gKpdpNotifyWq.lock, irql);
        queue_work(gKpdpNotifyWq.notify_wq, &gKpdpNotifyWq.notifyq);
    }
    else //response
    {
        unsigned long irql;
#ifdef CONFIG_HAS_WAKELOCK
        wake_lock(&kpdp_rsp_wake_lock);
#endif
        spin_lock_irqsave(&gKpdpRespWq.lock, irql);
        list_add_tail(&capi2_rsp->list, &gKpdpRespWq.capi2_head.list); 
        spin_unlock_irqrestore(&gKpdpRespWq.lock, irql);
        queue_work(gKpdpRespWq.rsp_wq, &gKpdpRespWq.responseq);
    }
}

/* Ack call back */
void KPDP_Capi2HandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData)
{
    switch(ackResult)
    {
        case ACK_SUCCESS:
        {
            //capi2 request ack succeed
	        //KPDP_DEBUG(DBG_INFO, "KPDP_HandleCapi2AckCbk::AckCbk ACK_SUCCESS tid=%lu\n", tid);
        }
        break;

        case ACK_FAILED:
        {
	        KPDP_DEBUG(DBG_ERROR, "KPDP_HandleCapi2AckCbk::AckCbk ACK_FAILED\n");
            //capi2 ack fail for unknown reasons
        }
        break;

        case ACK_TRANSMIT_FAIL:
        {
	        KPDP_DEBUG(DBG_ERROR, "KPDP_HandleCapi2AckCbk::AckCbk ACK_TRANSMIT_FAIL\n");
            //capi2 ack fail due to fifo full, fifo mem full etc.
        }
        break;

        case ACK_CRITICAL_ERROR:
        {
            KPDP_DEBUG(DBG_ERROR, "KPDP_HandleCapi2AckCbk::AckCbk ACK_CRITICAL_ERROR\n");
            //capi2 ack fail due to comms processor reset ( The use case for this error is TBD )  
        }
        break;

        default:
        {
            KPDP_DEBUG(DBG_ERROR, "KPDP_HandleCapi2AckCbk::AckCbk ackResult error!\n");
        }
        break;
    }
}

void KPDP_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel)
{
    return;
}
