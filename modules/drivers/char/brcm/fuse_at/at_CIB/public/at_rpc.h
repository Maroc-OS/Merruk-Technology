//
/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
*   @file   at_rpc.h
*
*   @brief  This file contains the function prototypes for the ATC RPC.
*
****************************************************************************/

#ifndef _AT_RPC_H_
#define _AT_RPC_H_

typedef struct
{
	UInt8			channel;		///< AP Channel Info
	Boolean 			unsolicited;		///< unsolicited flag
}AtRegisterInfo_t;


void AT_InitRpc(void);
Result_t AT_SendRpcMsg(UInt32 msgId, void *val);
Result_t AT_PostRpcCommandStr(UInt8 chan, const UInt8* atCmdStr);
Result_t AT_RegisterCPTerminal(UInt8 chan, Boolean unsolicited);


#endif //_AT_RPC_H_
