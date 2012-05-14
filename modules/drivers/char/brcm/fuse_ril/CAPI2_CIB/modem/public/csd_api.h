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
*   @file   csd_api.h
*
*   @brief  This file defines APIs for CSD data access.
*
****************************************************************************/
/**

*   @defgroup   DATACSDGroup   CSD Data Access
*   @ingroup    DATAServiceGroup
*
*   @brief      This group defines APIs for CSD data access
*
*	The whole CSD call is divided into two setups --- 
*	CSD call setup and CSD data access

*	1. The CSD setup is accomplished by the API ::CC_MakeDataCall / ::CC_AcceptDataCall and 
*	message ::MSG_DATACALL_CONNECTED_IND. 
*
*	2. The CSD API will be a thin data access layer to shield the ecdc/v24 detail 
*	and is parallel with the current MPX/NON_MUX code. It could be used by 
*	WAP client and other clients which needs to access CSD data.
*
*	Stack <--> ECDC <-->V24<-->CSD API <---->WAP client\n
*	Stack <--> ECDC <-->V24<-->MUX/NON_MUX-->UART
*
*
****************************************************************************/
#ifndef __CSD_API_H__
#define __CSD_API_H__

typedef enum
{
	CSDLINESTATE_LINE_ON = 0,	// on-line
	CSDLINESTATE_LINE_OFF		// off-line
} CSDLineState_t;

/**
 * @addtogroup DATACSDGroup
 * @{
 */

/**
Data callback function prototype
@param clientID		(in)	Specify Client interesting in the CSD data
@param newDataSize	(out)	Received ata size
@return
**/
typedef void (*DataIndCallBackFunc_t)(UInt8 clientID, UInt16 newDataSize);

/**
//Send data pointed by the data pointer to the network(air interface).

//The caller should use the returned size as the indication of data buffer 
//availability.The caller can release its buffer after this API call 
//without affecting the data transfer.

@param clientID	(in) Specify Client to send the data to the network       
@param data		(in) date pointer 
@param dataSize	(in) size of the sent data
@return	Return the size of the data that is successfully sent. 
**/
UInt16 CSD_SendData(UInt8 clientID, UInt8 *data, UInt16 dataSize);

/**
//Check the data buffer space for sending     
//The client can call this API to check the free space 
//in the data buffer before sending the data using API CSD_SendData.

@param clientID (in) Specify Client to query the free space. 
@return	Return the size of the free buffer space for sending 
**/
UInt16 CSD_BufferFreeSpace(UInt8 clientID);

/**
//Register the callback to receive the indication of data available 
//from the network(air interface)

@param clientID (in) Specify Client interesting in the CSD data
@param dataIndFun (in) callback function to inform the availability of the data.
@return	Return RESULT_OK if the registration is successful. Otherwise RESULT_ERROR
**/

Result_t CSD_RegisterDataInd (UInt8 clientID, DataIndCallBackFunc_t dataIndFun);

/**
//The client of API should manage the receiver buffer after the data has been 
//processed in the buffer. Usually, After the client receives the data available 
//indication through the callback registered with CSD_RegisterDataInd API, 
//It calls this API to retrieve the available data based on it's capacity to 
//handle the data 

@param clientID		(in)	Specify Client to receive the data from the network(air interface).
@param recvDataBuf	(out)	The pointer to the receiver buffer.
@param readSize     (in)	number of bytes to retrieve from data buffer.
@return	Return the real available data size in the recvDataBuf.
**/

UInt16  CSD_GetData(UInt8 clientID, UInt8* recvDataBuf, UInt16 readSize);

typedef void (*DataIndCallBackFuncEx_t)(DLC_t dlc, UInt16 newDataSize);
typedef void (*FCIndCallBackFunc_t)(DLC_t dlc, CSDLineState_t fc);
typedef void (*BuffRdyCallBackFunc_t)(DLC_t dlc);


DLC_t CsdApi_RegisterDataInd (ClientInfo_t *clientInfoPtr, DataIndCallBackFuncEx_t dataIndFun, FCIndCallBackFunc_t FCIndFun);
Result_t CsdApi_RegisterBufRdy (ClientInfo_t *clientInfoPtr,  BuffRdyCallBackFunc_t buffRdyFun);
Result_t CsdApi_DeregisterDataInd (ClientInfo_t *clientInfoPtr);

Int16  CsdApi_GetData(DLC_t dlc, UInt8* recvDataBuf, UInt16 readSize);
Int16 CsdApi_GetBufferFreeSpace(DLC_t dlc);
Int16 CsdApi_SendData(DLC_t dlc, UInt8 *data, UInt16 dataSize);


typedef UInt16 (*VTIndCallBackFunc_t)(DLC_t dlc, UInt8 *data, UInt16 newDataSize);

DLC_t CsdApi_RegisterVTDataInd (ClientInfo_t *clientInfoPtr, VTIndCallBackFunc_t vtDataIndFun, VTIndCallBackFunc_t buffRdyFun);


/** @} */
#endif

