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
*   @file   vcc_api.h
*
*   @brief	This file contains task and queue function prototypes.
*
****************************************************************************/
#ifndef _VCC_API_H_
#define _VCC_API_H_

#include "vcc_def.h"


typedef enum
{
	VCC_VM_SUSPENDED,     ///< VM has been suspended, network request is likely rejected after VCC retry timeout
	VCC_VM_RESUMED,       ///< VM has been resumed, network request will be processed
	VCC_VM_CS_ALLOWED	  ///< VM is still suspended for PS connection, but CS request will be processed by suspending PS connection
} VCC_VM_STATUS_t;


#define VccApi_PostIdleIndication(clientInfoPtr, chkClientId, isPowerDownCnf) VccApi_PostIdleInd(clientInfoPtr, chkClientId, isPowerDownCnf, __FILE__, __LINE__)


//***************************************************************************************
/**
    This function replaces the original TX_PRIM() function call in each CAPI module.
	@param		clientInfoPtr (in) pointer to ClientInfo

    @param		signalPtr (in) signal pointer. For example, use 
                (PrimPtr_t)OUTSIGNAL( MNCC_SETUP_REQ );} for MNCC_SETUP_REQ

	@param		sigNameStr (in) signal name string, e.g., use "MNCC_SETUP_REQ"
	            for MNCC_SETUP_REQ	
	
	@return		void

	@note       This function replaces the original TX_PRIM() function call 
	            in each CAPI module. It should be done in the prim level.
	
**/

void VccApi_SendSignal( ClientInfo_t * clientInfoPtr, PrimPtr_t signalPtr, SigNameString_t sigNameStr );


//***************************************************************************************
/**
    This function replaces the original TX_PRIM_TO() function call in each CAPI module.
	@param		clientInfoPtr (in) pointer to ClientInfo

    @param		signalPtr (in) signal pointer. For example, use 
                (PrimPtr_t)OUTSIGNAL( MNCC_SETUP_REQ );} for MNCC_SETUP_REQ

    @param		pid (in) id of process to receive signal
	
	@param		sigNameStr (in) signal name string, e.g., use "MNCC_SETUP_REQ"
	            for MNCC_SETUP_REQ
	
	@return		void

	@note       This function replaces the original TX_PRIM_TO() function call 
	            in each CAPI module. It should be done in the prim level.
	
**/

void VccApi_SendSignalTo(ClientInfo_t * clientInfoPtr, PrimPtr_t signalPtr, PId_t pid, 
                          SigNameString_t sigNameStr);


//***************************************************************************************
/**
    This function is for CAPI module to post MSG_VCC_CAPI_IDLE_IND to VCC task
	@param		clientInfoPtr (in) pointer to ClientInfo    

	@param		chkClientId (in) whether to check the validity of the clientID of the passed
				clientInfo. The clientID check needs to be skipped for MSG_VCC_CAPI_IDLE_IND
				sent as a result of ATTACH_REJ/ATTACH_CNF since ATTACH_REQ can be cancelled
				by a subsequent DETACH_REQ and CAPI will not receive ATTACH_REJ/ATTACH_CNF. 

    @param		isPowerDownCnf (in) TRUE if the idle indication is sent after receiving 
				power down detach confirmation from stack

	@param		fileName (in) Name of file in which this function is called
	@param		lineNo (in) Line of code in which this function is called

	@return		void
	
**/
void VccApi_PostIdleInd(ClientInfo_t * clientInfoPtr, Boolean chkClientId, Boolean isPowerDownCnf, const char* fileName, UInt32 lineNo);



//***************************************************************************************
/**
    This function posts the request to VCC to check the SIM PIN status of both VM's and decide whether 
	to request the protocol stack to enable/disable a VM and whether a previously powered-down VM needs 
	to be powered up again.
	
	@return		void
	
**/
void VccApi_PostChkVmReq(void);

//***************************************************************************************
/**
    This function cancels a send signal request
	@param		clientInfoPtr (in) pointer to ClientInfo
	
	@return		void

	@note       
	
**/
void VccApi_CancelSignal( ClientInfo_t * clientInfoPtr);

#endif
