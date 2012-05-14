/*********************************************************************
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
***************************************************************************/
#define UNDEF_SYS_GEN_MIDS
#define DEFINE_SYS_GEN_MIDS_NEW

#include "string.h"
#include "mobcom_types.h"

#include "taskmsgs.h"
#include "string.h"
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_sync_api.h"

#include "xdr.h"
#include "audio_consts.h"
#include "pmu.h"
#include "hal_pmu.h"
#include "rtc.h"
#include "i2c_drv.h"
#include "hal_pmu_glue.h"
#include "cpps_control.h"
#include "flash_api.h"
#include "sys_usb_rpc.h"
#include "sysparm.h"
#include "sys_api.h"
#include "capi2_cp_hal_api.h"
#include "hal_adc.h"
#include "hal_em_battmgr.h"
#include "meas_mgr.h"
#include "sys_common_rpc.h"
#include "sys_usb_rpc.h"
#include "sys_msc_rpc.h"
#include "logapi.h"
#include "sys_eem_rpc.h"
#include "usb_msg.h"


#if defined (ENABLE_USB_MEGASIM)
static SysUsbRpc_NotifyCb sysUsbRpcNotifyCb = NULL;
#endif

Boolean SysUsbAp_MsgHandler(InterTaskMsg_t *msg)
{
    Boolean result = 0;
    Boolean rsp = TRUE;
    SYS_TRACE("AP SysUsbAp_MsgHandler msg:%p", msg);

     Log_EnableLogging(LOGID_USB,TRUE);

     if(!msg)
     {
         Log_DebugPrintf(LOGID_USB, "%s SysUsbAp_MsgHandler: NULL msg!!\n", PROC_TYPE);
         return FALSE;        
     }
     switch(msg->msgType)
     {
        case USB_IPC_MSC_INIT:
             Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_INIT call ipcMsc_Init", PROC_TYPE);
             break;
        case USB_IPC_MSC_UNINIT:
             Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_UNINIT Nothing to do", PROC_TYPE);
             break;
        case USB_IPC_MSC_MEDIA_INSERT:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_MEDIA_INSERT ", PROC_TYPE);
            sysMscRpc_Insert(msg);
            break;
        case USB_IPC_MSC_MEDIA_MOUNT:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_MEDIA_MOUNT ", PROC_TYPE);
            sysMscRpc_Mount(msg);
            rsp = FALSE;
            break;
        case USB_IPC_MSC_MEDIA_REMOVE:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_MEDIA_REMOVE ", PROC_TYPE);
            sysMscRpc_Remove(msg);
            break;
        case USB_IPC_MSC_DEVICE_ATTACH:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_DEVICE_ATTACH Nothing to do", PROC_TYPE);
            break;
        case USB_IPC_MSC_DEVICE_DETACH:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_DEVICE_DETACH Nothing to do", PROC_TYPE);
            break;
        case USB_IPC_MSC_READ:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_READ Nothing to do", PROC_TYPE);
            break;
        case USB_IPC_MSC_WRITE:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_WRITE Nothing to do", PROC_TYPE);
            break;

#if defined (ENABLE_USB_MEGASIM)
        case USB_IPC_EEM_CTRL:
        {
            usbEemCtrl_t *ctrl = (usbEemCtrl_t *)msg->dataBuf;
            
            Log_DebugPrintf(LOGID_SYSEEMRPC, "EEM_Task MSG USB_IPC_EEM_CTRL Recvd EEM Msg:%d Value:%d", ctrl->msg, ctrl->value);
            result = JusbAdapter_SendMsg(0, (USB_MSG_TYPES)ctrl->msg, &ctrl->value, sizeof(ctrl->value));
            FreeInterTaskMsg(msg);
            rsp = FALSE;
            break;
        }
        case USB_IPC_NOTIFY:
            Log_DebugPrintf(LOGID_USB, "%s usb cb USB_IPC_NOTIFY val:%d ", PROC_TYPE, *(UInt32 *)msg->dataBuf);
            if(sysUsbRpcNotifyCb)
            {
                sysUsbRpcNotifyCb(*(UInt32 *)msg->dataBuf);
            }
            break;
#endif            
        default:
        Log_DebugPrintf(LOGID_USB, "%s USB msg Default Type:%d len:%d buf:%p", PROC_TYPE, msg->msgType, msg->dataLength, msg->dataBuf);
    }
    if(!result)
    {
        Log_DebugPrintf(LOGID_USB, "%s USB JusbAdapter_SendMsg failed type:%d", PROC_TYPE, msg->msgType);
    }

    return rsp;
}

void SysUsbRpc_RegisterNotify(SysUsbRpc_NotifyCb cb)
{
#if defined (ENABLE_USB_MEGASIM)
    sysUsbRpcNotifyCb = cb;
#endif            
}


void sys_usb_init(void)
{
    SYS_TRACE("AP sys_usb_init");
    SysUsbRpc_InstallHandler(SysUsbAp_MsgHandler);
    
#if defined(ENABLE_USB_MEGASIM)
    EEM_Init();
#endif
}


