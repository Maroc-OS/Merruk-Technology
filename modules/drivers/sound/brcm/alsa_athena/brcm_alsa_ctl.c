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
//#define FM_ENABLE_SYSFS_INTERFACE 0 //uncommnet this to enable sysfs interface to control FM

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include <linux/broadcom/hw_cfg.h>

#include "brcm_alsa.h"



#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "brcm_audio_thread.h"
// Include BRCM AAUD driver API header files
#include "audio_controller.h"

//Control item name
static char gpstrCtrlName[BRCM_CTL_TOTAL][40] =
{
	"Speaker Playback Volume",
	"Speaker Playback Switch",

	"Mic Capture Volume", 
	"Mic Capture Switch",
	
	"Earpiece Playback Volume",
	"Earpiece Playback Switch",

	"Headset Playback Volume",
	"Headset Playback Switch",

       "BTHeadset Volume",
	"BTHeadset Switch",
	"PCM Playback Route"
};
 
BRCM_CTRL_ENU		gCurPath = BRCM_CTL_TOTAL;

extern void audio_init(void);

static long			sgBrcmCtrlVal[BRCM_CTL_TOTAL];
#define	FMDirectPlayCount	5
static	int	sgFMDirectPlayValues[FMDirectPlayCount];

static const AUDIO_SAMPLING_RATE_t samplerate_tbl[]={8000,11030,12000,16000,22060,24000,32000,44100,48000,96000};
static const int FM_chmode[]={AUDIO_CHANNEL_STEREO,AUDIO_CHANNEL_MONO,AUDIO_CHANNEL_STEREO_RIGHT,AUDIO_CHANNEL_STEREO};

// FM Direct play sysFs interface methods
ssize_t FMDirPlaySysfs_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t FMDirPlaySysfs_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static struct device_attribute FMDirectplay_attrib = __ATTR(FMDirectPlay, 0644, FMDirPlaySysfs_show, FMDirPlaySysfs_store);	

#ifdef FM_ENABLE_SYSFS_INTERFACE
static int FMIsOn = 0;
#endif

//*****************************************************************
// Functiona Name: GetCtrlValue
//
// Description: Get control value by index
//
//*****************************************************************
long GetCtrlValue(int ctrlIndex)
{
	long val = 0;
	if(ctrlIndex>=0 && ctrlIndex<BRCM_CTL_TOTAL)
		val = sgBrcmCtrlVal[ctrlIndex];
	else
		DEBUG("GetCtrlValue: Invalid index=%d", ctrlIndex);
	return val;
}


//*****************************************************************
// Functiona Name: GetCtrlInfo
//
// Description: Get control item properties: range, channels, step, etc
//
//*****************************************************************
static int GetCtrlInfo(
	struct snd_kcontrol * kcontrol,
	struct snd_ctl_elem_info * uinfo
)
{
//	DEBUG("\n%lx:GetCtrlInfo ctrl->private_value=%d",jiffies, kcontrol->private_value);

	switch(kcontrol->private_value)
	{
		case BRCM_CTL_Speaker_Playback_Volume:
		case BRCM_CTL_HEADSET_Playback_Volume:
			uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
			uinfo->count = 2;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = 15;
			uinfo->value.integer.step = 1;
		break;

		case BRCM_CTL_EAR_Playback_Volume:
		case BRCM_CTL_Mic_Capture_Volume:
			uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
			uinfo->count			 = 1;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = 5;
			uinfo->value.integer.step = 1;

		break;

		case BRCM_CTL_Speaker_Playback_Mute:
		case BRCM_CTL_HEADSET_Playback_Mute:
			
			uinfo->type			     = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
			uinfo->count			 = 2;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = 1;		
			break;

		case BRCM_CTL_EAR_Playback_Mute:
		case BRCM_CTL_Mic_Capture_Mute:
			uinfo->type			     = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
			uinfo->count			 = 1;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = 1;		
		break;

		case BRCM_CTL_BTHeadset_Volume:
			//do something for BT volume
			uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
			uinfo->count			 = 1;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = AUDIO_MAX_OUTPUT_VOLUME;
			uinfo->value.integer.step = 1;
					
		break;
		
		case BRCM_CTL_BTHeadset_Mute:
			//do something for BT mute
			uinfo->type			     = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
			uinfo->count			 = 1;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = 1;		
			
		break;			
		
		case BRCM_CTL_ROUTE:
        		DEBUG("\nBRCM_CTL_ROUTE");
			uinfo->type			     = SNDRV_CTL_ELEM_TYPE_INTEGER;
			uinfo->count			 = 2;	   // 0 - add or remove   1 - speaker type
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = AUDCTRL_SPK_TOTAL_COUNT;	
			uinfo->value.integer.step = 1;
		break;
	
		default:
			DEBUG("\n%lx:Unknown GetCtrlInfo ctrl->private_value=%d",jiffies, (int)kcontrol->private_value);

	}

	
	return 0;

}


//*****************************************************************
// Functiona Name: GetControlItem
//
// Description: Get control item properties: range, channels, step, etc
//
//*****************************************************************
static int GetControlItem(
	struct snd_kcontrol * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	//int i=0;
	//DEBUG("\n%lx:GetControlItem",jiffies);
	DEBUG("\n%lx:GetControlItem ctrl->private_value=%d",jiffies, kcontrol->private_value);


	if(kcontrol->private_value>=0 && kcontrol->private_value<BRCM_CTL_TOTAL)
		ucontrol->value.integer.value[0] = sgBrcmCtrlVal[kcontrol->private_value];

	return 0;
}


//+++++++++++++++++++++++++++++++++++++++
//FMDirPlaySysfs_show (struct device *dev, struct device_attribute *attr, char *buf)
// Buffer values syntax -	 0 - on/off, 1 - output device, 2 - sample rate index, 3 - channel, 4 -volume, 
//
//---------------------------------------------------

ssize_t FMDirPlaySysfs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  	int i;
	char sbuf[256];
	for(i=0; i< sizeof(sgFMDirectPlayValues)/sizeof(sgFMDirectPlayValues[0]); i++)
	{
		snprintf(sbuf, sizeof(sbuf), "%d ",sgFMDirectPlayValues[i]); 
		strcat(buf, sbuf);
	}
	return strlen(buf);
}

//+++++++++++++++++++++++++++++++++++++++
// FMDirPlaySysfs_store (struct device *dev, struct device_attribute *attr, char *buf)
// Buffer values syntax -	 0 - on/off, 1 - output device, 2 - sample rate index, 3 - channel, 4 -volume, 
//
//---------------------------------------------------

ssize_t FMDirPlaySysfs_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
   	//int i=0;
	
	
	if(5!= sscanf(buf, "%d %d %d %d %d",&sgFMDirectPlayValues[0], &sgFMDirectPlayValues[1], &sgFMDirectPlayValues[2], &sgFMDirectPlayValues[3], &sgFMDirectPlayValues[4]))
	{
	
		DEBUG("\n<-FMDirectPlay SysFS Handler: On_Off=%d OpMode=%d, samplerate=%d, channel=%d, volume=%d \n",  sgFMDirectPlayValues[0],
											sgFMDirectPlayValues[1],
											sgFMDirectPlayValues[2],
											sgFMDirectPlayValues[3],
											sgFMDirectPlayValues[4]);
		DEBUG("error reading buf=%s count=%d\n", buf, count);
		return count;
	}
        DEBUG("\n<-FMDirectPlay SysFS Handler: On_Off=%d OpMode=%d, samplerate=%d, channel=%d, volume=%d \n",  sgFMDirectPlayValues[0],
											sgFMDirectPlayValues[1],
											samplerate_tbl[sgFMDirectPlayValues[2]],
											FM_chmode[sgFMDirectPlayValues[3]],
											sgFMDirectPlayValues[4]);

#ifdef FM_ENABLE_SYSFS_INTERFACE
	if(sgFMDirectPlayValues[0]) 		// enable path
	{
		if ( FMIsOn == 0 )
		{
			AUDCTRL_SaveAudioModeFlag((AUDCTRL_SPEAKER_t)sgFMDirectPlayValues[1],AUDIO_APP_FM);
			AUDCTRL_EnablePlay(AUDIO_HW_I2S_IN,
					   AUDIO_HW_AUDIO_OUT,
					   AUDIO_HW_NONE,
					   (AUDCTRL_SPEAKER_t)sgFMDirectPlayValues[1],
					   (AUDIO_CHANNEL_NUM_t)FM_chmode[sgFMDirectPlayValues[3]],
					   samplerate_tbl[sgFMDirectPlayValues[2]]
				          );
			FMIsOn = 1;
		}
		AUDCTRL_SetPlayVolume(AUDIO_HW_AUDIO_OUT,
				      (AUDCTRL_SPEAKER_t)sgFMDirectPlayValues[1],
				      AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE,
				      sgFMDirectPlayValues[4],
				      sgFMDirectPlayValues[4]
				);
	}
	else								//disable path
	{
		if ( FMIsOn )
		{
			AUDCTRL_DisablePlay(AUDIO_HW_I2S_IN,
				            AUDIO_HW_AUDIO_OUT,
					    (AUDCTRL_SPEAKER_t)sgFMDirectPlayValues[1]
					   );
			FMIsOn = 0;
		}
	}
#endif //FM_ENABLE_SYSFS_INTERFACE
	return count;
     
}



//*****************************************************************
// Functiona Name: SetControlItem
//
// Description: Set control item, will set hardware if value changed.
//
//*****************************************************************
static int SetControlItem(
	struct snd_kcontrol * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	int changed = 0;
    int vol = 0;
    int mute = 0;

    audio_init();
	//DEBUG("\n%lx:SetControlItem",jiffies);

	if(kcontrol->private_value>=0 && kcontrol->private_value<BRCM_CTL_TOTAL)
	{
		if(!(kcontrol->private_value&1))//volume
		{
			DEBUG("\n%lx:SetControlItem [%s]=%d",jiffies, gpstrCtrlName[kcontrol->private_value], (int)ucontrol->value.integer.value[0]);
			if((gCurPath != kcontrol->private_value) && (BRCM_CTL_Mic_Capture_Volume!=kcontrol->private_value)) //gCurPath is for playback only
			{
				//Enable path with volume
				DEBUG("\n%lx:SetControlItem path = %d changed to %d\n",jiffies, (int)gCurPath, (int)kcontrol->private_value);
				sgBrcmCtrlVal[kcontrol->private_value] = ucontrol->value.integer.value[0] ;
				changed=1;
				gCurPath = kcontrol->private_value;
			}

			if(sgBrcmCtrlVal[kcontrol->private_value] != ucontrol->value.integer.value[0] )
			{
				sgBrcmCtrlVal[kcontrol->private_value] = ucontrol->value.integer.value[0] ;
//				audvoc_configure_Volume(0, 0x1FF) ;
				changed=1;
			}
			DEBUG("SetControlItem: sgBrcmCtrlVal[%d] = %d\n", kcontrol->private_value, sgBrcmCtrlVal[kcontrol->private_value] );
		}
		else //mute
		{
			DEBUG("\n%lx:mute SetControlItem [%s]=%d",jiffies, gpstrCtrlName[kcontrol->private_value], (int)ucontrol->value.integer.value[0]);

			//if(sgBrcmCtrlVal[kcontrol->private_value] != ucontrol->value.integer.value[0] )
			{
				sgBrcmCtrlVal[kcontrol->private_value] = ucontrol->value.integer.value[0] ;
				changed=1;
			}
		}

		if(changed)
		{
            
			DEBUG("\n SET SetControlItem [%s]=%d", gpstrCtrlName[kcontrol->private_value], (int)ucontrol->value.integer.value[0]);

	        switch(kcontrol->private_value)
			{
				case BRCM_CTL_EAR_Playback_Volume:
                    {
                        vol = (int)ucontrol->value.integer.value[0];						

                        // ensure that it is not completely muted at the minimum volume level
                        if(vol > 5)
                            vol = 5;
                        AUDCTRL_SetPlayVolume(AUDIO_HW_VOICE_OUT,AUDCTRL_SPK_HANDSET,AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN,vol,vol);
							
                    }
                    break;
	
		case BRCM_CTL_Speaker_Playback_Volume:
			{ 
				unsigned int vol;
   				vol = (int)ucontrol->value.integer.value[0];

				if (vol == 0) {
					vol = AUDIO_VOLUME_FM_MUTE;  // for mute case send 0xff to distinguish from other values.
				}	
				else {
                                    vol = vol -1;  // volume range is converted from 15-1 to 14-0
				}	
			
				//if we are to set volume for FM app then apply FM app gain table
                      		AUDCTRL_SetPlayVolume(AUDIO_HW_AUDIO_OUT,AUDCTRL_SPK_LOUDSPK,
                      					 AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE,
				      				vol,
				      				vol);
				DEBUG("Set FM Play vol to %d\n",vol);
			}
                    break;

		case BRCM_CTL_Mic_Capture_Mute:
                    {
                        mute = (int)ucontrol->value.integer.value[0];

                        AUDCTRL_SetTelephonyMicMute(AUDIO_HW_VOICE_OUT,AUDCTRL_MIC_MAIN,mute);
                    }
                	break;
	    case BRCM_CTL_ROUTE:
            {   
                BRCM_AUDIO_Param_Route_t parm_route;

                parm_route.command = (int)ucontrol->value.integer.value[0];
                parm_route.speaker = (int)ucontrol->value.integer.value[1];
                // Send the route command
                AUDIO_Ctrl_Trigger(ACTON_ROUTE,&parm_route,NULL,0);
            }
            break;
	
      			
                default:
                    break;
            }

		}


	}


	
	return changed;
}

//Mixer control item definition
#define BRCM_MIXER_CTRL(xname, xindex, private_val) \
{	\
   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .name = xname, \
  .index = xindex, \
  .access= SNDRV_CTL_ELEM_ACCESS_READWRITE,\
  .private_value = private_val, \
  .info = GetCtrlInfo, \
  .get = GetControlItem, 	\
  .put = SetControlItem, \
}

//Define all the control items
static struct snd_kcontrol_new gpSndCtrls[] __devinitdata =
{
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Speaker_Playback_Volume], 0, BRCM_CTL_Speaker_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Speaker_Playback_Mute], 0, BRCM_CTL_Speaker_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Mic_Capture_Volume], 0, BRCM_CTL_Mic_Capture_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Mic_Capture_Mute], 0, BRCM_CTL_Mic_Capture_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_EAR_Playback_Volume], 0, BRCM_CTL_EAR_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_EAR_Playback_Mute], 0, BRCM_CTL_EAR_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_HEADSET_Playback_Volume], 0, BRCM_CTL_HEADSET_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_HEADSET_Playback_Mute], 0, BRCM_CTL_HEADSET_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_BTHeadset_Volume], 0, BRCM_CTL_BTHeadset_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_BTHeadset_Mute], 0, BRCM_CTL_BTHeadset_Mute),
	
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_ROUTE], 0, BRCM_CTL_ROUTE)
};

//*****************************************************************
// Functiona Name: ControlDeviceNew
//
// Description: Create control device.
//
//*****************************************************************
int __devinit ControlDeviceNew(struct snd_card *card)
{
	unsigned int idx;
	int err;

	strcpy(card->mixername, "Broadcom ALSA Mixer");

	for (idx = 0; idx < ARRAY_SIZE(gpSndCtrls); idx++)
	{
		if ((err = snd_ctl_add(card, snd_ctl_new1(&gpSndCtrls[idx], g_brcm_alsa_chip))) < 0)
			return err;
	}

	gCurPath = BRCM_CTL_TOTAL;
	memset(sgBrcmCtrlVal, 0, sizeof(sgBrcmCtrlVal));
   
   return 0;
}

int BrcmCreateControlSysFs(struct snd_card *card)
{
	int ret;
	//create sysfs file for FM directplay control
	ret = snd_add_device_sysfs_file(SNDRV_DEVICE_TYPE_CONTROL,card,-1,&FMDirectplay_attrib);
	//	DEBUG("BrcmCreateControlSysFs ret=%d", ret);	
	return ret;
}

