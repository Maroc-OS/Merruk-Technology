/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   mnssprim.h
*
*   @brief  This file defines the types and prototypes for the MNSS primitive functions.
*
****************************************************************************/

#ifndef _MSC_MNSSPRIM_H_
#define _MSC_MNSSPRIM_H_


//******************************************************************************
// Function Prototypes
//******************************************************************************

void MNSSPRIM_RxPrim(PrimPtr_t Primitive);


//**************************************************************************************
/**
	MnssPrim_RegisterMsg() function builds and send the Register messages.
	@param		inMnssCtxPtr (in) MNSS Context Pointer
	@return		(out) void
**/
void MnssPrim_RegisterMsg(MNSS_Context_t* inMnssCtxPtr);

//**************************************************************************************
/**
	MnssPrim_RegisterMsg() function builds and send the Facilityt messages.
	@param		inMnssCtxPtr (in) MNSS context pointer.
	@param		inCompType (in) component type.
	@return		(out) void
**/
void MnssPrim_FacilityMsg(MNSS_Context_t*	inMnssCtxPtr,
						  SS_Component_t	inCompType);

//**************************************************************************************
/**
	MnssPrim_ReleaseCompleteMsg() function builds and send the Release_Complete messages.
	@param		inTransId (in) transaction Id of the SS session.
	@param		inProcessId (in) process Id of the stack task.
	@param		inMnssCtxPtr (in) MNSS context pointer.
	@param		inCause (in) release cause to be sent to the network.
	@return		(out) void
**/
void MnssPrim_ReleaseCompleteMsg(UInt8				inTransId,
								 PId_t				inProcessId,
								 MNSS_Context_t*	inMnssCtxPtr,
								 Cause_t			inCause);

#endif //_MSC_MNSSPRIM_H_

