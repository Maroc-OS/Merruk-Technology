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
*   @file   lcs_cplane_rrlp_api.h
*
*   @brief  This header provides the interface description for the control plane Location Service API.
*
****************************************************************************/
/**

*   @defgroup   LCSCPLANERRLPAPIGroup   Control Plane RRLP API
*   @ingroup	LCSAPIGroup

*   @brief      This group defines the interfaces to the control plane RRLP module.

	
*
\section  Messages	

 The following inter-task message should be handled by a registered RRLP message handler.

<H3>
  MSG_LCS_RRLP_DATA_IND
</H3>
*	Payload: LcsCPlaneData_t. 
	\n This message is sent to the RRLP handler whenever a RRLP package is received from network.

<H3>
  MSG_LCS_RRLP_RESET_STORED_INFO_IND
</H3>
*	Payload: None. 
	\n Notify the RRLP handler to clear all position information stored in the GPS receiver.

****************************************************************************/
#ifndef LCS_CPLANE_RRLP_API_H__
#define LCS_CPLANE_RRLP_API_H__		///< Include guard.


// ---- Include Files -------------------------------------------------------
//The following header files should be included before include lcs_cplane_rrlp_api.h
// "mobcom_types.h"
// "resultcode.h"
// "lcs_cplane_shared_def.h"

//*******************************************************************************
/**
* Register a RRLP message handler with the LCS API. 
* This method should be called only once. The registered client should 
* be responsible to handle the following RRLP messages.
* If no RRLP message handler is registered the RRLP messages will not be sent.
*
*	- MSG_LCS_RRLP_DATA_IND                 Payload type {::LcsMsgData_t} The RRLP data received from network
*	- MSG_LCS_RRLP_RESET_STORED_INFO_IND    Payload type {::ClientInfo_t} The request to reset the store position information.
*	.
*
*	@param	inClientInfoPtr	(in)	The info of the client that will handle the RRLP messages.
*
*   @return         The returned value can be RESULT_OK or RESULT_ERROR.
*
*******************************************************************************/
extern Result_t LcsApi_RrlpRegisterDataHandler(ClientInfo_t* inClientInfoPtr);

//*******************************************************************************
/**
*	Request to send the provided RRLP data to control plane network.
*
*	@param inClientInfoPtr	(in) The registered RRLP message handler 
*	@param inData			(in) The buffer containing the RRLP data to be sent.
*	@param inDataLen    	(in) The RRLP data length.
*
*   @return         The returned value can be RESULT_OK or RESULT_ERROR.
*
*******************************************************************************/
extern Result_t LcsApi_RrlpSendDataToNetwork(ClientInfo_t* inClientInfoPtr, const UInt8 *inData, UInt32 inDataLen);

/** @} */

#endif	// LCS_CPLANE_RRLP_API_H__
