/**
 * COPYRIGHT (C)  SAMSUNG Electronics CO., LTD (Suwon, Korea). 2009           
 * All rights are reserved. Reproduction and redistiribution in whole or 
 * in part is prohibited without the written consent of the copyright owner.
 */
/**
 * @file	k_sril_gprs_handler.h
 *
 * @author	Sehee Park (sehee32.park@samsung.com)
 *
 * @brief	handler for gprs (OEM_FUNCTION_ID_GPRS)
 */
#ifndef _K_SRIL_GPRS_HANDLER_H
#define _K_SRIL_GPRS_HANDLER_H 	void KRIL_SRIL_requestGprsHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);	void KRIL_SRIL_requestOemSetDormancy(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
#endif // _K_SRIL_GPRS_HANDLER_H
