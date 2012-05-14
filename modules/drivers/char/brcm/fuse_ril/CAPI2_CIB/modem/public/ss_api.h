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
*   @file   ss_api.h
*
*   @brief  This file defines the interface for Supplementary Services API.
*
****************************************************************************/ 
/**
*   @defgroup   SSAPIGroup   Supplementary Services
*   @ingroup    TelephonyGroup
*
*   @brief      This group defines the interfaces to the supplementary
*				services group.
*
*				This group defines the interface for supplementary services API.
*				It provides the function prototypes necessary to
*				activate, register, interrogate, enable, and disable supplementary
*				services such as call forwarding, call barring, calling line
*				presentation for speech, data, fax and other call types. 
*****************************************************************************/
#ifndef _SS_API_H_
#define _SS_API_H_




//**************************************************************************************
//*****************************************NEW SS***************************************
//**************************************************************************************
//**************************************************************************************

/**
 * @addtogroup SSAPIGroup
 * @{
 */

/**
    Function to initiate a SS or USSD session with the network. The response
    primitive which caries the network respone back to the client is as follows:
    For SS/USSD related cases:
        MSG_MNSS_CLIENT_SS_SRV_RSP (for Facility msg from the NW) or
        MSG_MNSS_CLIENT_SS_SRV_REL (for Release Complete msg from the network)
 
    For MO/MT-LR related cases:
        MSG_MNSS_CLIENT_LCS_SRV_RSP (for Facility msg from the NW) or
        MSG_MNSS_CLIENT_LCS_SRV_REL (for Release Complete msg from the network)

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvReqPtr (in) Pointer to the SS API service request object.
	@return		Result_t
**/
Result_t SsApi_SsSrvReq(ClientInfo_t*	inClientInfoPtr,
						SsApi_SrvReq_t* inApiSrvReqPtr);


/**
	Function to continues the current SS session with the network. The response
	primitive which caries the network respone back to the client;
	For SS/USSD related cases:
		MSG_MNSS_CLIENT_SS_SRV_RSP (for Facility msg from the NW) or
		MSG_MNSS_CLIENT_SS_SRV_REL (for Release Complete msg from the network)

	For MO/MT-LR related cases:
		MSG_MNSS_CLIENT_LCS_SRV_RSP (for Facility msg from the NW) or
		MSG_MNSS_CLIENT_LCS_SRV_REL (for Release Complete msg from the network)
	These messages are broadcast to all the registered clients.

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDataReqPtr (in) Pointer to the SS API data request object.
	@return		Result_t
**/
Result_t SsApi_DataReq(	ClientInfo_t*		inClientInfoPtr,
						SsApi_DataReq_t*	inDataReqPtr);


//**************************************************************************************
/**
	Function to initiate a new USSD request. which causes the MNSS state machine to
	establish a USSD session.
    The response primitive which carries the network respone back to the client:

		MSG_MNSS_CLIENT_SS_SRV_RSP (for Facility msg from the network) or
		MSG_MNSS_CLIENT_SS_SRV_REL (for Release Complete msg from the network).
	These messages are broadcast to all the registered clients.


	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvReqPtr (in) Pointer to the USSD service request object.
	@return		Result_t

	@sa 		::SsApi_UssdDataReq()
**/
Result_t SsApi_UssdSrvReq(	ClientInfo_t*		inClientInfoPtr,
							SsApi_UssdSrvReq_t* inApiSrvReqPtr);


//**************************************************************************************
/**
	Function to continues the current USSD session with the network
	The response primitive which carries the network respone back to the client;
		MSG_MNSS_CLIENT_SS_SRV_RSP (for Facility msg from the network) or
		MSG_MNSS_CLIENT_SS_SRV_REL (for Release Complete msg from the network).
		
	These messages are broadcast to all the registered clients.

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvDataReqPtr (in) Pointer to the USSD data request object.
	@return		Result_t

	@sa			::SsApi_UssdSrvReq()
**/
Result_t SsApi_UssdDataReq(	ClientInfo_t*			inClientInfoPtr,
							SsApi_UssdDataReq_t*	inApiSrvDataReqPtr);


//**************************************************************************************
/**
	Function to initiate a release request, which causes the MNSS state machine to
	send a release complete message to the network. There is no response for this request.

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiRelReqPtr (in) Pointer to the SS API service request object.
	@return		Result_t
**/
Result_t SsApi_SsReleaseReq(ClientInfo_t*	inClientInfoPtr,
							SsApi_SrvReq_t* inApiRelReqPtr);


//**************************************************************************************
/**
	Function pars the SS String, then it initiats a new SS request, which causes the
	MNSS state machine to establish a SS session.

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDialStrSrvReqPtr (in) Dial Str Service Request Pointer.
	@return		Result_t
**/
Result_t SsApi_DialStrSrvReq(	ClientInfo_t*			inClientInfoPtr,
								SsApi_DialStrSrvReq_t*	inDialStrSrvReqPtr);

/** @} */



#endif // _SS_API_H_

