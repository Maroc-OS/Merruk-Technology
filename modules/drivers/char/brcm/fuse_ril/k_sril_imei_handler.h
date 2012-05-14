/**

 * COPYRIGHT (C)  SAMSUNG Electronics CO., LTD (Suwon, Korea). 2009           

 * All rights are reserved. Reproduction and redistiribution in whole or 

 * in part is prohibited without the written consent of the copyright owner.

 */



/**

 * @file	k_sril_imei_handler.h

 *

 * @author	Woonsang Son (woonsang.son@samsung.com)

 *

 * @brief	handler for imei (OEM_FUNCTION_ID_IMEI)

 */



#ifndef _K_SRIL_IMEI_HANDLER_H

#define _K_SRIL_IMEI_HANDLER_H

   

void KRIL_SRIL_requestImeiHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);

void KRIL_SRIL_requestSetPreConfiguration(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_requestGetPreConfiguration(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_requestSetWriteItem(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_requestGetWriteItem(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_requestSetUpdateItem(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_requestCfrmUpdateItem(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_requestEventVerifyCompare(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
void KRIL_SRIL_requestRespFileNum(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);

void KRIL_SRIL_requestOemSimOut(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);


#endif // _K_SRIL_IMEI_HANDLER_H

