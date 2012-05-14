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
*   @file   common_ecdc.h
*
*   @brief  This file defines structures for call configuration used by both CAPI and CAPI2
*
****************************************************************************/


#ifndef _COMMON_ECDC_
#define _COMMON_ECDC_

/**
	Error correction parameter requested

	- EC: either V42 or MNP protocol
	- AUTO: try EC and if EC fails fallback to Normal or Direct
	- RELIABLE: try EC and if EC fails Disconnect
	- DIRECT: No EC, non-buffered (not supported by our MS)
	- NORMAL: No EC, buffered
	- V42: only V42 protocol is used for Error Correction
	- MNP: only one of the MNP protocols is used for Error Correction
**/
typedef enum
{
	ECORIGREQ_DIRECT = 0,			///<  No Error Correction, DTE-DCE match line rate
	ECORIGREQ_NORMAL = 1,			///< No Error Correction, buffered
	ECORIGREQ_V42_NO_DETECTION = 2,	///< V42 Error Correction, no detection phase
	ECORIGREQ_V42 = 3,				///< V42 Error Correction
	ECORIGREQ_ALTERNATIVE = 4		///< Alternative Error Correction, i.e. MNP
} ECOrigReq_t;						///< Originator Request, see AT+ES

/**
	Error correction parameter fall back value if negotiation is not possible

	- EC: either V42 or MNP protocol
	- AUTO: try EC and if EC fails fallback to Normal or Direct
	- RELIABLE: try EC and if EC fails Disconnect
	- DIRECT: No EC, non-buffered (not supported by our MS)
	- NORMAL: No EC, buffered
	- V42: only V42 protocol is used for Error Correction
	- MNP: only one of the MNP protocols is used for Error Correction
**/
typedef enum
{
	ECORIGFB_AUTO_NORMAL = 0,		///< MO, if EC fails, Normal mode
	ECORIGFB_AUTO_DIRECT = 1,		///< MO, if EC fails, Direct mode
	ECORIGFB_RELIABLE = 2,			///< MO, if EC fails, Disconnect
	ECORIGFB_RELIABLE_V42 = 3,		///< MO, if V42 fails, Disconnect
	ECORIGFB_RELIABLE_MNP = 4		///< MO, if MNP fails, Disconnect
} ECOrigFB_t;						///< Originator Fallback, see AT+ES

/**
	Error correction parameter answer fall back value

	- EC: either V42 or MNP protocol
	- AUTO: try EC and if EC fails fallback to Normal or Direct
	- RELIABLE: try EC and if EC fails Disconnect
	- DIRECT: No EC, non-buffered (not supported by our MS)
	- NORMAL: No EC, buffered
	- V42: only V42 protocol is used for Error Correction
	- MNP: only one of the MNP protocols is used for Error Correction
**/
typedef enum
{
	ECANSFB_DIRECT = 0,				///< MT, Direct
	ECANSFB_NORMAL = 1,				///< MT, Normal
	ECANSFB_AUTO_NORMAL = 2,		///< MT, EC mode, if EC fails, then Normal
	ECANSFB_AUTO_DIRECT = 3,		///< MT, EC mode, if EC fails, then Direct
	ECANSFB_RELIABLE = 4,			///< MT, V42/MNP mode, if EC fails, Disconnect
	ECANSFB_RELIABLE_V42 = 5,		///< MT, V42 mode, if V42 fails, Disconnect
	ECANSFB_RELIABLE_MNP = 6		///< MT, MNP mode, if MNP fails, Disconnect
} ECAnsFB_t;						///< Answer Fallback, see AT+ES

/// Error Correction Mode
typedef enum
{
	ECMODE_UNKNOWN = 0,  			///< Unknown,
	ECMODE_DIRECT = 1,  			///< Direct, No Error Correction, DTE-DCE match line rate
	ECMODE_NORMAL = 2,  			///< Normal, No Error Correction, buffered
	ECMODE_MNP2 = 3,	   			///< MNP2
	ECMODE_MNP3 = 4,				///< MNP3
	ECMODE_V42 = 5		   			///< V42, also called LAPM
} ECMode_t;

///	Data Compression Mode
typedef enum
{
	DCMODE_NONE = 0,				///< No Data Compression
	DCMODE_COMPRESSION_ONLY = 1,	///< Data Compression Only
	DCMODE_DECOMPRESSION_ONLY = 2,	///< Data Decompression Only
	DCMODE_BOTH = 3					///< Compression/Decompression
} DCMode_t;							///< Data Compression Mode (P0), see AT+DS

/// Error correction Parameters
typedef struct
{
	  ECOrigReq_t  ES0;		///< Error correction parameter requested
	  ECOrigFB_t   ES1;		///< Error correction parameter fall back value if negotiation is not possible
	  ECAnsFB_t	   ES2;		///< Error correction parameter answer fall back value
}ECInfo_t;


#endif
