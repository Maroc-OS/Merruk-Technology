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
*   @file   dialstr.h
*
*   @brief  This file contains definitions for Dial string parsing API
*
*	@note	This is an interim version that is to be used only for ATD parsing
*
****************************************************************************/

#ifndef _DIALSTR_H_
#define _DIALSTR_H_








/** Parse a dial string.  The return value is the call type.  
 *	The DialParms_t argument contains the parameters associated 
 *	with the call type.
 */
CallType_t DIALSTR_Parse( 
	const UInt8			*dialStr,				/**< the dial string */
	CallParams_t		*callParams				/**< the call parameters */
	) ;


/**	Convert call parameters to a dial string.  \note This function
 *	is required by STK and should not be published as an API function.
 */

Result_t DIALSTR_CvtCallParamsToDialStr( 
	CallParams_t		*callParams,			/**< the call parameters */
	UInt8				dialStr [MAX_DIGITS+2] 	///< (Note: 1 byte for null termination and the other for int code '+')
	) ;

Result_t DIALSTR_DialStrParse( 
	const UInt8*		dialStr,
	CallParams_t*		callParams, 
	Boolean*			voiceSuffix
	);

	
void SS_SsType2Reason(SS_CallFwdReason_t* inReasonPtr, SuppSvcType_t inSsType);
void SS_BarSsType2CallBarType(SS_CallBarType_t*	inCallBarPtr, SuppSvcType_t inBarSsType);
UInt8* SsApiCmd2String(SS_SsApiReq_t* inSsApiReqPtr, UInt8* inDialStrPtr);
void SS_BsCode2SvcCls(SS_SvcCls_t* inSvcClsPtr, OptBService_t* inBasicSvcPtr);
void SS_SsData2SsApiReq(SSData_t* inSsDataPtr, SS_SsApiReq_t* inSsApiReqPtr);


//NEW API WITH ClientInfo
CallType_t CcApi_DialStrParse(ClientInfo_t*	inClientInfoPtr, const UInt8* dialStr, CallParams_t* callParams);

#endif // _DIALSTR_H_

