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
*   @file   dialparser_def.h
*
*   @brief  This file contains definitions for the type for dial parser API.
*
****************************************************************************/

#ifndef _DIALPARSER_DEF_
#define _DIALPARSER_DEF_

// ---- Include Files -------------------------------------------------------
//The following header files should be included before include dialedservice_def.h
// "mobcom_types.h"
// "resultcode.h"
// "common_defs.h"
// "ostypes.h"
// "taskmsgs.h"
// "ss_def.h"
// "rtc.h"
// "netreg_def.h" //PLMN
// "ss_api_old.h"


/**	GPRS call parameters 
 **/
typedef struct {
	UInt8	context_id;									///< context ID
}	GPRS_SrvReq_t ;	

/**	Supplementary service call parameters.  \note this is an interim implementation.
 *	\note the 'number' element will be delete once integration of the dialstring-to
 *	SS API is complete.
 */
typedef struct {
	UInt8				number[MAX_DIGITS+2];	///< (Note: 1 byte for null termination and the other for int code '+')
	SS_SsApiReq_t		ssApiReq ;				/**< parsed SS parameters */
}	CallParams_SS_t ;					

/**	Voice/CSD call parameters  \note this is an interim implementation.
 */
typedef struct {
	UInt8				number[MAX_DIGITS+2];///< (Note: 1 byte for null termination and the other for int code '+')
	VoiceCallParam_t	voiceCallParam;
	TelephoneNumber_t	phoneNumber;
}	CallParams_VCSD_t ;					

/**	USSD call parameters  
  */
typedef struct {
	UInt8				number[PHASE2_MAX_USSD_STRING_SIZE+1];		/**< the number */
}	CallParams_USSD_t ;

/**	GPRS call parameters 
 */
typedef struct {
	UInt8				context_id ;			/**< context ID */
}	CallParams_GPRS_t ;	

/** Call parameters parsed from dial string
 */

typedef struct
{
	CallType_t				callType ;			/**< the call type */

	union {
		CallParams_SS_t		ss ;				/**< supplementary service params */
		CallParams_VCSD_t	cc ;				/**< call control params */
		CallParams_USSD_t	ussd ;				/**< USSD params */
		CallParams_GPRS_t	gprs;				/**< GPRS params */
	}	params ;

	SS_SrvReq_t				ssSrvReq;			///< Supplementary Service parameters
	
} CallParams_t;

/**	Voice/CSD call parameters
 **/
typedef struct {
	CLIRMode_t		clir;								///< Calling Line ID Restriction
	UInt8			dtmfLength;							///< Dual-tone multi-frequency length
	DTMFTone_t		dtmfTones[DTMF_TONE_ARRAY_SIZE];	///< Dual-tone multi-frequency tone
	PartyAddress_t	partyAdd;							///< Party Address
}	CC_SrvReq_t ;	
				

#define	MAX_DIAL_STRING	128


/**
 Call parameters parsed from dial string 
**/

typedef struct
{
	CallType_t			callType;						///< the call type

	union {
		CC_SrvReq_t		ccSrvReq;						///< call control params
		SS_SrvReq_t		ssSrvReq;						///< supplementary service params
		GPRS_SrvReq_t	gprsSrvReq;						///< GPRS params
	} params ;

} DailedService_t;


/**
 Diastring parameters for parsing 
**/

typedef struct
{
	PLMN_t	plmn;										///< PLMN, mcc, mnc
	UInt8	dialStrLength;								///< dialed string length
	UInt8	dialStr[MAX_DIAL_STRING];					///< dialed string
} DailStrParam_t;


/**
 PLMN dynamic dialing related data structures start
**/


#ifndef	_WIN32

#define MAX_PLMN_DYNAMIC_DIALSTR_PREFIX_LEN			16
#define MAX_PLMN_DYNAMIC_DIALSTR_DN_LEN				64
#define MAX_PLMN_DYNAMIC_DIALSTR_SUFFIX_LEN			16
#define MAX_PLMN_DYNAMIC_DIALSTR_TABLE_ENTRIES		256

#else

#define MAX_PLMN_DYNAMIC_DIALSTR_PREFIX_LEN			8
#define MAX_PLMN_DYNAMIC_DIALSTR_DN_LEN				20
#define MAX_PLMN_DYNAMIC_DIALSTR_SUFFIX_LEN			8
#define MAX_PLMN_DYNAMIC_DIALSTR_TABLE_ENTRIES		8

#endif	//_WIN32

/**
 PLMN dynamic dialing entry.

 To make/add a valid entry, all following conditions must be satified
 - At least one valid field of prefix/Dialed-Number/suffix exist.
 - Both prefix and suffix are fixed length if exists.
 - If Dail-Number has variable length, at least one of prefix or suffix MUST exist
 - Call type: only CALLTYPE_SPEECH & CALLTYPE_MOUSSDSUPPSVC allowed

 Detail Entry fileds formated rules are commented below.
**/

typedef struct
{
	Boolean		bValid;											///< a valid entry
	PLMN_t		plmn;											///< PLMN, mcc, mnc

	UInt8		prefixLen;										///< When == 0, no prefix
//	Boolean		bPrefixAppend;									///< When TRUE, append with DN/suffix for API output
	UInt8		prefixStr[MAX_PLMN_DYNAMIC_DIALSTR_PREFIX_LEN];		///< any combination of */#/digits

	Int8		DNLen;											///< when == 0: no DN exist,
																///< when  < 0: veriable lenth DN
																///< when  > 0: fix length DN
	UInt8		DNStr[MAX_PLMN_DYNAMIC_DIALSTR_DN_LEN];				///< Dialed Number, \+?[0-9]{0,MAX_PLMN_DIAL_STRING_DN_LEN}

	UInt8		suffixLen;										///< when == 0: no suffix
//	Boolean		bSuffixAppend;									///< When TRUE, append with prefix/DN for API output
	UInt8		suffixStr[MAX_PLMN_DYNAMIC_DIALSTR_SUFFIX_LEN];		///< any combination of */#/digits

	CallType_t	callType;										///< the call type

} PLMN_Dynamic_DialStr_Entry_t;


/**
 Accessing type for PLMN dynamic dialing table
**/

typedef enum 
{
	PLMN_DYNAMIC_DIALSTR_ACCESS_INIT = 0,	///< Init PLMN dynamis dialing table
	PLMN_DYNAMIC_DIALSTR_ACCESS_ADD,		///< Add one entry to PLMN dynamis dialing table
	PLMN_DYNAMIC_DIALSTR_ACCESS_DELETE,		///< Delete one entry to PLMN dynamis dialing table
	PLMN_DYNAMIC_DIALSTR_ACCESS_CLEAN,		///< Clean all entries in PLMN dynamis dialing table
	PLMN_DYNAMIC_DIALSTR_ACCESS_STORE		///< Store table to database
} PLMN_Dynamic_DialStr_Access_t;;


#endif //_DIALPARSER_DEF_
