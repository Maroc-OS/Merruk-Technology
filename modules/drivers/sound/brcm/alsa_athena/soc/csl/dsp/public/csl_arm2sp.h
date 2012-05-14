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
*   @file   csl_arm2sp.h
*
*   @brief  This file contains CSL (DSP) API to ARM2SP interface
*
****************************************************************************/
#ifndef _CSL_ARM2SP_H_
#define _CSL_ARM2SP_H_

// ---- Include Files -------------------------------------------------------
#include "mobcom_types.h"

/**
 * @addtogroup CSL ARM2SP interface
 * @{
 */

// ---- Function Declarations -----------------------------------------
//*********************************************************************
/**
*
*   CSL_ARM2SP_Init initializes  ARM2SP input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP_Init(void);

//*********************************************************************
/**
*
*   CSL_ARM2SP_Write writes data to shared memory for ARM2SP voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize		(in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP_Write(UInt8* inBuf, UInt32 inSize_inBytes, UInt16 writeIndex);

//*********************************************************************
/**
*
*   CSL_ARM2SP2_Init initializes  ARM2SP2 input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP2_Init(void);

//*********************************************************************
/**
*
*   CSL_ARM2SP2_Write writes data to shared memory for ARM2SP2 voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize		(in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP2_Write(UInt8 *inBuf, UInt32 inSize_inBytes, UInt16 writeIndex);

/** @} */

#endif //_CSL_ARM2SP_H_
