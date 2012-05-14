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
*   @file   plmn_table.h
*
*   @brief  This file contains the declarations for the PLMN table lookup functions.
*
****************************************************************************/
/**

*   @defgroup   PLMNTABLEGroup   Plmn Table
*   @ingroup    MSCGroup
*
*   @brief      This file contains the declarations for the PLMN table lookup functions.
*	
****************************************************************************/

#ifndef _PLMN_TABLE_H_
#define _PLMN_TABLE_H_

/**
 * @addtogroup PLMNTABLEGroup
 * @{
 */

/** @} */


// Internal function
Boolean MS_IsHomePlmn(UInt16 mcc, UInt8 mnc, const PLMNId_t *hplmn);


#endif

