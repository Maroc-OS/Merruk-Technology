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

#if !defined(WIN32) && !defined(_HERA_)

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
#include "csl_aud_drv.h"
#include "audio_vdriver.h"

#ifndef NO_DSP
#include "dspcmd.h"
#endif
#include "audio_controller.h"
#ifndef NO_I2S
#ifdef UNDER_LINUX
#include <linux/spinlock_types.h>
#include "plat/bcm_i2sdai.h"
#include "i2s.h"
#else
#include "i2s.h"
#endif
#endif
#include "log.h"
#include "xassert.h"

#if !defined(NO_PMU)
#ifdef UNDER_LINUX
#ifdef PMU_BCM59038
#include "linux/broadcom/bcm59038/bcm59038-audio.h"
#elif PMU_MAX8986
#include "linux/broadcom/max8986/max8986-audio.h"
#endif
#else
#include "hal_pmu.h"
#include "hal_pmu_private.h"
#endif
#endif

#include "ostask.h"
//#include "vpripcmdq.h"
#include "csl_apcmd.h"

extern AUDDRV_SPKR_Enum_t voiceCallSpkr;
//extern UInt32 lp_voip_start; /* 20110715 for loopback check */

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

static AUDDRV_MIC_Enum_t   GetDrvMic (AUDCTRL_MICROPHONE_t mic);
static AUDDRV_SPKR_Enum_t  GetDrvSpk (AUDCTRL_SPEAKER_t speaker);

//static AUDDRV_SPKR_Enum_t voiceCallSpkr = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t voiceSpkr1 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t voiceSpkr2 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t audioSpkr1 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t audioSpkr2 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t polySpkr1 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t polySpkr2 = AUDDRV_SPKR_NONE;

static AudioApp_t stAudioApp = AUDIO_APP_VOICE_CALL;

#if !defined(NO_PMU)
#ifndef UNDER_LINUX

static PMU_HS_st_t pmu_hs;
static PMU_IHF_st_t pmu_ihf;
#endif
#endif

//static AudioMode_t stAudioMode = AUDIO_MODE_INVALID;
#ifdef UNDER_LINUX
static I2S_HANDLE stI2SHandle = NULL;

#define I2SDRV_Get_handle(id) stI2SHandle  
#endif

static UInt16 Telephony_volume_level_dB = 0;
//=============================================================================
// Private function prototypes
//=============================================================================

//on AP:
static SysAudioParm_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetAudioParmAccessPtr();
#endif
}

static SysIndMultimediaAudioParm_t* AUDIO_GetParmMMAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetMultimediaAudioParmAccessPtr();
#endif
}

#if 0
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
#endif

//#define AUDIOMODE_PARM_ACCESSOR(mode)	 (*AUDIO_GetParmAccessPtr(mode))

//#define AUDIOMODE_PARM_ACCESSOR(app,mode)	 ((SysAudioParm_t **)AUDIO_GetParmAccessPtr())[app][mode]

#define AUDIOMODE_PARM_ACCESSOR(app,mode)	 ((SysAudioParm_t *)((SysAudioParm_t *)AUDIO_GetParmAccessPtr()+app*AUDIO_MODE_NUMBER))[mode]

//#define AUDIOMODE_PARM_ACCESSOR(app,mode)	 (*AudioParmAccessor(app,mode))

//#define AUDIOMODE_PARM_ACCESSOR(app,mode)	 ((SysAudioParm_t **)SYSPARM_GetExtAudioParmAccessPtr(app,mode))[0][0]

//#define AUDIOMODE_PARM_MM_ACCESSOR(app,mode)	(AUDIO_GetParmMMAccessPtr())[mode]
#define AUDIOMODE_PARM_MM_ACCESSOR(app,mode)	((SysIndMultimediaAudioParm_t *)((SysIndMultimediaAudioParm_t *)AUDIO_GetParmMMAccessPtr()+app*AUDIO_MODE_NUMBER))[mode]


//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: AUDCTRL_Init
//
// Description:   Init function
//
//============================================================================
void AUDCTRL_Init (void)
{
	// do some hardware initialization based on the board setting. 
	
	
	// e.g. the speaker configuration.
	AUDDRV_SPKR_Enum_t spk;

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_Init::  \n"  );

	AUDDRV_Init ();

	for (spk = AUDDRV_SPKR_NONE; spk < AUDDRV_SPKR_TOTAL_NUM; spk++)
	{
		// on athena, only need to init these speakers.
		switch (spk)
		{
			case AUDDRV_SPKR_IHF:
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_Init::  AUDDRV_SPKR_IHF\n"  );
				AUDDRV_SPKRInit (spk, AUDIO_SPKR_CHANNEL_DIFF);
				break;

			case AUDDRV_SPKR_EP:
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_Init::  AUDDRV_SPKR_EP\n"  );
				AUDDRV_SPKRInit (spk, AUDIO_SPKR_CHANNEL_DIFF);
				break;

			case AUDDRV_SPKR_HS:
				AUDDRV_SPKRInit (spk, AUDIO_SPKR_CHANNEL_DIFF);
				break;

			case AUDDRV_SPKR_IHF_STEREO:
			case AUDDRV_SPKR_HS_LEFT:
			case AUDDRV_SPKR_HS_RIGHT:
			case AUDDRV_SPKR_PCM_IF:
			case AUDDRV_SPKR_USB_IF:
			default:
				break;
	
		}
	}
#ifdef UNDER_LINUX
    stI2SHandle = I2SDRV_Init (I2S_BLK1_ID);

    

#ifdef PMU_BCM59038
    // Initialize the PMU
    bcm59038_audioInit();
#endif

#else
    // init I2S
	I2SDRV_Init (I2S_BLK1_ID);
#endif
}


UInt16	AUDIO_Util_Convert( UInt16 input, UInt16 scale_in, UInt16 scale_out)
{
	UInt16 output=0;
//	UInt16 temp=0;

	output = ( input * scale_out ) / scale_in;
	return output;
}
//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrVolume
//
// Description:   Set dl volume of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrVolume(
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_SPEAKER_t		speaker,
				UInt32					volume,
				AUDIO_GAIN_FORMAT_t		gain_format
				)
{
//#if defined(FUSE_APPS_PROCESSOR) && defined(CAPI2_INCLUDED)

	UInt8	digital_gain_dB = 0;
	UInt16	volume_max = 0;
    UInt8	OV_volume_step = 0;  //OmegaVoice volume step

//#define  CUSTOMER_SW_SPECIFY_OV_VOLUME_STEP
#undef  CUSTOMER_SW_SPECIFY_OV_VOLUME_STEP

#if defined(CUSTOMER_SW_SPECIFY_OV_VOLUME_STEP)
		{
		//map the digital gain (in dB?) to OV volume level. Need to adjust based on mmi level.
		static UInt8 uVol2OV[37]={
					0, 
				   0, 0, 1, 
				   0, 0, 2, 
				   0, 0, 3, 
				   0, 0, 4, 
				   0, 0, 5, 
				   0, 0, 6, 
				   0, 0, 7, 
				   0, 0, 8, 
				   0, 0, 0, 
				   0, 0, 0, 
				   0, 0, 0, 
				   0, 0, 0
			};

		OV_volume_step = uVol2OV[volume];
		}
#else
		OV_volume_step = 0;
#endif

	switch (gain_format)
	{
		case AUDIO_GAIN_FORMAT_VOL_LEVEL:

			if( AUDIO_VOLUME_MUTE == volume )
			{  //mute
				audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkMute, 0, 0, 0, 0, 0);
			}
			else
			{
                
				AudioApp_t app = AUDDRV_GetAudioApp();
				AudioMode_t mode = AUDDRV_GetAudioMode();
				if ( mode >= AUDIO_MODE_NUMBER )
					mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);
				//volume_max = (AUDIO_GetParmAccessPtr()+app*AUDIO_MODE_NUMBER)[mode].voice_volume_max;  //dB
                volume_max = AUDIOMODE_PARM_ACCESSOR(app,mode).voice_volume_max;  //dB
                
				// convert the value in 1st param from range of 2nd_param to range of 3rd_param:
				digital_gain_dB = AUDIO_Util_Convert( volume, AUDIO_VOLUME_MAX, volume_max );

				Log_DebugPrintf(LOGID_AUDIO,"\n digital_gain_dB- %d volume_max - %d\n",digital_gain_dB,volume_max);
                Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrVolume: volume = 0x%x, OV volume step %d (FORMAT_VOL_LEVEL) \n", volume, OV_volume_step );

				//if parm4 (OV_volume_step) is zero, volumectrl.c will calculate OV volume step based on digital_gain_dB, VOICE_VOLUME_MAX and NUM_SUPPORTED_VOLUME_LEVELS.
				audio_control_generic( AUDDRV_CPCMD_SetBasebandVolume, digital_gain_dB, 0, OV_volume_step, 0, 0);
                Telephony_volume_level_dB = digital_gain_dB;
			}
			break;

		case AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN:
			{

				AudioApp_t app = AUDDRV_GetAudioApp();
				AudioMode_t mode = AUDDRV_GetAudioMode();
				if ( mode >= AUDIO_MODE_NUMBER )
					mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);

				//digital_gain_dB = (AUDIO_GetParmAccessPtr()+app*AUDIO_MODE_NUMBER)[mode].dsp_voice_vol_tbl[volume];  //dB table in sysparm

                digital_gain_dB = AUDIOMODE_PARM_ACCESSOR(app,mode).dsp_voice_vol_tbl[volume];  //dB table in sysparm

				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrVolume: AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN = 0x%x, OV volume step %d \n", digital_gain_dB, OV_volume_step );

				//if parm4 (OV_volume_step) is zero, volumectrl.c will calculate OV volume step based on digital_gain_dB, VOICE_VOLUME_MAX and NUM_SUPPORTED_VOLUME_LEVELS.
				audio_control_generic( AUDDRV_CPCMD_SetBasebandVolume, digital_gain_dB, 0, OV_volume_step, 0, 0);
                Telephony_volume_level_dB = digital_gain_dB;
			}
			break;

		case AUDIO_GAIN_FORMAT_HW_REG:

			if( AUDIO_VOLUME_MUTE == volume )
			{  //mute
				audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkMute, 0, 0, 0, 0, 0);
			}
			else
			{

				AudioApp_t app = AUDDRV_GetAudioApp();
				AudioMode_t mode = AUDDRV_GetAudioMode();
				if ( mode >= AUDIO_MODE_NUMBER )
					mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);
				//volume_max = (AUDIO_GetParmAccessPtr()+app*AUDIO_MODE_NUMBER)[mode].voice_volume_max;  //dB

                volume_max = AUDIOMODE_PARM_ACCESSOR(app,mode).voice_volume_max;  //dB

				digital_gain_dB = (volume < volume_max) ? volume : volume_max;

				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrVolume: volume = 0x%x, OV volume step %d (GAIN_FORMAT_HW) \n", volume, OV_volume_step );

				//if parm4 (OV_volume_step) is zero, volumectrl.c will calculate OV volume step based on digital_gain_dB, VOICE_VOLUME_MAX and NUM_SUPPORTED_VOLUME_LEVELS.
				audio_control_generic( AUDDRV_CPCMD_SetBasebandVolume, digital_gain_dB, 0, OV_volume_step, 0, 0);

			}
			Telephony_volume_level_dB = digital_gain_dB;
			break;

		case AUDIO_GAIN_FORMAT_Q14_1:
			break;
		case AUDIO_GAIN_FORMAT_Q1_14:
			//VOLUMECTRL_SetDigitalRxGain((Int16)(param2+param3/2));
			break;
		default:
			break;
    }

    

//#endif
}
//============================================================================
//
// Function Name: AUDCTRL_GetTelephonySpkrVolume
//
// Description:   Set dl volume of telephony path
//
//============================================================================
UInt16 AUDCTRL_GetTelephonySpkrVolume()
{
    return Telephony_volume_level_dB;
}

//============================================================================
//
// Function Name: AUDCTRL_EnablePlay
//
// Description:   enable a playback path
//
//============================================================================
void AUDCTRL_EnablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
	AUDDRV_SPKR_Enum_t spkSel = AUDDRV_SPKR_NONE;
	AUDDRV_SPKR_Enum_t tapSpkSel = AUDDRV_SPKR_NONE;
	Boolean srcI2S = FALSE;
	I2S_HANDLE i2sHandle;
	UInt32	i2sSr = I2S_SAMPLERATE_48000HZ;
	
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay: src = 0x%x, sink = 0x%x, tap = 0x%x, spkr %d \n", src, sink, tap, spk);
	// don't consider src right now. only consider playback from memory.

	if (src == AUDIO_HW_I2S_IN || sink == AUDIO_HW_I2S_OUT)
	{
		switch (sr)
		{
			case AUDIO_SAMPLING_RATE_8000:
				i2sSr = I2S_SAMPLERATE_8000HZ;
				break;
					
			case AUDIO_SAMPLING_RATE_11030:
				i2sSr = I2S_SAMPLERATE_11030HZ;
				break;
				
			case AUDIO_SAMPLING_RATE_12000:
				i2sSr = I2S_SAMPLERATE_12000HZ;
				break;
					
			case AUDIO_SAMPLING_RATE_16000:
				i2sSr = I2S_SAMPLERATE_16000HZ;
				break;
				
			case AUDIO_SAMPLING_RATE_22060:
				i2sSr = I2S_SAMPLERATE_22060HZ;
				break;
					
			case AUDIO_SAMPLING_RATE_24000:
				i2sSr = I2S_SAMPLERATE_24000HZ;
				break;
			
			case AUDIO_SAMPLING_RATE_32000:
				i2sSr = I2S_SAMPLERATE_32000HZ;
				break;
					
			case AUDIO_SAMPLING_RATE_44100:
				i2sSr = I2S_SAMPLERATE_44100HZ;
				break;

			case AUDIO_SAMPLING_RATE_48000:
				i2sSr = I2S_SAMPLERATE_48000HZ;
				break;

			default:
				break;
		}

	}

	switch (src)
	{
		// FM direct to audio path
		case AUDIO_HW_I2S_IN:
			// enable I2S
			{	
				i2sHandle = I2SDRV_Get_handle (I2S_BLK1_ID);
	
				// We need to check the I2S status.
				if (I2SDRV_Get_rx_state (i2sHandle) == I2STST_READY)
				{	
					I2SDRV_Set_mode(i2sHandle, TRUE); 

					
					// How to know which channel is used if it is mono?
					if (numCh == AUDIO_CHANNEL_STEREO)
					{
						I2SDRV_Set_rx_format(i2sHandle, i2sSr, TRUE, I2S_RX_LEFT_CHAN); 
					}
					else
					{
						I2SDRV_Set_rx_format(i2sHandle, i2sSr, FALSE, I2S_RX_LEFT_CHAN);
					}

					I2SDRV_Start_rx_noDMA(i2sHandle);
				}
							
				srcI2S = TRUE;	
			
			}		
			break;

		default:
			
			break;
	}

	// load sysparm, program coefficients.
	// Leave it open, will use audio_control_generic
	// audio_control_generic ();


	// speaker selection. We hardcode headset,handset and loud speaker right now. 
	// Later, need to provide a configurable table.
	spkSel = GetDrvSpk (spk);

	// tap speaker selection. Use the same spkSel if use BB speakers, 
	// otherwise use always handset/loudspeaker (AUDDRV_SPKR_IHF_STEREO).
	if((AUDIO_HW_TAP_VOICE == tap) || (AUDIO_HW_TAP_AUDIO == tap))
	{
		switch(spk)
		{
			case AUDCTRL_SPK_HANDSET:
			case AUDCTRL_SPK_LOUDSPK:
			case AUDCTRL_SPK_HEADSET:
				tapSpkSel = spkSel;
				break;
		
			default:
				// Need to add mixer selection for tap. Use these two mixers for BT TAP inputs
				// Also look at the code in AUDCTRL_EnableTap() for BT TAP inputs selection.
				//why select this?
				tapSpkSel = AUDDRV_SPKR_IHF_STEREO;
				break;
		}
	}
	
	// sink
	switch  (sink)
	{
		case AUDIO_HW_VOICE_OUT:
			if (spkSel != AUDDRV_SPKR_NONE)
			{
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay: VO spkSel = %d, sr %d \n", spkSel, sr);
				AUDDRV_Enable_Output (AUDDRV_VOICE_OUTPUT, spkSel, TRUE, sr, numCh);
			}
			if (tapSpkSel != AUDDRV_SPKR_NONE && tapSpkSel != spkSel)
			{
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay: VO tapSpkSel = %d, sr %d \n", tapSpkSel, sr);
				// just add the mixer, don't power on the speaker.
				AUDDRV_Enable_Output (AUDDRV_VOICE_OUTPUT, tapSpkSel, FALSE, sr, numCh);
			}
			voiceSpkr1 = spkSel;
			OSTASK_Sleep( 10 );
			powerOnExternalAmp( spk, VoiceUseExtSpkr, TRUE );
			break;

		case AUDIO_HW_AUDIO_OUT:
			if ( src == AUDIO_HW_I2S_IN)  //only set I2S mode when src is I2S
				AUDDRV_Set_I2sMuxToAudio (srcI2S);

                        if ( tap == AUDIO_HW_TAP_VOICE )
                        {
                            // Music plus Voice call mixing so only add mixer don't power on Speaker
                            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay(AUDIO_HW_AUDIO_OUT) spkSel = 0x%x, tapSpkSel = 0x%x BB \n", spkSel, tapSpkSel );
                            // just add the mixer, don't power on the speaker.
                            AUDDRV_Enable_Output (AUDDRV_AUDIO_OUTPUT, tapSpkSel, FALSE, sr, numCh);
                        }
                        else
                        {
			    if (spkSel != AUDDRV_SPKR_NONE)
			    {
			    	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay(AUDIO_HW_AUDIO_OUT) spkSel = 0x%x, tapSpkSel = 0x%x AA \n", spkSel, tapSpkSel );
			    	AUDDRV_Enable_Output (AUDDRV_AUDIO_OUTPUT, spkSel, TRUE, sr, numCh);
			    }
			    if (tapSpkSel != AUDDRV_SPKR_NONE && tapSpkSel != spkSel)
			    {
			    	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay(AUDIO_HW_AUDIO_OUT) spkSel = 0x%x, tapSpkSel = 0x%x BB \n", spkSel, tapSpkSel );
			    	// just add the mixer, don't power on the speaker.
			    	AUDDRV_Enable_Output (AUDDRV_AUDIO_OUTPUT, tapSpkSel, FALSE, sr, numCh);
			    }
                        }
			audioSpkr1 = spkSel;
			OSTASK_Sleep( 10 );
			powerOnExternalAmp( spk, AudioUseExtSpkr, TRUE );
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay(AUDIO_HW_AUDIO_OUT) spkSel = 0x%x, tapSpkSel = 0x%x, audioSpkr1 = 0x%x, polySpkr1 %d \n", spkSel, tapSpkSel, audioSpkr1, polySpkr1);
			break;

		case AUDIO_HW_PLR_OUT:
                        if ( tap == AUDIO_HW_TAP_VOICE )
                        {
                            // Music plus Voice call mixing so only add mixer don't power on Speaker
                            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay(AUDIO_HW_PLR_OUT) spkSel = 0x%x, tapSpkSel = 0x%x BB \n", spkSel, tapSpkSel );
                            // just add the mixer, don't power on the speaker.
                            AUDDRV_Enable_Output (AUDDRV_RINGTONE_OUTPUT, tapSpkSel, FALSE, sr, numCh);
                        }
                        else
                        {
			    if (spkSel != AUDDRV_SPKR_NONE)
			    {
			    	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay(AUDIO_HW_PLR_OUT) spkSel = 0x%x, tapSpkSel = 0x%x AA \n", spkSel, tapSpkSel );
			    	AUDDRV_Enable_Output (AUDDRV_RINGTONE_OUTPUT, spkSel, TRUE, sr, numCh);
			    }
			    if (tapSpkSel != AUDDRV_SPKR_NONE && tapSpkSel != spkSel)
			    {
			    	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay(AUDIO_HW_PLR_OUT) spkSel = 0x%x, tapSpkSel = 0x%x BB \n", spkSel, tapSpkSel );
			    	// just add the mixer, don't power on the speaker.
			    	AUDDRV_Enable_Output (AUDDRV_RINGTONE_OUTPUT, tapSpkSel, FALSE, sr, numCh);
			    }
                        }
			polySpkr1 = spkSel;
			//OSTASK_Sleep( 10 );
			powerOnExternalAmp( spk, PolyUseExtSpkr, TRUE );
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay(AUDIO_HW_PLR_OUT) spkSel = 0x%x, tapSpkSel = 0x%x, audioSpkr1 = 0x%x, polySpkr1 %d \n", spkSel, tapSpkSel, audioSpkr1, polySpkr1);
			break;

		case AUDIO_HW_MONO_BT_OUT:  // PCM I/F
			AUDDRV_Enable_Output (AUDDRV_VOICE_OUTPUT, spkSel, TRUE, sr, numCh);
			break;

		case AUDIO_HW_STEREO_BT_OUT:		
			break;

		case AUDIO_HW_USB_OUT:
			break;

		case AUDIO_HW_I2S_OUT:
			// configure I2S, don't start it.
			{	
				i2sHandle = I2SDRV_Get_handle (I2S_BLK1_ID);
	
				// We need to check the I2S status.
				if (I2SDRV_Get_tx_state (i2sHandle) == I2STST_READY)
				{	
					I2SDRV_Set_mode(i2sHandle, TRUE); 
					
					// How to know which channel is used if it is mono?
					if (numCh == AUDIO_CHANNEL_STEREO)
					{
						I2SDRV_Set_tx_format(i2sHandle, i2sSr, TRUE); 
					}
					else
					{
						I2SDRV_Set_tx_format(i2sHandle, i2sSr, FALSE);
					}
				}
			}

			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnablePlay: This audio HW can't be playback sink. sink = 0x%x\n", sink);
			break;
	}

	// in case it was muted from last play,
	//AUDCTRL_SetPlayMute (sink, spk, FALSE); 
}

//============================================================================
//
// Function Name: AUDCTRL_DisablePlay
//
// Description:   disable a playback path
//
//============================================================================
void AUDCTRL_DisablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisablePlay: end, src = 0x%x, sink = 0x%x, spk = 0x%x\n", src, sink,  spk);
	
	// don't consider src right now. only consider playback from memory.
	switch (src)
	{
		// FM direct to audio path
		case AUDIO_HW_I2S_IN:
			{
				I2S_HANDLE i2sHandle;
				i2sHandle = I2SDRV_Get_handle (I2S_BLK1_ID);
	
				// We need to check the I2S status.
				if (I2SDRV_Get_tx_state (i2sHandle) == I2STST_RUNNING_NO_DMA)
				{
					I2SDRV_Stop_rx_noDMA(i2sHandle);
				}

                AUDDRV_Set_I2sMuxToAudio (FALSE);
			}
			break;

		default:
			break;
	}

	// speaker selection. We hardcode headset,handset and loud speaker right now. 
	// Later, need to provide a configurable table.
	//spkSel = GetDrvSpk (spk);

	// sink
	switch  (sink)
	{
		case AUDIO_HW_VOICE_OUT:
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisablePlay: VOICE_OUT sink = 0x%x\n", sink);
			powerOnExternalAmp( spk, VoiceUseExtSpkr, FALSE );
			OSTASK_Sleep( 10 );
			AUDDRV_Disable_Output (AUDDRV_VOICE_OUTPUT);
			voiceSpkr1 = AUDDRV_SPKR_NONE;
			voiceSpkr2 = AUDDRV_SPKR_NONE;
			break;
		case AUDIO_HW_AUDIO_OUT:
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisablePlay: AUDIO_OUT sink = 0x%x\n", sink);
			powerOnExternalAmp( spk, AudioUseExtSpkr, FALSE );
			OSTASK_Sleep( 10 );
			AUDDRV_Disable_Output (AUDDRV_AUDIO_OUTPUT);
			audioSpkr1 = AUDDRV_SPKR_NONE;
			audioSpkr2 = AUDDRV_SPKR_NONE;
			break;
		case AUDIO_HW_PLR_OUT:
			powerOnExternalAmp( spk, PolyUseExtSpkr, FALSE );
			//OSTASK_Sleep( 10 );
			AUDDRV_Disable_Output (AUDDRV_RINGTONE_OUTPUT);
			polySpkr1 = AUDDRV_SPKR_NONE;
			polySpkr2 = AUDDRV_SPKR_NONE;
			break;
		case AUDIO_HW_MONO_BT_OUT:
			AUDDRV_Disable_Output (AUDDRV_VOICE_OUTPUT);
			break;

		case AUDIO_HW_STEREO_BT_OUT:		
			break;
		case AUDIO_HW_USB_OUT:
			break;
		case AUDIO_HW_I2S_OUT:
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisablePlay: This audio HW can't be playback sink. sink = 0x%x\n", sink);
			break;
	}
}

//============================================================================
//
// Function Name: AUDCTRL_SetPlayVolume
//
// Description:   set volume of a playback path
//
//============================================================================
void AUDCTRL_SetPlayVolume(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_GAIN_FORMAT_t     gainF,
				UInt32					vol_left,
				UInt32					vol_right
				)
{
	// right now, only set gain on sink

	UInt32 gain_hexL=0, gain_hexR=0;

	//Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: sink = 0x%x,  spk = 0x%x, vol = 0x%x\n", sink, spk, vol_left);
    switch (gainF)
	{
		case AUDIO_GAIN_FORMAT_VOL_LEVEL:
			// right now, convert vol to gain register value. 
		    // Later, we need to have a gain mappping mechanism.
		    ///gain_hexL = (0x1ff - 40*4) + (vol_left * 4);
			///gain_hexR = (0x1ff - 40*4) + (vol_right * 4);
			break;
		case AUDIO_GAIN_FORMAT_HW_REG:
	        // directly write gain register value. 
	        gain_hexL = vol_left;
		    gain_hexR = vol_right;
        	break;
		case AUDIO_GAIN_FORMAT_Q14_1:
	        // Need to convert Q14.1 gain to gain register value. 
    	    gain_hexL = (UInt32)AUDTABL_getQ14_1HWSlopGain(vol_left);
	    	gain_hexR = (UInt32)AUDTABL_getQ14_1HWSlopGain(vol_right);
        	break;
		case AUDIO_GAIN_FORMAT_Q1_14:
	        // Need to convert Q1.14 gain to gain register value. 
    	    gain_hexL = (UInt32)AUDTABL_getQ1_14HWSlopGain(vol_left);
	    	gain_hexR = (UInt32)AUDTABL_getQ1_14HWSlopGain(vol_right);
        	break;
		default:
        	xassert(0, gainF);
			break;
	}

	if( AUDIO_VOLUME_MUTE == vol_left && gainF==AUDIO_GAIN_FORMAT_VOL_LEVEL )
	{
		AUDCTRL_SetPlayMute( sink, spk, TRUE );
	}
	else
	{
		UInt32 slopgain = 0x1ff;
		AudioMode_t mode;

		AudioApp_t app = AUDDRV_GetAudioApp();

	    // sink
		switch  (sink)
		{
			case AUDIO_HW_VOICE_OUT:
				mode = AUDDRV_GetAudioMode();

				if ( mode >= AUDIO_MODE_NUMBER )
					mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);
				//slopgain = (AUDIO_GetParmAccessPtr()+app*AUDIO_MODE_NUMBER)[mode].audvoc_vslopgain;

                slopgain = AUDIOMODE_PARM_ACCESSOR(app,mode).audvoc_vslopgain;
                
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: format = 0x%x mode = 0x%x slopgain = 0x%x\n", gainF, mode, slopgain);

				switch (gainF)
				{
					case AUDIO_GAIN_FORMAT_VOL_LEVEL:
						// Later, we need to have a gain mapping mechanism.
						gain_hexL = slopgain;
						break;
					case AUDIO_GAIN_FORMAT_HW_REG:
                         gain_hexL = slopgain;
        				break;
					case AUDIO_GAIN_FORMAT_Q14_1:
        				break;
					case AUDIO_GAIN_FORMAT_Q1_14:
        				break;
					case AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN:
                        gain_hexL = slopgain;
        				break;
					default:
        				xassert(0, gainF);
						break;
				}
				AUDDRV_SetGain_Hex	(AUDDRV_GAIN_VOICE_OUTPUT, gain_hexL);
				// if DSP init is done,
				// should change the playback volume in DSP, not change voice call volume.
				AUDCTRL_SetTelephonySpkrVolume( AUDIO_HW_NONE, spk, vol_left, gainF );
				break;

			case AUDIO_HW_AUDIO_OUT:
				mode = AUDDRV_GetAudioMode();
				if ( mode >= AUDIO_MODE_NUMBER )
					mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);
				//slopgain = AUDIO_GetParmMMAccessPtr()[ mode ].audvoc_aslopgain;
                slopgain = AUDIOMODE_PARM_MM_ACCESSOR(app,mode).audvoc_aslopgain;
                
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: sysparm aslopgain = 0x%x\n", slopgain);
				switch (gainF)
				{
					case AUDIO_GAIN_FORMAT_VOL_LEVEL:
						// Later, we need to have a gain mappping mechanism.
						gain_hexL = (slopgain - 40*4) + (vol_left * 4);
						gain_hexR = (slopgain - 40*4) + (vol_right * 4);
						break;
					case AUDIO_GAIN_FORMAT_HW_REG:
        				break;
					case AUDIO_GAIN_FORMAT_Q14_1:
        				break;
					case AUDIO_GAIN_FORMAT_Q1_14:
        				break;
					case AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN:
        				break;

                                        case AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE:
						{
						AudioApp_t app = AUDDRV_GetAudioApp();
						AudioMode_t mode = AUDDRV_GetAudioMode();
						UInt16 aslopgain;

                                                gain_hexL = vol_left;
                                                gain_hexR  = vol_right;
                        			if ( gain_hexL == AUDIO_VOLUME_FM_MUTE)  {
                                                     Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: L MUTE ");
                                                     gain_hexL = 0x000;

                        			}
                                                else {
                         	 	             Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: gain_hexL = 0x%x\n", gain_hexL);
                         	 	             if(gain_hexL >= NUM_OF_ENTRY_IN_FM_RADIO_DIGITAL_VOLUME)
                                                          gain_hexL = NUM_OF_ENTRY_IN_FM_RADIO_DIGITAL_VOLUME-1;

                         	 	             Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: gain_hexL = 0x%x\n", gain_hexL);
                         	 	             aslopgain = AUDIOMODE_PARM_MM_ACCESSOR(app, mode).fm_radio_digital_vol[ gain_hexL ];

                         	 	             Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: sysparm aslopgain = 0x%x\n", aslopgain);
                         	 	             gain_hexL = aslopgain;
                        			}
                        			if ( gain_hexR == AUDIO_VOLUME_FM_MUTE)  {
                                                     Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: R MUTE ");
                                                     gain_hexR = 0x000;								  

                        			}
                                                else {
                                                     Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: gain_hexR = 0x%x\n", gain_hexR);
                                                     if( gain_hexR >= NUM_OF_ENTRY_IN_FM_RADIO_DIGITAL_VOLUME)
                                                         gain_hexR = NUM_OF_ENTRY_IN_FM_RADIO_DIGITAL_VOLUME-1;
                                                     aslopgain = AUDIOMODE_PARM_MM_ACCESSOR(app, mode).fm_radio_digital_vol[ gain_hexR ];
                                                     gain_hexR = aslopgain;
						}
					}
					break;

					default:
        				xassert(0, gainF);
						break;
				}
				//Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: slopgain = 0x%x\n", slopgain);
				AUDDRV_SetGain_Hex (AUDDRV_GAIN_AUDIO_OUTPUT_L, gain_hexL);
				AUDDRV_SetGain_Hex (AUDDRV_GAIN_AUDIO_OUTPUT_R, gain_hexR);
				break;

			case AUDIO_HW_PLR_OUT:
				mode = AUDDRV_GetAudioMode();
				if ( mode >= AUDIO_MODE_NUMBER )
					mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);
				//slopgain = AUDIO_GetParmMMAccessPtr()[ mode ].audvoc_pslopgain;
                slopgain = AUDIOMODE_PARM_MM_ACCESSOR(app,mode).audvoc_pslopgain;

				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: sysparm pslopgain = 0x%x\n", slopgain);
				switch (gainF)
				{
					case AUDIO_GAIN_FORMAT_VOL_LEVEL:
						// Later, we need to have a gain mappping mechanism.
						gain_hexL = (slopgain - 40*4) + (vol_left * 4);
						gain_hexR = (slopgain - 40*4) + (vol_right * 4);
						break;
					case AUDIO_GAIN_FORMAT_HW_REG:
        				break;
					case AUDIO_GAIN_FORMAT_Q14_1:
        				break;
					case AUDIO_GAIN_FORMAT_Q1_14:
        				break;
					case AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN:
        				break;
					default:
        				xassert(0, gainF);
						break;
				}
				AUDDRV_SetGain_Hex (AUDDRV_GAIN_RINGTONE_OUTPUT_L, gain_hexL);
				AUDDRV_SetGain_Hex (AUDDRV_GAIN_RINGTONE_OUTPUT_R, gain_hexR);
				break;

			case AUDIO_HW_MONO_BT_OUT:
				break;

			case AUDIO_HW_STEREO_BT_OUT:		
				break;
			case AUDIO_HW_USB_OUT:
				break;
			case AUDIO_HW_I2S_OUT:
				break;

			default:
				//Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: This audio HW can't be playback sink. sink = 0x%x\n", sink);
				break;
		}
	}
}

//============================================================================
//
// Function Name: AUDCTRL_SetPlayMute
//
// Description:   mute/unmute a playback path
//
//============================================================================
void AUDCTRL_SetPlayMute(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
	// right now, only mute on sink
	// sink

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayMute: Set Play mute. sink = 0x%x,  spk = 0x%x, mute = 0x%x\n", sink, spk, mute);
	
	switch  (sink)
	{
        // don't want to mute or turn off external amp since multiple paths (voice, audio, etc) could be sharing 
        // the same external Headset amp or IHF amp
		case AUDIO_HW_VOICE_OUT:
			AUDDRV_SetMute  (AUDDRV_MUTE_VOICE_OUTPUT, mute);
			break;
		case AUDIO_HW_AUDIO_OUT:
			AUDDRV_SetMute (AUDDRV_MUTE_AUDIO_OUTPUT_L, mute);
			AUDDRV_SetMute (AUDDRV_MUTE_AUDIO_OUTPUT_R, mute);
			break;
		case AUDIO_HW_PLR_OUT:
			AUDDRV_SetMute (AUDDRV_MUTE_RINGTONE_OUTPUT_L, mute);
			AUDDRV_SetMute (AUDDRV_MUTE_RINGTONE_OUTPUT_R, mute);
			break;
		case AUDIO_HW_MONO_BT_OUT:
			break;

		case AUDIO_HW_STEREO_BT_OUT:		
			break;
		case AUDIO_HW_USB_OUT:
			break;
		case AUDIO_HW_I2S_OUT:
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayMute: This audio HW can't be playback sink. sink = 0x%x\n", sink);
			break;
	}

}


//============================================================================
//
// Function Name: AUDCTRL_AddPlaySpk
//
// Description:   add a speaker to a playback path
//
//============================================================================
void AUDCTRL_AddPlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_AddPlaySpk: Add speaker, sink = 0x%x,  spk = 0x%x\n", sink, spk);
//audioMode()?
//power on/off audio path.

	switch  (sink)
	{
		case AUDIO_HW_VOICE_OUT:
			if ( voiceCallSpkr == AUDDRV_SPKR_NONE )
			{
				if ( voiceSpkr1 == AUDDRV_SPKR_NONE )
					voiceSpkr1 = GetDrvSpk (spk);
				else
					voiceSpkr2 = GetDrvSpk (spk);
			}
			else
			{
				voiceSpkr1 = voiceCallSpkr;
				
				if ( voiceSpkr1 != GetDrvSpk(spk) )
					voiceSpkr2 = GetDrvSpk (spk);
			}
			AUDDRV_SelectSpkr( AUDDRV_VOICE_OUTPUT, voiceSpkr1, voiceSpkr2);
			OSTASK_Sleep( 100 );
			powerOnExternalAmp( spk, VoiceUseExtSpkr, TRUE );
			break;

		case AUDIO_HW_AUDIO_OUT:
			// to set speaker, set primary speaker, leave secondary speaker none
			// to add speaker, set primary speaker unchanged, set secondary speaker.  AUDDRV_SPKR_NONE means keep the whatever primary spkr as it is.
			// to enable two speaker, set primary speaker/secondary speaker
			if ( audioSpkr1 == AUDDRV_SPKR_NONE )
				audioSpkr1 = GetDrvSpk (spk);
			else
				audioSpkr2 = GetDrvSpk (spk);

			AUDDRV_SelectSpkr( AUDDRV_AUDIO_OUTPUT, audioSpkr1, audioSpkr2 );
			OSTASK_Sleep( 100 );
			powerOnExternalAmp( spk, AudioUseExtSpkr, TRUE );
			break;

		case AUDIO_HW_PLR_OUT:
			if ( polySpkr1 == AUDDRV_SPKR_NONE )
				polySpkr1 = GetDrvSpk (spk);
			else
				polySpkr2 = GetDrvSpk (spk);

			AUDDRV_SelectSpkr( AUDDRV_RINGTONE_OUTPUT, polySpkr1, polySpkr2 );
			//OSTASK_Sleep( 100 );
			powerOnExternalAmp( spk, PolyUseExtSpkr, TRUE );
			break;

		default:
			break;
	}
}

//============================================================================
//
// Function Name: AUDCTRL_RemovePlaySpk
//
// Description:   remove a speaker to a playback path
//
//============================================================================
void AUDCTRL_RemovePlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_RemovePlaySpk: Remove speaker, sink = 0x%x,  spk = 0x%x\n", sink, spk);
		
	switch  (sink)
	{
		case AUDIO_HW_VOICE_OUT:
			if ( voiceCallSpkr == AUDDRV_SPKR_NONE )
			{
				if ( voiceSpkr2 == GetDrvSpk (spk) )
				{
					voiceSpkr2 = AUDDRV_SPKR_NONE;
				}
				else
				if ( voiceSpkr1 == GetDrvSpk (spk) )
				{
					voiceSpkr1 = voiceSpkr2;
					voiceSpkr2 = AUDDRV_SPKR_NONE;
				}
			}
			else
			{
				if ( voiceSpkr2 == GetDrvSpk (spk) )
				{
					voiceSpkr2 = AUDDRV_SPKR_NONE;
				}
				voiceSpkr1 = voiceCallSpkr;
			}
			powerOnExternalAmp( spk, VoiceUseExtSpkr, FALSE );
			OSTASK_Sleep( 100 );
			AUDDRV_SelectSpkr( AUDDRV_VOICE_OUTPUT, voiceSpkr1, voiceSpkr2 );
			break;

		case AUDIO_HW_AUDIO_OUT:
			if ( audioSpkr2 == GetDrvSpk (spk) )
			{
				audioSpkr2 = AUDDRV_SPKR_NONE;
			}
			else
			if ( audioSpkr1 == GetDrvSpk (spk) )
			{
				audioSpkr1 = audioSpkr2;
				audioSpkr2 = AUDDRV_SPKR_NONE;
			}
			// to remove speaker
			powerOnExternalAmp( spk, AudioUseExtSpkr, FALSE );
			OSTASK_Sleep( 100 );
			AUDDRV_SelectSpkr( AUDDRV_AUDIO_OUTPUT, audioSpkr1, audioSpkr2 );
			break;

		case AUDIO_HW_PLR_OUT:
			if ( polySpkr2 == GetDrvSpk (spk) )
			{
				polySpkr2 = AUDDRV_SPKR_NONE;
			}
			else
			if ( polySpkr1 == GetDrvSpk (spk) )
			{
				polySpkr1 = polySpkr2;
				polySpkr2 = AUDDRV_SPKR_NONE;
			}
			powerOnExternalAmp( spk, PolyUseExtSpkr, FALSE );
			//OSTASK_Sleep( 100 );
			AUDDRV_SelectSpkr( AUDDRV_RINGTONE_OUTPUT, polySpkr1, polySpkr2 );
			break;

		default:
			break;
	}
}


//============================================================================
//
// Function Name: AUDCTRL_EnableRecord
//
// Description:   enable a record path
//
//============================================================================
void AUDCTRL_EnableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
	AUDDRV_MIC_Enum_t micSel;
	
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableRecord: src = 0x%x, sink = 0x%x,  mic = 0x%x, sr %d\n", src, sink, mic, sr);

	micSel = GetDrvMic (mic);

	switch (src)
	{
		case AUDIO_HW_VOICE_IN:
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableRecord: voice in, mode = %d\n", AUDDRV_GetAudioMode() );
			AUDDRV_Enable_Input (AUDDRV_VOICE_INPUT, micSel, sr);
			break;

		case AUDIO_HW_AUDIO_IN:
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableRecord: audio in, mode = %d\n", AUDDRV_GetAudioMode() );
			AUDDRV_Enable_Input (AUDDRV_AUDIO_INPUT, micSel, sr);
			break;

		case AUDIO_HW_MONO_BT_IN:
			break;

		case AUDIO_HW_STEREO_BT_IN:
			break;

		case AUDIO_HW_USB_IN:
			break;

			// FM recording. need to sync with FM playback. 
		case AUDIO_HW_I2S_IN:
			break;

		default:
			break;
	}

	// in case it was muted from last record
	//AUDCTRL_SetRecordMute (src, mic, FALSE); 

	// don't consider sink right now. only consider record to memory.

}

//============================================================================
//
// Function Name: AUDCTRL_DisableRecord
//
// Description:   disable a record path
//
//============================================================================
void AUDCTRL_DisableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic
				)
{
	//AUDDRV_MIC_Enum_t micSel;
	
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisableRecord: Disabled record path, src = 0x%x, sink = 0x%x,  mic = 0x%x\n", src, sink, mic);

	//micSel = GetDrvMic (mic);

	switch (src)
	{
		case AUDIO_HW_VOICE_IN:
			AUDDRV_Disable_Input (AUDDRV_VOICE_INPUT);
			break;

		case AUDIO_HW_AUDIO_IN:
			AUDDRV_Disable_Input (AUDDRV_AUDIO_INPUT);
			break;

		case AUDIO_HW_MONO_BT_IN:
			break;

		case AUDIO_HW_STEREO_BT_IN:
			break;

		case AUDIO_HW_USB_IN:
			break;

			// FM recording. need to sync with FM playback. 
		case AUDIO_HW_I2S_IN:
			break;

		default:
			
			break;
	}

	// don't consider sink right now. only consider record to memory.

}

//============================================================================
//
// Function Name: AUDCTRL_EnableTap
//
// Description:   enable a tap path
//
//============================================================================
void AUDCTRL_EnableTap(
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
	// Based on the analysis of the audio use cases, we decided to always use the mixer ouput
	// selections of earpiece and loud speaker as the tap inputs.
    AUDDRV_SPKR_Enum_t leftIn = AUDDRV_SPKR_NONE; 
    AUDDRV_SPKR_Enum_t rightIn = AUDDRV_SPKR_NONE;  

	// figure tap left input and right input.
	if((AUDIO_HW_TAP_VOICE == tap) || (AUDIO_HW_TAP_AUDIO == tap))
	{
		switch(spk)
		{
			case AUDCTRL_SPK_HANDSET:
				leftIn = rightIn = AUDDRV_SPKR_EP;
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTap: HANDSET\n");
				break;
			case AUDCTRL_SPK_LOUDSPK:
				leftIn = rightIn = AUDDRV_SPKR_IHF;
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTap: LOUDSPK\n");
				break;
			
			case AUDCTRL_SPK_HEADSET:
				leftIn = AUDDRV_SPKR_HS_LEFT;
				rightIn = AUDDRV_SPKR_HS_RIGHT;
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTap: HEADSET\n");
				break;
		
			default:
				// for BTS Speaker, we choose the following two mixer outputs for BT TAP inputs.
				// Also look at the code in AUDCTRL_EnablePlay() that enables these two mixers
				leftIn = /*why not AUDDRV_SPKR_HS_LEFT*/  AUDDRV_SPKR_EP;
				rightIn = /*why not AUDDRV_SPKR_HS_RIGHT*/ AUDDRV_SPKR_IHF;
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTap: others like A2DP, HEADSET\n");
				break;
		}
	}

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTap: Enabled Tap, leftIn = 0x%x,  rightIn = 0x%x\n", leftIn, rightIn);

	switch (tap)
	{
		case AUDIO_HW_TAP_VOICE:
			// We always configure voice TAP as 8K, and enable BTM(PCM interface)
			//sr = AUDIO_SAMPLING_RATE_8000;
			AUDDRV_Enable_MixerTap ( AUDDRV_MIXERTap_VB_INPUT, leftIn, rightIn, sr,
				AUDDRV_REASON_HW_CTRL );
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTap:  AUDIO_HW_TAP_VOICE sr %d \n",  sr);

                        if ( AUDDRV_GetVCflag() )
                        {
                                Log_DebugPrintf(LOGID_AUDIO," AUDCTRL_EnableTap arg0 = 0x%x. VC, No Need to init telephony\n", 0x11d);
                        }
                        else
                        {
                                AUDDRV_Telephony_Init( AUDDRV_MIC_PCM_IF, AUDDRV_SPKR_PCM_IF );
                                Log_DebugPrintf(LOGID_AUDIO," AUDCTRL_EnableTap arg0 = 0x%x. and init telephony\n", 0x11d);
                        }

          
			VPRIPCMDQ_SetBTNarrowBand( 0x55d );

			//bt-nb tap does not work because of this line. reason is not very clear.
			    //BTW, We know there is no need to run ARM2SP and BT-nb tap tapping voice path at the same time.(?)
			  //if this line is needed we can move this line to AUDCTRL_AddPlaySpk() or AUDCTRL_EnablePlay( ).
			//comment our this line for now to make bt-nb tap work.
			//AUDDRV_Enable_Output (AUDDRV_VOICE_OUTPUT, AUDDRV_SPKR_PCM_IF, TRUE, sr);

			  //if use this line, BT-nb tap works:
			//AUDDRV_Enable_Output (AUDDRV_VOICE_OUTPUT, AUDDRV_SPKR_EP, TRUE, sr);
			break;

		case AUDIO_HW_TAP_AUDIO:
			// we always configure BT TAP as 48K
			sr = AUDIO_SAMPLING_RATE_48000;
			AUDDRV_Enable_MixerTap ( AUDDRV_MIXERTap_WB_INPUT, leftIn, rightIn, sr,
				AUDDRV_REASON_HW_CTRL );
			break;

		default:
			break;
	}

	// hardcode BT tap gain, no gain change.
	AUDCTRL_SetTapGain(tap, 0x1fff);			
}

//============================================================================
//
// Function Name: AUDCTRL_DisableTap
//
// Description:   disable a tap path
//
//============================================================================
void AUDCTRL_DisableTap( AUDIO_HW_ID_t	tap)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisableTap: Disabled Tap, tap = 0x%x\n", tap);
	
	switch (tap)
	{
		case AUDIO_HW_TAP_VOICE:
			VPRIPCMDQ_SetBTNarrowBand( 0 );

			if ( !AUDDRV_GetVCflag() )
			{
			    AUDDRV_Telephony_Deinit();
			}

			AUDDRV_Disable_MixerTap ( AUDDRV_MIXERTap_VB_INPUT, AUDDRV_REASON_HW_CTRL );
			break;

		case AUDIO_HW_TAP_AUDIO:
			AUDDRV_Disable_MixerTap ( AUDDRV_MIXERTap_WB_INPUT, AUDDRV_REASON_HW_CTRL );
			break;

		default:		
			break;
	}

}

//============================================================================
//
// Function Name: AUDCTRL_SetTapGain
//
// Description:   set gain of a tap path
//
//============================================================================
void AUDCTRL_SetTapGain(
				AUDIO_HW_ID_t			tap,
				UInt32					gain
				)
{

	UInt32 gain_hex;	//in mDB

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTapGain:	tap = 0x%x,  gain = 0x%x\n", tap, gain);

	switch (tap)
	{
		case AUDIO_HW_TAP_VOICE: 
		// No voice tap gain control
			break;

		case AUDIO_HW_TAP_AUDIO:
			gain_hex = gain;
			AUDDRV_SetGain_Hex(AUDDRV_GAIN_MIXERTap_WB_L, gain_hex);
			AUDDRV_SetGain_Hex(AUDDRV_GAIN_MIXERTap_WB_R, gain_hex);
		
			break;

		default:
			
			break;
	}

}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordGain
//
// Description:   set gain of a record path
//
//============================================================================
void AUDCTRL_SetRecordGain(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				UInt32					gainL,
				UInt32					gainR
				)
{
	// right now, only set gain on sink

	UInt32 gain_hex;	//in mDB
	UInt32 gain = gainL;
	//AUDDRV_MIC_Enum_t micSel;

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetRecordGain: Set Record gain. src = 0x%x,  mic = 0x%x, gain = 0x%x\n", src, mic, gain);

	//micSel = GetDrvMic (mic);

	switch (src)
	{
		case AUDIO_HW_VOICE_IN:
			// right now, convert vol to gain register value. 
			// Later, we need to have a gain mappping mechanism.
			// Conversion should be done in driver?

			//only analog mic has GAIN setting
			if ((AUDCTRL_MIC_MAIN == mic) || (AUDCTRL_MIC_AUX == mic))
			{
				//gain_hex = gain * 100;
                gain_hex = gain;
				AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIC, gain_hex );
			}
			break;

		case AUDIO_HW_AUDIO_IN:
			//gain setting for audio in
			break;

		case AUDIO_HW_MONO_BT_IN:
			//no HW gain in NB bt tap.
			break;

		case AUDIO_HW_STEREO_BT_IN:
			gain_hex = gain;
			AUDDRV_SetGain_Hex(AUDDRV_GAIN_MIXERTap_WB_L, gain_hex);
			AUDDRV_SetGain_Hex(AUDDRV_GAIN_MIXERTap_WB_R, gain_hex);
			break;

		case AUDIO_HW_USB_IN:
			break;

			// FM recording. need to sync with FM playback. 
		case AUDIO_HW_I2S_IN:
			break;

		default:
			
			break;
	}

	// don't consider sink right now. only consider record to memory.

}


//============================================================================
//
// Function Name: AUDCTRL_SetRecordMute
//
// Description:   mute/unmute a record path
//
//============================================================================
void AUDCTRL_SetRecordMute(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetRecordMute: src = 0x%x,  mic = 0x%x, mute = 0x%x\n", src, mic, mute);

	switch (src)
	{
		case AUDIO_HW_VOICE_IN:
		case AUDIO_HW_AUDIO_IN:
			if ((mic == AUDCTRL_MIC_MAIN) || (mic == AUDCTRL_MIC_AUX)) 
			{ //now only can mute analog mic
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetRecordMute: analog mic \n");
				AUDDRV_SetMute(AUDDRV_MUTE_MIC, mute);
			}
			else if ((mic >= AUDCTRL_MIC_DIGI1) && (mic <= AUDCTRL_DUAL_MIC_DIGI21))
			{
				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetRecordMute: can not mute DMIC \n");
			}
			break;



		case AUDIO_HW_MONO_BT_IN:
			break;

		case AUDIO_HW_STEREO_BT_IN:
			break;

		case AUDIO_HW_USB_IN:
			break;

			// FM recording. need to sync with FM playback. 
		case AUDIO_HW_I2S_IN:
			break;

		default:
			
			break;
	}

}

//============================================================================
//
// Function Name: AUDCTRL_AddRecordMic
//
// Description:   add a microphone to a record path
//
//============================================================================
void AUDCTRL_AddRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				)
{
	AUDDRV_MIC_Enum_t micSel;

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_AddRecordMic: Add mic, src = 0x%x,  mic = 0x%x\n", src, mic);

	micSel = GetDrvMic (mic);

	switch (src)
	{
		case AUDIO_HW_VOICE_IN:
			AUDDRV_SelectMic (AUDDRV_VOICE_INPUT, micSel);
			break;

		case AUDIO_HW_AUDIO_IN:
			AUDDRV_SelectMic (AUDDRV_AUDIO_INPUT, micSel);
			break;

		case AUDIO_HW_MONO_BT_IN:
			break;

		case AUDIO_HW_STEREO_BT_IN:
			break;

		case AUDIO_HW_USB_IN:
			break;

			// FM recording. need to sync with FM playback. 
		case AUDIO_HW_I2S_IN:
			break;

		default:
			
			break;
	}
}

//============================================================================
//
// Function Name: AUDCTRL_RemoveRecordMic
//
// Description:   remove a microphone from a record path
//
//============================================================================
void AUDCTRL_RemoveRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				)
{
	// Nothing to do.
}

void AUDCTRL_SetAudioLoopback( 
							Boolean					enable_lpbk,
							AUDCTRL_MICROPHONE_t	mic,
							AUDCTRL_SPEAKER_t		speaker
							)
{
	if ( enable_lpbk )
	{
		Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback::  mic %d, speaker %d \n", mic, speaker  );

		//enable baseband audio path and external audio device (in PMU)

		//PMU audio gain needs correct audio mode.
		if ( speaker == AUDCTRL_SPK_HANDSET )
			AUDCTRL_SetAudioMode ( AUDIO_MODE_HANDSET, stAudioApp );
		else
		if ( speaker == AUDCTRL_SPK_HEADSET)
			AUDCTRL_SetAudioMode ( AUDIO_MODE_HEADSET, stAudioApp );
		else
		if ( speaker == AUDCTRL_SPK_LOUDSPK )
			AUDCTRL_SetAudioMode ( AUDIO_MODE_SPEAKERPHONE, stAudioApp );

		AUDCTRL_EnablePlay(
				AUDIO_HW_NONE,       //src
				AUDIO_HW_VOICE_OUT,  //sink
				AUDIO_HW_NONE,		//tap,
				speaker,
				AUDIO_CHANNEL_MONO,
				AUDIO_SAMPLING_RATE_8000
				);
		AUDCTRL_EnableRecord(
				AUDIO_HW_VOICE_IN,	//src,
				AUDIO_HW_NONE,		//sink,
				mic,
				AUDIO_CHANNEL_MONO,
				AUDIO_SAMPLING_RATE_8000
				);
		AUDDRV_SetAudioLoopback( TRUE, GetDrvMic (mic), GetDrvSpk (speaker) );
	}
	else
	{
		AUDDRV_SetAudioLoopback( FALSE, AUDDRV_MIC_NONE, AUDDRV_SPKR_NONE );
		AUDCTRL_DisablePlay(
				AUDIO_HW_NONE,       //src
				AUDIO_HW_VOICE_OUT,  //sink
				speaker
				);
		AUDCTRL_DisableRecord(
				AUDIO_HW_VOICE_IN,	//src,
				AUDIO_HW_NONE,		//sink,
				mic
				);
	}
}

void AUDCTRL_SetEQ( 
				AUDIO_HW_ID_t	audioPath,
				AudioEqualizer_en_t  equType
				)
{
	AUDDRV_SetEquType( AUDDRV_TYPE_AUDIO_OUTPUT, equType );
	AUDDRV_SetEquType( AUDDRV_TYPE_RINGTONE_OUTPUT, equType );
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

#ifdef UNDER_LINUX


#if defined(PMU_MAX8986)  //maxim change later

typedef enum
{
	PMU_HSGAIN_MUTE = -1,
	PMU_HSGAIN_64DB_N = 0x00,
	PMU_HSGAIN_60DB_N,
	PMU_HSGAIN_56DB_N,
	PMU_HSGAIN_52DB_N,
	PMU_HSGAIN_48DB_N,
	PMU_HSGAIN_44DB_N,
	PMU_HSGAIN_40DB_N,
	PMU_HSGAIN_37DB_N,
	PMU_HSGAIN_34DB_N,
	PMU_HSGAIN_31DB_N,
	PMU_HSGAIN_28DB_N,
	PMU_HSGAIN_25DB_N,
	PMU_HSGAIN_22DB_N,
	PMU_HSGAIN_19DB_N,
	PMU_HSGAIN_16DB_N,
	PMU_HSGAIN_14DB_N,
	PMU_HSGAIN_12DB_N,
	PMU_HSGAIN_10DB_N,
	PMU_HSGAIN_8DB_N,
	PMU_HSGAIN_6DB_N,
	PMU_HSGAIN_4DB_N,
	PMU_HSGAIN_2DB_N,
    PMU_HSGAIN_1DB_N,
    PMU_HSGAIN_0DB,
    PMU_HSGAIN_1DB_P,
    PMU_HSGAIN_2DB_P,
    PMU_HSGAIN_3DB_P,
    PMU_HSGAIN_4DB_P,
    PMU_HSGAIN_4P5DB_P,
    PMU_HSGAIN_5DB_P,
    PMU_HSGAIN_5P5DB_P,
    PMU_HSGAIN_6DB_P
}PMU_HS_Gain_t;

typedef enum
{
    PMU_IHFGAIN_MUTE,
	PMU_IHFGAIN_30DB_N=0x18,
	PMU_IHFGAIN_26DB_N,
	PMU_IHFGAIN_22DB_N,
	PMU_IHFGAIN_18DB_N,
	PMU_IHFGAIN_14DB_N,
	PMU_IHFGAIN_12DB_N,
	PMU_IHFGAIN_10DB_N,
	PMU_IHFGAIN_8DB_N,
	PMU_IHFGAIN_6DB_N,
	PMU_IHFGAIN_4DB_N,
	PMU_IHFGAIN_2DB_N,
	PMU_IHFGAIN_0DB,
	PMU_IHFGAIN_1DB_P,
	PMU_IHFGAIN_2DB_P,
	PMU_IHFGAIN_3DB_P,
	PMU_IHFGAIN_4DB_P,
    PMU_IHFGAIN_5DB_P,
    PMU_IHFGAIN_6DB_P,
    PMU_IHFGAIN_7DB_P,
    PMU_IHFGAIN_8DB_P,
    PMU_IHFGAIN_9DB_P,
    PMU_IHFGAIN_10DB_P,
    PMU_IHFGAIN_11DB_P,
    PMU_IHFGAIN_12DB_P,
    PMU_IHFGAIN_12P5DB_P,
    PMU_IHFGAIN_13DB_P,
    PMU_IHFGAIN_13P5DB_P,
    PMU_IHFGAIN_14DB_P,
    PMU_IHFGAIN_14P5DB_P,
    PMU_IHFGAIN_15DB_P,
    PMU_IHFGAIN_15P5DB_P,
    PMU_IHFGAIN_16DB_P,
    PMU_IHFGAIN_16P5DB_P,
    PMU_IHFGAIN_17DB_P,
    PMU_IHFGAIN_17P5DB_P,
    PMU_IHFGAIN_18DB_P,
    PMU_IHFGAIN_18P5DB_P,
    PMU_IHFGAIN_19DB_P,
    PMU_IHFGAIN_19P5DB_P,
    PMU_IHFGAIN_20DB_P
}PMU_IHF_Gain_t;

#if 0
static PMU_HS_Gain_t map2pmu_hs_gain( Int16 db_gain )
{
	
	return PMU_HSGAIN_5DB_P;
}

static PMU_IHF_Gain_t map2pmu_ihf_gain( Int16 db_gain )
{
	return PMU_IHFGAIN_6DB_P;//PMU_IHFGAIN_20DB_P;//PMU_IHFGAIN_6DB_P;//PMU_IHFGAIN_0DB ;//PMU_IHFGAIN_18P5DB_P;
}
#endif

#else
static int map2pmu_hs_gain( Int16 db_gain )
{
		
	//return BCM59038_HSGAIN_3DB_N;
    return 1;
}

static int map2pmu_ihf_gain( Int16 db_gain )
{
	//return BCM59038_IHFGAIN_0DB;
    return 1;
}
#endif


#else

#if !defined(NO_PMU) && ( defined( PMU_BCM59038) || defined( PMU_BCM59055 ) || defined( PMU_MAX8986) )

#if defined(PMU_MAX8986)  //maxim change later

static PMU_HS_Gain_t map2pmu_hs_gain( Int16 db_gain )
{
	
	return PMU_HSGAIN_5DB_P;
}

static PMU_IHF_Gain_t map2pmu_ihf_gain( Int16 db_gain )
{
	return PMU_IHFGAIN_18P5DB_P;
}
#else
static PMU_HS_Gain_t map2pmu_hs_gain( Int16 db_gain )
{
	if ( db_gain== -18 )  	return PMU_HSGAIN_18DB_N;
	if ( db_gain== -17)  	return PMU_HSGAIN_17DB_N;
	if ( db_gain== -16)		return PMU_HSGAIN_16DB_N;
	if ( db_gain== -15) 	return PMU_HSGAIN_15DB_N;
	if ( db_gain== -14)		return PMU_HSGAIN_14DB_N;
	if ( db_gain== -13)		return PMU_HSGAIN_13DB_N;
	if ( db_gain== -12)		return PMU_HSGAIN_12DB_N;
	if ( db_gain== -11)		return PMU_HSGAIN_11DB_N;
	if ( db_gain== -10)		return PMU_HSGAIN_10DB_N;
	if ( db_gain== -9)		return PMU_HSGAIN_9DB_N;
	if ( db_gain== -8)		return PMU_HSGAIN_8DB_N;
	if ( db_gain== -7)		return PMU_HSGAIN_7DB_N;
	if ( db_gain== -6)		return PMU_HSGAIN_6DB_N;
	if ( db_gain== -5)		return PMU_HSGAIN_5DB_N;
	if ( db_gain== -4)		return PMU_HSGAIN_4DB_N;
	if ( db_gain== -3)		return PMU_HSGAIN_3DB_N;
	if ( db_gain== -2)		return PMU_HSGAIN_2DB_N;
	
	return PMU_HSGAIN_3DB_N;
}

static PMU_IHF_Gain_t map2pmu_ihf_gain( Int16 db_gain )
{
	if ( db_gain== -18 )	return PMU_IHFGAIN_18DB_N;
	if ( db_gain== -17 )	return PMU_IHFGAIN_16P5DB_N;
	if ( db_gain== -16 )	return PMU_IHFGAIN_16DB_N;
	if ( db_gain== -15 )	return PMU_IHFGAIN_15DB_N;
	if ( db_gain== -14 )	return PMU_IHFGAIN_14DB_N;
	if ( db_gain== -13 )	return PMU_IHFGAIN_13DB_N;
	if ( db_gain== -12 )	return PMU_IHFGAIN_12DB_N;
	if ( db_gain== -11 )	return PMU_IHFGAIN_11DB_N;
	if ( db_gain== -10 )	return PMU_IHFGAIN_10DB_N;
	if ( db_gain== -9 )		return PMU_IHFGAIN_9DB_N;
	if ( db_gain== -8 )		return PMU_IHFGAIN_8DB_N;
	if ( db_gain== -7 )		return PMU_IHFGAIN_7DB_N;
	if ( db_gain== -6 )		return PMU_IHFGAIN_6DB_N;
	if ( db_gain== -5 )		return PMU_IHFGAIN_5DB_N;
	if ( db_gain== -4 )		return PMU_IHFGAIN_4DB_N;
	if ( db_gain== -3 )		return PMU_IHFGAIN_3DB_N;
	if ( db_gain== -2 )		return PMU_IHFGAIN_2DB_N;
	if ( db_gain== -1 )		return PMU_IHFGAIN_1DB_N;
	if ( db_gain== 0 )		return PMU_IHFGAIN_0DB;
	if ( db_gain== 1 )		return PMU_IHFGAIN_1DB_P;
	if ( db_gain== 2 )		return PMU_IHFGAIN_2DB_P;
	if ( db_gain== 3 )		return PMU_IHFGAIN_3DB_P;
	if ( db_gain== 4 )		return PMU_IHFGAIN_4DB_P;

	return PMU_IHFGAIN_0DB;
}
#endif

#endif

#endif //UNDER_LINUX


#ifdef UNDER_LINUX

void setExternalPreAmpGain(Int16 gain)
{
    Int16 audioMode;
    audioMode = AUDDRV_GetAudioMode();

    if(audioMode >= AUDIO_MODE_NUMBER)
        audioMode -= AUDIO_MODE_NUMBER;

    if(audioMode == AUDIO_MODE_HANDSET || 
       audioMode == AUDIO_MODE_SPEAKERPHONE)
    {
        //max8986_set_input_preamp_gain(MAX8986_INPUTB, gain);
        max8986_set_ihf_preamp_gain(gain);
    }
    else if (audioMode == AUDIO_MODE_HEADSET)
    {
        //max8986_set_input_preamp_gain(MAX8986_INPUTA, gain);
        max8986_set_hs_preamp_gain(gain);
    }
}

void setExternalAmpGain(Int16 gain)
{
    Int16 audioMode;
    audioMode = AUDDRV_GetAudioMode();

    if(audioMode >= AUDIO_MODE_NUMBER)
        audioMode -= AUDIO_MODE_NUMBER;

    if(audioMode == AUDIO_MODE_HANDSET || 
       audioMode == AUDIO_MODE_SPEAKERPHONE)
    {
        max8986_audio_hs_ihf_set_gain(gain);
    }
    else if (audioMode == AUDIO_MODE_HEADSET)
    {
        max8986_audio_hs_set_gain(AUDIO_HS_BOTH, gain);
    }
}


void powerOnExternalAmp( AUDCTRL_SPEAKER_t speaker, ExtSpkrUsage_en_t usage_flag, Boolean use )
{
//check for current baseband_use_speaker: OR of voice_spkr, audio_spkr, poly_speaker, and second_speaker
//
//ext_use_speaker could be external FM radio, etc.
//baseband and ext not use amp, can power it off.
// PMU driver needs to know AUDIO_CHNL_HEADPHONE type, so call it from here.
//AUDCTRL_SPEAKER_t should be moved to public and let PMU driver includes it.
//and rename it AUD_SPEAKER_t

#if !defined(NO_PMU)

	static Boolean telephonyUseHS = FALSE;
	static Boolean voiceUseHS = FALSE;
	static Boolean audioUseHS = FALSE;
	static Boolean polyUseHS = FALSE;

	static Boolean telephonyUseIHF = FALSE;
	static Boolean voiceUseIHF = FALSE;
	static Boolean audioUseIHF = FALSE;
	static Boolean polyUseIHF = FALSE;

	static Boolean IHF_IsOn = FALSE;
	static Boolean HS_IsOn = FALSE;

	Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp, speaker = %d, IHF_IsOn= %d, HS_IsOn = %d, Boolean_Use=%d\n", speaker, IHF_IsOn, HS_IsOn, use);

	switch(speaker)
	{
		case AUDCTRL_SPK_HEADSET:
		case AUDCTRL_SPK_TTY:
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseHS = use;
					if(use)
					{
						telephonyUseIHF = FALSE; //only one output channel for voice call
					}
					break;

				case VoiceUseExtSpkr:
					voiceUseHS = use;
					break;

				case AudioUseExtSpkr:
					audioUseHS = use;
					break;

				case PolyUseExtSpkr:
					polyUseHS = use;
					break;

				default:
					break;
			}
			break;

		case AUDCTRL_SPK_LOUDSPK:
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseIHF = use;
					if(use)
					{
						telephonyUseHS = FALSE; //only one output channel for voice call
					}
					break;

				case VoiceUseExtSpkr:
					voiceUseIHF = use;
					break;

				case AudioUseExtSpkr:
					audioUseIHF = use;
					break;

				case PolyUseExtSpkr:
					polyUseIHF = use;
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	if ((telephonyUseHS==FALSE) && (voiceUseHS==FALSE) && (audioUseHS==FALSE) && (polyUseHS==FALSE))
	{
		if ( HS_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu HS amp\n");
			//power off handset            

#ifdef PMU_BCM59038
            bcm59038_audio_hs_powerUp(FALSE);
#elif PMU_MAX8986
            max8986_audio_hs_poweron(FALSE);
#endif

		}
		HS_IsOn = FALSE;
	}
	else
	{
		int i;
		int hs_gain;
		int hs_path;
		int preamp_gain;
		AudioApp_t app = AUDDRV_GetAudioApp();
		AudioMode_t mode = AUDDRV_GetAudioMode();
		if ( mode >= AUDIO_MODE_NUMBER )
			mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);
#ifdef PMU_BCM59038
		hs_path = BCM59038_AUDIO_HS_BOTH;
#elif PMU_MAX8986        
		hs_path =  AUDIO_HS_BOTH;
#endif
		//i = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga;
		//i = AUDIOMODE_PARM_ACCESSOR(AUDDRV_GetAudioMode()).ext_speaker_pga;
		//i= (AUDIO_GetParmAccessPtr()+app*AUDIO_MODE_NUMBER)[mode].ext_speaker_pga;
                //if(lp_voip_start  == 1) app = 5;/* 20110715 for loopback check */
		
		i = AUDIOMODE_PARM_ACCESSOR(app,AUDIO_MODE_HEADSET).ext_speaker_pga;
		preamp_gain = AUDIOMODE_PARM_ACCESSOR(app,AUDIO_MODE_HEADSET).ext_speaker_preamp_pga;

		//hs_gain = map2pmu_hs_gain( i );
		hs_gain = i;

		// 20110421 for ringtone earphone gain
		Log_DebugPrintf(LOGID_AUDIO,"power ON pmu HS amp, AUDDRV_GetAudioApp app %d\n",app);
        	if((speaker == AUDCTRL_SPK_LOUDSPK) && (IHF_IsOn == FALSE) && HS_IsOn && polyUseHS && polyUseIHF)
        	{
	        	hs_gain = AUDIOMODE_PARM_ACCESSOR(AUDIO_APP_MUSIC,AUDIO_MODE_TTY).ext_speaker_pga;
	        	Log_DebugPrintf(LOGID_AUDIO,"power ON pmu HS amp, Ringtone case Earphone gain %d\n",hs_gain);
        	}
        	
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (HS on), telephonyUseHS = %d, voiceUseHS= %d, audioUseHS= %d, polyUseHS = %d\n", telephonyUseHS, voiceUseHS, audioUseHS, polyUseHS);

#ifdef PMU_BCM59038
		bcm59038_audio_hs_setGain(hs_path, hs_gain);
#elif PMU_MAX8986
            max8986_audio_hs_set_gain(hs_path, hs_gain);
            //max8986_set_input_preamp_gain(MAX8986_INPUTA, preamp_gain);
            max8986_set_hs_preamp_gain(preamp_gain);
#endif
		if ( HS_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu HS amp, gain %d\n",hs_gain);
			//power on handset           
#ifdef PMU_BCM59038
			bcm59038_audio_hs_powerUp(TRUE);
#elif PMU_MAX8986
			max8986_audio_hs_poweron(TRUE);
#endif

		}
		
		HS_IsOn = TRUE;
	}

	if ((telephonyUseIHF==FALSE) && (voiceUseIHF==FALSE) && (audioUseIHF==FALSE) && (polyUseIHF==FALSE))
	{
		if ( IHF_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu IHF amp\n");
			//power down IHF
#ifdef PMU_BCM59038
			bcm59038_audio_ihf_powerDown();
#elif PMU_MAX8986
			max8986_audio_hs_ihf_poweroff();
#endif

		}
		IHF_IsOn = FALSE;
	}
	else
	{
		int i;
		int ihf_gain;
		int preamp_gain;
		AudioApp_t app = AUDDRV_GetAudioApp();
		AudioMode_t mode = AUDDRV_GetAudioMode();
		//SysAudioParm_t** sysparmptr;


		if ( mode >= AUDIO_MODE_NUMBER )
			mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);
		//i = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga;
		//i= AUDIOMODE_PARM_ACCESSOR(AUDDRV_GetAudioMode()).ext_speaker_pga;

		//sysparmptr = (SysAudioParm_t**)AUDIO_GetParmAccessPtr();
                //if(lp_voip_start  == 1) app = 5;/* 20110715 for loopback check */
		i = AUDIOMODE_PARM_ACCESSOR(app,AUDIO_MODE_SPEAKERPHONE).ext_speaker_pga;
		preamp_gain = AUDIOMODE_PARM_ACCESSOR(app,AUDIO_MODE_SPEAKERPHONE).ext_speaker_preamp_pga;

		//i = (AUDIO_GetParmAccessPtr()+app*AUDIO_MODE_NUMBER)[mode].ext_speaker_pga;
		//ihf_gain = map2pmu_ihf_gain( i );
		ihf_gain = i;
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (IHF on), app = %d, mode= %d, ihf gain= %d preamp_gain=%d \n", app, mode, ihf_gain,preamp_gain);
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (IHF on), telephonyUseIHF = %d, voiceUseIHF= %d, audioUseIHF= %d, polyUseIHF = %d \n", telephonyUseIHF, voiceUseIHF, audioUseIHF, polyUseIHF);

#ifdef PMU_BCM59038
		bcm59038_audio_ihf_setGain(ihf_gain);
#elif PMU_MAX8986
            max8986_audio_hs_ihf_set_gain(ihf_gain);
            //max8986_set_input_preamp_gain(MAX8986_INPUTB, preamp_gain);
             max8986_set_ihf_preamp_gain(preamp_gain);
#endif
		if ( IHF_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu IHF amp, gain %d\n", ihf_gain);
#ifdef PMU_BCM59038
			bcm59038_audio_ihf_powerUp();
#elif PMU_MAX8986
			max8986_audio_hs_ihf_poweron();
#endif

		}
		
		IHF_IsOn = TRUE;
	}
#endif
}


#else
//============================================================================
//
// Function Name: powerOnExternalAmp
//
// Description:   call external amplifier driver
//
//============================================================================
void powerOnExternalAmp( AUDCTRL_SPEAKER_t speaker, ExtSpkrUsage_en_t usage_flag, Boolean use )
{
//check for current baseband_use_speaker: OR of voice_spkr, audio_spkr, poly_speaker, and second_speaker
//
//ext_use_speaker could be external FM radio, etc.
//baseband and ext not use amp, can power it off.
// PMU driver needs to know AUDIO_CHNL_HEADPHONE type, so call it from here.
//AUDCTRL_SPEAKER_t should be moved to public and let PMU driver includes it.
//and rename it AUD_SPEAKER_t

#if !defined(NO_PMU)
#if defined(EXTERNAL_AMP_CONTROL)

	static Boolean telephonyUseHS = FALSE;
	static Boolean voiceUseHS = FALSE;
	static Boolean audioUseHS = FALSE;
	static Boolean polyUseHS = FALSE;

	static Boolean telephonyUseIHF = FALSE;
	static Boolean voiceUseIHF = FALSE;
	static Boolean audioUseIHF = FALSE;
	static Boolean polyUseIHF = FALSE;

	static Boolean IHF_IsOn = FALSE;
	static Boolean HS_IsOn = FALSE;

	Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp, speaker = %d, IHF_IsOn= %d, HS_IsOn = %d, Boolean_Use=%d\n", speaker, IHF_IsOn, HS_IsOn, use);

	switch(speaker)
	{
		case AUDCTRL_SPK_HEADSET:
		case AUDCTRL_SPK_TTY:
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseHS = use;
					if(use)
					{
						telephonyUseIHF = FALSE; //only one output channel for voice call
					}
					break;

				case VoiceUseExtSpkr:
					voiceUseHS = use;
					break;

				case AudioUseExtSpkr:
					audioUseHS = use;
					break;

				case PolyUseExtSpkr:
					polyUseHS = use;
					break;

				default:
					break;
			}
			break;

		case AUDCTRL_SPK_LOUDSPK:
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseIHF = use;
					if(use)
					{
						telephonyUseHS = FALSE; //only one output channel for voice call
					}
					break;

				case VoiceUseExtSpkr:
					voiceUseIHF = use;
					break;

				case AudioUseExtSpkr:
					audioUseIHF = use;
					break;

				case PolyUseExtSpkr:
					polyUseIHF = use;
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	if ((telephonyUseHS==FALSE) && (voiceUseHS==FALSE) && (audioUseHS==FALSE) && (polyUseHS==FALSE))
	{
		if ( HS_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu HS amp\n");
			pmu_hs.hs_power = FALSE;
			HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_HS_POWER, &pmu_hs, NULL);
		}
		HS_IsOn = FALSE;
	}
	else
	{
		pmu_hs.hs_power = TRUE;
		pmu_hs.hs_path = PMU_AUDIO_HS_BOTH;
		pmu_hs.hs_gain = (PMU_HS_Gain_t) AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga;
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (HS on), telephonyUseHS = %d, voiceUseHS= %d, audioUseHS= %d, polyUseHS = %d\n", telephonyUseHS, voiceUseHS, audioUseHS, polyUseHS);

		if ( HS_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu HS amp, gain %d\n", pmu_hs.hs_gain);
			HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_HS_POWER, &pmu_hs, NULL);
		}
		HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_HS_SET_GAIN, &pmu_hs, NULL);
		HS_IsOn = TRUE;
	}

	if ((telephonyUseIHF==FALSE) && (voiceUseIHF==FALSE) && (audioUseIHF==FALSE) && (polyUseIHF==FALSE))
	{
		if ( IHF_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu IHF amp\n");
			HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_IHF_POWER_DOWN, &pmu_ihf, NULL);
		}
		IHF_IsOn = FALSE;
	}
	else
	{
		pmu_ihf.ihf_gain = (PMU_IHF_Gain_t) AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga;
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (IHF on), telephonyUseIHF = %d, voiceUseIHF= %d, audioUseIHF= %d, polyUseIHF = %d\n", telephonyUseIHF, voiceUseIHF, audioUseIHF, polyUseIHF);

		if ( IHF_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu IHF amp, gain %d\n", pmu_ihf.ihf_gain);
			HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_IHF_POWER_UP, &pmu_ihf, NULL);
		}
		HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_IHF_SET_GAIN, &pmu_ihf, NULL);
		IHF_IsOn = TRUE;
	}

#endif     // #if defined(EXTERNAL_AMP_CONTROL)
#endif
}


#endif


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
// Function Name: AUDCTRL_Init
//
// Description:   Init function
//
//============================================================================
void AUDCTRL_Init (void)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrVolume
//
// Description:   Set dl volume of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrVolume(
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_SPEAKER_t		speaker,
				UInt32					volume,
				AUDIO_GAIN_FORMAT_t		gain_format
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicMute
//
// Description:   mute/unmute ul of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicMute(
				AUDIO_HW_ID_t			ulSrc,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_EnablePlay
//
// Description:   enable a playback path
//
//============================================================================

void AUDCTRL_EnablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_DisablePlay
//
// Description:   disable a playback path
//
//============================================================================
void AUDCTRL_DisablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetPlayVolume
//
// Description:   set volume of a playback path
//
//============================================================================
void AUDCTRL_SetPlayVolume(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_GAIN_FORMAT_t     gainF,
				UInt32					vol_left,
				UInt32					vol_right
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetPlayMute
//
// Description:   mute/unmute a playback path
//
//============================================================================
void AUDCTRL_SetPlayMute(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
}


//============================================================================
//
// Function Name: AUDCTRL_AddPlaySpk
//
// Description:   add a speaker to a playback path
//
//============================================================================
void AUDCTRL_AddPlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_RemovePlaySpk
//
// Description:   remove a speaker to a playback path
//
//============================================================================
void AUDCTRL_RemovePlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
}


//============================================================================
//
// Function Name: AUDCTRL_EnableRecord
//
// Description:   enable a record path
//
//============================================================================
void AUDCTRL_EnableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_DisableRecord
//
// Description:   disable a record path
//
//============================================================================
void AUDCTRL_DisableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_EnableTap
//
// Description:   enable a tap path
//
//============================================================================
void AUDCTRL_EnableTap(
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_DisableTap
//
// Description:   disable a tap path
//
//============================================================================
void AUDCTRL_DisableTap( AUDIO_HW_ID_t	tap)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetTapGain
//
// Description:   set gain of a tap path
//
//============================================================================
void AUDCTRL_SetTapGain(
				AUDIO_HW_ID_t			tap,
				UInt32					gain
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordGain
//
// Description:   set gain of a record path
//
//============================================================================
void AUDCTRL_SetRecordGain(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				UInt32					gainL,
				UInt32					gainR
				)
{
}


//============================================================================
//
// Function Name: AUDCTRL_SetRecordMute
//
// Description:   mute/unmute a record path
//
//============================================================================
void AUDCTRL_SetRecordMute(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_AddRecordMic
//
// Description:   add a microphone to a record path
//
//============================================================================
void AUDCTRL_AddRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_RemoveRecordMic
//
// Description:   remove a microphone from a record path
//
//============================================================================
void AUDCTRL_RemoveRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				)
{
	// Nothing to do.
}


void AUDCTRL_SetAudioLoopback( 
							Boolean					enable_lpbk,
							AUDCTRL_MICROPHONE_t	mic,
							AUDCTRL_SPEAKER_t		speaker
							)
{
}

void AUDCTRL_SetEQ( 
				AUDIO_HW_ID_t	audioPath,
				AudioEqualizer_en_t  equType
				)
{
}

//=============================================================================
// Private function definitions
//=============================================================================

#endif //WIN32
