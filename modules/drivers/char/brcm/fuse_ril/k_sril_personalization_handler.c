/**
 * COPYRIGHT (C)  SAMSUNG Electronics CO., LTD (Suwon, Korea). 2009           
 * All rights are reserved. Reproduction and redistiribution in whole or 
 * in part is prohibited without the written consent of the copyright owner.
 */

/**
 * @file	request.h
 *
 * @author	Sungdeuk Park (sungdeuk.park@samsung.com)
 *
 * @brief	Request from RIL deamon
 */


#include "bcm_kril_common.h"
#include "bcm_kril_capi2_handler.h"
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"

#include "bcm_kril_simlockfun.h"

#include "k_sril_handler.h"
#include "k_sril_personalization_handler.h"

//for persu
#define 	OEM_PERSO_UNLOCK_MODE 			 0
#define 	OEM_PERSO_LOCK_MODE 			 1
#define 	OEM_PERSO_GET_LOCK_STATUS	 	 2
#define 	OEM_PERSO_GET_LOCK_TYPE	 		 3


static void KRIL_SRIL_requestGetLockStatus(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
static void KRIL_SRIL_requestGetLockType(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
static void KRIL_SRIL_requestLockMode(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
static void KRIL_SRIL_requestUnlockMode(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);

/****************************************************/
/*  [Structure] pdata->ril_cmd->data				*/
/*													*/
/*	[0] byte cmd 									*/
/*	[1] byte svcMode 								*/
/*	[2] short fileSize								*/
/*	[4] byte modeType 								*/
/*	[5] byte subType 								*/
/*	[6] byte query 									*/
/****************************************************/


void KRIL_SRIL_PersonalizationHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	char* rawdata = NULL;

	rawdata = (char*)pdata->ril_cmd->data;

	UInt8 subCmd;

	subCmd = (UInt8)rawdata[1];	//svcMode
	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_PersonalizationHandler -subCmd:%d\n", subCmd);

	if(pdata->handler_state != BCM_SendCAPI2Cmd)
		KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_PersonalizationHandler : Invalid handler state");
				
	switch (subCmd)
	{
		case OEM_PERSO_UNLOCK_MODE :
			KRIL_SRIL_requestUnlockMode(ril_cmd, capi2_rsp);
			break;
		case OEM_PERSO_LOCK_MODE :
			KRIL_SRIL_requestLockMode(ril_cmd, capi2_rsp);
			break;
		case OEM_PERSO_GET_LOCK_STATUS :
			KRIL_SRIL_requestGetLockStatus(ril_cmd, capi2_rsp);
			break;
		case OEM_PERSO_GET_LOCK_TYPE :
			KRIL_SRIL_requestGetLockType(ril_cmd, capi2_rsp); //currnetly not use this func. 
			break;			

		default : 
			KRIL_DEBUG(DBG_ERROR,"Unsupported subCmd:%d Error!!!\n", subCmd);
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			break;
	}

}

static void KRIL_SRIL_requestGetLockStatus(void *ril_cmd,  Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	SIMLOCK_STATE_t simlock_state;
	char    test[16] ={0, };

	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestGetLockStatus");

	if(pdata->handler_state != BCM_SendCAPI2Cmd)
		KRIL_DEBUG(DBG_ERROR,"Invalid handler state");

	switch (pdata->handler_state)
	{
        case BCM_SendCAPI2Cmd:
		{	
			pdata->bcm_ril_rsp = kmalloc(32, GFP_KERNEL);

			if (!pdata->bcm_ril_rsp)            
			{                
				KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n"); 
				pdata->handler_state = BCM_ErrorCAPI2Cmd;                
				return;          
			}                        

			SIMLockGetSIMLockState(&simlock_state);

			test[0] = simlock_state.network_lock; //np (if active = 1, deactive = 0)
			test[1] = simlock_state.network_subset_lock; //subset
			test[2] = simlock_state.service_provider_lock; //sp
			test[3] = simlock_state.corporate_lock; //cp

			KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestGetLockStatus - sizeof(test) : %d",sizeof(test));
			memcpy(pdata->bcm_ril_rsp, test, 16);
			pdata->rsp_len = 32;
			pdata->handler_state = BCM_FinishCAPI2Cmd;
			break;
        	}

	    default:
	    {
	        KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
	        pdata->handler_state = BCM_ErrorCAPI2Cmd;
	        break;
	    }
	}

	return;
}


static void KRIL_SRIL_requestUnlockMode(void *ril_cmd,  Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	//KrilSetNetLock_t *cmd_data = (KrilSetNetLock_t*)pdata->ril_cmd->data;
	SIMLock_Status_t simlock_status;
	SIMLockType_t simlock_type = SIMLOCK_INVALID_LOCK;
	char    test[16] ={0, };
	int i;
	char* rawdata = NULL;
	rawdata = (char*)pdata->ril_cmd->data;
	UInt8 locktype;
	char    password[20] ={0, };	

	KRIL_DEBUG(DBG_ERROR,"## KRIL_SRIL_requestUnlockMode");

	if(pdata->handler_state != BCM_SendCAPI2Cmd)
		KRIL_DEBUG(DBG_ERROR,"Invalid handler state");

	locktype = (UInt8)rawdata[4];

	switch (locktype)
	{
		case 5:
			simlock_type = SIMLOCK_NETWORK_LOCK;
		break;

		case 6:
			simlock_type = SIMLOCK_NET_SUBSET_LOCK;
		break;			

		case 7:
			simlock_type = SIMLOCK_PROVIDER_LOCK;
		break;

		case 8:
			simlock_type = SIMLOCK_CORP_LOCK;
		break;

		default:
			KRIL_DEBUG(DBG_ERROR,"Unsupported locktype:%d Error!!!\n", locktype);
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			break;
	}

	KRIL_DEBUG(DBG_ERROR,"## simlock_type :%d",simlock_type);

	for (i=0; i<8; i++)		
	{
		password[i] = rawdata[i+6];	//svcMode
	}

	KRIL_DEBUG(DBG_ERROR,"password :%s",password);

	switch (pdata->handler_state)
	{
        case BCM_SendCAPI2Cmd:
		{	
			pdata->bcm_ril_rsp = kmalloc(32, GFP_KERNEL);

			if (!pdata->bcm_ril_rsp)            
			{                
				KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n"); 
				pdata->handler_state = BCM_ErrorCAPI2Cmd;                
				return;          
			}                        

			simlock_status = SIMLockUnlockSIM(simlock_type, (UInt8*)password);
#if 0
			switch (simlock_status)
			{
			    case SIMLOCK_SUCCESS:
			        test[0]  = RIL_E_SUCCESS;
			        break;
			    
			    case SIMLOCK_WRONG_KEY:
			    case SIMLOCK_PERMANENTLY_LOCKED:
			        test[0]  = RIL_E_PASSWORD_INCORRECT;
			        break;
			    
			    default:
			        test[0]  = RIL_E_GENERIC_FAILURE;
			        break;
			}
#else
			test[0] = 1;
#endif

			KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestUnlockMode - sizeof(test) : %d",sizeof(test));
			memcpy(pdata->bcm_ril_rsp, test, 16);
			pdata->rsp_len = 32;
			pdata->handler_state = BCM_FinishCAPI2Cmd;
			break;
        	}

	    default:
	    {
	        KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
	        pdata->handler_state = BCM_ErrorCAPI2Cmd;
	        break;
	    }
	}

	return;
}


static void KRIL_SRIL_requestLockMode(void *ril_cmd,  Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	//KrilSetNetLock_t *cmd_data = (KrilSetNetLock_t*)pdata->ril_cmd->data;
	SIMLock_Status_t simlock_status;
	SIMLockType_t simlock_type = SIMLOCK_INVALID_LOCK;
	char    test[16] ={0, };
	int i;
	char* rawdata = NULL;
	rawdata = (char*)pdata->ril_cmd->data;
	UInt8 locktype;
	UInt8 lenth;
	char    mccmnc[20] ={0, };
	char    password[20] ={0, };	

	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestLockMode");

	if(pdata->handler_state != BCM_SendCAPI2Cmd)
		KRIL_DEBUG(DBG_ERROR,"Invalid handler state");

	locktype = (UInt8)rawdata[4];
	lenth = (UInt8)rawdata[6];

	switch (locktype)
	{
		case 5:
			simlock_type = SIMLOCK_NETWORK_LOCK;
		break;

		case 6:
			simlock_type = SIMLOCK_NET_SUBSET_LOCK;
		break;			

		case 7:
			simlock_type = SIMLOCK_PROVIDER_LOCK;
		break;

		case 8:
			simlock_type = SIMLOCK_CORP_LOCK;
		break;

		default:
			KRIL_DEBUG(DBG_ERROR,"Unsupported locktype:%d Error!!!\n", locktype);
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			break;
	}

	KRIL_DEBUG(DBG_ERROR,"simlock_type :%d, lenth :%d",simlock_type,lenth);

	for (i=0; i<lenth; i++)		
	{
		mccmnc[i] = rawdata[i+7];	//svcMode
	}

	KRIL_DEBUG(DBG_ERROR,"mccmnc :%s",mccmnc);


	for (i=0; i<8; i++)		
	{
		password[i] = rawdata[i+12];	//svcMode
	}

	KRIL_DEBUG(DBG_ERROR,"password :%s",password);

	switch (pdata->handler_state)
	{
        case BCM_SendCAPI2Cmd:
		{	
			pdata->bcm_ril_rsp = kmalloc(32, GFP_KERNEL);

			if (!pdata->bcm_ril_rsp)            
			{                
				KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n"); 
				pdata->handler_state = BCM_ErrorCAPI2Cmd;                
				return;          
			}                        

			KRIL_DEBUG(DBG_ERROR,"## Test IMSI : %s, Locktype : %d, Password : %s \n", mccmnc, simlock_type, password);

			KRIL_DEBUG(DBG_ERROR,"## Test IMSI : %s, Locktype : %d, Password : %s \n", (UInt8*)mccmnc, simlock_type, (UInt8*)password);


//SIMLock_Status_t  SIMLockSetLock(UInt8 action, Boolean ph_sim_full_lock_on, SIMLockType_t lockType, UInt8* key, UInt8* imsi, UInt8* gid1, UInt8* gid2)			
			simlock_status = SIMLockSetLock(1, FALSE, simlock_type, (UInt8*)password, (UInt8*)mccmnc, NULL, NULL);

#if 0
			switch (simlock_status)
			{
			    case SIMLOCK_SUCCESS:
			        test[0]  = RIL_E_SUCCESS;
			        break;
			    
			    case SIMLOCK_WRONG_KEY:
			    case SIMLOCK_PERMANENTLY_LOCKED:
			        test[0]  = RIL_E_PASSWORD_INCORRECT;
			        break;
			    
			    default:
			        test[0]  = RIL_E_GENERIC_FAILURE;
			        break;
			}
#else
			test[0] = 1;
#endif
			KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestLockMode - sizeof(test) : %d",sizeof(test));
			memcpy(pdata->bcm_ril_rsp, test, 16);
			pdata->rsp_len = 32;
			pdata->handler_state = BCM_FinishCAPI2Cmd;
			break;
        	}

	    default:
	    {
	        KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
	        pdata->handler_state = BCM_ErrorCAPI2Cmd;
	        break;
	    }
	}

	return;
}


//currnetly not use this func. 
static void KRIL_SRIL_requestGetLockType(void *ril_cmd,  Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	char    test[16] ={0, };

	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestGetLockType");

	switch (pdata->handler_state)
	{
	    case BCM_SendCAPI2Cmd:
	    {
			pdata->bcm_ril_rsp = kmalloc(32, GFP_KERNEL);

			if (!pdata->bcm_ril_rsp)            
			{                
				KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n"); 
				pdata->handler_state = BCM_ErrorCAPI2Cmd;                
				return;          
	    }

			test[0] = 0;
			test[1] = 0;
			test[2] = 0;
			test[3] = 0;

			KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestGetLockType - sizeof(test) : %d",sizeof(test));
			memcpy(pdata->bcm_ril_rsp, test, 16);
			pdata->rsp_len = 32;
	        pdata->handler_state = BCM_FinishCAPI2Cmd;
	        break;
	    }

	    default:
	    {
	        KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
	        pdata->handler_state = BCM_ErrorCAPI2Cmd;
	        break;
	    }
	}

	return;
}

