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
*   @file   sys_een_rpc.h
*
*   @brief  This file defines the usb message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/

#ifndef RPC_EEM_MSG_H
#define RPC_EEM_MSG_H

#ifndef LOGID_SYSEEMRPC
#define LOGID_SYSEEMRPC LOGID_USB
#endif
#define CACHE_ALIGN 32

#define EEM_HEADER 2
#define EEM_FOOTER 4

/**
* @addtogroup  USBEemApi
* @{
*/
typedef Boolean (*eem_recv_cb_t)(void *buffer, UInt32 buflen, void* bufHandle);
typedef struct {
    eem_recv_cb_t recv;
} eem_cb_t;

//!
//! Function Name:  EEM_Send 
//!
//! Description:  This sends an ethernet packet to be processed.
//!  This api is non blocking
//!
//! \param[in]   void *buffer
//! \param[in]   UInt32 buflen
//!
//! \return void
//!
void EEM_Send(void *buffer, UInt32 buflen);

void EEM_SendEx(void *buffer, UInt32 buflen, UInt8 hdr, UInt8 footer, UInt8 cacheAlign);

//!
//! Function Name:  EEM_Echo
//!
//! Description:  This sends an ethernet packet to be echoed.
//!  This api is non blocking
//!
//! \param[in]   void *buffer
//! \param[in]   UInt32 buflen
//!
//! \return void
//!
void EEM_Echo(void *buffer, UInt32 buflen);

//!
//! Function Name:  EEM_RegisterRecvCb
//!
//! Description:  This api registers a callback that 
//!  gets called for each incoming ethernet packet.
//!  The callback is expected to copy data to a 
//!  private buffer.
//!
//! \param[in]   eem_cb_t *cbs - callback to be installed
//!
//! \return void
//!
void EEM_RegisterRecvCb(eem_cb_t *cbs);

//!
//! Function Name:  EEM_Init
//!
//! Description:  Sets up the EEM interface
//!
//!
//! \return void
//!
void EEM_Init(void);

#endif //



