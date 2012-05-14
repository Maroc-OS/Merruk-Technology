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
*   @file   pchex_pf_api.h
*
*   @brief  This file defines the api to hookup external packet filtering routines
*			to the cellular stack At this time only port white list filters are hooked up.
*
****************************************************************************/
#ifndef _PCHEX_PF_API_H_
#define _PCHEX_PF_API_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
Packet Filtering Results
**/
typedef enum
{
	PCHEX_PF_CONTINUE,	///< Would continue processing.
	PCHEX_PF_DROP,		///< The filter asked to drop this packet
	PCHEX_PF_CONSUMED	///< Filter consumed this packet
} PCHEx_PF_Result_t;

	
/**
Packet Filter priority.
**/
typedef enum
{
	PCHEX_PF_PRIORITY_0,
	MAX_PF_PRIORITY
} PCHEx_PF_Priority_t;

/**
Callback funtion types for external Uplink filter routine
**/
typedef PCHEx_PF_Result_t		(*PCHEx_PF_Send_t)(ClientInfo_t* inClienInfoPtr, UInt8 cid, void* data, int len);
/**
Callback funtion types for external Downlink filter routine
**/
typedef PCHEx_PF_Result_t		(*PCHEx_PF_Receive_t)(ClientInfo_t* inClienInfoPtr, UInt8 cid, void* data, int len);
/**
Callback funtion called to initialize the external packet filtering 
**/
typedef void					(*PCHEx_PF_Init_t)(ClientInfo_t* inClienInfoPtr);


typedef struct
{
	PCHEx_PF_Send_t	send_CB;
	PCHEx_PF_Receive_t	receive_CB;
} PCHEx_PF_t;

//***************************************************************************************
/**
	Function to register the external Uplink downlink filter routine.
	@param		inClienInfoPtr (in) Client Information Pointer.
	@param		inIrPfPtr (in) iprelay external uplink and downlink filter routines
	@param		inIrPriority (in) iprelay filter priority
	@param		inModemPfPtr (in) DUN external uplink and downlink filter routines 
	@param		inModemPriority (in) DUN filter priority
	@param		inStkPfPtr (in) stk uplink and downlink filter routines
	@param		inStkPriority (in) stk filter priority
	@return		RESULT_OK
	@note
**/
Result_t PCHEx_RegisterPf(ClientInfo_t* inClienInfoPtr, PCHEx_PF_t* inIrPfPtr, PCHEx_PF_Priority_t inIrPriority,  PCHEx_PF_t* inModemPfPtr, PCHEx_PF_Priority_t inModemPriority, PCHEx_PF_t* inStkPfPtr, PCHEx_PF_Priority_t inStkPriority);
//***************************************************************************************
/**
	Function to call the registered external uplink filter routine
	@param		inClienInfoPtr (in) Client Information Pointer.
	@param		inReason (in) pdp activation reason
	@param		inCid (in) cid
	@param		inPduPtr (in) The uplink pdu 
	@param		inLen (in) the pdu length
	@return		
	@note
**/
PCHEx_PF_Result_t PCHEx_SendPf(ClientInfo_t* inClienInfoPtr, PCHActivateReason_t inReason, UInt8 inCid, void* inPduPtr, int inLen);
//***************************************************************************************
/**
	Function to call the registered external downlink filter routine
	@param		inClienInfoPtr (in) Client Information Pointer.
	@param		inReason (in) pdp activation reason
	@param		inCid (in) cid
	@param		inPduPtr (in) The uplink pdu 
	@param		inLen (in) the pdu length
	@return		
	@note
**/
PCHEx_PF_Result_t PCHEx_RcvPf(ClientInfo_t* inClienInfoPtr, PCHActivateReason_t inReason, UInt8 inCid, void* inPduPtr, int inLen);

void PchEx_InitPf(ClientInfo_t* inClienInfoPtr);
int pch_printf(const char * fmt, ...);

#ifdef __cplusplus
}
#endif
#endif
