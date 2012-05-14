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
*   @file   datacomm_api.h
*
*   @brief  This file defines the interface function for dlink(data connection) API 
			Services.
*
****************************************************************************/
/**

*   @defgroup   DATAConnectionGroup   Data Connection Management
*   @ingroup    DATAServiceGroup
*
*   @brief      This group defines the interface functions to intialize/setup/shutdown  
				data connections
*
*
****************************************************************************/

#ifndef _DATACOMM_API_H
#define _DATACOMM_API_H


/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

/**
	This function handles data setup request. ATC/MMI must register their message posting callback function by using
	inClientId = SYS_RegisterForMSEvent(). ATC/MMI must store the ID for later purpose(like DeRegister).Data service 
	layer will notify application clients the data connection status with asynchronous events (refer to DC_ReportCallStatus_t)
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param acctId		(in)	Data account id
	@param linkType		(in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server);								
								DC_MS_INITIATED=1, when use as GPRS phone application
	@return Result_t, Return PDP_CIRCUIT_CALL_ACTIVE if there is a voice or CSD call active and it is not UMTS network
					  Otherwise return generic RESULT_OK or RESULT_ERROR depending on DC setup conditions
**/
Result_t DC_SetupDataConnection(	UInt8							inClientID,
									UInt8							acctId, 
									DC_ConnectionType_t				linkType );

/**
	This function handles data setup requests for clients preferring to share a PDP context and APN. If no match is found, a new
    connection is established. Data service layer will notify application clients the data connection status with asynchronous 
    events (refer to DC_ReportCallStatus_t)
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param acctId		(in)	Data account id
	@param linkType		(in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server);								
								DC_MS_INITIATED=1, when use as GPRS phone application
    @param apnCheck     (in)    APN associated with the connection client wants to share
    @param actDCAcctId  (out)   Account id of the active connection that is shared
	@return Result_t, Return PDP_CIRCUIT_CALL_ACTIVE if there is a voice or CSD call active and it is not UMTS network
					  Otherwise return generic RESULT_OK or RESULT_ERROR depending on DC setup conditions
**/
Result_t DC_SetupDataConnectionEx(	UInt8					inClientID,
									UInt8					acctId, 
									DC_ConnectionType_t		linkType,
                                    UInt8 *apnCheck, 
                                    UInt8 *actDCAcctId );

/**
	This function handles PDP context modification. User need to call DATA_SetGPRSQos() to set the new QoS parameters
	before calling this API to modify the QoS of the corresponding PDP context
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param acctId		(in)	Data account id
	@param modifyCb 		(in) modify call back
	@return Result_t, Return RESULT_ERROR if the context can not be modified (ex. if the context is not active)
					  Otherwise return RESULT_OK 
**/
Result_t DC_ModifyDataConnection(	UInt8							inClientID,
									UInt8							acctId, 
									PCHGPRS_ModifyCb_t				modifyCb );


/**
	This function handles data shutdown request

	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param acctId		(in)	Data account id
	@return Result_t
**/
Result_t DC_ShutdownDataConnection(	UInt8							inClientID,								   
									UInt8							acctId );


/**
	This function retrieves the P-CSCF address received from the network upon successful PDP activation.

	@param acctId		(in)	Data account id
	@return const UInt8 *
**/
const UInt8 *DC_GetPCscfAddr(UInt8 acctId);

#ifdef DUAL_SIM_DATACOMM
Result_t DcApi_SetupDataConnection(	ClientInfo_t* inClientInfoPtr,
									UInt8					inAcctId, 
									DC_ConnectionType_t		linkType,		
									Result_t				rejectCode );
									
Result_t DcApi_ShutdownDataConnection(ClientInfo_t* inClientInfoPtr,	
								   UInt8	acctId );
#endif

/** @} */


#endif //_DATACOMM_API_H


