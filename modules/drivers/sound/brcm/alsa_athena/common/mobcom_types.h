/****************************************************************************
*
*     Copyright (c) 2009-2010 Broadcom Corporation
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
*   @file   mobcom_types.h
*
*   @brief  This file contains the global typedefs of the mobile station
*		    processor software component
*
****************************************************************************/

#ifndef _MOBCOM_TYPES_H_
#define _MOBCOM_TYPES_H_

/** @{ */
typedef char		Int8;
typedef short		Int16;
typedef long		Int32;
typedef unsigned char	UInt8;
typedef unsigned short	UInt16;
typedef unsigned long	UInt32;
typedef char		Boolean;
#define TRUE		1
#define FALSE		0

typedef unsigned int BitField;	        ///< Unsigned Integer

typedef enum
{
	UNICODE_NONE	= 0x00, ///< deprecated (use UNICODE_GSM)
	UNICODE_GSM		= 0x00, ///< 7-bit GSM Default Alphabet
	UNICODE_UCS1	= 0x01, ///< 8-bit Extended ASCII (or ISO-8859-1)

	// Unicode 80, 81, 82 are defined in GSM 11.11 Annex B
	UNICODE_80 		= 0x80,	///< unicode tag 80
	UNICODE_81 		= 0x81,	///< unicode tag 81
	UNICODE_82 		= 0x82,	///< unicode tag 82

	UNICODE_UCS2	= 0xF0, ///< 2 byte unicode characters with native endianess

	UNICODE_RESERVED = 0xFF	///< reserved value
} Unicode_t; ///< unicode types

#endif

