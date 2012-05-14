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
*   @file   csl_vpu.h
*
*   @brief  This file contains DSP API to VPU interface
*
****************************************************************************/
#ifndef _CSL_VPU_H_
#define _CSL_VPU_H_

// ---- Include Files -------------------------------------------------------
#include "mobcom_types.h"
#include "audio_consts.h"

/**
 * @addtogroup CSL VPU Interface
 * @{
 */

typedef enum
{
    CSL_VP_SPEECH_MODE_NONE = 0,
    CSL_VP_SPEECH_MODE_FR = 1,
    CSL_VP_SPEECH_MODE_EFR = 2,
    CSL_VP_SPEECH_MODE_LINEAR_PCM_8K= 3,
    CSL_VP_SPEECH_MODE_AMR = 4,
    CSL_VP_SPEECH_MODE_AMR_2 = 5,
    CSL_VP_SPEECH_MODE_LINEAR_PCM_16K= 6
} CSL_VP_Speech_Mode_t;

//#include "shared.h"

typedef enum VOICE_PLAYBACK_MODE_t
{
	VOICE_PLAYBACK_NONE,  // = PLAYBACK_NONE,
	VOICE_PLAYBACK_DL,  // = SPEAKER_EAR_PIECE_ONLY,
	VOICE_PLAYBACK_UL,  // = OVER_THE_CHANNEL_ONLY,
	VOICE_PLAYBACK_BOTH  // = SPEAKER_EAR_PIECE_AND_CHANNEL
} VOICE_PLAYBACK_MODE_t;
						
typedef enum VOICE_PLAYBACK_MIX_MODE_t
{
	VOICE_PLAYBACK_MIX_NONE,  // = MIX_NONE,
	VOICE_PLAYBACK_MIX_DL,  // = MIX_SPEAKER_EAR_PIECE_ONLY,
	VOICE_PLAYBACK_MIX_UL,  // = MIX_OVER_THE_CHANNEL_ONLY,
	VOICE_PLAYBACK_MIX_BOTH  // = MIX_SPEAKER_EAR_PIECE_AND_CHANNEL
} VOICE_PLAYBACK_MIX_MODE_t;

// ---- Function Declarations -----------------------------------------

//*********************************************************************
/**
*
*   CSL_VPU_Init initializes VPU interface.
*
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
* 
**********************************************************************/
void CSL_VPU_Init(CSL_VP_Speech_Mode_t speechMode, UInt16 numFramesPerInterrupt);

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
UInt32 CSL_VPU_ReadPCM(UInt8* outBuf, UInt32 outSize, UInt16 bufIndex, CSL_VP_Speech_Mode_t speechMode);

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
UInt32 CSL_VPU_WritePCM(UInt8* inBuf, UInt32 inSize, UInt16 writeIndex, CSL_VP_Speech_Mode_t speechMode, UInt16 numFramesPerInterrupt);

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
UInt32 CSL_VPU_ReadAMRNB(UInt8* outBuf, UInt32 outSize, UInt16 bufIndex);

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
UInt32 CSL_VPU_WriteAMRNB(UInt8* inBuf, UInt32 inSize, UInt16 writeIndex, CSL_VP_Speech_Mode_t speechMode, UInt32 numFramesPerInterrupt);

//*********************************************************************
/**
*
*   CSL_MMVPU_ReadAMRWB reads AMR-WB data from shared memory for MM VPU voice record.
* 
*   @param    outBuf		(out)	destination buffer
*   @param    outSize		(in)	data size to read
*   @param    bufIndex		(in)	index of circular buffer
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_MMVPU_ReadAMRWB(UInt8* outBuf, UInt32 outSize, UInt16 bufIndex);

//*********************************************************************
/**
*   CSL_MMVPU_Rec_GetReadPtr get the buffer read pointer (from shared memory)
* 
*   @param    none
*   @return   UInt16			the buffer write pointer
*
**********************************************************************/
UInt16 CSL_MMVPU_Rec_GetReadPtr( void );

//*********************************************************************
/**
*   CSL_MMVPU_Rec_ResetReadPtr reset the buffer read pointer to zero (in shared memory)
* 
*   @param    none
*   @return   none
*
**********************************************************************/
void CSL_MMVPU_Rec_ResetReadPtr( void );

//*********************************************************************
/**
*
*   CSL_MMVPU_WriteAMRWB writes AMR-WB data to shared memory for MM VPU voice playback.
* 
*   @param    inBuf					(in)	source buffer
*   @param    inSize				(in)	data size to read
*   @param    writeIndex			(in)	write index of circular buffer
*   @param    readIndex				(in)	read index of circular buffer
*   @return   UInt32				number of bytes written from the buffer
*
**********************************************************************/
UInt32 CSL_MMVPU_WriteAMRWB(UInt8* inBuf, UInt32 inSize, UInt16 writeIndex, UInt16 readIndex);

//*********************************************************************
/**
*   CSL_MMVPU_Play_GetWritePtr get the buffer write pointer (from shared memory)
* 
*   @param    none
*   @return   UInt16			the buffer write pointer
*
**********************************************************************/
UInt16 CSL_MMVPU_Play_GetWritePtr( void );

//*********************************************************************
/**
*   CSL_MMVPU_Play_ResetWritePtr reset the buffer write pointer to zero (in shared memory)
* 
*   @param    none
*   @return   none
*
**********************************************************************/
void CSL_MMVPU_Play_ResetWritePtr( void );

//*********************************************************************
/**
*   Tell DSP to stop AMRWB voic play
* 
*   @param    none
*   @return   none
*
**********************************************************************/
void CSL_MMVPU_Play_SetInBufDoneFlag ( void);

//*********************************************************************
/**
*   Start the data transfer of AMRWB play
* 
*   @param    playbackMode			(in)
*   @param    mixMode				(in)
*   @param    samplingRate			(in)
*   @param    speechMode			(in)
*   @param    dataRateSelection		(in)
*   @param    numFramesPerInterrupt (in)
*   @return   none
*
**********************************************************************/
void CSL_MMVPU_Play_Start ( VOICE_PLAYBACK_MODE_t		playbackMode,
								VOICE_PLAYBACK_MIX_MODE_t   mixMode,
								AUDIO_SAMPLING_RATE_t		samplingRate,
								UInt32						speechMode, // used by AMRNB and AMRWB
								UInt32						dataRateSelection, // used by AMRNB and AMRWB
								UInt32						numFramesPerInterrupt
								);

/** @} */

#endif //_CSL_VPU_H_
