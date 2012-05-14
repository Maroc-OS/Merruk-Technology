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
*   @file   common_sim.h
*
*   @brief  This file defines the data types used in both SIM API functions and 
*			other modules in platform.
*
****************************************************************************/


#ifndef _COMMON_SIM_H_
#define _COMMON_SIM_H_
   

#define MAX_SEEK_RECORD_RSP_LEN	255	///< Maximum number of byte returned in Seek Record Response

#define NUM_OF_MWI_TYPE 4	///< Number of MWI (Message Waiting Type) defined in SIM_MWI_TYPE_t enum

#define	SIM_SMS_REC_LEN	176		///< Record size of EF-SMS
#define	SMSMESG_DATA_SZ	(SIM_SMS_REC_LEN - 1)	///< Data size of an EF-SMS record excluding the first SMS status byte


typedef enum
{
	SIMACCESS_SUCCESS,					///< Access/Verify was successful
	SIMACCESS_INCORRECT_CHV,			///< Verify failed, at least one attempt left
	SIMACCESS_BLOCKED_CHV,				///< Verify failed, CHV is blocked
	SIMACCESS_NEED_CHV1,				///< No access, need CHV1
	SIMACCESS_NEED_CHV2,				///< No access, need CHV2
	SIMACCESS_NOT_NEED_CHV1,			///< CHV1 available and must not be entered
	SIMACCESS_NOT_NEED_CHV2,			///< CHV2 available and must not be entered
	SIMACCESS_NO_ACCESS,				///< No access, not allowed (NVR or ADM)
	SIMACCESS_INVALID,					///< No access, unusable when invalidated file
	SIMACCESS_MEMORY_ERR,				///< Access failed, memory problem
	SIMACCESS_OUT_OF_RANGE,				///< Access failed, invalid address
	SIMACCESS_NOT_FOUND,				///< Access failed, no such file / pattern not found
	SIMACCESS_MAX_VALUE,				///< Increase failed, already at max val
	SIMACCESS_INCORRECT_PUK,			///< Unblock failed, incorrect PUK
	SIMACCESS_BLOCKED_PUK,				///< Unblock failed, PUK is blocked (dead card)
	SIMACCESS_NO_SIM,					///< No SIM
	SIMACCESS_MEMORY_EXCEEDED,			///< SIM Memmory exceeded
	SIMACCESS_CANNOT_REPLACE_SMS,		///< SIM can not replace sms
	SIMACCESS_BLOCKED_CHV2,             ///< CHV2 is blocked
	SIMACCESS_ALREADY_POWERED_ON,       ///< SIM already powered on when a power on request is received
	SIMACCESS_ALREADY_POWERED_OFF,		///< SIM already powered off when a power off request is received
	SIMACCESS_WARNING_CHV,              ///< Security status not satisfied or no info given, i.e. SW1/SW2=0x69/0x82 (3G USIM), SW1/SW2=0x98/0x04 (2G SIM) or SW1/SW2=0x62/0x00
	SIMACCESS_CHV1_DISABLE_NOT_ALLOWED,  ///< Disabling CHV1 is not allowed in EF-SST of 2G SIM 
	SIMACCESS_INVALID_PARAM,			///< Client passed invalid parameter
	SIMACCESS_CHV_CONTRADICTION_ERROR   ///< CHV security status contradiction, e.g. try to change PIN1 while PIN1 is disabled
} SIMAccess_t;							


/*************************Needed by SMS*********************************/
/// SIM Message Waiting Indicator: do not change the following enum values.
/// It is defined to adopt the record index defined in 2G CPHS spec. 
typedef enum
{
	MWI_L1 = 1,		///< MWI flag for Voice L1
	MWI_L2 = 2,		///< MWI flag for Voice L2
	MWI_FAX = 3,	///< MWI flag for Fax
	MWI_DATA = 4	///< MWI flag for Data
} SIM_MWI_TYPE_t;


/// SIM Message Status 
typedef enum
{
	SIMSMSMESGSTATUS_FREE	= 0x00,		///< SMS status is Free
	SIMSMSMESGSTATUS_READ 	= 0x01,		///< SMS status is Read
	SIMSMSMESGSTATUS_UNREAD = 0x03,		///< SMS status is Unread
	SIMSMSMESGSTATUS_SENT	 = 0x05,	///< SMS status is Sent
	SIMSMSMESGSTATUS_UNSENT = 0x07		///< SMS status is Unsent
} SIMSMSMesgStatus_t;					///< SIM SMS Message Status


/* Non-alpha data length in a SIM phonebook EF, e.g. EF-ADN, EF-LND, EF-SDN */
#define SIM_PBK_NON_ALPHA_SZ	14		///< Data size in a phonebook record excluding alpha data
#define SIM_PBK_DIGIT_SZ		10		///< Number of bytes for digits in phonebook record
#define SIM_PBK_ASCII_DIGIT_SZ	(SIM_PBK_DIGIT_SZ * 2)	///< Number of digits allocated in phonebook record


#define ECC_DIGITS						6	///< Maximum number of digits in emergency call code
#define ECC_ENTRY_SIZE	 (ECC_DIGITS / 2)	///< Number of bytes of each emergency call code


#define SIM_EXT_DIGIT_NUM_OF_BYTES	13 ///< Number of bytes in a record of EF-EXTx, e.g. EF-EXT1 & EF-EXT2


#define SIM_PBK_EXT_ASCII_DIGIT_SIZE ((SIM_EXT_DIGIT_NUM_OF_BYTES - 3) * 2) /// Max number of extension digits in EF-EXTx record


/// SIM SMS record data 
typedef struct
{
	SIMSMSMesgStatus_t 	status;			///< SMS Message Status, first byte of SMS record
	UInt8 mesg_data[SMSMESG_DATA_SZ];	///< SMS Message data excluding status byte
} SIMSMSMesg_t;


/// SIM SMS Update Result
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	UInt16		rec_no;			///< 0-based record index
} SIM_SMS_UPDATE_RESULT_t;


/// SIM SMS Data
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	UInt16		rec_no;			///< 0-based record index
	SIMSMSMesg_t sms_mesg;		///< SMS record data
} SIM_SMS_DATA_t;

/// Type of SIM
typedef enum
{
	SIMTYPE_NORMAL_SIM,				///< Normal SIM type
	SIMTYPE_NORMAL_SPECIAL_SIM,		///< Normal Special SIM type
	SIMTYPE_APPROVAL_SIM,			///< Approval SIM (test SIM) type
	SIMTYPE_APPROVAL_SPECIAL_SIM,	///< Approval Special SIM type
	SIMTYPE_SERVICE_CARD_SIM,		///< Service Card SIM type
	SIMTYPE_CELL_TEST_SIM,			///< Cell Test SIM type
	SIMTYPE_NO_CARD					///< No SIM is inserted
} SIMType_t;						///< SIM card type indicated in EF-ADN

#endif  // _COMMON_SIM_H_
