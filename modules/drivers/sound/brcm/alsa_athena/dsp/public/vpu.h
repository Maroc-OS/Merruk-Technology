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
*   @file   vpu.h
*
*   @brief  This file contains the Voice Processing Unit interface function prototypes and defines
*
****************************************************************************/
/**
*	@defgroup	System_Interface_VPU	Voice Processing Unit (voice recorder)	
*
*	@brief		Voice Processing Unit (voice recorder)	
*
*	VPU record voice input and encoded it in AMR format or PCM format.
*
****************************************************************************/

#ifndef _INC_VPU_H_
#define _INC_VPU_H_

#ifdef VPU_INCLUDED

/**
 * @addtogroup System_Interface_VPU
 * @{
 */

//! --- VPU status return values
typedef enum {
	VPU_STATUS_OK,				///< OK
	VPU_STATUS_DONE,			///<  Done
	VPU_STATUS_READY_TO_DIAL,	///< Ready to dial
	VPU_STATUS_FAIL,			///< Fail
	VPU_STATUS_DB_CORRUPT,		///< corrupt
	VPU_STATUS_FULL,			///< Full
	VPU_STATUS_PAGE				///< Page
} VPU_Status_t;

/// VPU interrupt status
typedef enum {

	VPU_INTERRUPT	= 0x1000,		///< reserved             
	VPU_STATUS_RECORDING_DATA_READY	///< VPU recording data is ready for read

}Interrpt_Status; // amr nb pcm read back. 


//typedef Int16  VPU_Index_t;	// type used to  indentify  voice dialing and recording objects

//! call back function to fill next playback buffer
typedef Boolean (*VPUFillFramesCB_t)(UInt8 *pBuf, UInt32 nSize);

//! call back function to dump out next record buffer
typedef Boolean (*VPUDumpFramesCB_t)(UInt8 *pBuf, UInt32 nSize);

extern UInt8 playback_buffer_control;


//******************************************************************************
//  VPU interface function prototypes
//******************************************************************************

/**
*  @brief  Initialize the voice processing unit
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void VPU_Init( void );						// Initialize the voice processing unit

/**
*  @brief  Stop call/memo recording
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void VPU_StopRecording( void );				// Stop call/memo recording

/**
*  @brief  Stop  playback
*
*  @param  flag	
*
*  @return none
*
****************************************************************************/
void VPU_StopPlayback( UInt8 flag );		// Stop  playback

/**
*  @brief  Cancel playback
*
*  @param  none	
*
*  @return none
*
****************************************************************************/
void VPU_CancelPlayback( void );			// Cancel playback

/**
*  @brief  Cancel recording	
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void VPU_CancelRecording( void );			// Cancel recording	

/**
*  @brief  Process  VP status queue
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void VPU_ProcessStatus( void );				// Process  VP status queue

/**
*  @brief  Stop VPU process  for receiving page
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void VPU_Stop( void );						// Stop VPU process  for receiving page


/**
*  @brief  Start memo playback
*
*  @param  FillFramesCB		callback function to fill playback frame
*  @param  vp_speech_mode	speech mode
*  @param  audioPath		audio path
*
*  @return	Boolean
*
****************************************************************************/
Boolean VPU_StartPlaybackMemo(   			// Start memo playback
	VPUFillFramesCB_t	FillFramesCB,		// callback function to fill playback frame
	VP_Speech_Mode_t 	vp_speech_mode,		// speech mode
	UInt8				audioPath			// audio path
	);

/**
*  @brief  Stop memo recording
*
*  @param  none
*
*  @return	Boolean
*
****************************************************************************/
Boolean VPU_StopMemo(void);

/**
*  @brief  Turn on audio
*
*  @param  Uplink	 uplink, mic
*  @param  Downlink  downlink, speaker
*
*  @return	none
*
****************************************************************************/
void VPU_AudioTurnOn(Boolean Uplink, Boolean Downlink);

/**
*  @brief  Turn off audio
*
*  @param  none
*
*  @return	none
*
****************************************************************************/
void VPU_AudioTurnOff(void);

/**
*  @brief  VPU_PolyringerStartPlay_Test
*
*  @param  SynRate
*  @param  OutMode
*
*  @return	none
*
****************************************************************************/
void VPU_PolyringerStartPlay_Test(UInt16 SynRate, UInt16 OutMode);   	

/**
*  @brief  VPU_PolyringerCancelPlay_Test
*
*  @param  Uplink	 uplink, mic
*  @param  Downlink  downlink, speaker
*
*  @return	Boolean
*
****************************************************************************/
void VPU_PolyringerCancelPlay_Test(UInt8 audioPath);   	

/**
*  @brief  VPU_StartTelephony
*
*  @param  telephony_dump_cb	record callback
*  @param  telephony_fill_cb	playback callback
*  @param  encode_amr_mode		AMR mode for encoding the next speech frame
*  @param  dtx_mode				Turn DTX on (TRUE) or off (FALSE)
*  @param  amr_if2_enable		Select AMR IF1 (FALSE) or IF2 (TRUE) format
*
*  @return	Boolean
*
****************************************************************************/
Boolean VPU_StartTelephony(
	VPUDumpFramesCB_t	telephony_dump_cb,
	VPUFillFramesCB_t	telephony_fill_cb,
	VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
	Boolean				dtx_mode,			// Turn DTX on (TRUE) or off (FALSE)
	Boolean				amr_if2_enable		// Select AMR IF1 (FALSE) or IF2 (TRUE) format
	);

/**
*  @brief  VPU_StartTelephony
*
*  @param  telephony_dump_cb	record callback
*  @param  telephony_fill_cb	playback callback
*  @param  encode_amr_mode		AMR mode for encoding the next speech frame
*  @param  dtx_mode				Turn DTX on (TRUE) or off (FALSE)
*  @param  amr_if2_enable		Select AMR IF1 (FALSE) or IF2 (TRUE) format
*
*  @return	none
*
****************************************************************************/
void VPU_StartMainAMRDecodeEncode(
	VP_Mode_AMR_t		decode_amr_mode,	// AMR mode for decoding the next speech frame
	UInt8				*pBuf,		// buffer carrying the AMR speech data to be decoded
	UInt8				length,		// number of bytes of the AMR speech data to be decoded
	VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
	Boolean				dtx_mode	// Turn DTX on (TRUE) or off (FALSE)
	);

/**
*  @brief  VoIP_StartMainAMRDecodeEncode
*
*  @param  decode_amr_mode	record callback
*  @param  pBuf			DL data buffer pointer
*  @param  length		Data length
*  @param  encode_amr_mode	codec type for encoding the next speech frame
*  @param  dtx_mode		Turn DTX on (TRUE) or off (FALSE)
*
*  @return	none
*
****************************************************************************/
void VoIP_StartMainAMRDecodeEncode(
	VP_Mode_AMR_t		decode_amr_mode,	// AMR mode for decoding the next speech frame
	UInt8				*pBuf,		// buffer carrying the AMR speech data to be decoded
	UInt16				length,		// number of bytes of the AMR speech data to be decoded
	VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
	Boolean				dtx_mode	// Turn DTX on (TRUE) or off (FALSE)
	);

/**
*  @brief  VPU_ProcessStatusMainAMRDone
*
*  @param  none
*  @return	none
*
****************************************************************************/
void VPU_ProcessStatusMainAMRDone(StatQ_t status_msg);

/**
*  @brief  VPU_StopTelelphony
*
*  @param  none
*  @return	Boolean
*
****************************************************************************/
Boolean VPU_StopTelelphony(void);

/**
*  @brief  VoIP_StartTelephony
*
*  @param  telephony_dump_cb	record callback
*  @param  telephony_fill_cb	playback callback
*  @param  encode_amr_mode		AMR mode for encoding the next speech frame
*  @param  dtx_mode				Turn DTX on (TRUE) or off (FALSE)
*  @param  amr_if2_enable		Select AMR IF1 (FALSE) or IF2 (TRUE) format
*
*  @return	none
*
****************************************************************************/
Boolean VoIP_StartTelephony(
	VPUDumpFramesCB_t telephony_dump_cb,
	VPUDumpFramesCB_t telephony_fill_cb,
	UInt16	 voip_codec_type,  // AMR mode for encoding the next speech frame
	Boolean	       dtx_mode,	// Turn DTX on (TRUE) or off (FALSE): this is obsolete. contained in voip_codec_type
	Boolean	     amr_if2_enable	 // Select AMR IF1 (FALSE) or IF2 (TRUE) format: obsolete
	);

/**
*  @brief  VoIP_StopTelephony
*
*  @param  none
*  @return	Boolean
*
****************************************************************************/
Boolean VoIP_StopTelephony(void);

/**
*  @brief  get VPU_VT_Activ flag
*
*  @param  none
*  @return	Boolean
*
****************************************************************************/
Boolean VPU_VT_Activ(void);

/**
*  @brief  VPU_VT_Clear
*
*  @param  none
*  @return	none
*
****************************************************************************/
void VPU_VT_Clear(void);

/**
*  @brief  get VPU_VT_enc_mode
*
*  @param  none
*  @return	UInt8  mode
*
****************************************************************************/
UInt16 VPU_VT_enc_mode(void);

/**
*  @brief  get VPU_VT_dec_mode
*
*  @param  none
*  @return	UInt8  mode
*
****************************************************************************/
UInt16 VPU_VT_dec_mode(void);

/**
*  @brief  get VPU_VT_len
*
*  @param  none
*  @return	UInt8  length
*
****************************************************************************/
UInt8 VPU_VT_len(void);

/**
*  @brief  get VPU_VT_ptr
*
*  @param  none
*  @return	UInt32*  pointer
*
****************************************************************************/
UInt32 *VPU_VT_ptr(void);

/**
*  @brief  VPU_SetFlag_fuse_ap_vt_active
*
*  @param   flag
*  @return	none
*
****************************************************************************/
void VPU_SetFlag_fuse_ap_vt_active( UInt16 flag );

void AP_Process_VPU_Status( VPStatQ_t vp_status_msg);

#endif //#ifdef VPU_INCLUDED

#endif
