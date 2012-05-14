/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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

#ifndef _BCM_KPDP_CAPI2_HANDLER_H
#define _BCM_KPDP_CAPI2_HANDLER_H
#include "bcm_kpdp_cmd_handler.h"


// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @ Set PDP Context handler
//
// @comm None
//
// -----------------------------------------------------------------------------
#define BCM_PDP_SetPdpContext         0x001
#define BCM_PDP_SetPdpOption         0x002
#define BCM_PDP_SetPdpOption2         0x003

// for PDP data
void KPDP_SetupPdpHandler(void *pdp_cmd, Kpdp_CAPI2Info_t *capi2_rsp);
void KPDP_DeactivatePdpHandler(void *pdp_cmd, Kpdp_CAPI2Info_t *capi2_rsp);
void KPDP_DataStateHandler(void *pdp_cmd, Kpdp_CAPI2Info_t *capi2_rsp);
void KPDP_SendDataHandler(void *pdp_cmd, Kpdp_CAPI2Info_t *capi2_rsp);
#endif //_BCM_KPDP_CAPI2_HANDLER_H
