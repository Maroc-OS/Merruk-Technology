/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/
/**
*
* @file   audio_vdriver_common.c
* @brief  Audio VDriver API common for all HW platforms.
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "auddrv_def.h"
//#include "brcm_rdb_sysmap.h"
//#include "brcm_rdb_syscfg.h"
#define SYSCFG_BASE_ADDR      0x08880000      /* SYSCFG core */
#include "shared.h"
#include "dspcmd.h"
#include "ripcmdq.h"
//#include "ripisr.h"
#include "audio_consts.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "audio_ipc.h"
#ifdef UNDER_LINUX
#include "linux/broadcom/bcm_fuse_sysparm_CIB.h"
#else
#include "sysparm.h"
#endif
#include "ostask.h"
#include "audioapi_asic.h"
#include "log.h"
//#include "vpripcmdq.h"
#include "csl_apcmd.h"
#include "osdw_dsp_drv.h"
/**
*
* @addtogroup AudioDriverGroup
* @{
*/

//=============================================================================
// Public Variable declarations
//=============================================================================
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) && defined(IPC_AUDIO))
extern void CP_Audio_ISR_Handler(StatQ_t status_msg);
#else
extern void AP_Audio_ISR_Handler(StatQ_t status_msg);
#endif

#if !(defined(FUSE_APPS_PROCESSOR) && (defined(IPC_FOR_BSP_ONLY) || defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE)))
#else
extern void IPC_Audio_Create_BufferPool( void );
#endif

#ifdef VPU_INCLUDED
#if !(defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) )
extern void AP_VPU_ProcessStatus( void );
#endif
#endif

extern AUDDRV_MIC_Enum_t   currVoiceMic;   //used in pcm i/f control. assume one mic, one spkr.
extern AUDDRV_SPKR_Enum_t  currVoiceSpkr;  //used in pcm i/f control. assume one mic, one spkr.
extern Boolean inVoiceCall;
//=============================================================================
// Private Type and Constant declarations
//=============================================================================


Boolean voicePlayOutpathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN
extern Boolean voiceInPathEnabled ;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN for both voiceIn and voiceOut
Boolean controlFlagForCustomGain = FALSE;

void VPSHAREDMEM_Init( void );
static UInt32 voiceCallSampleRate = 8000;  // defalut to 8K Hz

static Boolean dspECEnable = TRUE;
static Boolean dspNSEnable = TRUE;
static Boolean voiceULMute = FALSE;

#if defined(FUSE_APPS_PROCESSOR)
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:      AUDIO_GetParmAccessPtr
//!
//! Description:        audio parm acces for FUSE AP only
//!
/////////////////////////////////////////////////////////////////////////////
static SysAudioParm_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
        return NULL;
#else
        return APSYSPARM_GetAudioParmAccessPtr();
#endif
}

#define AUDIOMODE_PARM_ACCESSOR(app,mode)        ((SysAudioParm_t *)((SysAudioParm_t *)AUDIO_GetParmAccessPtr()+app*AUDIO_MODE_NUMBER))[mode]
#endif
//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

void Audio_InitRpc();
//=============================================================================
//
// Function Name: AUDDRV_Init
//
// Description:   Initialize audio system
//
//=============================================================================
void AUDDRV_Init( void )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Init *\n\r");
#if defined(FUSE_APPS_PROCESSOR)

    

#ifdef UNDER_LINUX


    // Register proper handler here
#ifdef VPU_INCLUDED
	RIPISR_Register_VPU_ProcessStatus( (VPU_ProcessStatus_t) &AP_VPU_ProcessStatus );   //not used in runtime?
#endif
   // create the IP here because it will take some time. Otherwise, the first IPC message will take long time.
	IPC_Audio_Create_BufferPool();
	Audio_Apps_EP_Register( );


#else
		RIPISR_Register_AudioISR_Handler( (Audio_ISR_Handler_t) &AP_Audio_ISR_Handler );  //not used in runtime?
#ifdef VPU_INCLUDED
	RIPISR_Register_VPU_ProcessStatus( (VPU_ProcessStatus_t) &AP_VPU_ProcessStatus );   //not used in runtime?
#endif
#endif


	AUDDRV_RegisterCB_getAudioMode( (CB_GetAudioMode_t) &AUDDRV_GetAudioMode );
	AUDDRV_RegisterCB_setAudioMode( (CB_SetAudioMode_t) &AUDDRV_SetAudioMode );
	AUDDRV_RegisterCB_setMusicMode( (CB_SetMusicMode_t) &AUDDRV_SetMusicMode );
	AUDDRV_RegisterCB_getAudioApp( (CB_GetAudioApp_t) &AUDDRV_GetAudioApp );

    // Initialize RPC and shared memory here
#ifdef UNDER_LINUX
    Audio_InitRpc();
    //VPSHAREDMEM_Init();
    DSPDRV_Init();

#endif

#else  //#if defined(FUSE_APPS_PROCESSOR)


#if !defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE)
		// ATHENA_EDGE_CP_STANDALONE build
	RIPISR_Register_AudioISR_Handler( (Audio_ISR_Handler_t) &AP_Audio_ISR_Handler );  //not used in runtime?
	#ifdef VPU_INCLUDED
	RIPISR_Register_VPU_ProcessStatus( (VPU_ProcessStatus_t) &AP_VPU_ProcessStatus );   //not used in runtime?
	#endif

	AUDDRV_RegisterCB_getAudioMode( (CB_GetAudioMode_t) &AUDDRV_GetAudioMode );
	AUDDRV_RegisterCB_setAudioMode( (CB_SetAudioMode_t) &AUDDRV_SetAudioMode );
	AUDDRV_RegisterCB_setMusicMode( (CB_SetMusicMode_t) &AUDDRV_SetMusicMode );

#else
	RIPISR_Register_AudioISR_Handler( (Audio_ISR_Handler_t) &CP_Audio_ISR_Handler );

#endif
#endif	
}


// CSL driver will send a DSP_ENABLE_DIGITAL_SOUND?? cmd to DSP, 
// But ARM code (audio controller) turns on/off PCM interface.

//=============================================================================
//
// Function Name: AUDDRV_Telephony_Init
//
// Description:   Initialize audio system for voice call
//
//=============================================================================

//Prepare DSP before turn on hardware audio path for voice call.
//  This is part of the control sequence for starting telephony audio.
void AUDDRV_Telephony_Init ( AUDDRV_MIC_Enum_t  mic, AUDDRV_SPKR_Enum_t speaker )
{
        Boolean ec_enable_from_sysparm = dspECEnable;
        Boolean ns_enable_from_sysparm = dspNSEnable;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_Init");

        if ( ec_enable_from_sysparm == TRUE )
        {
            AudioMode_t audio_mode;
            audio_mode = AUDDRV_GetAudioMode();
            audio_mode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER);

            Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_Init: echo_cancelling_enable from sysparm \n\r");
            ec_enable_from_sysparm = AUDIOMODE_PARM_ACCESSOR(AUDDRV_GetAudioApp(), audio_mode).echo_cancelling_enable;
        }

        if ( ns_enable_from_sysparm == TRUE )
        {
            AudioMode_t audio_mode;
            audio_mode = AUDDRV_GetAudioMode();
            audio_mode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER);

            Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_Init: noise_suppression_enable from sysparm \n\r");
            ns_enable_from_sysparm = AUDIOMODE_PARM_ACCESSOR(AUDDRV_GetAudioApp(), audio_mode).noise_suppression_enable;
        }

	currVoiceMic = mic;
	currVoiceSpkr = speaker;

#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Init AP  mic %d, spkr %d *\n\r", mic, speaker);

	//control HW and flags at AP

	//at beginning
	inVoiceCall = TRUE;  //to prevent sending DSP Audio Enable when enable voice path.

	audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, FALSE, FALSE, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0 );

	//only check voiceCallSampleRate
    if (voiceCallSampleRate == 16000) 
    {
		AUDDRV_SetAudioMode( (AudioMode_t)(AUDDRV_GetAudioMode()% AUDIO_MODE_NUMBER), AUDIO_APP_VOICE_CALL_WB );  //WB
		AUDDRV_Telephony_InitHW ( mic, speaker, AUDIO_SAMPLING_RATE_16000 );
    }
	else
    {
		AUDDRV_SetAudioMode( (AudioMode_t)(AUDDRV_GetAudioMode()% AUDIO_MODE_NUMBER), AUDIO_APP_VOICE_CALL );  //NB
		AUDDRV_Telephony_InitHW ( mic, speaker, AUDIO_SAMPLING_RATE_8000 );
    }

	audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, TRUE, 0, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0 );
	//after AUDDRV_Telephony_InitHW to make SRST.
	AUDDRV_SetVCflag(TRUE);  //let HW control logic know.

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, TRUE, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0, 0 );

	OSTASK_Sleep( 40 );

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, TRUE, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0, 0 );
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Init dspECEnable = %d, dspNSEnable = %d *\n\r", ec_enable_from_sysparm, ns_enable_from_sysparm);
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, ec_enable_from_sysparm, ns_enable_from_sysparm, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, TRUE, 0, 0, 0, 0 );

	if ( !voiceULMute )
	    audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );

	  //per call basis: enable the DTX by calling stack api when call connected
	audio_control_generic( AUDDRV_CPCMD_ENABLE_DSP_DTX, TRUE, 0, 0, 0, 0 );

	if (speaker == AUDDRV_SPKR_PCM_IF)
		AUDDRV_SetPCMOnOff( 1 );
	else
	{
		if(currVoiceMic != AUDDRV_MIC_PCM_IF) //need to check mic too.
			AUDDRV_SetPCMOnOff( 0 );
	}
#endif

	return;
}


//=============================================================================
//
// Function Name: AUDDRV_Telephony_RateChange
//
// Description:   Change the sample rate for voice call
//
//=============================================================================

void AUDDRV_Telephony_RateChange( UInt32 rate )
{
        Boolean ec_enable_from_sysparm = dspECEnable;
        Boolean ns_enable_from_sysparm = dspNSEnable;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_RateChange: rate = %d\n\r", rate);

#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_RateChange AP  *\n\r");

        if ( ec_enable_from_sysparm == TRUE )
        {
            AudioMode_t audio_mode;
            audio_mode = AUDDRV_GetAudioMode();
            audio_mode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER);

            Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_RateChange: reading echo_cancelling_enable from sysparm \n\r");
            ec_enable_from_sysparm = AUDIOMODE_PARM_ACCESSOR(AUDDRV_GetAudioApp(), audio_mode).echo_cancelling_enable;
        }

        if ( ns_enable_from_sysparm == TRUE )
        {
            AudioMode_t audio_mode;
            audio_mode = AUDDRV_GetAudioMode();
            audio_mode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER);

            Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_RateChange: noise_suppression_enable from sysparm \n\r");
            ns_enable_from_sysparm = AUDIOMODE_PARM_ACCESSOR(AUDDRV_GetAudioApp(), audio_mode).noise_suppression_enable;
        }

    voiceCallSampleRate = rate;  //remember the rate for current call. (or for the incoming call in ring state.)

	if ( AUDDRV_GetVCflag() )
	{

	audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, FALSE, FALSE, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0 );

	//need to know the mode!  need to set HW to 16Khz.
    if (voiceCallSampleRate == 8000)
		AUDDRV_SetAudioMode( (AudioMode_t)(AUDDRV_GetAudioMode()% AUDIO_MODE_NUMBER), AUDIO_APP_VOICE_CALL);  //NB
    else
		AUDDRV_SetAudioMode( (AudioMode_t)(AUDDRV_GetAudioMode()% AUDIO_MODE_NUMBER), AUDIO_APP_VOICE_CALL_WB);

	//AUDDRV_Enable_Output (AUDDRV_VOICE_OUTPUT, speaker, TRUE, AUDIO_SAMPLING_RATE_8000);
	audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, TRUE, 0, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0 );

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, TRUE, 0, 0, 0, 0 );

	//AUDDRV_Enable_Input ( AUDDRV_VOICE_INPUT, mic, AUDIO_SAMPLING_RATE_8000);
			
	OSTASK_Sleep( 40 );
	
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, TRUE, 0, 0, 0, 0 );
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_RateChange AP dspECEnable = %d, dspNSEnable = %d *\n\r", ec_enable_from_sysparm, ns_enable_from_sysparm);
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, ec_enable_from_sysparm, ns_enable_from_sysparm, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, TRUE, 0, 0, 0, 0 );

	if ( !voiceULMute )
	    audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );
	}
#endif

	return;
}

//=============================================================================
//
// Function Name: AUDDRV_SetTelephonyMicMute
//
// Description:   UL Mute/Unmute
//
//=============================================================================
void AUDDRV_SetTelephonyMicMute(Boolean mute)
{
    if( voiceULMute != mute )
    {
        voiceULMute = mute;

        if( AUDDRV_GetVCflag() )
        {
            if ( !voiceULMute )
            {
                Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetTelephonyMicMute : DSP UL is unmuted \n\r");
                audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );
            }
            else
            {
                Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetTelephonyMicMute : DSP UL is muted \n\r");
                audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );
            }
        }
    }
}

//=============================================================================
//
// Function Name: AUDDRV_EC
//
// Description:   DSP Echo cancellation ON/OFF
//
//=============================================================================
void AUDDRV_EC(Boolean enable, UInt32 arg)
{
    dspECEnable = enable;
}

//=============================================================================
//
// Function Name: AUDDRV_NS
//
// Description:   DSP Noise Suppression ON/OFF
//
//=============================================================================
void AUDDRV_NS(Boolean enable)
{
    dspNSEnable = enable;
}

//=============================================================================
//
// Function Name: AUDDRV_ECreset_NLPoff
//
// Description:   DSP control to turn EC ON but NLP OFF, regardless of sysparm
//
//=============================================================================
void AUDDRV_ECreset_NLPoff(Boolean ECenable)
{
    audio_control_dsp( DSPCMD_TYPE_CONTROL_ECHO_CANCELLATION, ECenable, 0, 0, 0, 0 );
}

//=============================================================================
//
// Function Name: AUDDRV_Telephony_Deinit
//
// Description:   DeInitialize audio system for voice call
//
//=============================================================================

//Prepare DSP before turn off hardware audio path for voice call. 
// This is part of the control sequence for ending telephony audio.
void AUDDRV_Telephony_Deinit (void )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Deinit voicePlayOutpathEnabled = %d*\n\r", voicePlayOutpathEnabled);
#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)	
	AUDDRV_SetVCflag(FALSE);  //let HW control logic know.

	// a quick fix not to disable voice path for speech playbck or recording when end the phone call.
	if ((voicePlayOutpathEnabled == FALSE) && (voiceInPathEnabled == FALSE))
	{
		  //per call basis: disable the DTX by calling stack api when call disconnected
		audio_control_generic( AUDDRV_CPCMD_ENABLE_DSP_DTX, FALSE, 0, 0, 0, 0 );

		audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, FALSE, FALSE, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, FALSE, 0, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, FALSE, 0, 0, 0, 0 );
	
		//audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, FALSE, 0, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );

		//OSTASK_Sleep( 3 ); //make sure audio is off

		AUDDRV_Telephony_DeinitHW( );
		audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, FALSE, 0, 0, 0, 0 );
	}

	if (AUDIO_CHNL_BLUETOOTH == AUDDRV_GetAudioMode() )
		VPRIPCMDQ_DigitalSound( FALSE );

	//at last
    voiceCallSampleRate = 8000;  //reset it to 8KHz,
	inVoiceCall = FALSE;
#endif
	return;
}

//=============================================================================
//
// Function Name: AUDDRV_IsVoiceCallWB
//
// Description:   Return Wideband Voice Call status
//
//=============================================================================

Boolean AUDDRV_IsVoiceCallWB(AudioMode_t audio_mode)
{
	Boolean is_wb;
	if(audio_mode<AUDIO_MODE_NUMBER)
		is_wb = FALSE;
	else
		is_wb = TRUE;
	return is_wb;
}

//=============================================================================
//
// Function Name: AUDDRV_IsCall16K
//
// Description:   Return Voice Call 16KHz sample rate status
//
//=============================================================================

Boolean AUDDRV_IsCall16K(AudioMode_t voiceMode)
{
	Boolean is_call16k = FALSE;
	switch(voiceMode)
		{
		case	AUDIO_MODE_HANDSET_WB:
		case	AUDIO_MODE_HEADSET_WB:
		case	AUDIO_MODE_HANDSFREE_WB:
		//case	AUDIO_MODE_BLUETOOTH_WB:
		case	AUDIO_MODE_SPEAKERPHONE_WB:
		case	AUDIO_MODE_TTY_WB:
		case	AUDIO_MODE_HAC_WB:
		case	AUDIO_MODE_USB_WB:
		case	AUDIO_MODE_RESERVE_WB:
			is_call16k = TRUE;
			break;
		default:
			break;
		}
	return is_call16k;
}
//=============================================================================
//
// Function Name: AUDDRV_InVoiceCall
//
// Description:   Return Voice Call status
//
//=============================================================================

Boolean AUDDRV_InVoiceCall( void )
{
	return inVoiceCall;
}

//=============================================================================
//
// Function Name: AUDDRV_ControlFlagFor_CustomGain
//
// Description:   Set a flag to allow custom gain settings.
//				  If the flag is set the above three parameters are not set 
//				  in AUDDRV_SetAudioMode( ) .
//
//=============================================================================

void AUDDRV_ControlFlagFor_CustomGain( Boolean on_off )
{
	controlFlagForCustomGain = on_off;
}

