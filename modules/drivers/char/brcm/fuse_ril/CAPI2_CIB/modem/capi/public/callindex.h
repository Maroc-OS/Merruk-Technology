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
*   @file   callindex.h
*
*   @brief  This file contains definitions for Call Index Database.
*
****************************************************************************/

#ifndef _MSC_CALLINDEX_H_
#define _MSC_CALLINDEX_H_


//******************************************************************************
// Enumerations
//******************************************************************************

typedef enum
{
	CALLINDEXTYPE_FREE 		= 0x00,	// Call Index is not used
	CALLINDEXTYPE_MO_CC		= 0x01,	// Call Index is used for MO Call Control
	CALLINDEXTYPE_MO_CC_DS	= 0x02,	// Call Index is used for MO Call Control, Data Service
	CALLINDEXTYPE_MO_SS		= 0x03,	// Call Index is used for MO Supplementary Service
	CALLINDEXTYPE_MO_SMS	= 0x04,	// Call Index is used for MO Short Message Service
	CALLINDEXTYPE_MT_CC		= 0x05,	// Call Index is used for MT Call Control
	CALLINDEXTYPE_MT_CC_DS	= 0x06,	// Call Index is used for MT Call Control, Data Service
	CALLINDEXTYPE_MT_SS		= 0x07,	// Call Index is used for MT Supplementary Service
	CALLINDEXTYPE_MT_SMS	= 0x08,	// Call Index is used for MT Short Message Service
									// where MO = Mobile Originated
									//       MT = Mobile Terminated
	CALLINDEXTYPE_NO_TI		= 0x09	// TI hasn't been set
} CallIndexType_t;

//******************************************************************************
// Callbacks
//******************************************************************************


//******************************************************************************
// Function Prototypes
//******************************************************************************

void CALLINDEX_Init( void );			// Initialize the call-control manager



Result_t CALLINDEX_AllocateCallIndex(
	CallIndex_t *ci,				// newly allocated Call Index
	CallIndexType_t proposed_type	// proposed type of call
	);

Result_t CALLINDEX_UpdateCallIndex(
	CallIndex_t ci,					// newly allocated Call Index
	UInt8 ti,						// Transaction Id
	CallIndexType_t allocated_type	// allocated type of call
	);

void CALLINDEX_FreeCallIndex(
	CallIndex_t ci					// Call Index to be free
	);

Result_t CALLINDEX_GetCallIndexInfo(
	CallIndex_t ci,					// Call Index
	UInt8 *ti,						// Transaction Id
	CallIndexType_t *type			// type of call
	);



Boolean  CALLINDEX_IsCIDataCall(
	CallIndex_t ci
	);

CallIndex_t CALLINDEX_GetCI(
	UInt8 ti
	);

UInt8 CALLINDEX_GetTI(
	CallIndex_t ci
	);


#endif

