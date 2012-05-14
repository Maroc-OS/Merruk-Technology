/*********************************************************************
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
***************************************************************************/
/**
*
*   @file   pch_stubs.c
*
*   @brief  This file implements the functions needed by pch and snprim.
*			There are two main functionality. The implementation is from dlink.
*			1. creation/deletion of T_OS_PP_ID.
*			2. update tft into dc data structure.
*
****************************************************************************/
#define	ENABLE_LOGGING
#include	<stdio.h>
#ifndef UNDER_LINUX
#include <string.h>
#endif

#include	"mobcom_types.h"
#include    "ms.h" 
#include    "resultcode.h"
#include    "common_defs.h"
#include	"taskmsgs.h"
#include	"pch_def.h"
#include	"consts.h"
#include 	"ostypes.h"
#include	"mti_trace.h"
#include	"xassert.h"
#include	"log.h" //for PPP_LOGV
#include	"logapi.h" //for Log_DebugOutputString
#include	"pchex_def.h"
#include	"pchex_api.h"



Result_t PCHStubs_PDP_SendPDPActivateReq(UInt8	clientID,
	UInt8						cid,
	PCHActivateReason_t			reason,
	PCHProtConfig_t				*protConfig)
{
	return PCHEx_SendPDPActivateReq(clientID, cid, reason, protConfig);
}


Result_t PCHStubs_PDP_SendPDPActivateSecReq(
	UInt8						clientID,
	UInt8						cid
	)
{
	return PCHEx_SendPDPActivateSecReq(clientID, cid);
}



Result_t PCHStubs_PDP_SendPDPDeactivateReq(
	UInt8						clientID,
	UInt8						cid
	)
{
	return PCHEx_SendPDPDeactivateReq(clientID, cid);
}

Result_t PCHStubs_PDP_SendPDPModifyReq(UInt8	clientID,
	UInt8						cid)
{
	return PCHEx_SendPDPModifyReq(clientID, cid);
}



