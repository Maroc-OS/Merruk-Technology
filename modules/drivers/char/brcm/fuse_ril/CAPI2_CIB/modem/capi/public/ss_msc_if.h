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
*   @file   ss_msc_if.h
*
*   @brief  This file defines the interface for between msc and ss.
*
****************************************************************************/ 

#ifndef _SS_MSC_IF_H_
#define _SS_MSC_IF_H_




void SS_InitUssdSessionId(void);


// Internal Functions

void SS_HandleUssdDataInd(InterTaskMsg_t *msg);
void SS_HandleUssdIndexInd(InterTaskMsg_t *msg);
void SS_HandleUssdReleaseInd(InterTaskMsg_t*inMsg);


//******************************************************************************
// Function Name:	SS_ProcessSsSrvRel
//
// Description:		Process MSG_MNSS_CLIENT_SS_SRV_REL message.
//					Parse the input return result component and send appropriate
//					upper layer messages.
//******************************************************************************
void SS_ProcessSsSrvRel(InterTaskMsg_t* inMsgPtr);

//******************************************************************************
// Function Name:	SS_ProcessSsSrvInd
//
// Description:		Process MSG_MNSS_CLIENT_SS_SRV_IND message.
//******************************************************************************
void SS_ProcessSsSrvInd(InterTaskMsg_t* inMsgPtr);

//******************************************************************************
// Function name:	SS_ProcessSsSrvRsp
//
// Description:		Process MSG_MNSS_CLIENT_SS_SRV_RSP message.
//******************************************************************************
void SS_ProcessSsSrvRsp(InterTaskMsg_t* inMsgPtr);



#endif // _SS_MSC_IF_H_

