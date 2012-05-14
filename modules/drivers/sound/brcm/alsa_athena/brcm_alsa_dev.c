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


#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include "plat/osdal_os.h"
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "ossemaphore.h"
#include <linux/delay.h>

#include "audio_controller.h"
#include "audio_ddriver.h"
#include "audio_vdriver_voif.h"
#include "audio_vdriver_gpio.h"

#include "brcm_alsa.h"
#include "brcm_audio_thread.h"
#include  "brcm_audio_devices.h"

#include "brcm_alsa_pcg.h"

#include "linux/broadcom/bcm_fuse_sysparm_CIB.h"
#include "linux/broadcom/bcm_reset_utils.h"



#include "shared.h"
#include "auddrv_audlog.h"

#include <linux/broadcom/hw_cfg.h>
#include <linux/broadcom/bcm_major.h>

#include "log_sig_code.h"

#include "ostask.h"

//  Module declarations.
//
MODULE_AUTHOR("Broadcom MobMM");
MODULE_DESCRIPTION("Broadcom AP sound interface");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{ALSA,Broadcom AP soundcard}}");

//global
brcm_alsa_chip_t *g_brcm_alsa_chip=NULL;
int debug = 2; 

//
static char *id = NULL;
static int enable = 1;
static int index = 0;

//
module_param(index, int, 0444);
MODULE_PARM_DESC(index, "Index value for Broadcom soundcard.");
module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for Broadcom soundcard.");
module_param(enable, bool, 0444);
MODULE_PARM_DESC(enable, "Enable the Broadcom soundcard.");
module_param(debug, int, 0444);
MODULE_PARM_DESC(debug, "debug value for Broadcom soundcard.");


//--------------------------------------------------------------------
// AUDIO LOGGING

#define DATA_TO_READ 4
int logging_link[LOG_STREAM_NUMBER] = {0,0,0,0};

// wait queues
extern wait_queue_head_t bcmlogreadq;

//static int audio_stop = 0;
int audio_data_arrived = 0;


// IOCTL for audio logging
#define LOG_IOCTL_CONFIG_CHANNEL                105
#define LOG_IOCTL_START_CHANNEL                 106
#define LOG_IOCTL_STOP                          107
#define LOG_IOCTL_GET_DSPDATA_ID		      108	

#define VOIP_IOCTL_ENABLE                111
#define VOIP_IOCTL_DISABLE               112
#define readbuffnodata 99
#define VOIP_FRAME_SIZE 320
#define VOIP_FRAMES_IN_BUFFER 50

#define VOIP_BUFFER_SIZE (VOIP_FRAME_SIZE*VOIP_FRAMES_IN_BUFFER)

typedef struct AUDIO_VOIP_DRIVER_t
{
    UInt32 voip_data_dl_rd_index;
    UInt32 voip_data_dl_wr_index;
    UInt32 voip_dl_framecount;
    UInt32 voip_data_ul_rd_index;
    UInt32 voip_data_ul_wr_index;
    UInt32 voip_ul_framecount;
    UInt8* voip_data_dl_buf_ptr;
    UInt8* voip_data_ul_buf_ptr;
    AUDIO_DRIVER_HANDLE_t drv_handle;
}AUDIO_VOIP_DRIVER_t;

AUDIO_VOIP_DRIVER_t* voip_driver_handle = NULL;
Semaphore_t voip_driver_lock = NULL;

static Boolean VOIP_DumpUL_CB(AUDIO_DRIVER_HANDLE_t drv_handle, UInt8	*pSrc, UInt32 amrMode);
static Boolean VOIP_FillDL_CB(AUDIO_DRIVER_HANDLE_t drv_handle, UInt8 *pDst, UInt32 nFrames);

extern UInt16 *bcmlog_stream_area;


extern int BrcmCreateAuddrv_testSysFs(struct snd_card *card); //brcm_auddrv_test.c
extern int BrcmCreateControlSysFs(struct snd_card *card); //brcm_alsa_ctl.c

extern TIDChanOfVoiceCallDev	sgTableIDChannelOfVoiceCallDev[];

extern SysAudioParm_t* SYSPARM_GetAudioParmAccessPtr(UInt8 AudioMode);
extern 	UInt16	AUDIO_Util_Convert( UInt16 input, UInt16 scale_in, UInt16 scale_out);

static void 
AUDTST_VOIF (UInt32 Val2,UInt32 Val3, UInt32 Val4, UInt32 Val5, UInt32 Val6);

//--------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverProbe
//
//  Description: 'probe' call back function
//
//------------------------------------------------------------
static int __devinit DriverProbe(struct platform_device *pdev)
{
	struct snd_card *card;
	int err;
	
	DEBUG("\n %lx:DriverProbe \n",jiffies);
		
	err = -ENODEV;
	if (!enable)
      return err;
    
    err = -ENOMEM;  
	err  = snd_card_create(SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1, 
		THIS_MODULE, sizeof(brcm_alsa_chip_t), &card);
		
	if (!card)
      goto err;
            
	g_brcm_alsa_chip = (brcm_alsa_chip_t*)card->private_data;
	g_brcm_alsa_chip->card = card;
	
	card->dev = &pdev->dev;
	strncpy(card->driver, pdev->dev.driver->name, sizeof(card->driver));


	//PCM interface	
	err = PcmDeviceNew(card);
	if (err)
    	goto err;

    //CTRL interface	
	err = ControlDeviceNew(card);
	if (err)
    	goto err;

		
	strcpy(card->driver, "Broadcom");
	strcpy(card->shortname, "Broadcom ALSA");
	sprintf(card->longname, "Broadcom ALSA PCM %i", 0);
	
	
	err = snd_card_register(card);
	if (err==0)
	{
      int ret;
      platform_set_drvdata(pdev, card);

      ret = BrcmCreateAuddrv_testSysFs(card);
      if(ret!=0)
 	  	DEBUG("ALSA DriverProbe Error to create sysfs for Auddrv test ret = %d\n", ret);
	ret = BrcmCreateControlSysFs(card);
	if(ret!=0)
		DEBUG("ALSA DriverProbe Error to create sysfs for FMDirectPlay ret = %d\n", ret);

      return 0;
	}

err:
	DEBUG("\n probe failed =%d\n",err);
	if (card)
		snd_card_free(card);
	
	g_brcm_alsa_chip=NULL;
	return err;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverRemove
//
//  Description: 'remove' call back function
//
//------------------------------------------------------------
static int DriverRemove(struct platform_device *pdev)
{
	return 0;
}


static int DriverSuspend(
		struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverResume
//
//  Description: 'resume' call back function
//
//------------------------------------------------------------
static int DriverResume(struct platform_device *pdev)
{
	return 0;
}

//Platform device data
typedef struct _PlatformDevData_t
{
	int init;	
} PlatformDevData_t;

static PlatformDevData_t sgDeviceInfo =
{
	.init = 0,
};                                

//---------------------------------------------------------------------------
// File opeations for audio logging

static int
BCMLOG_open(struct inode *inode, struct file *file)
{

    return 0;
} /* BCMLOG_open */


/****************************************************************************
*
*  BCMLOG_read
*
***************************************************************************/
static int
BCMLOG_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{


    if ( wait_event_interruptible(bcmlogreadq, (audio_data_arrived != 0)))
    {   
        //(" Wait for read  ...\n");
        return -ERESTARTSYS;
    }   
    audio_data_arrived = 0;  

	copy_to_user(buf, "read", 4); 
    return DATA_TO_READ;
               
}

/****************************************************************************
*
*  BCMLOG_release
*
***************************************************************************/

static int
BCMLOG_release(struct inode *inode, struct file *file)
{

    return 0;

} /* BCMLOG_release */

/****************************************************************************
*
*  BCMLOG_mma
*
***************************************************************************/

static int BCMLOG_mmap(struct file *filp, struct vm_area_struct *vma)
{


    int ret;
    long length = vma->vm_end - vma->vm_start;

    /* check length - do not allow larger mappings than the number of pages allocated */
    if (length > (PAGE_SIZE + (sizeof(LOG_FRAME_t)*4)) )
    {
        DEBUG("\n Failed at page boundary \n\r");
        return -EIO;
    }


    /* map the whole physically contiguous area in one piece */
    ret = remap_pfn_range(vma, vma->vm_start,
                    virt_to_phys((void *)bcmlog_stream_area) >> PAGE_SHIFT,
                    length, vma->vm_page_prot);

        if(ret != 0) {
            DEBUG("\n BCMLOG_mmap_kmem -EAGAIN \r\n");
            return -EAGAIN;
        }
 return 0;
}

/****************************************************************************
*
*  BCMLOG_ioctl - IOCTL to set switch to kernel mode for
                1. Audio logging setup
                2. Dump Audio data to MTT &  Copy audio data to user space
                3. Stop and reset audio logging
*
***************************************************************************/

static int
BCMLOG_ioctl(struct inode *inode, struct file *file,
		unsigned int cmd, UInt32 arg)
{

    int rtn = 0;


    switch (cmd)
    {

        case LOG_IOCTL_CONFIG_CHANNEL:
        {
#if 0
            AUDVOC_CFG_LOG_INFO *p_log_info1 = (AUDVOC_CFG_LOG_INFO *) (arg) ;
            rtn= Audio_configure_log_channel (p_log_info1);
            if (rtn < 0 )
            {
                pr_info("\n Couldnt setup channel \n\n");
                rtn = -1;
            }
#endif
        }
        break;
 		
		case LOG_IOCTL_START_CHANNEL:
        {
                                                               
            AUDVOC_CFG_LOG_INFO *p_log_info2 = (AUDVOC_CFG_LOG_INFO *) (arg) ;

			logging_link[p_log_info2->log_link-1] = 1;
            rtn= AUDDRV_AudLog_Start(p_log_info2->log_link,p_log_info2->log_capture_point,p_log_info2->log_consumer);
            if (rtn < 0 )
            {
                pr_info("\n Couldnt setup channel \n\n");
                rtn = -1;
            }
        }
        break;
		
		case LOG_IOCTL_STOP:
        {
            AUDVOC_CFG_LOG_INFO *p_log_info3 = (AUDVOC_CFG_LOG_INFO *) (arg) ;
            logging_link[p_log_info3->log_link-1] = 0;
            rtn = AUDDRV_AudLog_Stop(p_log_info3->log_link);
            if (rtn < 0 )
            {
                rtn = -1;
            }
        }
        break;
	
	case LOG_IOCTL_GET_DSPDATA_ID:
        {
            if(access_ok(VERIFY_WRITE,arg,sizeof(UInt32*)))
		put_user((UInt32)(DSP_DATA),(UInt32*)arg);
	     else
		rtn = -1;		

		DEBUG("\n----> BCMLOG_dsp_data : 0x%x retutn %d\r\n",DSP_DATA,rtn);

        }
        break;

    }
    return(rtn);
}
//---------------------------------------------------------------------------


/**************************************************************************
*
*  BCMPCG_open
*
***************************************************************************/

static int
BCMPCG_open(struct inode *inode, struct file *file)
{
    DEBUG("BCMPCG_open()\r\n");		

    return 0;
} /* BCMPCG_open */

/****************************************************************************
*
*  BCMPCG_release
*
***************************************************************************/

static int
BCMPCG_release(struct inode *inode, struct file *file)
{
    DEBUG("BCMPCG_release()\r\n");		

    return 0;

} /* BCMPCG_release */

UInt32 GetVoiceChannelId(AudioMode_t mode)
{
	UInt32 voice_chnl  = 0;

	switch(mode)
	{
		case	AUDIO_MODE_HANDSET:
		case	AUDIO_MODE_HANDSET_WB:
        case    AUDIO_MODE_HAC:
        case    AUDIO_MODE_HAC_WB:                
			voice_chnl = 1;
			break;
		case	AUDIO_MODE_HEADSET:
		case	AUDIO_MODE_HEADSET_WB:
        case    AUDIO_MODE_TTY:
        case    AUDIO_MODE_TTY_WB:
			voice_chnl = 2;
			break;
		case	AUDIO_MODE_BLUETOOTH:
		case	AUDIO_MODE_BLUETOOTH_WB:
			voice_chnl = 3;
			break;
		case	AUDIO_MODE_SPEAKERPHONE:
		case	AUDIO_MODE_SPEAKERPHONE_WB:
			voice_chnl = 0;
			break;
		default:
			DEBUG("GetVoiceChannelId() mode is out of range %d\r\n", mode);		
			break;
	}

	DEBUG("GetVoiceChannelId() channel %d\r\n", voice_chnl);		
	return voice_chnl;

}

static Boolean VOIP_DumpUL_CB(
AUDIO_DRIVER_HANDLE_t drv_handle,
UInt8		*pSrc,		// pointer to start of speech data
UInt32		amrMode		// AMR codec mode of speech data
)
{
//    OSSEMAPHORE_Obtain(voip_driver_lock, TICKS_FOREVER);
    if( voip_driver_handle )
    {
        if( voip_driver_handle->drv_handle == drv_handle )
        {
            if( voip_driver_handle->voip_data_ul_buf_ptr )
            {
                //DEBUG(" VOIP_DumpUL_CB : VOIP_buf_ul_index 0x%x, \r\n", voip_driver_handle->voip_data_ul_wr_index);
                memcpy(voip_driver_handle->voip_data_ul_buf_ptr + voip_driver_handle->voip_data_ul_wr_index, pSrc, VOIP_FRAME_SIZE);
                voip_driver_handle->voip_ul_framecount++;
                voip_driver_handle->voip_data_ul_wr_index += VOIP_FRAME_SIZE;
                if( voip_driver_handle->voip_data_ul_wr_index >= VOIP_BUFFER_SIZE )
                {
                    voip_driver_handle->voip_data_ul_wr_index -= VOIP_BUFFER_SIZE;
                }
            }
        }
        else
        {
            DEBUG(" VOIP_DumpUL_CB : Audio Driver Mismatch\r\n");
        }
    }
    OSSEMAPHORE_Release(voip_driver_lock);
    return TRUE;
}

static Boolean VOIP_FillDL_CB(AUDIO_DRIVER_HANDLE_t drv_handle, UInt8 *pDst, UInt32 nFrames)
{
//    OSSEMAPHORE_Obtain(voip_driver_lock, TICKS_FOREVER);
    if( voip_driver_handle )
    {
        if( voip_driver_handle->drv_handle == drv_handle )
        {
            if( voip_driver_handle->voip_dl_framecount == 0 )
            {
                 memset(pDst, 0, VOIP_FRAME_SIZE);
//                 OSSEMAPHORE_Release(voip_driver_lock);
                 return TRUE;
            }
            if( voip_driver_handle->voip_data_dl_buf_ptr )
            {
                //DEBUG(" VOIP_FillDL_CB : VOIP_buf_dl_index 0x%x, \r\n", voip_driver_handle->voip_data_dl_rd_index);
                memcpy(pDst, voip_driver_handle->voip_data_dl_buf_ptr + voip_driver_handle->voip_data_dl_rd_index, VOIP_FRAME_SIZE);
                voip_driver_handle->voip_dl_framecount--;
                voip_driver_handle->voip_data_dl_rd_index += VOIP_FRAME_SIZE;
                if( voip_driver_handle->voip_data_dl_rd_index >= VOIP_BUFFER_SIZE )
                {
                    voip_driver_handle->voip_data_dl_rd_index -= VOIP_BUFFER_SIZE;
                }
            }
        }
        else
        {
             memset(pDst, 0, VOIP_FRAME_SIZE);
//             OSSEMAPHORE_Release(voip_driver_lock);
             return TRUE;
        }
    }
    else
    {
        memset(pDst, 0, VOIP_FRAME_SIZE);
//        OSSEMAPHORE_Release(voip_driver_lock);
        return TRUE;
    }
//    OSSEMAPHORE_Release(voip_driver_lock);
    return TRUE;
}

/**************************************************************************
*
*  BCMVOIP_open
*
***************************************************************************/
static int
BCMVOIP_open(struct inode *inode, struct file *file)
{
    return 0;
} /* BCMVOIP_open */

/****************************************************************************
*
*  BCMVOIP_release
*
***************************************************************************/
static int
BCMVOIP_release(struct inode *inode, struct file *file)
{
    return 0;
} /* BCMVOIP_release */

/****************************************************************************
*
*  BCMVOIP_read
*
***************************************************************************/
static ssize_t BCMVOIP_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    OSStatus_t status;

    status = OSSEMAPHORE_Obtain(voip_driver_lock, 40);

    if( status != OSSTATUS_SUCCESS )
    {
        printk("Semaphore obtain failed \r\n");
        return (ssize_t)readbuffnodata;
    }

    if( voip_driver_handle )
    {
        if( voip_driver_handle->voip_ul_framecount )
        {
            if( voip_driver_handle->voip_data_ul_buf_ptr )
            {
                copy_to_user(buf, voip_driver_handle->voip_data_ul_buf_ptr + voip_driver_handle->voip_data_ul_rd_index,320);
                voip_driver_handle->voip_ul_framecount--;
                voip_driver_handle->voip_data_ul_rd_index += VOIP_FRAME_SIZE;
                if( voip_driver_handle->voip_data_ul_rd_index >= VOIP_BUFFER_SIZE )
                {
                    voip_driver_handle->voip_data_ul_rd_index -= VOIP_BUFFER_SIZE;
                }
//                OSSEMAPHORE_Release(voip_driver_lock);
                return (ssize_t) 320;
            }
            else
            {
//                OSSEMAPHORE_Release(voip_driver_lock);
                return (ssize_t)readbuffnodata;
            }
        }
        else
        {
//            OSSEMAPHORE_Release(voip_driver_lock);
            return (ssize_t)readbuffnodata;
        }
    }
    else
    {
//        OSSEMAPHORE_Release(voip_driver_lock);
        return (ssize_t)readbuffnodata;
    }
}

/****************************************************************************
*
*  BCMVOIP_write
*
***************************************************************************/
static ssize_t BCMVOIP_write(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
//    OSSEMAPHORE_Obtain(voip_driver_lock, TICKS_FOREVER);
    if( voip_driver_handle )
    {
        if( voip_driver_handle->voip_data_dl_buf_ptr )
        {
            if( count != VOIP_FRAME_SIZE )
            {
                DEBUG(" BCMVOIP_write:  Write Size does not match VOIP Frame Size\r\n");
            }
            copy_from_user(voip_driver_handle->voip_data_dl_buf_ptr + voip_driver_handle->voip_data_dl_wr_index, buf, VOIP_FRAME_SIZE);
            voip_driver_handle->voip_dl_framecount++;
            voip_driver_handle->voip_data_dl_wr_index += VOIP_FRAME_SIZE;
            if( voip_driver_handle->voip_data_dl_wr_index >= VOIP_BUFFER_SIZE )
            {
                voip_driver_handle->voip_data_dl_wr_index -= VOIP_BUFFER_SIZE;
            }
        }
    }
//    OSSEMAPHORE_Release(voip_driver_lock);
    return (ssize_t) 320;
}

static int
BCMVOIP_ioctl(struct inode *inode, struct file *file,
             unsigned int cmd, UInt32 arg)
{
    int rtn = 1;
    DEBUG(" BCMVOIP_ioctl:  cmd: %d  arg =%d  \r\n",_IOC_NR(cmd),  arg);

    switch (cmd)
    {
        case VOIP_IOCTL_ENABLE:
        {
            DEBUG(" VOIP_IOCTL_ENABLE \r\n");
            voip_driver_handle = (AUDIO_VOIP_DRIVER_t*) OSDAL_ALLOCHEAPMEM(sizeof(AUDIO_VOIP_DRIVER_t));

            if ( voip_driver_lock == NULL )
            {
                voip_driver_lock = OSSEMAPHORE_Create(0,0);
                if ( voip_driver_lock == NULL )
                {
                    DEBUG(" VOIP_IOCTL_ENABLE : Semaphore Creation failed\r\n");
                    return -ENOMEM;
                }
            }

            if ( voip_driver_handle )
            {
                memset((UInt8*) voip_driver_handle, 0, sizeof(AUDIO_VOIP_DRIVER_t));
            }
            else
            {
                DEBUG(" VOIP_IOCTL_ENABLE : Memory allocation failed 1 \r\n");
                return -ENOMEM;
            }

            voip_driver_handle->voip_data_dl_buf_ptr = (UInt8*) OSDAL_ALLOCHEAPMEM(VOIP_BUFFER_SIZE);
            if( voip_driver_handle->voip_data_dl_buf_ptr )
            {
                memset(voip_driver_handle->voip_data_dl_buf_ptr, 0, VOIP_BUFFER_SIZE);
            }
            else
            {
                OSDAL_FREEHEAPMEM( voip_driver_handle );
                DEBUG(" VOIP_IOCTL_ENABLE : Memory allocation failed 2 \r\n");
                return -ENOMEM;
            }

            voip_driver_handle->voip_data_ul_buf_ptr = (UInt8*) OSDAL_ALLOCHEAPMEM(VOIP_BUFFER_SIZE);
            if( voip_driver_handle->voip_data_ul_buf_ptr )
            {
                memset(voip_driver_handle->voip_data_ul_buf_ptr, 0, VOIP_BUFFER_SIZE);
            }
            else
            {
                OSDAL_FREEHEAPMEM( voip_driver_handle->voip_data_dl_buf_ptr );
                OSDAL_FREEHEAPMEM( voip_driver_handle );
                DEBUG(" VOIP_IOCTL_ENABLE : Memory allocation failed 3 \r\n");
                return -ENOMEM;
            }

            voip_driver_handle->drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_VOIP);
            if ( !voip_driver_handle->drv_handle )
            {
                OSDAL_FREEHEAPMEM( voip_driver_handle->voip_data_dl_buf_ptr );
                OSDAL_FREEHEAPMEM( voip_driver_handle->voip_data_ul_buf_ptr );
                OSDAL_FREEHEAPMEM( voip_driver_handle );
                DEBUG(" VOIP_IOCTL_ENABLE : VOIP Driver Open failed \r\n");

                return -ENOMEM;
            }

            DEBUG(" Audio DDRIVER Open Complete\n");

            DEBUG(" Audio VOIP DDRIVER Config\n");
            //set UL callback
            AUDIO_DRIVER_Ctrl(voip_driver_handle->drv_handle,AUDIO_DRIVER_SET_VOIP_UL_CB,(void*)VOIP_DumpUL_CB);

            //set the callback
            AUDIO_DRIVER_Ctrl(voip_driver_handle->drv_handle,AUDIO_DRIVER_SET_VOIP_DL_CB,(void*)VOIP_FillDL_CB);
#if 0
            AUDCTRL_SaveAudioModeFlag(AUDCTRL_SPK_HANDSET,0);
            AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,AUDCTRL_MIC_MAIN,AUDCTRL_SPK_HANDSET);
            AUDCTRL_SetTelephonySpkrVolume(AUDIO_HW_VOICE_OUT, AUDCTRL_SPK_HANDSET, AUDIO_VOLUME_DEFAULT, AUDIO_GAIN_FORMAT_VOL_LEVEL);
            AUDCTRL_SetTelephonyMicGain(AUDIO_HW_VOICE_IN, AUDCTRL_MIC_MAIN, AUDIO_VOLUME_DEFAULT);
#endif
            AUDIO_DRIVER_Ctrl(voip_driver_handle->drv_handle,AUDIO_DRIVER_START,NULL);
        }
        break;

        case VOIP_IOCTL_DISABLE:
        {
//            OSSEMAPHORE_Obtain(voip_driver_lock, TICKS_FOREVER);
            AUDIO_DRIVER_Ctrl(voip_driver_handle->drv_handle,AUDIO_DRIVER_STOP,NULL);
//            AUDCTRL_DisableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,AUDCTRL_MIC_MAIN,AUDCTRL_SPK_HANDSET);

            AUDIO_DRIVER_Close(voip_driver_handle->drv_handle);
            OSDAL_FREEHEAPMEM( voip_driver_handle->voip_data_dl_buf_ptr );
            OSDAL_FREEHEAPMEM( voip_driver_handle->voip_data_ul_buf_ptr );
            OSDAL_FREEHEAPMEM( voip_driver_handle );
            voip_driver_handle = NULL;
//            OSSEMAPHORE_Release(voip_driver_lock);
            OSSEMAPHORE_Reset(voip_driver_lock, 0);
        }
        break;

        default:
            DEBUG("Unrecognized voipioctl: '0x%x'  \n", cmd);
            return -ENOTTY;
    }

    return (rtn);
} /* voip_ioctl */

static int loopback_status = 0;
static int loopback_input = 0;
static int loopback_output = 0;


/*VoIP loopback command */

#define LB_VOIP_FRAMES	100
#define LB_VOIP_FRAME_SIZE 320
#define LB_VOIP_BUFFER_SIZE (LB_VOIP_FRAME_SIZE*LB_VOIP_FRAMES)

static AUDIO_DRIVER_HANDLE_t drv_handle = NULL;
static UInt8* voip_loopbackbuffer = NULL;

//static UInt32 VOIP_buffer_count = 0;
static UInt32 VOIP_buf_ul_index = 0;
static UInt32 VOIP_buf_dl_index = 0;
static UInt32 lp_voip_mic;
static UInt32 lp_voip_speaker;

//UInt32 lp_voip_start = 0; /* 20110715 for loopback check */

static Boolean LB_VOIP_DumpUL_CB(
AUDIO_DRIVER_HANDLE_t drv_handle,
UInt8		*pSrc,		// pointer to start of speech data
UInt32		amrMode		// AMR codec mode of speech data
);

static Boolean LB_VOIP_FillDL_CB(AUDIO_DRIVER_HANDLE_t drv_handle, UInt8 *pDst, UInt32 nFrames);

/*VoIP loopback command end */

/****************************************************************************
*
*  BCMPCG_ioctl 
*
***************************************************************************/

static int
BCMPCG_ioctl(struct inode *inode, struct file *file,
             unsigned int cmd, UInt32 arg)
{
    int rtn = 1;

     
     //const UInt16 volume_max = 14;

    switch (cmd)
    {
    case PCG_IOCTL_SETMODE:
    {
        UInt32 pcg_arg, voice_chnl;
        copy_from_user(&pcg_arg, (UInt32 *)arg, sizeof(UInt32));
        voice_chnl = GetVoiceChannelId(pcg_arg);
        AUDCTRL_SaveAudioModeFlag( pcg_arg,0 );
        AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,sgTableIDChannelOfVoiceCallDev[voice_chnl].mic,sgTableIDChannelOfVoiceCallDev[voice_chnl].speaker);
        rtn = 1;

        DEBUG("AUD:PCG_IOCTL_SETMODE: pcg_arg=%d\n", pcg_arg);
    }
    break;

    case PCG_IOCTL_GETMODE:
    {
        UInt32 curmode;
        curmode = AUDCTRL_GetAudioMode();
        copy_to_user((UInt32 *)arg, &curmode, sizeof(UInt32));
        rtn=1;
        DEBUG("AUD:PCG_IOCTL_GETMODE curmode=%x  \n",curmode);
    }
    break;
    case PCG_IOCTL_GETVOL:
    {
        UInt32 volume;
        volume = AUDCTRL_GetTelephonySpkrVolume();
        copy_to_user( (UInt32 *)arg, &volume, sizeof(UInt32));
        rtn = 1;
        DEBUG("AUD:PCG_IOCTL_GETVOL volume=%x  \n",volume);
    }
    break;
    case PCG_IOCTL_SETVOL:
    {
        UInt32 spkvol;
        copy_from_user(&spkvol, (UInt32 *)arg, sizeof(UInt32));

        AUDCTRL_SetPlayVolume(AUDIO_HW_VOICE_OUT,AUDCTRL_SPK_HANDSET,AUDIO_GAIN_FORMAT_HW_REG,spkvol,spkvol);
        
        rtn=1;
        DEBUG("AUD:  PCG_IOCTL_SETVOL   spkvol=%d \r\n", spkvol);
    }
    break;
    case PCG_IOCTL_SetExtPAVol:
	{
		UInt32 extvol;
		copy_from_user(&extvol, (UInt32 *)arg, sizeof(UInt32));
	
		DEBUG("AUD:  PCG_IOCTL_SetExtPAVol	extvol=%d  \r\n", extvol);

        setExternalAmpGain(extvol);

        rtn = 1;

	}
	break;
    case PCG_IOCTL_GETLPBKSTATUS:
    {
        UInt32 lb_status;
        lb_status = loopback_status;
        copy_to_user( (UInt32 *)arg, &lb_status, sizeof(UInt32));
        rtn = 1;
        DEBUG("AUD:PCG_IOCTL_GETLPBKSTATUS lb_status=%x  \n",lb_status);

    }
    break;
    case PCG_IOCTL_SETLPBK:
    {
        UInt32 lb_status;
        copy_from_user(&lb_status, (UInt32 *)arg, sizeof(UInt32));

        if(lb_status >1)
            lb_status = 1;

        loopback_status = lb_status;

        AUDCTRL_SetAudioLoopback(loopback_status,loopback_input,loopback_output);

        rtn = 1;
        DEBUG("AUD:PCG_IOCTL_SETLPBK lb_status=%x  \n",lb_status);

    }
    break;
    case PCG_IOCTL_GETLPBKPATH:
    {
//        UInt32 curmode;
        UInt32 lb_output;
        UInt32 lbpaths[2];
        lb_output = AUDCTRL_GetAudioMode();

        if(lb_output > 2)
            lb_output = 2;
        lbpaths[0] = loopback_input;
        lbpaths[1] = lb_output;

        copy_to_user( (UInt32 *)arg, lbpaths, 2* sizeof(UInt32));
    
        rtn = 1;
        DEBUG("AUD:PCG_IOCTL_GETLPBKPATH lb_output=%d loopback_input=%d  \n",lb_output,loopback_input);

    }
    break;
    case PCG_IOCTL_SETLPBKPATH:
    {
//        UInt32 lb_output;
        UInt32 lbpaths[2];

        copy_from_user(lbpaths, (UInt32 *)arg, 2* sizeof(UInt32));
    

        if(lbpaths[1] == 2)
            lbpaths[1] = 4;
         loopback_input = lbpaths[0];
         loopback_output = lbpaths[1];

        AUDCTRL_SetAudioLoopback(1,loopback_input,loopback_output);
        rtn = 1;
        DEBUG("AUD:PCG_IOCTL_SETLPBKPATH loopback_output=%d loopback_input=%d  \n",loopback_output,loopback_input);

    }
    break;
    case PCG_IOCTL_MAUDTST:
    {
        // Start voip loopback - 123,mic,speaker,delay
        // stoop voip loopback - 99
	 //   int i;
	    arg_t voip;
        UInt16 voip_audio_mode;
        copy_from_user(&voip,(const void*)arg, sizeof(voip));

        DEBUG("AUD:PCG_IOCTL_MAUDTST VOIP val1 =%d val2=%d val3=%d val4=%d val5=%d val6=%d  \n",voip.val1,voip.val2,voip.val3,voip.val4,voip.val5,voip.val6);
	
	    switch (voip.val1) 
  	    {
           case 100:
            {
                UInt32 delay, delay_in_frames;
                DEBUG("Start loopback \n");
                //lp_voip_start = 1; /* 20110715 for loopback check */
                voip_loopbackbuffer = (UInt8*) OSDAL_ALLOCHEAPMEM(LB_VOIP_BUFFER_SIZE);

                if(!voip_loopbackbuffer)
                    return 0;

                memset(voip_loopbackbuffer,0,LB_VOIP_BUFFER_SIZE);
            
                drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_VOIP);
                if ( !drv_handle )
                    return 0;

                DEBUG(" Audio DDRIVER Open Complete\n");

                DEBUG(" Audio VOIP DDRIVER Config\n");
                //set UL callback
                AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_VOIP_UL_CB,(void*)LB_VOIP_DumpUL_CB);

                //set the callback
                AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_VOIP_DL_CB,(void*)LB_VOIP_FillDL_CB);

                // save the audio mode
                switch (voip.val3)
                {
                    case AUDIO_MODE_HANDSET:
#ifdef TTY_ENABLED
                        voip_audio_mode = voip.val3;
#else
                        voip_audio_mode = AUDIO_MODE_TTY;
#endif
                        break;
                    case AUDIO_MODE_HEADSET:
                        voip_audio_mode = AUDIO_MODE_HAC;
                        break;
                    case AUDIO_MODE_BLUETOOTH:
                    case AUDIO_MODE_HANDSFREE: // BT NREC
                        voip_audio_mode = AUDIO_MODE_USB;
                        break;
                    case AUDIO_MODE_SPEAKERPHONE:
                        voip_audio_mode = AUDIO_MODE_RESERVE;
                        break;
                    default :
                        voip_audio_mode = voip.val3;
                        break;
                }
                AUDCTRL_SaveAudioModeFlag(voip_audio_mode, AUDIO_APP_VOICE_CALL);

                // set the speaker and mic
                lp_voip_speaker = voip.val3;
                lp_voip_mic = voip.val2;
                delay = voip.val4;

                delay_in_frames = (voip.val4/20); //20ms frame size

                // delay can't be more than half of the buffer
                if(delay_in_frames >= LB_VOIP_FRAMES /2)
                    delay_in_frames = LB_VOIP_FRAMES /2;

                // increment write pointer by delay
                VOIP_buf_ul_index = delay_in_frames * 320;
                
                // disable NS to pass Samsung's loopback test case.
                if(voip.val3 == AUDIO_MODE_HEADSET)
                {
                    AUDCTRL_NS(FALSE); // set FALSE before AUDCTRL_EnableTelephony() calls AUDDRV_Telephony_Init().
                }

                AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,lp_voip_mic,lp_voip_speaker);

                // disable NLP due to volume fluctuation for loopback case only, ported from BCM215x / BCM2133x platform.
                // reset EC/NLP after AUDCTRL_EnableTelephony() calls AUDDRV_Telephony_Init().
                if(voip.val3 == AUDIO_MODE_HEADSET)
                {
                    AUDCTRL_ECreset_NLPoff(TRUE);
                }

                AUDCTRL_SetTelephonySpkrVolume(AUDIO_HW_VOICE_OUT, lp_voip_speaker, AUDIO_VOLUME_DEFAULT, AUDIO_GAIN_FORMAT_VOL_LEVEL);
                AUDCTRL_SetTelephonyMicGain(AUDIO_HW_VOICE_IN, lp_voip_mic, AUDIO_MICGAIN_DEFAULT);
                AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_START,NULL);

                if(voip.val3 == AUDIO_MODE_SPEAKERPHONE)	
                    AUDDRV_GPIO_Start();
                else
                    AUDDRV_GPIO_Stop();

            }
            return 1;

           case 99:
           {
                DEBUG("Stop  loopback\n");
                //lp_voip_start = 0; /* 20110715 for loopback check */
                AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_STOP,NULL);
                AUDCTRL_DisableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,lp_voip_mic,lp_voip_speaker);

                // reactivate NS to refer to sysparm.
                AUDCTRL_NS(TRUE); // set TRUE for the next call for AUDCTRL_EnableTelephony().

                AUDIO_DRIVER_Close(drv_handle);
                
                if(voip_loopbackbuffer != NULL)
                {
                    OSDAL_FREEHEAPMEM( voip_loopbackbuffer );
                    voip_loopbackbuffer = NULL;
                }
                
                VOIP_buf_dl_index = 0;  /* 20110819 fix loopback delay issue */
                drv_handle = NULL;

                AUDDRV_GPIO_Stop();
            }
            return 1; 
            
            case 118:  /* 20110804 external amp off for fm radio volume 0 */
            {
                UInt32 amp_onoff;
                UInt32 amp_path;
                //UInt32 fm_radio_vol;                
                
                amp_onoff = voip.val2;
                amp_path = AUDCTRL_GetAudioMode();
                //if(amp_onoff == 0) fm_radio_vol = AUDIO_VOLUME_MIN;
                //else fm_radio_vol = AUDIO_VOLUME_MAX;

                DEBUG("AUD:  setExtAmp onoff = [%d], amp_path = [%d] \r\n", amp_onoff, amp_path);

                /* 20110822 fm radio volume 0 for mixing with ringtone 
                AUDCTRL_SetPlayVolume(AUDIO_HW_AUDIO_OUT,
                    amp_path,
                    AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE,
                    fm_radio_vol,
                    fm_radio_vol 
                ); */

                OSTASK_Sleep(300);

                powerOnExternalAmp( amp_path, AudioUseExtSpkr, amp_onoff );
            }
            return 1;            
            
            case 121:
            {
                
                UInt32 ext_preamp_gain;
                UInt32 amp_id;
                
                amp_id = voip.val2;
                ext_preamp_gain = voip.val3;
                if(amp_id == 1)
                {
                    DEBUG("AUD:  SetPreamp gain	ext_preamp_gain=%d  \r\n", ext_preamp_gain);
                    setExternalPreAmpGain(ext_preamp_gain);

                }
                
            }
            return 1; 
            
            case 29000:
            {
                DEBUG("VoIF Test \n");
                AUDTST_VOIF(voip.val2, voip.val3, voip.val4, voip.val5, voip.val6);
                
            }
            return 1; 
   
           default:
               DEBUG("Invalid ioctl: '0x%x'  \n", voip.val1);
            return 0; 
  	    }
    }
    break;
    case PCG_IOCTL_SETMODEAPP:
    {
        UInt32 pcg_arg[2];
        UInt32 mode,app,voice_chnl,cur_mode,cur_channel;
        copy_from_user(&pcg_arg, (UInt32 *)arg, 2 * sizeof(UInt32));

        cur_mode = AUDCTRL_GetAudioMode();
        cur_channel = GetVoiceChannelId(cur_mode);
        AUDCTRL_DisableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,sgTableIDChannelOfVoiceCallDev[cur_channel].mic,sgTableIDChannelOfVoiceCallDev[cur_channel].speaker);
                
        mode = pcg_arg[0];
        app= pcg_arg[1];

        voice_chnl = GetVoiceChannelId(pcg_arg[0]);
        if(app == AUDIO_APP_VOICE_CALL_WB)
        {
            mode = pcg_arg[0] + AUDIO_MODE_NUMBER;
        }
       
        if((app == AUDIO_APP_VOICE_CALL) || (app == AUDIO_APP_VOICE_CALL_WB))
        {
            DEBUG("Enable Telephony\n");
            AUDCTRL_SaveAudioModeFlag( mode,app);
            AUDCTRL_RateChangeTelephony();
            AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,sgTableIDChannelOfVoiceCallDev[voice_chnl].mic,sgTableIDChannelOfVoiceCallDev[voice_chnl].speaker);
        }
        else
        {
            AUDCTRL_SetAudioMode(mode, app);
        }
        rtn = 1;
        DEBUG("AUD:PCG_IOCTL_SETMODEAPP: mode=%d app=%d\n", pcg_arg[0],pcg_arg[1]);
    }
    break;

    case PCG_IOCTL_GETMODEAPP:
    {
        UInt32 pcg_arg[2];
//        UInt32 curmode;
        
        pcg_arg[0] = AUDCTRL_GetAudioMode();
        pcg_arg[1]= AUDCTRL_GetAudioApp();
        
        if ( pcg_arg[0] >= AUDIO_MODE_NUMBER )
            pcg_arg[0] = (AudioMode_t) (pcg_arg[0] - AUDIO_MODE_NUMBER);

        copy_to_user((UInt32 *)arg, pcg_arg, 2* sizeof(UInt32));
        rtn=1;
        DEBUG("AUD:PCG_IOCTL_GETMODEAPP mode=%d app=%d  \n",pcg_arg[0],pcg_arg[1]);
    }
    break;
    default:
        DEBUG("Unrecognized ioctl: '0x%x'  \n", cmd);
        return -ENOTTY;
    }

    return (rtn);

} 

static Boolean LB_VOIP_DumpUL_CB(
AUDIO_DRIVER_HANDLE_t drv_handle,
UInt8		*pSrc,		// pointer to start of speech data
UInt32		amrMode		// AMR codec mode of speech data
)
{
    //DEBUG(" VOIP_DumpUL_CB : VOIP_buf_ul_index 0x%x, \r\n", VOIP_buf_ul_index);

    memcpy(&voip_loopbackbuffer[VOIP_buf_ul_index], pSrc, VOIP_FRAME_SIZE);

    VOIP_buf_ul_index += VOIP_FRAME_SIZE;

    if ( VOIP_buf_ul_index >= VOIP_BUFFER_SIZE )
    {
        VOIP_buf_ul_index = 0;
    }
    return TRUE;
}

static Boolean LB_VOIP_FillDL_CB(AUDIO_DRIVER_HANDLE_t drv_handle, UInt8 *pDst, UInt32 nFrames)
{
    //DEBUG(" VOIP_FillDL_CB :  VOIP_buf_dl_index 0x%x \r\n", VOIP_buf_dl_index);
    memcpy(pDst, &voip_loopbackbuffer[VOIP_buf_dl_index], VOIP_FRAME_SIZE);
    VOIP_buf_dl_index += VOIP_FRAME_SIZE;
    if ( VOIP_buf_dl_index >= VOIP_BUFFER_SIZE )
    {
        VOIP_buf_dl_index = 0;
    }
    return TRUE;
}

static int voifDelay = 0;
static int voifGain = 0x4000;
static void VOIF_Callback (Int16 *ulData, Int16 *dlData, UInt32 sampleCount, UInt8 isCall16K)
{
	if (voifDelay == 0)
	{
		// copy the dlData to dlData without delay.
		memcpy(dlData, ulData, sampleCount * sizeof(Int16));
	}
	else if (voifDelay == 1)
	{
		Int32 t, i;
		for( i=0; i<sampleCount; i++ )
		{
			t = (Int32) *dlData;
			t = (t * voifGain)>>14;
			*dlData++ = (Int16)(t&0xffff);
		}
	}
	else
	{
		// delay test, mute the downlink
		memset (dlData, 0, sampleCount * sizeof(Int16)); 
	}
	if(voifDelay > 1)
        mdelay(voifDelay);
    //OSTASK_Sleep(voifDelay);
}

static void 
AUDTST_VOIF (UInt32 Val2,UInt32 Val3, UInt32 Val4, UInt32 Val5, UInt32 Val6)
{
  
  if (Val2 == 1) 
  {
    DEBUG((UInt8 *)"\n\n\r\t*** Start VOIF test ***\n\r");	
	voifDelay = Val3; // the delay in ms.
	AUDDRV_VOIF_Start (VOIF_Callback);
  } 
  else if (Val2 == 0)
  { 
    DEBUG((UInt8 *)"\n\n\r\t*** Stop VOIF test ***\n\r");	  
    AUDDRV_VOIF_Stop ();     
  }
  else if (Val2 == 2)
  {
	DEBUG((UInt8 *)"\n\n\r\t*** Change VOIF delay ***\n\r");	
	voifDelay = Val3; // the delay in ms.
  }
  else if (Val2 == 3)
  {
	  DEBUG((UInt8 *)"\n\n\r\t*** Change VOIF downlink gain ***\n\r");
	  voifGain = Val3; // the downlink gain
  }

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DeviceRelease
//
//  Description: 'release' call back function for platform device
//
//------------------------------------------------------------
static void DeviceRelease(struct device *pdev)
{
	DEBUG("\n TO DO:what am i supposed to do\n");	
}

//Platform device structure
static struct platform_device sgPlatformDevice =
{
	.name		= "brcm_alsa_device",
	.dev		= 
	{
		.platform_data	= &sgDeviceInfo,
		.release = DeviceRelease,
	},
	.id		= -1,
};

//Platfoorm driver structure
static struct platform_driver sgPlatformDriver =
{
	.probe		= DriverProbe,
	.remove 	= DriverRemove,
	.suspend	= DriverSuspend,
	.resume		= DriverResume,
	.driver		= 
		{
		.name	= "brcm_alsa_device",
		.owner	= THIS_MODULE,
		},
};

static struct file_operations bcmpcg_fops =
{
    .owner=  THIS_MODULE,
    .open =   BCMPCG_open,
    .ioctl  =   BCMPCG_ioctl,
    .mmap = NULL,
    .release = BCMPCG_release,
};

static struct file_operations bcmvoip_fops =
{
    .owner=  THIS_MODULE,
    .open =  BCMVOIP_open,
    .ioctl=  BCMVOIP_ioctl,
    .write=  BCMVOIP_write,
    .read =  BCMVOIP_read,
    .mmap =  NULL,
    .release = BCMVOIP_release,

};
static struct class *audvoip_class;

static struct class *aud_class;
static struct class *audlog_class;

//--------------------------------------------------------------------
// File operations for audio logging
static struct file_operations bcmlog_fops =
{
    .owner=    THIS_MODULE,
    .open =    BCMLOG_open,
    .read =    BCMLOG_read,
    .ioctl  =   BCMLOG_ioctl,
    .mmap =    BCMLOG_mmap,
    .release =         BCMLOG_release,
};

//--------------------------------------------------------------------


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: ModuleInit
//
//  Description: Module initialization
//
//------------------------------------------------------------
static int __devinit ModuleInit(void)
{
	int err = 1;
	
	DEBUG("\n %lx:ModuleInit debg=%d id=%s\n",jiffies,debug,id);

    if(get_ap_boot_mode() == AP_ONLY_BOOT)
        return 0;
	
	err =  platform_device_register(&sgPlatformDevice);
	DEBUG("\n %lx:device register done %d\n",jiffies,err);
	if (err)
		return err;
		
	err = platform_driver_register(&sgPlatformDriver);
	DEBUG("\n %lx:driver register done %d\n",jiffies,err);

    LaunchAudioCtrlThread();


    if ((err = register_chrdev(BCM_ALSA_PCG_MAJOR, "bcm_alsa_pcg", &bcmpcg_fops)) < 0)
    {
        return err;
    }

    aud_class = class_create(THIS_MODULE, "bcm_alsa_pcg");
    if (IS_ERR(aud_class))
    {
        return PTR_ERR(aud_class);
    }
    device_create(aud_class, NULL, MKDEV(BCM_ALSA_PCG_MAJOR, 0),NULL, "bcm_alsa_pcg");

    // Device for VOIP
    if ((err = register_chrdev(BCM_ALSA_VOIP_MAJOR, "bcm_alsa_voip", &bcmvoip_fops)) < 0)
    {
        return err;
    }

    audvoip_class = class_create(THIS_MODULE, "bcm_alsa_voip");
    if (IS_ERR(audvoip_class))
    {
        return PTR_ERR(audvoip_class);
    }
    device_create(audvoip_class, NULL, MKDEV(BCM_ALSA_VOIP_MAJOR, 0),NULL, "bcm_alsa_voip");

    // Device for audio logging

    if ((err = register_chrdev(BCM_ALSA_LOG_MAJOR, "bcm_audio_log", &bcmlog_fops)) < 0)
    {
        return err;
    }  
    audlog_class = class_create(THIS_MODULE, "bcm_audio_log");
    if (IS_ERR(audlog_class))
    {
        return PTR_ERR(audlog_class);
    }

    device_create(audlog_class, NULL, MKDEV(BCM_ALSA_LOG_MAJOR, 0),NULL, "bcm_audio_log");
    init_waitqueue_head(&bcmlogreadq);

	// Initialize the AUDIO device
	
	return err;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: ModuleExit
//
//  Description: Module de-initialization
//
//------------------------------------------------------------
static void __devexit ModuleExit(void)
{
	//int err;
	
	DEBUG("\n %lx:ModuleExit\n",jiffies);

    if(get_ap_boot_mode() == AP_ONLY_BOOT)
        return;

	snd_card_free(g_brcm_alsa_chip->card);
	
	platform_driver_unregister(&sgPlatformDriver);
	platform_device_unregister(&sgPlatformDevice);

    TerminateAudioHalThread();
	
	// unInitialize the AUDIO device
	DEBUG("\n %lx:exit done \n",jiffies);
}

module_init(ModuleInit);
module_exit(ModuleExit);
