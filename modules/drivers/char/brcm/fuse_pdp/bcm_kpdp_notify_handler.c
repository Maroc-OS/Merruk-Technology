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

#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"


extern KpdpDataCallResponse_t Kpdp_pdp_resp[BCM_NET_MAX_DUN_PDP_CNTXS];


//using hotplug to stop pppd
#define PDPUTIL "/system/bin/pdputil"

//remember to change to static
void Kpdp_pdp_deactivate_ind(int cid)
{
    //invoke "pdputil Event 1002 (MSG_PDP_DEACTIVATION_IND)" 

    char event_id[10];
    char cid_str[10];
    sprintf(event_id,"%d", PDP_DEACTIVATION_IND);
    sprintf(cid_str,"%d", cid);

    KPDP_DEBUG(DBG_INFO, "invoke \"pdputil Event %s %s\" \n", event_id, cid_str);
    {
        char *event_args[] = {PDPUTIL,"Event", event_id, cid_str, NULL};
        call_usermodehelper(PDPUTIL, event_args, NULL,0);
    }
}


//******************************************************************************
//
// Function Name: ProcessNotification
//
// Description:  Process the notify message from CP side.
//
// Notes:
//
//******************************************************************************
void KPDP_ProcessNotification(Kpdp_CAPI2Info_t *notify)
{
    //KPDP_DEBUG(DBG_TRACE, "msgType:0x%lX!\n", (UInt32)notify->msgType);

    switch((UInt32)notify->msgType)
        {
        case MSG_PDP_DEACTIVATION_IND:
        {
            PDP_PDPDeactivate_Ind_t *pind = (PDP_PDPDeactivate_Ind_t *) notify->dataBuf;
            UInt8 i;
            
            KPDP_DEBUG(DBG_INFO, "MSG_PDP_DEACTIVATION_IND::cid %d,type[%s],add[%s]\n", pind->cid, pind->pdpType, pind->pdpAddress);
            for (i=0; i<BCM_NET_MAX_DUN_PDP_CNTXS; i++)
            {
                if (pind->cid == Kpdp_pdp_resp[i].cid)
                {
                    //Kpdp_pdp_resp[i].active = 0;
                    memset(&Kpdp_pdp_resp[i], 0, sizeof(KpdpDataCallResponse_t));
		    Kpdp_pdp_resp[i].cid = pind->cid;
                    KPDP_DEBUG(DBG_INFO, "MSG_PDP_DEACTIVATION_IND[%d]=%d \n", i, Kpdp_pdp_resp[i].cid);
                    //KPDP_SendNotify(PDP_UNSOL_DATA_CALL_LIST_CHANGED, &Kpdp_pdp_resp[i], sizeof(KpdpDataCallResponse_t));
                    Kpdp_pdp_resp[i].cid = 0; //KPDP_SendNotify has backup Kpdp_Kpdp_pdp_resp by memcpy, so we can modify here

		    //notify user space
		    Kpdp_pdp_deactivate_ind(pind->cid);
                    break;
                }
            }
            break;
        }
        default:
        {
            //KPDP_DEBUG(DBG_INFO, "The msgType:0x%lX is not process yet...!\n", (UInt32)notify->msgType);
            break;
        }
    }
}

