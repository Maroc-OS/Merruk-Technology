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
#include "bcm_kpdp_cmd_handler.h"
#include "bcm_kpdp_ioctl.h"
#include "bcm_kpdp_capi2_handler.h"
//#include "bcm_cp_cmd_handler.h"

static UInt8  g_PdpClientID = 0;
static UInt32 g_PdpCAPI2TID = 0;

KPDP_CmdQueue_t gKpdpCmdQueue;
KPDP_RespWq_t gKpdpRespWq;
KPDP_NotifyWq_t gKpdpNotifyWq;
KPDP_CmdList_t gKpdpCmdList;
extern struct completion gKpdpCmdThreadExited;

extern KPDP_ResultQueue_t gKpdpResultQueue;
extern KPDP_Param_t gKpdpParam;

extern int g_kpdp_initialised;

// wake lock
#ifdef CONFIG_HAS_WAKELOCK
extern struct wake_lock kpdp_rsp_wake_lock;
extern struct wake_lock kpdp_notify_wake_lock;
#endif

kpdp_capi2_handler_fn_t g_kpdp_capi2_handler_array[]=
{
     {0, NULL}, //none
     {PDP_REQUEST_SETUP_DATA_CALL, KPDP_SetupPdpHandler, 0},
     {PDP_REQUEST_DEACTIVATE_DATA_CALL, KPDP_DeactivatePdpHandler, 0}
};

//******************************************************************************
/**
*  Function to get the new TID
*
*  @param    None
*
*  @return   Valid TID number between BCM_TID_INIT and BCM_TID_MAX.
*
*  @note
*  Get a new TID number for the CAPI2 funciton in this handler
*  
*******************************************************************************/
UInt32 KPDPGetNewTID(void)
{
    if(g_PdpCAPI2TID > BCM_TID_MAX) 
    {
        g_PdpCAPI2TID = BCM_TID_INIT;
    }
    g_PdpCAPI2TID++;
    KPDP_DEBUG(DBG_TRACE, "g_PdpCAPI2TID:%lu\n", g_PdpCAPI2TID);

    return g_PdpCAPI2TID;
}

//******************************************************************************
/**
*  Function to get the TID
*
*  @param    None
*
*  @return   Valid TID number between BCM_TID_INIT and BCM_TID_MAX.
*
*  @note
*  Get the TID number for the CAPI2 funciton in this handler
*  
*******************************************************************************/
UInt32 KPDPGetTID(void)
{
    return g_PdpCAPI2TID;
}

 //******************************************************************************
 /**
 *  Function to set the Client ID
 *
 *  @param    ClientID (in) Get client id from CAPI2 client
 *
 *  @return   None
 *
 *  @note
 *  Store the Client ID that from CAPI2
 *
 *******************************************************************************/
void KPDPSetClientID(UInt8 ClientID)
{
    g_PdpClientID = ClientID;
}

//******************************************************************************
/**
*  Function to get the Client ID
*
*  @param    None
*
*  @return   Vaid the Client ID
*
*  @note
*  Get the Client ID
*
*******************************************************************************/
UInt32 KPDPGetClientID(void)
{
    return g_PdpClientID;
}


//******************************************************************************
/**
*  Function to Initialition all parameter
*
*  @param    data (in) Does not to use
*
*  @return   0 for success 1 for failure
*
*  @note
*  Initialition all parameter.
*  
*******************************************************************************/
void KPDP_InitHandler(void)
{
    KPDP_DEBUG(DBG_TRACE, "Enter!\n");
    /* Init head list */
    INIT_LIST_HEAD(&gKpdpCmdQueue.list);
    INIT_LIST_HEAD(&gKpdpRespWq.capi2_head.list);
    INIT_LIST_HEAD(&gKpdpNotifyWq.capi2_head.list);
    INIT_LIST_HEAD(&gKpdpCmdList.list);

    /* Init work queue */
    INIT_WORK(&gKpdpCmdQueue.commandq, KPDP_CommandThread);
    gKpdpCmdQueue.cmd_wq = create_workqueue("cmd_wq");

    INIT_WORK(&gKpdpRespWq.responseq, KPDP_ResponseHandler);
    gKpdpRespWq.rsp_wq = create_workqueue("rsp_wq");
    INIT_WORK(&gKpdpNotifyWq.notifyq, KPDP_NotifyHandler);
    gKpdpNotifyWq.notify_wq = create_workqueue("notify_wq");
    
    /* Init lock section */
    mutex_init(&gKpdpCmdList.mutex);
    mutex_init(&gKpdpCmdQueue.mutex);
    spin_lock_init(&gKpdpRespWq.lock);
    spin_lock_init(&gKpdpNotifyWq.lock);

    g_kpdp_initialised = 1;

} // KPDP_InitHandler

//******************************************************************************
/**
*  Function to send the response data
*
*  @param    listentry (in)
*
*  @return   None
*
*  @note
*  Send response data in result queue.
*  
*******************************************************************************/
void KPDP_SendResponse(KPDP_CmdList_t *listentry)
{
    UInt32  flags;
    KPDP_ResultQueue_t *entry = kmalloc(sizeof(KPDP_ResultQueue_t), GFP_KERNEL);

    entry->result_info.CmdID = listentry->pdp_cmd->CmdID;
    entry->result_info.datalen = listentry->rsp_len;
    entry->result_info.result = listentry->result; // Assign the error result

    KPDP_DEBUG(DBG_INFO, "entry result:%d CmdID:%ld datalen:%d\n", entry->result_info.result, entry->result_info.CmdID, entry->result_info.datalen);

    if (0 == entry->result_info.datalen)
    {
        entry->result_info.data = NULL;
    }
    else
    {
        entry->result_info.data = kmalloc(entry->result_info.datalen, GFP_KERNEL);
        memcpy(entry->result_info.data, listentry->bcm_pdp_rsp, entry->result_info.datalen);
    }
    spin_lock_irqsave(&(gKpdpParam.recv_lock), flags);
    list_add_tail(&entry->list, &(gKpdpResultQueue.list));
    spin_unlock_irqrestore(&(gKpdpParam.recv_lock), flags);
    wake_up_interruptible(&gKpdpParam.read_wait);
}

//******************************************************************************
/**
*  Function to Send notify data
*
*  @param    CmdID (in)
*  @param    rsp_data (in)
*  @param    rsp_len (in)
*
*  @return   None
*
*  @note
*  Send notify data in result queue
*  
*******************************************************************************/
void KPDP_SendNotify(int CmdID, void *rsp_data, UInt32 rsp_len)
{
    UInt32  flags;
    KPDP_ResultQueue_t *entry = kmalloc(sizeof(KPDP_ResultQueue_t), GFP_KERNEL);

    entry->result_info.CmdID = CmdID;
    entry->result_info.datalen = rsp_len;
    KPDP_DEBUG(DBG_INFO, " CmdID:%lu\n", entry->result_info.CmdID);
    KPDP_DEBUG(DBG_INFO, "notify_len:%ld CmdID:%d\n", rsp_len, CmdID);
    if (0 == entry->result_info.datalen)
    {
        entry->result_info.data = NULL;
    }
    else
    {
        entry->result_info.data = kmalloc(entry->result_info.datalen, GFP_KERNEL);
        memcpy(entry->result_info.data, rsp_data, entry->result_info.datalen);
    }
    spin_lock_irqsave(&(gKpdpParam.recv_lock), flags);
    list_add_tail(&entry->list, &(gKpdpResultQueue.list));
    spin_unlock_irqrestore(&(gKpdpParam.recv_lock), flags);
    wake_up_interruptible(&gKpdpParam.read_wait);
}

//******************************************************************************
/**
* Function Name: KPDP_CommandThread
*
* Description  : Worker thread to receive data from command queue.
*
* PARAMETERS   : void
*
* RETURN       : void
* Notes:
*
*******************************************************************************/
void KPDP_CommandThread(struct work_struct *data)
{
    struct list_head *listptr, *pos;
    KPDP_CmdQueue_t *entry = NULL;
    KPDP_CmdList_t *cmd_list = NULL;
    bool found = FALSE;
    int i = 0;

    KPDP_DEBUG(DBG_TRACE2, "Enter...KPDP_CommandThread!\n");
    mutex_lock(&gKpdpCmdQueue.mutex);
    list_for_each_safe(listptr, pos, &gKpdpCmdQueue.list)
    {
        entry = list_entry(listptr, KPDP_CmdQueue_t, list);
        KPDP_DEBUG(DBG_TRACE, "entry cmd:%ld list:%p next:%p prev:%p\n", entry->cmd, &entry->list, entry->list.next, entry->list.prev);
        mutex_unlock(&gKpdpCmdQueue.mutex);

        // add cmd list
        cmd_list = kmalloc(sizeof(KPDP_CmdList_t), GFP_KERNEL);
        memset(cmd_list, 0, sizeof(KPDP_CmdList_t));
        cmd_list->cmd = entry->cmd;
        cmd_list->pdp_cmd = entry->pdp_cmd;
        cmd_list->bcm_pdp_rsp = NULL;
        cmd_list->rsp_len = 0;
        cmd_list->cmdContext = NULL;
        cmd_list->result = RESULT_ERROR; 
        cmd_list->handler_state = BCM_SendCAPI2Cmd;
        KPDP_DEBUG(DBG_TRACE2, "cmd list cmd:%ld list:%p next:%p prev:%p\n", cmd_list->cmd, &cmd_list->list, cmd_list->list.next, cmd_list->list.prev);
        mutex_lock(&gKpdpCmdList.mutex);
        list_add_tail(&cmd_list->list, &gKpdpCmdList.list);
        mutex_unlock(&gKpdpCmdList.mutex);

        // search function
        for (i = 0; i < ARRAY_SIZE(g_kpdp_capi2_handler_array); ++i)
        {
            //KPDP_DEBUG(DBG_TRACE,"Current command is0x%lx",g_kpdp_capi2_handler_array[i].cmd);
            //KPDP_SendResponse(cmd_list);
            if (g_kpdp_capi2_handler_array[i].cmd == entry->pdp_cmd->CmdID)
            {
                KPDP_DEBUG(DBG_TRACE, "match command success 0x%lx, msgid %d\n", entry->cmd, i);
                if(g_kpdp_capi2_handler_array[i].capi2_handler != NULL)
                {
                    if (g_kpdp_capi2_handler_array[i].contextSize != 0)
                    {
                        KPDP_DEBUG(DBG_INFO, "contextSize %d\n", g_kpdp_capi2_handler_array[i].contextSize);
                        cmd_list->cmdContext = kmalloc(g_kpdp_capi2_handler_array[i].contextSize, GFP_KERNEL);
                        memset(cmd_list->cmdContext, 0, g_kpdp_capi2_handler_array[i].contextSize);
                    }
                    found = TRUE;
                    break;
                }
                else
                {
                    KPDP_DEBUG(DBG_ERROR, "The function does not implement yet or not in Capi2 handler array ...!\n");
                }
            }
        }
        //KPDP_DEBUG(DBG_ERROR, "Iterations are over...!\n");
        //KPDP_SendResponse(cmd_list);
        if(TRUE == found)
        {
            mutex_lock(&gKpdpCmdList.mutex);
            KPDP_DEBUG(DBG_INFO, "match command success 0x%lx, msgid %d\n", entry->cmd, i);
            cmd_list->capi2_handler = g_kpdp_capi2_handler_array[i].capi2_handler;
            g_kpdp_capi2_handler_array[i].capi2_handler((void *)cmd_list, NULL);
            if(BCM_ErrorCAPI2Cmd == cmd_list->handler_state || BCM_FinishCAPI2Cmd == cmd_list->handler_state)
            {
                KPDP_DEBUG(DBG_TRACE, "handler_state:%ld CmdID:%ld\n", cmd_list->handler_state, cmd_list->pdp_cmd->CmdID);
                KPDP_SendResponse(cmd_list);
                list_del(&cmd_list->list); // delete command list
                if (cmd_list->pdp_cmd->data != NULL && cmd_list->pdp_cmd->datalen != 0)
                {
                    kfree(cmd_list->pdp_cmd->data);
                }
                kfree(cmd_list->pdp_cmd);

                if (cmd_list->bcm_pdp_rsp != NULL && cmd_list->rsp_len != 0)
                {
                    kfree(cmd_list->bcm_pdp_rsp);
                }
                if (cmd_list->cmdContext != NULL)
                {
                    kfree(cmd_list->cmdContext);
                    cmd_list->cmdContext = NULL;
                }
                kfree(cmd_list);
                cmd_list = NULL;
            }
            else
            {
                cmd_list->tid = KPDPGetTID(); // store next TID in command list
                KPDP_DEBUG(DBG_INFO, "other handler state:0x%lx tid:%ld\n", cmd_list->handler_state, cmd_list->tid);
            }
            mutex_unlock(&gKpdpCmdList.mutex);
        }
        else
        {
            KPDP_DEBUG(DBG_ERROR, "Command not found...!0x%lx CmdID:%ld\n", cmd_list->cmd, entry->pdp_cmd->CmdID);
            cmd_list->result = RESULT_ERROR;
            KPDP_SendResponse(cmd_list);
            mutex_lock(&gKpdpCmdList.mutex);
            list_del(&cmd_list->list); // delete command list
            if (cmd_list->pdp_cmd->data != NULL && cmd_list->pdp_cmd->datalen != 0)
            {
                kfree(cmd_list->pdp_cmd->data);
            }
            kfree(cmd_list->pdp_cmd);
            
            if (cmd_list->bcm_pdp_rsp != NULL && cmd_list->rsp_len != 0)
            {
                kfree(cmd_list->bcm_pdp_rsp);
            }
            kfree(cmd_list);
            cmd_list = NULL;
            mutex_unlock(&gKpdpCmdList.mutex);
        }
        KPDP_DEBUG(DBG_INFO, "list_empty(gKpdpCmdQueue.list):%d...!\n", list_empty(&gKpdpCmdQueue.list));
        mutex_lock(&gKpdpCmdQueue.mutex);
        list_del(listptr);
        kfree(entry);
    }
    mutex_unlock(&gKpdpCmdQueue.mutex);
} // KPDP_cmd_thread

//******************************************************************************
/**
* Function Name: KPDP_ResponseHandler
*
* Description  : When put the CAPI2 response in response queue, 
*                the handler will wake up and process the response data
*
* PARAMETERS   : void
*
* RETURN       : void
* Notes:
*
*******************************************************************************/
void KPDP_ResponseHandler(struct work_struct *data)
{
    struct list_head *ptr, *pos;
    struct list_head *listptr, *listpos;
    Kpdp_CAPI2Info_t *entry = NULL;
    KPDP_CmdList_t *listentry = NULL;
    bool found = FALSE;
    UInt32 irql;

    KPDP_DEBUG(DBG_TRACE2, "head tid:%ld list:%p next:%p prev:%p\n", gKpdpRespWq.capi2_head.tid, &gKpdpRespWq.capi2_head.list, gKpdpRespWq.capi2_head.list.next, gKpdpRespWq.capi2_head.list.prev);
    spin_lock_irqsave(&gKpdpRespWq.lock, irql);
    list_for_each_safe(ptr, pos, &gKpdpRespWq.capi2_head.list)
    {
        entry = list_entry(ptr, Kpdp_CAPI2Info_t, list);
        spin_unlock_irqrestore(&gKpdpRespWq.lock, irql);
        KPDP_DEBUG(DBG_INFO, "entry tid:%ld msgType:%d list:%p next:%p prev:%p\n", entry->tid, entry->msgType, &entry->list, entry->list.next, entry->list.prev);

        // search in command list
        mutex_lock(&gKpdpCmdList.mutex);
        list_for_each_safe(listptr, listpos, &gKpdpCmdList.list)
        {
            listentry = list_entry(listptr, KPDP_CmdList_t, list);
            mutex_unlock(&gKpdpCmdList.mutex);

            KPDP_DEBUG(DBG_INFO, "listentry->tid:%ld entry->tid:%ld listentry->handler_state:%lX\n", listentry->tid, entry->tid,listentry->handler_state);
            mutex_lock(&gKpdpCmdList.mutex);
            if(listentry->tid == entry->tid)
            {
                found = TRUE;
                break;
            }
            else
            {
                found = FALSE;
            }
        }
        mutex_unlock(&gKpdpCmdList.mutex);

        if(TRUE == found)
        {
            KPDP_DEBUG(DBG_INFO, "Run response CAPI2 handler\n");
            mutex_lock(&gKpdpCmdList.mutex);
            listentry->capi2_handler((void *)listentry, entry);
            if(BCM_FinishCAPI2Cmd == listentry->handler_state || BCM_ErrorCAPI2Cmd == listentry->handler_state)
            {
                KPDP_DEBUG(DBG_INFO, "handler_state:0x%lx  CmdID:%ld\n", listentry->handler_state, listentry->pdp_cmd->CmdID);

		KPDP_SendResponse(listentry);
                list_del(listptr); // delete command list
                if(listentry->pdp_cmd->data != NULL && listentry->pdp_cmd->datalen != 0)
                {
                    kfree(listentry->pdp_cmd->data);
                }
                kfree(listentry->pdp_cmd);

                if(listentry->bcm_pdp_rsp != NULL && listentry->rsp_len != 0)
                {
                    kfree(listentry->bcm_pdp_rsp);
                }
                if (listentry->cmdContext != NULL)
                {
                    kfree(listentry->cmdContext);
                    listentry->cmdContext = NULL;
                }
                kfree(listentry);
                listentry = NULL;
            }
            else
            {
                if (!(entry->msgType == MSG_SYS_ENABLE_CELL_INFO_RSP || // Skip to update the TID for enable cell information
                     entry->msgType == MSG_PBK_ENTRY_DATA_RSP)) // Skip to update the TID for read phonebook
                {
                    listentry->tid = KPDPGetTID(); // store next TID in command list
                }
                KPDP_DEBUG(DBG_INFO, "other handler state:0x%lx tid:%ld\n", listentry->handler_state, listentry->tid);
            }
            found = FALSE;
            mutex_unlock(&gKpdpCmdList.mutex);
        }
        else
        {
	    KPDP_DEBUG(DBG_ERROR, "TID doesn't match...!\nentry->tid:%ld entry->client=%d msgType:%ld\n",entry->tid, entry->clientID, (UInt32)entry->msgType);
        }
        spin_lock_irqsave(&gKpdpRespWq.lock, irql);
        RPC_SYSFreeResultDataBuffer(entry->dataBufHandle);

        list_del(ptr); // delete response capi2
        kfree(entry);
        entry = NULL;
    }
    spin_unlock_irqrestore(&gKpdpRespWq.lock, irql);
#ifdef CONFIG_HAS_WAKELOCK
    wake_unlock(&kpdp_rsp_wake_lock);
#endif
    return;
} // KPDP_ResponseHandler

/****************************************************************************
*
*  KPDP_NotifyHandler
*
*  Worker thread to receive data from notify queue.
*
****************************************************************************/
void KPDP_NotifyHandler(struct work_struct *data)
{
    struct list_head *listptr, *pos;
    Kpdp_CAPI2Info_t *entry = NULL;
    UInt32 irql;

    //KPDP_DEBUG(DBG_INFO, "head tid:%ld list:%p next:%p prev:%p\n", gKpdpNotifyWq.capi2_head.tid, &gKpdpNotifyWq.capi2_head.list, gKpdpNotifyWq.capi2_head.list.next, gKpdpNotifyWq.capi2_head.list.prev);

    spin_lock_irqsave(&gKpdpNotifyWq.lock, irql);
    list_for_each_safe(listptr, pos, &gKpdpNotifyWq.capi2_head.list)
    {
        entry = list_entry(listptr, Kpdp_CAPI2Info_t, list);
        spin_unlock_irqrestore(&gKpdpNotifyWq.lock, irql);

        KPDP_ProcessNotification(entry);

        //KPDP_DEBUG(DBG_INFO, "entry tid:%ld msgType:%d list:%p next:%p prev:%p list_empty:%d\n", entry->tid, entry->msgType, &entry->list, entry->list.next, entry->list.prev, list_empty(&gKpdpNotifyWq.capi2_head.list));
        spin_lock_irqsave(&gKpdpNotifyWq.lock, irql);
        RPC_SYSFreeResultDataBuffer(entry->dataBufHandle);

        list_del(listptr);
        kfree(entry);
        entry = NULL;
    }
    spin_unlock_irqrestore(&gKpdpNotifyWq.lock, irql);
#ifdef CONFIG_HAS_WAKELOCK
    wake_unlock(&kpdp_notify_wake_lock);
#endif
} // KPDP_NotifyHandler

//******************************************************************************
//
// Function Name: KPDP_CmdQueueWork
//
// Description:   Trigger Command Queue work
//
// Notes:
//
//******************************************************************************
void KPDP_CmdQueueWork(void)
{
    queue_work(gKpdpCmdQueue.cmd_wq, &gKpdpCmdQueue.commandq);
}


//******************************************************************************
//
// Function Name:      KpdpRawDataPrintfun
//
// Description:   Print Raw data in hex format. This is just for debug.
//                
//******************************************************************************
void KpdpRawDataPrintfun(UInt8* rawdata, UInt16 datalen, char* showstr)
{
    UInt8  *pdata = rawdata;
    UInt8  array[32];
    
    KPDP_DEBUG(DBG_INFO,"%s: datalen:%d\n", showstr, datalen);
    memset(array, 0, 32);
    
    while (datalen > 0)
    {
        if (datalen >= 32)
        {
            KPDP_DEBUG(DBG_INFO,"%s: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",showstr,pdata[0],pdata[1],pdata[2],pdata[3],
            pdata[4],pdata[5],pdata[6],pdata[7],pdata[8],pdata[9],pdata[10],pdata[11],pdata[12],pdata[13],pdata[14],pdata[15],pdata[16],pdata[17],pdata[18],pdata[19],pdata[20],pdata[21],pdata[22],pdata[23],pdata[24],pdata[25],pdata[26],
            pdata[27],pdata[28],pdata[29],pdata[30],pdata[31]);
            
            pdata += 32;
            datalen -= 32;
        }
        else
        {
            memcpy(array, pdata, datalen);
            KPDP_DEBUG(DBG_INFO,"%s: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",showstr,array[0],array[1],array[2],array[3],
            array[4],array[5],array[6],array[7],array[8],array[9],array[10],array[11],array[12],array[13],array[14],array[15],array[16],array[17],array[18],array[19],array[20],array[21],array[22],array[23],array[24],array[25],array[26],
            array[27],array[28],array[29],array[30],array[31]);
            
            datalen = 0;
        }
    }
}

