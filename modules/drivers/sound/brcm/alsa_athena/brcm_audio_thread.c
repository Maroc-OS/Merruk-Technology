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


/*
*
*****************************************************************************
*
*  brcm_audio_thread.c
*
*  PURPOSE:
*
*     Serialize audio  control operation
*	Eliminate waits in audio control because of atomic operation requirement from ALSA
*
*  NOTES:
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/wait.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "ossemaphore.h"

#include "brcm_alsa.h"

#include "brcm_audio_thread.h"

// Include BRCM AAUD driver API header files
#include "audio_controller.h"
#include "audio_ddriver.h"

#include "auddrv_def.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h" 

#include "brcm_audio_devices.h"
#include "brcm_audio_policy.h"

#include "brcm_rdb_sysmap.h"


/* ---- Functions ecported ---------------------------------------------------- */
//int LaunchAudioHalThread(void);
//int TerminateAudioHalThread(void);
//Result_t HAL_AUDIO_Ctrl(	HAL_AUDIO_ACTION_en_t action_code,	void *arg_param,	void *callback	);

long GetCtrlValue(int ctrlIndex);

static UInt8 telephony_started = 0;
static UInt8 telephony_codecId = 6;// Set default to AMR-NB
static int telephony_stream_number = 0;
static Boolean musicduringcall = FALSE;
/* ---- Data structure  ------------------------------------------------- */

//++++++++++++++++++++++++++++++++++++++++++++++
//	Wrap up HAL_AUDIO_Ctrl parameters
//	The record is passed to worker thread via KFIFO
//
//----------------------------------------
typedef	struct	_TMsgBrcmAudioCtrl
{
	BRCM_AUDIO_ACTION_en_t			action_code;
	BRCM_AUDIO_Control_Params_un_t		param;
	void								*pCallBack;
    int                                 block;

}TMsgAudioCtrl, *PTMsgAudioCtrl;

extern int playback_prev_time;


//++++++++++++++++++++++++++++++++++++++++++++++
//	The thread private data structure
//
//----------------------------------------
typedef	struct	_TAudioHalThreadData
{
	struct kfifo	m_pkfifo;	//KFIFO to pass control parameters from audio HAL caller  to worker thread
	spinlock_t	m_lock;		//spin lock to protect KFIFO access so that audio HAL can accept concurrent caller
	struct work_struct mwork; //worker thread data structure
	struct workqueue_struct *pWorkqueue_AudioControl;
    Semaphore_t                     action_complete;
    struct kfifo	m_pkfifo_out;	
    spinlock_t	m_lock_out;

}TAudioControlThreadData, *PTAudioControlThreadData;


static TAudioControlThreadData	sgThreadData;
#define	KFIFO_SIZE		(9*sizeof(TMsgAudioCtrl))

extern TIDChanOfPlaybackDev sgTableIDChannelOfDev[];

extern TIDChanOfCaptureDev	sgTableIDChannelOfCaptDev[];

extern TIDChanOfVoiceCallDev	sgTableIDChannelOfVoiceCallDev[];

void AUDIO_Ctrl_Process(
	BRCM_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback,
    int block
	);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AudioCtrlWorkThread
//	Worker thread, it query KFIFO for operation message and call HAL_AudioProcess.
//----------------------------------------------------------------
static void AudioCtrlWorkThread(struct work_struct *work)
{
	TMsgAudioCtrl	msgAudioCtrl;
	unsigned int len = 0;

    set_user_nice(current, -16);


	while(1)
	{
		//get operation code from fifo
		len = kfifo_out_locked(&sgThreadData.m_pkfifo, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl), &sgThreadData.m_lock);
		if( (len != sizeof(TMsgAudioCtrl)) && (len!=0) )
			DEBUG("Error AUDIO_Ctrl len=%d expected %d in=%d, out=%d\n", len, sizeof(TMsgAudioCtrl), sgThreadData.m_pkfifo.in, sgThreadData.m_pkfifo.out);
		if(len == 0) //FIFO empty sleep
			return;
		
		//process the operation
		AUDIO_Ctrl_Process(msgAudioCtrl.action_code, &msgAudioCtrl.param, msgAudioCtrl.pCallBack,msgAudioCtrl.block);
	}

	return;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//LaunchAudioHalThread
//	Create Worker thread.
//----------------------------------------------------------------
int LaunchAudioCtrlThread(void)
{
	sgThreadData.m_lock =  SPIN_LOCK_UNLOCKED;
	kfifo_alloc(&sgThreadData.m_pkfifo, KFIFO_SIZE, GFP_KERNEL);
	DEBUG("LaunchAudioCtrlThread KFIFO_SIZE= %d actual =%d\n", KFIFO_SIZE,sgThreadData.m_pkfifo.size);
    kfifo_alloc(&sgThreadData.m_pkfifo_out, KFIFO_SIZE, GFP_KERNEL);
    
	DEBUG("LaunchAudioCtrlThread KFIFO_SIZE= %d actual =%d\n", KFIFO_SIZE,sgThreadData.m_pkfifo_out.size);
	INIT_WORK(&sgThreadData.mwork, AudioCtrlWorkThread);
	
	sgThreadData.pWorkqueue_AudioControl = create_workqueue("AudioCtrlWq");
	if(!sgThreadData.pWorkqueue_AudioControl)
		DEBUG("\n Error : Can not create work queue:AudioCtrlWq\n");

    
    //create a semaphor for blocking
    sgThreadData.action_complete = OSSEMAPHORE_Create(0,0);

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//LaunchAudioHalThread
//	Clean up: free KFIFO
//----------------------------------------------------------------
int TerminateAudioHalThread(void)
{
	
	if(sgThreadData.pWorkqueue_AudioControl)
	{
		flush_workqueue(sgThreadData.pWorkqueue_AudioControl);
		destroy_workqueue(sgThreadData.pWorkqueue_AudioControl);
	}
	kfifo_free(&sgThreadData.m_pkfifo);
    kfifo_free(&sgThreadData.m_pkfifo_out);
	return 0;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//HAL_AUDIO_Ctrl
//	Client call this function to execute audio HAL functions.
//	This function for the message to worker thread to do actual work
//----------------------------------------------------------------
Result_t AUDIO_Ctrl_Trigger(
	BRCM_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback,
    int block
	)
{
	TMsgAudioCtrl	msgAudioCtrl;
	Result_t status = RESULT_OK;
	unsigned int	len;
    OSStatus_t  osStatus;

	{
		//BRCM_AUDIO_Control_Params_un_t *paudioControlParam = (BRCM_AUDIO_Control_Params_un_t *)arg_param;
		DEBUG("AudioHalThread action=%d\r\n", action_code);
	}


	msgAudioCtrl.action_code = action_code;
	if(arg_param)
		memcpy(&msgAudioCtrl.param, arg_param, sizeof(BRCM_AUDIO_Control_Params_un_t));
	else
		memset(&msgAudioCtrl.param, 0, sizeof(BRCM_AUDIO_Control_Params_un_t));
	msgAudioCtrl.pCallBack = callback;
    msgAudioCtrl.block = block;

	len = kfifo_in_locked(&sgThreadData.m_pkfifo, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl), &sgThreadData.m_lock);
	if(len != sizeof(TMsgAudioCtrl))
	{
		DEBUG("Error AUDIO_Ctrl_Trigger len=%d expected %d \n", len, sizeof(TMsgAudioCtrl));
		return RESULT_ERROR;
	}

	queue_work(sgThreadData.pWorkqueue_AudioControl, &sgThreadData.mwork);

    if(block)
    {
        // wait for 10sec
        osStatus = OSSEMAPHORE_Obtain(sgThreadData.action_complete,1280);
        if(osStatus != OSSTATUS_SUCCESS)
        {
            DEBUG("AUDIO_Ctrl_Trigger Timeout=%d\r\n",osStatus);
        }

        while(1)
	    {

            //wait for output from output fifo
		    len = kfifo_out_locked(&sgThreadData.m_pkfifo_out, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl), &sgThreadData.m_lock_out);
		    if( (len != sizeof(TMsgAudioCtrl)) && (len!=0) )
			    DEBUG("Error AUDIO_Ctrl_Trigger len=%d expected %d in=%d, out=%d\n", len, sizeof(TMsgAudioCtrl), sgThreadData.m_pkfifo_out.in, sgThreadData.m_pkfifo_out.out);
		    if(len == 0) //FIFO empty sleep
			    return status;
            if(arg_param)
		        memcpy(arg_param,&msgAudioCtrl.param,  sizeof(BRCM_AUDIO_Control_Params_un_t));
	        else
		        memset(arg_param, 0, sizeof(BRCM_AUDIO_Control_Params_un_t));
	    }
    }
	
	return status;
}
static int playback_triggered = 0;
static int extra_speaker = 0;


void AUDIO_Ctrl_Process(
	BRCM_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback,
    int  block
	)
{
   	TMsgAudioCtrl	msgAudioCtrl;
	unsigned int	len;
	AUDIO_DRIVER_TYPE_t drv_type;
    UInt32 app_profile = 0 ;
	static AudioApp_t prev_app_profile = 0;
	static AudioMode_t prev_mode_profile = 0;
	static bool mode_restore = FALSE;

	if(arg_param == NULL)
	{
		DEBUG("AUDIO_Ctrl_Process: arg_param is NULL \n");
		return;
	}
    switch (action_code)
    {
        case ACTION_AUD_StartPlay:
        {
            BRCM_AUDIO_Param_Start_t* param_start = (BRCM_AUDIO_Param_Start_t*) arg_param;
            
		//20110905 	//prev_app_profile = AUDDRV_GetAudioApp();
            app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_MUSIC);

            AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_START,NULL);

            AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_GET_DRV_TYPE,(void*)&drv_type);

            if (AUDIO_Policy_GetState() == BRCM_STATE_INCALL)
            {
                DEBUG("Play Music During Voice call \n");
                musicduringcall = TRUE;
                if ( sgTableIDChannelOfDev[param_start->substream_number].speaker == AUDCTRL_SPK_HEADSET ||
                     sgTableIDChannelOfDev[param_start->substream_number].speaker == AUDCTRL_SPK_BTM )
                {
                    DEBUG("Play Music During Voice call to HEADSET or BTM\n");
                    if ( app_profile == AUDIO_APP_VOICE_CALL )
                    {
                        DEBUG(" Play music during Voice call with BT Tap at 8k \r\n");
                        AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_BTM, 8000);
                    }
                    else
                    {
                        DEBUG(" Play music during Voice call with BT Tap at 16k \r\n");
                        if ( sgTableIDChannelOfDev[param_start->substream_number].speaker == AUDCTRL_SPK_BTM )
                        {
                            AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_BTM, 8000);
                        }
                        else
                        {
                            AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_BTM, 16000);
                        }
                    }

                    AUDCTRL_EnablePlay(AUDIO_HW_NONE,
                                       sgTableIDChannelOfDev[param_start->substream_number].hw_id,
                                       AUDIO_HW_TAP_VOICE,
                                       AUDCTRL_SPK_BTM,
                                       param_start->channels,
                                       param_start->rate
                                      );
                }
                else
                {
                    DEBUG("Play Music During Voice call to HANDSET or LOUDSPEAKER\n");
                    if ( app_profile == AUDIO_APP_VOICE_CALL )
                    {
                        DEBUG(" Play music during Voice call with BT Tap at 8k \r\n");
                        AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_HEADSET, 8000);
                    }
                    else
                    {
                        DEBUG(" Play music during Voice call with BT Tap at 16k \r\n");
                        AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_HEADSET, 16000);
                    }

                    AUDCTRL_EnablePlay(AUDIO_HW_NONE,
                                       sgTableIDChannelOfDev[param_start->substream_number].hw_id,
                                       AUDIO_HW_TAP_VOICE,
                                       AUDCTRL_SPK_HEADSET,
                                       param_start->channels,
                                       param_start->rate
                                      );
                }
            }
            else
            {
               	AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfDev[param_start->substream_number].speaker,app_profile);
            
                if( sgTableIDChannelOfDev[param_start->substream_number].speaker == AUDCTRL_SPK_BTM )
                {
                    AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, sgTableIDChannelOfDev[param_start->substream_number].speaker, 8000);

                    // Enable the playback the path
                    AUDCTRL_EnablePlay(AUDIO_HW_NONE,
                                       sgTableIDChannelOfDev[param_start->substream_number].hw_id,
                                       AUDIO_HW_TAP_VOICE,
                                       sgTableIDChannelOfDev[param_start->substream_number].speaker,
                                       param_start->channels,
                                       param_start->rate
				      );
                }
                else
                {
                    // Enable the playback the path
                    AUDCTRL_EnablePlay(AUDIO_HW_NONE,
                                       sgTableIDChannelOfDev[param_start->substream_number].hw_id,
                                       AUDIO_HW_NONE,
                                       sgTableIDChannelOfDev[param_start->substream_number].speaker,
                                       param_start->channels,
                                       param_start->rate
                                      );

                    //if (param_start->substream_number == 5)  // Ring case
                    if(extra_speaker)
                    {
                        DEBUG("Play to speaker as well \n");
                        AUDCTRL_AddPlaySpk(sgTableIDChannelOfDev[param_start->substream_number].hw_id, //		AUDIO_HW_ID_t			sink,
                                           AUDCTRL_SPK_LOUDSPK                 //          AUDCTRL_SPEAKER_t		spk
                                          );
                    }
                }
            }

            // set the slopgain register to max value
            AUDCTRL_SetPlayVolume(sgTableIDChannelOfDev[param_start->substream_number].hw_id,
                                  sgTableIDChannelOfDev[param_start->substream_number].speaker,
                                  AUDIO_GAIN_FORMAT_VOL_LEVEL,
                                  AUDIO_VOLUME_MAX,
                                  AUDIO_VOLUME_MAX
                                 );

            // start DMA now
            AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_RESUME,NULL);

            playback_prev_time = 0;
            playback_triggered = 1;

        }
        break;
        case ACTION_AUD_StopPlay:
        {
            BRCM_AUDIO_Param_Stop_t* param_stop = (BRCM_AUDIO_Param_Stop_t*) arg_param;

            // stop DMA first
            AUDIO_DRIVER_Ctrl(param_stop->drv_handle,AUDIO_DRIVER_PAUSE,NULL);

            AUDIO_DRIVER_Ctrl(param_stop->drv_handle,AUDIO_DRIVER_GET_DRV_TYPE,(void*)&drv_type);
            //if (param_stop->substream_number == 5)  // Ring case
            if(extra_speaker)
            {
		            DEBUG(" REMOVING  LOUDSPK  \n");				  
                    AUDCTRL_RemovePlaySpk(sgTableIDChannelOfDev[param_stop->substream_number].hw_id, //		AUDIO_HW_ID_t			sink,
									  AUDCTRL_SPK_LOUDSPK				  //		  AUDCTRL_SPEAKER_t 	spk
                    );
            }

		//20110905	//AUDCTRL_SaveAudioModeFlag( AUDDRV_GetAudioMode(), prev_app_profile );

            //disable the playback path
            if (AUDIO_Policy_GetState() == BRCM_STATE_INCALL)
            {
                musicduringcall = FALSE;
                if ( sgTableIDChannelOfDev[param_stop->substream_number].speaker == AUDCTRL_SPK_HEADSET ||
                     sgTableIDChannelOfDev[param_stop->substream_number].speaker == AUDCTRL_SPK_BTM )
                {
                    AUDCTRL_DisablePlay(AUDIO_HW_NONE,
                                        sgTableIDChannelOfDev[param_stop->substream_number].hw_id,
                                        AUDCTRL_SPK_BTM
                                       );
                }
                else
                {
                    AUDCTRL_DisablePlay(AUDIO_HW_NONE,
                                        sgTableIDChannelOfDev[param_stop->substream_number].hw_id,
                                        AUDCTRL_SPK_HEADSET
                                       );
                }
                AUDCTRL_DisableTap (AUDIO_HW_TAP_VOICE);
            }
            else
            {
                if( sgTableIDChannelOfDev[param_stop->substream_number].speaker == AUDCTRL_SPK_BTM )
                {
                    AUDCTRL_DisablePlay(AUDIO_HW_NONE,
                                        sgTableIDChannelOfDev[param_stop->substream_number].hw_id,
                                        sgTableIDChannelOfDev[param_stop->substream_number].speaker
                                       );

                    AUDCTRL_DisableTap (AUDIO_HW_TAP_VOICE);
                }
                else
                {
                    AUDCTRL_DisablePlay(AUDIO_HW_NONE,
                                        sgTableIDChannelOfDev[param_stop->substream_number].hw_id,
                                        sgTableIDChannelOfDev[param_stop->substream_number].speaker
                                       );
                }
            }
            AUDIO_DRIVER_Ctrl(param_stop->drv_handle,AUDIO_DRIVER_STOP,NULL);

            playback_prev_time = 0;
            playback_triggered = 0;

            

        }
        break;
        case ACTION_AUD_PausePlay:
        {
            BRCM_AUDIO_Param_Pause_t* param_pause = (BRCM_AUDIO_Param_Pause_t*) arg_param;

            AUDIO_DRIVER_Ctrl(param_pause->drv_handle,AUDIO_DRIVER_GET_DRV_TYPE,(void*)&drv_type);

            //disable the playback path
             AUDCTRL_DisablePlay(AUDIO_HW_NONE,
                        sgTableIDChannelOfDev[param_pause->substream_number].hw_id,
                        sgTableIDChannelOfDev[param_pause->substream_number].speaker
                    );

            if( sgTableIDChannelOfDev[param_pause->substream_number].speaker == AUDCTRL_SPK_BTM )
            {
                AUDCTRL_DisableTap (AUDIO_HW_TAP_VOICE);
            }
            AUDIO_DRIVER_Ctrl(param_pause->drv_handle,AUDIO_DRIVER_PAUSE,NULL);
        }
        break;

        case ACTION_AUD_ResumePlay:
        {
            BRCM_AUDIO_Param_Resume_t* param_resume = (BRCM_AUDIO_Param_Resume_t*) arg_param;

            AUDIO_DRIVER_Ctrl(param_resume->drv_handle,AUDIO_DRIVER_GET_DRV_TYPE,(void*)&drv_type);

            AUDIO_DRIVER_Ctrl(param_resume->drv_handle,AUDIO_DRIVER_RESUME,NULL);

            if( sgTableIDChannelOfDev[param_resume->substream_number].speaker == AUDCTRL_SPK_BTM )
            {
                AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, sgTableIDChannelOfDev[param_resume->substream_number].speaker, param_resume->rate);

                // Enable the playback the path
                AUDCTRL_EnablePlay(AUDIO_HW_NONE,
                                   sgTableIDChannelOfDev[param_resume->substream_number].hw_id,
                                   AUDIO_HW_TAP_VOICE,
                                   sgTableIDChannelOfDev[param_resume->substream_number].speaker,
				                   param_resume->channels,
                                   param_resume->rate
				    );
            }
            else
            {
                // Enable the playback the path
                AUDCTRL_EnablePlay(AUDIO_HW_NONE,
                                   sgTableIDChannelOfDev[param_resume->substream_number].hw_id,
                                   AUDIO_HW_NONE,
                                   sgTableIDChannelOfDev[param_resume->substream_number].speaker,
				                   param_resume->channels,
                                   param_resume->rate
				    );
            }
        }
        break;
        case ACTION_AUD_StartRecord:
        {
            AudioApp_t app_prof = AUDIO_APP_RECORDING;
            AudioMode_t  new_mode = AUDIO_MODE_SPEAKERPHONE;
            AudioMode_t cur_mode;

            BRCM_AUDIO_Param_Start_t* param_start = (BRCM_AUDIO_Param_Start_t*) arg_param;
            DEBUG("ACTION_AUD_StartRecord : param_start->substream_number -  %d \n",param_start->substream_number);

			prev_app_profile = AUDDRV_GetAudioApp();
            cur_mode = AUDCTRL_GetAudioMode();

            if ( cur_mode >= AUDIO_MODE_NUMBER )
                cur_mode = (AudioMode_t) (cur_mode - AUDIO_MODE_NUMBER);

            if (param_start->substream_number == 6 || param_start->substream_number == 7) // record request with Google voice search profile
			{
                app_prof = AUDIO_APP_RECORDING_GVS;
				mode_restore = TRUE;
				prev_mode_profile = cur_mode;
				DEBUG("ACTION_AUD_StartRecord : [%d, %d]", prev_app_profile, prev_mode_profile);
			}

            if (param_start->substream_number == 9 || param_start->substream_number == 10) // record request with voip profile
            {
				app_prof = AUDIO_APP_VOIP;
                new_mode = cur_mode; // use current mode based on earpiece or speaker
            }
            
            if (param_start->substream_number == 11 || param_start->substream_number == 12) // record request with voip incomm profile
            {
				app_prof = AUDIO_APP_VOIP_INCOMM;
                new_mode = cur_mode; // use current mode based on earpiece or speaker
            }

            if (param_start->substream_number == 1 || param_start->substream_number == 7  || param_start->substream_number == 8 
                || param_start->substream_number == 10 || param_start->substream_number == 12) // record request with auxilary mic
                new_mode = AUDIO_MODE_HEADSET;

            app_profile = AUDIO_Policy_Get_Profile(app_prof);
            new_mode = AUDIO_Policy_Get_Mode(new_mode);
            AUDCTRL_SaveAudioModeFlag(new_mode,app_profile);

            AUDCTRL_EnableRecord(sgTableIDChannelOfCaptDev[param_start->substream_number].hw_id,
				                     AUDIO_HW_NONE,
                                     sgTableIDChannelOfCaptDev[param_start->substream_number].mic,
				                     param_start->channels,
                                     param_start->rate);

            AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_START,NULL);

             AUDIO_Policy_SetState(BRCM_STATE_RECORD);

        }
        break;
        case ACTION_AUD_StopRecord:
        {
            BRCM_AUDIO_Param_Stop_t* param_stop = (BRCM_AUDIO_Param_Stop_t*) arg_param;
               
            AUDIO_DRIVER_Ctrl(param_stop->drv_handle,AUDIO_DRIVER_STOP,NULL);

			if (mode_restore)
			{
				mode_restore = FALSE;
				AUDCTRL_SaveAudioModeFlag( prev_mode_profile, prev_app_profile );
				DEBUG("ACTION_AUD_StartRecord : [%d, %d]", prev_app_profile, prev_mode_profile);
			}
			else
			{
				AUDCTRL_SaveAudioModeFlag( AUDDRV_GetAudioMode(), prev_app_profile );
			}

            AUDCTRL_DisableRecord(sgTableIDChannelOfCaptDev[param_stop->substream_number].hw_id,
                                      AUDIO_HW_NONE,
                                      sgTableIDChannelOfCaptDev[param_stop->substream_number].mic);

            AUDIO_Policy_RestoreState();

        }
	break;
        case ACTION_AUD_OpenPlay:
        {
            BRCM_AUDIO_Param_Open_t* param_open = (BRCM_AUDIO_Param_Open_t*) arg_param;

            param_open->drv_handle = AUDIO_DRIVER_Open(sgTableIDChannelOfDev[param_open->substream_number].drv_type);

            DEBUG("param_open->drv_handle -  0x%x \n",param_open->drv_handle);

        }
        break;
        case ACTION_AUD_ClosePlay:
        {
            BRCM_AUDIO_Param_Close_t* param_close = (BRCM_AUDIO_Param_Close_t*) arg_param;

            DEBUG("param_close->drv_handle -  0x%x \n",param_close->drv_handle);

            AUDIO_DRIVER_Close(param_close->drv_handle);

        }
        break;

	case ACTION_AUD_OpenRecord:
        {
            BRCM_AUDIO_Param_Open_t* param_open = (BRCM_AUDIO_Param_Open_t*) arg_param;

            param_open->drv_handle = AUDIO_DRIVER_Open(sgTableIDChannelOfCaptDev[param_open->substream_number].drv_type);

            DEBUG("param_open->drv_handle -  0x%x \n",param_open->drv_handle);

        }
        break;

        case ACTION_AUD_CloseRecord:
        {
            BRCM_AUDIO_Param_Close_t* param_close = (BRCM_AUDIO_Param_Close_t*) arg_param;

            DEBUG("param_close->drv_handle -  0x%x \n",param_close->drv_handle);

            AUDIO_DRIVER_Close(param_close->drv_handle);

        }
        break;


        case ACTON_VOICECALL_START:
        {
            BRCM_VOICE_Param_Start_t* param_voice_start = (BRCM_VOICE_Param_Start_t*)arg_param;
            int vol_level;

            telephony_stream_number = param_voice_start->substream_number;

            if ( telephony_codecId == 10 )
            {
                DEBUG("Enable telephony WB Call \r\n");
                app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_VOICE_CALL_WB);
                AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfVoiceCallDev[telephony_stream_number-VOICE_CALL_SUB_DEVICE].speaker + AUDIO_MODE_NUMBER,app_profile);
            }
            else if ( telephony_codecId == 6 )
            {
                DEBUG("Enable telephony NB Call \r\n");
                app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_VOICE_CALL);
                AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfVoiceCallDev[telephony_stream_number-VOICE_CALL_SUB_DEVICE].speaker,app_profile);
            }
            else
            {
                DEBUG("Enable telephony Invalid Codec : Setting as NB \r\n");
                app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_VOICE_CALL);
                AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfVoiceCallDev[telephony_stream_number-VOICE_CALL_SUB_DEVICE].speaker,app_profile);
            }

            if ( (telephony_stream_number-VOICE_CALL_SUB_DEVICE) == 4 )
            {
		if ( telephony_codecId == 10 )
            	{
                	DEBUG("Enable telephony WB Call for BT NREC \r\n");
                	app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_VOICE_CALL_WB);
                	AUDCTRL_SaveAudioModeFlag(AUDCTRL_SPK_HANDSFREE, app_profile);
            	}
            	else if ( telephony_codecId == 6 )
            	{
                	DEBUG("Enable telephony NB Call BT NREC \r\n");
                	app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_VOICE_CALL);
                	AUDCTRL_SaveAudioModeFlag(AUDCTRL_SPK_HANDSFREE, app_profile);
            	}

                DEBUG(" Telephony : Turning Off EC and NS \r\n");
                //AUDCTRL_EC(FALSE, 0);
                //AUDCTRL_NS(FALSE);
            }
            else
            {
                DEBUG(" Telephony : Turning On EC and NS \r\n");
                //AUDCTRL_EC(TRUE, 0);
                //AUDCTRL_NS(TRUE);
            }

            vol_level= GetCtrlValue(BRCM_CTL_EAR_Playback_Volume);
            if(vol_level > 5)
                vol_level = 5;
            AUDCTRL_SetPlayVolume(AUDIO_HW_VOICE_OUT,AUDCTRL_SPK_HANDSET,AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN,vol_level,vol_level);

            AUDCTRL_RateChangeTelephony();
            AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,sgTableIDChannelOfVoiceCallDev[telephony_stream_number-VOICE_CALL_SUB_DEVICE].mic,sgTableIDChannelOfVoiceCallDev[telephony_stream_number-VOICE_CALL_SUB_DEVICE].speaker);
            telephony_started = 1;

	        if(TRUE==GetCtrlValue(BRCM_CTL_Mic_Capture_Mute))
            {
                DEBUG("Muting device \r\n");
			    AUDCTRL_SetTelephonyMicMute(AUDIO_HW_VOICE_OUT,AUDCTRL_MIC_MAIN,TRUE);;
            }
            AUDIO_Policy_SetState(BRCM_STATE_INCALL);


        }
        break;

        case ACTON_VOICECALL_STOP:
        {
            BRCM_VOICE_Param_Stop_t* param_voice_stop = (BRCM_VOICE_Param_Stop_t*)arg_param;

            telephony_stream_number = param_voice_stop->substream_number;

            DEBUG("DISABLE TELEPHONY substream = %d \r\n", telephony_stream_number);
            AUDCTRL_DisableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,sgTableIDChannelOfVoiceCallDev[telephony_stream_number-VOICE_CALL_SUB_DEVICE].mic,sgTableIDChannelOfVoiceCallDev[telephony_stream_number-VOICE_CALL_SUB_DEVICE].speaker);
            telephony_started = 0;
             AUDIO_Policy_RestoreState();
        }
        break;

        case ACTON_VOICECALL_UPDATE:
        {
            int local_sub_stream_number = 0;
            BRCM_VOICE_Param_Update_t* param_voice_update = (BRCM_VOICE_Param_Update_t*)arg_param;

            DEBUG("TELEPHONY UPDATE codecId = %d \r\n", param_voice_update->voicecall_codecId);

            if ( telephony_codecId != param_voice_update->voicecall_codecId )
            {
                telephony_codecId = param_voice_update->voicecall_codecId;

                if (telephony_started)
                {
                    if ( telephony_codecId == 10 )
                    {
                        app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_VOICE_CALL_WB);
                        DEBUG("call_CodedId_hander : changing Mode to AMR-WB ===>\r\n");
                        AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfVoiceCallDev[telephony_stream_number - VOICE_CALL_SUB_DEVICE].speaker + AUDIO_MODE_NUMBER,app_profile);
                        DEBUG("call_CodedId_hander : changing Mode to AMR-WB <===\r\n");
                    }
                    else
                    {
                        app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_VOICE_CALL);
                        DEBUG("call_CodedId_hander : changing Mode to AMR-NB ===>\r\n");
                        AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfVoiceCallDev[telephony_stream_number - VOICE_CALL_SUB_DEVICE].speaker,app_profile );
                        DEBUG("call_CodedId_hander : changing Mode to AMR-NB <===\r\n");
                    }
                    DEBUG("call_CodedId_hander : AUDCTRL_RateChangeTelephony ===>\r\n");
                    if ( musicduringcall == TRUE )
                    {
                        local_sub_stream_number = (telephony_stream_number - VOICE_CALL_SUB_DEVICE)*2;
                        // wired headphone
                        if(telephony_stream_number == 17)
                        {
                            // set the substream number to same as headset
                            local_sub_stream_number = 4;
                        }

                        AUDCTRL_DisableTap (AUDIO_HW_TAP_VOICE);
                        if ( app_profile == AUDIO_APP_VOICE_CALL )
                        {
                            
                            AUDCTRL_RateChangeTelephony();
                            if( sgTableIDChannelOfDev[local_sub_stream_number].speaker == AUDCTRL_SPK_HEADSET ||
                                sgTableIDChannelOfDev[local_sub_stream_number].speaker == AUDCTRL_SPK_BTM )
                            {
                                AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_BTM, 8000);
                            }
                            else
                            {
                                AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_HEADSET, 8000);
                            }
                        }
                        else
                        {
                            AUDCTRL_RateChangeTelephony();
                            if( sgTableIDChannelOfDev[local_sub_stream_number].speaker == AUDCTRL_SPK_HEADSET )
                            {
                                AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_BTM, 16000);
                            }
                            else if ( sgTableIDChannelOfDev[local_sub_stream_number].speaker == AUDCTRL_SPK_BTM )
                            {
                                AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_BTM, 8000);
                            }
                            else
                            {
                                AUDCTRL_EnableTap (AUDIO_HW_TAP_VOICE, AUDCTRL_SPK_HEADSET, 16000);
                            }
                        }
                    }
                    else
                    {
                        AUDCTRL_RateChangeTelephony();
                    }
                    DEBUG("call_CodedId_hander : AUDCTRL_RateChangeTelephony <===\r\n");
                }
            }
        }
        break;
		
        case ACTON_FM_START:
        {
	    BRCM_FM_Param_Start_t* param_start = (BRCM_FM_Param_Start_t*) arg_param;
        app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_FM);
			
      	    AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfDev[param_start->substream_number].speaker,app_profile);
                			
	        // Enable the FM playback the path
            AUDCTRL_EnablePlay(AUDIO_HW_I2S_IN,
				sgTableIDChannelOfDev[param_start->substream_number].hw_id,
				AUDIO_HW_NONE,
				sgTableIDChannelOfDev[param_start->substream_number].speaker,
				param_start->channels,
				param_start->rate
						);
            // set the slopgain register to max value
            // 20110530 FM radio volume is controlled by FM radio chip.
            AUDCTRL_SetPlayVolume(sgTableIDChannelOfDev[param_start->substream_number].hw_id,
				  sgTableIDChannelOfDev[param_start->substream_number].speaker,
				  AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE,
				  AUDIO_VOLUME_MAX,
				  AUDIO_VOLUME_MAX 
						);

            AUDIO_Policy_SetState(BRCM_STATE_FM);

	    }
        break;
		
        case ACTON_FM_STOP:
        {

            BRCM_FM_Param_Stop_t* param_fm_stop = (BRCM_FM_Param_Stop_t*)arg_param;
	
            //disable the FM playback path
            AUDCTRL_DisablePlay(AUDIO_HW_I2S_IN,
				  sgTableIDChannelOfDev[param_fm_stop->substream_number].hw_id,
				  sgTableIDChannelOfDev[param_fm_stop->substream_number].speaker
						 );
            AUDIO_Policy_RestoreState();
        }
        break;
        case  ACTON_ROUTE:
            {
                BRCM_AUDIO_Param_Route_t* parm_route = (BRCM_AUDIO_Param_Route_t*)arg_param;
                 DEBUG("ACTON_ROUTE  \n");			
                

                if(playback_triggered == 1){

                    if(parm_route->command == 1)
                    {
                        DEBUG(" ADDING  SPK - %d  \n",parm_route->speaker);				  
                        AUDCTRL_AddPlaySpk(AUDIO_HW_PLR_OUT,
                                               parm_route->speaker
                                              );
                        extra_speaker = 1;

                    }
                    else if (parm_route->command == 0)
                    {
             
                        DEBUG(" REMOVING  SPK - %d  \n",parm_route->speaker);				  
                        AUDCTRL_RemovePlaySpk(AUDIO_HW_PLR_OUT, 
									      parm_route->speaker	
                        );

                        extra_speaker = 0;
                    }
                }
                else
                {
                    if(parm_route->command == 1)
                        extra_speaker = 1;
                    else if (parm_route->command == 0)
                        extra_speaker = 0;
                }
            }
            break;
        default:
            DEBUG("Error AUDIO_Ctrl_Process Invalid acction command \n");
    }
    if(block)
    {
        // put the message in output fifo if waiting
        msgAudioCtrl.action_code = action_code;
	    if(arg_param)
		    memcpy(&msgAudioCtrl.param, arg_param, sizeof(BRCM_AUDIO_Control_Params_un_t));
	    else
		    memset(&msgAudioCtrl.param, 0, sizeof(BRCM_AUDIO_Control_Params_un_t));
	    msgAudioCtrl.pCallBack = callback;
        msgAudioCtrl.block = block;

        len = kfifo_in_locked(&sgThreadData.m_pkfifo_out, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl), &sgThreadData.m_lock_out);
        if(len != sizeof(TMsgAudioCtrl))
		    DEBUG("Error AUDIO_Ctrl_Process len=%d expected %d \n", len, sizeof(TMsgAudioCtrl));
#if 1        
        // release the semaphore 
        DEBUG("Semaphore released - %d \n",action_code);
        OSSEMAPHORE_Release(sgThreadData.action_complete);
#endif
    }

}
