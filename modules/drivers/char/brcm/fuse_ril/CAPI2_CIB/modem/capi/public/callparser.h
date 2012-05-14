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
*   @file   callparser.h
*
*   @brief  This file contains definitions for Call Parser.
*
****************************************************************************/
#ifndef _CALLPARSER_H_
#define _CALLPARSER_H_


//******************************************************************************
// Enumerations
//******************************************************************************
#define	MAX_PARM_LIST			4
#define	USSD_LEN				40


//******************************************************************************
// Structure Typedefs
//******************************************************************************
typedef UInt8	PWord_t[SS_PASSWORD_LENGTH+1];				// Password type

typedef UInt16	NoReplyCondTime_t;

typedef enum
{
	BSTYPE_BEARER			= 2,	// Bearer Service
	BSTYPE_TELE				= 3,	// Tele Service
	BSTYPE_UNKNOWN			= 0xFF	// Unknown Type
} BSType_t;

typedef struct
{
	BSType_t		type;
	UInt8			service;
} BService_t;


typedef struct
{
	Boolean			is_used;
	BService_t		value;
} OptBService_t;


typedef struct
{
	Boolean				is_used;
	TelephoneNumber_t	value;
} OptRegCFNum_t;

typedef struct
{
	Boolean			is_used;
	NoReplyCondTime_t value;
} OptNoReplyCondTime_t;


typedef struct
{
	SuppSvcType_t	ss_type;			// SS Type 
	SSAction_t		ss_action;			// SS Action to perform
	UInt8			total_parm;			// running total number of parameters
	UInt8			ussd[PHASE2_MAX_USSD_STRING_SIZE+1];	// for USSD data
	OptBService_t	bs_code;
	OptRegCFNum_t	forwarded_num;
	OptNoReplyCondTime_t reply_timeout;
	PWord_t			old_password;		// used for BAR_PASSWORD and other Barring
	PWord_t			new_password1;		// used for BAR_PASSWORD
	PWord_t			new_password2;		// used for BAR_PASSWORD
	SuppSvcType_t	bar_ss_type;		// used for BAR_PASSWORD
	Boolean			pound_end_flag;		// TRUE if the string is ended with '#'
} SSData_t;

typedef struct
{
	TelephoneNumber_t phone_number;
	CLIRMode_t		clir_mode;
	DTMFTone_t		dtmfTones[DTMF_TONE_ARRAY_SIZE];
	UInt8			dtmfLen;	
} CallData_t;
	
typedef struct
{
	CallType_t		type;
	SS_SrvReq_t		ssSrvReq;
	SSData_t		ss_data;
	CallData_t		call_data;
} CallCommand_t;


//******************************************************************************
// Function Prototypes 
//******************************************************************************

void CALLPARSER_ParseCmdStr(
	CallCommand_t *p_ccmd,				// pointer to parsed command
	UInt8 *input_str					// pointer to input string
	);

void ConvertPhoneNumber(const UInt8* s, TelephoneNumber_t* d);

#endif // _CALLPARSER_H_

