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
#include "bcm_kril_common.h"
#include "bcm_kril_main.h"
#include "bcm_kril_ioctl.h"
#include "bcm_kril_cmd_handler.h"
#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"
#include "bcm_cp_cmd_handler.h"
#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/bcm_reset_utils.h>
#include "bcm_kril_sysrpc_handler.h"
#include "bcm_kril_adcrpc_handler.h"

static int __init bcm_fuse_kril_init_module(void);
static void __exit bcm_fuse_kril_exit_module(void);
static struct class *kril_class; 

#ifdef CONFIG_HAS_WAKELOCK
struct wake_lock kril_rsp_wake_lock;
struct wake_lock kril_notify_wake_lock;
struct wake_lock kril_result_wake_lock;
#endif
extern Int32 KRIL_GetRsp(struct file *filp, UInt32 cmd, UInt32 arg);
extern bcm_kril_dev_result_t bcm_dev_results[TOTAL_BCMDEVICE_NUM];

static long  gKRILCmdThreadPid = 0;
struct completion gCmdThreadExited;

KRIL_ResultQueue_t gKrilResultQueue;
KRIL_Param_t gKrilParam;

extern UInt8 CAPI2_SYS_RegisterClient(RPC_RequestCallbackFunc_t reqCb,
                                      RPC_ResponseCallbackFunc_t respCb,
                                      RPC_AckCallbackFunc_t ackCb,
                                      RPC_FlowControlCallbackFunc_t flowControlCb);
extern RPC_Result_t RPC_SYS_EndPointRegister(RpcProcessorType_t processorType);

#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
extern void KRIL_InitDVFSHandler( void );
extern void KRIL_UnregDVFSHandler( void );
#endif

static int KRIL_Open(struct inode *inode, struct file *filp)
{
    KRIL_Param_t *priv;

    priv = kmalloc(sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    KRIL_DEBUG(DBG_INFO, "KRIL_Open %s: major %d minor %d (pid %d)\n", __func__, imajor(inode), iminor(inode), current->pid);

    priv->file = filp;
    filp->private_data = priv;

    /*init response*/
    spin_lock_init(&(priv->recv_lock));
    INIT_LIST_HEAD(&(gKrilResultQueue.list));

    init_waitqueue_head(&gKrilParam.read_wait);
    mutex_init(&priv->recv_mutex);

    KRIL_DEBUG(DBG_INFO, "i_private=%p private_data=%p, wait=%p\n", inode->i_private, filp->private_data, &priv->recv_wait);

    gKrilParam.recv_lock = priv->recv_lock;
    return 0;
}

static int KRIL_Release(struct inode *inode, struct file *filp)
{
    KRIL_DEBUG(DBG_INFO, "KRIL_Release %s: major %d minor %d (pid %d)\n", __func__, imajor(inode), iminor(inode), current->pid);
    KRIL_DEBUG(DBG_INFO, "i_private=%p private_data=%p\n", inode->i_private, filp->private_data);
    return 0;
}

int KRIL_Read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
    /*buffer copy to user*/
    int rc = 0;
    KRIL_DEBUG(DBG_INFO, "KRIL_Read Enter %s\n", __func__);

    return rc;
}

int KRIL_Write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
    /*buffer copy from user*/
    int rc = 0;
    KRIL_DEBUG(DBG_INFO, "KRIL_Write %s: count %d pos %lld\n", __func__, size, *offset);

    return rc;
}

static long KRIL_Ioctl(struct file *filp, unsigned int cmd, UInt32 arg)
{
    int rc = -EINVAL;

    switch(cmd)
    {
        case BCM_RILIO_SET_COMMAND:
            rc = KRIL_SendCmd(cmd, arg);
            break;

        case BCM_RILIO_GET_RESPONSE:
            rc = KRIL_GetRsp(filp, cmd, arg);
            break;

        default:
            KRIL_DEBUG(DBG_ERROR, "we don't process the kril_ioctl cmd:0x%x!\n", cmd);
            break;
    }

    return(rc);
}


static unsigned int KRIL_Poll(struct file *filp, poll_table *wait)
{
    KRIL_Param_t *priv = filp->private_data;
    UInt32 mask = 0;
    UInt32 flags;
    #ifdef CONFIG_HAS_WAKELOCK
    if (wake_lock_active(&kril_result_wake_lock) != 0 && TRUE == list_empty(&(gKrilResultQueue.list))) // if wake lock exist and result queue is empty, need to do wake_unlock
    {
        do
        {
            KRIL_DEBUG(DBG_INFO, "wake_lock_active(&kril_result_wake_lock):%d wake_unlock\n", wake_lock_active(&kril_result_wake_lock));
            wake_unlock(&kril_result_wake_lock);
        } while (wake_lock_active(&kril_result_wake_lock) != 0 && TRUE == list_empty(&(gKrilResultQueue.list))); // don't need to unlock wake_lock if no lock state and result is empty
    }
#endif
   	poll_wait(filp, &gKrilParam.read_wait, wait);

   	spin_lock_irqsave(&priv->recv_lock, flags);

   	if (!list_empty(&(gKrilResultQueue.list)))
  		    mask |= (POLLIN | POLLRDNORM);

	   spin_unlock_irqrestore(&priv->recv_lock, flags);

	   return mask;
}


static struct file_operations kril_ops = 
{
	.owner = THIS_MODULE,
	.open	 = KRIL_Open,
	.read	 = KRIL_Read,
	.write = KRIL_Write,
	.unlocked_ioctl = KRIL_Ioctl,
	.poll  = KRIL_Poll,
	.mmap	 = NULL,
	.release = KRIL_Release,
};

#ifdef BRCM_AGPS_CONTROL_PLANE_ENABLE
void AGPS_CP_Init(void)
{
	ClientInfo_t lcsClientInfo;
	CAPI2_InitClientInfo(&lcsClientInfo, GetNewTID(), GetClientID());

	//CAPI2_LCS_RegisterRrcDataHandler(GetNewTID(), GetClientID(), lcsClientInfo);
	//CAPI2_LCS_RegisterRrlpDataHandler(GetNewTID(), GetClientID(), lcsClientInfo);
	CAPI2_LcsApi_RrlpRegisterDataHandler(&lcsClientInfo);
	CAPI2_LcsApi_RrcRegisterDataHandler(&lcsClientInfo);
	KRIL_DEBUG(DBG_ERROR, "CAPI2_LCS_RegisterRrlpDataHandler ClientID:0x%x\n", lcsClientInfo.clientId);
}
#endif

/****************************************************************************
*
*  KRIL_Receive_thread
*
*  Worker thread to receive data from URIL.
*
***************************************************************************/
static int KRIL_Init(void)
{
    int ret = 0;

    /*wake lock -- init*/
#ifdef CONFIG_HAS_WAKELOCK
    wake_lock_init(&kril_rsp_wake_lock, WAKE_LOCK_SUSPEND, "kril_rsp_wake_lock");
    wake_lock_init(&kril_notify_wake_lock, WAKE_LOCK_SUSPEND, "kril_notify_wake_lock");
    wake_lock_init(&kril_result_wake_lock, WAKE_LOCK_SUSPEND, "kril_result_wake_lock");
#endif
    //init kril parameter
    KRIL_InitHandler();

#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
    // initialize RIL DVFS client
    KRIL_InitDVFSHandler(  );
#endif

#ifdef BRCM_AGPS_CONTROL_PLANE_ENABLE
	if(ret ==0)
	{
		//Init Success
		AGPS_CP_Init();
	}
#endif

    return ret;
} // KRIL_Init


void G_RPC_RespCbk(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData)
{
    UInt32 len;
    void* dataBuf;
    CAPI2_ReqRep_t* reqRep =  (CAPI2_ReqRep_t*)pMsg->dataBuf;
    CAPI2_GetPayloadInfo(reqRep, pMsg->msgId, &dataBuf, &len);

    KRIL_Capi2HandleRespCbk(pMsg->tid, pMsg->clientID, pMsg->msgId, reqRep->clientInfo.dialogId, reqRep->result, dataBuf, len, dataBufHandle);
}

/****************************************************************************
*
*  CAPI2_ClientInit(void);
*
*  Register IPC module.
*
***************************************************************************/
UInt32 CAPI2_ClientInit(void)
{
    UInt32 ret = 0;
    UInt8  clientID;
    RPC_Handle_t handle;
    Boolean bEnabled;
    
    clientID = CAPI2_SYS_RegisterClient(NULL, G_RPC_RespCbk, KRIL_Capi2HandleAckCbk, KRIL_Capi2HandleFlowCtrl);
    bEnabled = RPC_EnableUnsolicitedMsgs(RPC_SYS_GetClientHandle(clientID), TRUE);

    SetClientID(clientID);
    
    KRIL_DEBUG(DBG_INFO, "CAPI2_ClientInit clientID:0x%x UnsolEnabled:%s\n", clientID, (bEnabled?"TRUE":"FALSE"));
    
    // initialize additional RPC interfaces needed for CIB
    // **FIXME** capirpc channel appears to no longer be required (only had SMS ME, which
    // isn't needed under Android)
//    KRIL_CapiRpc_Init();
    KRIL_SysRpc_Init();
    KRIL_ADCRpc_Init();
    
    return(ret);
}

/****************************************************************************
*
*  bcm_fuse_kril_init_module(void);
*
*  Init module.
*
***************************************************************************/
static int __init bcm_fuse_kril_init_module(void)
{
    int ret = 0;

    pr_info("KRIL Support 1.00 (BUILD TIME "__DATE__" "__TIME__")\n" );

    // check for AP only boot mode
    if ( AP_ONLY_BOOT == get_ap_boot_mode() )
    {
        // AP only, so don't continue with KRIL init
        KRIL_DEBUG(DBG_INFO, "bcm_fuse_kril_init_module: AP only; don't register driver\n");
        return ret;
    }

    if (( ret = register_chrdev( BCM_KRIL_MAJOR, "bcm_kril", &kril_ops )) < 0 )
    {
        KRIL_DEBUG(DBG_ERROR, "kril: register_chrdev failed for major %d\n", BCM_KRIL_MAJOR );
        goto out;
    }

    kril_class = class_create(THIS_MODULE, "bcm_kril");
    if (IS_ERR(kril_class)) {
        return PTR_ERR(kril_class);
    }

    device_create(kril_class, NULL, MKDEV(BCM_KRIL_MAJOR, 0),NULL, "bcm_kril");

    KRIL_DEBUG(DBG_INFO, "%s driver(major %d) installed.\n", "bcm_kril", BCM_KRIL_MAJOR);
    /**
       Register callbacks with the IPC module
       Note: does not depend on the channel
    */
    ret = CAPI2_ClientInit();

    /** Init KRIL Driver */
    ret = KRIL_Init();

    if (ret) 
    {
        ret = -1;
        KRIL_DEBUG(DBG_ERROR, "KRIL_Init fail...!\n");
        goto out_unregister;
    }

    return ret;
out_unregister:
    unregister_chrdev( BCM_KRIL_MAJOR, "bcm_kril");
out:
   return(ret);
}

/****************************************************************************
*
*  bcm_fuse_kril_exit_module(void);
*
*  Exit module.
*
***************************************************************************/
static void __exit bcm_fuse_kril_exit_module(void)
{
    int i;

    if (gKRILCmdThreadPid >= 0)
    {
        kill_proc_info(SIGTERM, SEND_SIG_PRIV, gKRILCmdThreadPid);
        wait_for_completion(&gCmdThreadExited);
    }
	  
	  // Release allocated Notify list memory.
    for( i = 0; i < TOTAL_BCMDEVICE_NUM ; i++)
    {
        if(bcm_dev_results[i].notifyid_list != NULL && bcm_dev_results[i].notifyid_list_len != 0)
        {
            kfree(bcm_dev_results[i].notifyid_list);
            bcm_dev_results[i].notifyid_list = NULL;
            bcm_dev_results[i].notifyid_list_len = 0;
        }
    }
#ifdef CONFIG_WAKELCOK
    wake_lock_destroy(&kril_rsp_wake_lock);
    wake_lock_destroy(&kril_notify_wake_lock);
#endif

#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
    KRIL_UnregDVFSHandler( );
#endif

    return;
}


module_init(bcm_fuse_kril_init_module);
module_exit(bcm_fuse_kril_exit_module);
