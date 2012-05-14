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
*   @file   capi2_sms_comm.h
*
*   @brief  This file defines the capi2 message handler for sms on the
*			comm side.
*
****************************************************************************/
#ifndef CAPI2_SIM_COMM_H
#define CAPI2_SIM_COMM_H

extern Result_t CP_CAPI2_SIM_SendOpenSocketReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SendSelectAppiReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SendDeactivateAppiReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SendCloseSocketReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitDFileInfoReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitEFileInfoReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitWholeBinaryEFileReadReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitBinaryEFileReadReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitRecordEFileReadReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitBinaryEFileUpdateReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitLinearEFileUpdateReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SendSeekRecordReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitCyclicEFileUpdateReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SendWritePuctReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitRestrictedAccessReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIMLockSetLock(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SendWriteMulPLMNEntryReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SubmitSeekRecordReq(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);
extern Result_t CP_CAPI2_SIM_SendGenericApduCmd(CAPI2_ReqRep_t* reqRep, CAPI2_ReqRep_t* rsp);

#endif

