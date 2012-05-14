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
*  brcm_audio_policy.c
*
*  PURPOSE:
*
*     Apply policy based on current mode and new mode.
*
*  NOTES:
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */


#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"

#include "brcm_audio_policy.h"

// Include BRCM AAUD driver API header files
#include "audio_controller.h"

/* Local typedefs */
#include "log.h"
#include "brcm_alsa.h"

static UInt32 tState = BRCM_STATE_NORMAL;
static UInt32 tPrevState = BRCM_STATE_NORMAL;

/* ---- Functions ecported ---------------------------------------------------- */

static Int32 NextAudioProfile[AUDIO_STATE_NUM][AUDIO_APP_NUMBER] =
{
    {AUDIO_APP_VOICE_CALL, AUDIO_APP_VOICE_CALL_WB, AUDIO_APP_MUSIC,AUDIO_APP_RECORDING,AUDIO_APP_FM, AUDIO_APP_RECORDING_GVS,AUDIO_APP_VOIP,AUDIO_APP_VOIP_INCOMM},  //BRCM_STATE_NORMAL
    {AUDIO_APP_VOICE_CALL, AUDIO_APP_VOICE_CALL_WB,-1,-1, -1, -1,-1,-1}, //BRCM_STATE_INCALL
    {AUDIO_APP_VOICE_CALL, AUDIO_APP_VOICE_CALL_WB, AUDIO_APP_FM,AUDIO_APP_RECORDING,AUDIO_APP_FM, AUDIO_APP_RECORDING_GVS,AUDIO_APP_VOIP,AUDIO_APP_VOIP_INCOMM},  //BRCM_STATE_FM
    {AUDIO_APP_VOICE_CALL, AUDIO_APP_VOICE_CALL_WB, -1,AUDIO_APP_RECORDING,-1, AUDIO_APP_RECORDING_GVS,AUDIO_APP_VOIP,AUDIO_APP_VOIP_INCOMM}  //BRCM_STATE_RECORD
};

/* ---- Data structure  ------------------------------------------------- */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AUDIO_Policy_SetState
//	Sets the state.
//----------------------------------------------------------------
Result_t AUDIO_Policy_SetState(UInt32 state)
{
    if(state < BRCM_STATE_NORMAL || state >= BRCM_STATE_END)
    {
        return 0;
    }
    // back up the current state
    tPrevState  = tState;
    // if we are currently in incall state, don't change it.
    if(tState == BRCM_STATE_INCALL)
    {
        return 0;
    }
    tState = state;

    DEBUG("AUDIO_Policy_SetState:tPrevState - %d tState-%d state - %d\n",tPrevState,tState,state);
    return 1;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AUDIO_Policy_RestoreState
//	restore the state.
//----------------------------------------------------------------
Result_t AUDIO_Policy_RestoreState()
{
    tState = tPrevState;
    tPrevState = BRCM_STATE_NORMAL;
    DEBUG("AUDIO_Policy_RestoreState:tPrevState - %d tState-%d \n",tPrevState,tState);
    return 1;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AUDIO_Policy_GetState
//	Gets the state.
//----------------------------------------------------------------
UInt32 AUDIO_Policy_GetState(void)
{
     return tState;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AUDIO_Policy_Get_Profile
//	Identifies the profile to be applied.
//----------------------------------------------------------------
UInt32 AUDIO_Policy_Get_Profile(UInt32 app)
{
    AudioApp_t  new_app, cur_app;

    if(app >= AUDIO_APP_NUMBER)
    {
        app = AUDIO_APP_VOICE_CALL;  // defalult profile
    }
    cur_app = AUDCTRL_GetAudioApp();

    if(cur_app >= AUDIO_APP_NUMBER)
    {
        cur_app = AUDIO_APP_VOICE_CALL;  // defalult profile
    }
    new_app = NextAudioProfile[tState][app];
    if(new_app == -1)
    {
        //Set the profile to existing profile
        new_app = cur_app;
    }
    DEBUG("AUDIO_Policy_Get_Profile:cur_app - %d new_app-%d app - %d\n",cur_app,new_app,app);
    return new_app;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AUDIO_Policy_Get_Mode
//	Identifies the mode to be applied.
//----------------------------------------------------------------
UInt32 AUDIO_Policy_Get_Mode(UInt32 mode)
{
    AudioMode_t  new_mode, cur_mode;

    cur_mode = AUDCTRL_GetAudioMode();

    if ( cur_mode >= AUDIO_MODE_NUMBER )
        cur_mode = (AudioMode_t) (cur_mode - AUDIO_MODE_NUMBER);

    if(tState == BRCM_STATE_INCALL)
        new_mode = cur_mode;
    else 
        new_mode = mode;

    return new_mode;
}
