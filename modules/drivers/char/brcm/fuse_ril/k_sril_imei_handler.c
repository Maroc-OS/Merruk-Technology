/**

 * COPYRIGHT (C)  SAMSUNG Electronics CO., LTD (Suwon, Korea). 2009           

 * All rights are reserved. Reproduction and redistiribution in whole or 

 * in part is prohibited without the written consent of the copyright owner.

 */



/**

 * @file	k_sril_imei_handler.c

 *

 * @author	Woonsang Son (woonsang.son@samsung.com)

 *

 * @brief	handler for imei (OEM_FUNCTION_ID_IMEI)

 */





#include "bcm_kril_common.h"

#include "bcm_kril_capi2_handler.h"

#include "bcm_kril_cmd_handler.h"

#include "bcm_kril_ioctl.h"



#include "k_sril_handler.h"

#include "k_sril_imei_handler.h"

#include "capi2_stk_ds.h"

#include "capi2_pch_msg.h"

#include "capi2_gen_msg.h"

#include "capi2_reqrep.h"

#include "capi2_gen_api.h"


extern void put_factory_string(const char *s, unsigned int count);


void KRIL_SRIL_requestImeiHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)

{

	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

	char* rawdata = NULL;

	UInt8 subCmd;



	rawdata = (char*)pdata->ril_cmd->data;



	subCmd = (UInt8)rawdata[1];



	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestImeiHandler -subCmd:%d\n", subCmd);

	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestImeiHandler -handler_state:%lu\n", pdata->handler_state);


	printk("########## KRIL  Request Imei Handler   ############\n");

	switch (subCmd)

	{

		case OEM_IMEI_EVENT_START_IMEI :
			
			KRIL_DEBUG(DBG_ERROR,"########## BOOT_COMPLETE ############\n");
			put_factory_string("BOOTING COMPLETED\n",19);

			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			break;

		case OEM_IMEI_SET_PRECONFIGURAION:

			KRIL_SRIL_requestSetPreConfiguration(ril_cmd, capi2_rsp);
			break;
					
#if defined(SEC_MULTI_CSC)
		case OEM_IMEI_GET_PRECONFIGURAION:
			KRIL_SRIL_requestGetPreConfiguration(ril_cmd, capi2_rsp);
			break;			
#endif

		case OEM_IMEI_SET_WRITE_ITEM:
			KRIL_SRIL_requestSetWriteItem(ril_cmd, capi2_rsp);
			break;
			
		case OEM_IMEI_GET_WRITE_ITEM:
			KRIL_SRIL_requestGetWriteItem(ril_cmd, capi2_rsp);
			break;
			
		case OEM_IMEI_SET_UPDATE_ITEM:
			KRIL_SRIL_requestSetUpdateItem(ril_cmd, capi2_rsp);
			break;

		case OEM_IMEI_CFRM_UPDATE_ITEM:
			KRIL_SRIL_requestCfrmUpdateItem(ril_cmd, capi2_rsp);
			break;

		case OEM_IMEI_EVENT_VERIFY_COMPARE:
			KRIL_SRIL_requestEventVerifyCompare(ril_cmd, capi2_rsp);
			break;			
			
		case OEM_IMEI_RESP_FILE_NUM:
			KRIL_SRIL_requestRespFileNum(ril_cmd, capi2_rsp);
			break;	
		
		case OEM_IMEI_SIM_OUT:
			KRIL_SRIL_requestOemSimOut(ril_cmd, capi2_rsp);
			break;				
		
		default : 

			KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestImeiHandler : Unsupported subCmd:%d Error!!!\n", subCmd);

			pdata->handler_state = BCM_ErrorCAPI2Cmd;

			break;

	}


	return;

}


void KRIL_SRIL_requestSetPreConfiguration(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)

{

	printk("####### Request Oem Set Preconfig ##########\n");


	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

	char* rawdata = NULL;

	UInt8 subCmd;


	rawdata = (char*)pdata->ril_cmd->data;

	subCmd = (UInt8)rawdata[1];
	pdata->handler_state = BCM_FinishCAPI2Cmd;

	return;

}

void KRIL_SRIL_requestGetPreConfiguration(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	printk("Request Oem Get PreConfiguration ##########");
}

void KRIL_SRIL_requestSetWriteItem(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	printk("Request Oem Set Write Item ##########");

/*
		if(item[0] == 0x81){			// IPC_IMEI_ITEM_GENERAL_LOCK
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK\n");			
			
		}
		else if(item[0] == 0x82){		//IPC_IMEI_ITEM_NP_LOCK
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_NP_LOCK\n");
		}
		else if(item[0] == 0x83){		//IPC_IMEI_ITEM_NSP_LOCK
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_NSP_LOCK\n");
		}
		else if(item[0] == 0x84){		//IPC_IMEI_ITEM_SP_LOCK
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_SP_LOCK\n");
		}
		else if(item[0] == 0x85){		//IPC_IMEI_ITEM_CP_LOCK
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_CP_LOCK\n");
		}
		else if(item[0] == 0x86){		//IPC_IMEI_ITEM_SIM_USIM_LOCK
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_SIM_USIM_LOCK\n");
		}
		else if(item[0] == 0x87){		//IPC_IMEI_ITEM_NETWORK_MODE
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_NETWORK_MODE >> item Len = [%x]\n", item[4]);
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_NETWORK_MODE >> item value = [%x]\n", item[6]);

		}
		else if(item[0] == 0x88){		//IPC_IMEI_ITEM_SECURITY_CODE
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_SECURITY_CODE\n");
		}
		else if(item[0] == 0x89){		//IPC_IMEI_ITEM_GPRS_MODE
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_GPRS_MODE >> item Len = [%x]\n", item[4]);
			LOGE("#### uSril_SetUpdateItem : IPC_IMEI_ITEM_GPRS_MODE >> item value = [%x]\n", item[6]);

		}		

		*/
}

void KRIL_SRIL_requestGetWriteItem(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	printk("Request Oem Get Write Item ##########");
}



void KRIL_SRIL_requestSetUpdateItem(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)

{

	printk("Request Oem Set Update Item ##########");
	put_factory_string("RESET COMPLETED\n",17);

	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

	/*
	
	UInt8 subCmd;

	char* rawdata = NULL;	
	char* item = NULL;
	char* element = NULL;

	int byteLen = 0;
	int itemNum = 0;
	int itemCnt = 0;
	int itemLen = 0;
	int elementCnt = 0;

	rawdata = (char*)pdata->ril_cmd->data;
	subCmd = (UInt8)rawdata[1];

	printk("#### k_Sril_SetUpdateItem =  modeType[%x]\n", rawdata[0]);
	printk("#### k_Sril_SetUpdateItem =  subType[%x]\n", rawdata[1]);

	byteLen = ((int)rawdata[3])-6;
	itemNum = (int)rawdata[5];

	printk("#### k_Sril_SetUpdateItem =  byte Len[%d]\n", (int)byteLen);
	printk("#### k_Sril_SetUpdateItem =  item Num[%d]\n", (int)itemNum);

	item = rawdata+6;

	for(itemCnt=0;itemCnt<(int)itemNum;itemCnt++){
	  	//RIL_SRIL_requestSetWriteItem(request, item, (size_t)byteLen/(size_t)itemNum, t, inClientId); 
		
		if(item[0] == 0x87){		//IPC_IMEI_ITEM_NETWORK_MODE
			printk("#### k_Sril_SetUpdateItem : IPC_IMEI_ITEM_NETWORK_MODE >> item Len = [%x]\n", item[4]);
			printk("#### k_Sril_SetUpdateItem : IPC_IMEI_ITEM_NETWORK_MODE >> item value = [%x]\n", item[6]);

		}
		else if(item[0] == 0x88){		//IPC_IMEI_ITEM_SECURITY_CODE
			printk("#### k_Sril_SetUpdateItem : IPC_IMEI_ITEM_SECURITY_CODE\n");
		}
		else if(item[0] == 0x89){		//IPC_IMEI_ITEM_GPRS_MODE
			printk("#### k_Sril_SetUpdateItem : IPC_IMEI_ITEM_GPRS_MODE >> item Len = [%x]\n", item[4]);
			printk("#### k_Sril_SetUpdateItem : IPC_IMEI_ITEM_GPRS_MODE >> item value = [%x]\n", item[6]);
			
		}		
		else if(item[0] == 0x8D){		//IPC_GSM_IMEI_ITEM_SELLOUT_SMS; // item id for the 1st item
			printk("#### k_Sril_SetUpdateItem : IPC_GSM_IMEI_ITEM_SELLOUT_SMS >> item Len = [%x]\n", item[4]);	// item length (0x18 == 24)
			printk("#### k_Sril_SetUpdateItem : IPC_GSM_IMEI_ITEM_SELLOUT_SMS >> element cnt = [%x]\n", item[6]);	// Element Count
			
			element = (char*)item[7];
			for(elementCnt=0;elementCnt<(int)item[6];elementCnt++){
				printk("#### k_Sril_SetUpdateItem : IPC_GSM_IMEI_ITEM_SELLOUT_SMS >> elelment ID = [%x]\n", element[0]);	// IEMI_TOOL_TRIGGER_AP // Sellout SMS Mode
				printk("#### k_Sril_SetUpdateItem : IPC_GSM_IMEI_ITEM_SELLOUT_SMS >> elelment len = [%x]\n", element[2]);	// length
				printk("#### k_Sril_SetUpdateItem : IPC_GSM_IMEI_ITEM_SELLOUT_SMS >> elelment value = [%x]\n", element[4]);	// value
				if (element[0] ==0x01){		// IEMI_TOOL_TRIGGER_AP // Sellout SMS Mode
					printk("#### k_Sril_SetUpdateItem : IPC_GSM_IMEI_ITEM_SELLOUT_SMS >> elelment ID = [%x]\n", element[0]);	
				} 
				else if (element[0] ==0x02){		// IEMI_TOOL_TRIGGER_PC // Sellout Product Info
					printk("#### k_Sril_SetUpdateItem : IPC_GSM_IMEI_ITEM_SELLOUT_SMS >> elelment ID = [%x]\n", element[0]);	
				} 

				element = (char*)element[4+(int)element[2]];
			}
			
		}		

		item = item + (byteLen/itemNum);
	}



*/
	
	pdata->handler_state = BCM_FinishCAPI2Cmd;
	
	return;
}

void KRIL_SRIL_requestCfrmUpdateItem(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	printk("Request Oem Cfrm Update Item ##########");

	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

	UInt8 subCmd;

	char* rawdata = NULL;	
	char* item = NULL;
	char* element = NULL;

	int byteLen = 0;
	int itemNum = 0;
	int itemCnt = 0;
	int itemLen = 0;
	int elementCnt = 0;

	rawdata = (char*)pdata->ril_cmd->data;
	subCmd = (UInt8)rawdata[1];



	printk("#### k_Sril_CfrmUpdateItem = modeType[%x]\n", rawdata[0]);
	printk("#### k_Sril_CfrmUpdateItem = subType[%x]\n", rawdata[1]);


	byteLen = ((int)rawdata[3])-5;
	itemNum = (int)rawdata[4];
	//memcpy(&byteLen, rawdata+2, 2);
	//memcpy(&itemNum, rawdata+4, 2);	

	printk("#### k_Sril_CfrmUpdateItem =  byte Len[%d]\n", (int)byteLen);
	printk("#### k_Sril_CfrmUpdateItem =  item Num[%d]\n", (int)itemNum);

	item = rawdata+5;

	for(itemCnt=0;itemCnt<(int)itemNum;itemCnt++){
	  	/*uSril_SetWriteItemMessage(request, item, (size_t)byteLen/(size_t)itemNum, t, inClientId);*/
		
		if(item[0] == 0x81){			// IPC_IMEI_ITEM_GENERAL_LOCK
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK\n");			
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> item Len = [%x]\n", item[4]);
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> element cnt = [%x]\n", item[6]);
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> element ID = [%x]\n", item[7]);			
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> element len = [%x]\n", item[9]);
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> element value = [%x]\n", item[11]);
			
		}
		else if(item[0] >= 0x82 && item[0] <= 0x85){		//IPC_IMEI_ITEM_NP_LOCK, //IPC_IMEI_ITEM_NSP_LOCK, //IPC_IMEI_ITEM_SP_LOCK, //IPC_IMEI_ITEM_CP_LOCK
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_NP_LOCK\n");
			/* len */
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> item Len = [%x]\n", item[4]);
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> item Len = [%x]\n", item[5]);
			/**/
			itemLen=(int)item[4]*10+(int)item[5]/10;
			printk("#### k_Sril_SetUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> element cnt = [%x]\n", item[6]);

			element = (char*)item[7];
			for(elementCnt=0;elementCnt<(int)item[6];elementCnt++){
				printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> element ID = [%x]\n", element[0]);			
				printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> element len = [%x]\n", element[2]);	//len
				printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_GENERAL_LOCK >> element value = [%x]\n",  element[4]);

				/* to do *
				     save element value */
				element = (char*)element[4+(int)element[2]];
			}			
			
		}
		/*
		else if(item[0] == 0x83){		//IPC_IMEI_ITEM_NSP_LOCK
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_NSP_LOCK\n");
		}
		else if(item[0] == 0x84){		//IPC_IMEI_ITEM_SP_LOCK
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_SP_LOCK\n");
		}
		else if(item[0] == 0x85){		//IPC_IMEI_ITEM_CP_LOCK
			printk("#### k_Sril_CfrmUpdateItem : IPC_IMEI_ITEM_CP_LOCK\n");
		}
		*/
			
		item = item + (byteLen/itemNum);
	}
	/* to implement
	    send response to CSC appl */

	
	pdata->handler_state = BCM_FinishCAPI2Cmd;
	
	return;
}

void KRIL_SRIL_requestEventVerifyCompare(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	printk("Request Oem Event Verify Compare ##########");
}

void KRIL_SRIL_requestRespFileNum(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	printk("Request Oem Resp file Num ##########");
}


void KRIL_SRIL_requestOemSimOut(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestOemSimOut \n");

	switch (pdata->handler_state)
	{
		case BCM_SendCAPI2Cmd:
		{
			ClientInfo_t clientInfo;
			CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
			CAPI2_SimApi_PowerOnOffCard (&clientInfo, FALSE, SIM_POWER_ON_INVALID_MODE);
			pdata->handler_state = BCM_FinishCAPI2Cmd;
		}
		break;

		default:
		{
			KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
		}
		break;
	}
	
	return;
}


