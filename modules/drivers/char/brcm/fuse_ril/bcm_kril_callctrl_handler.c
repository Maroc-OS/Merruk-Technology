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
#include "capi2_cc_api.h"

#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"

static char DTMFString;
extern Boolean StkCall; //yihwa

static BRIL_LastCallFailCause ResultToRilFailCause(Result_t inResult);
extern UInt16 KRIL_USSDSeptet2Octet(UInt8 *p_src, UInt8 *p_dest, UInt16 num_of_Septets);
#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
extern void lcd_enter_during_call(void);
extern void lcd_restore_during_call(void);
#endif
static int g_totalMPTYCall = 0;

static UInt8 sLastJoinCallIndex = INVALID_CALL;

//******************************************************************************
//
// Function Name: KRIL_VoiceCallReleased
//
// Description: Called from notification handler when MSG_VOICECALL_RELEASE_IND 
//              notification received from CP.
//
//******************************************************************************
void KRIL_HandleVoiceCallReleased( UInt8 inReleasedCallIndex )
{
    KRIL_DEBUG(DBG_INFO, "KRIL_HandleVoiceCallReleased call index %d\n", inReleasedCallIndex);
    // check if the release call index matches the cached join call index
    if ( sLastJoinCallIndex == inReleasedCallIndex )
    {
        // yes, so reset the cached join call index value
        KRIL_DEBUG(DBG_INFO, "KRIL_HandleVoiceCallReleased sLastJoinCallIndex match, resetting\n");
        sLastJoinCallIndex = INVALID_CALL;
    }
    else
    {
        KRIL_DEBUG(DBG_INFO, "KRIL_HandleVoiceCallReleased sLastJoinCallIndex %d no match, ignoring\n", sLastJoinCallIndex);
    }
}

//******************************************************************************
//
// Function Name: GetCallStateForIndex
//
// Description: Internal helper function for KRIL_GetCurrentCallHandler to determine call
//              state for a given index; this is required as there is a potential difference
//              in the call lists returned from CAPI2_CC_GetAllCallIndex() and
//              CAPI2_CC_GetAllCallStates; CAPI2_CC_GetAllCallStates() returns
//              a list of call states for any entry in the call table that doesn't
//              have a call index of INVALID_CALL; CAPI2_CC_GetAllCallIndex()
//              returns a list of call indices for any entry in the call table
//              that doesn't have a call index of INVALID_CALL **AND** is not
//              in CC_CALL_DISCONNECT state. To match up call indices from CAPI2_CC_GetAllCallIndex()
//              with call states from CAPI2_CC_GetAllCallStates(), this helper
//              basically just skips past any entry in the call state list in state
//              CC_CALL_DISCONNECT.
//
// Notes:
//
//******************************************************************************
CCallState_t GetCallStateForIndex( UInt8 inIndex, CCallStateList_t* inCallStateList, UInt8 inListSize )
{
    UInt8 i;
    UInt8 validStateCount = 0;
    CCallState_t state = UNKNOWN_ST;

    // need to look for the inIndex "valid" state in the call state list
    // (valid in this case meaning not CC_CALL_DISCONNECT)
    for ( i = 0; i < inListSize; i++ )
    {
        if ( CC_CALL_DISCONNECT != (*inCallStateList)[i] )
        {
            if ( validStateCount == inIndex )
            {
                state = (*inCallStateList)[i];
                break;
            }
            validStateCount++;
        }
    }

    return state;
}

void KRIL_GetCurrentCallHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilCallListState_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilCallListState_t);
            memset(pdata->bcm_ril_rsp, 0, sizeof(KrilCallListState_t));
            CAPI2_CC_GetAllCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_AllCallIndex;
            break;
        }

        case BCM_CC_AllCallIndex:
        {
            ALL_CALL_INDEX_t *rsp = (ALL_CALL_INDEX_t*) capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            UInt8 i;
            Boolean RemoveJoinIndex = TRUE;
            rdata->total_call = rsp->listSz;
            KRIL_DEBUG(DBG_INFO, "MSG_CC_GETALLCALLINDEX_RSP::total_call:%d listSz:%d\n", rdata->total_call, rsp->listSz);
            if(0 == rsp->listSz)
            {
                KRIL_ClearCallNumPresent(); // Clear to default presentation allowed
                for (i = 0 ; i < BCM_MAX_CALLS_NO ; i++) // Clear all call type to UNKNOWN_TY
                    KRIL_SetCallType(i, UNKNOWN_TY);
                
                // reset cached value of join call index
                sLastJoinCallIndex = INVALID_CALL;
                
                pdata->handler_state = BCM_FinishCAPI2Cmd;
                break;
            }

            for(i = 0 ; i < rsp->listSz ; i++)
            {
                if (VALID_CALL == rsp->indexList[i])
                {
                    CAPI2_CC_GetAllCallIndex(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_CC_AllCallIndex;
                    return;
                }
                rdata->KRILCallState[i].index = (int)rsp->indexList[i];
                KRIL_DEBUG(DBG_INFO, "MSG_CC_GETALLCALLINDEX_RSP::indexList:%d callIndex:%d\n", (int)rsp->indexList[i], rdata->KRILCallState[i].index);
                if (rdata->KRILCallState[i].index == sLastJoinCallIndex)
                    RemoveJoinIndex = FALSE;
            }
            if (TRUE == RemoveJoinIndex)
            {
                KRIL_DEBUG(DBG_INFO, "MSG_CC_GETALLCALLINDEX_RSP::JoinCallIndex does not exist\n");
                sLastJoinCallIndex = INVALID_CALL;
            }
            CAPI2_CC_GetCallType(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
            pdata->handler_state = BCM_CC_GetCallType;
            break;
        }

        case BCM_CC_GetCallType:
        {
            UInt32 *rsp = (UInt32 *)capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            KRIL_DEBUG(DBG_INFO, "MSG_CC_GETCALLTYPE_RSP::call type:%lu\n",*rsp);
            
            rdata->KRILCallState[rdata->index].isMTVTcall = 0;
            
            switch(*rsp)
            {
                case MOVOICE_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 1;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;

                case MTVOICE_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 1;
                    rdata->KRILCallState[rdata->index].isMT = 1;
                break;

                case MODATA_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;

                case MTDATA_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 1;
                break;

                case MOFAX_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;

                case MTFAX_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 1;
                break;

                case MOVIDEO_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;

                case MTVIDEO_CALL:
#ifdef VIDEO_TELEPHONY_ENABLE
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 1;
                    rdata->KRILCallState[rdata->index].isMTVTcall = 1;
                break;
#else
                // If Video telephony not support, reject the VT call.
                {
                    KRIL_DEBUG(DBG_ERROR,"VT call not supported!!!\n");
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 0;

                    CAPI2_CC_EndCall(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                    pdata->handler_state = BCM_CC_GetVTCallEndResult;
                    return;
                }
#endif //VIDEO_TELEPHONY_ENABLE

                default:
                    //Unknow dir and mode
                    rdata->KRILCallState[rdata->index].isVoice = 1;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;
            }
            
            KRIL_SetCallType(rdata->index, *rsp);
            KRIL_DEBUG(DBG_INFO, "GetCallNumPresent:%d rdata->index:%d rdata->KRILCallState[rdata->index].index:%d\n",KRIL_GetCallNumPresent(rdata->KRILCallState[rdata->index].index), rdata->index, rdata->KRILCallState[rdata->index].index);
            if (CC_PRESENTATION_ALLOWED == KRIL_GetCallNumPresent(rdata->KRILCallState[rdata->index].index) || 0 == rdata->KRILCallState[rdata->index].isMT) //The number is always present for MO call and MT call with presentation allowed
            {
                CAPI2_CC_GetCallNumber(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                rdata->KRILCallState[rdata->KRILCallState[rdata->index].index].numberPresentation = 0; //Allowed
                pdata->handler_state = BCM_CC_GetCallNumber;
            }
            else
            {
                if (CC_PRESENTATION_RESTRICTED == KRIL_GetCallNumPresent(rdata->KRILCallState[rdata->index].index))
                {
                    rdata->KRILCallState[rdata->KRILCallState[rdata->index].index].numberPresentation = 1; //Restricted
                }
                else
                {
                    rdata->KRILCallState[rdata->KRILCallState[rdata->index].index].numberPresentation = 2; //Not Specified/Unknown
                }
                CAPI2_CC_IsMultiPartyCall(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                pdata->handler_state = BCM_CC_IsMultiPartyCall;
            }
        }
        break;

        case BCM_CC_GetCallNumber:
        {
            PHONE_NUMBER_STR_t *rsp = (PHONE_NUMBER_STR_t *) capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            KRIL_DEBUG(DBG_INFO, "MSG_CC_GETCALLNUMBER_RSP::phone_number:%s\n",rsp->phone_number);

            if(rsp->phone_number[0] == INTERNATIONAL_CODE)
            {
                rdata->KRILCallState[rdata->index].toa = TOA_International;
            }
            else if(rsp->phone_number[0] == '\0')
            {
                rdata->KRILCallState[rdata->index].toa = 0;
            }
            else
            {
                rdata->KRILCallState[rdata->index].toa = TOA_Unknown;
            }
            strncpy(rdata->KRILCallState[rdata->index].number, rsp->phone_number, PHONE_NUMBER_LENGTH_MAX);
            KRIL_DEBUG(DBG_INFO, "MSG_CC_GETCALLNUMBER_RSP::rdata->index:%d KRILCallState->phone_number:%s\n", rdata->index, rdata->KRILCallState[rdata->index].number);
            CAPI2_CC_IsMultiPartyCall(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
            pdata->handler_state = BCM_CC_IsMultiPartyCall;
        }
        break;

        case BCM_CC_IsMultiPartyCall:
        {
            Boolean *rsp = (Boolean *) capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            if (TRUE == *rsp)
                rdata->KRILCallState[rdata->index].isMpty = 1;
            else
                rdata->KRILCallState[rdata->index].isMpty = 0;
            KRIL_DEBUG(DBG_INFO, "MSG_CC_ISMULTIPARTYCALL_RSP::Ismultipartcall:%d isMpty:%d\n", *rsp, rdata->KRILCallState[rdata->index].isMpty);
            CAPI2_CC_GetCNAPName(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
            pdata->handler_state = BCM_CC_GetCNAPName;
        }
        break;

        case BCM_CC_GetCNAPName:
        {
            CcCnapName_t *rsp = (CcCnapName_t *) capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;

            KRIL_DEBUG(DBG_INFO, "MSG_CC_GETCNAPNAME_REQ::calling nameCoding:%d namelen:%d\n", rsp->nameCoding, rsp->nameLength);
            if (rsp->nameLength != 0)
            {
                if (ALPHA_CODING_GSM_ALPHABET == rsp->nameCoding)
                {
                    rdata->KRILCallState[rdata->index].namelen = KRIL_USSDSeptet2Octet(rsp->name, rdata->KRILCallState[rdata->index].name, rsp->nameLength);
                    KRIL_DEBUG(DBG_INFO, "MSG_CC_GETCNAPNAME_REQ::index:%d name:%s namelen:%d\n", rdata->index, rdata->KRILCallState[rdata->index].name, rdata->KRILCallState[rdata->index].namelen);
                    rdata->KRILCallState[rdata->index].codetype = UNICODE_UCS1;
                }
                else
                {
                    rdata->KRILCallState[rdata->index].namelen = rsp->nameLength;
                    memcpy(rdata->KRILCallState[rdata->index].name, rsp->name, rsp->nameLength);
                    rdata->KRILCallState[rdata->index].codetype = UNICODE_UCS2;
                }
                rdata->KRILCallState[rdata->index].namePresentation = 0; //Allowed
            }
            else
            {
                rdata->KRILCallState[rdata->index].namelen = 0;
                memset(rdata->KRILCallState[rdata->index].name, 0, sizeof(rdata->KRILCallState[rdata->index].name));
                rdata->KRILCallState[rdata->index].namePresentation = 1; //Restricted
            }
            if((rdata->index + 1) >= rdata->total_call)
            {
                CAPI2_CC_GetAllCallStates(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetAllCallStates;
            }
            else
            {
                rdata->index++;
                CAPI2_CC_GetCallType(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                pdata->handler_state = BCM_CC_GetCallType;
            }
        }
        break;

        case BCM_CC_GetAllCallStates:
        {
            UInt8 i;
            ALL_CALL_STATE_t *rsp = (ALL_CALL_STATE_t *) capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;

            // rdata->total_call is list size from CAPI2_CC_GetAllCallIndex(); it will be <= to the list size
            // CAPI2_CC_GetAllCallStates(), as CAPI2_CC_GetAllCallIndex() doesn't include calls in DISCONNECT state;
            // as a result, we're just interested in the number of calls from CAPI2_CC_GetAllCallIndex()
            for(i = 0 ; i < rdata->total_call ; i++)
            {
                CCallState_t state = GetCallStateForIndex( i, &(rsp->stateList), rsp->listSz );
                KRIL_DEBUG(DBG_INFO, "BCM_CC_GetAllCallStates::call listSz:%d state:%d index:%d callIndex:%d\n", rsp->listSz, state, rdata->KRILCallState[i].state);
                switch( state )
                {
                    case CC_CALL_ACTIVE:
                        rdata->KRILCallState[i].state = BCM_CALL_ACTIVE;
                    break;

                    case CC_CALL_HOLD:
                        rdata->KRILCallState[i].state = BCM_CALL_HOLDING;
                    break;

                    case CC_CALL_CALLING:
                        rdata->KRILCallState[i].state = BCM_CALL_DIALING;
                    break;

                    case CC_CALL_ALERTING:
                    case CC_CALL_CONNECTED:
                        if(1 == rdata->KRILCallState[i].isMT)
                        {
                            rdata->KRILCallState[i].state = BCM_CALL_INCOMING;
                        }
                        else
                        {
                            rdata->KRILCallState[i].state = BCM_CALL_ALERTING;
                        }
                    break;

                    case CC_CALL_WAITING:
                        rdata->KRILCallState[i].state = BCM_CALL_WAITING;
                    break;

                    case CC_CALL_BEGINNING:
                        if(1 == rdata->KRILCallState[i].isMT)
                        {
                            rdata->KRILCallState[i].state = BCM_CALL_INCOMING;
                        }
                        else
                        {
                            rdata->KRILCallState[i].state = BCM_CALL_DIALING;
                        }
                    break;

                    default:
                        rdata->KRILCallState[i].state = 0; //Unknow state
                    break;
                }
                KRIL_SetCallState(i, rdata->KRILCallState[i].state);
                KRIL_DEBUG(DBG_INFO, "MSG_CC_GETALLCALLSTATES_RSP::[%d] state:%d rstate:%d\n", i, rsp->stateList[i], rdata->KRILCallState[i].state);
            }
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }
        
        case BCM_CC_SetVTCallEndCause:
        {
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            CAPI2_CC_EndCall(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
            pdata->handler_state = BCM_CC_GetVTCallEndResult;
            break;
        }
        
        case BCM_CC_GetVTCallEndResult:
        {
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            
            if (capi2_rsp->result == CC_END_CALL_FAIL)
            {
                KRIL_DEBUG(DBG_ERROR,"End VT call Failed!!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }
            
            KRIL_DEBUG(DBG_INFO,"End VT call successfully index:%d\n",rdata->KRILCallState[rdata->index].index);
            
            if ((rdata->index + 1) >= rdata->total_call)
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                rdata->index++;
                CAPI2_CC_GetCallType(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                pdata->handler_state = BCM_CC_GetCallType;
            }
            
            rdata->total_call -= 1;
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

#define MAX_NORMAL_CALL_RETRY_TIMES 5
#define MAX_NORMAL_CALL_RETRY_DURATION_SECOND 30
#define MAX_EMERGENCY_CALL_RETRY_TIMES 10
#define MAX_EMERGENCY_CALL_RETRY_DURATION_SECOND 60
 
void KRIL_DialHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
    
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilDial_t *tdata = (KrilDial_t *)pdata->ril_cmd->data;
            
            if (tdata->isVTcall)
            {
#ifdef VIDEO_TELEPHONY_ENABLE
                //Owen VT test
                //CAPI2_CallConfig_t    CallCongif;

                // Set "AT+CBST:134,1,0" for Video Telephony.
                //CallCongif.configType = CALL_CFG_CBST_PARAMS;
                //CallCongif.param_u.cbst_params.speed = 134;    // RIL_SPEED_64000_MULTIMEDIA
                //CallCongif.param_u.cbst_params.name = 1;       // RIL_BSVCNAME_DATACIRCUIT_SYNC_UDI_MODEM
                //CallCongif.param_u.cbst_params.ce = 0;         // RIL_BSVCCE_TRANSPARENT
                
                //CAPI2_CC_SetCallCfg(GetNewTID(), GetClientID(), &CallCongif);
                //pdata->handler_state = BCM_CC_SetCBSTFinished;

                if (NULL != tdata->address)
                {
                    KRIL_DEBUG(DBG_ERROR, "Phone number: %s\n", tdata->address);
                    CAPI2_CC_MakeVideoCall(GetNewTID(), GetClientID(), (UInt8*)tdata->address);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
                else
                {
                    KRIL_DEBUG(DBG_ERROR, "Phone number is Null. Error!!!");
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
                
#else
                KRIL_DEBUG(DBG_ERROR, "VT call not supported. Error!!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
#endif //VIDEO_TELEPHONY_ENABLE
            }
            else
            {
                CAPI2_SIM_GetAlsDefaultLine(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_SIM_GetAlsDefaultLine;
                KRIL_SetLastCallFailCause(BCM_CALL_FAIL_NORMAL);
            }

            break;
        }

        case BCM_SIM_GetAlsDefaultLine:
        {
            UInt32* rsp = (UInt32*) capi2_rsp->dataBuf;
            if ( capi2_rsp->result == RESULT_OK )
            {
                KrilDial_t *tdata = (KrilDial_t *)pdata->ril_cmd->data;
                VoiceCallParam_t m_VoiceCallParam;
                Subaddress_t defaultSubAddress = {0,0,0};

				// retry context
            	KrilCallRetryInfo_t *context = (KrilCallRetryInfo_t *)pdata->cmdContext;
                memset(context, 0, sizeof(KrilCallRetryInfo_t));

                KRIL_DEBUG(DBG_INFO, "address:%s clir:%d, is emergency:%d\n", tdata->address, tdata->clir, tdata->isEmergency);
                memset(&m_VoiceCallParam, 0, sizeof(VoiceCallParam_t));

                m_VoiceCallParam.subAddr = defaultSubAddress;
                m_VoiceCallParam.cug_info.cug_index = CUGINDEX_NONE;
                m_VoiceCallParam.cug_info.suppress_pref_cug = CUGSUPPRESSPREFCUG_DISABLE;
                m_VoiceCallParam.cug_info.suppress_oa = CUGSUPPRESSOA_DISABLE;
                m_VoiceCallParam.clir = tdata->clir;
				
				// CWYoon 110827::CSP447453 Emergency call in FDN mode.					
				if ( (tdata->isEmergency)||(StkCall == TRUE) ) // gearn  FDN enable setup call 
					m_VoiceCallParam.isFdnChkSkipped = TRUE;
				else
					m_VoiceCallParam.isFdnChkSkipped = FALSE;
					
                m_VoiceCallParam.auxiliarySpeech = rsp == 1 ? TRUE : FALSE;

                m_VoiceCallParam.isEmergency = tdata->isEmergency;
                m_VoiceCallParam.emergencySvcCat = tdata->emergencySvcCat;
				// save dial information for retry
				memcpy(&context->call_param, &m_VoiceCallParam, sizeof(VoiceCallParam_t));
				memcpy((UInt8 *)context->address, (UInt8*)tdata->address, sizeof(tdata->address));
				context->current_retry_count = 0;
				context->total_retry_count = (tdata->isEmergency)? 10:5;
				if(tdata->isEmergency)
				{
					context->retry_duration = jiffies + MAX_EMERGENCY_CALL_RETRY_DURATION_SECOND * HZ;
				}
				else
				{
					context->retry_duration = jiffies + MAX_NORMAL_CALL_RETRY_DURATION_SECOND * HZ;
				} 
                KRIL_DEBUG(DBG_INFO, "auxiliarySpeech:%d isEmergency:%d\n", m_VoiceCallParam.auxiliarySpeech, m_VoiceCallParam.isEmergency);
                KRIL_DEBUG(DBG_INFO, "retry_duration:%ld total retry count%d, jiffies: %ld \n", context->retry_duration, context->total_retry_count, jiffies);
                CAPI2_CC_MakeVoiceCall(GetNewTID(), GetClientID(), (UInt8*)tdata->address, m_VoiceCallParam);
#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)	
                KRIL_DEBUG(DBG_INFO, "KRIL_DialHandler:BCM_SIM_GetAlsDefaultLine::enter lcd frame inversion\n");
								lcd_enter_during_call();
#endif				
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Can't Dial...!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;  // can't dial
            }
            break;
        }

#ifdef VIDEO_TELEPHONY_ENABLE        
        case BCM_CC_SetCBSTFinished:
        {
            //Owen VT test
            //CAPI2_ECHO_SetDigitalTxGain(GetNewTID(), GetClientID(), -100);
            pdata->handler_state = BCM_CC_MakeVideoCall;           
            break;
        }

        case BCM_CC_MakeVideoCall:
        {
            KrilDial_t *tdata = (KrilDial_t *)pdata->ril_cmd->data;
            
            if (NULL != tdata->address)
            {
                KRIL_DEBUG(DBG_ERROR, "Phone number: %s\n", tdata->address);
                CAPI2_CC_MakeVideoCall(GetNewTID(), GetClientID(), (UInt8*)tdata->address);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Phone number is Null. Error!!!");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }
#endif //VIDEO_TELEPHONY_ENABLE
        
        case BCM_RESPCAPI2Cmd:
        {
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            
            if (capi2_rsp->msgType == MSG_VOICECALL_CONNECTED_IND)
            {
                VoiceCallConnectMsg_t *rsp = (VoiceCallConnectMsg_t *) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_CONNECTED_IND::callIndex:%d progress_desc:%d\n", rsp->callIndex,rsp->progress_desc);
                KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            }
            else if (capi2_rsp->msgType == MSG_VOICECALL_RELEASE_IND || 
                       capi2_rsp->msgType == MSG_VOICECALL_RELEASE_CNF)
            {
                pdata->result = RILErrorResult(capi2_rsp->result);
                KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_RELEASE_IND::result:%d, capi2_rsp->result:%d\n", pdata->result, capi2_rsp->result);
                if (capi2_rsp->dataBuf != NULL)
                {
                    VoiceCallReleaseMsg_t *rsp = (VoiceCallReleaseMsg_t *) capi2_rsp->dataBuf;
            		KrilCallRetryInfo_t *context = (KrilCallRetryInfo_t *)pdata->cmdContext;
					
                    KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_RELEASE_IND::callIndex:%d exitCause:%d callCCMUnit:%ld callDuration:%ld\n", rsp->callIndex, rsp->exitCause, rsp->callCCMUnit, rsp->callDuration);
					if(capi2_rsp->msgType == MSG_VOICECALL_RELEASE_IND && rsp->exitCause == MNCAUSE_RADIO_LINK_FAILURE_APPEARED)
					{
						KRIL_DEBUG(DBG_INFO, "RETRY-1 ::jiffies:%ld exitCause:%d retry_duration:%ld\n", jiffies, rsp->exitCause, context->retry_duration);
						if(context->retry_duration > jiffies )
						{
							// start retry session
							pdata->handler_state = BCM_CC_SilentCallRetry;
							context->current_retry_count ++;
                			KRIL_DEBUG(DBG_INFO, "BCM_CC_SilentCallRetry: MSG_VOICECALL_RELEASE_IND-0::current_retry_count:%d, retry_duration: %ld\n", context->current_retry_count, context->retry_duration);
                			CAPI2_CC_MakeVoiceCall(GetNewTID(), GetClientID(), (UInt8*)context->address, context->call_param);
							return;
						}
					}
                    KRIL_SetLastCallFailCause( KRIL_MNCauseToRilError(rsp->exitCause) );
                }
                else
                {
                    KRIL_SetLastCallFailCause( ResultToRilFailCause(capi2_rsp->result) );
                }
                KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            }
#ifdef VIDEO_TELEPHONY_ENABLE
            else if (capi2_rsp->msgType == MSG_DATACALL_CONNECTED_IND)
            {
                if (NULL != capi2_rsp->dataBuf)
                {
                    DataCallConnectMsg_t *rsp = (DataCallConnectMsg_t *) capi2_rsp->dataBuf;
                    int callIndex = rsp->callIndex;
                    
                    KRIL_DEBUG(DBG_INFO, "MSG_DATACALL_CONNECTED_IND::callIndex: %d\n",rsp->callIndex);
                    KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_CONNECT, &callIndex, sizeof(int));
                }
                else
                {
                    KRIL_DEBUG(DBG_ERROR, "MSG_DATACALL_CONNECTED_IND\n");
                    KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_CONNECT, NULL, 0);
                }
            }
            else if (capi2_rsp->msgType == MSG_DATACALL_RELEASE_IND)
            {
                if (NULL != capi2_rsp->dataBuf)
                {
                    DataCallReleaseMsg_t *rsp = (DataCallReleaseMsg_t *) capi2_rsp->dataBuf;
                    int callIndex = rsp->callIndex;
                    
                    KRIL_DEBUG(DBG_INFO, "MSG_DATACALL_RELEASE_IND:: callIndex: %d exitCause: 0x%X\n", rsp->callIndex, rsp->exitCause);
                    KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_END, &callIndex, sizeof(int));
                }
                else
                {
                    KRIL_DEBUG(DBG_ERROR, "MSG_DATACALL_RELEASE_IND\n");
                    KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_END, NULL, 0);
                }
            }
#endif //VIDEO_TELEPHONY_ENABLE
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Receive Unkenow Message :: msgType:0x%x\n", capi2_rsp->msgType);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }

        case BCM_CC_SilentCallRetry:
        {
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            
            KRIL_DEBUG(DBG_INFO, "RETRY: BCM_CC_SilentCallRetry-1\n" );
            if (capi2_rsp->msgType == MSG_VOICECALL_CONNECTED_IND)
            {
                VoiceCallConnectMsg_t *rsp = (VoiceCallConnectMsg_t *) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_CONNECTED_IND::callIndex:%d progress_desc:%d\n", rsp->callIndex,rsp->progress_desc);
                KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            }
            else if (capi2_rsp->msgType == MSG_VOICECALL_RELEASE_IND || 
                       capi2_rsp->msgType == MSG_VOICECALL_RELEASE_CNF)
            {
                pdata->result = RILErrorResult(capi2_rsp->result);
                KRIL_DEBUG(DBG_INFO, "BCM_CC_SilentCallRetry: MSG_VOICECALL_RELEASE_IND-1::result:%d, capi2_rsp->result:%d\n", pdata->result, capi2_rsp->result);
                if (capi2_rsp->dataBuf != NULL)
                {
                    VoiceCallReleaseMsg_t *rsp = (VoiceCallReleaseMsg_t *) capi2_rsp->dataBuf;
            		KrilCallRetryInfo_t *context = (KrilCallRetryInfo_t *)pdata->cmdContext;

                    KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_RELEASE_IND::callIndex:%d exitCause:%d callCCMUnit:%ld callDuration:%ld\n", rsp->callIndex, rsp->exitCause, rsp->callCCMUnit, rsp->callDuration);
					if(capi2_rsp->msgType == MSG_VOICECALL_RELEASE_IND && rsp->exitCause == MNCAUSE_RADIO_LINK_FAILURE_APPEARED)
					{
                		KRIL_DEBUG(DBG_INFO, "BCM_CC_SilentCallRetry: retry_duration: %ld, jiffies: %ld, retry_count:%d, total retry count:%d\n", context->retry_duration, jiffies, context->current_retry_count, context->total_retry_count);
						if(context->retry_duration > jiffies && context->current_retry_count++ <= context->total_retry_count)
						{
							// start retry session
							pdata->handler_state = BCM_CC_SilentCallRetry;

                			KRIL_DEBUG(DBG_INFO, "BCM_CC_SilentCallRetry: MSG_VOICECALL_RELEASE_IND-3::retry_count:%d\n", context->current_retry_count);
                			CAPI2_CC_MakeVoiceCall(GetNewTID(), GetClientID(), (UInt8*)context->address, context->call_param);
							return;
						}
					}
                    KRIL_SetLastCallFailCause( KRIL_MNCauseToRilError(rsp->exitCause) );
                }
                else
                {
            		KRIL_DEBUG(DBG_INFO, "RETRY: BCM_CC_SilentCallRetry-2\n" );
                    KRIL_SetLastCallFailCause( ResultToRilFailCause(capi2_rsp->result) );
                }
				
                KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Receive Unkenow Message :: msgType:0x%x\n", capi2_rsp->msgType);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
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


void KRIL_HungupHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "KRIL_HungupHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int *Index = (int *)(pdata->ril_cmd->data);
            KRIL_DEBUG(DBG_TRACE,"Current call Index:%d\n", *Index);
            if(*Index == 11)
               CAPI2_CC_EndAllCalls(GetNewTID(), GetClientID());
            else
               CAPI2_CC_EndCall(GetNewTID(), GetClientID(), (*Index-1));
            pdata->handler_state = BCM_RESPCAPI2Cmd;
#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
						KRIL_DEBUG(DBG_INFO, "KRIL_HungupHandler:BCM_SendCAPI2Cmd::restore lcd frame inversion\n");
	    			lcd_restore_during_call();
#endif
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {

	        if(capi2_rsp->result == RESULT_OK)
 	        {
				
                pdata->handler_state = BCM_FinishCAPI2Cmd;
 	         }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
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


void KRIL_HungupWaitingOrBackgroundHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_HungupWaitingOrBackgroundHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            // check for waiting call first...
            CAPI2_CC_GetNextWaitCallIndex (GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextWaitCallIndex;
            break;
        }

        case BCM_CC_GetNextWaitCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO,"WaitIndex:%d\n", *rsp);

            if(*rsp != INVALID_CALL)
            {
                // there was a waiting call, so end it and we're outta here
                CAPI2_CC_EndCall(GetNewTID(), GetClientID(), *rsp);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
								KRIL_DEBUG(DBG_INFO, "KRIL_HungupWaitingOrBackgroundHandler:BCM_CC_GetNextWaitCallIndex::restore lcd frame inversion\n");
	  						lcd_restore_during_call();
#endif				
            }
            else
            {
                // no waiting call, so check for held calls
                CAPI2_CC_GetAllHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_EndNextHeldCall;
            }
            break;
        }

        case BCM_CC_EndNextHeldCall:
        {
            ALL_CALL_INDEX_t *rsp = (ALL_CALL_INDEX_t *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "BCM_CC_EndNextHeldCall: rsp->listSz:%d\n", rsp->listSz);
            if (rsp->listSz != 0)
            {
                if (1 == rsp->listSz)
                {
                    // single held call
                    CAPI2_CC_EndCall(GetNewTID(), GetClientID(), rsp->indexList[0]);
                }
                else
            {
                    // multiparty held call
                    CAPI2_CC_EndMPTYCalls(GetNewTID(), GetClientID());
                }
                pdata->handler_state = BCM_RESPCAPI2Cmd;
#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
								KRIL_DEBUG(DBG_INFO, "KRIL_HungupWaitingOrBackgroundHandler:BCM_CC_EndNextHeldCall::restore lcd frame inversion\n");
								lcd_restore_during_call();
#endif				
            }
            else
            {
                // no held calls, so check if there is incoming call to end
                if(KRIL_GetIncomingCallIndex() != INVALID_CALL)
                {
                    CAPI2_CC_EndCall(GetNewTID(), GetClientID(), KRIL_GetIncomingCallIndex());
                    KRIL_SetIncomingCallIndex(INVALID_CALL);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
										KRIL_DEBUG(DBG_INFO, "KRIL_HungupWaitingOrBackgroundHandler:BCM_CC_EndNextHeldCall_else::restore lcd frame inversion\n");
		    						lcd_restore_during_call();
#endif				
                }
                else
                {
                    // we're done...   
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
            }
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result == RESULT_OK)
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
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


void KRIL_HungupForegroundResumeBackgroundHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_HungupForegroundResumeBackgroundHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_TRACE,"handler state:%lu\n", pdata->handler_state);
            CAPI2_CC_GetAllActiveCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextActiveCallIndex;
            break;
        }

        case BCM_CC_GetNextActiveCallIndex:
        {
            ALL_CALL_INDEX_t *rsp = (ALL_CALL_INDEX_t *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_ERROR, "rsp->listSz:%d\n", rsp->listSz);
            if (rsp->listSz != 0)
            {
                if (1 == rsp->listSz)
                {
                    CAPI2_CC_EndCall(GetNewTID(), GetClientID(), rsp->indexList[0]);
                    pdata->handler_state = BCM_SendCAPI2Cmd;
                }
                else
                {
                    g_totalMPTYCall = rsp->listSz;
                    CAPI2_CC_EndMPTYCalls(GetNewTID(), GetClientID());
                    KRIL_SetHungupForegroundResumeBackgroundEndMPTY(GetTID());
                    pdata->handler_state = BCM_CC_EndMPTYCalls;

#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
								KRIL_DEBUG(DBG_INFO, "KRIL_HungupForegroundResumeBackgroundHandler:BCM_CC_GetNextActiveCallIndex::restore lcd frame inversion\n");
								lcd_restore_during_call();
#endif				
               }
			}
            else
            {
                CAPI2_CC_GetNextWaitCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextWaitCallIndex;
            }
            break;
        }

        case BCM_CC_EndMPTYCalls:
        {
            g_totalMPTYCall--;
            KRIL_DEBUG(DBG_INFO,"g_totalMPTYCall:%d\n", g_totalMPTYCall);
            if (0 == g_totalMPTYCall)
            {
                KRIL_SetHungupForegroundResumeBackgroundEndMPTY(0);
                // ***YY*** Revert the change
                CAPI2_CC_GetNextWaitCallIndex(GetNewTID(), GetClientID());
                // **MAG** use GetAllHeldCallIndex so we can check for MPTY call
                //CAPI2_CC_GetAllHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextWaitCallIndex;
            }
            else
            {
                 // tid for handler will be updated in KRIL_ResponseHandler() on returning from here, so update here to new tid
                 KRIL_SetHungupForegroundResumeBackgroundEndMPTY(GetNewTID());
            }

               
            break;
        }

        case BCM_CC_GetNextWaitCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "WaitCallIndex:%d\n", *rsp);
            if(*rsp != INVALID_CALL)
            {
                CAPI2_CC_AcceptVoiceCall(GetNewTID(), GetClientID(), *rsp);
                pdata->handler_state = BCM_CC_AcceptVoiceCall;
            }
            else
            {
                // **MAG** use GetAllHeldCallIndex so we can check for MPTY call
                CAPI2_CC_GetAllHeldCallIndex(GetNewTID(), GetClientID());
                //CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;
        }

        case BCM_CC_AcceptVoiceCall:
        {
			
            if(capi2_rsp->result == RESULT_OK)
            {
                if ( MSG_VOICECALL_ACTION_RSP == capi2_rsp->msgType )
                {
    				VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t *)capi2_rsp->dataBuf;
    				KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_ACTION_RSP result:%d\n", rsp->callResult);
    				if(rsp->callResult == CC_ACCEPT_CALL_FAIL )
    				{
                        // **MAG** use GetAllHeldCallIndex so we can check for MPTY call
                        CAPI2_CC_GetAllHeldCallIndex(GetNewTID(), GetClientID());
    					//CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
    					pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
    				}
    				else
    				{
    					pdata->handler_state = BCM_FinishCAPI2Cmd;
    				}
				}
				else if ( MSG_VOICECALL_RELEASE_IND == capi2_rsp->msgType )
				{
                    VoiceCallReleaseMsg_t *rsp = (VoiceCallReleaseMsg_t *) capi2_rsp->dataBuf;
    				KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_RELEASE_IND exitCause:%d\n", rsp->exitCause);
                    KRIL_SetLastCallFailCause( KRIL_MNCauseToRilError(rsp->exitCause) );
				    pdata->handler_state = BCM_ErrorCAPI2Cmd;
 				}
 				else if ( MSG_VOICECALL_CONNECTED_IND == capi2_rsp->msgType )
 				{
 				    VoiceCallConnectMsg_t* rsp = (VoiceCallReleaseMsg_t *) capi2_rsp->dataBuf;
    				KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_CONNECTED_IND call index:%d\n", rsp->callIndex);
    				pdata->handler_state = BCM_FinishCAPI2Cmd;
 				}
				else
				{
    				KRIL_DEBUG(DBG_ERROR, "Unexpected response to  CAPI2_CC_AcceptVoiceCall msg 0x%x\n",capi2_rsp->msgType);
				    pdata->handler_state = BCM_ErrorCAPI2Cmd;
				}
            }
            else
            {
                // **MAG** use GetAllHeldCallIndex so we can check for MPTY call
                CAPI2_CC_GetAllHeldCallIndex(GetNewTID(), GetClientID());
				//CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
				pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;

        }

        case BCM_CC_GetNextHeldCallIndex:
        {
            ALL_CALL_INDEX_t *rsp = (ALL_CALL_INDEX_t *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "BCM_CC_GetNextHeldCallIndex: rsp->listSz:%d\n", rsp->listSz);
            if (rsp->listSz != 0)
            {
            	/* ***YY*** FIX compile error */
            	UInt8 callIndex = INVALID_CALL;
                if ( rsp->listSz > 1 )
                {
                    // check if we cached the call index we used for the join...
                    if ( INVALID_CALL != sLastJoinCallIndex )
                    {
                        // yes, so use it to retrieve the call
                        KRIL_DEBUG(DBG_INFO, "using call index cached from last join req:%d\n", sLastJoinCallIndex);
                        callIndex = sLastJoinCallIndex;
                    }
                    else
                    {
                        // no, so use the first call index from the held call list
                        KRIL_DEBUG(DBG_INFO, "no cached index, using first from held call list:%d\n", (int)rsp->indexList[0]);
                        callIndex = (int)rsp->indexList[0];
                    }
                }
                else
                {
                    // single held call, not MPTY
                    KRIL_DEBUG(DBG_INFO, "not mpty call, using first from held call list:%d\n", (int)rsp->indexList[0]);
                    callIndex = (int)rsp->indexList[0];
                }
                
                CAPI2_CC_RetrieveCall(GetNewTID(), GetClientID(), callIndex);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
           }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        /*
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "HeldCallIndex:%d\n", *rsp);
            if(*rsp != INVALID_CALL)
            {
                CAPI2_CC_RetrieveCall(GetNewTID(), GetClientID(), *rsp);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        */
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
		
            KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd result:%d\n", capi2_rsp->result);
            if(capi2_rsp->result == RESULT_OK)
            {
				VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t *)capi2_rsp->dataBuf;
				KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd result:%d\n", rsp->callResult);
				if(rsp->callResult == CC_RESUME_CALL_SUCCESS || rsp->callResult == RESULT_OK){
				   pdata->handler_state = BCM_FinishCAPI2Cmd;
				}
				else{
				   pdata->handler_state = BCM_ErrorCAPI2Cmd;
				}
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
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


void KRIL_SwitchWaitingOrHoldingAndActiveHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp) //CHLD=2
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_SwitchWaitingOrHoldingAndActiveHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
        	  KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
        	  
        	  // Initialize the misc index in the call context (MPTY_FIX_V4_diff)
        	  context->ActiveIndex = INVALID_CALL;
        	  context->WaitIndex   = INVALID_CALL;
        	  context->MptyIndex   = INVALID_CALL;       	  
        	  context->HeldIndex   = INVALID_CALL;
        	  context->activeMPTY = FALSE;
        	  context->heldMPTY = FALSE;
        	  // End of (MPTY_FIX_V4_diff)
        	  
            KRIL_SetInHoldCallHandler(TRUE);
            //CAPI2_CC_GetNextActiveCallIndex(GetNewTID(), GetClientID());
            // get all active and we can check for MPTY by list size
            CAPI2_CC_GetAllActiveCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextActiveCallIndex;
            break;
        }

        case BCM_CC_GetNextActiveCallIndex:
        {
            KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
            ALL_CALL_INDEX_t *rsp = (ALL_CALL_INDEX_t *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_ERROR, "rsp->listSz:%d\n", rsp->listSz);
            if (rsp->listSz != 0)
            {
                context->ActiveIndex = rsp->indexList[0];
                if (1 == rsp->listSz)
                {
                    context->activeMPTY = FALSE;
                }
                else
                {
                    context->activeMPTY = TRUE;
                }
            }
/*
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
            context->ActiveIndex= *rsp;
*/
            KRIL_DEBUG(DBG_INFO, "ActiveIndex:%d MPTY:%s\n", context->ActiveIndex, (context->activeMPTY?"TRUE":"FALSE") );
            CAPI2_CC_GetNextWaitCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextWaitCallIndex;

            break;
        }

        case BCM_CC_GetNextWaitCallIndex:
        {
            UInt8 *rsp = (UInt8 *) capi2_rsp->dataBuf;
            KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
            context->WaitIndex = *rsp;
            KRIL_DEBUG(DBG_INFO, "WaitIndex:%d\n", context->WaitIndex);
/*
            if(context->ActiveIndex != INVALID_CALL && context->WaitIndex == INVALID_CALL)
            {
                CAPI2_CC_GetMPTYCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetMPTYCallIndex;
            }
            else
*/
            {
                // **MAG** use GetAllHeldCallIndex so we can check for MPTY call
                CAPI2_CC_GetAllHeldCallIndex(GetNewTID(), GetClientID());
                //CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;
        }
/*
        // **FIXME** MAG - state no longer needed?
        case BCM_CC_GetMPTYCallIndex:
        {
            UInt8 *rsp = (UInt8*) capi2_rsp->dataBuf;
            KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
            context->MptyIndex = *rsp;
            KRIL_DEBUG(DBG_INFO, "MptyIndex:%d\n", context->MptyIndex);
            if(context->MptyIndex != INVALID_CALL)
            {
                if(INVALID_CALL != context->ActiveIndex)
                {
                    CAPI2_CC_SwapCall(GetNewTID(), GetClientID(), context->MptyIndex);
                    pdata->handler_state = BCM_CC_SwapCall;
                }
                else
                {
                    CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
                }
            }
            else
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;
        }
*/
        case BCM_CC_GetNextHeldCallIndex:
        {
            ALL_CALL_INDEX_t *rsp = (ALL_CALL_INDEX_t *)capi2_rsp->dataBuf;
            //UInt8 *rsp = (UInt8*) capi2_rsp->dataBuf;
            KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
            //context->HeldIndex = *rsp;

            KRIL_DEBUG(DBG_INFO, "BCM_CC_GetNextHeldCallIndex: rsp->listSz:%d\n", rsp->listSz);
            if (rsp->listSz != 0)
            {
                context->HeldIndex = (int)rsp->indexList[0];
                if ( rsp->listSz > 1 )
                {
                    // MPTY held call
                    context->heldMPTY = TRUE;
                }
                else
                {
                    // single held call
                    context->heldMPTY = FALSE;
                }
                
            }
            KRIL_DEBUG(DBG_INFO, "HeldIndex:%d MPTY:%s\n", context->HeldIndex, (context->heldMPTY?"TRUE":"FALSE"));
            
            if(context->HeldIndex != INVALID_CALL)
            {
                UInt8 heldIndexToUse = INVALID_CALL;
                
                if ( context->heldMPTY && (INVALID_CALL != sLastJoinCallIndex) )
                {
                    UInt8 listIndex;
                    for (listIndex = 0 ; listIndex < rsp->listSz ; listIndex++)
                    {
                        if(sLastJoinCallIndex == rsp->indexList[listIndex])
                        {
                            KRIL_DEBUG(DBG_INFO, "cached join index valid, using as held index %d\n", sLastJoinCallIndex);
                            heldIndexToUse = sLastJoinCallIndex;
                            break;
                        }
                        // last Join call index is active call, swap call need the held call index
                        KRIL_DEBUG(DBG_INFO, "cached join index valid, using as held index %d\n", rsp->indexList[0]);
                        heldIndexToUse = context->HeldIndex;
                    }
                }
                else
                {
                    KRIL_DEBUG(DBG_INFO, "cached join index not valid, using held index from context %d\n", context->HeldIndex);
                    heldIndexToUse = context->HeldIndex;
                }
                
                if(context->ActiveIndex != INVALID_CALL)
                {
                    // one held, one active
                    CAPI2_CC_SwapCall(GetNewTID(), GetClientID(), heldIndexToUse);
                    pdata->handler_state = BCM_CC_SwapCall;
                }
                else if(context->WaitIndex != INVALID_CALL)
                {
                    // one held, one waiting
                    CAPI2_CC_AcceptWaitingCall(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
                else
                {
                    // just one held
                    //CAPI2_CC_RetrieveNextHeldCall(GetNewTID(), GetClientID());
                    CAPI2_CC_RetrieveCall(GetNewTID(), GetClientID(), heldIndexToUse);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
            }
            else
            {
                UInt8 activeIndexToUse = INVALID_CALL;
                
                if ( context->activeMPTY && (INVALID_CALL != sLastJoinCallIndex) )
                {
                    KRIL_DEBUG(DBG_INFO, "cached join index valid, using as active index %d\n", sLastJoinCallIndex);
                    activeIndexToUse = sLastJoinCallIndex;
                }
                else
                {
                    KRIL_DEBUG(DBG_INFO, "cached join index not valid, using active index from context %d\n", context->HeldIndex);
                    activeIndexToUse = context->ActiveIndex;
                }
                if(context->WaitIndex != INVALID_CALL)
                {
/*                    if(context->MptyIndex != INVALID_CALL)
                    {
                        CAPI2_CC_HoldCall(GetNewTID(), GetClientID(), context->MptyIndex);
                        pdata->handler_state = BCM_CC_HoldAnswerWaitCall;
                    }
                    else if(context->ActiveIndex != INVALID_CALL)
*/
                    if(context->ActiveIndex != INVALID_CALL)
                    {
                        CAPI2_CC_HoldCall(GetNewTID(), GetClientID(), activeIndexToUse);
                        pdata->handler_state = BCM_CC_HoldAnswerWaitCall;
                    }
                    else
                    {
                        CAPI2_CC_AcceptWaitingCall(GetNewTID(), GetClientID());
                        pdata->handler_state = BCM_RESPCAPI2Cmd;
                    }
                }
                else
                {
                    CAPI2_CC_HoldCall(GetNewTID(), GetClientID(), activeIndexToUse);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
            }
            break;
        }

        case BCM_CC_HoldAnswerWaitCall:
        {
            if(capi2_rsp->result == RESULT_OK )
            {
                VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t*) capi2_rsp->dataBuf;
                if(rsp->callResult == CC_HOLD_CALL_SUCCESS || 
                   rsp->callResult == RESULT_OK )
                {
                    // current call was held successfully, so now answer waiting
                    CAPI2_CC_AcceptWaitingCall(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
                else
                {
                    // end waitingcall and return error if active call can't held
                    KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
                    CAPI2_CC_EndCall(GetNewTID(), GetClientID(), context->WaitIndex);
                    pdata->handler_state = BCM_CC_HoldCallFailEndWaitCall;
                }
            }
            else
            {
                // end waitingcall and return error if active call can't held
                KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
                CAPI2_CC_EndCall(GetNewTID(), GetClientID(), context->WaitIndex);
                pdata->handler_state = BCM_CC_HoldCallFailEndWaitCall;
            }
            break;
        }

        case BCM_CC_HoldCallFailEndWaitCall:
        {
            KRIL_DEBUG(DBG_INFO, "BCM_CC_HoldCallFailEndWaitCall::result:%d\n", capi2_rsp->result);
            KRIL_SetInHoldCallHandler(FALSE);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
        
        case BCM_CC_SwapCall:
        {
            if(capi2_rsp->result == RESULT_OK )
            {
                VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t*) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::rsp->callResult:%d\n", rsp->callResult);
                if(rsp->callResult == CC_RESUME_CALL_SUCCESS || 
                   rsp->callResult == CC_SWAP_CALL_SUCCESS || 
                   rsp->callResult == RESULT_OK )
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
                else
                {
                    KRIL_DEBUG(DBG_INFO, "BCM_CC_SwapCall::callResult:%d\n", rsp->callResult);
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
            }
            else
            {
                KRIL_DEBUG(DBG_INFO, "BCM_CC_SwapCall::result:%d\n", capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            KRIL_SetInHoldCallHandler(FALSE);
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
//yihwa
            Boolean b_send_release_complete=false;
            
            if(capi2_rsp->result == RESULT_OK)
            {
                VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t*) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::rsp->callResult:%d\n", rsp->callResult);
                if(rsp->callResult == CC_ACCEPT_CALL_SUCCESS || 
                   rsp->callResult == CC_RESUME_CALL_SUCCESS || 
                   rsp->callResult == CC_SWAP_CALL_SUCCESS || 
                   rsp->callResult == CC_HOLD_CALL_SUCCESS || 
                   rsp->callResult == RESULT_OK)
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
		  else if((rsp->callResult == CC_HOLD_CALL_FAIL)&&(StkCall == TRUE))//yihwa
		  {
                    b_send_release_complete = TRUE;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
                else
                {
                    KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::rsp->callResult:%d\n", rsp->callResult);
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            KRIL_SetInHoldCallHandler(FALSE);
			
//yihwa
	  if(b_send_release_complete)
	  {
                b_send_release_complete = false;
                RIL_SS_Release_Comp_Msg rilSSReleaseCompMsg;
                rilSSReleaseCompMsg.status = RIL_PARAM_SSDI_DATA | RIL_PARAM_SSDI_STATUS;
                rilSSReleaseCompMsg.dataLen = 0;
                rilSSReleaseCompMsg.params = 0;
                rilSSReleaseCompMsg.size = sizeof(rilSSReleaseCompMsg);

                KRIL_DEBUG(DBG_ERROR, "b_send_release_complete:%d\n", b_send_release_complete);
				
                KRIL_SendNotify(RIL_UNSOL_RELEASE_COMPLETE_MESSAGE, &rilSSReleaseCompMsg,sizeof(rilSSReleaseCompMsg ));
            }
			
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            KRIL_SetInHoldCallHandler(FALSE);            
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_ConferenceHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_ConferenceHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_CC_GetNextActiveCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextActiveCallIndex;
            break;
        }

        case BCM_CC_GetNextActiveCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            // save join call index for "future" use...
            sLastJoinCallIndex = *rsp;
            KRIL_DEBUG(DBG_INFO, "ActiveCallIndex:%d\n", *rsp);
            CAPI2_CC_JoinCall(GetNewTID(), GetClientID(), *rsp);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result == RESULT_OK)
            {
                VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t *)capi2_rsp->dataBuf;
				
                KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::rsp->callResult:%d\n", rsp->callResult);
                if(rsp->callResult == CC_JOIN_CALL_SUCCESS || rsp->callResult == RESULT_OK)
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
                else
                {
                    sLastJoinCallIndex = INVALID_CALL;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
            }
            else
            {
                sLastJoinCallIndex = INVALID_CALL;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
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


void KRIL_UDUBHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_UDUBHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "KRIL_GetIncomingCallIndex:%d KRIL_GetWaitingCallIndex:%d\n", KRIL_GetIncomingCallIndex(), KRIL_GetWaitingCallIndex());
            if(KRIL_GetWaitingCallIndex() != INVALID_CALL)
            {
                CAPI2_CC_EndCall(GetNewTID(), GetClientID(), KRIL_GetWaitingCallIndex());
            }
            else
            {
                CAPI2_CC_EndCall(GetNewTID(), GetClientID(), KRIL_GetIncomingCallIndex());
            }
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
    	   KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd\n");
           pdata->bcm_ril_rsp = NULL;
           pdata->rsp_len = 0;
                KRIL_SetIncomingCallIndex(INVALID_CALL);
                KRIL_SetWaitingCallIndex(INVALID_CALL);
           if(capi2_rsp->result == RESULT_OK)
           {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
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


void KRIL_LastCallFailCauseHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
    KrilLastCallFailCause_t *rdata;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_LastCallFailCauseHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilLastCallFailCause_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilLastCallFailCause_t);
            memset(pdata->bcm_ril_rsp, 0, sizeof(KrilLastCallFailCause_t));

    // Return the cached failure cause.
    rdata = (KrilLastCallFailCause_t *)pdata->bcm_ril_rsp;
    rdata->failCause = KRIL_GetLastCallFailCause();
    pdata->handler_state = BCM_FinishCAPI2Cmd;
        }

//******************************************************************************
//
// Function Name: ResultToRilFailCause
//
// Description:   Converts the CAPI error code to a RIL last call failure cause.
//
// Notes:
//
//******************************************************************************
BRIL_LastCallFailCause ResultToRilFailCause(Result_t inResult)
        {
    BRIL_LastCallFailCause failCause;
    switch (inResult)
            {
        case CC_MAKE_CALL_SUCCESS:
        case RESULT_OK:
            failCause = BCM_CALL_FAIL_NORMAL;
                    break;

         case CC_FDN_BLOCK_MAKE_CALL:
            failCause = BCM_CALL_FAIL_FDN_BLOCKED;
                    break;

	case RESULT_DIALSTR_INVALID:
            failCause = BCM_CALL_FAIL_UNOBTAINABLE_NUMBER;
		     break;

        // Don't distinguish between these failures;
        // the UI only requires RIL_LastCallFailCause
        // failure codes.
        case STK_DATASVRC_BUSY:
//        case RESULT_DIALSTR_INVALID:
        case RESULT_SIM_NOT_READY:
        case CC_FAIL_MAKE_CALL:
        case CC_FAIL_CALL_SESSION:
        case CC_WRONG_CALL_TYPE:
                default:
            failCause = BCM_CALL_FAIL_ERROR_UNSPECIFIED;
            break;
        }

    KRIL_DEBUG(DBG_INFO, "CAPI2 result:%d failCause:%d\n", inResult, failCause);
    return failCause;
    }

void KRIL_AnswerHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_AnswerHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            if(INVALID_CALL == KRIL_GetIncomingCallIndex())
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                // save incoming call index for "future" calls...
                *(UInt8*)pdata->cmdContext = KRIL_GetIncomingCallIndex();
                KRIL_SetIncomingCallIndex(INVALID_CALL);
                CAPI2_CC_GetCallType (GetNewTID(), GetClientID(), *(UInt8*)(pdata->cmdContext));
                pdata->handler_state = BCM_CC_GetCallType;
            }
            break;
        }

        case BCM_CC_GetCallType:
        {
            CCallType_t *callTy = (CCallType_t *)capi2_rsp->dataBuf;
            // call index cached in context field
            UInt8* pCallIndex = (UInt8*)(pdata->cmdContext);

            pdata->handler_state = BCM_RESPCAPI2Cmd;
            if((*callTy == MTDATA_CALL) || (*callTy == MTFAX_CALL)) 
            {
                CAPI2_CC_AcceptDataCall(GetNewTID(), GetClientID(), *pCallIndex);
            } 
            else if(*callTy == MTVIDEO_CALL)
            { 
                CAPI2_CC_AcceptVideoCall(GetNewTID(), GetClientID(), *pCallIndex);
            }
            else if(*callTy == MTVOICE_CALL) 
            {
                CAPI2_CC_AcceptVoiceCall(GetNewTID(), GetClientID(), *pCallIndex);
          
#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
                KRIL_DEBUG(DBG_INFO, "AcceptVoiceCall::Lcd frame inversion\n");
	        			lcd_enter_during_call();
#endif				               
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Unknown call type:%d...!\n", *callTy);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
#ifdef VIDEO_TELEPHONY_ENABLE
            if (capi2_rsp->msgType == MSG_DATACALL_CONNECTED_IND)
            {
                DataCallConnectMsg_t *rsp = (DataCallConnectMsg_t *) capi2_rsp->dataBuf;
                int callIndex = rsp->callIndex;
                
                KRIL_DEBUG(DBG_INFO, "MSG_DATACALL_CONNECTED_IND::callIndex: %d\n",rsp->callIndex);
                KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_CONNECT, &callIndex, sizeof(int));
            }
            else if (capi2_rsp->msgType == MSG_DATACALL_RELEASE_IND)
            {
                DataCallReleaseMsg_t *rsp = (DataCallReleaseMsg_t *) capi2_rsp->dataBuf;
                int callIndex = rsp->callIndex;
                
                KRIL_DEBUG(DBG_INFO, "MSG_DATACALL_RELEASE_IND:: callIndex: %d exitCause: 0x%X\n", rsp->callIndex, rsp->exitCause);
                KRIL_SendNotify(BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_END, &callIndex, sizeof(int));
            }
#endif //VIDEO_TELEPHONY_ENABLE
			pdata->bcm_ril_rsp = NULL;
            pdata->rsp_len = 0;
            if(capi2_rsp->result == RESULT_OK)
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
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


void KRIL_SeparateConnectionHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_SeparateConnectionHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilSeparate_t *tdata = (KrilSeparate_t *)pdata->ril_cmd->data;
            tdata->index--;
            KRIL_DEBUG(DBG_INFO, "call index:%d\n", tdata->index);
            CAPI2_CC_SplitCall(GetNewTID(), GetClientID(), (UInt8) tdata->index);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result == RESULT_OK)
            {
                VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t *)capi2_rsp->dataBuf;
                if(rsp->callResult == CC_SPLIT_CALL_SUCCESS)
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
                else
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
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


void KRIL_ExplicitCallTransferHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_ExplicitCallTransferHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_CC_GetNextActiveCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextActiveCallIndex;
            break;
        }

        case BCM_CC_GetNextActiveCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "ActiveCallIndex:%d\n", *rsp);
            if (*rsp != INVALID_CALL)
            {
                CAPI2_CC_TransferCall(GetNewTID(), GetClientID(), *rsp);
                pdata->handler_state = BCM_CC_TransferCall;
            }
            else
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;
        }

        case BCM_CC_TransferCall:
        {
            if(capi2_rsp->result == RESULT_OK)
            {
                VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t *)capi2_rsp->dataBuf;
				
                KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::rsp->callResult:%d\n", rsp->callResult);
                if(rsp->callResult == CC_TRANS_CALL_SUCCESS || rsp->callResult == RESULT_OK)
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
                else
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
            }
            else
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            break;
        }

        case BCM_CC_GetNextHeldCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "HeldCallIndex:%d\n", *rsp);
            CAPI2_CC_TransferCall(GetNewTID(), GetClientID(), *rsp);
            pdata->handler_state = BCM_CC_TransferCall;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "HeldCallIndex:%d\n", *rsp);
            if (*rsp != INVALID_CALL)
            {
                CAPI2_CC_TransferCall(GetNewTID(), GetClientID(), *rsp);
                pdata->handler_state = BCM_CC_TransferCall;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
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


void KRIL_SendDTMFRequestHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
 {
     KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

     if (capi2_rsp != NULL)
		 KRIL_DEBUG(DBG_INFO, "KRIL_SendDTMFRequestHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            ClientInfo_t clientInfo;
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->inCcCallState = CC_CALL_ACTIVE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_CcApi_GetCallIndexInThisState(&clientInfo, context->inCcCallState);
            pdata->handler_state = BCM_CcApi_SetDtmfTimer;
            break;
        }

        case BCM_CcApi_SetDtmfTimer:
        {
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->DTMFCallIndex = *(UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "CallIndex:%d\n", context->DTMFCallIndex);

            if (context->DTMFCallIndex != INVALID_CALL)
            {
                CAPI2_CcApi_SetDtmfTimer(GetNewTID(), GetClientID(), context->DTMFCallIndex, DTMF_TONE_DURATION_TYPE, 100);
                pdata->handler_state = BCM_CcApi_SendDtmf;
            }
            else if (CC_CALL_ACTIVE == context->inCcCallState)
            {
                ClientInfo_t clientInfo;
                context->inCcCallState = CC_CALL_CONNECTED;
                CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                CAPI2_CcApi_GetCallIndexInThisState(&clientInfo, context->inCcCallState);
                pdata->handler_state = BCM_CcApi_SetDtmfTimer;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }

        case BCM_CcApi_SendDtmf:
        {
            ApiDtmf_t DtmfObjPtr;
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;

            DTMFString = ((char *)pdata->ril_cmd->data)[0];
            DtmfObjPtr.callIndex = context->DTMFCallIndex;
            DtmfObjPtr.dtmfIndex = 0;
            DtmfObjPtr.dtmfTone = DTMFString;
            DtmfObjPtr.duration = 100;
            DtmfObjPtr.toneValume = 0;
            DtmfObjPtr.isSilent = FALSE;
            KRIL_DEBUG(DBG_INFO, "DTMFCallIndex:%d DTMFString:%c\n", context->DTMFCallIndex, DTMFString);
            CAPI2_CcApi_SendDtmf(GetNewTID(), GetClientID(), &DtmfObjPtr);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_TRACE, "capi2_rsp->result:%d\n", capi2_rsp->result);
                pdata->result = RILErrorResult(capi2_rsp->result);
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
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SendDTMFStartHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
		KRIL_DEBUG(DBG_INFO, "KRIL_SendDTMFStartHandler handler_state:0x%lX::result:%d::msgType:0x%1X\n", pdata->handler_state, capi2_rsp->result,capi2_rsp->msgType);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            ClientInfo_t clientInfo;
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->inCcCallState = CC_CALL_ACTIVE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_CcApi_GetCallIndexInThisState(&clientInfo, context->inCcCallState);
            pdata->handler_state = BCM_CcApi_ResetDtmfTimer;
            break;
        }

        case BCM_CcApi_ResetDtmfTimer:
        {
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->DTMFCallIndex = * (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "CallIndex:%d inCcCallState:%d\n", context->DTMFCallIndex, context->inCcCallState);
 
            if (context->DTMFCallIndex != INVALID_CALL)
            {
                CAPI2_CcApi_ResetDtmfTimer(GetNewTID(), GetClientID(), context->DTMFCallIndex, DTMF_TONE_DURATION_TYPE);
                pdata->handler_state = BCM_CcApi_SendDtmf;
            }
            else if (CC_CALL_ACTIVE == context->inCcCallState)
            {
                ClientInfo_t clientInfo;
                context->inCcCallState = CC_CALL_CONNECTED;
                CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                CAPI2_CcApi_GetCallIndexInThisState(&clientInfo, context->inCcCallState);
                pdata->handler_state = BCM_CcApi_ResetDtmfTimer;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            break;
        }

        case BCM_CcApi_SendDtmf:
        {
            ApiDtmf_t DtmfObjPtr;
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;

            DTMFString = ((char *)pdata->ril_cmd->data)[0];
            DtmfObjPtr.callIndex = context->DTMFCallIndex;
            DtmfObjPtr.dtmfIndex = 0;
            DtmfObjPtr.dtmfTone = DTMFString;
            DtmfObjPtr.duration = 0;
            DtmfObjPtr.toneValume = 0;
            DtmfObjPtr.isSilent = FALSE;
            KRIL_DEBUG(DBG_INFO, "DTMFCallIndex:%d DTMFString:%c\n", context->DTMFCallIndex, DTMFString);
            CAPI2_CcApi_SendDtmf(GetNewTID(), GetClientID(), &DtmfObjPtr);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_TRACE, "capi2_rsp->result:%d\n", capi2_rsp->result);
                pdata->result = RILErrorResult(capi2_rsp->result);
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
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SendDTMFStopHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            ClientInfo_t clientInfo;
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->inCcCallState = CC_CALL_ACTIVE;
            CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
            CAPI2_CcApi_GetCallIndexInThisState(&clientInfo, context->inCcCallState);
            pdata->handler_state = BCM_CcApi_SendDtmf;
            break;
        }

        case BCM_CcApi_SendDtmf:
        {
            UInt8 DTMFCallIndex = * (UInt8 *)capi2_rsp->dataBuf;
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            KRIL_DEBUG(DBG_INFO, "CallIndex:%d\n", DTMFCallIndex);

            if (DTMFCallIndex != INVALID_CALL)
            {
                ApiDtmf_t DtmfObjPtr;
                DtmfObjPtr.callIndex = DTMFCallIndex;
                DtmfObjPtr.dtmfIndex = 0;
                DtmfObjPtr.dtmfTone = DTMFString;
                DtmfObjPtr.duration = 0;
                DtmfObjPtr.toneValume = 0;
                DtmfObjPtr.isSilent = FALSE;
                KRIL_DEBUG(DBG_INFO, "DTMFCallIndex:%d DTMFString:%c\n", DTMFCallIndex, DTMFString);
                CAPI2_CcApi_StopDtmf(GetNewTID(), GetClientID(), &DtmfObjPtr);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else if (CC_CALL_ACTIVE == context->inCcCallState)
            {
                ClientInfo_t clientInfo;
                context->inCcCallState = CC_CALL_CONNECTED;
                CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                CAPI2_CcApi_GetCallIndexInThisState(&clientInfo, context->inCcCallState);
                pdata->handler_state = BCM_CcApi_SendDtmf;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_TRACE, "capi2_rsp->result:%d\n", capi2_rsp->result);
                pdata->result = RILErrorResult(capi2_rsp->result);
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
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

