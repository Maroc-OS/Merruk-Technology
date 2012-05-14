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
*   @file   mnatds.h
*
*   @brief  This file contains definitions for MNATDS function prototypes.
*
****************************************************************************/
#ifndef _MSC_MNATDS_H_
#define _MSC_MNATDS_H_

#include "common_struct.h"
//******************************************************************************
// The following are the general services provided by all interfaces
//******************************************************************************
void MNATDS_Init( void );		   	// Initialize the MNATDS API
void MNATDS_Shutdown( void );
void MNATDS_ProcessMsg( MNATDSMsg_t	*msg );
void MNATDS_ReportDataCallStatus(MNATDSMsg_t* msg);
void MN_PostMNATDSMsg(MNATDSMsg_t* mnatds_msg);



typedef void (*PostMNATDSMsg_t)(			// Post an MNATDS Message
	MNATDSMsg_t	*msg
	);




#endif

