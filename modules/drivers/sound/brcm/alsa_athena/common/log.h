/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/
/**
*
*   @file   log.h
*
*   @brief  This file defines the interface for log API.
*
****************************************************************************/

#ifndef __LOG_H__
#define __LOG_H__

#include <plat/osdal_os.h>

#define Log_DebugPrintf(logID,fmt,args...) 

//#define Log_DebugPrintf dprintf

//#define Log_DebugPrintf(logid,args...) printk(args)

typedef enum
{
    LOGID_SOC_AUDIO_DETAIL
}LOG_ID;

#define L1_LOGV Log_DebugPrintf


#endif

