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
*   @file   phonebk_types.h
*
*   @brief  This file contains definitions for phonebook data types.
*
****************************************************************************/

#ifndef _PHONEBK_TYPES_H_
#define _PHONEBK_TYPES_H_


//******************************************************************************
//							include block
//******************************************************************************



/**
	struct for phone book extended BCD number
**/
typedef struct
{
	gsm_TON_t		ton;	                    ///< GSM Type of Number
	gsm_NPI_t		npi;	                    ///< GSM Number Plan Identifier
	UInt8			digits[SIM_PBK_DIGIT_SZ + SIM_PBK_EXT_BCD_DIGIT_SIZE];	///< Digit data that may include EF-EXTx extension
    UInt8			ext_rec_index;              ///< Extended record index
} PBK_EXT_BCD_Number_t;

/**
	struct for phone book ADN data
**/
typedef struct
{
	PBK_Name_t				adn_name;	///< ADN alpha data
	PBK_EXT_BCD_Number_t	adn_num;	///< ADN digit data
} PBK_ADN_DATA_t;



#endif

