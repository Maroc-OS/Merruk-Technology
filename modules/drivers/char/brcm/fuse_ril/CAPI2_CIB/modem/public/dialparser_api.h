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
*   @file   dialparser_api.h
*
*   @brief  This file defines the dial parser API.
*
****************************************************************************/
/**
*
*   @defgroup   DialedServiceGroup	Mobile Station DialedService APIs
*   @ingroup    SystemGroup				
*
*   @brief      This group provides dialing related API functions.
*
****************************************************************************/

#ifndef __DIALEDSERVICE_API_H__
#define __DIALEDSERVICE_API_H__


/**
 * @addtogroup DialedServiceGroup
 * @{
 */ 

//**************************************************************************************
/**
	This function will parse the dialed string and return detail instrction and infomation of a call.

  	@param		inClientInfoPtr		(in) pointer to the client Info
	@param		inDailStrParamPtr	(in) dialed string related paramaters
	@param		outDailSrvPtr		(out) result of parsed dialed string detail infomation

**/

void UtilApi_DialParser(
			ClientInfo_t*		inClientInfoPtr,
			DailStrParam_t*		inDailStrParamPtr,
			DailedService_t*	outDailSrvPtr
			);


//**************************************************************************************
/**
	This function access the PLMN dynamic dialed string entry table

  	@param		inClientInfoPtr		(in) pointer to the client Info
	@param		inAccessType		(in) access type
	@param		inPlmnEntryPtr		(in) entry need to be added/deleted
	@param		inTableIndex		(in) index of the table entry, 0 base.

	@return		RESULT_OK or NOT

**/


Result_t UtilApi_DialParserDynamicAccess(
				ClientInfo_t*				inClientInfoPtr,
				PLMN_Dynamic_DialStr_Access_t		inAccessType,
				PLMN_Dynamic_DialStr_Entry_t*		inPlmnEntryPtr,
				UInt16								inTableIndex
				);


//**************************************************************************************
/**
	This function search the PLMN dynamic dialed string entry table see if an entry exist

  	@param		inClientInfoPtr		(in) pointer to the client Info
	@param		inDailStrParamPtr	(in) entry need to be lookup
	@param		outDailSrvPtr		(out) result of found and parsed dialed string infomation

	@return		RESULT_OK or NOT

**/


Result_t UtilApi_DialParserDynamicLookup(
				ClientInfo_t* inClientInfoPtr,
				DailStrParam_t* inDailStrParamPtr,
				DailedService_t* outDailSrvPtr
				);


//**************************************************************************************
/**
	This generic function will parse the dialed string and return detail infomation of a call.

	@param		inDialStr		(in) dialed string
	@param		inDialStrLen	(in) dialed string length
	@param		outDailSrvPtr	(out) result of parsed dialed string detail infomation

	@return		none

**/

void UtilApi_DialParserGen(
				UInt8*				inDialStr,
				UInt8				inDialStrLen,
				DailedService_t*	outDailSrvPtr
				);

/** @} */


#endif	//	__DIALEDSERVICE_API_H__



