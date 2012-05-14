/*********************************************************************
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*     
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*    
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent. 
*   
***************************************************************************/
/**
*
*   @file   capi2_lcs_comm.h
*
*   @brief  This file defines the capi2 message handler for LCS on the
*			comm side.
*
****************************************************************************/
#ifndef CAPI2_LCS_COMM_H
#define CAPI2_LCS_COMM_H

Result_t CAPI2_LCS_CpMoLrReqReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_LCS_CpMoLrAbortReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_LCS_CpMtLrVerificationRspReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_LCS_CpMtLrRspReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_LCS_CpLocUpdateRspReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_LCS_DecodePosEstimateReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_LCS_EncodeAssistanceReqReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);

#endif


