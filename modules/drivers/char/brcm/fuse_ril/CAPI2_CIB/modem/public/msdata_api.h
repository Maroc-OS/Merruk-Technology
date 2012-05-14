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
*   @file   msdata_api.h
*
*   @brief  This file contains definitions for managing MS data access/backup
*			through File System API (whether the actual storage is NVRAM or
*			any 3rd party utilities.
*
*	@note	Any file need to access the nvram data have to include this header.
*
*
****************************************************************************/

#ifndef _NVRAM_DATA_H_
#define _NVRAM_DATA_H_


/******************************************************************************************
 * Constant definitions
 ******************************************************************************************/
#define SYSTEM_DIR_NAME "/msdata"

/* The following defines all the file names (except test files) used in the file system */

#define MSDATA_SMS_FILE_NAME			"/msdata/smsdata.dat"		// SMS related user data

#define MSDATA_DATA_ACCOUNT_FILE_NAME	"/msdata/pdpdataacct.dat"	// CSD/GPRS data acct and PDP context info

#define EQUIVALENT_PLMN_FILE_NAME		"/msdata/equiv_plmn.dat"	// Equivalent PLMN used in stack code

#define NET_PARAM_FILE_NAME				"/msdata/net_par.dat"		// Network Parameters used in stack code

#define NITZ_NETWORK_NAME_FILE_NAME		"/msdata/nitz_name.dat"		// Received NITZ Network Name


// API functions
void		MSDATA_InitMsData(void);
//MsData_t* MSDATA_GetMsDataBasePtr(void);
void* MSDATA_GetSMSSettingsPtr(void);
void		MSDATA_UpdateAllMsDataReq(Boolean);
void* MSDATA_GetDataAccountDbPtr(void);
void* MSDATA_GetPDPDefaultContextPtr(void);
void		MSDATA_UpdateDataAcctReq(Boolean);

void	MSDATA_SetUpdateMsDataFlag(Boolean enable);
Boolean MSDATA_GetUpdateMsDataFlag(void);
Boolean MSDATA_IsDualRatSupported(Boolean *umts_supported);
void MSDATA_UpdateAllDefaultPDPContext(void);
Boolean MSDATA_Get_RamOnly(void);
PCHContextState_t MSDATA_GetPDPDefaultContextState(UInt8 cid);

#endif // _NVRAM_DATA_H_

