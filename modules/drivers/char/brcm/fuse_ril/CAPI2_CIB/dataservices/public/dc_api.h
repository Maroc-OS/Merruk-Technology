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
*  @file   dc_api.h
*
*  @brief  API to the data connections.
*
*  @note   
*
****************************************************************************/

#ifndef _DC_API_H
#define _DC_API_H

// ---- Include Files -------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

// ---- Macro Declarations -------------------------------------------------------

// ---- Type Definitions -------------------------------------------------------

/// Description of Param1_t should be included here
/// the description can be multiple lines and is terminated when a blank
/// line is encountered.


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

/** @} */


/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

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

/** @} */

typedef struct
{
    UInt32 pri_addr;	// primary DNS server address
    UInt32 sec_addr;	//Secondary DNS server addres
} DcDNSServerAddr_t;

/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

/**
	This function handles data setup request. ATC/MMI must register their message posting callback function by using
	inClientId = SYS_RegisterForMSEvent(). ATC/MMI must store the ID for later purpose(like DeRegister).Data service 
	layer will notify application clients the data connection status with asynchronous events (refer to DC_ReportCallStatus_t)
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param inAcctId		(in)	Data account id
	@param inLinkType	(in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server);								
								DC_MS_INITIATED=1, when use as GPRS phone application
	@return Result_t, Return PDP_CIRCUIT_CALL_ACTIVE if there is a voice or CSD call active and it is not UMTS network
					  Otherwise return generic RESULT_OK or RESULT_ERROR depending on DC setup conditions
**/
Result_t DC_SetupDataConnection(	UInt8							inClientID,
									UInt8							inAcctId, 
									DC_ConnectionType_t				inLinkType );

/**
	This function handles data setup requests for clients preferring to share a PDP context and APN. If no match is found, a new
    connection is established. Data service layer will notify application clients the data connection status with asynchronous 
    events (refer to DC_ReportCallStatus_t)
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param inAcctId		(in)	Data account id
	@param inLinkType		(in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server);								
								DC_MS_INITIATED=1, when use as GPRS phone application
    @param inApnCheck     (in)    APN associated with the connection client wants to share
    @param inAcctDCAcctId  (out)   Account id of the active connection that is shared
	@return Result_t, Return PDP_CIRCUIT_CALL_ACTIVE if there is a voice or CSD call active and it is not UMTS network
					  Otherwise return generic RESULT_OK or RESULT_ERROR depending on DC setup conditions
**/
Result_t DC_SetupDataConnectionEx(	UInt8					inClientID,
									UInt8					inAcctId, 
									DC_ConnectionType_t		inLinkType,
                                    UInt8 *inApnCheck, 
                                    UInt8 *inAcctDCAcctId );
/**
	This function handles data shutdown request

	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param inAcctId		(in)	Data account id
	@return Result_t
**/
Result_t DC_ShutdownDataConnection(	UInt8							inClientID,								   
									UInt8							inAcctId );

/** @} */


/**
	This function initializes the data connections
**/
void DC_InitDataConnections(void);



/**
	This function cleans up the active data connections
**/
void DC_DeInitDataConnections(void);

/**
	This function is used to retrieve the DNS server addresses after data connection is setup.

	@param inAcctId		(in)	Data account id
	@return const UInt8 *
**/
DcDNSServerAddr_t DC_GetDnsServerAddr(UInt8 inAcctId);
/**
	This function is get the index of the tcp/ip stack interface for the data connection
	identified by the accountId. 

	@param inAcctId		(in)	Data account id
	@return -1 if error else the interface index/number in the tcp/ip stack interface list.
**/
int DC_GetNetIfaceIndex(UInt8 inAcctId);

/**
	This function gets the accountId from the interface index of the tcp/ip stack interface 

	@param inIface		(in)	TCP/IP stack based interface Id
	@return INVALID_ACCT_ID if error else the interface index/number in the tcp/ip stack interface list.
**/
UInt8 DC_GetAcctIdByNetIfaceIndex(int inIface);

/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

/**
	This function retrieves the P-CSCF address received from the network upon successful PDP activation.

	@param inAcctId		(in)	Data account id
	@return const UInt8 *
**/
const UInt8 *DC_GetPCscfAddr(UInt8 inAcctId);


/** @} */

#ifdef __cplusplus
}
#endif

#endif

