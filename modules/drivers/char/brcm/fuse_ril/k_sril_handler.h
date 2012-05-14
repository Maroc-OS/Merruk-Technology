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
#ifndef _K_SRIL_HANDLER_H
#define _K_SRIL_HANDLER_H

#include "u_sril.h"

void KRIL_SRIL_SetCellBroadcastHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_GetCellBroadcastHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
#if 1
void KRIL_SRIL_SendEncodeUSSDHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
#endif
void KRIL_SRIL_PhoneBookStorageInfoHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_GetPhoneBookEntryHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_AccessPhoneBookEnteryHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_DialVideoCallHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_CallDeflectionHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_ReadSMSFromSIMHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_USIM_PB_CAPAHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_LockInfoHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
#if 0
void KRIL_SRIL_DialEmergencyHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
#endif
void KRIL_SRIL_GetStorageMSGCountHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_OemHookRawHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
/*+ Band Selection sh0515.lee +*/
void KRIL_SRIL_SetBandHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp, KrilSrilSetBand_t *tdata);
/*- Band Selection sh0515.lee -*/
/*+ Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee +*/
void KRIL_SRIL_SetStackClasssMarkHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp, KrilStackNvramClassMark_t *tdata);CAPI2_MS_Element_t KRIL_SRIL_GetStackClasssMarkHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp, KrilStackNvramClassMark_t *tdata);
/*- Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee -*/
#endif  // _K_SRIL_HANDLER_H

