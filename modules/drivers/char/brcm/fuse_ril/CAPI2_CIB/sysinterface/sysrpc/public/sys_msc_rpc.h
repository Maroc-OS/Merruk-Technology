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
*   @file   sys_msc_rpc.h
*
*   @brief  This file defines the usb host msc apis.
*
****************************************************************************/

#ifndef RPC_MSC_MSG_H
#define RPC_MSC_MSG_H

/**
* @addtogroup  USBIpcApi
* @{
*/

typedef struct sysMscRpcInfo
{
    UInt32 sectorSize; //! Size in bytes of a sector in the drive
    UInt32 sectorCount;//! Number of sectors in the drive
    UInt32 headCount;//! Number of heads in the drive
    void *drvCtx;//! driver context
} sysMscRpcInfo_t;

typedef void (*sysMscRpc_Media_cb)(Boolean insert, sysMscRpcInfo_t *info);

//!
//! Function Name:  sysMscRpc_writeSectors 
//!
//! Description:  This sends a write sectors through the IPC to the USB stack on the CP 
//!             which issues the MSC write sectors to the Megasim device 
//!
//! \param[in]   void *buffer - Data to be written to the device
//! \param[in]   UInt32 startSector - start of sector write
//! \param[in]   UInt32 count - number of sectors to write
//! \param[in]   void *vol - context for the host stack
//!
//! \return int result of the write
//!
int sysMscRpc_WriteSectors(void *buffer, UInt32 startSector, int count, void *vol);

//!
//! Function Name:  sysMscRpc_readSectors 
//!
//! Description:  This sends a read sectors through the IPC to the USB stack on the CP 
//!             which issues the MSC read sectors to the Megasim device 
//!
//! \param[inout]   void *buffer - Data to be read from the device
//! \param[in]   UInt32 startSector - start of sector write
//! \param[in]   UInt32 count - number of sectors to write
//! \param[in]   void *vol - context for the host stack
//!
//! \return int result of the write
//!
int sysMscRpc_ReadSectors(void *buffer, UInt32 startSector, int count, void *vol);


//!
//! Function Name:  sysMscRpc_Insert 
//!
//! Description:  This handles the insert event from the USB stack on the CP and stores the 
//!             filesystem info for subsequent reads and writes.
//!
//! \param[in]   InterTaskMsg_t *msg - message for the event
//!
//! \return void
//!
void sysMscRpc_Insert(InterTaskMsg_t *msg);

//!
//! Function Name:  sysMscRpc_Remove
//!
//! Description:  This handles the remove event from the USB stack on the CP.
//!
//! \param[in]   InterTaskMsg_t *msg - message for the event
//!
//! \return void
//!
void sysMscRpc_Remove(InterTaskMsg_t *msg);

//!
//! Function Name:  sysMscRpc_Mount
//!
//! Description:  After an insert a mount message is sent to self so ipc can complete.
//!         This allows FS to run read/writes commands for mount.
//!
//! \param[in]   InterTaskMsg_t *msg - message for the event
//!
//! \return void
//!
void sysMscRpc_Mount(InterTaskMsg_t *msg);

//!
//! Function Name:  sysMscRpc_GetInfo
//!
//! Description:  Retrieves file system info based on name.
//!
//! \param[in]   char *name  - name of filesystem drive
//! \param[out]   sysMscRpcInfo_t **fsInfo - address of pointer to be return with info of drive
//!
//! \return void
//!
Boolean sysMscRpc_GetFsInfo(char *name, sysMscRpcInfo_t **fsInfo);

//!
//! Function Name:  sysMscRpc_IsAttached
//!
//! Description:  Retrieves whether a file system has been attached.
//!
//! \param[in]   char *name  - name of filesystem drive
//!
//! \return Boolean - True - Filesystem is attahed, False - not attached
//!
Boolean sysMscRpc_IsAttached(char *name);

//!
//! Function Name:  sysMscRpc_RegisterMediaCb
//!
//! Description:  Installs callback which notifies upper layer of a volume attach or detach.
//!
//! \param[in]   sysMscRpc_Media_cb  - callback to be installed
//!
//! \return void
//!
void sysMscRpc_RegisterMediaCb(sysMscRpc_Media_cb cb);


#endif //RPC_MSC_MSG_H

