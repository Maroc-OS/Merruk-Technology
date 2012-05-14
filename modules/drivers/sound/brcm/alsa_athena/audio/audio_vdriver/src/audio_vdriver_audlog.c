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
*   @file   audio_vdriver_audlog.c
*
*   @brief  
*
****************************************************************************/

#include <linux/module.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

#include "brcm_alsa.h"
#include "shared.h"
#include "auddrv_audlog.h"
#include "log_sig_code.h"
#include "log.h"
/**
*
* @addtogroup AudioDriverGroup
* @{
*/


//
// local defines
//

#define _DBG_(a)	(a)

//#define	DBG_MSG_TO_FILE
#undef	DBG_MSG_TO_FILE

//----------------------------------------------------------------------------------------------
// Audio logging declaration, initializations

//#define DEBUG_AUD_LOG_FEATURE
extern int logging_link[LOG_STREAM_NUMBER];
wait_queue_head_t bcmlogreadq;
extern int audio_data_arrived;

//#define DSP_DATA (0xa40003)		//(163<<16|3) //(155<<16|3)

static UInt16 audio_log_inited = 0;

#if 0
struct process_dev_channel_struct {

        UInt16 msg_status;
        UInt16 msg_val00;
        UInt16 msg_val01;
        UInt16 msg_val02;
};
struct process_dev_channel_struct p_dev_channel_task;
void process_Log_Channel_next (struct process_dev_channel_struct *p_dev_ch_s);

// TASKLET FOR AUDIO DUMP
DECLARE_TASKLET(tasklet_audio_logging, AUDLOG_ProcessLogChannel,(unsigned long )&p_dev_channel_task);
#endif

#ifdef DEBUG_AUD_LOG_FEATURE
UInt16 *pData = NULL;
#endif
void   *bcmlog_stream_ptr= NULL;
int    *bcmlog_stream_area = NULL;

typedef enum {
	AUDDRV_LOG_STATE_UNDEFEIND,
	AUDDRV_LOG_STATE_INITED,
	AUDDRV_LOG_STATE_STARTED,
	AUDDRV_LOG_STATE_CLOSED
} AUDDRV_LOG_STATE_en_t;

static AUDLOG_INFO sLogInfo;
static UInt16 auddrv_log_state = AUDDRV_LOG_STATE_CLOSED;

//static LOG_FRAME_t sLogFrame;
//static LOG_FRAME_t sLogFrameBuffer[LOG_BUFFER_NUMBER];
static int sAudioLogTaskReadyFlag = 0;
//static int sCurrentFileSize;

//
// APIs
//


//Initialize driver internal variables and task queue only needed to save to flash file. 
Int16 AUDDRV_AudLog_Init( void )
{
	Int16 n;

	if ( (sLogInfo.log_consumer[0] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[1] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[2] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[3] == LOG_TO_FLASH)
		&& (audio_log_inited==0)
		) 
	{
	 // This is for FFS option, create buffer in heap, copy data from shared memory to it and mmap it to user space
        // reserve memory with kmalloc - Allocating Memory in the Kernel
        //kmalloc_ptr = NULL;
        // Max we need LOG_FRAME_SIZE*4 so PAGESIZE+(LOG_FRAME_SIZE*4) because kmalloc_area should fall within PAGE BOUNDRY

        bcmlog_stream_ptr = NULL;
        bcmlog_stream_ptr = kmalloc(PAGE_SIZE+( (sizeof(LOG_FRAME_t)) *4 ), GFP_KERNEL); //  4k+1024 , page_size is linux page size , 4K by default
        

        if (bcmlog_stream_ptr == NULL)
        {
            pr_info("kmalloc failed\n");
            return -1;
        }

        // Make sure page boundry
        bcmlog_stream_area = (int*)(((unsigned long) bcmlog_stream_ptr + PAGE_SIZE -1) & PAGE_MASK);
        if (bcmlog_stream_area == NULL)
        {
            pr_info("Couldn't get proper page boundry , may be issue for page swapping to user space \n");
            return -1;
        }
		//printk(KERN_ALERT "Setup bcmlog_stream_area = %x\n",bcmlog_stream_area);

        for (n = 0; n < (2*PAGE_SIZE); n+=PAGE_SIZE)            {
            // reserve all pages to make them remapable
            SetPageReserved(virt_to_page( ((unsigned long) bcmlog_stream_area)+n));
        }

	}		

	audio_log_inited = 1;

	if ( auddrv_log_state != AUDDRV_LOG_STATE_INITED && auddrv_log_state != AUDDRV_LOG_STATE_STARTED )
	{
		auddrv_log_state = AUDDRV_LOG_STATE_INITED;
		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_AudLog_Init Done\n");
	}
	return 0;
}


// ISR to handle STATUS_AUDIO_STREAM_DATA_READY from DSP
// called from AP AP_Audio_ISR_Handler (status = STATUS_AUDIO_STREAM_DATA_READY
void AUDLOG_ProcessLogChannel( StatQ_t * msg)
{
	Int16 n;

	UInt16	audio_stream_buffer_idx;
	UInt16 size;    //number of 16-bit words
	UInt16 stream=0;  //the stream number: 1, 2, 3, 4
	UInt16 sender=0;  //the capture point
	Audio_Logging_Buf_t* ptr=NULL;
	LOG_FRAME_t *log_cb_info_ks_ptr = NULL;
	UInt16 samplerate=0;	//To indicate 8kHz or 16kHz samplerate

	AP_SharedMem_t *smp = SHAREDMEM_GetDsp_SharedMemPtr();

	//Log_DebugPrintf(LOGID_AUDIO,"Audlog DISR handler msg->arg0 = 0x%x, msg->arg1=0x%x, msg->arg2=0x%x \r\n",msg->arg0, msg->arg1, msg->arg2 );

	audio_stream_buffer_idx	= msg->arg2; //ping pong buffer index, 0 or 1

	if ( sLogInfo.log_consumer[0] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[1] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[2] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[3] == LOG_TO_FLASH
		) 
        {
        	if (bcmlog_stream_ptr != NULL)
            {
            	// memset mmap buffer to clean data from last interrupt
                memset(bcmlog_stream_area, 0, (sizeof(LOG_FRAME_t) *4));
                log_cb_info_ks_ptr = (LOG_FRAME_t *) bcmlog_stream_area;
            }
        }

	for(n = 0; n < LOG_STREAM_NUMBER; n++)
	{
		if(n == 0 )
		{
			stream	= 1;
			sender	= smp->shared_audio_stream_0_crtl;
			ptr = (Audio_Logging_Buf_t *)&smp->shared_audio_stream_0[audio_stream_buffer_idx].param[0];
			samplerate = smp->shared_audio_stream_0[audio_stream_buffer_idx].rate;
		}

		if(n == 1 )
		{
			stream	= 2;
			sender	= smp->shared_audio_stream_1_crtl;
			ptr = (Audio_Logging_Buf_t *)&smp->shared_audio_stream_1[audio_stream_buffer_idx].param[0];
			samplerate = smp->shared_audio_stream_1[audio_stream_buffer_idx].rate;
		}

		if(n == 2 )
		{
			stream	= 3;
			sender	= smp->shared_audio_stream_2_crtl;
			ptr = (Audio_Logging_Buf_t *)&smp->shared_audio_stream_2[audio_stream_buffer_idx].param[0];
			samplerate = smp->shared_audio_stream_2[audio_stream_buffer_idx].rate;
		}

		if(n == 3 )
		{
			stream	= 4;
			sender	= smp->shared_audio_stream_3_crtl;
			ptr = (Audio_Logging_Buf_t *)&smp->shared_audio_stream_3[audio_stream_buffer_idx].param[0];
			samplerate = smp->shared_audio_stream_3[audio_stream_buffer_idx].rate;

		}

		//check the SHmem ctrl point.
		if ( sender != 0 )
		{
			// 0x101 and 0x201 are only compressed frame cases
			if ( sender== 0x101 || sender== 0x201 )
			{
				// Compressed frame captured
				size = LOG_FRAME_SIZE; //162;
			}
			else
			{	//Sample rate is 16kHz
				if(samplerate == 0x3E80)
				{// PCM frame
					size = 642;
				}
				//Sample rate is 8kHz
				else
				{
					size = 320;
				}
			}

			if(sLogInfo.log_consumer[n] == LOG_TO_PC)
			{
				//Log_DebugPrintf(LOGID_AUDIO, "AUDLOG: 0x%x addr=0x%x size=%d stream=%d sender=%d", DSP_DATA, ptr, size, stream, sender);

				if(ptr){
					BCMLOG_LogSignal(DSP_DATA, (UInt16 *)ptr, size, stream, sender);	// send binary data to log port. The size is number of bytes (for MTT).
				}
				else{
					Log_DebugPrintf(LOGID_AUDIO, "!!!!!! Err ptr = 0x%x size=%d stream=%d sender=%d", ptr, size, stream, sender);
				}
			}
			else
			if (bcmlog_stream_ptr != NULL)
                   	{
			
                        memcpy(log_cb_info_ks_ptr[n].log_msg,(UInt16 *)ptr, size/2); // copy 81 bytes of data
                        log_cb_info_ks_ptr[n].log_capture_control = sender;
                        log_cb_info_ks_ptr[n].stream_index = stream;
			   //Log_DebugPrintf(LOGID_AUDIO, "AUDLOG: 0x%x addr=0x%x size=%d stream=%d sender=%d", DSP_DATA, ptr, size, stream, sender);	
			}
		}
	}

	if ( sLogInfo.log_consumer[0] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[1] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[2] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[3] == LOG_TO_FLASH
		) 
        {
        	if (bcmlog_stream_ptr != NULL)
        	{
	        	// Wakeup read in user space to go ahead and do mmap buffer read
	            audio_data_arrived = 1;
    	        wake_up_interruptible(&bcmlogreadq);
        	}	
    
        }

	//Log_DebugPrintf(LOGID_AUDIO, "<=== process_Log_Channel <===\r\n"));
}

// clean up audio log task and queue
Int16 AUDDRV_AudLog_Shutdown( void )
{    
	 UInt16 *local_bcmlogptr = bcmlog_stream_ptr;
	 // Use local pointer to free so it wouldnt disturb ISR activities
        bcmlog_stream_ptr = NULL;
	//printk(KERN_ALERT "AUDDRV_AudLog_Shutdown\n");
	if ( sLogInfo.log_consumer[0] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[1] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[2] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[3] == LOG_TO_FLASH
	   )
	{ 
    	   if (local_bcmlogptr != NULL)
    	   	kfree(local_bcmlogptr);
    	}   		
	// flag to stop writting logging message
	sAudioLogTaskReadyFlag = 0; 	// enable task 
	auddrv_log_state = AUDDRV_LOG_STATE_CLOSED;
	audio_log_inited = 0;
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDDRV_AudLog_Start
//!
//! Description:	Start the audio logging (at*maudlog=1,stream,channel cmd)
//!					Driver sets up shared memory control
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
Int16 AUDDRV_AudLog_Start 	(   	UInt32 log_stream,
				  	UInt32 log_capture_point,
				  	AUDLOG_DEST_en_t log_consumer
				)
{
	Int16 res = 0;
	AP_SharedMem_t *smp = SHAREDMEM_GetDsp_SharedMemPtr();

	// check the capture point number is in reasonable range
	if((log_capture_point <= 0) || (log_capture_point > 0x8000))
		return -1;

	// set up logging message consumer	
	sLogInfo.log_consumer[log_stream - 1] = log_consumer;
       //printk(KERN_ALERT "Setup Log msg consumer = %d\n",log_consumer);
	//call init to check if need to open file and create task
	AUDDRV_AudLog_Init( );
	auddrv_log_state = AUDDRV_LOG_STATE_STARTED;
	
	// check the stream number is between 1 and 4
	// start the stream logging captrue
	switch (log_stream)
	{
	case 1:
		smp->shared_audio_stream_0_crtl = log_capture_point;
		break;
	case 2:
		smp->shared_audio_stream_1_crtl = log_capture_point;
		break;
	case 3:
		smp->shared_audio_stream_2_crtl = log_capture_point;
		break;
	case 4:
		smp->shared_audio_stream_3_crtl = log_capture_point;
		break;
	default:
		res = -1;
		break;
	}

	//DEBUG( "===> Start_Log stream %d ===>\r\n", log_stream);
	
	return res;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDDRV_AudLog_Stop
//!
//! Description:	Stop the audio logging (at*maudlog=2 cmd)
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
Int16 AUDDRV_AudLog_Stop( UInt32 log_stream )
{
	AP_SharedMem_t *smp = SHAREDMEM_GetDsp_SharedMemPtr();

	switch (log_stream)
	{
	case 1:
		smp->shared_audio_stream_0_crtl = 0;
		sLogInfo.log_consumer[0] = LOG_TO_PC; //default to Not writing to file
		break;

	case 2:
		smp->shared_audio_stream_1_crtl = 0;
		sLogInfo.log_consumer[1] = LOG_TO_PC; //default to Not writing to file
		break;

	case 3:
		smp->shared_audio_stream_2_crtl = 0;
		sLogInfo.log_consumer[2] = LOG_TO_PC; //default to Not writing to file
		break;

	case 4:
		smp->shared_audio_stream_3_crtl = 0;
		sLogInfo.log_consumer[3] = LOG_TO_PC; //default to Not writing to file
		break;

	default:
		break;
	}

	DEBUG ( "<=== Stop_Log stream %d <===", log_stream);

	if ( smp->shared_audio_stream_0_crtl == 0
		&& smp->shared_audio_stream_1_crtl == 0
		&& smp->shared_audio_stream_2_crtl == 0
		&& smp->shared_audio_stream_3_crtl == 0
		)
	{
		AUDDRV_AudLog_Shutdown();
	}

	return 0;
}


