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

#include "capi2_pch_api.h"
#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"
#include "capi2_gen_api.h"
#include "capi2_pch_api_old.h"

KpdpDataCallResponse_t Kpdp_pdp_resp[BCM_NET_MAX_DUN_PDP_CNTXS] = {};
#define DUN_PDP_CID(index) (index+1)
#define DUN_PDP_INDEX(cid) (cid-1)

static void SendCHAPOptions();
static void SendPAPOptions();

static UInt8 GetFreePdpContext(void)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_DUN_PDP_CNTXS; i++)
    {
        if (0 == Kpdp_pdp_resp[i].cid)
        {
            Kpdp_pdp_resp[i].cid = DUN_PDP_CID(i);
            KPDP_DEBUG(DBG_INFO, "GetFreePdpContext[%d]=%d \n", i, Kpdp_pdp_resp[i].cid);
            return i;
        }
    }
    return BCM_NET_MAX_DUN_PDP_CNTXS;
}


static UInt8 ReleasePdpContext(UInt8 cid)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_DUN_PDP_CNTXS; i++)
    {
        if (cid == Kpdp_pdp_resp[i].cid)
        {
            Kpdp_pdp_resp[i].cid = 0;
            //memset(&Kpdp_pdp_resp[i], 0, sizeof(KpdpDataCallResponse_t));
            KPDP_DEBUG(DBG_INFO, "ReleasePdpContext[%d]=%d \n", i, Kpdp_pdp_resp[i].cid);
            return i;
        }
    }
    return BCM_NET_MAX_DUN_PDP_CNTXS;
}


static UInt8 AddPdpContext(UInt8 cid)
{
    UInt8 i;
   
    //index to be added to
    i = DUN_PDP_INDEX(cid);
 
    if (i >= BCM_NET_MAX_DUN_PDP_CNTXS)
    {
	KPDP_DEBUG(DBG_ERROR, "AddPdpContext cid=%d exceeds the range \n", cid);
        return BCM_NET_MAX_DUN_PDP_CNTXS;
    }
 
        if (cid == Kpdp_pdp_resp[i].cid)
        {
        //make this warning only, 'cause there are cases pdp context gets reused.
            KPDP_DEBUG(DBG_INFO, "AddPdpContext cid=%d already exists at [%d], error \n", cid, i);
        }
    else
    {
        Kpdp_pdp_resp[i].cid = cid;
    }

    return i;
}

static void FillDataResponseTypeApn(UInt8 cid, char* type, char* apn)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_DUN_PDP_CNTXS; i++)
    {
        if (cid == Kpdp_pdp_resp[i].cid)
        {
            if (NULL != type)
                strcpy(Kpdp_pdp_resp[i].type, type);
            if (NULL != apn)
                strcpy(Kpdp_pdp_resp[i].apn, apn);
            KPDP_DEBUG(DBG_INFO, "FillDataResponseTypeApn[%d]=[%s][%s] \n", i, Kpdp_pdp_resp[i].type, Kpdp_pdp_resp[i].apn);
            return;
        }
    }
}


static void FillDataResponseAddress(UInt8 cid, char* address)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_DUN_PDP_CNTXS; i++)
    {
        if (cid == Kpdp_pdp_resp[i].cid)
        {
            Kpdp_pdp_resp[i].active = 2; // 0=inactive, 1=active/physical link down, 2=active/physical link up
            if (NULL != address)
                strcpy(Kpdp_pdp_resp[i].address, address);
            KPDP_DEBUG(DBG_INFO, "FillDataResponseAddress[%d]=[%s] \n", i, Kpdp_pdp_resp[i].address);
            return;
        }
    }
}


void KPDP_SetupPdpHandler(void *pdp_cmd, Kpdp_CAPI2Info_t *capi2_rsp)
{
    KPDP_CmdList_t *pdata = (KPDP_CmdList_t *)pdp_cmd;
    static KpdpPdpContext_t gContext;

    if (NULL != capi2_rsp)
        KPDP_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::result:0x%x\n", capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            UInt8 pindex;
            char chPdpType[PDP_TYPE_LEN_MAX] = "IP";
            pdata->handler_state = BCM_RESPCAPI2Cmd;

            if (NULL == pdata->pdp_cmd->data)
            {
                KPDP_DEBUG(DBG_ERROR, "PDPActivate Fail with NULL data\n");
                pdata->result = RESULT_ERROR;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            memcpy(&gContext, (KpdpPdpContext_t *)(pdata->pdp_cmd->data), sizeof(KpdpPdpContext_t));
            KPDP_DEBUG(DBG_INFO, "KPDP_SetupPdpHandler - Set PDP Context : apn %s \n", gContext.apn);

            if (gContext.cid!=0)
            {
		KPDP_DEBUG(DBG_INFO, "PDPActivate, cid passed in as of %d\n", gContext.cid);
                if (AddPdpContext(gContext.cid) == BCM_NET_MAX_DUN_PDP_CNTXS)
                {
		    KPDP_DEBUG(DBG_ERROR, "PDPActivate Fail with already existing cid %d\n", gContext.cid);
		    pdata->result = RESULT_ERROR;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    break;
                }

	  	//Build PCHP
                if (gContext.authtype == AUTH_CHAP)
                {
		    SendCHAPOptions(&gContext);
	            pdata->handler_state = BCM_PDP_SetPdpOption2;
                }
                else 
                {
		    SendPAPOptions(&gContext);
	            pdata->handler_state = BCM_PDP_SetPdpOption;
                }
                break;	
	    }

            if (BCM_NET_MAX_DUN_PDP_CNTXS == (pindex = GetFreePdpContext()))
            {
                KPDP_DEBUG(DBG_ERROR, "PDPActivate Fail with over max cid[%d]\n", pindex);
                pdata->result = RESULT_ERROR;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            {
                ClientInfo_t clientInfo;
                UInt8 numParms = (gContext.apn == NULL)?2:3;
                CAPI2_InitClientInfo(&clientInfo, KPDPGetNewTID(), KPDPGetClientID());
                gContext.cid = Kpdp_pdp_resp[pindex].cid;
                FillDataResponseTypeApn(gContext.cid, chPdpType, gContext.apn);
                KPDP_DEBUG(DBG_INFO,"**Calling CAPI2_PdpApi_SetPDPContext numParms %d type:%s apn:%s pindex %d cid %d\n",numParms, chPdpType, (gContext.apn==NULL)?"NULL":gContext.apn, pindex,Kpdp_pdp_resp[pindex].cid  );
            
                CAPI2_PdpApi_SetPDPContext( &clientInfo, 
                                            Kpdp_pdp_resp[pindex].cid, 
                                            numParms, 
                                            chPdpType, 
                                            gContext.apn, 
                                            "", 
                                            0, 
                                            0);
            }
  
            pdata->handler_state = BCM_PDP_SetPdpContext;
        }
        break;

        case BCM_PDP_SetPdpContext:
        {
            if(RESULT_OK != capi2_rsp->result)
            {
                KPDP_DEBUG(DBG_ERROR, "PDPActivate Fail to SetPDPContext[%d]\n", gContext.cid);
                ReleasePdpContext(gContext.cid);
                pdata->result = RESULT_ERROR;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            if (gContext.authtype == AUTH_CHAP)
            {
		SendCHAPOptions(&gContext);
	        pdata->handler_state = BCM_PDP_SetPdpOption2;
            }
            else 
            {
		SendPAPOptions(&gContext);
	        pdata->handler_state = BCM_PDP_SetPdpOption;
            }
        }
	break;

        case BCM_PDP_SetPdpOption:
            {
                ClientInfo_t clientInfo;
	    CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t *rsp = (CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t *)capi2_rsp->dataBuf;

            KPDP_DEBUG(DBG_INFO, "KPDP_SetupPdpHandler - Activate PDP context \n");

            if( (RESULT_OK != capi2_rsp->result) || (rsp==NULL))
            {
                KPDP_DEBUG(DBG_ERROR, "CAPI2_PchExApi_BuildIpConfigOptions Fail\n");
                ReleasePdpContext(gContext.cid);
                pdata->result = RESULT_ERROR;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }


            CAPI2_InitClientInfo( &clientInfo, KPDPGetNewTID(), KPDPGetClientID());
            CAPI2_PchExApi_SendPDPActivateReq( &clientInfo, gContext.cid, ACTIVATE_MMI_IP_RELAY, &(rsp->cie));
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_PDP_SetPdpOption2:
            {
                ClientInfo_t clientInfo;
	    CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t *rsp = (CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t *)capi2_rsp->dataBuf;

            KPDP_DEBUG(DBG_INFO, "KPDP_SetupPdpHandler - Activate PDP context \n");

            if( (RESULT_OK != capi2_rsp->result) || (rsp==NULL))
	    {
                KPDP_DEBUG(DBG_ERROR, "CAPI2_PchExApi_BuildIpConfigOptions2 Fail\n");
                ReleasePdpContext(gContext.cid);
                pdata->result = RESULT_ERROR;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

	    
            CAPI2_InitClientInfo( &clientInfo, KPDPGetNewTID(), KPDPGetClientID());
            CAPI2_PchExApi_SendPDPActivateReq( &clientInfo, gContext.cid, ACTIVATE_MMI_IP_RELAY, &(rsp->ip_cnfg));
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            UInt32 u_pDNS1, u_sDNS1, u_pDNS2, u_sDNS2, u_act_pDNS, u_act_sDNS;
            pdata->bcm_pdp_rsp = kmalloc(sizeof(KpdpPdpData_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KpdpPdpData_t);
            memset(pdata->bcm_pdp_rsp, 0, pdata->rsp_len);

            KPDP_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KPDP_DEBUG(DBG_ERROR, "PDPActivate Fail to SendPDPActivateReq[%d] \n", gContext.cid);
                ReleasePdpContext(gContext.cid);
                pdata->result = RESULT_ERROR;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            if(NULL != capi2_rsp->dataBuf)
            {
                PDP_SendPDPActivateReq_Rsp_t *rsp = (PDP_SendPDPActivateReq_Rsp_t *)capi2_rsp->dataBuf;
                KpdpPdpData_t *rdata = pdata->bcm_pdp_rsp;

                if((rsp->cause != RESULT_OK) || (rsp->response != PCH_REQ_ACCEPTED))
                {
                    KPDP_DEBUG(DBG_ERROR, "PDPActivate Fail cause %d, resp(1 accept) %d, cid %d\r\n",
                        rsp->cause, rsp->response, rsp->activatedContext.cid);
                    ReleasePdpContext(gContext.cid);
                    rdata->cause = rsp->cause;
                    pdata->result = RESULT_ERROR;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    break;
                }

                if (NULL != rsp->activatedContext.pdpAddress)
                {
                    memcpy(rdata->pdpAddress, rsp->activatedContext.pdpAddress, PDP_ADDRESS_LEN_MAX);
                    KPDP_DEBUG(DBG_INFO, "\nPDP Activate: PDP Address %s \r\n", rdata->pdpAddress);
                }

                u_pDNS1 = u_sDNS1 = u_pDNS2 = u_sDNS2 = u_act_pDNS = u_act_sDNS = 0;
                Capi2ReadDnsSrv(&(rsp->activatedContext.protConfig), &u_pDNS1, &u_sDNS1, &u_pDNS2, &u_sDNS2);

                KPDP_DEBUG(DBG_INFO, "\nPDP Activate: pDns1 0x%x, sDns1 0x%x, pDns2 0x%x, sDns2 0x%x \r\n",
                    (unsigned int)u_pDNS1, (unsigned int)u_sDNS1, (unsigned int)u_pDNS2, (unsigned int)u_sDNS2);

                if((u_act_pDNS = u_pDNS1) == 0)
                {
                    u_act_pDNS = u_pDNS2;
                }
                if((u_act_sDNS = u_sDNS1) == 0)
                {
                    u_act_sDNS = u_sDNS2;
                }
                rdata->priDNS = u_act_pDNS;
                rdata->secDNS = u_act_sDNS;
                rdata->cid = rsp->activatedContext.cid;
                KPDP_DEBUG(DBG_INFO, "PDP Activate Resp - cid %d \n", rsp->activatedContext.cid);
                FillDataResponseAddress(rdata->cid, rdata->pdpAddress);

                pdata->result = RESULT_OK;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->result = RESULT_ERROR;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KPDP_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}



void KPDP_DeactivatePdpHandler(void *pdp_cmd, Kpdp_CAPI2Info_t *capi2_rsp)
{
    KPDP_CmdList_t *pdata = (KPDP_CmdList_t *)pdp_cmd;

    if (NULL != capi2_rsp)
        KPDP_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::result:0x%x\n", capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            char *cid = (char *)(pdata->pdp_cmd->data);
            UInt8 ContextID = (UInt8)(*cid - 0x30);
            UInt8 i;

            KPDP_DEBUG(DBG_INFO, "KPDP_DeactivatePdpHandler - length %d, Cid:%d \n", pdata->pdp_cmd->datalen, ContextID);
            pdata->bcm_pdp_rsp = kmalloc(sizeof(KpdpPdpData_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KpdpPdpData_t);
            memset(pdata->bcm_pdp_rsp, 0, pdata->rsp_len);
            for (i=0 ; i<BCM_NET_MAX_DUN_PDP_CNTXS ; i++)
            {
                if (ContextID == Kpdp_pdp_resp[i].cid) // To find the contextID in Kpdp_pdp_resp list, need to deactivate the context
                {
                    KPDP_DEBUG(DBG_INFO, "ReleasePdpContext[%d]=%d \n", i, Kpdp_pdp_resp[i].cid);
                    break;
                }
                else if ((BCM_NET_MAX_DUN_PDP_CNTXS-1) == i) // Return finish state if can't find the contestID in Kpdp_pdp_resp list.
                {
		    KPDP_DEBUG(DBG_INFO, "Can't find corresponding cid = %d\n", ContextID);
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                    return;
                }
             }
            {
                ClientInfo_t clientInfo;
                CAPI2_InitClientInfo( &clientInfo, KPDPGetNewTID(), KPDPGetClientID());
                CAPI2_PchExApi_SendPDPDeactivateReq( &clientInfo, ContextID );
            }
            ReleasePdpContext(ContextID);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
        	   KPDP_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KPDP_DEBUG(DBG_ERROR, "PDPDeActivate Fail to SendPDPDeActivateReq \n");
                pdata->result = RESULT_ERROR;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            if(NULL != capi2_rsp->dataBuf)
            {
                PDP_SendPDPDeactivateReq_Rsp_t *rsp = (PDP_SendPDPDeactivateReq_Rsp_t *)capi2_rsp->dataBuf;
                KpdpPdpData_t *rdata = pdata->bcm_pdp_rsp;
                if(rsp->response != PCH_REQ_ACCEPTED)
                {
                    KPDP_DEBUG(DBG_ERROR, "PDPDeActivate Fail resp(1 accept) %d, cid %d\r\n", rsp->response, rsp->cid);
                    pdata->result = RESULT_ERROR;
                    rdata->cause = rsp->cause;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    break;
                }

                rdata->cid = rsp->cid;
                KPDP_DEBUG(DBG_INFO, "PDP Deactivate Resp - cid %d \n", rsp->cid);

                pdata->result = RESULT_OK;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->result = RESULT_ERROR;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KPDP_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


static void SendCHAPOptions(KpdpPdpContext_t* context)
{
    ClientInfo_t clientInfo;
    CHAP_ChallengeOptions_t challOptions;
    CHAP_ResponseOptions_t respOptions;

    KPDP_DEBUG(DBG_INFO, "SendCHAPOptions chall_len=%d, resp_len=%d\r\n", context->chall_len, context->resp_len);

    CAPI2_InitClientInfo( &clientInfo, KPDPGetNewTID(), KPDPGetClientID());

    challOptions.flag=1;
    challOptions.len =context->chall_len;
    memcpy(challOptions.content, context->challenge, PDP_CHALLENGE_LEN_MAX); 

    respOptions.flag=1;
    respOptions.len =context->resp_len;
    memcpy(respOptions.content, context->response, PDP_RESPONSE_LEN_MAX); 


    CAPI2_PchExApi_BuildIpConfigOptions2(&clientInfo, REQUIRE_CHAP, &challOptions, &respOptions, NULL);
}

static void SendPAPOptions(KpdpPdpContext_t* context)
{
    ClientInfo_t clientInfo;
    CAPI2_InitClientInfo( &clientInfo, KPDPGetNewTID(), KPDPGetClientID());

    KPDP_DEBUG(DBG_INFO, "SendPAPOptions \r\n");
    CAPI2_PchExApi_BuildIpConfigOptions(&clientInfo, context->username, context->password, REQUIRE_PAP);
}
