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
*   @file   vcc_def.h
*
*   @brief	This file defines the VCC defenitions
*
****************************************************************************/
#ifndef _VCC_DEF_H_
#define _VCC_DEF_H_

#include "timer.h"
#include "ostimer.h"
#include "prim.h"
#include "taskmsgs.h"
#include "tones_def.h"
#include "sys_util.h"
#include "vcc_vmc.h"

#define MAX_SIG_NAME_LEN             50                   ///< Max lenth of signal name string
#define MS_CFG_ELEM_VCC_TIMER_BASE   300                  ///< will add to ms_database later

// Signal retry timer values
#define VCC_TIMER_INTERVAL_TI_ALLOC       (TICKS_ONE_SECOND * 15) ///< Default timer interval of MIMN_MMREG_ALLOC_REQ signal for voice, SS/USSD call setup
#define VCC_TIMER_INTERVAL_SMS_SUBMIT     (TICKS_ONE_SECOND * 15) ///< Default timer interval of MNMI_SMS_SUBMIT_REQ/SIMN_SMS_SEND_PDU_REQ signal for regular SMS submit or STK SMS submit
#define VCC_TIMER_INTERVAL_SMS_CMD        (TICKS_ONE_SECOND * 15) ///< Default timer interval of MNMI_SMS_COMMAND_REQ signal for SMS command submit
#define VCC_TIMER_INTERVAL_SMS_MEM_IND    (TICKS_ONE_SECOND * 15) ///< Default timer interval of MNMI_SMS_MEMORY_AVAIL_IND signal for SMS memory available indication
#define VCC_TIMER_INTERVAL_DS_CALL        (TICKS_ONE_SECOND * 15) ///< Default timer interval of ATDS_SETUP_REQ signal for CSD/video call setup


/// CAPI Module Identifier
typedef enum
{
    MM_REG,     ///< Network Registration
    MN_CC,      ///< Call Control
    MN_SMS,     ///< Short Message Service
    MN_SS,      ///< Supplementary Service
    MN_ATDS,    ///< Data Call
    SM_REG      ///< PDP Context
} CAPI_MODULE_t;
    

typedef UInt16 admCause_t;     ///< admission control cause

typedef char SigNameString_t[MAX_SIG_NAME_LEN + 1]; ///< NULL terminated string for signal name

/// callback function type for vcc timeout
typedef void (* VCC_TimeOutCb) (PrimPtr_t signalPtr, ClientInfo_t * inClientInfoPtr); ///< call back function

/// Structure : VCC Module Type
typedef struct
{
	SingleLinkList_t	srvReqCtxList;		///< Service Request Ctx List

}VCC_Module_t;								///< VCC Module Type


/// Structure : VCC Context
typedef struct
{
	ClientInfo_t	clientInfo;			///< Client Information
	UInt16          admCause;           ///< admission control cause
    PrimPtr_t       signalPtr;          ///< Pointer to signal from CAPI
    SigNameString_t sigNameStr;         ///< String of signal name
    Boolean         bResChk;            ///< Whether signal is resource request
	Boolean			pendOnVmcRsp;		///< The signal is pending on the VMC rsp for admission req
	Timer_t 		retryTimerPtr;	    ///< Retry Timer	
	CAPI_MODULE_t   capiModule;   	    ///< CAPI Module Id
	VCC_TimeOutCb   timeOutCb;          ///< Timeout callback function pointer
	UInt16			deregRetryCounter;	///< Number of times we kick off the retry timer to wait for the other VM to finish de-registration
	Boolean			waitingForDereg;	///< TRUE if VM is waiting for the other VM to finish de-registration
} VCC_Context_t;						///< VCC Context Type


/// Structure : Data content in CAPI request
typedef struct
{
    admCause_t      admCause;           ///< admission control cause
    PrimPtr_t       signalPtr;          ///< Pointer to signal (from CAPI)
    SigNameString_t sigNameStr;         ///< String of signal name
    Boolean         bResChk;            ///< Whether signal is radio resource request
    CAPI_MODULE_t   capiModule;   	    ///< CAPI Module Id
    VCC_TimeOutCb   timeOutCb;          ///< Timeout callback function pointer
} CapiData_t;                           ///< Capi data payload 


/// Structure: data content for CAPI Idle Indication
typedef struct
{
	Boolean chkClientId;				///< TRUE if we need to verify validity of client ID
	Boolean isPowerDownCnf;				///< TRUE if Idle Ind is sent after receiving power-down detach confirmation from stack
} CapiIdleInd_t;

#endif //_VCC_DEF_H_

