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
*   @file   datacomm_def.h
*
*   @brief  This file defines the interface difinitions for dlink(data connection) API 
			Services.
*
****************************************************************************/

#ifndef _DATACOMM_DEF_H
#define _DATACOMM_DEF_H


/**
Data connection status
**/
typedef enum
{
	DC_STATUS_CONNECTING,			///< data connection is in connecting state
	DC_STATUS_DIALING,				///< 
	DC_STATUS_INITIALIZING,			///< 
	DC_STATUS_VERIFYING,			///< data connection enter this state when csd data call to isp just return CONNECT
	DC_STATUS_VERIFIED,				///< 
	DC_STATUS_CONNECTED,			///< data connection is connected
	DC_STATUS_DISCONNECTING,		///< data connection is connecting
	DC_STATUS_DISCONNECTED,			///< data connection is disconnected
	DC_STATUS_ERR_NETWORK_FAILURE,	///< 
	DC_STATUS_ERR_CALL_BARRED,		///< 
	DC_STATUS_ERR_NO_ACCOUNT,		///< data connection failed due to invalid data account id
	DC_STATUS_ERR_NO_CARRIER,		///< data connection failed due to no carrier
	DC_STATUS_ERR_DATA_LINK_BROKEN,	///< 
	DC_STATUS_ERR_NO_CONNECTION		///< 
}DC_ConnectionStatus_t;


/**
Data connection type
**/
typedef enum
{
	DC_MODEM_INITIATED,	///< if the phone is used as modem (ppp server)
	DC_MS_INITIATED		///< if the connection is setup for data over WEDGE/GPRS (such as WAP)
}DC_ConnectionType_t;

/**
Asynchronous message to notify clients data connection information such as connection status, 
source IP, PCH reject code etc.
**/
typedef struct
{
	UInt8					acctId;			///< Data account id
	DC_ConnectionStatus_t		status;			///< Data connection status
	Result_t					rejectCode; 		///< Rejection code
	UInt32					srcIP;			///< Source IP of the phone from PDP activation 
} DC_ReportCallStatus_t;


#endif //_DATACOMM_DEF_H


