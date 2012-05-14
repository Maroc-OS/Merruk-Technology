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
#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"
#include "capi2_gen_cp_api.h"

#include <linux/broadcom/bcm_fuse_sysparm_CIB.h>
#include "capi2_cc_api.h"
#include "netreg_def.h"
#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"
#include <linux/reboot.h>	 /* For kernel_power_off() */


//Irine_22June_airplanemode
extern char satk_setup_menu_tlv_data_string[258 * 2 + 1];
extern UInt16 satk_setup_menu_tlv_length;

UInt8 g_RSSIThreshold = 7;

// Assume flight mode is on unless Android framework requests to set radio power
Boolean gIsFlightModeOnBoot = TRUE;

// In STK refresh reset case, it need to turn off then turn on SIM card.
Boolean gIsStkRefreshReset = FALSE;

// For STK
//UInt8 terminal_profile_data[17] = {0xFF,0xDF,0xFF,0xFF,0x1F,0x80,0x00,0xDF,0xDF,0x00,0x00,
//                                      0x00,0x00,0x10,0x20,0xA6,0x00};
UInt8 terminal_profile_data[30] = {0xFF,0xFF,0xFF,0xFF,0xFF,0x81,0x00,0xDF,0xFF,0x00,0x00,
                                      0x00,0x00,0x10,0x20,0xA6,0x00,0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // gearn TP fixed


// IMEI information
#ifdef CONFIG_BRCM_SIM_SECURE_ENABLE
static Boolean ProcessImei(UInt8* imeiStr, UInt8* imeiVal);
#endif // CONFIG_BRCM_SIM_SECURE_ENABLE

UInt8 sImei_Info[BCD_IMEI_LEN] = {0};

static void ParseIMSIData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp);
static void ParseIMEIData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp);

// number of characters in IMEI string (15 digit IMEI plus check digit, plus 1 for null termination)
#define IMEI_STRING_LEN (IMEI_DIGITS+1)

#define isdigit(c) ((c) >= '0' && (c) <= '9')

// external utility function to convert RIL network type value to equivalent RATSelect_t value
extern RATSelect_t ConvertNetworkType(int type);

//++ JSHAN Attach for next power on
extern UInt8 vGprsAttachMode;
//-- JSHAN Attach for next power on

// external utility functions to convert RIL band mode to set of BandSelect_t values
extern UInt32 ConvertBandMode(int mode);

void KRIL_InitCmdHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
    ClientInfo_t clientInfo;
    CAPI2_MS_Element_t data;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        
        if(capi2_rsp->result != RESULT_OK)
        {
           pdata->handler_state = BCM_ErrorCAPI2Cmd;
        }
    }

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilInit_t *pInitData = (KrilInit_t *)(pdata->ril_cmd->data);

            // if there is a valid IMEI, make appropriate CAPI2 call to set
            // IMEI on CP, otherwise fall through to next init command
            if (pInitData->is_valid_imei)
            {
                KRIL_DEBUG(DBG_INFO, "OTP IMEI:%s\n", pInitData->imei);

#ifdef CONFIG_BRCM_SIM_SECURE_ENABLE
                    // Record IMEI1 infomation  
                    if (FALSE == ProcessImei((UInt8*)pInitData->imei, sImei_Info))
                    {
                        KRIL_DEBUG(DBG_ERROR,"Process IMEI:%s Failed!!!", pInitData->imei);
                        pdata->handler_state = BCM_ErrorCAPI2Cmd;
                        kernel_power_off();
                    }
#endif //CONFIG_BRCM_SIM_SECURE_ENABLE

                memset(&data, 0, sizeof(CAPI2_MS_Element_t));
                memcpy(data.data_u.imeidata, pInitData->imei, IMEI_DIGITS);
                data.inElemType = MS_LOCAL_PHCTRL_ELEM_IMEI;
                CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                CAPI2_MsDbApi_SetElement(&clientInfo, &data);
                pdata->handler_state = BCM_SMS_ELEM_CLIENT_HANDLE_MT_SMS;
                break;
            }
#ifdef CONFIG_BRCM_SIM_SECURE_ENABLE 
            else
            {
                // For secure boot, the IMEI is important inform for verifying SIM lock data.
                KRIL_DEBUG(DBG_ERROR, "IMEI is invalid. Error!!!\n");
                kernel_power_off();
            }
#endif //CONFIG_BRCM_SIM_SECURE_ENABLE 

            // if OTP IMEI passed from URIL is not valid, we skip the
            // CAPI2_MS_SetElement() call and fall through to execute the
            // next CAPI2 init call instead...
        }

        case BCM_SMS_ELEM_CLIENT_HANDLE_MT_SMS:
        {
            memset((UInt8*)&data, 0, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SMS_ELEM_CLIENT_HANDLE_MT_SMS;
            data.data_u.bData = TRUE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_SMS_SetSmsReadStatusChangeMode;
            break;
        }

        case BCM_SMS_SetSmsReadStatusChangeMode:
        {
            CAPI2_SMS_SetSmsReadStatusChangeMode(GetNewTID(), GetClientID(), FALSE);
            pdata->handler_state = BCM_SYS_SetFilteredEventMask;
            break;
        }

        case BCM_SYS_SetFilteredEventMask:
        {
            UInt16 filterList[]={MSG_RSSI_IND, MSG_CELL_INFO_IND, MSG_LCS_RRC_UE_STATE_IND, 
                                 MSG_DATE_TIMEZONE_IND, MSG_DATA_SUSPEND_IND, 
                                 MSG_DATA_RESUME_IND, MSG_CAPI2_AT_RESPONSE_IND, 
                                 MSG_UE_3G_STATUS_IND};
            CAPI2_SYS_SetFilteredEventMask(GetNewTID(), GetClientID(), &filterList[0], sizeof(filterList)/sizeof(UInt16), SYS_AP_DEEP_SLEEP_MSG_FILTER);
            pdata->handler_state = BCM_SYS_SetRssiThreshold;
            break;
        }

        case BCM_SYS_SetRssiThreshold:
        {
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_PhoneCtrlApi_SetRssiThreshold(&clientInfo, g_RSSIThreshold, 20, g_RSSIThreshold, 20);
            pdata->handler_state = BCM_TIMEZONE_SetTZUpdateMode;
            break;
        }

        case BCM_TIMEZONE_SetTZUpdateMode:
        {
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_NetRegApi_SetTZUpdateMode(&clientInfo, TIMEZONE_UPDATEMODE_NO_TZ_UPDATE);
            pdata->handler_state = BCM_SATK_SetTermProfile;
            break;
        }

        case BCM_SATK_SetTermProfile:
        {
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_SatkApi_SetTermProfile(&clientInfo, terminal_profile_data,
                sizeof(terminal_profile_data)/sizeof(UInt8));
            pdata->handler_state = BCM_SATK_SETUP_CALL_CTR;
            break;
        }

        case BCM_SATK_SETUP_CALL_CTR:
        {
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SATK_ELEM_SETUP_CALL_CTR;
#ifdef OEM_RIL_ENABLE
            data.data_u.bData = FALSE;
            pdata->handler_state = BCM_SATK_SEND_SS_CTR;
#else
            data.data_u.bData = TRUE;
            pdata->handler_state = BCM_SS_SET_ENABLE_OLD_SS_MSG;
#endif
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_SATK_ICON_DISP_SUPPORTED;
            break;
        }

        case BCM_SATK_ICON_DISP_SUPPORTED:
        {
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SATK_ELEM_ICON_DISP_SUPPORTED;
            data.data_u.bData = TRUE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
#ifdef OEM_RIL_ENABLE
			pdata->handler_state = BCM_SATK_SEND_SS_CTR;  
#else
            pdata->handler_state = BCM_SS_SET_ENABLE_OLD_SS_MSG;            
#endif
            break;
        }

#ifdef OEM_RIL_ENABLE
        case BCM_SATK_SEND_SS_CTR:
        {
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SATK_ELEM_SEND_SS_CTR;
            data.data_u.bData = FALSE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_SATK_SEND_USSD_CTR;
            break;
        }
        case BCM_SATK_SEND_USSD_CTR:
        {
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SATK_ELEM_SEND_USSD_CTR;
            data.data_u.bData = FALSE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_SATK_SEND_SMS_CTR;
            break;
        }
        case BCM_SATK_SEND_SMS_CTR:
        {
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SATK_ELEM_SEND_SMS_CTR;
            data.data_u.bData = FALSE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_SATK_SEND_ENABLE_7BIT_CONVERSIONS;
            break;
        }

        case BCM_SATK_SEND_ENABLE_7BIT_CONVERSIONS:
        {
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SATK_ELEM_ENABLE_7BIT_CONVERSIONS;
            data.data_u.bData = FALSE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_SATK_SEND_SETUP_EVENT_LIST_CTR;
            break;
        }
        case BCM_SATK_SEND_SETUP_EVENT_LIST_CTR:
        {
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SATK_ELEM_SETUP_EVENT_LIST_CTR;
            data.data_u.bData = FALSE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_CFG_SIM_LOCK_SUPPORTED;
            break;
        }

         case BCM_CFG_SIM_LOCK_SUPPORTED:
        {
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_CFG_ELEM_SIM_LOCK_SUPPORTED ;
            data.data_u.bData = TRUE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_SS_SET_ENABLE_OLD_SS_MSG;
            break;
        }


#endif


        case BCM_SS_SET_ENABLE_OLD_SS_MSG:
        {
            // enabled sending of "old" supp svcs messages
            // NOTE: this should go away when we move to the new SS apis
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SS_ELEM_ENABLE_OLD_SS_MSG;
            data.data_u.u8Data = 1;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_SS_SET_ELEM_FDN_CHECK;
            break;
        }
                
        case BCM_SS_SET_ELEM_FDN_CHECK:
        {
            // enable FDN check for SS dialing
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SS_ELEM_FDN_CHECK;
            data.data_u.u8Data = 1;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_SetElement(&clientInfo, &data);
            pdata->handler_state = BCM_SET_SupportedRATandBand;
            break;
        }

        case BCM_SET_SupportedRATandBand:
        {
            KrilInit_t *pInitData = (KrilInit_t *)(pdata->ril_cmd->data);
            KRIL_SetPreferredNetworkType(pInitData->networktype);
            KRIL_SetBandMode(pInitData->band);
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            KRIL_DEBUG(DBG_INFO,"BCM_SET_SupportedRATandBand network type %d\n", pInitData->networktype);
            KRIL_DEBUG(DBG_INFO,"BCM_SET_SupportedRATandBand band %d conv band %d\n", pInitData->band, ConvertBandMode(pInitData->band));
            //TODO jw check this new api last two extra parameter.
            CAPI2_NetRegApi_SetSupportedRATandBand(&clientInfo, ConvertNetworkType(pInitData->networktype), ConvertBandMode(pInitData->band), ConvertNetworkType(pInitData->networktype), ConvertBandMode(pInitData->band) );

            //++ JSHAN Attach for next power on
		vGprsAttachMode = pInitData->gprs_attach_init;
		if (vGprsAttachMode == 1 || vGprsAttachMode == 2)
			pdata->handler_state = BCM_SET_AttachMode;
		else 
			pdata->handler_state = BCM_SET_RADIO_OFF;
		//-- JSHAN Attach for next power on
            break;
        }

	//++ JSHAN Attach for next power on
	case BCM_SET_AttachMode:
	{
		CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
		data.inElemType = MS_LOCAL_PHCTRL_ELEM_ATTACH_MODE;
            data.data_u.u8Data = vGprsAttachMode;
		CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
		CAPI2_MsDbApi_SetElement ( &clientInfo,&data);
            pdata->handler_state = BCM_SET_RADIO_OFF;
            break;
        }
	//-- JSHAN Attach for next power on

        case BCM_SET_RADIO_OFF:
        {
            // For flight mode power up battery ADC & deep sleep issue (MobC00131482), set the initial CP state to RADIO_OFF.
            // If MS is powered up in normal mode, Android framework will send BRCM_RIL_REQUEST_RADIO_POWER to RIL.
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_PhoneCtrlApi_ProcessNoRfReq(&clientInfo);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }
                
        case BCM_RESPCAPI2Cmd:
        {
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


/*
     this function is implemented for meeting customer's requirement:
     If SIM card is locked, the PIN is required when the Airplane mode is turned off.
     But the PIN is NOT required  if using CAPI2_SYS_ProcessPowerUpReq to turned off Airplane mode.
     Workaround: Power off and then on the sim card, the CP will do the unlock sim process.
*/
void KRIL_RadioPowerHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
    ClientInfo_t clientInfo;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int *OnOff = (int *)(pdata->ril_cmd->data);

            KRIL_DEBUG(DBG_INFO, "On-Off:%d\n", *OnOff);

			if(gIsStkRefreshReset == TRUE){
				
				if(*OnOff == 0){
					KRIL_DEBUG(DBG_INFO, "Power off Sim card - Refresh\n");
                    CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                    CAPI2_SimApi_PowerOnOffCard (&clientInfo, FALSE, SIM_POWER_ON_INVALID_MODE);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;

				}else{
					KRIL_DEBUG(DBG_INFO, "Power on Sim card - Refresh\n");
					CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
					CAPI2_SimApi_PowerOnOffCard (&clientInfo, TRUE, SIM_POWER_ON_NORMAL_MODE);
					pdata->handler_state = BCM_RESPCAPI2Cmd;
					gIsStkRefreshReset = FALSE;
				}
				break;

			}
			else {

            	if (*OnOff == 1)
            	{
                    CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                	CAPI2_PhoneCtrlApi_ProcessPowerUpReq(&clientInfo);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
            	}
				else{
                	CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                	CAPI2_PhoneCtrlApi_ProcessNoRfReq(&clientInfo);
                	pdata->handler_state = BCM_RESPCAPI2Cmd;
            	}
				break;
			}
        }

        case BCM_RESPCAPI2Cmd:
        {
	     //Irine_22June_airplanemode
            int *OnOff = (int *)(pdata->ril_cmd->data);

            KRIL_DEBUG(DBG_TRACE, "handler state:%lu\n", pdata->handler_state);
            pdata->bcm_ril_rsp = NULL;
            pdata->rsp_len = 0;
            pdata->handler_state = BCM_FinishCAPI2Cmd;
			
            KRIL_DEBUG(DBG_INFO, "On-Off:%d\n", *OnOff);
            if (1 == *OnOff)
            {
            	  KRIL_DEBUG(DBG_ERROR, "KRIL_RadioPowerHandler: Offline off\n");
            	  KRIL_DEBUG(DBG_ERROR, "satk_setup_menu_tlv_data_string: %s\n",satk_setup_menu_tlv_data_string);
            	  KRIL_DEBUG(DBG_ERROR, "satk_setup_menu_tlv_length: %d\n",satk_setup_menu_tlv_length);
	         if(satk_setup_menu_tlv_length!=0)
                	KRIL_SendNotify(BRCM_RIL_UNSOL_STK_PROACTIVE_COMMAND, satk_setup_menu_tlv_data_string, (satk_setup_menu_tlv_length * 2 + 1));
            }

            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }

    if (pdata->handler_state == BCM_ErrorCAPI2Cmd || pdata->handler_state == BCM_FinishCAPI2Cmd)
    {
        /* allow to send the radio state change notification to Android framework when request is done. */
        gIsFlightModeOnBoot = FALSE;
    }
}


void KRIL_SetTTYModeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int *mode = (int *)(pdata->ril_cmd->data);
            KRIL_DEBUG(DBG_INFO, "mode:%d\n", *mode);
            CAPI2_CC_SetTTYCall(GetNewTID(), GetClientID(), (Boolean) *mode);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_QueryTTYModeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(krilQueryTTYModeType_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(krilQueryTTYModeType_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_CC_IsTTYEnable(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {

            if(capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                Boolean mode = *(Boolean *)capi2_rsp->dataBuf;
                krilQueryTTYModeType_t *rdata = (krilQueryTTYModeType_t *)pdata->bcm_ril_rsp;
                rdata->mode = (int)mode;
                KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd:: rdata->mode:%d mode:%d\n", rdata->mode, mode);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_BasebandVersionHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(krilQueryBaseBandVersion_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(krilQueryBaseBandVersion_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_SYSPARM_GetSWVersion(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            UInt8 *version = (UInt8 *)capi2_rsp->dataBuf;
            UInt32 rdata_len = pdata->rsp_len;
            krilQueryBaseBandVersion_t *rdata = (krilQueryBaseBandVersion_t *)pdata->bcm_ril_rsp;
            strncpy(rdata->version, (char *)version, MIN(rdata_len, strlen((char *)version)));
            KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd:: rdata->version:[%s] version:[%s]\n", (char *)rdata->version, (char *)version);
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

void KRIL_GetIMSIHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_SIM_GetIMSI(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            ParseIMSIData(pdata, capi2_rsp);
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


void KRIL_GetIMEIHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    ClientInfo_t clientInfo;
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_GetElement(&clientInfo, MS_LOCAL_PHCTRL_ELEM_IMEI);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            ParseIMEIData(pdata, capi2_rsp);
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


void KRIL_GetIMEISVHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    ClientInfo_t clientInfo;
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_GetElement(&clientInfo, MS_LOCAL_PHCTRL_ELEM_SW_VERSION);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            CAPI2_MS_Element_t *rsp = (CAPI2_MS_Element_t *) capi2_rsp->dataBuf;
            KrilImeiData_t *imeisv_result;
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilImeiData_t), GFP_KERNEL);
            imeisv_result = (KrilImeiData_t *)pdata->bcm_ril_rsp;
            memset(imeisv_result, 0, sizeof(KrilImeiData_t));
            pdata->rsp_len = sizeof(KrilImeiData_t);
            strcpy(imeisv_result->imeisv, rsp->data_u.u3Bytes);
            KRIL_DEBUG(DBG_INFO, "u3Bytes:[%s] imeisv:[%s]\n", rsp->data_u.u3Bytes, imeisv_result->imeisv);
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


void KRIL_GetDeviceIdentityHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    ClientInfo_t clientInfo;
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_GetElement(&clientInfo, MS_LOCAL_PHCTRL_ELEM_IMEI);
            pdata->handler_state = BCM_GetIMEIInfo;
            break;
        }

        case BCM_GetIMEIInfo:
        {
            ParseIMEIData(pdata, capi2_rsp);
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_MsDbApi_GetElement(&clientInfo, MS_LOCAL_PHCTRL_ELEM_SW_VERSION);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            CAPI2_MS_Element_t *rsp = (CAPI2_MS_Element_t *) capi2_rsp->dataBuf;
            KrilImeiData_t *imeisv_result = (KrilImeiData_t *)pdata->bcm_ril_rsp;
            strcpy(imeisv_result->imeisv, rsp->data_u.u3Bytes);
            KRIL_DEBUG(DBG_INFO, "u3Bytes:[%s] imeisv:[%s]\n", rsp->data_u.u3Bytes, imeisv_result->imeisv);
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

void KRIL_QuerySimEmergencyNumberHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::msgType:%d result:%d\n", pdata->handler_state, capi2_rsp->msgType, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(Kril_SIMEmergency), GFP_KERNEL);
            pdata->rsp_len = sizeof(Kril_SIMEmergency);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_PBK_SendInfoReq(GetNewTID(), GetClientID(), PB_EN);
            pdata->handler_state = BCM_PBK_SendInfoReq;
            break;
        }

        case BCM_PBK_SendInfoReq:
        {
            PBK_INFO_RSP_t *rsp = (PBK_INFO_RSP_t *) capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO,"total_entries:[%d] result:[%d]\n", rsp->total_entries, rsp->result);
            if (0 == rsp->total_entries ||FALSE == rsp->result)
            {
                Kril_SIMEmergency *rdata = (Kril_SIMEmergency *)pdata->bcm_ril_rsp;
                rdata->simAppType = KRIL_GetSimAppType();
                KRIL_SendNotify(BRIL_UNSOL_EMERGENCY_NUMBER, pdata->bcm_ril_rsp, pdata->rsp_len);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                CAPI2_PBK_SendReadEntryReq(GetNewTID(), GetClientID(), PB_EN, 0, (rsp->total_entries-1));
                pdata->handler_state = BCM_PBK_ReadENEnteryReq;
            }
            break;
        }

        case BCM_PBK_ReadENEnteryReq:
        {
            PBK_ENTRY_DATA_RSP_t *rsp = (PBK_ENTRY_DATA_RSP_t *) capi2_rsp->dataBuf;
            Kril_SIMEmergency *rdata = (Kril_SIMEmergency *)pdata->bcm_ril_rsp;
			
			if(rsp == NULL || rdata == NULL)
			{
				KRIL_DEBUG(DBG_INFO,"rsp or rdata is NULL. ERROR!!");
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
				return;
			}
			
            KRIL_DEBUG(DBG_INFO,"rsp->data_result:%d\n", rsp->data_result);

            if (rsp->data_result == PBK_ENTRY_VALID_IS_LAST || rsp->data_result == PBK_ENTRY_VALID_NOT_LAST)
            {
                KRIL_DEBUG(DBG_INFO,"simecclist:[%s] number:[%s] numlen:%d\n", rdata->simecclist, rsp->pbk_rec.number, strlen(rsp->pbk_rec.number));
                if (strlen(rsp->pbk_rec.number) != 0 && 
                    !(strcmp("112", rsp->pbk_rec.number) == 0 || strcmp("911", rsp->pbk_rec.number) == 0))
                {
                    if (strlen(rdata->simecclist) != 0)
                    {
                        sprintf(&rdata->simecclist[0], "%s%s%s", rdata->simecclist, ",", rsp->pbk_rec.number);
                    } 
                    else
                    {
                        strcpy(rdata->simecclist, rsp->pbk_rec.number);
                    }
                }
                if (rsp->data_result == PBK_ENTRY_VALID_IS_LAST)
                {
                    rdata->simAppType = KRIL_GetSimAppType();
                    KRIL_SendNotify(BRIL_UNSOL_EMERGENCY_NUMBER, pdata->bcm_ril_rsp, pdata->rsp_len);
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
            }
            else
            {
                rdata->simAppType = KRIL_GetSimAppType();
                KRIL_SendNotify(BRIL_UNSOL_EMERGENCY_NUMBER, pdata->bcm_ril_rsp, pdata->rsp_len);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
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


void KRIL_GetCurrentSimVoltageHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    ClientInfo_t clientInfo;
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    
    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }    
    
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
	          CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_SimApi_GetCurrentSimVoltage(&clientInfo);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            UInt8 *resp = NULL;
            SimVoltage_t *voltage = (SimVoltage_t*)capi2_rsp->dataBuf;
            
            if (!voltage)
            {
                KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            pdata->bcm_ril_rsp = kmalloc(sizeof(UInt8)*6, GFP_KERNEL);
            if (!pdata->bcm_ril_rsp)
            {
                KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            pdata->rsp_len = sizeof(UInt8)*6 ;
            
            resp = (UInt8*)pdata->bcm_ril_rsp;
            resp[0] = (UInt8)'B';
            resp[1] = (UInt8)'R';
            resp[2] = (UInt8)'C';
            resp[3] = (UInt8)'M';
            resp[4] = (UInt8)BRIL_HOOK_GET_SIM_VOLTAGE;
            resp[5] = (UInt8)*voltage;
            
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;                
    }    
}


void ParseIMSIData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp)
{
    IMSI_t* rsp = (IMSI_t*)capi2_rsp->dataBuf;

    KrilImsiData_t *imsi_result;

    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilImsiData_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }    

    imsi_result = pdata->bcm_ril_rsp;
    memset(imsi_result, 0, sizeof(KrilImsiData_t));
    pdata->rsp_len = sizeof(KrilImsiData_t);
    
    if ( capi2_rsp->result != RESULT_OK )
    {
        KRIL_DEBUG(DBG_ERROR,"IMSI: SIM access failed!! result:%d\n",capi2_rsp->result);
        imsi_result->result = BCM_E_GENERIC_FAILURE;
    }
    else
    {
        imsi_result->result = BCM_E_SUCCESS;
        KRIL_DEBUG(DBG_INFO,"IMSI:%s\n", (char*)rsp);
        strncpy(imsi_result->imsi, (char*)rsp, (IMSI_DIGITS+1));
    }
    
    pdata->handler_state = BCM_FinishCAPI2Cmd;
}


//******************************************************************************
// Function Name:      ParseIMEIData
//
// Description:        Internal helper function used to parse CAPI response
//                     to retrieval of IMEI from MS database. If retrieval
//                     from MS database fails, or IMEI stored there is all
//                     0's, we instead return the IMEI stored in sysparms
//                     (this mirrors behaviour of SYSPARM_GetImeiStr() on CP)
//
//******************************************************************************
static void ParseIMEIData(KRIL_CmdList_t* pdata, Kril_CAPI2Info_t* capi2_rsp)
{
    CAPI2_MS_Element_t* rsp = (CAPI2_MS_Element_t*)capi2_rsp->dataBuf;
    KrilImeiData_t* imei_result;

    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilImeiData_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }    

    imei_result = pdata->bcm_ril_rsp;
    memset(imei_result, 0, sizeof(KrilImeiData_t));
    pdata->rsp_len = sizeof(KrilImeiData_t);
    
    if (rsp->inElemType != MS_LOCAL_PHCTRL_ELEM_IMEI)
    {
        KRIL_DEBUG(DBG_ERROR,"IMEI: inElemType Error!! inElemType:%d\n",rsp->inElemType);
        imei_result->result = BCM_E_GENERIC_FAILURE;
    }
    else
    {
        // at this point, we have the MS database IMEI string; check if it is non-zero, and if not,
        // use IMEI from sysparms
        UInt8 i=0;
        Boolean bUseMSDBImei = FALSE;
        UInt8* pMSDBImeiStr = (UInt8*)rsp->data_u.imeidata;

        imei_result->result = BCM_E_SUCCESS;
        
        do
        {
            bUseMSDBImei = (pMSDBImeiStr[i] != '0');
        }
        while ( !bUseMSDBImei && (++i < IMEI_DIGITS) );
        
        if ( bUseMSDBImei )
        {
            // MS database IMEI is not all 0's, so we use it
            strncpy(imei_result->imei, pMSDBImeiStr, IMEI_DIGITS);
            imei_result->imei[IMEI_DIGITS] = '\0';
            KRIL_DEBUG(DBG_INFO,"Using MS DB IMEI:%s\n", pMSDBImeiStr );
        }
        else
        {
            // MS database IMEI is all 0's, so retrieve IMEI from sysparms
            UInt8 tmpImeiStr[IMEI_STRING_LEN];
            Boolean bFound;
            
            // retrieve null terminated IMEI string
            bFound = SYSPARM_GetImeiStr( tmpImeiStr );
            if ( bFound )
            {
                // got it from sysparms, so copy to the response struct
                KRIL_DEBUG(DBG_INFO,"Using sysparm IMEI:%s\n", tmpImeiStr );
                strncpy(imei_result->imei, tmpImeiStr, IMEI_STRING_LEN);
            }
            else
            {
                KRIL_DEBUG(DBG_INFO,"** SYSPARM_GetImeiStr() failed\n" );
                imei_result->result = BCM_E_GENERIC_FAILURE;
            }
        }
        
    }
    
    pdata->handler_state = BCM_FinishCAPI2Cmd;
}

//Irine_SIMInfo_SvcMenu
void KRIL_GetSimInfoHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_DEBUG(DBG_INFO,"KRIL_GetSimInfoHandler() pdata->handler_state:\n");

	
}


//--------------------------------------------------------------------------
// Function Name: ReadIMEI()
//
// Description:  Get IMEI information
//
// Return:  IMEI string
//      
//
//----------------------------------------------------------------------------
UInt8* ReadIMEI(void)
{
    return sImei_Info;
}


//--------------------------------------------------------------------------
// Function Name: ProcessImei()
//
// Description:  Convert the IMEI string to the format in our system parameter files
//
// Return:  TRUE - Convert is successful; FALSE - Convert is failed.
//      
//
//----------------------------------------------------------------------------
#ifdef CONFIG_BRCM_SIM_SECURE_ENABLE
static Boolean ProcessImei(UInt8* imeiStr, UInt8* imeiVal)
{
    int i ;
    
    for (i = 0; i < 14; i++)
    {
        if (!isdigit(imeiStr[i]))
        {
            return FALSE ;
        }
    }
    
    /* Use the format in our system parameter files: first nibble and last nibble are not used and
     * set to 0, giving rise to a total of 8 bytes. 
     */
    imeiVal[0] = ( imeiStr[0] - '0')  << 4;
    imeiVal[1] = ((imeiStr[2] - '0')  << 4)  | (imeiStr[1] - '0');
    imeiVal[2] = ((imeiStr[4] - '0')  << 4)  | (imeiStr[3] - '0');
    imeiVal[3] = ((imeiStr[6] - '0')  << 4)  | (imeiStr[5] - '0');
    imeiVal[4] = ((imeiStr[8] - '0')  << 4)  | (imeiStr[7] - '0');
    imeiVal[5] = ((imeiStr[10] - '0') << 4)  | (imeiStr[9] - '0');
    imeiVal[6] = ((imeiStr[12] - '0') << 4)  | (imeiStr[11] - '0');
    imeiVal[7] = imeiStr[13] - '0';
    
    return TRUE;
}
#endif // CONFIG_BRCM_SIM_SECURE_ENABLE
