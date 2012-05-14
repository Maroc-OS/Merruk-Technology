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
*   @file   phonectrl_def.h
*
*   @brief  This file contains definitions for the type for PCH 
*			(GPRS PDP Context Handler) API.
*
****************************************************************************/

#ifndef _PHONECTRL_DEF_
#define _PHONECTRL_DEF_


#define RX_SIGNAL_INFO_UNKNOWN 0xFF
#define PHONECTRL_MAX_NMR_LEN 96
typedef enum
{
	POWER_DOWN_STATE_OFF,		///< System is running
	POWER_DOWN_STATE_DETACH,	///< System is in Power Down Detach process
	POWER_DOWN_STATE_RESET		///< System is waiting for reset

} PowerDownState_t;				///< System power down state

///	CAPI system states
typedef enum
{
 	SYSTEM_STATE_OFF,				///< System is off, waiting for power-on
 	SYSTEM_STATE_ON,				///< System is on
 	SYSTEM_STATE_ON_NO_RF,			///< System is On but no radio activity
 	SYSTEM_STATE_OFF_IN_PROGRESS,	///< System is powering down in progress
 	SYSTEM_STATE_OFF_CHARGING		///< Charger is plugged in while handset is off
} SystemState_t;					///< System state

typedef enum
{
    PHONECTRL_INVALID_DOMAIN,
    PHONECTRL_CS_DOMAIN,
    PHONECTRL_PS_DOMAIN,
} PhonectrlDomain_t;

typedef enum
{
    PHONECTRL_CONTEXT_UNKNOWN,
    PHONECTRL_CONTEXT_GSM,
    PHONECTRL_CONTEXT_UMTS,
    PHONECTRL_CONTEXT_EPS
} PhonectrlCipherContext_t;

typedef struct
{
    PhonectrlDomain_t domain;
    PhonectrlCipherContext_t context;
    GPRS_CiphAlg_t ps_alg;  
} PhonectrlCipherAlg_t;

typedef struct
{
  UInt8 len;                        ///< length of NMR info
  UInt8 nmr[PHONECTRL_MAX_NMR_LEN]; ///< NMR info
  UInt8 rat;                        ///< current RAT (RAT_GSM, RAT_UMTS)
  UInt8 regState;                   ///< registration state based on MSRegState_t enum
} Phonectrl_Nmr_t;

#endif
