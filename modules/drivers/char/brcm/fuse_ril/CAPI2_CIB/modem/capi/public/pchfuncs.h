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
*   @file   pchfuncs.h
*
*   @brief  This file contains definitions for interface between PCH and ATC.
*
****************************************************************************/

#ifndef _PCH_PCHFUNCS_H_
#define _PCH_PCHFUNCS_H_



typedef void (*PCHGPRS_DefineContextCb_t) (
	ClientInfo_t				*clientInfoPtr,
	PCHResponseType_t			response,
	PCHCid_t					cid,
	PCHXid_t					*xid,
	PCHRejectCause_t			cause
	);

//******************************************************************************
// The following are the general services provided by all interfaces
//******************************************************************************

void PCHGPRS_Init( void );

void PCHGPRS_Run( void );

void PCH_InitPool(void);

#endif

