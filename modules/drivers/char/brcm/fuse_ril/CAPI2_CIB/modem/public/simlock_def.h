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
*   @file   simlock_def.h
*
*   @brief  This file defines common definitionss for simlock.
*
****************************************************************************/
/**
*   @defgroup   SIMLOCKGroup   SIM Security definitions
*   @brief      This group defines the interfaces to the SIM Lock module 
*   @ingroup    SIMSTK
*
*****************************************************************************/

#ifndef _SIMLOCK_DEF_H_
#define _SIMLOCK_DEF_H_


/**
 * @addtogroup SIMLOCKGroup
 * @{
 */

/**
 Simlock type, first SIMLOCK type must start with 0 and subsequent SIMLOCK 
 types must be consecutive. Code implementation assumes this.
**/
typedef enum {
	SIMLOCK_NETWORK_LOCK = 0, 
	SIMLOCK_NET_SUBSET_LOCK,
	SIMLOCK_PROVIDER_LOCK,
	SIMLOCK_CORP_LOCK,
	SIMLOCK_PHONE_LOCK,

	SIMLOCK_INVALID_LOCK ///< This must be put in the last

} SIMLockType_t;


/**
 Status of unlocking all type of SIM Locks (Network, Service Provider, etc).
**/
typedef enum
{
	SIMLOCK_SUCCESS,			///< Lock successfully unlocked
	SIMLOCK_FAILURE,			///< Lock not unlocked, but can try again
	SIMLOCK_PERMANENTLY_LOCKED,	///< Permanent locked	
	SIMLOCK_WRONG_KEY,          ///< Unlock password is wrong, can retry
	SIMLOCK_NOT_ALLOWED,        ///< Unlock not allowed
	SIMLOCK_DISABLED,           ///< Unlock disabled

	SIMLOCK_INVALID_STATUS      ///< Invalid status of SIMLOCK

} SIMLock_Status_t;



/** @} */

#endif  // _SIMLOCK_DEF_H_
