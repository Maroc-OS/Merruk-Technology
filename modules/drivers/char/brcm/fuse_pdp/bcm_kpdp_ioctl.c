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
#include "bcm_kpdp_main.h"

#define RESULT_NOT_EMPTY 255

extern KPDP_CmdQueue_t gKpdpCmdQueue;

extern KPDP_Param_t gKpdpParam;
extern KPDP_ResultQueue_t gKpdpResultQueue;

int g_kpdp_initialised  = 0;

/**
   @fn Int32 KPDP_GetRsp()
*/
Int32 KPDP_GetRsp(struct file *filp, UInt32 cmd, UInt32 arg)
{
    Int32 rv = 0;
    KPDP_Param_t *priv = filp->private_data;
    KPDP_Response_t rsp;
    struct list_head *entry;
    KPDP_ResultQueue_t *buf_handle = NULL;
    UInt32    flags;

    if (copy_from_user(&rsp, (KPDP_Response_t*)arg, sizeof(rsp)))
    {
        KPDP_DEBUG(DBG_INFO, "KPDP_GetRsp() error in copy_from_user() \n");
        rv = -EFAULT;
        return rv;
    }

    /* We claim a mutex because we don't want two
       users getting something from the queue at a time.
       Since we have to release the spinlock before we can
       copy the data to the user, it's possible another
       user will grab something from the queue, too.  Then
       the messages might get out of order if something
       fails and the message gets put back onto the
       queue.  This mutex prevents that problem. */
    mutex_lock(&priv->recv_mutex);

    /* Grab the message off the list. */
    spin_lock_irqsave(&(priv->recv_lock), flags);
    if (list_empty(&(gKpdpResultQueue.list)))
    {
        spin_unlock_irqrestore(&(priv->recv_lock), flags);
        rv = -EAGAIN;
        KPDP_DEBUG(DBG_ERROR, "ERROR: KPDP Result List is empty %p\n", &(gKpdpResultQueue.list));
        goto recv_err;
    }
    entry = gKpdpResultQueue.list.next;
    buf_handle = list_entry(entry, KPDP_ResultQueue_t, list);
    list_del(entry);
    spin_unlock_irqrestore(&(priv->recv_lock), flags);

    if ((NULL == buf_handle->result_info.data) || (0 == buf_handle->result_info.datalen))
    {
        if ((NULL == buf_handle->result_info.data) != (0 == buf_handle->result_info.datalen))
        {
            KPDP_DEBUG(DBG_ERROR, "ERROR: KPDP Result data is Wrong %p, len %d\n", buf_handle->result_info.data, buf_handle->result_info.datalen);
            //Must enter data abort here
            goto recv_putback_on_err;
        }
    }

    rsp.result = buf_handle->result_info.result;
    rsp.CmdID = buf_handle->result_info.CmdID;
    rsp.datalen = buf_handle->result_info.datalen;

    if (0 != buf_handle->result_info.datalen)
    {
        if (copy_to_user(rsp.data, buf_handle->result_info.data, buf_handle->result_info.datalen))
        {
            rv = -EFAULT;
            KPDP_DEBUG(DBG_ERROR, "ERROR: KPDP copy response dara to user Fail\n");
            goto recv_putback_on_err;
        }
    }


    if (copy_to_user(arg, &rsp, sizeof(KPDP_Response_t)))
    {
        rv = -EFAULT;
        KPDP_DEBUG(DBG_ERROR, "ERROR: KPDP copy response infor to user Fail\n");
        goto recv_putback_on_err;
    }
    if (0 != buf_handle->result_info.datalen && buf_handle->result_info.data )
    {
        kfree( buf_handle->result_info.data );
    }
    kfree(buf_handle);
    buf_handle = NULL;

    if (false == list_empty(&(gKpdpResultQueue.list))) //not empty
    {
        KPDP_DEBUG(DBG_INFO, "rsp continue read list:%p, next:%p\n", &(gKpdpResultQueue.list), gKpdpResultQueue.list.next);
        rv = RESULT_NOT_EMPTY;
    }
    mutex_unlock(&priv->recv_mutex);
    return rv;

recv_putback_on_err:
    /* If we got an error, put the message back onto
       the head of the queue. */
    //KPDP_DEBUG(DBG_INFO, "recv_putback_on_err handle s_addr:%p, d_addr:%p\n", entry, &(gKpdpResultQueue.list));
    spin_lock_irqsave(&(priv->recv_lock), flags);
    list_add(entry, &(gKpdpResultQueue.list));
    spin_unlock_irqrestore(&(priv->recv_lock), flags);
    mutex_unlock(&priv->recv_mutex);
    return rv;
 
recv_err:
    mutex_unlock(&priv->recv_mutex);
    return rv;
}



Int32 KPDP_SendCmd(UInt32 cmd, UInt32 arg)
{
    KPDP_CmdQueue_t *kpdp_cmd = NULL;
    void *tdata = NULL;


    kpdp_cmd = kmalloc(sizeof(KPDP_CmdQueue_t), GFP_KERNEL);

    if(!kpdp_cmd)
    {
        KPDP_DEBUG(DBG_ERROR, "Unable to allocate kpdp_cmd memory\n");
    }
    else
    {
        kpdp_cmd->cmd = cmd;
        kpdp_cmd->pdp_cmd = kmalloc(sizeof(KPDP_Command_t), GFP_KERNEL);
        copy_from_user(kpdp_cmd->pdp_cmd, (KPDP_Command_t *)arg, sizeof(KPDP_Command_t));

        KPDP_DEBUG(DBG_INFO, "CmdID:%d datalen:%d\n", kpdp_cmd->pdp_cmd->CmdID, (int)kpdp_cmd->pdp_cmd->datalen);

        if (0 != kpdp_cmd->pdp_cmd->datalen)
        {
            tdata = kmalloc(kpdp_cmd->pdp_cmd->datalen, GFP_KERNEL);
            if(NULL == tdata)
            {
                KPDP_DEBUG(DBG_ERROR, "tdata memory allocate fail!\n");
            }
            else
            {
                copy_from_user(tdata, kpdp_cmd->pdp_cmd->data, kpdp_cmd->pdp_cmd->datalen);
                KPDP_DEBUG(DBG_INFO, "tdata memory allocate success tdata:%p\n", kpdp_cmd->pdp_cmd->data);
                kpdp_cmd->pdp_cmd->data = tdata;
            }
        }
        else
        {
            KPDP_DEBUG(DBG_TRACE, "updp datalen is 0\n");
            kpdp_cmd->pdp_cmd->data = NULL;
        }

        mutex_lock(&gKpdpCmdQueue.mutex);
        list_add_tail(&kpdp_cmd->list, &gKpdpCmdQueue.list); 
        mutex_unlock(&gKpdpCmdQueue.mutex);
        queue_work(gKpdpCmdQueue.cmd_wq, &gKpdpCmdQueue.commandq);
        KPDP_DEBUG(DBG_INFO, "head cmd:%ld list:%p next:%p prev:%p\n", kpdp_cmd->cmd, &kpdp_cmd->list, kpdp_cmd->list.next, kpdp_cmd->list.prev);
    }
    return 0;
}
