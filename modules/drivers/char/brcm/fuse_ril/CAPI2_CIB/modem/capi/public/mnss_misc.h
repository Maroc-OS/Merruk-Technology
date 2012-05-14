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
*   @file   mnss_misc.h
*
*   @brief  This file defines the types and prototypes for the MNSS misc info.
*
****************************************************************************/

#ifndef _MNSS_MISC_H_
#define _MNSS_MISC_H_


/// Structure : MNSS Context
typedef struct
{
	ClientInfo_t		clientInfo;			///< Client Information
	CallIndex_t			callIndex;			///< Call Index
	PId_t				processId;			///< Process Identifier
	UInt8				transId;			///< Transaction Identifier
	Boolean				broadcastRsp;		///< Broadcast Response
	MNSS_State_t		state;				///< SS state
	Timer_t				timerPtr;			///< Termination Timer
	SS_Component_t		component;			///< Component Type
	UInt8				invokeId;			///< Invoke Id
	SS_Operation_t		operation;			///< SS Operation
	SS_SrvReq_t*		srvReqPtr;			///< SS Service Request Pointer	
	UInt8				resend;				///< Resend the request to the network

} MNSS_Context_t;							///< MNSS Context Type



/// Structure : MNSS Message Type
typedef struct
{
	ClientInfo_t	clientInfo;			///< Client Information
	UInt8			callIndex;			///< Call Index                     Shall be removed
	PId_t			processId;			///< Process Identifier
	UInt8			transId;			///< Transaction Identifier
	MNSS_Prim_t		primType;			///< MNSS Primitive Type
	void*			dataPtr;			///< Data Pointer
} MNSS_Msg_t;							///< MNSS Message Type


/// Structure : MNSS Service Request Context Type
typedef struct
{
	Timer_t				timerPtr;			///< Termination timer pointer
	MNSS_Msg_t*			mnssMsgPtr;			///< MNSS msg ptr "Service Request"

}MNSS_SrvReqCtx_t;							///< MNSS Service Request Context Type


#endif //_MNSS_MISC_H_

