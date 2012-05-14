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
#include "capi2_lcs_cplane_api.h"
#include "bcm_kril.h"
#include "bcm_cp_cmd_handler.h"

#ifdef BRCM_AGPS_CONTROL_PLANE_ENABLE

void KRIL_AgpsSendUpLinkHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_ERROR, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
    
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            AgpsCp_Data *tdata = (AgpsCp_Data *)pdata->ril_cmd->data;
			//LcsClientInfo_t lcsClientInfo = {0};
			ClientInfo_t lcsClientInfo;
			CAPI2_InitClientInfo(&lcsClientInfo, GetNewTID(), GetClientID());
            
            if ((tdata->cPlaneData) && (tdata->cPlaneDataLen))
            {
				//KRIL_DEBUG(DBG_ERROR,"AGPS DATA: pro=%d, len=%d, data 0x%lx, 0x%1x, 0x%1x, 0x%1x\n", tdata->protocol, tdata->cPlaneDataLen, tdata->cPlaneData[0], tdata->cPlaneData[1], tdata->cPlaneData[2], tdata->cPlaneData[3]);
				KRIL_DEBUG(DBG_ERROR,"AGPS DATA: pro=%d, len=%d, data 0x%lx\n", tdata->protocol, tdata->cPlaneDataLen, tdata->cPlaneData[0]);
				if(tdata->protocol == AGPS_PROC_RRLP)
				{

					//CAPI2_LCS_SendRrlpDataToNetwork(GetNewTID(), GetClientID(), lcsClientInfo, tdata->cPlaneData, tdata->cPlaneDataLen);
					CAPI2_LcsApi_RrlpSendDataToNetwork(&lcsClientInfo, tdata->cPlaneData, tdata->cPlaneDataLen);
				}
				else if(tdata->protocol == AGPS_PROC_RRC)
				{
					CAPI2_LcsApi_RrcSendUlDcch(&lcsClientInfo, tdata->cPlaneData, tdata->cPlaneDataLen);
				}
				else
				{
					KRIL_DEBUG(DBG_ERROR,"Unknow UL protocol handler_state:0x%lX\n", pdata->handler_state);
					pdata->handler_state = BCM_ErrorCAPI2Cmd;
					break;
				}
				pdata->handler_state = BCM_RESPCAPI2Cmd;
			}
			else
			{
				KRIL_DEBUG(DBG_ERROR,"Invalid UL data handler_state:0x%lX\n", pdata->handler_state);
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
			}
			
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_ERROR, "BCM_RESPCAPI2Cmd\n");
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

LcsRrcMcFailure_t KRIL_Cp2LcsRrcMcFailure(AgpsCp_RrcMcFailureCode inMcFailure)
{
	LcsRrcMcFailure_t lcsMcFailure = LCS_RRC_unsupportedMeasurement;

	switch(inMcFailure)
	{
	case AGPS_RRC_configurationUnsupported:
		lcsMcFailure = LCS_RRC_configurationUnsupported;
		break;
	case AGPS_RRC_unsupportedMeasurement:
		lcsMcFailure = LCS_RRC_unsupportedMeasurement;
		break;
	case AGPS_RRC_invalidConfiguration:
		lcsMcFailure = LCS_RRC_invalidConfiguration;
		break;
	}
	return lcsMcFailure;
}

void KRIL_AgpsRrcMcFailureHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_ERROR, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
			AgpsCp_McFailure *tdata = (AgpsCp_McFailure *)pdata->ril_cmd->data;
			//LcsClientInfo_t lcsClientInfo = {0};
			ClientInfo_t lcsClientInfo;
			CAPI2_InitClientInfo(&lcsClientInfo, GetNewTID(), GetClientID());

			//CAPI2_LCS_RrcMeasurementControlFailure(GetNewTID(), GetClientID(), lcsClientInfo, tdata->transId, KRIL_Cp2LcsRrcMcFailure(tdata->failureCode), 0);				
			CAPI2_LcsApi_RrcMeasCtrlFailure(&lcsClientInfo, tdata->transId, KRIL_Cp2LcsRrcMcFailure(tdata->failureCode), 0);

            KRIL_DEBUG(DBG_ERROR,"transId:%d\n", tdata->transId);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_ERROR, "BCM_RESPCAPI2Cmd\n");
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

LcsRrcMcStatus_t KRIL_Cp2LcsRrcStatus(AgpsCp_RrcStatus inRrcStatus)
{
	LcsRrcMcStatus_t lcsMcStatus = LCS_RRC_asn1_ViolationOrEncodingError;

	switch(inRrcStatus)
	{
	case AGPS_RRC_STATUS_asn1_ViolationOrEncodingError:
		lcsMcStatus = LCS_RRC_asn1_ViolationOrEncodingError;
		break;
	case AGPS_RRC_STATUS_messageTypeNonexistent:
		lcsMcStatus = LCS_RRC_messageTypeNonexistent;
		break;
	}
	return lcsMcStatus;
}

void KRIL_AgpsRrcStatusHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_ERROR, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
			AgpsCp_RrcStatus *tdata = (AgpsCp_RrcStatus *)pdata->ril_cmd->data;
			//LcsClientInfo_t lcsClientInfo = {0};
			ClientInfo_t lcsClientInfo = {0};
			CAPI2_InitClientInfo(&lcsClientInfo, GetNewTID(), GetClientID());

			CAPI2_LcsApi_RrcStatus(&lcsClientInfo, KRIL_Cp2LcsRrcStatus(*tdata));
			//CAPI2_LCS_RrcStatus(GetNewTID(), GetClientID(), lcsClientInfo, KRIL_Cp2LcsRrcStatus(*tdata));
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_ERROR, "BCM_RESPCAPI2Cmd\n");
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

#endif //BRCM_AGPS_CONTROL_PLANE_ENABLE

