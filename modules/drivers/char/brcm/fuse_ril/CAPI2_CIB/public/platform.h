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
*   @file   platform.h
*
*   @brief  This file contains definitions for GPRS platform API.
*
****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
*/
#ifndef _PLATFORM_H_
#define _PLATFORM_H_


//-------------------------------------
// Types and Enum
//-------------------------------------
 


//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

// MS related
const UInt8 *GetSVN(void);


// System Related
void		SYS_ClientInit(void);
void		SYS_ClientRun(void);

extern UInt16	SYSPARM_GetSysparmIndPartFileVersion(void);
extern UInt8*	PATCH_GetRevision(void);
void 			SYS_GetBootLoaderVersion(UInt8* BootLoaderVersion, UInt8 BootLoaderVersion_sz);
void			SYS_GetDSFVersion(UInt8* DSFVersion, UInt8 DSFVersion_sz);

// Device Related
extern UInt32	RTC_GetCount(void);
extern void		RTC_ResetCount(void);


#endif  // _PLATFORM_H_

