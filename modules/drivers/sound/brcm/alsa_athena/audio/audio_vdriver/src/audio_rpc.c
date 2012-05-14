/****************************************************************************
*
*     Copyright (c) 2010 Broadcom Corporation. All rights reserved.
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
//include "string.h"
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#if defined(FUSE_APPS_PROCESSOR)
#include "consts.h"

#include "xdr_porting_layer.h"
#include "xdr.h"

#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "common_sim.h"
#include "sim_def.h"
#include "engmode_api.h"

#include "i2c_drv.h"
#include "common_ecdc.h"
#include "uelbs_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "logapi.h"
#include "tones_def.h"
#include "ostypes.h"
#include "phonebk_def.h"
#include "phonectrl_def.h"
#include "phonectrl_api.h"
#include "rtc.h"
#include "netreg_def.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "netreg_util.h"
#include "netreg_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "ss_def.h"
#include "sim_api.h"
#include "phonectrl_def.h"
#include "isim_def.h"
#include "pch_def.h"
#include "pchex_def.h"
#include "hal_em_battmgr.h"
#include "cc_def.h"
#include "rtem_events.h"
#include "rtc.h"
#include "engmode_def.h"
#include "sms_def.h"
#include "simlock_def.h"
#include "isim_def.h"
#include "pch_def.h"
#include "pchex_def.h"

#include "engmode_api.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "ss_api.h"
#include "sms_api_atc.h"
#include "sms_api_old.h"
#include "sms_api.h"
#include "cc_api_old.h"
#include "sim_api.h"
#include "phonebk_api_old.h"
#include "phonebk_api.h"
#include "phonectrl_api.h"
#include "isim_api_old.h"
#include "isim_api.h"

#include "util_api.h"
#include "dialstr_api.h"
#include "stk_api_old.h"
#include "stk_api.h"

#include "pch_api_old.h"
#include "pch_api.h"
#include "pchex_api.h"
#include "ss_api_old.h"
#include "lcs_cplane_rrlp_api.h"
#include "cc_api.h"
#include "netreg_util.h"
#include "netreg_api.h"
#include "lcs_ftt_api.h"
#include "lcs_cplane_rrc_api.h"
#include "lcs_cplane_shared_def.h"

#include "capi2_mstruct.h"
#include "capi2_sim_api.h"
#include "capi2_phonectrl_api.h"
#include "capi2_sms_api.h"
#include "capi2_cc_api.h"
#include "capi2_lcs_cplane_api.h"
#include "capi2_ss_api.h"
#include "capi2_phonebk_api.h"
#include "capi2_cmd_resp.h"
#include "capi2_phonectrl_api.h"
#endif

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#if defined(FUSE_APPS_PROCESSOR)
#include "capi2_global.h"
#include "capi2_mstruct.h"
#include "capi2_cc_ds.h"
#include "capi2_cc_msg.h"
#include "capi2_msnu.h"
#include "ss_api_old.h"
#include "ss_lcs_def.h"
#include "lcs_cplane_api.h"
#include "lcs_cplane_rrc_api.h"
#include "lcs_cplane_shared_def.h"
#include "lcs_ftt_api.h"
#include "capi2_gen_api.h"
#include "rpc_sync_api.h"
#include "capi2_stk_api.h"

#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#define MAX_CC_GET_ELEM_SIZE  64
#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"
#endif

#include "xassert.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "audio_rpc.h"
#include "rpc_sync_api.h"

#include "dspcmd.h"
#include "ripcmdq.h"
#include "audioapi_asic.h"

#define Log_DebugPrintf(logID,fmt,args...)

static UInt8 audioClientId = 0;
static RPC_Handle_t rpcHandle ;
static call_codecId_handler_t client_call_codecId_handler = NULL;

//If this struct is changed then please change xdr_Audio_Params_t() also.
typedef struct
{
	UInt32 param1;
	UInt32 param2;
	UInt32 param3;
	UInt32 param4;
	UInt32 param5;
	UInt32 param6;
}Audio_Params_t;

static bool_t xdr_Audio_Params_t(void* xdrs, Audio_Params_t *rsp);
#define _T(a) a

/*************************************  FRAMEWORK CODE *******************************************************************/
static RPC_XdrInfo_t AUDIO_Prim_dscrm[] = {
	
	/* Add phonebook message serialize/deserialize routine map */
	{ MSG_AUDIO_CTRL_GENERIC_REQ,_T("MSG_AUDIO_CTRL_GENERIC_REQ"), (xdrproc_t) xdr_Audio_Params_t,  sizeof( Audio_Params_t ), 0},
	{ MSG_AUDIO_CTRL_GENERIC_RSP,_T("MSG_AUDIO_CTRL_GENERIC_RSP"), (xdrproc_t)xdr_UInt32, sizeof( UInt32 ), 0 },
	{ MSG_AUDIO_CTRL_DSP_REQ,_T("MSG_AUDIO_CTRL_DSP_REQ"), (xdrproc_t) xdr_Audio_Params_t,  sizeof( Audio_Params_t ), 0},
	{ MSG_AUDIO_CTRL_DSP_RSP,_T("MSG_AUDIO_CTRL_DSP_RSP"), (xdrproc_t)xdr_UInt32, sizeof( UInt32 ), 0 },
	{ (MsgType_t)__dontcare__, "",NULL_xdrproc_t, 0,0 } 
};

void register_call_codecId_handler( call_codecId_handler_t call_codecId_cb )
{
	Log_DebugPrintf(LOGID_MISC, "\n\r\t* AP register_call_codecId_handler, 0x%x\n\r", call_codecId_cb);
	client_call_codecId_handler = call_codecId_cb;
}

#if defined(FUSE_COMMS_PROCESSOR) 
static Result_t SendAudioRspForRequest(RPC_Msg_t* req, MsgType_t msgType, void* payload)
{
	RPC_Msg_t rsp;
	
	rsp.msgId = msgType;
	rsp.tid = req->tid;
	rsp.clientID = req->clientID;
	rsp.dataBuf = (void*)payload;
	rsp.dataLen = 0;
	
	return RPC_SerializeRsp(&rsp);
}
#endif

#if defined(FUSE_APPS_PROCESSOR)
void HandleAudioEventrespCb(RPC_Msg_t* pMsg,
                            ResultDataBufHandle_t dataBufHandle,
                            UInt32 userContextData)
{
    if ( pMsg->tid == 0 )
    {
        if (MSG_CALL_STATUS_IND == pMsg->msgId )
        {
            UInt32 len;
            void* dataBuf;
            CallStatusMsg_t* p = NULL;
            CAPI2_ReqRep_t * const reqRep = (CAPI2_ReqRep_t*)pMsg->dataBuf;

            Log_DebugPrintf(LOGID_MISC, "HandleAudioEventrespCb : ===pMsg->tid=0x%x, pMsg->clientID=0x%x, pMsg->msgId=0x%x \r\n",(unsigned int)pMsg->tid, pMsg->clientID, pMsg->msgId);

            CAPI2_GetPayloadInfo(reqRep, pMsg->msgId, &dataBuf, &len);

            p = (CallStatusMsg_t*)dataBuf;

            Log_DebugPrintf(LOGID_MISC, "HandleAudioEventrespCb : codecid=0x%x, callstatus=0x%x \r\n",p->codecId, p->callstatus);

            if ( client_call_codecId_handler && p->codecId != 0 && p->callstatus == CC_CALL_SYNC_IND)
            {
                Log_DebugPrintf(LOGID_MISC, "HandleAudioEventrespCb : calling client_call_codecId_handler\r\n");
                client_call_codecId_handler(p->codecId);
            }
        }
    }

    if ( dataBufHandle )
    {
        RPC_SYSFreeResultDataBuffer(dataBufHandle);
    }
    else
        Log_DebugPrintf(LOGID_MISC, "HandleAudioEventrespCb : dataBufHandle is NULL \r\n");
}
#endif

void HandleAudioEventReqCb(RPC_Msg_t* pMsg, 
						 ResultDataBufHandle_t dataBufHandle, 
						 UInt32 userContextData)
{
	Log_DebugPrintf(LOGID_MISC, "HandleAudioEventRspCb msg=0x%x clientID=%d ", pMsg->msgId, 0);

#if defined(FUSE_COMMS_PROCESSOR) 

	 RPC_SendAckForRequest(dataBufHandle, 0);

	if(pMsg->msgId == MSG_AUDIO_CTRL_GENERIC_REQ)
	{
		Audio_Params_t* p = (Audio_Params_t*)pMsg->dataBuf;
		UInt32 val = audio_control_generic(p->param1,p->param2,p->param3,p->param4,p->param5,p->param6);

		SendAudioRspForRequest(pMsg, MSG_AUDIO_CTRL_GENERIC_RSP, &val);
	}
	else if(pMsg->msgId == MSG_AUDIO_CTRL_DSP_REQ)
	{
		Audio_Params_t* p = (Audio_Params_t*)pMsg->dataBuf;
		UInt32 val = audio_control_dsp(p->param1,p->param2,p->param3,p->param4,p->param5,p->param6);
		
		SendAudioRspForRequest(pMsg, MSG_AUDIO_CTRL_DSP_RSP, &val);
	}
	else
		xassert(0, pMsg->msgId);
#endif

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

static Boolean AudioCopyPayload( MsgType_t msgType, 
						 void* srcDataBuf, 
						 UInt32 destBufSize,
						 void* destDataBuf, 
						 UInt32* outDestDataSize, 
						 Result_t *outResult)
{
	UInt32 len;

	xassert(srcDataBuf != NULL, 0);

	len = RPC_GetMsgPayloadSize(msgType);

	*outResult = RESULT_OK;
	*outDestDataSize = len;

	if(destDataBuf && srcDataBuf && len <= destBufSize)
	{
		memcpy(destDataBuf, srcDataBuf, len);
		return TRUE;
	}
	return FALSE;
}



void Audio_InitRpc(void)
{
    static int first_time = 1;

    if(first_time)
    {
        RPC_InitParams_t params={0};
        RPC_SyncInitParams_t syncParams;

        params.iType = INTERFACE_RPC_DEFAULT;
        params.table_size = (sizeof(AUDIO_Prim_dscrm)/sizeof(RPC_XdrInfo_t));
        params.xdrtbl = AUDIO_Prim_dscrm;
#if defined(FUSE_APPS_PROCESSOR)
        params.respCb = HandleAudioEventrespCb;
#else
        params.respCb = NULL;
#endif
        params.reqCb = HandleAudioEventReqCb;
        syncParams.copyCb = AudioCopyPayload;

        rpcHandle = RPC_SyncRegisterClient(&params,&syncParams);
        audioClientId = RPC_SYS_GetClientID(rpcHandle);
#if defined(FUSE_APPS_PROCESSOR)
        {
            UInt16 regMsgIds[]={ MSG_CALL_STATUS_IND};
            RPC_RegisterUnsolicitedMsgs(rpcHandle, regMsgIds, sizeof(regMsgIds)/sizeof(UInt16) );
            RPC_EnableUnsolicitedMsgs(rpcHandle, TRUE);
        }
#endif
        //audioClientId = RPC_SyncRegisterClient(&params,&syncParams);

        first_time = 0;
        Log_DebugPrintf(LOGID_MISC, "Audio_InitRpc %d", audioClientId);
    }
}

/*************************************  AUDIO API CODE *******************************************************************/


void CAPI2_audio_control_generic(UInt32 tid, UInt8 clientID, Audio_Params_t* params)
{
	RPC_Msg_t msg;

	msg.msgId = MSG_AUDIO_CTRL_GENERIC_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)params;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_audio_control_dsp(UInt32 tid, UInt8 clientID, Audio_Params_t* params)
{
	RPC_Msg_t msg;
	
	msg.msgId = MSG_AUDIO_CTRL_DSP_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)params;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

bool_t xdr_Audio_Params_t(void* xdrs, Audio_Params_t *rsp)
{
	XDR_LOG(xdrs,"Audio_Params_t")

	if(
		xdr_UInt32(xdrs, &rsp->param1) &&
		xdr_UInt32(xdrs, &rsp->param2) &&
		xdr_UInt32(xdrs, &rsp->param3) &&
		xdr_UInt32(xdrs, &rsp->param4) &&
		xdr_UInt32(xdrs, &rsp->param5) &&
		xdr_UInt32(xdrs, &rsp->param6) &&
	1)
		return TRUE;
	else
		return FALSE;
}

#if defined(FUSE_APPS_PROCESSOR) 

UInt32 audio_control_generic(UInt32 param1,UInt32 param2,UInt32 param3,UInt32 param4,UInt32 param5,UInt32 param6)
{
	Audio_Params_t audioParam;
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	UInt32 val = (UInt32)0;

	audioParam.param1 = param1;
	audioParam.param2 = param2;
	audioParam.param3 = param3;
	audioParam.param4 = param4;
	audioParam.param5 = param5;
	audioParam.param6 = param6;

	tid = RPC_SyncCreateTID( &val, sizeof( UInt32 ) );
	CAPI2_audio_control_generic(tid, audioClientId,&audioParam);
	RPC_SyncWaitForResponse( tid,audioClientId, &ackResult, &msgType, NULL );
	return val;
}

UInt32 audio_control_dsp(UInt32 param1,UInt32 param2,UInt32 param3,UInt32 param4,UInt32 param5,UInt32 param6)
{
	Audio_Params_t audioParam;
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	UInt32 val = (UInt32)0;
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* audio_control_dsp (AP) param1 %d, param2 %d param3 %d param4 %d *\n\r", param1, param2, param3, param4);

	switch (param1)
	{

#if 0
		case DSPCMD_TYPE_COMMAND_DIGITAL_SOUND:
			RIPCMDQ_DigitalSound((UInt16)param2);
			
			break;

		case DSPCMD_TYPE_COMMAND_SET_ARM2SP:
			RIPCMDQ_SetARM2SP((UInt16)param2, (UInt16)param3);
			
			break;

		case DSPCMD_TYPE_COMMAND_SET_ARM2SP2:
			RIPCMDQ_SetARM2SP2((UInt16)param2, (UInt16)param3);
			
			break;
			
		case DSPCMD_TYPE_COMMAND_SET_BT_NB:
			RIPCMDQ_SetBTNarrowBand((UInt16)param2);
			
			break;

		case DSPCMD_TYPE_COMMAND_USB_HEADSET:
			RIPCMDQ_USBHeadset((UInt16)param2);

			break;		


		case DSPCMD_TYPE_MM_VPU_ENABLE:
				RIPCMDQ_MMVPUEnable((UInt16)param2);
				
			break;
			
		case DSPCMD_TYPE_MM_VPU_DISABLE:
				RIPCMDQ_MMVPUDisable();
				
			break;
			
	// AMCR PCM enable bit is controlled by ARM audio		
		case DSPCMD_TYPE_AUDIO_SET_PCM:
			RIPCMDQ_DigitalSound((UInt16)param2);

			break;			


		case DSPCMD_TYPE_SET_VOCODER_INIT:
			RIPCMD_Vocoder_Init((UInt16)param2);
			
			break;		
		case DSPCMD_TYPE_COMMAND_VOIF_CONTROL:
			RIPCMDQ_VOIFControl((UInt16)param2);
			
			break;	 

		case DSPCMD_TYPE_COMMAND_SP:
			RIPCMDQ_SP((UInt16)param2, (UInt16)param3, (UInt16)param4);
			
			break;	

		case DSPCMD_TYPE_COMMAND_CLEAR_VOIPMODE:
			RIPCMDQ_Clear_VoIPMode( (UInt16) param2 );
			break;
#endif			
		default:
            Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* audio_control_dsp (AP) default case param1 %d, param2 %d param3 %d param4 %d *\n\r", param1, param2, param3, param4);

			audioParam.param1 = param1;
			audioParam.param2 = param2;
			audioParam.param3 = param3;
			audioParam.param4 = param4;
			audioParam.param5 = param5;
			audioParam.param6 = param6;

			tid = RPC_SyncCreateTID( &val, sizeof( UInt32 ) );
			CAPI2_audio_control_dsp(tid, audioClientId,&audioParam);
			RPC_SyncWaitForResponse( tid,audioClientId, &ackResult, &msgType, NULL );
	
			break;
	} 
	
	return val;

}

#endif

