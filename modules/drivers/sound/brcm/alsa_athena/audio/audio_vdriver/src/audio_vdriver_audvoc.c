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
* @file   audio_vdriver_audvoc.c
* @brief  Audio VDriver API for Athena-type Audio HW
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_syscfg.h"
//#define SYSCFG_BASE_ADDR      0x08880000      /* SYSCFG core */
//#define SYSCFG_ANACR0_OFFSET                                              0x00000080
#include "shared.h"
#include "dspcmd.h"
#include "ripcmdq.h"
#include "ripisr.h"
#include "audio_consts.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
//#ifndef NO_SYSPARAM
#ifdef UNDER_LINUX
#include "linux/broadcom/bcm_fuse_sysparm_CIB.h"
#else
#include "sysparm.h"
#endif
//#endif
#include "ostask.h"
#include "audioapi_asic.h"
#include "log.h"
//#include "vpripcmdq.h"
#include "csl_apcmd.h"
/**
*
* @addtogroup AudioDriverGroup
* @{
*/

#define SEPARATE_MODE_FOR_GVS_WTIH_RATE
// #undef SEPARATE_MODE_FOR_GVS_WTIH_RATE

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

typedef void (*AUDDRV_User_CB) (UInt32 param1, UInt32 param2, UInt32 param3);

AUDDRV_MIC_Enum_t   currVoiceMic = AUDDRV_MIC_NONE;   //used in pcm i/f control. assume one mic, one spkr.
AUDDRV_SPKR_Enum_t  currVoiceSpkr = AUDDRV_SPKR_NONE;  //used in pcm i/f control. assume one mic, one spkr.
Boolean inVoiceCall = FALSE;

extern Boolean voicePlayOutpathEnabled;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN
extern Boolean controlFlagForCustomGain;

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

#if 0
#define ANACR0_ModeMask (0x001F9FC0)  //ANACR0 bits [20:15] Mode_selR, Mode_selL, [12:06] Cm_seland, Iq_dbl, BgTcsp.
#define ANACR0_DrvrSelMask (0x006000) //Drvr_selR, Drvr_selL
#endif

Boolean voiceInPathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN for both voiceIn and voiceOut

static AudioEqualizer_en_t	sEqualizerType = EQU_NORMAL;
static void *sUserCB = NULL;

//=============================================================================
// Private function prototypes
//=============================================================================


//=============================================================================
// Functions
//=============================================================================



// CSL driver will send a DSP_AUDIO_ENABLE cmd to DSP, dsp access AMCR.bit5. 
// But ARM code (audio controller) tracks the usage of this bit and control it.

// PCM I/F is accessed and controlled by DSP.
// CSL driver will send a DSP_ENABLE_DIGITAL_SOUND?? cmd to DSP, dsp turns on/off PCM interface.
// But ARM code (audio controller) tracks the usage of this bit and control it.


#if defined(FUSE_APPS_PROCESSOR)

// intentionally put this static var here to prevent any code above this lines to use this var.
static AudioApp_t currAudioApp = AUDIO_APP_VOICE_CALL;
static AudioMode_t currAudioMode = AUDIO_MODE_HANDSET;  //need to update this on AP and also in audioapi.c on CP.
static AudioMode_t currMusicAudioMode = AUDIO_MODE_HANDSET;
static AUDIO_SAMPLING_RATE_t currInputSamplingRate = AUDIO_SAMPLING_RATE_UNDEFINED;

#ifdef SEPARATE_MODE_FOR_GVS_WTIH_RATE
static void AUDDRV_RemapAudioModeForGVS ( void );
#endif // SEPARATE_MODE_FOR_GVS_WTIH_RATE

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmAccessPtr
//!
//! Description:	audio parm acces for FUSE AP only
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

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmMMAccessPtr
//!
//! Description:	audio parm multimedia acces for FUSE AP only
//!
/////////////////////////////////////////////////////////////////////////////
static SysIndMultimediaAudioParm_t* AUDIO_GetParmMMAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetMultimediaAudioParmAccessPtr();
#endif
}

static SysAudioParm_t* AudioParmAccessor(UInt32 app,UInt32 mode)
{

    SysAudioParm_t* parm_ptr;

    SysAudioParm_t** p_parm_ptr;

    Log_DebugPrintf(LOGID_AUDIO,"\n  app - %d mode - %d\n",app,mode);

    parm_ptr = SYSPARM_GetExtAudioParmAccessPtr(app);
    p_parm_ptr =(SysAudioParm_t**)parm_ptr;

    Log_DebugPrintf(LOGID_AUDIO,"\n  volume_max - %d\n",p_parm_ptr[0][mode].voice_volume_max);

    Log_DebugPrintf(LOGID_AUDIO,"\n  ext_speaker_pga - %d\n",p_parm_ptr[0][mode].ext_speaker_pga);

    return &p_parm_ptr[0][mode];
}


//#define AUDIOMODE_PARM_ACCESSOR(app,mode)	 ((SysAudioParm_t **)AUDIO_GetParmAccessPtr())[app][mode]

#define AUDIOMODE_PARM_ACCESSOR(app,mode)	 ((SysAudioParm_t *)((SysAudioParm_t *)AUDIO_GetParmAccessPtr()+app*AUDIO_MODE_NUMBER))[mode]

//#define AUDIOMODE_PARM_ACCESSOR(app,mode)	 (*AudioParmAccessor(app,mode))



//#define AUDIOMODE_PARM_MM_ACCESSOR(app,mode)	 (AUDIO_GetParmMMAccessPtr())[mode]
#define AUDIOMODE_PARM_MM_ACCESSOR(app,mode)	((SysIndMultimediaAudioParm_t *)((SysIndMultimediaAudioParm_t *)AUDIO_GetParmMMAccessPtr()+app*AUDIO_MODE_NUMBER))[mode]


#else //#if defined(FUSE_APPS_PROCESSOR)

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmAccessPtr
//!
//! Description:	audio parm access on CP
//! 
/////////////////////////////////////////////////////////////////////////////
static SysCalDataInd_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return SYSPARM_GetAudioParmAccessPtr();
#endif
}


  //CP

#define AUDIOMODE_PARM_ACCESSOR(app, mode)	 AUDIO_GetParmAccessPtr()->audio_parm[app][mode]

#define AUDIOMODE_PARM_MM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()->mmaudio_parm[mode]

#endif //#if defined(FUSE_APPS_PROCESSOR)



//=============================================================================
//
// Function Name: AUDDRV_Telephony_SelectMicSpkr
//
// Description:   Select the mic and speaker for voice call
//
//=============================================================================

void AUDDRV_Telephony_SelectMicSpkr (
					AUDDRV_MIC_Enum_t   mic,
					AUDDRV_SPKR_Enum_t  speaker )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_SelectMicSpkr (1) mic %d, spkr %d *\n\r", mic, speaker);

#if defined(FUSE_APPS_PROCESSOR)
  #if defined(THUNDERBIRD)
			//thunderbird, loud speaker go to PMU
		if( currVoiceMic == mic && currVoiceSpkr == speaker)
			return;
  #else
			//AthenaRay board
			// loud speaker should go to PMU, but right now, using the same as HANDSET
		//if( currVoiceMic == mic && currVoiceSpkr == speaker)
		//	return;
  #endif

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_SelectMicSpkr (2) mic %d, spkr %d *\n\r", mic, speaker);

	currVoiceMic = mic;
	currVoiceSpkr = speaker;

	//need to follow the sequence. avoid enable again
	AUDDRV_SelectSpkr( AUDDRV_VOICE_OUTPUT, speaker, AUDDRV_SPKR_NONE );

	//select mic input, include DMIC support
	AUDDRV_SelectMic( AUDDRV_VOICE_INPUT, mic);

	//if( speaker == AUDDRV_SPKR_PCM_IF || mic==AUDDRV_MIC_PCM_IF )
	if( mic==AUDDRV_MIC_PCM_IF )
	{
		//use audio_control_dsp( ), and combine this file with csl_aud_drv_hw.c
		AUDDRV_SetPCMOnOff( 1 );
	}
	else
	{
		AUDDRV_SetPCMOnOff( 0 );
	}
#endif //#if defined(FUSE_APPS_PROCESSOR)
}


//=============================================================================
//
// Function Name: AUDDRV_Enable_Output
//
// Description:   Enable audio output for voice call
//
//=============================================================================

void AUDDRV_Enable_Output (
				AUDDRV_InOut_Enum_t    input_path_to_mixer,
				AUDDRV_SPKR_Enum_t     mixer_speaker_selection,
				Boolean                enable_speaker,
				AUDIO_SAMPLING_RATE_t  sample_rate,
				AUDIO_CHANNEL_NUM_t    input_to_mixer
				)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Enable_Output mixer %d, en %d input_path_to_mixer %d invoicecall %d, sample_rate %d *\n\r", 
          mixer_speaker_selection, enable_speaker, input_path_to_mixer, inVoiceCall, sample_rate);

#if defined(FUSE_APPS_PROCESSOR)
	AUDDRV_EnableHWOutput( input_path_to_mixer, mixer_speaker_selection, enable_speaker, sample_rate, input_to_mixer,
		AUDDRV_REASON_HW_CTRL );
	OSTASK_Sleep( 5 );  //sometimes BBC video has no audio. This delay may help the mixer filter and mixer gain loading.

	switch(input_path_to_mixer)
	{
		case AUDDRV_VOICE_OUTPUT:
			
			if(inVoiceCall != TRUE)
			{
				//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_DL
				if (sample_rate == AUDIO_SAMPLING_RATE_8000)
				{
					audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 0, 0, 0, 0 );
					audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 0, 0, 0, 0 );
				}
				else
				{
					audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 1, 0, 0, 0 );
					audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 1, 0, 0, 0 );
				}
				voicePlayOutpathEnabled = TRUE;

				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Enable_Output: inVoiceCall = %d, voicePlayOutpathEnabled = %d\n\r", inVoiceCall, voicePlayOutpathEnabled);
			}	
			
			currVoiceSpkr = mixer_speaker_selection;
			if(inVoiceCall != TRUE)
			{
				if (currVoiceSpkr == AUDDRV_SPKR_PCM_IF)
					AUDDRV_SetPCMOnOff( 1 );
				else
				{
					if(currVoiceMic != AUDDRV_MIC_PCM_IF) //need to check mic too.
						AUDDRV_SetPCMOnOff( 0 );
				}
				
			}
			//else, Enable_Input( ) set pcm i/f.

			break;

		case AUDDRV_AUDIO_OUTPUT:
			break;

		case AUDDRV_RINGTONE_OUTPUT:
			break;

		default:
			break;
	}
#endif  //#if defined(FUSE_APPS_PROCESSOR)	
}


//=============================================================================
//
// Function Name: AUDDRV_Disable_Output
//
// Description:   Disable audio output for voice call
//
//=============================================================================

void AUDDRV_Disable_Output ( AUDDRV_InOut_Enum_t  path )
{
#if defined(FUSE_APPS_PROCESSOR)
	switch(path) 
	{
		case AUDDRV_VOICE_OUTPUT:
				
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Disable_Output *\n\r" );

			if(inVoiceCall != TRUE)
			{
				if ( voiceInPathEnabled==FALSE )
				{
				//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_DL
				audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 0, 0, 0, 0, 0 );
				audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 0, 0, 0, 0, 0 );
				voicePlayOutpathEnabled = FALSE;
				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Disable_Output: inVoiceCall = %d, voicePlayOutpathEnabled = %d\n\r", voicePlayOutpathEnabled);
				}
			}
			
			if (currVoiceSpkr == AUDDRV_SPKR_PCM_IF)  //turn off PCM i/f
			{
				currVoiceSpkr = AUDDRV_SPKR_NONE;
				if(currVoiceMic != AUDDRV_MIC_PCM_IF)
					VPRIPCMDQ_DigitalSound( FALSE );
			} //else, no need to care PCM i/f.

			currVoiceSpkr = AUDDRV_SPKR_NONE;

			break;
			
		default:
			break;
	}

	OSTASK_Sleep( 3 ); //make sure audio is done

	AUDDRV_DisableHWOutput ( path, AUDDRV_REASON_HW_CTRL );
#endif //#if defined(FUSE_APPS_PROCESSOR)	

}



//=============================================================================
//
// Function Name: AUDDRV_Enable_Input
//
// Description:   Enable audio input for voice call
//
//=============================================================================

void AUDDRV_Enable_Input (
                    AUDDRV_InOut_Enum_t      input_path,
                    AUDDRV_MIC_Enum_t        mic_selection,
					AUDIO_SAMPLING_RATE_t    sample_rate )
{
#if defined(FUSE_APPS_PROCESSOR)
	currInputSamplingRate = sample_rate;
	
	AUDDRV_EnableHWInput ( input_path, mic_selection, sample_rate,
		AUDDRV_REASON_HW_CTRL );

	switch(input_path) {
	case AUDDRV_VOICE_INPUT:
		
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Enable_Input mic_selection %d *\n\r", mic_selection );

		if(inVoiceCall != TRUE)
		{
			//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_UL
			if (sample_rate == AUDIO_SAMPLING_RATE_8000)
			{
				currInputSamplingRate = AUDIO_SAMPLING_RATE_8000;
				audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 0, 0, 0, 0);
				audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 0, 0, 0, 0 );
			}
			else
			{
				currInputSamplingRate = AUDIO_SAMPLING_RATE_16000;
				audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 1, 0, 0, 0);
				audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 1, 0, 0, 0 );
			}
			
		}
        voiceInPathEnabled = TRUE;
		currVoiceMic = mic_selection;
		if (currVoiceMic == AUDDRV_MIC_PCM_IF)
			AUDDRV_SetPCMOnOff( 1 );
		else
		{
			if (currVoiceSpkr != AUDDRV_SPKR_PCM_IF) //need to check spkr too.
				AUDDRV_SetPCMOnOff( 0 );
		}
			
		break;

	default:
		break;
	}
#endif //#if defined(FUSE_APPS_PROCESSOR)
}


//=============================================================================
//
// Function Name: AUDDRV_Disable_Input
//
// Description:   Disable audio input for voice call
//
//=============================================================================

void AUDDRV_Disable_Input (  AUDDRV_InOut_Enum_t      path )
{
	switch(path) {
	case AUDDRV_VOICE_INPUT:

		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Disable_Input *\n\r" );

		
		if(inVoiceCall != TRUE)
		{
			if ( voicePlayOutpathEnabled == FALSE )
			{
			//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_UL
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 0, 0, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0);
			}
			
		
		    if (currVoiceMic == AUDDRV_MIC_PCM_IF)  //turn off PCM
		    {
			    currVoiceMic = AUDDRV_MIC_NONE;
			    if (currVoiceSpkr != AUDDRV_SPKR_PCM_IF)  //turn off PCM
			    {
				    VPRIPCMDQ_DigitalSound( FALSE );
			    }
		    } //else, no need to care PCM i/f.

		    OSTASK_Sleep( 3 ); //make sure audio is done

		    currVoiceMic = AUDDRV_MIC_NONE;
        }
        voiceInPathEnabled = FALSE;
	    AUDDRV_DisableHWInput ( path, AUDDRV_REASON_HW_CTRL );

        break;

	default:
		AUDDRV_DisableHWInput ( path, AUDDRV_REASON_HW_CTRL );
		break;
	}
}




//=============================================================================
//
// Function Name: AUDDRV_GetAudioApp
//
// Description:   get audio application.
//
//=============================================================================

AudioApp_t AUDDRV_GetAudioApp( void )
{
	return currAudioApp;
}


//=============================================================================
//
// Function Name: AUDDRV_SaveAudioMode
//
// Description:   save audio mode with its corresponding audio app (profile).
//
//=============================================================================

void AUDDRV_SaveAudioMode( AudioMode_t audio_mode, AudioApp_t audio_app)
{
	currAudioMode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER); //internally mode is protected between 0 to 8
	currAudioApp = audio_app;

	if (audio_mode >= AUDIO_MODE_NUMBER_VOICE)
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SaveAudioMode() invalid audio_mode==%d\n\r", audio_mode );
	}
	else if (( audio_mode >= AUDIO_MODE_NUMBER ) && (audio_mode < AUDIO_MODE_NUMBER_VOICE ))
    {
		if (audio_app == AUDIO_APP_VOICE_CALL) currAudioApp = AUDIO_APP_VOICE_CALL_WB;
	}
}


//=============================================================================
//
// Function Name: AUDDRV_GetAudioMode
//
// Description:   Return Audio mode
//				  For new audio param format, currAudioMode only takes value from 0 to 8.
//				  But the return value will be [0 to 8] for voice NB 
//				  and [9 to 17] for voice WB
//=============================================================================
AudioMode_t AUDDRV_GetAudioMode( void )
{

	AudioMode_t mode = currAudioMode;;
	
    if (currAudioApp == AUDIO_APP_VOICE_CALL) //NB application
		mode = currAudioMode;
	else if(currAudioApp == AUDIO_APP_VOICE_CALL_WB) //WB application
		mode =  (AudioMode_t)(currAudioMode + AUDIO_MODE_NUMBER);

	return mode;
	//extend to check further applications to be NB or WB mode

}


//=============================================================================
//
// Function Name: AUDDRV_SetAudioMode
//
// Description:   set audio mode with its corresponding audio app (profile).
//
//=============================================================================

void AUDDRV_SetAudioMode( AudioMode_t audio_mode, AudioApp_t audio_app)
{

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() audio_mode==%d\n\r", audio_mode );

	// load DSP parameters:
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() audio_app==%d\n\r", audio_app );
	
	currAudioMode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER); // update mode
	currMusicAudioMode = currAudioMode;
	currAudioApp = audio_app;

	if (( audio_mode >= AUDIO_MODE_NUMBER ) && (audio_mode < AUDIO_MODE_NUMBER_VOICE ))
	{
		if (audio_app == AUDIO_APP_VOICE_CALL)
			currAudioApp = AUDIO_APP_VOICE_CALL_WB;
	}
	else if ((audio_app == AUDIO_APP_VOICE_CALL_WB) && (audio_mode < AUDIO_MODE_NUMBER))
		audio_mode = (AudioMode_t)(audio_mode + AUDIO_MODE_NUMBER);

#ifdef SEPARATE_MODE_FOR_GVS_WTIH_RATE
	// remap audio mdoe to separate 8 kHz and 16 kHz for GVS function.
	if (currAudioApp == AUDIO_APP_RECORDING_GVS)
		AUDDRV_RemapAudioModeForGVS();
#endif // SEPARATE_MODE_FOR_GVS_WTIH_RATE

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* to call AUDDRV_CPCMD_SetAudioMode() mode=%d \t app =%d\n\r", currAudioMode, currAudioApp );
	audio_control_generic( AUDDRV_CPCMD_SetAudioMode, currAudioMode, currAudioApp, 0, 0, 0 );

    AUDDRV_SetFilter( AUDDRV_VoiceADC, (UInt16*) AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).voice_adc);
	AUDDRV_SetFilter( AUDDRV_VoiceDAC, (UInt16*) AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).voice_dac);

	//AUDDRV_VoiceDAC
#if 0
		//should be part of audio mode API
	//or make SYSPARM_GetAudioParmAccessPtr to be the same on AP and CP:
	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) &= ~ANACR0_ModeMask;  //clear the bits
	//*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_anacr0 & ANACR0_ModeMask ) ;
    if(currAudioMode == 0)
        *(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( 0x006040ca & ANACR0_ModeMask ) ;
    else if(currAudioMode == 1)
        *(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( 0x01E000FE & ANACR0_ModeMask ) ;
    else if(currAudioMode == 4)
        *(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( 0x00006000 & ANACR0_ModeMask ) ;
    else
        *(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( 0x000000C0 & ANACR0_ModeMask ) ;
#endif	
    
    //*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ANACR0_DrvrSelMask;

	if ( controlFlagForCustomGain == FALSE )
	{
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_IHF, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).speaker_pga);
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_EP, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).speaker_pga);
	}

	AUDDRV_SetGain_Hex( AUDDRV_GAIN_VOICE_OUTPUT_CFGR, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_vcfgr );	

	// Only load the slopgain for voice call. For voice playback/record, we don't want to load the sysparm value to overwrite what user set.  <==NO, this is wrong.
	// all voice playback goes throught DSP, and shall use DSP SW gain control on the playback path. The voice slopgain is fixed at the sysparm value.
	
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_SetAudioMode vopath_slopgain %x, mode %d \n\r", AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_vslopgain, currAudioMode );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_VOICE_OUTPUT, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_vslopgain );

	AUDDRV_SetEquType ( AUDDRV_TYPE_AUDIO_OUTPUT, sEqualizerType );
	// AUDDRV_SetEquType ( AUDDRV_TYPE_RINGTONE_OUTPUT, sEqualizerType );

	// remove aslopgain loading from sysparm, user set it.
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_AUDIO_OUTPUT_L, AUDIOMODE_PARM_MM_ACCESSOR(currAudioApp,currMusicAudioMode).audvoc_aslopgain );
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_AUDIO_OUTPUT_R, AUDIOMODE_PARM_MM_ACCESSOR(currAudioApp,currMusicAudioMode).audvoc_aslopgain );

	//move to CP.  //to read and set this after the sys parm are readable at AP
	//AUDDRV_SetFilter( AUDDRV_AFIR,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0] );

	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQ,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQCOF[0][0] );
	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQPATHGAIN,	(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHGAIN[0][0] );
	//AUDDRV_SetFilter( AUDDRV_PEQPATHOFST, & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHOFST[0][0] );

	AUDDRV_SetFilter( AUDDRV_PIIR,			(const UInt16 *) & AUDIOMODE_PARM_MM_ACCESSOR(currAudioApp,currMusicAudioMode).PR_DAC_IIR[0] );

	// remove pslopgain loading from sysparm, user set it.
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_RINGTONE_OUTPUT_L, AUDIOMODE_PARM_MM_ACCESSOR(currAudioApp,currMusicAudioMode).audvoc_pslopgain );
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_RINGTONE_OUTPUT_R, AUDIOMODE_PARM_MM_ACCESSOR(currAudioApp,currMusicAudioMode).audvoc_pslopgain );

	AUDDRV_SetMPM( AUDDRV_Mixer_BIQUAD_CFG, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).MPMbiquad_cfg );
	AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );

	if ( controlFlagForCustomGain == FALSE )
	{
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() currAudioMode==%d AUDDRV_GAIN_MIXER1 %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER3, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER4, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	}

		//Open loop target gain parameters
  	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA1, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA2, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA3, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA4, 0x00 );

	if ( controlFlagForCustomGain == FALSE )
	{
		AUDDRV_SetGain_Hex( AUDDRV_MIC, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).mic_pga);
	}

	if ( AUDDRV_GetVCflag() )
	{
		  // AUDIO_MODE_BLUETOOTH_WB should not be used for voice call. it should be convert to AUDIO_MODE_BLUETOOTH.
		if ((currAudioApp == AUDIO_APP_VOICE_CALL_WB ) && (currAudioMode != AUDIO_MODE_BLUETOOTH) )
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_16000 );
		else
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_8000 );
	}

	// Test to disable VP COMPANDER for GVS:
	if (currAudioApp == AUDIO_APP_RECORDING_GVS)
		audio_control_generic(AUDDRV_CPCMD_SET_VP_COMPANDER, (UInt32)2, 0, 0, 0, 0); // bit 0 compressr, bit 1 expander
	else if (currAudioApp == AUDIO_APP_RECORDING)
		audio_control_generic(AUDDRV_CPCMD_SET_VP_COMPANDER, (UInt32)3, 0, 0, 0, 0);
}

//=============================================================================
//
// Function Name: AUDDRV_SetMusicMode
//
// Description:   set audio mode with its corresponding audio app (profile).
//
//=============================================================================
void AUDDRV_SetMusicMode( AudioMode_t audio_mode, AudioApp_t audio_app)
{
#define	AUDIO_5BAND_EQ_MODE_NUMBER		6	///< Up to 6 Audio EQ Profiles (modes)
    #define EQPATH_SIZE					6  //path 0-5 in EQ.
    #define PR_DAC_IIR_SIZE             25

    //    UInt16 AUDVOC_ADAC_FIR[65]; 
  //  UInt32 AUDVOC_MIXER_IIR[60];
    UInt16 AUDVOC_AEQCOF[ PR_DAC_IIR_SIZE];
	UInt16 AUDVOC_AEQPATHGAIN[ EQPATH_SIZE - 1 ];
	UInt16 AUDVOC_AEQPATHOFST[ EQPATH_SIZE];


	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() audio_mode==%d\n\r", audio_mode );

	currAudioMode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER); // update mode
	currMusicAudioMode = currAudioMode;
	currAudioApp = audio_app;

	if (( audio_mode >= AUDIO_MODE_NUMBER ) && (audio_mode < AUDIO_MODE_NUMBER_VOICE ))
	{
		if (audio_app == AUDIO_APP_VOICE_CALL) currAudioApp = AUDIO_APP_VOICE_CALL_WB;
	}

#if 0
		//should be part of audio mode API
	//or make SYSPARM_GetAudioParmAccessPtr to be the same on AP and CP:
	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) &= ~ANACR0_ModeMask;  //clear the bits
//	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_anacr0 & ANACR0_ModeMask ) ;

    if(currAudioMode == 0)
        *(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( 0x006040ca & ANACR0_ModeMask ) ;
    else if(currAudioMode == 1)
        *(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( 0x01E000FE & ANACR0_ModeMask ) ;
    else if(currAudioMode == 4)
        *(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( 0x00006000 & ANACR0_ModeMask ) ;
    else
        *(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( 0x000000C0 & ANACR0_ModeMask ) ;
#endif
	
	//*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ANACR0_DrvrSelMask;

	if ( controlFlagForCustomGain == FALSE )
    {
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_IHF, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).speaker_pga);
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_EP, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).speaker_pga);
    }

	AUDDRV_SetEquType ( AUDDRV_TYPE_AUDIO_OUTPUT, sEqualizerType );

	
	//move to CP.  //to read and set this after the sys parm are readable at AP
	//AUDDRV_SetFilter( AUDDRV_AFIR,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0] );

	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQ,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQCOF[0][0] );
	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQPATHGAIN,	(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHGAIN[0][0] );
	//AUDDRV_SetFilter( AUDDRV_PEQPATHOFST, & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHOFST[0][0] );
	AUDDRV_SetFilter( AUDDRV_PIIR,			(const UInt16 *) & AUDIOMODE_PARM_MM_ACCESSOR(currAudioApp,currAudioMode).PR_DAC_IIR[0] );

	AUDDRV_SetMPM( AUDDRV_Mixer_BIQUAD_CFG, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).MPMbiquad_cfg );

	AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );

	if ( controlFlagForCustomGain == FALSE )
	{
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() currAudioMode==%d AUDDRV_GAIN_MIXER1 %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER3, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER4, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	}

		//Open loop target gain parameters
  	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA1, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA2, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA3, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA4, 0x00 );

	if ( controlFlagForCustomGain == FALSE )
	{
		AUDDRV_SetGain_Hex( AUDDRV_MIC, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).mic_pga);
    }
}

//=============================================================================
//
// Function Name: AUDDRV_GetEquType
//
// Description:   Get Equalizer Type
//
//=============================================================================

AudioEqualizer_en_t AUDDRV_GetEquType( AUDDRV_TYPE_Enum_t   path )
{
#if ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_GetEquType (AP before read from CP) %d \n\r", sEqualizerType);
	sEqualizerType = (AudioEqualizer_en_t) audio_control_generic( AUDDRV_CPCMD_READ_AUDVOC_AEQMODE, (UInt32) path, 0, 0, 0, 0 );
	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_GetEquType (AP after read from CP) %d \n\r", sEqualizerType);
#else
	UInt16 * ptr=0;
	  // PCG could chagned the equ type from PC. 
	  // Read equ type from sysinterface/dsp/audio/audioapi.c.
	ptr = (UInt16 *) AUDIO_GetAudioParam( PARAM_AUDVOC_AEQMODE );
	sEqualizerType = (AudioEqualizer_en_t) *ptr;
	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_GetEquType (CP) %d \n\r", sEqualizerType);
#endif
	return sEqualizerType;
}


//=============================================================================
//
// Function Name: AUDDRV_SetEquType
//
// Description:   Set Equalizer Type
//
//=============================================================================

void AUDDRV_SetEquType( 
					AUDDRV_TYPE_Enum_t   path,
					AudioEqualizer_en_t	 equ_id
					)
{

#if ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	sEqualizerType = equ_id;

	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_SetEquType (AP) %d \n\r", sEqualizerType);
	audio_control_generic( AUDDRV_CPCMD_WRITE_AUDVOC_AEQMODE, (UInt32) equ_id, 0, 0, 0, 0 );

#else

	SysCalDataInd_t* pSysparm;
	pSysparm = SYSPARM_GetAudioParmAccessPtr();

	sEqualizerType = equ_id;

	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_SetEquType (CP) %d \n\r", sEqualizerType);

	  // CP: update audvoc_aeqMode in sysinterface/dsp/audio/audioapi.c
	AUDIO_SetAudioParam( PARAM_AUDVOC_AEQMODE, (void *) & sEqualizerType );

	 //set these parameters
	AUDDRV_SetFilter( AUDDRV_AEQPATHGAIN, (const UInt16 *)& pSysparm->AUDVOC_AEQPATHGAIN[ sEqualizerType ][0] );
	AUDDRV_SetFilter( AUDDRV_AEQ, (const UInt16 *)& pSysparm->AUDVOC_AEQCOF[ sEqualizerType ][0] );

	AUDDRV_SetFilter( AUDDRV_PEQPATHGAIN, (const UInt16 *)& pSysparm->AUDVOC_PEQPATHGAIN[ sEqualizerType ][0] );
	AUDDRV_SetFilter( AUDDRV_PEQ, (const UInt16 *)& pSysparm->AUDVOC_PEQCOF[ sEqualizerType ][0] );
	//AUDDRV_SetFilter( AUDDRV_PEQPATHOFST, (const UInt16 *)& pSysparm->AUDVOC_PEQCOF[ sEqualizerType ][0] );

	//to remove this after the sys parm are readable at AP

	AUDDRV_SetFilter( AUDDRV_AFIR,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0] );
	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_SetEquType (CP) FIR [0] %x, [32] %x, [33] %x \n\r", 
			SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0],
			SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[32],
			SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[33]
			);

#endif
}






#if defined(FUSE_APPS_PROCESSOR)

//=============================================================================
//
// Function Name: AUDDRV_SetPCMOnOff
//
// Description:   	set PCM on/off for BT
// 
//=============================================================================

void AUDDRV_SetPCMOnOff(Boolean	on_off)
{
	  //return;

#define DSP_AUDIO_REG_AMCR  0xe540

	// By default the PCM port is occupied by trace port on development board
	if(on_off)
	{
#define SYSCFG_IOCR0                     (SYSCFG_BASE_ADDR + 0x0000)  /* IOCR0 bit I/O Configuration Register 0       */
#define SYSCFG_IOCR0_PCM_MUX                       0x00C00000 

		//mux to PCM interface (set to 00)
		*(volatile UInt32*) SYSCFG_IOCR0 &= ~(SYSCFG_IOCR0_PCM_MUX);

	

	}
		
	VPRIPCMDQ_DigitalSound( on_off );
}


//=============================================================================
//
// Function Name: AUDDRV_User_CtrlDSP
//
// Description:   Control DSP Loudspeaker Protection
//
//=============================================================================

#if defined(ENABLE_SPKPROT)
void AUDDRV_User_CtrlDSP ( AudioDrvUserParam_t audioDrvUserParam, void *user_CB, UInt32 param1, UInt32 param2 )
{
	Boolean spkProtEna = FALSE;

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_CtrlDSP *\n\r");
	if (user_CB != NULL)
		sUserCB = user_CB;

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_CtrlDSP, UserCB = %x *\n\r", sUserCB);
	switch (audioDrvUserParam)
	{
		case AUDDRV_USER_GET_SPKPROT:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_GET_SPKPROT *\n\r");
			
			audio_control_dsp(DSPCMD_TYPE_COMMAND_SP, 3, (UInt16) param1, 0,0,0);
			break;
		case AUDDRV_USER_ENA_SPKPROT:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_ENA_SPKPROT *\n\r");
			spkProtEna = (Boolean) param1;
			if (spkProtEna)
				
				audio_control_dsp(DSPCMD_TYPE_COMMAND_SP, (UInt16) spkProtEna, (UInt16) param2, 1,0,0);
			
			else
				audio_control_dsp(DSPCMD_TYPE_COMMAND_SP, (UInt16) spkProtEna, (UInt16) param2, 0,0,0);
			
			break;
		default:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_User_CtrlDSP: Invalid request %d \n\r", audioDrvUserParam);
			break;
	}
}
#endif

//=============================================================================
//
// Function Name: AUDDRV_User_HandleDSPInt
//
// Description:   Handle DSP Interrupt
//
//=============================================================================

void AUDDRV_User_HandleDSPInt ( UInt32 param1, UInt32 param2, UInt32 param3 )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_HandleDSPInt *\n\r");
	if(sUserCB)
		((AUDDRV_User_CB) sUserCB) (param1, param2, param3);
}

#ifdef SEPARATE_MODE_FOR_GVS_WTIH_RATE
//=============================================================================
//
// Function Name: AUDDRV_RemapAudioModeForGVS
//
// Description:   Remap audio mode to separate 8 kHz and 16 kHz for GVS
//
//=============================================================================

typedef enum
{
	HANDSET_GVS_16K = AUDIO_MODE_HANDSET,
	HEADSET_GVS_16K = AUDIO_MODE_HEADSET,			// actually in use.
	UNUSED_GVS = AUDIO_MODE_HANDSFREE,				// reserve for the symmetric use for BT NREC.
	BLUETOOTH_GVS_16K = AUDIO_MODE_BLUETOOTH,
	LOUDSPEAKER_GVS_16K = AUDIO_MODE_SPEAKERPHONE,	// actually in use.
	HANDSET_GVS_8K = AUDIO_MODE_TTY,
	HEADSET_GVS_8K = AUDIO_MODE_HAC,				// actually in use.
	BLUETOOTH_GVS_8K = AUDIO_MODE_USB,
	LOUDSPEAKER_GVS_8K = AUDIO_MODE_RESERVE,		// actually in use.
	MODE_GVS_RATE_TOTAL
} AUDDRV_GVS_Mode_Enum_t;

static void AUDDRV_RemapAudioModeForGVS ( void )
{
	AUDDRV_GVS_Mode_Enum_t tmp_mode = (AUDDRV_GVS_Mode_Enum_t)currAudioMode;

	switch (currAudioMode)
	{
		case AUDIO_MODE_HANDSET:
#ifndef TTY_ENABLED
			if (currInputSamplingRate == AUDIO_SAMPLING_RATE_16000)
				tmp_mode = HANDSET_GVS_16K;
			else if (currInputSamplingRate == AUDIO_SAMPLING_RATE_8000)
				tmp_mode = HANDSET_GVS_8K;
#endif
			break;
		case AUDIO_MODE_HEADSET:
			if (currInputSamplingRate == AUDIO_SAMPLING_RATE_16000)
				tmp_mode = HEADSET_GVS_16K;
			else if (currInputSamplingRate == AUDIO_SAMPLING_RATE_8000)
				tmp_mode = HEADSET_GVS_8K;
			break;
		case AUDIO_MODE_BLUETOOTH:
		case AUDIO_MODE_HANDSFREE: // BT NREC
			if (currInputSamplingRate == AUDIO_SAMPLING_RATE_16000)
				tmp_mode = BLUETOOTH_GVS_16K;
			else if (currInputSamplingRate == AUDIO_SAMPLING_RATE_8000)
				tmp_mode = BLUETOOTH_GVS_8K;
			break;
		case AUDIO_MODE_SPEAKERPHONE:
			if (currInputSamplingRate == AUDIO_SAMPLING_RATE_16000)
				tmp_mode = LOUDSPEAKER_GVS_16K;
			else if (currInputSamplingRate == AUDIO_SAMPLING_RATE_8000)
				tmp_mode = LOUDSPEAKER_GVS_8K;
			break;
		default:
			break;
	}

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_RemapAudioModeForGVS : sample_rate = %d, original_mode = %d, mapped_mode = %d *\n\r",
		currInputSamplingRate, currAudioMode, tmp_mode);

	currAudioMode = (AudioMode_t)tmp_mode;
}
#endif // SEPARATE_MODE_FOR_GVS_WTIH_RATE

#endif // #if defined(FUSE_APPS_PROCESSOR)
