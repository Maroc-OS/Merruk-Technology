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
*   @file   csl_vpu.c
*
*   @brief  This file contains CSL (DSP) API to VPU interface
*
****************************************************************************/
#include <string.h>
#include "assert.h"
#include "mobcom_types.h"
#include "shared.h"
#include "audio_consts.h"
#include "csl_vpu.h"
#include "csl_apcmd.h"

extern VPSharedMem_t	*vp_shared_mem;


//*********************************************************************
/**
*
*   CSL_VPU_Init initializes VPU interface.
*
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
* 
**********************************************************************/
void CSL_VPU_Init(CSL_VP_Speech_Mode_t speechMode, UInt16 numFramesPerInterrupt)
{
 VPlayBack_Buffer_t *pBuf;
 UInt8 i;

	// init the pingpong buffer 
	for(i = 0; i < 2; i++)
	{
		pBuf = &vp_shared_mem->shared_voice_buf.vp_buf[i];
		pBuf->nb_frame = numFramesPerInterrupt;
		pBuf->vp_speech_mode = speechMode;
		memset(&pBuf->vp_frame, 0, sizeof(pBuf->vp_frame));
	
	}


} // CSL_VPU_Init


//*********************************************************************
/**
*
*   CSL_VPU_ReadPCM reads PCM data from shared memory for VPU voice record.
* 
*   @param    outBuf		(out)	destination buffer
*   @param    outSize		(in)	data size to read
*   @param    bufIndex		(in)	index of ping-pong buffer
*   @param    speechMode	(in)	speech mode (codec and sample rate)
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_VPU_ReadPCM(UInt8* outBuf, UInt32 outSize, UInt16 bufIndex, CSL_VP_Speech_Mode_t speechMode)
{
	UInt32 frameSize;
	UInt8	*buffer = NULL;
	UInt8	i, frameCount = 0;
	UInt32	totalCopied = 0;
	
	if(speechMode == VP_SPEECH_MODE_LINEAR_PCM_16K)
		frameSize = WB_LIN_PCM_FRAME_SIZE*sizeof(UInt16);
	else
		frameSize = LIN_PCM_FRAME_SIZE*sizeof(UInt16);

	frameCount = vp_shared_mem->shared_voice_buf.vr_buf[bufIndex].nb_frame;	

	for (i = 0; i < frameCount; i++)
	{	
		buffer = (UInt8 *)(&vp_shared_mem->shared_voice_buf.vr_buf[bufIndex].vr_frame.vectors_lin_PCM[i].param);
		if (outSize - totalCopied >= frameSize)
		{
			// copy it from shared memeory 
			memcpy(outBuf, buffer, frameSize);
			outBuf += frameSize;
			totalCopied += frameSize; 

		}

	}

	return totalCopied;

} // CSL_VPU_ReadPCM



//*********************************************************************
/**
*
*   CSL_VPU_WritePCM writes PCM data to shared memory for VPU voice playback.
* 
*   @param    inBuf					(in)	source buffer
*   @param    inSize				(in)	data size to read
*   @param    writeIndex			(in)	index of ping-pong buffer
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
*   @return   UInt32				number of bytes written from the buffer
*
**********************************************************************/
UInt32 CSL_VPU_WritePCM(UInt8* inBuf, UInt32 inSize, UInt16 writeIndex, CSL_VP_Speech_Mode_t speechMode, UInt16 numFramesPerInterrupt)
{
	UInt32 frameSize;
	UInt8	*buffer = NULL;
	UInt8	i, frameCount = 0;
	UInt32	totalCopied = 0;
	
	frameSize = LIN_PCM_FRAME_SIZE*sizeof(UInt16);
	frameCount = vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame;
	// need to reset them every time?
	vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].vp_speech_mode = speechMode;
	vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame = numFramesPerInterrupt;

	for(i = 0; i < frameCount; i++)
	{	
		buffer = (UInt8 *)(&vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].vp_frame.vectors_lin_PCM[i].param);
		if(inSize - totalCopied >= frameSize)
		{
			// copy it to shared memeory
			memcpy(buffer, inBuf, frameSize);
			inBuf += frameSize;
			totalCopied += frameSize; 
		}
		else
		{
			memcpy(buffer, inBuf, inSize - totalCopied);
			memset(buffer + inSize - totalCopied, 0, frameSize - (inSize - totalCopied));
		}
	}

	return totalCopied;

} // CSL_VPU_WritePCM


//*********************************************************************
/**
*
*   CSL_VPU_ReadAMRNB reads AMR-NB data from shared memory for VPU voice record.
* 
*   @param    outBuf		(out)	destination buffer
*   @param    outSize		(in)	data size to read
*   @param    bufIndex		(in)	index of ping-pong buffer
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_VPU_ReadAMRNB(UInt8* outBuf, UInt32 outSize, UInt16 bufIndex)
{
	UInt32 frameSize;	
	UInt8	*buffer = NULL;
	UInt8	i, frameCount = 0;
	UInt32	totalCopied = 0;
				
	frameSize = sizeof(VR_Frame_AMR_t);
	
	frameCount = vp_shared_mem->shared_voice_buf.vr_buf[bufIndex].nb_frame;
	
	for (i = 0; i < frameCount; i++)
	{	
		buffer = (UInt8 *)(&vp_shared_mem->shared_voice_buf.vr_buf[bufIndex].vr_frame.vectors_amr[i]);
			
		if (outSize - totalCopied >= frameSize)
		{
			// copy it from shared memeory 
			memcpy(outBuf, buffer, frameSize);
			outBuf += frameSize;
			totalCopied += frameSize; 

		}

	}

	return totalCopied;

} // CSL_VPU_ReadAMRNB

//*********************************************************************
/**
*
*   CSL_VPU_WriteAMRNB writes AMR-NB data to shared memory for VPU voice playback.
* 
*   @param    inBuf					(in)	source buffer
*   @param    inSize				(in)	data size to read
*   @param    writeIndex			(in)	index of ping-pong buffer
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
*   @return   UInt32				number of bytes written from the buffer
*
**********************************************************************/
UInt32 CSL_VPU_WriteAMRNB(UInt8* inBuf, UInt32 inSize, UInt16 writeIndex, CSL_VP_Speech_Mode_t speechMode, UInt32 numFramesPerInterrupt)
{
	UInt32 frameSize;
	UInt8	*buffer = NULL;
	UInt8	i, frameCount = 0;
	UInt32	totalCopied = 0;
		
	frameSize = sizeof(VR_Frame_AMR_t);
	frameCount = vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame;
	// need to reset them every time?
	vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].vp_speech_mode = speechMode;
	vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame = numFramesPerInterrupt;

	for (i = 0; i < frameCount; i++)
	{	
		buffer = (UInt8 *)(&vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].vp_frame.vectors_amr[i]);
			
		if (inSize - totalCopied >= frameSize)
		{
			// copy it to shared memeory
			memcpy(buffer, inBuf, frameSize);
			inBuf += frameSize;
			totalCopied += frameSize; 
		}

	}

	return totalCopied;

} // CSL_VPU_WriteAMRNB


//*********************************************************************
/**
*
*   CSL_MMVPU_ReadAMRWB reads AMR-WB data from shared memory for MM VPU voice record.
* 
*   @param    outBuf		(out)	destination buffer
*   @param    outSize		(in)	data size to read
*   @param    bufIndex_no_use	(in)	read index of circular buffer (no use)
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_MMVPU_ReadAMRWB(UInt8* outBuf, UInt32 outSize, UInt16 bufIndex_no_use)
{
	UInt16 size_copied, size_wraparound, totalCopied; 
	UInt32 frameSize = outSize;
	UInt16 bufIndex;
	UInt8 *buffer;

	bufIndex = vp_shared_mem->shared_encodedSamples_buffer_out[0];

	buffer = (UInt8* )&vp_shared_mem->shared_encoder_OutputBuffer[bufIndex&0x0fff];
	
	totalCopied = frameSize;
	
	if(bufIndex + totalCopied/2 >= AUDIO_SIZE_PER_PAGE)//wrap around
	{
		// copy first part
		size_copied = (AUDIO_SIZE_PER_PAGE - bufIndex)<<1;
		memcpy(outBuf, buffer, size_copied);
		outBuf += size_copied;
		// copy second part
		size_wraparound = (totalCopied/2 + bufIndex - AUDIO_SIZE_PER_PAGE)<<1;
		memcpy(outBuf, buffer, size_wraparound);

		vp_shared_mem->shared_encodedSamples_buffer_out[0] = totalCopied/2 + bufIndex - AUDIO_SIZE_PER_PAGE;
	}
	else // no wrap around
	{
		// just copy it from shared memeory
		size_copied = totalCopied;
		memcpy(outBuf, buffer, size_copied);

		vp_shared_mem->shared_encodedSamples_buffer_out[0] += totalCopied/2;
	}

	// the bytes has been really copied.
    return totalCopied;

} // CSL_MMVPU_ReadAMRWB

// ==========================================================================
//
// Function Name: CSL_MMVPU_Rec_GetReadPtr
//
// Description: Get the buffer read pointer (from shared memory)
//
// =========================================================================
UInt16 CSL_MMVPU_Rec_GetReadPtr( void )
{
	return vp_shared_mem->shared_encodedSamples_buffer_out[0];
}

// ==========================================================================
//
// Function Name: CSL_MMVPU_Rec_ResetReadPtr
//
// Description: Reset the buffer read pointer to zero (in shared memory)
//
// =========================================================================
void CSL_MMVPU_Rec_ResetReadPtr( void )
{
	vp_shared_mem->shared_encodedSamples_buffer_out[0] = 0;
}

//*********************************************************************
/**
*
*   CSL_MMVPU_WriteAMRWB writes AMR-WB data to shared memory for MM VPU voice playback.
* 
*   @param    inBuf					(in)	source buffer
*   @param    inSize				(in)	data size to read
*   @param    writeIndex_no_use		(in)	write index of circular buffer (no use)
*   @param    readIndex				(in)	read index of circular buffer
*   @return   UInt32				number of bytes written from the buffer
*
**********************************************************************/
UInt32 CSL_MMVPU_WriteAMRWB(UInt8* inBuf, UInt32 inSize, UInt16 writeIndex_no_use, UInt16 readIndex)
{
	UInt16 size_copied_bytes, size_wraparound; // bytes
	UInt16 space, totalCopied_words;  // words
	UInt32 q_load_words = inSize>>1; // words 
	UInt16 writeIndex;
	UInt8 *buffer;

	writeIndex = vp_shared_mem->shared_NEWAUD_InBuf_in[0];

	buffer = (UInt8* ) &vp_shared_mem->shared_decoder_InputBuffer[writeIndex&0x0fff];

	if(writeIndex >= readIndex) //arm ahead of dsp
	{
		// shared memory available space
		space = (AUDIO_SIZE_PER_PAGE - writeIndex + readIndex - 1);
		
		// words to copy this time.
		totalCopied_words = (space <= q_load_words) ? space : q_load_words;
		
		if (totalCopied_words > 0)
		{
			if ( (writeIndex + totalCopied_words) > AUDIO_SIZE_PER_PAGE) //wrap around
			{
				// copy first part
				size_copied_bytes = (AUDIO_SIZE_PER_PAGE - writeIndex)<<1;
				memcpy(buffer, inBuf, size_copied_bytes);
				inBuf += size_copied_bytes;
				// copy second part
				size_wraparound = (totalCopied_words<<1) - size_copied_bytes;
				memcpy( (UInt8* )&vp_shared_mem->shared_decoder_InputBuffer[0], inBuf, size_wraparound );
			}
			else // no wrap around
			{
				size_copied_bytes = totalCopied_words<<1;
				size_wraparound = 0;
				memcpy(buffer, inBuf, size_copied_bytes);
			}
			
			vp_shared_mem->shared_NEWAUD_InBuf_in[0] = (writeIndex + totalCopied_words ) % AUDIO_SIZE_PER_PAGE;
		}
		

	}
	else //dsp ahead of arm
	{
		// available shared memory space
		space = (readIndex - writeIndex - 1);
		// words to copy this time.
		totalCopied_words = (space <= q_load_words) ? space : q_load_words;
		
		if (totalCopied_words > 0)
		{
			size_copied_bytes = totalCopied_words<<1;
			memcpy(buffer, inBuf, size_copied_bytes);
			vp_shared_mem->shared_NEWAUD_InBuf_in[0] = writeIndex + totalCopied_words;
		}
		
	}

	// the bytes has been really copied.
	return (totalCopied_words<<1);

} // CSL_MMVPU_WriteAMRWB


// ==========================================================================
//
// Function Name: CSL_MMVPU_Play_GetWritePtr
//
// Description: Get the buffer write pointer (from shared memory)
//
// =========================================================================
UInt16 CSL_MMVPU_Play_GetWritePtr( void )
{
	return vp_shared_mem->shared_NEWAUD_InBuf_in[0];
}

// ==========================================================================
//
// Function Name: CSL_MMVPU_Play_GetWritePtr
//
// Description: Reset the buffer write pointer to zero (in shared memory)
//
// =========================================================================
void CSL_MMVPU_Play_ResetWritePtr( void )
{
	vp_shared_mem->shared_NEWAUD_InBuf_in[0] = 0;
}

// ==========================================================================
//
// Function Name: CSL_MMVPU_Play_SetInBufDoneFlag
//
// Description: Tell DSP to stop AMRWB voic play
//
// =========================================================================
void CSL_MMVPU_Play_SetInBufDoneFlag ( void)
{
	// set the flag based on the output select
	vp_shared_mem->shared_NEWAUD_InBuf_done_flag[0] = 1;		
}

// ==========================================================================
//
// Function Name: CSL_MMVPU_Play_Start
//
// Description: Start the data transfer of AMRWB play
//
// =========================================================================
void CSL_MMVPU_Play_Start ( VOICE_PLAYBACK_MODE_t		playbackMode,
								VOICE_PLAYBACK_MIX_MODE_t   mixMode,
								AUDIO_SAMPLING_RATE_t		samplingRate,
								UInt32						speechMode, // used by AMRNB and AMRWB
								UInt32						dataRateSelection, // used by AMRNB and AMRWB
								UInt32						numFramesPerInterrupt)
{
	UInt16 output_buf_select = 0; // fifo0 [0 2], dsp index depends on output buf select
	
	vp_shared_mem->shared_WB_AMR_Ctrl_state = (output_buf_select << 11) | (speechMode << 5);

	// enhancemnet per dsp team suggestion
	// need to init the buffer pointer everytime we start the playback
	vp_shared_mem->shared_NEWAUD_InBuf_in[output_buf_select] = 0;
	vp_shared_mem->shared_NEWAUD_InBuf_out[output_buf_select] = 0;
	
	// program the threshold per dsp suggestion
	vp_shared_mem->shared_Inbuf_LOW_Sts_TH	= 0x200;
	vp_shared_mem->shared_Outbuf_LOW_Sts_TH = 0x400;

	VPRIPCMDQ_MMVPUEnable( 1 );
}
