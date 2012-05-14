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
#include "bcm_kril_capi2_handler.h"
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"

#include "capi2_pch_api.h"
#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"
#include "capi2_gen_api.h"
#include "capi2_pch_api_old.h"

KrilDataCallResponse_t pdp_resp[BCM_NET_MAX_RIL_PDP_CNTXS] = {0};
#define RIL_PDP_CID(index) (BCM_NET_RIL_PDP_CNTXS_BASE + index)

static int ParsePdpFailCause(Result_t value)
{
    BRIL_LastDataCallActivateFailCause cause;
    switch (value)
    {
        case PDP_OPERATOR_DETERMINED_BARRING:
            cause = BCM_PDP_FAIL_OPERATOR_BARRED;
            break;

        case PDP_INSUFFICIENT_RESOURCES:
            cause = BCM_PDP_FAIL_INSUFFICIENT_RESOURCES;
            break;

        case PDP_MISSING_OR_UNKNOWN_APN:
            cause = BCM_PDP_FAIL_MISSING_UKNOWN_APN;
            break;

        case PDP_UNKNOWN_PDP_ADDRESS:
            cause = BCM_PDP_FAIL_UNKNOWN_PDP_ADDRESS_TYPE;
            break;

        case PDP_USER_AUTH_FAILED:
            cause = BCM_PDP_FAIL_USER_AUTHENTICATION;
            break;

        case PDP_ACTIVATION_REJECTED_BY_GGSN:
            cause = BCM_PDP_FAIL_ACTIVATION_REJECT_GGSN;
            break;

        case PDP_ACTIVATION_REJECTED_UNSPECIFIED:
            cause = BCM_PDP_FAIL_ACTIVATION_REJECT_UNSPECIFIED;
            break;

        case PDP_SERVICE_OPT_NOT_SUPPORTED:
            cause = BCM_PDP_FAIL_SERVICE_OPTION_NOT_SUPPORTED;
            break;

        case PDP_REQ_SERVICE_NOT_SUBSCRIBED:
            cause = BCM_PDP_FAIL_SERVICE_OPTION_NOT_SUBSCRIBED;
            break;

        case PDP_SERVICE_TEMP_OUT_OF_ORDER:
            cause = BCM_PDP_FAIL_SERVICE_OPTION_OUT_OF_ORDER;
            break;

        case PDP_NSAPI_ALREADY_USED:
            cause = BCM_PDP_FAIL_NSAPI_IN_USE;
            break;

        case PDP_PROTOCOL_ERROR_UNSPECIFIED:
            cause = BCM_PDP_FAIL_PROTOCOL_ERRORS;
            break;

        default:
            cause = BCM_PDP_FAIL_ERROR_UNSPECIFIED;
            break;
    }
    return (int)cause;
}

static UInt8 GetFreePdpContext(void)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_RIL_PDP_CNTXS; i++)
    {
        if (0 == pdp_resp[i].cid)
        {
            pdp_resp[i].cid = RIL_PDP_CID(i);;
            KRIL_DEBUG(DBG_INFO, "GetFreePdpContext[%d]=%d \n", i, pdp_resp[i].cid);
            return i;
        }
    }
    return BCM_NET_MAX_RIL_PDP_CNTXS;
}


static UInt8 ReleasePdpContext(UInt8 cid)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_RIL_PDP_CNTXS; i++)
    {
        if (cid == pdp_resp[i].cid)
        {
            pdp_resp[i].cid = 0;
            pdp_resp[i].active = 0;
            //memset(&pdp_resp[i], 0, sizeof(KrilDataCallResponse_t));
            KRIL_DEBUG(DBG_INFO, "ReleasePdpContext[%d]=%d \n", i, pdp_resp[i].cid);
            return i;
        }
    }
    return BCM_NET_MAX_RIL_PDP_CNTXS;
}


static void FillDataResponseTypeApn(UInt8 cid, char* type, char* apn)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_RIL_PDP_CNTXS; i++)
    {
        if (cid == pdp_resp[i].cid)
        {
            if (NULL != type)
                 strncpy(pdp_resp[i].type, type, MIN(strlen(type), PDP_TYPE_LEN_MAX));
            if (NULL != apn)
                strncpy(pdp_resp[i].apn, apn, MIN(strlen(apn), PDP_APN_LEN_MAX));
            KRIL_DEBUG(DBG_INFO, "FillDataResponseTypeApn[%d]=[%s][%s] \n", i, pdp_resp[i].type, pdp_resp[i].apn);
            return;
        }
    }
}


static void FillDataResponseAddress(UInt8 cid, char* address)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_RIL_PDP_CNTXS; i++)
    {
        if (cid == pdp_resp[i].cid)
        {
            pdp_resp[i].active = 2; // 0=inactive, 1=active/physical link down, 2=active/physical link up
            if (NULL != address)
                strcpy(pdp_resp[i].address, address);
            KRIL_DEBUG(DBG_INFO, "FillDataResponseAddress[%d]=[%s] \n", i, pdp_resp[i].address);
            return;
        }
    }
}


void KRIL_SetupPdpHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
    static KrilPdpContext_t gContext;
    UInt8 i;

    if (NULL != capi2_rsp)
        KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::result:0x%x\n", capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            UInt8 pindex;
            char chPdpType[PDP_TYPE_LEN_MAX] = "IP";

            if (NULL == pdata->ril_cmd->data)
            {
                KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail with NULL data\n");
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            memcpy(&gContext, (KrilPdpContext_t *)(pdata->ril_cmd->data), sizeof(KrilPdpContext_t));

            if (gContext.apn != NULL)
            {
                for (i=0; i<BCM_NET_MAX_RIL_PDP_CNTXS; i++)
                {
                    if (strcmp(gContext.apn, pdp_resp[i].apn) == 0)
                    {
                        if (pdp_resp[i].active == 2) // connected
                        {
                            KrilPdpData_t *rdata;
                            KRIL_DEBUG(DBG_ERROR, "KRIL_SetupPdpHandler - Data already setup : apn %s \n", gContext.apn);                    
                            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilPdpData_t), GFP_KERNEL);
                            pdata->rsp_len = sizeof(KrilPdpData_t);
                            rdata = pdata->bcm_ril_rsp;

                            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);

                            if (pdp_resp[i].address != NULL)
                            {
                                memcpy(rdata->pdpAddress, pdp_resp[i].address, PDP_ADDRESS_LEN_MAX);
                            }

                            if (pdp_resp[i].apn != NULL)
                            {
                                memcpy(rdata->apn, pdp_resp[i].apn, PDP_APN_LEN_MAX);
                            }
                            rdata->cid = pdp_resp[i].cid;
                            
                            pdata->result = BCM_E_SUCCESS;
                            pdata->handler_state = BCM_FinishCAPI2Cmd;
                            return;
                        }
                        else if (pdp_resp[i].active == 3)// connecting
                        {
                            KRIL_DEBUG(DBG_ERROR, "KRIL_SetupPdpHandler - Ignore due to state(Connecting) : apn %s \n", gContext.apn);                    
                            pdata->result = BCM_E_GENERIC_FAILURE;
                            pdata->handler_state = BCM_ErrorCAPI2Cmd;
                            return;
                        }
                    }
                }
            }

            if (BCM_NET_MAX_RIL_PDP_CNTXS == (pindex = GetFreePdpContext()))
            {
                KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail with over max cid[%d]\n", pindex);
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            KRIL_DEBUG(DBG_INFO, "KRIL_SetupPdpHandler - Set PDP Context : apn %s \n", gContext.apn);

            {
                ClientInfo_t clientInfo;
                UInt8 numParms = (gContext.apn == NULL)?2:3;
                CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                gContext.cid = pdp_resp[pindex].cid;
                FillDataResponseTypeApn(gContext.cid, chPdpType, gContext.apn);
                KRIL_DEBUG(DBG_INFO,"**Calling CAPI2_PdpApi_SetPDPContext numParms %d type:%s apn:%s pindex %d cid %d\n",numParms, chPdpType, (gContext.apn==NULL)?"NULL":gContext.apn, pindex,pdp_resp[pindex].cid  );
            
				KRIL_SetInSetupPDPHandler(TRUE);
				
                CAPI2_PdpApi_SetPDPContext( &clientInfo, 
                                            pdp_resp[pindex].cid, 
                                            numParms, 
                                            chPdpType, 
                                            gContext.apn, 
                                            "", 
                                            0, 
                                            0);
            }
  
            pdp_resp[pindex].active = 3; //connecting

            pdata->handler_state = BCM_PDP_SetPdpContext;
        }
        break;

        case BCM_PDP_SetPdpContext:
        {
            PCHProtConfig_t t_PCHP;
            char *username = gContext.username;
            char *password = gContext.password;
            IPConfigAuthType_t t_Authtype = REQUIRE_PAP;/*default setting*/
            //UInt8 ContextID = 1;
            KRIL_DEBUG(DBG_INFO, "KRIL_SetupPdpHandler - Activate PDP context \n");

            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail to SetPDPContext[%d]\n", gContext.cid);
                ReleasePdpContext(gContext.cid);
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
				
				KRIL_SetInSetupPDPHandler(FALSE);
                break;
            }

            if (AUTH_PAP == gContext.authtype)
                t_Authtype = REQUIRE_PAP;
            else if (AUTH_CHAP == gContext.authtype)
                t_Authtype = REQUIRE_CHAP;

            memset(&t_PCHP, 0, sizeof(PCHProtConfig_t));
            // **FIXME** MAG - update this to use CAPI2_PCHEx_BuildIpConfigOptions() when api
            // is fixed in CIB CP (likely .30 release or so)
            Capi2BuildIpConfigOptions(&t_PCHP, username, password, t_Authtype);
            {
                ClientInfo_t clientInfo;
                CAPI2_InitClientInfo( &clientInfo, GetNewTID(), GetClientID());
                CAPI2_PchExApi_SendPDPActivateReq( &clientInfo, gContext.cid, ACTIVATE_MMI_IP_RELAY, &t_PCHP );
            }
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            UInt32 u_pDNS1, u_sDNS1, u_pDNS2, u_sDNS2, u_act_pDNS, u_act_sDNS;
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilPdpData_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilPdpData_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);

            KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail to SendPDPActivateReq[%d] \n", gContext.cid);
                ReleasePdpContext(gContext.cid);
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
				KRIL_SetInSetupPDPHandler(FALSE);
                break;
            }

            if(NULL != capi2_rsp->dataBuf)
            {
                PDP_SendPDPActivateReq_Rsp_t *rsp = (PDP_SendPDPActivateReq_Rsp_t *)capi2_rsp->dataBuf;
                KrilPdpData_t *rdata = pdata->bcm_ril_rsp;

                if((rsp->cause != RESULT_OK) || (rsp->response != PCH_REQ_ACCEPTED))
                {
                    KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail cause %d, resp(1 accept) %d, cid %d\r\n",
                        rsp->cause, rsp->response, rsp->activatedContext.cid);
                    ReleasePdpContext(gContext.cid);
                    rdata->cause = ParsePdpFailCause(rsp->cause);
                    pdata->result = BCM_E_RADIO_NOT_AVAILABLE;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
					
					KRIL_SetInSetupPDPHandler(FALSE);
                    break;
                }

                if (NULL != rsp->activatedContext.pdpAddress)
                {
                    memcpy(rdata->pdpAddress, rsp->activatedContext.pdpAddress, PDP_ADDRESS_LEN_MAX);
                    KRIL_DEBUG(DBG_INFO, "PDP Activate: PDP Address %s \r\n", rdata->pdpAddress);
                }

                u_pDNS1 = u_sDNS1 = u_pDNS2 = u_sDNS2 = u_act_pDNS = u_act_sDNS = 0;
                Capi2ReadDnsSrv(&(rsp->activatedContext.protConfig), &u_pDNS1, &u_sDNS1, &u_pDNS2, &u_sDNS2);

                KRIL_DEBUG(DBG_INFO, "PDP Activate: pDns1 0x%x, sDns1 0x%x, pDns2 0x%x, sDns2 0x%x \r\n",
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
                KRIL_DEBUG(DBG_INFO, "PDP Activate Resp - cid %d \n", rsp->activatedContext.cid);
                FillDataResponseAddress(rdata->cid, rdata->pdpAddress);

                for (i=0; i<BCM_NET_MAX_RIL_PDP_CNTXS; i++)
                {
                    if (rdata->cid == pdp_resp[i].cid)
                    {
                        if (pdp_resp[i].apn != NULL)
                        {
                            memcpy(rdata->apn, pdp_resp[i].apn, PDP_APN_LEN_MAX);
                        }
                        break;
                    }
                }

                pdata->result = BCM_E_SUCCESS;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
			
			KRIL_SetInSetupPDPHandler(FALSE);
        }
        break;

        default:
        {
			KRIL_SetInSetupPDPHandler(FALSE);
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}



void KRIL_DeactivatePdpHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (NULL != capi2_rsp)
        KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::result:0x%x\n", capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            ClientInfo_t clientInfo;
            char *cid = (char *)(pdata->ril_cmd->data);
            UInt8 ContextID = 0;
            UInt8 i;

            if (pdata->ril_cmd->datalen == 3)
            {
                ContextID = (UInt8)((*(cid) - 0x30) * 10 + (*(cid+1) - 0x30));
            }
            else
            {
                ContextID = (UInt8)(*cid - 0x30);
            }

            KRIL_DEBUG(DBG_INFO, "KRIL_DeactivatePdpHandler - length %d, Cid:%d \n", pdata->ril_cmd->datalen, ContextID);
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilPdpData_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilPdpData_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            for (i=0 ; i<BCM_NET_MAX_RIL_PDP_CNTXS ; i++)
            {
                if (ContextID == pdp_resp[i].cid) 
                {
                    // found the active context we're looking for...
                    KRIL_DEBUG(DBG_INFO, "ReleasePdpContext[%d]=%d \n", i, pdp_resp[i].cid);
                    break;
                }
                else if ((BCM_NET_MAX_RIL_PDP_CNTXS-1) == i) 
                {
                    // no match, so we assume context has already been deactivated...
                    KrilPdpData_t* rspData = pdata->bcm_ril_rsp;
                    // pass the cid back down to URIL; will be required for shutting down
                    // network interface
                    rspData->cid = ContextID;
                    KRIL_DEBUG(DBG_INFO, "no active context with cid %d \n", ContextID);
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                    return;
                }
             }

                CAPI2_InitClientInfo( &clientInfo, GetNewTID(), GetClientID());
                CAPI2_PchExApi_SendPDPDeactivateReq( &clientInfo, ContextID );

            ReleasePdpContext(ContextID);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
        	   KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "PDPDeActivate Fail to SendPDPDeActivateReq \n");
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            if(NULL != capi2_rsp->dataBuf)
            {
                PDP_SendPDPDeactivateReq_Rsp_t *rsp = (PDP_SendPDPDeactivateReq_Rsp_t *)capi2_rsp->dataBuf;
                KrilPdpData_t *rdata = pdata->bcm_ril_rsp;
                if(rsp->response != PCH_REQ_ACCEPTED)
                {
                    KRIL_DEBUG(DBG_ERROR, "PDPDeActivate Fail resp(1 accept) %d, cid %d\r\n", rsp->response, rsp->cid);
                    pdata->result = BCM_E_RADIO_NOT_AVAILABLE;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    break;
                }

                rdata->cid = rsp->cid;
                KRIL_DEBUG(DBG_INFO, "PDP Deactivate Resp - cid %d \n", rsp->cid);

                pdata->result = BCM_E_SUCCESS;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

void KRIL_DataStateHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
	static KrilDataState_t gDataState;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

	switch(pdata->handler_state)
    {
	case BCM_SendCAPI2Cmd:
	{
		UInt16 tid_test;	
        ClientInfo_t clientInfo;

		if (NULL == pdata->ril_cmd->data)
            {
                KRIL_DEBUG(DBG_ERROR, "Enter Data State Fail with NULL data\n");
            pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }
		tid_test = GetNewTID();
        CAPI2_InitClientInfo( &clientInfo, tid_test, GetClientID());
		memcpy(&gDataState, (KrilDataState_t *)(pdata->ril_cmd->data), sizeof(KrilDataState_t));
		CAPI2_PdpApi_GetPCHContextState(&clientInfo, gDataState.cid);
		pdata->handler_state = BCM_PDP_Verify;
	}
	break;

	case BCM_PDP_Verify:
        {
		UInt16 tid_test = BCM_TID_INIT;	
		 KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
		 if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "Fail to send Enter Data State \n");
            pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }	 
		 if(NULL != capi2_rsp->dataBuf)
            {
		PCHContextState_t *rsp = (PCHContextState_t *)capi2_rsp->dataBuf;		
            ClientInfo_t clientInfo;

		KRIL_DEBUG(DBG_INFO, "[BCM_PDP_Verify] - rsp:: %d  *rsp:: %d \n", rsp, *rsp);
		if((gDataState.cid != NULL) && (*rsp== CONTEXT_UNDEFINED))
            {
            	   KRIL_DEBUG(DBG_ERROR, "[BCM_PDP_Verify]::CONTEXT_UNDEFINED\n");
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }
		tid_test = GetNewTID();
                CAPI2_InitClientInfo( &clientInfo, tid_test, GetClientID());
                CAPI2_PdpApi_DeactivateSNDCPConnection( &clientInfo, gDataState.cid );
            }
		  KRIL_DEBUG(DBG_INFO, "New tid_test is %d\n", tid_test);
		 pdata->handler_state = BCM_RESPCAPI2Cmd;
	 	}
	    
	    break;

	case BCM_RESPCAPI2Cmd:
    {
		  KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "KRIL_DataStateHandler - Fail to send Enter Data State \n");
            pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }	
		else
		{
            KRIL_DEBUG(DBG_INFO, "Enter data state Successful\r\n");
            pdata->result = BCM_E_SUCCESS;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
			}
        }
	break;
	
	default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
	}
}

void KRIL_SendDataHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
	static KrilSendData_t gSendData;
	KRIL_DEBUG(DBG_INFO, "KRIL_SendDataHandler Entered \n");

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

	switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
		UInt32 tid_test;	
        ClientInfo_t clientInfo;

		KRIL_DEBUG(DBG_INFO, "KRIL_SendDataHandler Entered::BCM_SendCAPI2Cmd \n");
		
    		if (NULL == pdata->ril_cmd->data)
            {
                KRIL_DEBUG(DBG_ERROR, "Send Data Fail with NULL data\n");
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

		memcpy(&gSendData, (KrilSendData_t *)(pdata->ril_cmd->data), sizeof(KrilSendData_t));

		KRIL_DEBUG(DBG_INFO, "KRIL_SendDataHandler - Send Data : CID %d \n", gSendData.cid);
//		KRIL_DEBUG(DBG_ERROR, "KRIL_SendDataHandler - Send Data : NumberofBytes %d \n", gSendData.numberBytes);
		
		tid_test = GetNewTID();
        CAPI2_InitClientInfo(&clientInfo, tid_test, GetClientID());
		CAPI2_PdpApi_GetPCHContextState(&clientInfo,gSendData.cid);
		  KRIL_DEBUG(DBG_INFO, "My new tid_test is %d\n", tid_test);
		 pdata->handler_state = BCM_PDP_Verify;
        }
		break;

	case BCM_PDP_Verify:
        {
		UInt32 tid_test;	
            ClientInfo_t clientInfo;
		 KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
		 if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "KRIL_DataStateHandler - Fail to send Enter Data State \n");
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }	 

		 if(NULL != capi2_rsp->dataBuf)
            {
		PCHContextState_t *rsp = (PCHContextState_t *)capi2_rsp->dataBuf;		

		KRIL_DEBUG(DBG_INFO, "[BCM_PDP_Verify] - rsp:: %d  *rsp:: %d \n", rsp, *rsp);
		if((gSendData.cid != NULL) && (*rsp== CONTEXT_UNDEFINED))
            {
            	   KRIL_DEBUG(DBG_ERROR, "[BCM_PDP_Verify]::CONTEXT_UNDEFINED\n");
//             KRIL_DEBUG(DBG_ERROR, "%d CID not supported\n", gDataState.cid);
                    pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }
		tid_test = GetNewTID();
                CAPI2_InitClientInfo(&clientInfo, tid_test, GetClientID());
                CAPI2_PdpApi_SendTBFData(&clientInfo, gSendData.cid, gSendData.numberBytes);
		  KRIL_DEBUG(DBG_INFO, "My new tid_test is %d\n", tid_test);
		 pdata->handler_state = BCM_RESPCAPI2Cmd;
	 	}
        }
	break;
	
	case BCM_RESPCAPI2Cmd:
        {
		  KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "KRIL_SendDataHandler - Fail to send data \n");
                pdata->result = BCM_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }	
		else
		{
			 KRIL_DEBUG(DBG_ERROR, "KRIL_DataStateHandler - RESULT_OK-> result:0x%x\n\n",  capi2_rsp->result);		
		}
            pdata->result = BCM_E_SUCCESS;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
	  }	
	break;
	}
}
		



