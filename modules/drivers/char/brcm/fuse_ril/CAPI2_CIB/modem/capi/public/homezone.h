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
//
// File:			homezone.h
//
// Description:		This file contains function declarations for the implementations 
//					of the German O2 HomeZone features. 
//
//==============================================================================
#ifndef _HOMEZONE_H_
#define _HOMEZONE_H_
   


//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------


//-------------------------------------------------
// Struct Type Definitions
//-------------------------------------------------


void HZ_Init(void);

void HZ_EvaluateInd(Boolean refresh_flag);

void HZ_PostEvaluateIndReq(Boolean refresh_flag);

void HZ_StopCbMonitor(void);

Boolean HZ_CheckHomeZoneCb(UInt16 msg_id, const UInt8* cb_data);

void HZ_ProcessCbMonitorTimeout(void);

#endif	

