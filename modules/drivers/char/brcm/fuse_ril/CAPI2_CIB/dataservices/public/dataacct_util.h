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
*   @file   dataacct_util.h
*
*   @brief  This file contains the utility functions and definitions to the Data Account 
			Services.
*
****************************************************************************/

#ifndef _DATAACCT_UTIL_H_
#define _DATAACCT_UTIL_H_

Result_t		resetDataSize(UInt8 cid);
Result_t		addDataSentSizebyCid(UInt8 cid, UInt32 size);
Result_t		addDataRcvSizebyCid(UInt8 cid, UInt32 size);
Result_t		DATA_SetSecondaryPDP(UInt8 acctId, UInt8 priCid);

#endif // _DATAACCT_UTIL_H_

