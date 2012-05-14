/**
 * COPYRIGHT (C)  SAMSUNG Electronics CO., LTD (Suwon, Korea). 2009           
 * All rights are reserved. Reproduction and redistiribution in whole or 
 * in part is prohibited without the written consent of the copyright owner.
 */

/**
 * @file	k_sril_Personalization_handler.h
 *
 * @author	Sungdeuk Park (seungdeuk.park @samsung.com)
 *
 * @brief	Request from RIL deamon
 */

#ifndef _K_SRIL_Personalization_HANDLER_H
#define _K_SRIL_Personalization_HANDLER_H

typedef struct
{
    int   state[64];
} kril_personalizationstatus_t;

   
void KRIL_SRIL_PersonalizationHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);

#endif // _K_SRIL_SVCMODE_HANDLER_H
