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
*   @file   csl_arm2sp.c
*
*   @brief  This file contains CSL (DSP) API to ARM2SP interface
*
****************************************************************************/

#include <string.h>
#include "assert.h"
#include "mobcom_types.h"
#include "shared.h"
#include "csl_arm2sp.h"

extern VPSharedMem_t	*vp_shared_mem;

//*********************************************************************
/**
*
*   CSL_ARM2SP_Init initializes  ARM2SP input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP_Init(void)
{
    memset(&vp_shared_mem->shared_Arm2SP_InBuf, 0, sizeof(vp_shared_mem->shared_Arm2SP_InBuf)); // clean buffer 

} // CSL_ARM2SP_Init


//*********************************************************************
/**
*
*   CSL_ARM2SP_Write writes data to shared memory for ARM2SP voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize_inBytes  (in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP_Write(UInt8* inBuf, UInt32 inSize_inBytes, UInt16 writeIndex)
{
	UInt32 offset = 0;
	UInt32  bufSize_inWords;
	UInt32  halfBufSize_inBytes;
	UInt32	totalCopied_bytes;

	bufSize_inWords = ARM2SP_INPUT_SIZE;  //in number of words.
	halfBufSize_inBytes = ARM2SP_INPUT_SIZE;
		
	//beginning of the buffer or the half point in the buffer.
	offset = (writeIndex == 0)? (bufSize_inWords/2) : 0; // offset is in 16bit words


	totalCopied_bytes = (inSize_inBytes < halfBufSize_inBytes) ? inSize_inBytes : halfBufSize_inBytes;

	memcpy( (UInt8 *)(&vp_shared_mem->shared_Arm2SP_InBuf[offset]), inBuf, totalCopied_bytes);
	
	return totalCopied_bytes;  	// return the number of bytes has been copied

} // CSL_ARM2SP_Write


//*********************************************************************
/**
*
*   CSL_ARM2SP2_Init initializes  ARM2SP2 input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP2_Init(void)
{
    memset(&vp_shared_mem->shared_Arm2SP2_InBuf, 0, sizeof(vp_shared_mem->shared_Arm2SP2_InBuf)); // clean buffer 

} // CSL_ARM2SP2_Init


//*********************************************************************
/**
*
*   CSL_ARM2SP2_Write writes data to shared memory for ARM2SP2 voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize_inBytes  (in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP2_Write(UInt8* inBuf, UInt32 inSize_inBytes, UInt16 writeIndex)
{
	UInt32 offset = 0;
	UInt32  bufSize_inWords;
	UInt32  halfBufSize_inBytes;
	UInt32	totalCopied_bytes;

	bufSize_inWords = ARM2SP_INPUT_SIZE;  //in number of words.
	halfBufSize_inBytes = ARM2SP_INPUT_SIZE;
		
	//beginning of the buffer or the half point in the buffer.
	offset = (writeIndex == 0)? (bufSize_inWords/2) : 0; // offset is in 16bit words


	totalCopied_bytes = (inSize_inBytes < halfBufSize_inBytes) ? inSize_inBytes : halfBufSize_inBytes;

	memcpy( (UInt8 *)(&vp_shared_mem->shared_Arm2SP2_InBuf[offset]), inBuf, totalCopied_bytes);
	
	return totalCopied_bytes;  	// return the number of bytes has been copied

} // CSL_ARM2SP2_Write

