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

#ifdef VPU_INCLUDED
//#define USE_HR_TIMER
#include "mobcom_types.h"
#ifndef UNDER_LINUX
#include "assert.h"
#endif
#include "msconsts.h"
#include <linux/broadcom/chip_version.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include "brcm_alsa.h"

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>

#include "ossemaphore.h"
#include "sharedmem.h"
#include "ripisr.h"
#include "vpu.h"

//#include "vpripcmdq.h"
#include "csl_apcmd.h"
//#include "ostypes.h"
#include "log.h"

#include "audio_consts.h"
#include "ostask.h"
#include <linux/broadcom/gpt.h>
#ifdef USE_HR_TIMER
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#define MS_TO_NS(x) (x * 1E6L)
static struct hrtimer hr_timer;
static ktime_t ktime;
//enum hrtimer_restart hrtimer_callback( struct hrtimer *timer );
#endif

#define VT_USE_GPT_INDEX 4

static VPUFillFramesCB_t	Telephony_FillVPFramesCB;

// cb function pointers for telephony application
static VPUDumpFramesCB_t	Telephony_DumpVPFramesCB;

static void VoIP_ProcessStatusMainAMRDone(StatQ_t status_msg);

static struct work_struct intr_work;
static struct workqueue_struct *intr_workqueue = NULL;

// VoIP related APIs

#if defined(ATHENA_INCLUDE_VOIP)

void VoIP_Task_Entry(struct work_struct *work)
{
    // fill DL data, 1 frame (20ms per dsp interface).
    if (Telephony_FillVPFramesCB)
        Telephony_FillVPFramesCB(NULL, 1);
}

#ifdef USE_HR_TIMER
static enum hrtimer_restart hrtimer_callback( struct hrtimer *timer )
{
    if ( intr_workqueue )
    {
        hrtimer_forward_now(timer, ktime);
        queue_work(intr_workqueue, &intr_work);
        Log_DebugPrintf(LOGID_AUDIO," hrtimer_callback \r\n");
        return HRTIMER_RESTART;
    }
    printk(" hrtimer_callback timer not started again\r\n");
    return HRTIMER_NORESTART;
}
#else
static int gptimer_callback(void *dev_id)
{
    if ( intr_workqueue )
        queue_work(intr_workqueue, &intr_work);
    return 0;
}

#endif

//******************************************************************************
//
// Function Name:  VoIP_StartTelephony()
//
// Description:	This function starts full duplex telephony session
//
// Notes:	The full duplex DSP interface is in sharedmem, not vsharedmem.
//		But since its function is closely related to voice processing,
//		we put it here.
//
//******************************************************************************
Boolean VoIP_StartTelephony(
VPUDumpFramesCB_t telephony_dump_cb,
VPUDumpFramesCB_t telephony_fill_cb,
UInt16	 voip_codec_type,  	// codec mode for encoding the next speech frame
Boolean	       dtx_mode,	// Turn DTX on (TRUE) or off (FALSE): this is obsolete. contained in voip_codec_type
Boolean	     amr_if2_enable	// Select AMR IF1 (FALSE) or IF2 (TRUE) format: obsolete
)
{
#ifdef USE_HR_TIMER
    unsigned long delay_in_ms = 20L;

    intr_workqueue = create_workqueue("vt-gpt");
    if (!intr_workqueue)
    {
        return TRUE;
    }

    INIT_WORK(&intr_work, VoIP_Task_Entry);

    ktime = ktime_set( 0, (delay_in_ms*1000000) );

    hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );

    hr_timer.function = &hrtimer_callback;

    hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );

#else
    struct gpt_cfg gptimer_config;

    intr_workqueue = create_workqueue("vt-gpt");
    if (!intr_workqueue)
    {
        return TRUE;
    }

    INIT_WORK(&intr_work, VoIP_Task_Entry);

    if ( gpt_request(VT_USE_GPT_INDEX) != VT_USE_GPT_INDEX )
        return TRUE;

    /* Disable Pedestral mode */
    gptimer_config.gctrl = SW_PEDEMODE;
    /* Set the clock to 1 Mhz */
    gptimer_config.gclk = 1024*1024;
    /* Configure the timer mode oneshot/periodic */
    gptimer_config.gmode = GPT_MODE_PERIODIC;

    /* Configure the timer for the above configration */
    if ( gpt_config(VT_USE_GPT_INDEX, &gptimer_config, gptimer_callback, NULL) != GPT_SUCCESS )
    {
        gpt_free(VT_USE_GPT_INDEX);
        return TRUE;
    }

    /* The timer starts here */
    if ( gpt_start(VT_USE_GPT_INDEX, 1000*20) != GPT_SUCCESS )
    {
        gpt_free(VT_USE_GPT_INDEX);
        return TRUE;
    }
#endif
    Telephony_DumpVPFramesCB=telephony_dump_cb;
    Telephony_FillVPFramesCB=telephony_fill_cb;

    return FALSE;
}

//******************************************************************************
//
// Function Name:  VoIP_StartMainAMRDecodeEncode()
//
// Description:		This function passes the AMR frame to be decoded
//					from application to DSP and starts its decoding
//					as well as encoding of the next frame.
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
void VoIP_StartMainAMRDecodeEncode(
VP_Mode_AMR_t		decode_amr_mode,	// AMR mode for decoding the next speech frame
UInt8				*pBuf,		// buffer carrying the AMR speech data to be decoded
UInt16				length,		// number of bytes of the AMR speech data to be decoded
VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
Boolean				dtx_mode	// Turn DTX on (TRUE) or off (FALSE)
)
{

    // decode the next downlink AMR speech data from application
    SHAREDMEM_WriteDL_VoIP_Data((UInt16)decode_amr_mode, (UInt16 *)pBuf);

    // signal DSP to start AMR decoding and encoding
    //Log_DebugPrintf(LOGID_AUDIO, "=====VoIP_StartMainAMRDecodeEncode UL codecType=0x%x, send COMMAND_MAIN_AMR_RUN to DSP\r\n", encode_amr_mode);

    VPRIPCMDQ_DSP_AMR_RUN((UInt16)encode_amr_mode, 0, FALSE);
}

//******************************************************************************
//
// Function Name:  VPU_ProcessStatusMainAMRDone()
//
// Description:		This function handles the processing needed after ARM receives
//					STATUS_MAIN_AMR_DONE from DSP.
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
void VPU_ProcessStatusMainAMRDone(StatQ_t status_msg)
{
    VPSharedMem_t *vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());
    UInt16 codecType;

    codecType = vp_shared_mem->VOIP_UL_buf.voip_vocoder;

    //Log_DebugPrintf(LOGID_AUDIO, "=====AP: VPU_ProcessStatusMainAMRDone codecType=0x%x\r\n", codecType);

    if (((codecType & 0xf000) == VOIP_PCM)		// VoIP PCM
	|| ((codecType & 0xf000) == VOIP_FR)		// VoIP FR
	|| ((codecType & 0xf000) == VOIP_AMR475)	// VoIP AMR-NB
	|| ((codecType & 0xf000) ==  VOIP_G711_U))	// VoIP G.711
    {
	VoIP_ProcessStatusMainAMRDone(status_msg);
	return;
    }
}

//******************************************************************************
//
// Function Name:  VoIP_StopTelephony()
//
// Description:	This function stops full duplex telephony session
//
// Notes:	The full duplex DSP interface is in sharedmem, not vsharedmem.
//		But since its function is closely related to voice processing,
//		we put it here.
//
//******************************************************************************
Boolean VoIP_StopTelephony(void)
{
    int ret;

    Telephony_DumpVPFramesCB=NULL;
    Telephony_FillVPFramesCB=NULL;
    flush_workqueue(intr_workqueue);
    destroy_workqueue(intr_workqueue);

    intr_workqueue = NULL;

#ifdef USE_HR_TIMER
    OSTASK_Sleep( 40 );			// To make sure the timer is not running anymore
    ret = hrtimer_cancel( &hr_timer );
    if (ret) printk("The timer was still in use...\n");

     printk("HR Timer module uninstalling\n");

#else
    gpt_stop(VT_USE_GPT_INDEX);
    gpt_free(VT_USE_GPT_INDEX);
#endif

    return TRUE;
}

//******************************************************************************
//
// Function Name:  VoIP_ProcessStatusMainAMRDone()
//
// Description:		This function handles the processing needed after ARM receives
//					STATUS_MAIN_AMR_DONE from DSP.
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
static void VoIP_ProcessStatusMainAMRDone(StatQ_t status_msg)
{
    static UInt16 Buf[161]; // buffer to hold UL data and codec type

    //Log_DebugPrintf(LOGID_AUDIO, "=====AP: VoIP_ProcessStatusMainAMRDone status=0x%x 0x%x 0x%x 0x%x \r\n",
    //                status_msg.status, status_msg.arg0, status_msg.arg1, status_msg.arg2);

    // encoded uplink AMR speech data now ready in DSP shared memory, copy it to application
    // pBuf is to point the start of the encoded speech data buffer
    if (Telephony_DumpVPFramesCB)
    {
        SHAREDMEM_ReadUL_VoIP_Data(status_msg.arg0, Buf);
        Telephony_DumpVPFramesCB((UInt8*)Buf, 0);
    }
}
#endif //#if defined(ATHENA_INCLUDE_VOIP)
#endif //#ifdef VPU_INCLUDED
