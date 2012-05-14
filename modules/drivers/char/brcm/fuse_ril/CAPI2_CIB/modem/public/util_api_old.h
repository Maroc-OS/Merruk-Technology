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
*   @file   util_api_old.h
*
*   @brief  This file defines the utility functions used by the platform.
*
********************************************************************************************/
#ifndef _UTIL_API_OLD_H_
#define _UTIL_API_OLD_H_


//***********************************************************************************
/**
	Util_DialStr2PartyAdd() This function parses the dialed string, and coverts it to
	partry address structure, which  consists of Type of Number (TON), Numbering Plan
	Identification (NPI) and number of digits. See GSM 02.30, Section 2.3 for encoding
	rules.
	@param		outPartyAddPtr (out) Called/Calling party phone address pointer.
	@param		inDataPtr (in) Pointer to the dialed string.
	@return		void (out)
**/
void Util_DialStr2PartyAdd(	PartyAddress_t*	outPartyAddPtr,
							const UInt8*	inDataPtr);




UInt8 UTIL_GetNumOffsetInSsStr(const char *ssStr);


NetworkCause_t UTIL_Cause2NetworkCause(Cause_t inCause);

NetworkCause_t UTIL_ErrCodeToNetCause(SS_ErrorCode_t inErrCode);

const UInt8* UTIL_GetTeToGsmConvTbl ( void );
UInt8* UTIL_GetGsmToTeConvTbl ( void );


Boolean UCS2_CheckAlphaValidity(ALPHA_CODING_t coding, const UInt8 *alpha, UInt16 alpha_size);

Boolean UCS2_ConvertToUcs2_80( ALPHA_CODING_t coding, const UInt8 *alpha, UInt16 alpha_size, 
		UInt8 *alpha_out, UInt16 *alpha_size_out );

char* UCS2_ConvertToAlphaStr(Boolean ucs2, ALPHA_CODING_t coding, const UInt8 *alpha, UInt16 alpha_size); 

Boolean UCS2_ConvertToAlphaData( Boolean ucs2, char *alpha_str, 
		ALPHA_CODING_t *coding, UInt8 *alpha_out, UInt16 *alpha_len );

Int16 UCS2_AlphaCompare(UInt8 *a, UInt8 *b, UInt16 size_a, UInt16 size_b, Boolean partial_match);

UInt16 UCS2_AlphaDataCpy(ALPHA_CODING_t coding, UInt8 *alpha_dest, UInt8 *alpha_src, UInt16 dest_len, UInt16 src_len);

#endif //_UTIL_API_OLD_H_

