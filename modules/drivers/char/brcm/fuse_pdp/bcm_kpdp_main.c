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
#include "bcm_kpdp_main.h"

#include "bcm_kpdp_ioctl.h"
#include "bcm_kpdp_cmd_handler.h"

#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"
//#include "bcm_cp_cmd_handler.h"
#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/bcm_reset_utils.h>
//#include "bcm_kril_sysrpc_handler.h"
//#include "bcm_kril_adcrpc_handler.h"

static int __init bcm_fuse_kpdp_init_module(void);
static void __exit bcm_fuse_kpdp_exit_module(void);
static struct class *kpdp_class; 

#ifdef CONFIG_HAS_WAKELOCK
struct wake_lock kpdp_rsp_wake_lock;
struct wake_lock kpdp_notify_wake_lock;
#endif
extern Int32 KPDP_GetRsp(struct file *filp, UInt32 cmd, UInt32 arg);

struct completion gKpdpCmdThreadExited;

KPDP_ResultQueue_t gKpdpResultQueue;
KPDP_Param_t gKpdpParam;


extern UInt8 CAPI2_SYS_RegisterClient(RPC_RequestCallbackFunc_t reqCb,
                                      RPC_ResponseCallbackFunc_t respCb,
                                      RPC_AckCallbackFunc_t ackCb,
                                      RPC_FlowControlCallbackFunc_t flowControlCb);

static int KPDP_Open(struct inode *inode, struct file *filp)
{
    KPDP_Param_t *priv;

    priv = kmalloc(sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    KPDP_DEBUG(DBG_INFO, "KPDP_Open %s: major %d minor %d (pid %d)\n", __func__, imajor(inode), iminor(inode), current->pid);

    priv->file = filp;
    filp->private_data = priv;

    /*init response*/
    spin_lock_init(&(priv->recv_lock));
    INIT_LIST_HEAD(&(gKpdpResultQueue.list));

    init_waitqueue_head(&gKpdpParam.read_wait);
    mutex_init(&priv->recv_mutex);

    KPDP_DEBUG(DBG_INFO, "i_private=%p private_data=%p, wait=%p\n", inode->i_private, filp->private_data, &priv->recv_wait);

    gKpdpParam.recv_lock = priv->recv_lock;
    return 0;
}

static int KPDP_Release(struct inode *inode, struct file *filp)
{
    KPDP_DEBUG(DBG_INFO, "KPDP_Release %s: major %d minor %d (pid %d)\n", __func__, imajor(inode), iminor(inode), current->pid);
    KPDP_DEBUG(DBG_INFO, "i_private=%p private_data=%p\n", inode->i_private, filp->private_data);
    return 0;
}

int KPDP_Read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
    /*buffer copy to user*/
    int rc = 0;
    KPDP_DEBUG(DBG_INFO, "KPDP_Read Enter %s\n", __func__);

    return rc;
}

int KPDP_Write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
    /*buffer copy from user*/
    int rc = 0;
    KPDP_DEBUG(DBG_INFO, "KPDP_Write %s: count %d pos %lld\n", __func__, size, *offset);

    return rc;
}

static long KPDP_Ioctl(struct file *filp, unsigned int cmd, UInt32 arg)
{
    int rc = -1;

    switch(cmd)
    {
        case BCM_KPDPIO_SET_COMMAND:
            rc = KPDP_SendCmd(cmd, arg);
            break;

        case BCM_KPDPIO_GET_RESPONSE:
            rc = KPDP_GetRsp(filp, cmd, arg);
            break;

        default:
            KPDP_DEBUG(DBG_ERROR, "we don't process the kpdp_ioctl cmd:0x%x!\n", cmd);
            break;
    }

    return(rc);
}


static unsigned int KPDP_Poll(struct file *filp, poll_table *wait)
{
    KPDP_Param_t *priv = filp->private_data;
    UInt32 mask = 0;
    UInt32 flags;

    poll_wait(filp, &gKpdpParam.read_wait, wait);

    spin_lock_irqsave(&priv->recv_lock, flags);

    if (!list_empty(&(gKpdpResultQueue.list)))
     {
         KPDP_DEBUG(DBG_INFO, "KPDP_Poll() mask \n");
        mask |= (POLLIN | POLLRDNORM);
     }

    spin_unlock_irqrestore(&priv->recv_lock, flags);

   return mask;
}


static struct file_operations kpdp_ops = 
{
	.owner = THIS_MODULE,
	.open	 = KPDP_Open,
	.read	 = KPDP_Read,
	.write = KPDP_Write,
	.unlocked_ioctl = KPDP_Ioctl,
	.poll  = KPDP_Poll,
	.mmap	 = NULL,
	.release = KPDP_Release,
};


static int KPDP_Init(void)
{
    int ret = 0;

    /*wake lock -- init*/
#ifdef CONFIG_HAS_WAKELOCK
    wake_lock_init(&kpdp_rsp_wake_lock, WAKE_LOCK_SUSPEND, "kpdp_rsp_wake_lock");
    wake_lock_init(&kpdp_notify_wake_lock, WAKE_LOCK_SUSPEND, "kpdp_notify_wake_lock");
#endif
    //init kpdp parameter
    KPDP_InitHandler();

    return ret;
} // KPDP_Init


static void KPDP_RPC_RespCbk(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData)
{
    UInt32 len;
    void* dataBuf;
    CAPI2_ReqRep_t* reqRep =  (CAPI2_ReqRep_t*)pMsg->dataBuf;
    CAPI2_GetPayloadInfo(reqRep, pMsg->msgId, &dataBuf, &len);

    //KPDP_DEBUG(DBG_INFO, "KPDP_RPC_RespCbk\n");
    KPDP_Capi2HandleRespCbk(pMsg->tid, pMsg->clientID, pMsg->msgId, reqRep->result, dataBuf, len, dataBufHandle);
}

/****************************************************************************
*
*  CAPI2_ClientInit(void);
*
*  Register IPC module.
*
***************************************************************************/
UInt32 KPDPCAPI2_ClientInit(void)
{
    UInt32 ret = 0;
    UInt8  clientID;
    Boolean bEnabled;
    
    clientID = CAPI2_SYS_RegisterClient(NULL, KPDP_RPC_RespCbk, KPDP_Capi2HandleAckCbk, KPDP_Capi2HandleFlowCtrl);
    bEnabled = RPC_EnableUnsolicitedMsgs(RPC_SYS_GetClientHandle(clientID), TRUE);

    KPDPSetClientID(clientID);
    
    KPDP_DEBUG(DBG_INFO, "CAPI2_ClientInit clientID:0x%x UnsolEnabled:%s\n", clientID, (bEnabled?"TRUE":"FALSE"));
    
    // initialize additional RPC interfaces needed for CIB
    // **FIXME** capirpc channel appears to no longer be required (only had SMS ME, which
    // isn't needed under Android)
//    KPDP_CapiRpc_Init();
    //KPDP_SysRpc_Init();
    //KPDP_ADCRpc_Init();
    
    return(ret);
}

/****************************************************************************
*
*  bcm_fuse_kpdp_init_module(void);
*
*  Init module.
*
***************************************************************************/
static int __init bcm_fuse_kpdp_init_module(void)
{
    int ret = 0;

    KPDP_DEBUG(DBG_INFO, "bcm_fuse_kpdp_init_module()");
    pr_info("KPDP Support 1.00 (BUILD TIME "__DATE__" "__TIME__")\n" );

    if ( AP_ONLY_BOOT == get_ap_boot_mode() )
    {
        KPDP_DEBUG(DBG_INFO, "bcm_fuse_kpdp_init_module: AP only; don't register driver\n");
        return ret;
    }

    if (( ret = register_chrdev( BCM_KPDP_MAJOR, "bcm_kpdp", &kpdp_ops )) < 0 )
    {
        KPDP_DEBUG(DBG_ERROR, "kpdp: register_chrdev failed for major %d\n", BCM_KPDP_MAJOR );
        goto out;
    }

    kpdp_class = class_create(THIS_MODULE, "bcm_kpdp");
    if (IS_ERR(kpdp_class)) {
        return PTR_ERR(kpdp_class);
    }

    device_create(kpdp_class, NULL, MKDEV(BCM_KPDP_MAJOR, 0),NULL, "bcm_kpdp");

    KPDP_DEBUG(DBG_INFO, "%s driver(major %d) installed.\n", "bcm_kpdp", BCM_KPDP_MAJOR);
    /**
       Register callbacks with the IPC module
       Note: does not depend on the channel
    */
    ret = KPDPCAPI2_ClientInit();

    /** Init KPDP Driver */
    ret = KPDP_Init();

    if (ret) 
    {
        ret = -1;
        KPDP_DEBUG(DBG_ERROR, "KPDP_Init fail...!\n");
        goto out_unregister;
    }

    return ret;
out_unregister:
    unregister_chrdev( BCM_KPDP_MAJOR, "bcm_kpdp");
out:
   return(ret);
}

/****************************************************************************
*
*  bcm_fuse_kpdp_exit_module(void);
*
*  Exit module.
*
***************************************************************************/
static void __exit bcm_fuse_kpdp_exit_module(void)
{
#ifdef CONFIG_WAKELCOK
    wake_lock_destroy(&kpdp_rsp_wake_lock);
    wake_lock_destroy(&kpdp_notify_wake_lock);
#endif

    return;
}


module_init(bcm_fuse_kpdp_init_module);
module_exit(bcm_fuse_kpdp_exit_module);
