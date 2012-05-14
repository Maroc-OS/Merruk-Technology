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
*   @file   csl_log.h
*
*   @brief  This file contains CSL (DSP) API to audio logging interface
*
****************************************************************************/
#ifndef _CSL_LOG_H_
#define _CSL_LOG_H_

// ---- Include Files -------------------------------------------------------
#include "mobcom_types.h"

/**
 * @addtogroup CSL audio logging interface
 * @{
 */

#define	LOG_COMPRESSED_SIZE		162
#define	LOG_NB_SIZE				320
#define	LOG_WB_SIZE				642


// ---- Function Declarations -----------------------------------------
//*********************************************************************
/**
*
*   CSL_LOG_Start starts audio logging for specified stream.
* 
*   @param    streamNumber	(in)	stream number 1:4
*   @param    controlInfo	(in)	control information
*
**********************************************************************/
void CSL_LOG_Start(UInt16 streamNumber, UInt16 controlInfo);

//*********************************************************************
/**
*
*   CSL_LOG_Start stops audio logging for specified stream.
* 
*   @param    streamNumber	(in)	stream number 1:4
*
**********************************************************************/
void CSL_LOG_Stop(UInt16 streamNumber);

//*********************************************************************
/**
*
*   CSL_LOG_Read reads data from shared memory for audio logging.
* 
*   @param    streamNumber	(in)	stream number 1:4
*   @param    readIndex		(in)	index of ping-pong buffer 
*   @param    outBuf		(out)	output buffer
*   @param    controlInfo	(out)	logging control information
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_LOG_Read(UInt16 streamNumber, UInt16 readIndex, UInt8 *outBuf, UInt16 *controlInfo);

/** @} */

#endif //_CSL_LOG_H_
