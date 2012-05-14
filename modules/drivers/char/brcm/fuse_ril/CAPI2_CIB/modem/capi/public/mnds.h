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
*   @file   mnds.h
*
*   @brief  This file contains definitions for MNDS.
*
****************************************************************************/
#ifndef _MSC_MNDS_H_
#define _MSC_MNDS_H_

//******************************************************************************
// Function Prototypes
//******************************************************************************
// AoC 
UInt32 MNDS_ObtainDataBytes (	// Obtain the number of data bytes received 
								// or transmitted 
	CallIndex_t		call_index	// CallIndex
	);
// end 

//******************************************************************************
// The following are the general services provided by all interfaces
//******************************************************************************
void MNDS_Init( void );		   	// Initialize the MNDS API
void MNDS_Run( void );
void MNDS_Shutdown( void );
void MNDS_ProcessMsg( MNDSMsg_t	*msg );


//******************************************************************************
// Direct Call Definitions
//******************************************************************************
void MNDS_ReportDataCallReceived(	// Receive Data Call
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t			ci,					// CallIndex
	UInt8				ti,					// Transaction Id
	BearerCapability_t 	*bc,				// Bearer Capability
	LLCompatibility_t 	*llc,				// Low Level Compatibility
	HLCompatibility_t 	*hlc,				// High Level Compatibility
	TelephoneNumber_t 	*phone_number,		// Phone number of calling party
	Subaddress_t		*calling_subaddr,	// Calling subaddress
	Subaddress_t		*called_subaddr		// Called subaddress
	);
	
void MNDS_ReportDataCallConnected(	// Data Call Connected
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t 		ci,
	UInt8				ti,
	TelephoneNumber_t 	*phone_number,
	Subaddress_t		*subaddr 
	);
	
void MNDS_ReportDataCallCalling(	// Data Call has started
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t ci,					// CallIndex
	UInt8		ti					// Transaction Id
	);

void MNDS_RequestDataCallHangUp(	// Data Call is requested to Hang Up
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t ci,					// CallIndex
	Cause_t cause					// cause of Hang Up request
	);

void MNDS_ReportBearerCapabilityReceived(// Receive Bearer Capability
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t 		ci,
	UInt8				ti,
	BearerCapability_t *bc	
	);

void MNDS_SetDataCallMode(			// Set Data Service Mode
	DataCallMode_t mode				// Either allow or disallow DS
	);

void MNDS_DataCallSyncInd(
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t 		ci,	
	UInt8				ti,
    T_RAT               rat,
    Boolean             rab_assigned,
	UInt8				channel_mode,
	UInt8				ts_uplink,
	UInt8				ts_downlink
	);


void MNDS_GetDataCallBytes(UInt32* rxBytes,
						   UInt32* txBytes);

void MNDS_ResetDataCallBytes(void);

/* callback functions for MNDS_Register() */
void MNDS_SendDataCallReq(ClientInfo_t*	inClientInfoPtr,			// Callback to request connection
	T_EST_CAUSE			est_cause,
	BearerCapability_t 	*bc,				// Bearer Capability
	LLCompatibility_t 	*llc,				// Low Level Compatibility
	HLCompatibility_t 	*hlc,				// High Level Compatibility
	T_SUBADDR			*calling_subaddr,			
	TelephoneNumber_t 	*phone_number, 		// Phone Number to call
	T_SUBADDR			*called_subaddr,
	CLIRMode_t 			clir_mode,
	T_MN_CUG			*cug,
	Boolean				auto_call_ind);

void MNDS_ReportDataCallServiceStatus(
	ClientInfo_t* inClientInfoPtr,
	CallIndex_t ci,						// CallIndex
	Boolean is_accepted,				// TRUE, if Data Call Service accepted
	Cause_t reject_cause,				// if Data Call Service is rejected, then this is used
	BearerCapability_t *bc				// if Data Call Service is accepted, then this is used
	);

void MNDS_AcceptDataCall(ClientInfo_t* inClientInfoPtr, CallIndex_t c_idx);

void MNDS_DisconnectDataCall(	ClientInfo_t*	inClientInfoPtr,
								CallIndex_t		c_idx,
								Cause_t			cause);

void MN_ModifyHSCSDCall(ClientInfo_t*		inClientInfoPtr,
						BearerCapability_t 	*bc,
						LLCompatibility_t 	*llc,
						HLCompatibility_t 	*hlc);

void MN_PostMNDSMsg(MNDSMsg_t *mnds_msg);					// Post message to MNDS

int IsMNDS_InIdle( void );

#endif

