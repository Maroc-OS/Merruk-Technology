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
*
***************************************************************************/
/**
*
*   @file   capi_emulator.h
*
*   @brief  This file defines the types and prototypes for the CAPI emulator functions.
*
****************************************************************************/
/**

*   @defgroup   CAPI_EmulatorGroup   CAPI Emulator
*
*   @brief      This group defines the types and prototypes for the CAPI emulator.
*	
****************************************************************************/

#ifndef _CAPI_EMULATOR_H_
#define _CAPI_EMULATOR_H_

//******************************************************************************
//	 			include block
//******************************************************************************

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @addtogroup CAPI_EMULATORGroup
 * @{
 */


//**************************************************************************************
/**
	Initialize the CAPI emulator.  
	@return		\n RESULT_OK for success, 
				\n RESULT_ERROR for failure, 
**/
Result_t CEMU_Initialize( void );

//**************************************************************************************
/**
	Register new CAPI client.  
	@return		Client ID 
**/
UInt8 CEMU_RegisterCAPIClient( UInt8 inClientId );

//**************************************************************************************
/**
	Deregister CAPI client.  
	@param		inClientId (in) Client id of CAPI client.
**/
void CEMU_DeregisterCAPIClient( UInt8 inClientId );



#if defined(FUSE_APPS_PROCESSOR) || defined(WIN32)
typedef void (*CEMU_DS_Notification_t) (MsgType_t msgType, void* dataBuf);
void CEMU_RegisterPDPCloseCbk(CEMU_DS_Notification_t cbk);
#endif

/** @} */
#ifdef __cplusplus
}
#endif

#endif
