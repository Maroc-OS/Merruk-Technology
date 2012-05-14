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
* @file   audio_controller.c
* @brief  
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#if !defined(WIN32)
#ifdef UNDER_LINUX
#include "plat/osdal_os.h"   
#endif

#include "mobcom_types.h"
#include "resultcode.h"

#include "audio_consts.h"
#include "auddrv_def.h"
#ifdef UNDER_LINUX
#include "linux/broadcom/bcm_fuse_sysparm_CIB.h"
#else
#include "sysparm.h"
#endif
#include "audio_gain_table.h"
#include "auddrv_def.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "dspcmd.h"
#include "audio_controller.h"
#ifdef UNDER_LINUX
#include <linux/spinlock_types.h>
#include "plat/bcm_i2sdai.h"
#endif
#include "i2s.h"
#include "log.h"
#include "ostask.h"
#include "xassert.h"

#if !defined(NO_PMU)
#if defined(EXTERNAL_AMP_CONTROL)
#include "hal_pmu.h"
#include "hal_pmu_private.h"
#endif
#endif
//#include "vpripcmdq.h"

#ifdef ENABLE_VOIF
#include "voif_handler.h"
#endif

#ifdef ENABLE_GPIO
#include "gpio_handler.h"
#endif

//=============================================================================
// Public Variable declarations
//=============================================================================
AUDDRV_SPKR_Enum_t voiceCallSpkr = AUDDRV_SPKR_NONE;

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

static AUDDRV_MIC_Enum_t	GetDrvMic (AUDCTRL_MICROPHONE_t mic);
static AUDDRV_SPKR_Enum_t   GetDrvSpk (AUDCTRL_SPEAKER_t speaker);

static AudioMode_t stAudioMode = AUDIO_MODE_INVALID;

static AudioApp_t stAudioApp = AUDIO_APP_VOICE_CALL;

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: AUDCTRL_EnableTelephony
//
// Description:   Enable telephonly path, both ul and dl
//
//============================================================================
void AUDCTRL_EnableTelephony(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	AUDDRV_MIC_Enum_t	micSel;
	AUDDRV_SPKR_Enum_t	spkSel;

    AudioMode_t mode;
	
	// mic selection. 
	micSel = GetDrvMic (mic);

	// speaker selection. We hardcode headset,handset and loud speaker right now. 
	// Later, need to provide a configurable table.
	spkSel = GetDrvSpk (speaker);

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTelephony::  spkSel %d, micSel %d \n", spkSel, micSel );

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_Init ( micSel, spkSel );
	voiceCallSpkr = spkSel;

	// in case it was muted from last voice call,
	//AUDCTRL_SetTelephonySpkrMute (dlSink, speaker, FALSE); 
	// in case it was muted from last voice call,
	//AUDCTRL_SetTelephonyMicMute (ulSrc, mic, FALSE); 


	OSTASK_Sleep( 100 );
	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, TRUE );

#ifdef ENABLE_VOIF
    mode = AUDDRV_GetAudioMode();
    
    if ( mode >= AUDIO_MODE_NUMBER )
        mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);

    VoIF_init(mode);
#endif

#ifdef ENABLE_GPIO
    mode = AUDDRV_GetAudioMode();
    
    if ( mode >= AUDIO_MODE_NUMBER )
        mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);

    GPIO_init(mode);
#endif


	return;
}


void AUDCTRL_RateChangeTelephony( )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_RateChangeTelephony::  stAudioMode %d \n",stAudioMode);

    AUDDRV_SetAudioMode(stAudioMode, stAudioApp);
    if(stAudioApp == AUDIO_APP_VOICE_CALL)
        AUDDRV_Telephony_RateChange(8000);
    else if (stAudioApp == AUDIO_APP_VOICE_CALL_WB)
        AUDDRV_Telephony_RateChange(16000);
}

//=============================================================================
//
// Function Name: AUDCTRL_EC
//
// Description:   DSP Echo cancellation ON/OFF
//
//=============================================================================
void AUDCTRL_EC(Boolean enable, UInt32 arg)
{
    AUDDRV_EC(enable, arg);
}

//=============================================================================
//
// Function Name: AUDCTRL_NS
//
// Description:   DSP Noise Suppression ON/OFF
//
//=============================================================================
void AUDCTRL_NS(Boolean enable)
{
    AUDDRV_NS(enable);
}

//=============================================================================
//
// Function Name: AUDCTRL_ECreset_NLPoff
//
// Description:   DSP control to turn EC ON but NLP OFF, regardless of sysparm
//
//=============================================================================
void AUDCTRL_ECreset_NLPoff(Boolean ECenable)
{
    AUDDRV_ECreset_NLPoff(ECenable);
}

//============================================================================
//
// Function Name: AUDCTRL_DisableTelephony
//
// Description:   disable telephony path, both dl and ul
//
//============================================================================
void AUDCTRL_DisableTelephony(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisableTelephony \n" );
#ifdef ENABLE_VOIF
    VoIF_Deinit();
#endif

#ifdef ENABLE_GPIO
    GPIO_Deinit();
#endif

	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, FALSE );
	OSTASK_Sleep( 100 );

	// The following is the sequence we need to follow
	AUDDRV_Telephony_Deinit ();
	voiceCallSpkr = AUDDRV_SPKR_NONE;

	return;
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicSpkr
//
// Description:   Set the micphone and speaker to telephony path, previous micophone
//	and speaker is disabled
//
//============================================================================
void AUDCTRL_SetTelephonyMicSpkr(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	AUDDRV_MIC_Enum_t	micSel; 
	AUDDRV_SPKR_Enum_t	spkSel;
	
	micSel = GetDrvMic (mic);
	spkSel = GetDrvSpk (speaker);

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicSpkr::  spkSel %d, micSel %d \n", spkSel, micSel );

	//driver needs to know mode!

	if(stAudioMode == AUDIO_MODE_USB) AUDCTRL_SetAudioMode ( AUDIO_MODE_HANDSET, stAudioApp ); 
	else AUDCTRL_SetAudioMode ( stAudioMode, stAudioApp );


	AUDDRV_Telephony_SelectMicSpkr ( micSel, spkSel );
	voiceCallSpkr = spkSel;
	//need to think about better design!!  do mode switch after EC off, mic mute, etc.

	OSTASK_Sleep( 100 );  //depending on switch to headset or off of headset, PMU is first off or last on.
	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, TRUE );
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrMute
//
// Description:   mute/unmute the dl of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrMute: mute = 0x%x\n",  mute);
	if(mute)
		audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkMute, 0, 0, 0, 0, 0);
	else
		audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkUnmute, 0, 0, 0, 0, 0);
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicGain
//
// Description:   Set ul gain of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicGain(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				Int16					gain    //dB
				)
{
	//this API only take UInt32 param. pass (gain+100) in that it is Positive integer.
	audio_control_generic( AUDDRV_CPCMD_SetBasebandUplinkGain, (gain+100), 0, 0, 0, 0);
}


//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicMute
//
// Description:   mute/unmute ul of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicMute(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicMute: mute = 0x%x\n",  mute);
        AUDDRV_SetTelephonyMicMute( mute );
}

//============================================================================
//
// Function Name: AUDCTRL_SetAMRVolume_DL
//
// Description:   set gain on the VPU playback path on the downlink path
//
//============================================================================
void AUDCTRL_SetAMRVolume_DL(UInt16 uVol)
{
	audio_control_dsp( AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_DL, uVol, 0, 0, 0, 0 );
}

//============================================================================
//
// Function Name: AUDCTRL_SetAMRVolume_UL
//
// Description:   set gain on the VPU playback path on the uplink path
//
//============================================================================
void AUDCTRL_SetAMRVolume_UL(UInt16 uVol)
{
	audio_control_dsp( AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_UL, uVol, 0, 0, 0, 0 );
}

//*********************************************************************
//  Function Name: AUDCTRL_InVoiceCall 
//	@return	 TRUE/FALSE (in/out voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCall( void )
{
	return AUDDRV_InVoiceCall();
}

//*********************************************************************
//  Function Name: AUDCTRL_InVoiceCallWB
//	@return	 TRUE/FALSE (in/out WB voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCallWB( void )
{
	return AUDDRV_IsVoiceCallWB(AUDDRV_GetAudioMode());
}

//*********************************************************************
//  Get current (voice call) audio mode 
//	@return		mode		(voice call) audio mode 
//**********************************************************************/
AudioMode_t AUDCTRL_GetAudioMode( void )
{
	return AUDDRV_GetAudioMode( );
}


//*********************************************************************
//  Get current (voice call) audio application 
//	@return		mode		(voice call) audio application 
//**********************************************************************/
AudioApp_t AUDCTRL_GetAudioApp( void )
{
	return AUDDRV_GetAudioApp( );
}

//*********************************************************************
//  Save audio mode before call AUDCTRL_SaveAudioModeFlag( )
//	@param		mode		(voice call) audio mode 
//	@param		app			(voice call) audio app 
//	@return		none
//**********************************************************************/
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode, AudioApp_t app )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SaveAudioModeFlag: mode = %d, app=%d\n",  mode, app);
	stAudioMode = mode;
	stAudioApp = app;
	AUDDRV_SaveAudioMode( mode, app );
}

//*********************************************************************
//   Set (voice call) audio mode 
//	@param		mode		(voice call) audio mode 
//	@param		app		(voice call) audio app 
//	@return		none
//**********************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode, AudioApp_t app)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioMode: mode = %d, app=%d\n",  mode, app);
	AUDCTRL_SaveAudioModeFlag( mode, app );
	AUDDRV_SetAudioMode( mode, app );
}

//=============================================================================
// Private function definitions
//=============================================================================

//============================================================================
//
// Function Name: GetDrvMic
//
// Description:   convert audio controller microphone enum to auddrv microphone enum
//
//============================================================================
static AUDDRV_MIC_Enum_t GetDrvMic (AUDCTRL_MICROPHONE_t mic)
{
	AUDDRV_MIC_Enum_t micSel=AUDDRV_MIC_ANALOG_MAIN;

	// microphone selection. We hardcode microphone for headset,handset and loud right now. 
	// Later, need to provide a configurable table.
	switch (mic)
	{
		case AUDCTRL_MIC_MAIN:
			micSel = AUDDRV_MIC_ANALOG_MAIN;
			break;
		case AUDCTRL_MIC_AUX:
			micSel = AUDDRV_MIC_ANALOG_AUX;
			break;

		case AUDCTRL_MIC_DIGI1:
			micSel = AUDDRV_MIC_DIGI1;
			break;
		case AUDCTRL_MIC_DIGI2:
			micSel = AUDDRV_MIC_DIGI2;
			break;
		case AUDCTRL_DUAL_MIC_DIGI12:
			micSel = AUDDRV_DUAL_MIC_DIGI12;
			break;
		case AUDCTRL_DUAL_MIC_DIGI21:
			micSel = AUDDRV_DUAL_MIC_DIGI21;
			break;
		case AUDCTRL_DUAL_MIC_ANALOG_DIGI1:
			micSel = AUDDRV_DUAL_MIC_ANALOG_DIGI1;
			break;
		case AUDCTRL_DUAL_MIC_DIGI1_ANALOG:
			micSel = AUDDRV_DUAL_MIC_DIGI1_ANALOG;
			break;

		case AUDCTRL_MIC_BTM:
			micSel = AUDDRV_MIC_PCM_IF;
			break;
		//case AUDCTRL_MIC_BTS:
			//break;
		case AUDCTRL_MIC_I2S:
			break;

		case AUDCTRL_MIC_USB:
		default:
			Log_DebugPrintf(LOGID_AUDIO,"GetDrvMic: Unsupported microphpne type. mic = 0x%x\n", mic);
			break;
	}

	return micSel;
}


//============================================================================
//
// Function Name: GetDrvSpk
//
// Description:   convert audio controller speaker enum to auddrv speaker enum
//
//============================================================================
static AUDDRV_SPKR_Enum_t GetDrvSpk (AUDCTRL_SPEAKER_t speaker)
{
	AUDDRV_SPKR_Enum_t spkSel = AUDDRV_SPKR_NONE;

	Log_DebugPrintf(LOGID_AUDIO,"GetDrvSpk: spk = 0x%x\n", speaker);

	// speaker selection. We hardcode headset,handset and loud speaker right now. 
	// Later, need to provide a configurable table.
	switch (speaker)
	{
		case AUDCTRL_SPK_HANDSET:
			spkSel = AUDDRV_SPKR_EP;
			break;
		case AUDCTRL_SPK_HEADSET:
			spkSel = AUDDRV_SPKR_HS;
			break;
		case AUDCTRL_SPK_LOUDSPK:
			spkSel = AUDDRV_SPKR_IHF;
			break;
		case AUDCTRL_SPK_BTM:
			spkSel = AUDDRV_SPKR_PCM_IF;
			break;
		case AUDCTRL_SPK_BTS:
			break;
		
		case AUDCTRL_SPK_I2S:
			break;

		case AUDCTRL_SPK_USB:
		default:
			Log_DebugPrintf(LOGID_AUDIO,"GetDrvSpk: Unsupported Speaker type. spk = 0x%x\n", speaker);
			break;
	}

	return spkSel;
}


#else // WIN32

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "audio_gain_table.h"
#include "auddrv_def.h"
#include "csl_aud_drv.h"
#include "audio_manager_commons.h"
#include "audio_controller.h"
#include "i2s.h"
#include "log.h"
#include "xassert.h"
//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: AUDCTRL_EnableTelephony
//
// Description:   Enable telephonly path, both ul and dl
//
//============================================================================
void AUDCTRL_EnableTelephony(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_RateChangeTelephony
//
// Description:   RateChange telephonly path, both ul and dl
//
//============================================================================
void AUDCTRL_RateChangeTelephony()
{
}

//============================================================================
//
// Function Name: AUDCTRL_DisableTelephony
//
// Description:   disable telephony path, both dl and ul
//
//============================================================================
void AUDCTRL_DisableTelephony(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicSpkr
//
// Description:   Set the micphone and speaker to telephony path, previous micophone
//	and speaker is disabled
//
//============================================================================
void AUDCTRL_SetTelephonyMicSpkr(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrMute
//
// Description:   mute/unmute the dl of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicGain
//
// Description:   Set ul gain of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicGain(
				AUDIO_HW_ID_t			ulSrc,
				AUDCTRL_MICROPHONE_t	mic,
				Int16					gain
				)
{
}

//*********************************************************************
//
//  Get current (voice call) audio mode 

//	@return		mode		(voice call) audio mode 
//   @note      
//**********************************************************************/
AudioMode_t AUDCTRL_GetAudioMode( void )
{
	return (AudioMode_t)0;
}

//*********************************************************************
//  Save audio mode before call AUDCTRL_SetAudioMode( )
//	@param		mode		(voice call) audio mode 
//	@return		none
//**********************************************************************/
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode )
{
}

//*********************************************************************
//
//   Set (voice call) audio mode 

//	@param		mode		(voice call) audio mode 
///	@return		
//**********************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode )
{
}


//*********************************************************************
//  Function Name: AUDCTRL_InVoiceCall 
//	@return	 TRUE/FALSE (in/out voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCall( void )
{
#if defined(WIN32)
	{
		extern int modeVoiceCall;
		if(modeVoiceCall) return TRUE;
	}
#endif
	return FALSE;
}

//*********************************************************************
//  Function Name: AUDCTRL_InVoiceCallWB
//	@return	 TRUE/FALSE (in/out WB voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCallWB( void )
{
#if defined(WIN32)
	{
		extern int modeVoiceCall;
		if(modeVoiceCall==2) return TRUE;
	}
#endif
	return FALSE;
}

//=============================================================================
// Private function definitions
//=============================================================================

#endif //WIN32
