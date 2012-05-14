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
*   @file   mndstypes.h
*
*   @brief  This file contains definitions for MNDS types.
*
****************************************************************************/

#ifndef _MSC_MNDSTYPES_H_
#define _MSC_MNDSTYPES_H_



//******************************************************************************
//
//	Typedefs for MNDS Messages
//
//******************************************************************************


//******************************************************************************
//
// Structure Typedefs
//
//******************************************************************************


//******************************************************************************
//
// MNDS Message Type Enumerations
//
//******************************************************************************

typedef enum
{
										// System messages:
										// ================
	MNDSMSG_NULL,						// Null message

										// DS messages:
										// =============
	MNDSMSG_DS_CALL_ACC,
	MNDSMSG_DS_DISC_REQ,
	MNDSMSG_DS_SERVICE_ACC,
	MNDSMSG_DS_SERVICE_REJ,
	MNDSMSG_DS_SETUP_REQ,
										// MN messages:
										// =============
	MNDSMSG_MN_BEARER_IND,
	MNDSMSG_MN_DISC_IND,
	MNDSMSG_MN_DISC_CNF,
	MNDSMSG_MN_SETUP_ACC,
	MNDSMSG_MN_SETUP_CNF,
	MNDSMSG_MN_SETUP_IND,
	MNDSMSG_MN_SETUP_REJ,
	MNDSMSG_MN_SYNC_IND,
									// SIMAP messages:
	MNDSMSG_SIMTOOLKIT_REFRESH_REQ,
	MNDSMSG_SIMTOOLKIT_RST_REQ,
	MNDSMSG_SIMTOOLKIT_RST_CONFIRM_REQ,
	MNDSMSG_DS_MODIFY_REQ


} MNDSMsgType_t;




//******************************************************************************
//
// System Messages, messages from MMI->MSC->MN->MNDS
//
//******************************************************************************

typedef struct
{
	UInt8				ti;
	T_SUBADDR			conn_subaddr;
} MNDSParmDSCallAcc_t;

typedef struct
{
	UInt8				ti;
	Cause_t 			cause;
} MNDSParmDSDiscReq_t;

typedef struct
{
	UInt8				ti;
	BearerCapability_t 	bc;
} MNDSParmDSServiceAcc_t;

typedef struct
{
	UInt8				ti;
	Cause_t 			cause;
} MNDSParmDSServiceRej_t;

typedef struct
{
	T_EST_CAUSE			est_cause;
	BearerCapability_t 	bc;
	LLCompatibility_t 	llc;
	HLCompatibility_t 	hlc;
	T_SUBADDR			calling_subaddr;
	TelephoneNumber_t 	phone_number;
	T_SUBADDR			called_subaddr;
	CLIRMode_t			clir_mode;
	T_MN_CUG			cug;
	Boolean				auto_call_ind;
} MNDSParmDSSetupReq_t;

typedef struct 
{
	BearerCapability_t 	bc;
	LLCompatibility_t 	llc;
	HLCompatibility_t 	hlc;
}MNDSParmMNModifyReq_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
	BearerCapability_t 	bc;
} MNDSParmMNBearerInd_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
	Cause_t 			cause;
} MNDSParmMNDiscInd_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
} MNDSParmMNDiscCnf_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
} MNDSParmMNSetupAcc_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
	TelephoneNumber_t 	phone_number;
	Subaddress_t		conn_subaddr;  
} MNDSParmMNSetupCnf_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
	BearerCapability_t 	bc;
	LLCompatibility_t 	llc;
	HLCompatibility_t 	hlc;
	TelephoneNumber_t 	phone_number;
	Subaddress_t		calling_subaddr;
	Subaddress_t		called_subaddr;
} MNDSParmMNSetupInd_t;

typedef struct
{
	CallIndex_t			ci;
	Cause_t 			cause;
} MNDSParmMNSetupRej_t;

typedef struct
{
	UInt8				ti;
    T_RAT               rat;
    Boolean             rab_assigned;   // For UMTS
	UInt8				chan_mode;
	UInt8				ts_uplink;		// Uplink Allocated Timeslots
	UInt8				ts_downlink;	// Downlink Allocated Timeslots
} MNDSParmMNSyncInd_t;

typedef struct
{
	Boolean			full_changed;
} MNDSParmSTKRefreshReq_t;

//******************************************************************************
//
// Combined MNDS messages
//
//******************************************************************************
typedef union
{
										// System parameters
										// =================
	
	
										// Internal parameters
										// ===================

										// DS parameters
										// ==============
	MNDSParmDSCallAcc_t		call_acc;
	MNDSParmDSDiscReq_t		disc_req;
	MNDSParmDSServiceAcc_t	service_acc;
	MNDSParmDSServiceRej_t	service_rej;
	MNDSParmDSSetupReq_t	setup_req;
										// MN parameters
										// ==============
	MNDSParmMNBearerInd_t	bearer_ind;
	MNDSParmMNDiscInd_t		disc_ind;
	MNDSParmMNDiscCnf_t		disc_cnf;
	MNDSParmMNSetupAcc_t	setup_acc;
	MNDSParmMNSetupCnf_t	setup_cnf;
	MNDSParmMNSetupInd_t	setup_ind;
	MNDSParmMNSetupRej_t	setup_rej;
	MNDSParmMNSyncInd_t		sync_ind;
	MNDSParmMNModifyReq_t	modify_req;

										 // SIMAP parameters
	MNDSParmSTKRefreshReq_t	stkRefreshReq;
} MNDSMsgParm_t;						// MNDS message parameters

//**************
// MNDS Message 
//**************
typedef struct
{
	MNDSMsgType_t	type;					// MNDS Message Type
	ClientInfo_t	clientInfo;				//< Client Information
	MNDSMsgParm_t	parm;					// MNDS Message Parameters
} MNDSMsg_t;							// MNDS Message

#define MNDS_MSG_SIZE					sizeof( MNDSMsg_t )

#endif

