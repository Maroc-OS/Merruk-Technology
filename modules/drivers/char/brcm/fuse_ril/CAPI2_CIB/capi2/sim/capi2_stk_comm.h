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
***************************************************************************/
/**
*
*   @file   capi2_stk_comm.h
*
*   @brief  This file defines the capi2 message handler for stk on the
*			comm side.
*
****************************************************************************/
#ifndef CAPI2_STK_COMM_H
#define CAPI2_STK_COMM_H

Result_t CAPI2_SATK_GetCachedRootMenuPtrMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendUserActivityEventMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendIdleScreenAvaiEventMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendBrowserTermEventMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATKCmdRespMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATKDataServCmdRespMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATKDataServReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendLangSelectEventMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SetTermProfileMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_GetTermProfileMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendTerminalRspMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendEnvelopeCmdMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendBrowsingStatusEvtMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendCcSetupReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendCcSsReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendCcUssdReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATK_SendCcSmsReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATKCmdRespMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATKSendDataServReqMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
Result_t CAPI2_SATKSendTerminalRspMsgHnd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);

#endif	//CAPI2_STK_COMM_H

