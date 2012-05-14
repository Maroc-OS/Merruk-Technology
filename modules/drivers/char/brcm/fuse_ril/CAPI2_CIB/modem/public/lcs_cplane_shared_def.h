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
*   @file   lcs_cplane_shared_def.h
*
*   @brief  This header provides the definition of the data types used by control plane Location Service API.
*
****************************************************************************/

#ifndef LCS_CPLANE_SHARED_DEF_H__
#define LCS_CPLANE_SHARED_DEF_H__		///< Include guard.

//The following header files should be included before include lcs_cplane_shared_def.h
// "mobcom_types.h"
// "taskmsgs.h"


/**
	The PDU data of the control plane messages.
**/
typedef struct
{
	UInt32 mDataLen;	///< The length of the data
	UInt8 * mData;      ///< The data buffer.
} LcsPduData_t;

/**
	The data of LCS messages.
**/
typedef struct
{
    ClientInfo_t mClientInfo;    ///< The registered handler client info
	UInt32 mDataLen;	///< The length of the data
	UInt8 * mData;      ///< The data buffer.
} LcsMsgData_t;

#endif	// LCS_CPLANE_SHARED_DEF_H__


