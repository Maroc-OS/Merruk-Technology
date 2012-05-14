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
//++ JSHAN_GPRS_Attach_Mode
#include "bcm_cp_cmd_handler.h"
//-- JSHAN_GPRS_Attach_Mode

#include "k_sril_handler.h"

#include "k_sril_svcmode_handler.h"



#include <asm/uaccess.h>

#include <linux/syscalls.h>


static void KRIL_SRIL_requestOemSvcEnterModeMessage(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);

static void KRIL_SRIL_requestOemSvcModeTestManual(void *ril_cmd,  Kril_CAPI2Info_t *capi2_rsp);

static void KRIL_SRIL_requestOemSvcProcessKeyCodeMessage(void *ril_cmd, unsigned char iKey, Kril_CAPI2Info_t *capi2_rsp);

static void KRIL_SRIL_requestOemSvcEndModeMessage(void *ril_cmd, unsigned char SvcMode, Kril_CAPI2Info_t *capi2_rsp);

extern void KRIL_SRIL_svc_update_screen(void *ril_cmd, UInt32 total_line, byte line, byte *txt, bool reverse, bool is_full_update);


//++ JSHAN_GPRS_Attach_Mode
static void KRIL_SRIL_requestOemSvcGPRSAttachMode_Get(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);
static void KRIL_SRIL_requestOemSvcGPRSAttachMode_Key(void *ril_cmd, unsigned char iKey, Kril_CAPI2Info_t *capi2_rsp);
static void KRIL_SRIL_requestOemSvcGPRSAttachMode_Set(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp, UInt8 attach_mode);
//-- JSHAN_GPRS_Attach_Mode

/*+20110418 BCOM PATCH FOR DebugScreen*/
static void KRIL_SRIL_requestOemSvcTest(void *ril_cmd,  Kril_CAPI2Info_t *capi2_rsp);
/*-20110418 BCOM PATCH FOR DebugScreen*/

/*  [Structure] pdata->ril_cmd->data				*/

/*													*/

/*	[0] byte cmd 									*/

/*	[1] byte svcMode 								*/

/*	[2] short fileSize								*/

/*	[4] byte modeType 								*/

/*	[5] byte subType 								*/

/*	[6] byte query 									*/

/****************************************************/

//++ JSHAN_GPRS_Attach_Mode
static bool bGprsAttach;
//++ JSHAN Attach for next power on
UInt8 vGprsAttachMode = 0; 
//-- JSHAN Attach for next power on
//-- JSHAN_GPRS_Attach_Mode

/*+20110418 BCOM PATCH FOR DebugScreen*/
static bool bDebugScreen;
/*-20110418 BCOM PATCH FOR DebugScreen*/

char                testbuffer[11][32];



void KRIL_SRIL_requestSvcModeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)

{

	KRIL_CmdList_t * pdata = (KRIL_CmdList_t*)ril_cmd;

	OemReqMsgHdr * pSvcMsg = NULL;

	UInt8 svcMode;



	pSvcMsg = (OemReqMsgHdr *)pdata->ril_cmd->data;

	svcMode = (UInt8)pSvcMsg->subfId;	



	KRIL_DEBUG(DBG_ERROR,"svcMode:%d\n", svcMode);

				

	switch (svcMode)

	{

		case OEM_SVC_ENTER_MODE_MESSAGE :

			KRIL_SRIL_requestOemSvcEnterModeMessage(ril_cmd, capi2_rsp);

			break;

			

		case OEM_SVC_PROCESS_KEY_MESSAGE :

		{

			OemSvcProcKeyMsg * pSvcKeyMsg = (OemSvcProcKeyMsg *)pdata->ril_cmd->data;

			KRIL_SRIL_requestOemSvcProcessKeyCodeMessage(ril_cmd, pSvcKeyMsg->keyCode, capi2_rsp);

		}

			break;

			

		case OEM_SVC_END_MODE_MESSAGE :

		{

			OemSvcEndModeMsg * pSvcEndMsg = (OemSvcEndModeMsg *)pdata->ril_cmd->data;

			KRIL_SRIL_requestOemSvcEndModeMessage(ril_cmd, pSvcEndMsg->modeType, capi2_rsp);

		}

			break;

			

		case OEM_SVC_DEBUG_DUMP_MESSAGE :

		case OEM_SVC_DEBUG_STRING_MESSAGE :

		default : 

			KRIL_DEBUG(DBG_ERROR,"Unsupported subCmd:%d Error!!!\n", svcMode);

			pdata->handler_state = BCM_ErrorCAPI2Cmd;

			break;

	}

	

}



static void KRIL_SRIL_requestOemSvcEnterModeMessage(void *ril_cmd,  Kril_CAPI2Info_t *capi2_rsp)

{

	UInt8 modetype;

	UInt8 subtype;


	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

	OemSvcEnterModeMsg * pSvcEnterMsg = NULL;



	pSvcEnterMsg = (OemSvcEnterModeMsg *)pdata->ril_cmd->data;



	modetype = (UInt8)pSvcEnterMsg->modeType;

	subtype = (UInt8)pSvcEnterMsg->subType;

	KRIL_DEBUG(DBG_ERROR,"modetype:%d, subtype :%d\n", modetype,subtype);



	switch (modetype)

	{

		case SVC_MODE_TEST_MANUAL:              /* 0x01 : Manual test mode */

			KRIL_SRIL_requestOemSvcModeTestManual(ril_cmd, capi2_rsp);

		    break;



		case SVC_MODE_TEST_AUTO:                /* 0x02 : Auto test mode */    

		    break;



		case SVC_MODE_NAM:                      /* 0x03 : Full NAM edit mode */

		    break;



		case SVC_MODE_MONITOR:    // *#0011# : debug screen              /* 0x04 : Monitor screen mode */

			KRIL_DEBUG(DBG_ERROR,"SVC_MODE_MONITOR:%d !\n", modetype);
			/*+20110418 BCOM PATCH FOR DebugScreen*/
			KRIL_SRIL_requestOemSvcTest(ril_cmd, capi2_rsp);
			/*-20110418 BCOM PATCH FOR DebugScreen*/
		    break;



		case SVC_MODE_PHONE_TEST:               /* 0x05 : Phone test mode ( just for the debugging ) */

		    break;

			

		case SVC_MODE_OPERATOR_SPECIFIC_TEST:   /*0x06: Specific test mode required by operator*/

		    break;



		default : 

			KRIL_DEBUG(DBG_ERROR,"Unsupported modetype:%d Error!!!\n", modetype);

			pdata->handler_state = BCM_ErrorCAPI2Cmd;

			break;

	}

	

}





static void KRIL_SRIL_requestOemSvcEndModeMessage(void *ril_cmd, unsigned char SvcMode, Kril_CAPI2Info_t *capi2_rsp)

{

    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;



	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestOemSvcEndModeMessage");
        //++ JSHAN_GPRS_Attach_Mode
	bGprsAttach = 0;
        //-- JSHAN_GPRS_Attach_Mode

	/*+20110418 BCOM PATCH FOR DebugScreen*/
	bDebugScreen = 0;
	/*-20110418 BCOM PATCH FOR DebugScreen*/
	return;

}



static void KRIL_SRIL_requestOemSvcProcessKeyCodeMessage(void *ril_cmd, unsigned char iKey, Kril_CAPI2Info_t *capi2_rsp)

{

    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;



	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestOemSvcProcessKeyCodeMessage = iKey : %d \n",iKey);



	//++ JSHAN_GPRS_Attach_Mode
	if ( bGprsAttach == 1 )	
	{
		KRIL_SRIL_requestOemSvcGPRSAttachMode_Key(ril_cmd, iKey, capi2_rsp);	
		return;
	}
        //-- JSHAN_GPRS_Attach_Mode

	/*+20110418 BCOM PATCH FOR DebugScreen*/
	else if(  bDebugScreen == 1 )	//if svc window need to update 
	{
		KRIL_SRIL_requestOemSvcTest(ril_cmd,  capi2_rsp);
	}
	/*-20110418 BCOM PATCH FOR DebugScreen*/

	return;

}





static void KRIL_SRIL_requestOemSvcModeTestManual(void *ril_cmd,  Kril_CAPI2Info_t *capi2_rsp)

{

	UInt8 subtype;

	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

	OemSvcEnterModeMsg * pSvcEnterMsg = NULL;



	pSvcEnterMsg = (OemSvcEnterModeMsg *)pdata->ril_cmd->data;


        /*+ Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee +*/
        KrilStackNvramClassMark_t tdata_class; 
	CAPI2_MS_Element_t tdata_ms;
	//tdata_ms = kmalloc(sizeof(CAPI2_MS_Element_t), GFP_KERNEL);
        /*- Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee -*/
        /*+ Band Selection sh0515.lee +*/  
        KrilSrilSetBand_t tdata_band;
        /*- Band Selection sh0515.lee -*/

	/*+ sh0515.lee - Band Selection for CSC update +*/
	UInt8 funcId = pSvcEnterMsg -> hdr.funcId;	
	//#define OEM_FUNCTION_ID_IMEI				0x06
	/*- sh0515.lee - Band Selection for CSC update -*/



	subtype = (UInt8)pSvcEnterMsg->subType;	//subtype	

        //++ JSHAN_GPRS_Attach_Mode
	bGprsAttach = 0; //init test mode value
        //-- JSHAN_GPRS_Attach_Mode

	switch (subtype) //each func. match by subtype.

	{

		case TST_TESTMODE_ENTER: //*#197328640# Admin menu

			KRIL_DEBUG(DBG_ERROR,"TST_TESTMODE_ENTER:%d !!!\n", subtype);

		    break;



		case TST_SW_VERSION_ENTER:

		{

			OemSvcModeRsp * pSvcRsp = NULL;

			OemSvcModeRsp * pTmpRsp = NULL;

			int lenVersion = 0;

			const int numLine = 3;

			char sVersion[100] = {0x0,};

						

			KRIL_DEBUG(DBG_ERROR,"TST_SW_VERSION_ENTER:%d !!!\n", subtype);



			pSvcRsp =  kmalloc(sizeof(OemSvcModeRsp) * numLine, GFP_KERNEL);

		    if (!pSvcRsp)

		    {

		        pdata->handler_state = BCM_ErrorCAPI2Cmd;

		        return;

		    }  

		    memset(pSvcRsp, 0, sizeof(OemSvcModeRsp) * numLine);



			pTmpRsp = pSvcRsp;	// Line[0]

			//lenVersion = property_get("ro.build.PDA", sVersion, NULL);

			pTmpRsp->line = 0;

			pTmpRsp->reverse = 0;

			//strncpy(pTmpRsp->strin\g, sVersion, lenVersion<MAX_SVCSTR_PER_LINE ? lenVersion:MAX_SVCSTR_PER_LINE-1);

			sprintf(pTmpRsp->string, "SW Version : %s", "S5360XXKC1");

			KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);



			

			pTmpRsp++;	// Line[1]

			pTmpRsp->line = 1;

			pTmpRsp->reverse = 0;

			sprintf(pTmpRsp->string, "Compile Date: %s", __DATE__);

			KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);



			pTmpRsp++;	// Line[2]

			pTmpRsp->line = 2;

			pTmpRsp->reverse = 0;

			sprintf(pTmpRsp->string, "Compile Time: %s", __TIME__);

			KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);





			pdata->bcm_ril_rsp = pSvcRsp;

			pdata->handler_state = BCM_FinishCAPI2Cmd;

			pdata->rsp_len = sizeof(OemSvcModeRsp) * numLine;

		}

		    break;

		    

		case TST_FTA_SW_VERSION_ENTER:	//*#1111# : OEM_SM_TYPE_SUB_TST_FTA_SW_VERSION_ENTER

			KRIL_DEBUG(DBG_ERROR,"TST_FTA_SW_VERSION_ENTER:%d !!!\n", subtype);

		    break;



		case TST_FTA_HW_VERSION_ENTER:	//*#2222# : OEM_SM_TYPE_SUB_TST_FTA_HW_VERSION_ENTER

			KRIL_DEBUG(DBG_ERROR,"TST_FTA_HW_VERSION_ENTER:%d !!!\n", subtype);

		    break;

		    

		case TST_ALL_VERSION_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_ALL_VERSION_ENTER:%d !!!\n", subtype);

		    break;



		case TST_BATTERY_INFO_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_BATTERY_INFO_ENTER:%d !!!\n", subtype);

		    break;



		case TST_CIPHERING_PROTECTION_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_CIPHERING_PROTECTION_ENTER:%d !!!\n", subtype);			

		    break;



		case TST_INTEGRITY_PROTECTION_ENTER: 

			KRIL_DEBUG(DBG_ERROR,"TST_INTEGRITY_PROTECTION_ENTER:%d !!!\n", subtype);

		    break;



		case TST_IMEI_READ_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_IMEI_READ_ENTER:%d !!!\n", subtype);



			KRIL_GetIMEIHandler(ril_cmd, capi2_rsp);



			if(pdata->handler_state == BCM_FinishCAPI2Cmd)

			{

				OemSvcModeRsp * pSvcRsp = NULL;

				OemSvcModeRsp * pTmpRsp = NULL;



				KrilImeiData_t * imei_result;



				pSvcRsp =  kmalloc(sizeof(OemSvcModeRsp)*11, GFP_KERNEL);

			    if (!pSvcRsp)

			    {

			        pdata->handler_state = BCM_ErrorCAPI2Cmd;

			        return;

			    }  

			    

			    memset(pSvcRsp, 0, sizeof(OemSvcModeRsp)*MAX_SVCMENU_LINE);

			    pTmpRsp = pSvcRsp;

			    imei_result = (KrilImeiData_t *)pdata->bcm_ril_rsp;

			    

				pTmpRsp->line = 0;

				pTmpRsp->reverse = 0;

				sprintf(pTmpRsp->string, "IMEI :");



				pTmpRsp++;

				pTmpRsp->line = 1;

				pTmpRsp->reverse = 0;

				strncpy(pTmpRsp->string, imei_result->imei, IMEI_DIGITS + 1);



				kfree(pdata->bcm_ril_rsp);

				pdata->bcm_ril_rsp = pSvcRsp;

				pdata->rsp_len = sizeof(OemSvcModeRsp)*MAX_SVCMENU_LINE;

			}



			break;

			

		case TST_BLUETOOTH_TEST_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_BLUETOOTH_TEST_ENTER:%d !!!\n", subtype);

		    break;



		case TST_VIBRATOR_TEST_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_VIBRATOR_TEST_ENTER:%d !!!\n", subtype);

		    break;



		case TST_MELODY_TEST_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_MELODY_TEST_ENTER:%d !!!\n", subtype);

		    break;



		case TST_MP3_TEST_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_MP3_TEST_ENTER:%d !!!\n", subtype);

		    break;



		case TST_FACTORY_RESET_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_FACTORY_RESET_ENTER:%d !!!\n", subtype);			

		    break;



		case TST_FACTORY_PRECONFIG_ENTER: 

			KRIL_DEBUG(DBG_ERROR,"TST_FACTORY_PRECONFIG_ENTER:%d !!!\n", subtype);

		    break;



		case TST_TFS4_EXPLORE_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_TFS4_EXPLORE_ENTER:%d !!!\n", subtype);

		    break;



		case TST_RTC_TIME_DISPLAY_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_RTC_TIME_DISPLAY_ENTER:%d !!!\n", subtype);

		    break;



		case TST_RSC_FILE_VERSION_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_RSC_FILE_VERSION_ENTER:%d !!!\n", subtype);

		    break;



		case TST_USB_DRIVER_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_USB_DRIVER_ENTER:%d !!!\n", subtype);

		    break;



		case TST_USB_UART_DIAG_CONTROL_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_USB_UART_DIAG_CONTROL_ENTER:%d !!!\n", subtype);			

		    break;



		case TST_RRC_VERSION_ENTER: 

			KRIL_DEBUG(DBG_ERROR,"TST_RRC_VERSION_ENTER:%d !!!\n", subtype);

		    break;



		case TST_GPSONE_SS_TEST_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_GPSONE_SS_TEST_ENTER:%d !!!\n", subtype);

		    break;



		case TST_BAND_SEL_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_BAND_SEL_ENTER:%d !!!\n", subtype);

		    break;



		case TST_GCF_TESTMODE_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_GCF_TESTMODE_ENTER:%d !!!\n", subtype);

		    break;



		case TST_GSM_FACTORY_AUDIO_LB_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_GSM_FACTORY_AUDIO_LB_ENTER:%d !!!\n", subtype);

		    break;



		case TST_FACTORY_VF_TEST_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_FACTORY_VF_TEST_ENTER:%d !!!\n", subtype);			

		    break;



		case TST_TOTAL_CALL_TIME_INFO_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_TOTAL_CALL_TIME_INFO_ENTER:%d !!!\n", subtype);

		    break;



		case TST_SELLOUT_SMS_ENABLE_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_SELLOUT_SMS_ENABLE_ENTER:%d !!!\n", subtype);

		    break;



		case TST_SELLOUT_SMS_DISABLE_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_SELLOUT_SMS_DISABLE_ENTER:%d !!!\n", subtype);

		    break;



		case TST_SELLOUT_SMS_TEST_MODE_ON:

			KRIL_DEBUG(DBG_ERROR,"TST_SELLOUT_SMS_TEST_MODE_ON:%d !!!\n", subtype);

		    break;



		case TST_SELLOUT_SMS_PRODUCT_MODE_ON:

			KRIL_DEBUG(DBG_ERROR,"TST_SELLOUT_SMS_PRODUCT_MODE_ON:%d !!!\n", subtype);			

		    break;



		case TST_GET_SELLOUT_SMS_INFO_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_GET_SELLOUT_SMS_INFO_ENTER:%d !!!\n", subtype);

		    break;



		case TST_WCDMA_SET_CHANNEL_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_WCDMA_SET_CHANNEL_ENTER:%d !!!\n", subtype);

		    break;



		case TST_GPRS_ATTACH_MODE_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_GPRS_ATTACH_MODE_ENTER:%d !!!\n", subtype);
			//++ JSHAN_GPRS_Attach_Mode					
			KRIL_SRIL_requestOemSvcGPRSAttachMode_Get(ril_cmd, capi2_rsp);			
			//-- JSHAN_GPRS_Attach_Mode
		    break;



		case TST_IMSI_READ_ENTER:

			KRIL_DEBUG(DBG_ERROR,"TST_IMSI_READ_ENTER:%d !!!\n", subtype);



			KRIL_GetIMSIHandler(ril_cmd, capi2_rsp);



			if(pdata->handler_state == BCM_FinishCAPI2Cmd)

			{

				OemSvcModeRsp * pSvcRsp = NULL;

				OemSvcModeRsp * pTmpRsp = NULL;



				KrilImsiData_t * imsi_result;



				pSvcRsp =  kmalloc(sizeof(OemSvcModeRsp)*MAX_SVCMENU_LINE, GFP_KERNEL);

			    if (!pSvcRsp)

			    {

			        pdata->handler_state = BCM_ErrorCAPI2Cmd;

			        return;

			    }  

			    

			    memset(pSvcRsp, 0, sizeof(OemSvcModeRsp)*MAX_SVCMENU_LINE);

			    pTmpRsp = pSvcRsp;				

			    imsi_result = (KrilImsiData_t *)pdata->bcm_ril_rsp;



   				pTmpRsp->line = 0;

				pTmpRsp->reverse = 0;

				sprintf(pTmpRsp->string, "IMSI :");



				pTmpRsp++;

				pTmpRsp->line = 1;

				pTmpRsp->reverse = 0;

				strncpy(pTmpRsp->string, imsi_result->imsi, IMSI_DIGITS+1);



				kfree(pdata->bcm_ril_rsp);

				pdata->bcm_ril_rsp = pSvcRsp;

				pdata->rsp_len = sizeof(OemSvcModeRsp)*MAX_SVCMENU_LINE;

			}

		    break;	



		case TST_LOG_KRIL_ENABLE:

		{

			BCMLOG_IoctlEnable_t arg_log;

			int fd_log;

			int result;

			mm_segment_t old_fs = get_fs();

			set_fs(KERNEL_DS);

			

			arg_log.id=BCMLOG_ANDROID_KRIL_DETAIL;

			arg_log.set=1;

			arg_log.enable=1;

			KRIL_DEBUG(DBG_ERROR,"TST_LOG_KRIL_ENABLE:%d !!!\n", subtype);	

			



			fd_log= sys_open( "/dev/"BCMLOG_MODULE_NAME, O_RDWR,0) ;

			

			if ( fd_log <= 0)

			{

			       result = fd_log;

			}

			result = sys_ioctl(fd_log, BCMLOG_IOC_ENABLE, &arg_log);

			sys_close(fd_log);



			set_fs(old_fs);

			sprintf(testbuffer[0]," Logging");

			sprintf(testbuffer[1]," [1] On");

			KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);			

		    break;				

		}

		case TST_LOG_KRIL_DISABLE:

		{

			BCMLOG_IoctlEnable_t arg_log;

			int fd_log;

			int result;

			mm_segment_t old_fs = get_fs();

			set_fs(KERNEL_DS);



			arg_log.id=BCMLOG_ANDROID_KRIL_DETAIL;

			arg_log.set=1;

			arg_log.enable=0;

			KRIL_DEBUG(DBG_ERROR,"TST_LOG_KRIL_DISABLE:%d !!!\n", subtype);		

			

			fd_log= sys_open( "/dev/"BCMLOG_MODULE_NAME, O_RDWR,0) ;

			

			if ( fd_log <= 0)

			{

			       result = fd_log;

			}

			result = sys_ioctl(fd_log, BCMLOG_IOC_ENABLE, &arg_log);

			sys_close(fd_log);



			set_fs(old_fs);

			sprintf(testbuffer[0]," Logging");

			sprintf(testbuffer[1]," [1] Off");

			KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);			

		    break;				

		}

		case TST_SIM_INFO_ENTER : //Irine_SIMInfo_SvcMenu

			{

			int simAppType = 0;

			int sim_Phase = 0;

			char card_type[10] = {0x0,};



			KRIL_DEBUG(DBG_ERROR,"TST_SIM_INFO_ENTER:subtype(%d) !!!\n", subtype);



			sprintf(testbuffer[0]," SIM Phase : %d", sim_Phase);

			KRIL_DEBUG(DBG_ERROR,"TST_SIM_INFO_ENTER:1111\n");



			simAppType = KRIL_GetSimAppType();//Card Capability / EF_category

			

			KRIL_DEBUG(DBG_ERROR,"TST_SIM_INFO_ENTER:simAppType(%d) !!!\n", simAppType);



			if(simAppType ==SIM_APPL_2G ) 

			{

				sprintf(&testbuffer[1]," Card Capability : GSM");

				sprintf(&testbuffer[2]," EF_category : GSM\n");	

			}

			else if(simAppType == SIM_APPL_3G)

			{

				sprintf(&testbuffer[1]," Card Capability : USIM");

				sprintf(&testbuffer[2]," EF_category : USIM");	

			}

			else

			{

				sprintf(&testbuffer[1]," Card Capability : Invalid");

				sprintf(&testbuffer[2]," EF_category : Invalid");	

			}

			

			KRIL_DEBUG(DBG_ERROR,"TST_SIM_INFO_ENTER:card_type(%s) !!!\n", card_type);



			sprintf(testbuffer[3]," SIM_PB : 0");			

			sprintf(testbuffer[4]," USIM_global_PB : 1");

			sprintf(testbuffer[5]," USIM_local_PB : 0");

			sprintf(testbuffer[6]," First_inst_class : GSM");



			//sprintf(testbuffer[7],"SIM voltage class : 3V SIM\n")

			KRIL_SRIL_svc_update_screen(ril_cmd, 7, 0, (unsigned char*)testbuffer, TRUE, TRUE);

			KRIL_DEBUG(DBG_ERROR,"TST_SIM_INFO_ENTER:END OF CASE\n");



			}

		    break;	

/*+ Band Selection sh0515.lee +*/	
		case TST_SET_BAND_AUTO:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_AUTO:%d !!!\n", subtype);

			tdata_band.curr_rat = DUAL_MODE_UMTS_PREF; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_AUTO; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "DUAL_MODE_UMTS_PREF");
				sprintf(testbuffer[1], "BAND: %s", "BAND_AUTO");

				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/				
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
				
			}
			
		    break;

		case TST_SET_BAND_ALL_GSM:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_ALL_GSM:%d !!!\n", subtype);

			tdata_band.curr_rat = GSM_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_AUTO; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "GSM_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_AUTO");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/				
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
			
		    break;
			
		case TST_SET_BAND_GSM_850:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_GSM_850:%d !!!\n", subtype);

			tdata_band.curr_rat = GSM_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_GSM850_ONLY; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "GSM_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_GSM850_ONLY");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/				
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
			
		    break;

		case TST_SET_BAND_GSM_900:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_GSM_900:%d !!!\n", subtype);
                     printk("TST_SET_BAND_GSM_900:%d !!!\n", subtype);

			tdata_band.curr_rat = GSM_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_GSM900_ONLY; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "GSM_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_GSM900_ONLY");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/				
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
			
		    break;

		case TST_SET_BAND_DCS_1800:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_DCS_1800:%d !!!\n", subtype);

			tdata_band.curr_rat = GSM_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_DCS1800_ONLY; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "GSM_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_DCS_1800");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
			
		    break;

		case TST_SET_BAND_PCS_1900:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_PCS_1900:%d !!!\n", subtype);

			tdata_band.curr_rat = GSM_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_PCS1900_ONLY; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "GSM_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_PCS1900_ONLY");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
				
			}
			
		    break;

		case TST_SET_BAND_ALL_UMTS:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_ALL_UMTS:%d !!!\n", subtype);

			tdata_band.curr_rat = UMTS_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_AUTO; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{

				sprintf(testbuffer[0], "RAT : %s", "UMTS_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_AUTO");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
			
		    break;

		case TST_SET_BAND_UMTS_2100:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_UMTS_2100:%d !!!\n", subtype);

			tdata_band.curr_rat = UMTS_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_UMTS2100_ONLY; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "UMTS_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_UMTS2100_ONLY");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
			
		    break;

		case TST_SET_BAND_UMTS_1900:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_UMTS_1900:%d !!!\n", subtype);

			tdata_band.curr_rat = UMTS_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_UMTS1900_ONLY; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "UMTS_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_UMTS1900_ONLY");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
			
		    break;
			
		case TST_SET_BAND_UMTS_900:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_UMTS_900:%d !!!\n", subtype);

			tdata_band.curr_rat = UMTS_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_UMTS900_ONLY; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "UMTS_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_UMTS900_ONLY");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
			
		    break;

		case TST_SET_BAND_UMTS_850:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_BAND_UMTS_850:%d !!!\n", subtype);

			tdata_band.curr_rat = UMTS_ONLY; //(RATSelect_t) tdata->curr_rat, 
			tdata_band.new_band = BAND_UMTS850_ONLY; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetBandHandler(ril_cmd, capi2_rsp, &tdata_band);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{
				sprintf(testbuffer[0], "RAT : %s", "UMTS_ONLY");
				sprintf(testbuffer[1], "BAND: %s", "BAND_UMTS850_ONLY");
				
				/*+ sh0515.lee - Band Selection for CSC update +*/
				if (funcId != 0x06) // OEM_FUNCTION_ID_IMEI
				/*- sh0515.lee - Band Selection for CSC update -*/
					KRIL_SRIL_svc_update_screen(ril_cmd, 2, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
			
		    break;			
/*- Band Selection sh0515.lee -*/
/*+ Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee +*/
		case TST_SET_CIPHERING_MODE_ON:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_CIPHERING_MODE_ON:%d !!!\n", subtype);

			//KrilStackNvramClassMark_t *tdata_cipher = NULL; // (KrilStackNvramClassMark_t *) pdata->ril_cmd->data;
			//tdata_cipher = kmalloc(sizeof(KrilStackNvramClassMark_t), GFP_KERNEL);

			tdata_class.classmark_id = KRIL_CIPHERING_MODE; 
			tdata_class.data_u.is_supported = 1; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetStackClasssMarkHandler(ril_cmd, capi2_rsp, &tdata_class);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{/*
				OemSvcModeRsp * pSvcRsp = NULL;
				OemSvcModeRsp * pTmpRsp = NULL;
				const int numLine = 2;

				KrilSrilSetBand_t * bandsel_result;

				pSvcRsp =  kmalloc(sizeof(OemSvcModeRsp)*MAX_SVCMENU_LINE, GFP_KERNEL);
			    if (!pSvcRsp)
			    {
			        pdata->handler_state = BCM_ErrorCAPI2Cmd;
			        return;
			    }  
			    
				memset(pSvcRsp, 0, sizeof(OemSvcModeRsp) * numLine);

				pTmpRsp = pSvcRsp;	// Line[0]
				//lenVersion = property_get("ro.build.PDA", sVersion, NULL);
				pTmpRsp->line = 0;
				pTmpRsp->reverse = 0;
				//strncpy(pTmpRsp->strin\g, sVersion, lenVersion<MAX_SVCSTR_PER_LINE ? lenVersion:MAX_SVCSTR_PER_LINE-1);
				sprintf(pTmpRsp->string, "classmark_id : %d", KRIL_CIPHERING_MODE);
				KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);

				
				pTmpRsp++;	// Line[1]
				pTmpRsp->line = 1;
				pTmpRsp->reverse = 0;
				sprintf(pTmpRsp->string, "data_u.is_supported: %d", 1);
				KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);

				pdata->bcm_ril_rsp = pSvcRsp;
				pdata->handler_state = BCM_FinishCAPI2Cmd;
				pdata->rsp_len = sizeof(OemSvcModeRsp) * numLine;	*/

				sprintf(testbuffer[0], "CIPHERING_MODE_ON");
				
				KRIL_SRIL_svc_update_screen(ril_cmd, 1, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
		    break;

		case TST_SET_CIPHERING_MODE_OFF:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_CIPHERING_MODE_OFF:%d !!!\n", subtype);

			//KrilStackNvramClassMark_t *tdata_cipher = NULL; // (KrilStackNvramClassMark_t *) pdata->ril_cmd->data;
			//tdata_cipher = kmalloc(sizeof(KrilStackNvramClassMark_t), GFP_KERNEL);

			tdata_class.classmark_id = KRIL_CIPHERING_MODE; 
			
			tdata_class.data_u.is_supported = 0; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetStackClasssMarkHandler(ril_cmd, capi2_rsp, &tdata_class);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{/*
				OemSvcModeRsp * pSvcRsp = NULL;
				OemSvcModeRsp * pTmpRsp = NULL;
				const int numLine = 2;

				KrilSrilSetBand_t * bandsel_result;

				pSvcRsp =  kmalloc(sizeof(OemSvcModeRsp)*MAX_SVCMENU_LINE, GFP_KERNEL);
			    if (!pSvcRsp)
			    {
			        pdata->handler_state = BCM_ErrorCAPI2Cmd;
			        return;
			    }  
			    
				memset(pSvcRsp, 0, sizeof(OemSvcModeRsp) * numLine);

				pTmpRsp = pSvcRsp;	// Line[0]
				//lenVersion = property_get("ro.build.PDA", sVersion, NULL);
				pTmpRsp->line = 0;
				pTmpRsp->reverse = 0;
				//strncpy(pTmpRsp->strin\g, sVersion, lenVersion<MAX_SVCSTR_PER_LINE ? lenVersion:MAX_SVCSTR_PER_LINE-1);
				sprintf(pTmpRsp->string, "classmark_id : %d", KRIL_CIPHERING_MODE);
				KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);

				
				pTmpRsp++;	// Line[1]
				pTmpRsp->line = 1;
				pTmpRsp->reverse = 0;
				sprintf(pTmpRsp->string, "data_u.is_supported: %d", 0);
				KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);

				pdata->bcm_ril_rsp = pSvcRsp;
				pdata->handler_state = BCM_FinishCAPI2Cmd;
				pdata->rsp_len = sizeof(OemSvcModeRsp) * numLine;		*/

				sprintf(testbuffer[0], "CIPHERING_MODE_OFF");
				
				KRIL_SRIL_svc_update_screen(ril_cmd, 1, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
		    break;

		case TST_GET_CIPHERING_MODE:
				
			tdata_class.classmark_id = KRIL_CIPHERING_MODE; 
			tdata_ms = KRIL_SRIL_GetStackClasssMarkHandler(ril_cmd, capi2_rsp, &tdata_class);

			//data.data_u.stackClassmark.uasConfigParams.ciphering_updated = rsp->data_u.stackClassmark.uasConfigParams.ciphering_updated;
			//data.data_u.stackClassmark.uasConfigParams.ciphering_support = rsp->data_u.stackClassmark.uasConfigParams.ciphering_support;
					
			if (tdata_ms.data_u.stackClassmark.uasConfigParams.ciphering_support) //UInt8 ( ATTACH_MODE_GSM_GPRS=1,ATTACH_MODE_GSM_ONLY=2,ATTACH_MODE_GPRS_ONLY=3)
				sprintf(testbuffer[0]," CIPHERING MODE SUPPORT : TRUE");
			else
				sprintf(testbuffer[0]," CIPHERING MODE SUPPORT : FALSE");

			KRIL_SRIL_svc_update_screen(ril_cmd, 1, 0, (unsigned char*)testbuffer, TRUE, TRUE);

		    break;
			
		case TST_SET_INTEGRATE_MODE_ON:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_INTEGRATE_MODE_ON:%d !!!\n", subtype);

			//KrilStackNvramClassMark_t *tdata_cipher = NULL; // (KrilStackNvramClassMark_t *) pdata->ril_cmd->data;
			//tdata_cipher = kmalloc(sizeof(KrilStackNvramClassMark_t), GFP_KERNEL);

			tdata_class.classmark_id = KRIL_INTEGRATE_MODE; 
			tdata_class.data_u.is_supported = 1; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetStackClasssMarkHandler(ril_cmd, capi2_rsp, &tdata_class);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{/*
				OemSvcModeRsp * pSvcRsp = NULL;
				OemSvcModeRsp * pTmpRsp = NULL;
				const int numLine = 2;

				KrilSrilSetBand_t * bandsel_result;

				pSvcRsp =  kmalloc(sizeof(OemSvcModeRsp)*MAX_SVCMENU_LINE, GFP_KERNEL);
			    if (!pSvcRsp)
			    {
			        pdata->handler_state = BCM_ErrorCAPI2Cmd;
			        return;
			    }  
			    
				memset(pSvcRsp, 0, sizeof(OemSvcModeRsp) * numLine);

				pTmpRsp = pSvcRsp;	// Line[0]
				//lenVersion = property_get("ro.build.PDA", sVersion, NULL);
				pTmpRsp->line = 0;
				pTmpRsp->reverse = 0;
				//strncpy(pTmpRsp->strin\g, sVersion, lenVersion<MAX_SVCSTR_PER_LINE ? lenVersion:MAX_SVCSTR_PER_LINE-1);
				sprintf(pTmpRsp->string, "classmark_id : %d", KRIL_INTEGRATE_MODE);
				KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);

				
				pTmpRsp++;	// Line[1]
				pTmpRsp->line = 1;
				pTmpRsp->reverse = 0;
				sprintf(pTmpRsp->string, "data_u.is_supported: %d", 1);
				KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);

				pdata->bcm_ril_rsp = pSvcRsp;
				pdata->handler_state = BCM_FinishCAPI2Cmd;
				pdata->rsp_len = sizeof(OemSvcModeRsp) * numLine;	*/

				sprintf(testbuffer[0], "INTEGRATE_MODE_ON");
				
				KRIL_SRIL_svc_update_screen(ril_cmd, 1, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
		    break;

		case TST_SET_INTEGRATE_MODE_OFF:
			KRIL_DEBUG(DBG_ERROR,"TST_SET_INTEGRATE_MODE_OFF:%d !!!\n", subtype);

			//KrilStackNvramClassMark_t *tdata_cipher = NULL; // (KrilStackNvramClassMark_t *) pdata->ril_cmd->data;
			//tdata_cipher = kmalloc(sizeof(KrilStackNvramClassMark_t), GFP_KERNEL);

			tdata_class.classmark_id = KRIL_INTEGRATE_MODE; 
			tdata_class.data_u.is_supported = 0; //(BandSelect_t) tdata->new_band
			KRIL_SRIL_SetStackClasssMarkHandler(ril_cmd, capi2_rsp, &tdata_class);

			if(pdata->handler_state == BCM_FinishCAPI2Cmd)
			{/*
				OemSvcModeRsp * pSvcRsp = NULL;
				OemSvcModeRsp * pTmpRsp = NULL;
				const int numLine = 2;

				KrilSrilSetBand_t * bandsel_result;

				pSvcRsp =  kmalloc(sizeof(OemSvcModeRsp)*MAX_SVCMENU_LINE, GFP_KERNEL);
			    if (!pSvcRsp)
			    {
			        pdata->handler_state = BCM_ErrorCAPI2Cmd;
			        return;
			    }  
			    
				memset(pSvcRsp, 0, sizeof(OemSvcModeRsp) * numLine);

				pTmpRsp = pSvcRsp;	// Line[0]
				//lenVersion = property_get("ro.build.PDA", sVersion, NULL);
				pTmpRsp->line = 0;
				pTmpRsp->reverse = 0;
				//strncpy(pTmpRsp->strin\g, sVersion, lenVersion<MAX_SVCSTR_PER_LINE ? lenVersion:MAX_SVCSTR_PER_LINE-1);
				sprintf(pTmpRsp->string, "classmark_id : %d", KRIL_INTEGRATE_MODE);
				KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);

				
				pTmpRsp++;	// Line[1]
				pTmpRsp->line = 1;
				pTmpRsp->reverse = 0;
				sprintf(pTmpRsp->string, "data_u.is_supported: %d", 0);
				KRIL_DEBUG(DBG_ERROR, "%s", pTmpRsp->string);

				pdata->bcm_ril_rsp = pSvcRsp;
				pdata->handler_state = BCM_FinishCAPI2Cmd;
				pdata->rsp_len = sizeof(OemSvcModeRsp) * numLine;		*/

				sprintf(testbuffer[0], "INTEGRATE_MODE_OFF");
				
				KRIL_SRIL_svc_update_screen(ril_cmd, 1, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			}
		    break;			
		
		case TST_GET_INTEGRATE_MODE:
				
			//KrilStackNvramClassMark_t *tdata_cipher = NULL; // (KrilStackNvramClassMark_t *) pdata->ril_cmd->data;
			//tdata_cipher = kmalloc(sizeof(KrilStackNvramClassMark_t), GFP_KERNEL);

			tdata_class.classmark_id = KRIL_INTEGRATE_MODE; 
			//tdata_class.data_u.is_supported = 0; //(BandSelect_t) tdata->new_band
			tdata_ms = KRIL_SRIL_GetStackClasssMarkHandler(ril_cmd, capi2_rsp, &tdata_class);

			//data.data_u.stackClassmark.uasConfigParams.ciphering_updated = rsp->data_u.stackClassmark.uasConfigParams.ciphering_updated;
			//data.data_u.stackClassmark.uasConfigParams.ciphering_support = rsp->data_u.stackClassmark.uasConfigParams.ciphering_support;
					
			if (tdata_ms.data_u.stackClassmark.uasConfigParams.integrity_support) //UInt8 ( ATTACH_MODE_GSM_GPRS=1,ATTACH_MODE_GSM_ONLY=2,ATTACH_MODE_GPRS_ONLY=3)
				sprintf(testbuffer[0]," INTEGRATE MODE SUPPORT : TRUE");
			else
				sprintf(testbuffer[0]," INTEGRATE MODE SUPPORT : FALSE");

			KRIL_SRIL_svc_update_screen(ril_cmd, 1, 0, (unsigned char*)testbuffer, TRUE, TRUE);

		    break;
/*- Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee -*/

		default : 

			KRIL_DEBUG(DBG_ERROR,"Unsupported subtype:%d Error!!!\n", subtype);

			pdata->handler_state = BCM_ErrorCAPI2Cmd;

			break;

	}

	

}


/*+20110418 BCOM PATCH FOR DebugScreen*/
static void KRIL_SRIL_requestOemSvcTest(void *ril_cmd,  Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestOemSvcTest \n");

	/* This is example test code for update svcmode window */	
	bDebugScreen = 1;  //need update this menu.
	/* update screen */
		
	UInt32 TimeInterval = 0;

	switch (pdata->handler_state)
	{
		case BCM_SendCAPI2Cmd:
		{
			KRIL_SetInMeasureReportHandler( TRUE );
			ClientInfo_t clientInfo;
			UInt32 new_tid;
			new_tid = GetNewTID();
			CAPI2_InitClientInfo(&clientInfo, new_tid, GetClientID());
			KRIL_SetMeasureReportTID(new_tid);
			CAPI2_DiagApi_MeasurmentReportReq ( &clientInfo,TRUE,TimeInterval);
			pdata->handler_state = BCM_MeasureReport;
			KRIL_DEBUG(DBG_ERROR,"SendCAPI2Cmd End TID = %lu \n" , new_tid);
		}
		break;

		case BCM_MeasureReport:
		{
			KRIL_DEBUG(DBG_ERROR,"msgType :%x \n", capi2_rsp->msgType );
			
			if (capi2_rsp->result != RESULT_OK)
			{
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
				KRIL_SetInMeasureReportHandler( FALSE );
			}
			else
			{
				if (MSG_MEASURE_REPORT_PARAM_IND == capi2_rsp->msgType)
				{
					MS_RxTestParam_t *rsp = (MS_RxTestParam_t *)capi2_rsp->dataBuf;

					if (1 == rsp->mm_param.rat)
					{
						UInt16 mcc, mnc;
						
						KRIL_DEBUG(DBG_ERROR,"RAT = 1 , PARAMETER SETTING \n");
						KRIL_DEBUG(DBG_ERROR,"VAILD : %d \n",rsp->mm_param.rat);

						// Problem : MCC = 123, MNC = 45  --display--> MCC: F321, MNC: 54 
						mcc = ((rsp->umts_param.plmn_id.mcc & 0x000F)<<8)|(rsp->umts_param.plmn_id.mcc & 0x00F0)|((rsp->umts_param.plmn_id.mcc & 0x0F00)>>8);

						if((rsp->umts_param.plmn_id.mcc & 0xF000) == 0xF000)
						{
							mnc = ((rsp->umts_param.plmn_id.mnc & 0x000F)<<4)|((rsp->umts_param.plmn_id.mnc & 0x00F0)>>4);
						}
						else
						{
							mnc = ((rsp->umts_param.plmn_id.mnc & 0x000F)<<8)|(rsp->umts_param.plmn_id.mnc & 0x00F0)|((rsp->umts_param.plmn_id.mcc & 0xF000)>>12);					
						}

						/*+LKWON Display RRC state +*/
						if(rsp->umts_param.rrc_state == 0)
						{
							sprintf(testbuffer[0], "CELL_DCH");
						}else if(rsp->umts_param.rrc_state == 1)
						{
							sprintf(testbuffer[0], "CELL_FACH");
						}else if(rsp->umts_param.rrc_state == 2)
						{
							sprintf(testbuffer[0], "CELL_PCH");
						}else if(rsp->umts_param.rrc_state == 3)
						{
							sprintf(testbuffer[0], "URA_PCH");
						}else if(rsp->umts_param.rrc_state == 4)
						{
							sprintf(testbuffer[0], "IDLE");
						}else if(rsp->umts_param.rrc_state == 5)
						{
							sprintf(testbuffer[0], "IDLE_CCCH");
						}else
						{
							sprintf(testbuffer[0], "---");
						}

						//sprintf(testbuffer[0],"WCDMA: %d", rsp->umts_param.rrc_state);
						/*-LKWON Display RRC state -*/						
//						sprintf(testbuffer[1],"MCC:%x  MNC:%x",rsp->umts_param.plmn_id.mcc,  rsp->umts_param.plmn_id.mnc);
						sprintf(testbuffer[1],"MCC:%x  MNC:%x",mcc,  mnc);
						sprintf(testbuffer[2],"Rx CH:%d , Rssi:%d",rsp->umts_param.dl_uarfcn, rsp->umts_param.rssi);
						sprintf(testbuffer[3],"Tx CH:%d , TxPwr:%d",rsp->umts_param.ul_uarfcn, rsp->umts_param.tx_pwr);
						sprintf(testbuffer[4],"Ecn0:-%d , RSCP:-%d",rsp->umts_param.cpich_ecn0, rsp->umts_param.cpich_rscp); // yg7948.park@samsung.com, Attach '-' sign in  DBG Screen. 
						sprintf(testbuffer[5],"Chanal Mode:%d",rsp->gsm_param.chan_mode);
						sprintf(testbuffer[6],"PSC:%d , LAC:%d",rsp->umts_param.p_sc, rsp->umts_param.lac);
					}
					
					else if (0 == rsp->mm_param.rat)
					{
						UInt16 mcc, mnc;
						
						KRIL_DEBUG(DBG_ERROR,"RAT = 0 , PARAMETER SETTING \n");
						KRIL_DEBUG(DBG_ERROR,"VAILD : %d \n",rsp->umts_param.valid); 

						// Problem : MCC = 123, MNC = 45  --display--> MCC: 21F3, MNC: 54
						mcc = (rsp->gsm_param.mcc & 0x000F)|((rsp->gsm_param.mcc & 0x0F00))|((rsp->gsm_param.mcc & 0xF000)>>8);

						if((rsp->gsm_param.mcc & 0x00F0) == 0x00F0)
						{
							mnc = ((rsp->gsm_param.mnc & 0x000F)<<4)|((rsp->gsm_param.mnc & 0x00F0)>>4);
						}
						else
						{
							mnc = ((rsp->gsm_param.mnc & 0x000F)<<8)|(rsp->gsm_param.mnc & 0x00F0)|((rsp->gsm_param.mcc & 0x00F0)>>4);					
						}

						sprintf(testbuffer[0],"GSM: %d", rsp->gsm_param.grr_state);
//						sprintf(testbuffer[1],"MCC:%x  MNC:%x",rsp->gsm_param.mcc,	rsp->gsm_param.mnc);
						sprintf(testbuffer[1],"MCC:%x  MNC:%x",mcc,	mnc);
						sprintf(testbuffer[2],"Band:%d  Arfcn:%d",rsp->gsm_param.band,	rsp->gsm_param.arfcn);

						/*110817_bongbong_debugscreen_request_from_H/W Engineering Part*/
						sprintf(testbuffer[3],"Rx Pwr: -%d , Rx Qual:%d",111-(rsp->gsm_param.rxlev), rsp->gsm_param.rxqualfull);
						sprintf(testbuffer[4],"Rx Lev:%d ,  Txlev:%d",rsp->gsm_param.rxlevfull, rsp->gsm_param.txpwr);
						/*110817_bongbong_debugscreen_request_from_H/W Engineering Part*/
						
						sprintf(testbuffer[5],"Chanal Mode:%d",rsp->gsm_param.chan_mode);
						sprintf(testbuffer[6],"Bsic:%d , LAC:%d",rsp->gsm_param.bsic, rsp->gsm_param.lac);
					}
					
					else
					{
						pdata->result = BCM_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW;
					}

					// get measurement data
					ClientInfo_t clientInfo;
					CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
					CAPI2_DiagApi_MeasurmentReportReq ( &clientInfo,FALSE,0);
					//CAPI2_DIAG_ApiMeasurmentReportReq(GetNewTID(), GetClientID(), FALSE, 0);
					pdata->handler_state = BCM_RESPCAPI2Cmd;
					KRIL_DEBUG(DBG_ERROR,"BCM_MeasureReport End \n");
				}

				else
				{
					KRIL_DEBUG(DBG_ERROR,"Go to KRIL_SetMeasureReportTID \n");
					KRIL_SetMeasureReportTID(capi2_rsp->tid);
				}
			}
		}
		break;

		case BCM_RESPCAPI2Cmd:
		{
			if (BCM_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW == pdata->result)
			{
				KRIL_DEBUG(DBG_ERROR,"DebugScreen_ERROR \n");
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
			}
			else
			{
				KRIL_DEBUG(DBG_ERROR,"DebugScreen_END \n");
				KRIL_DEBUG(DBG_ERROR,"BCM_RESPCAPI2Cmd End \n");
				KRIL_SRIL_svc_update_screen(ril_cmd, 7, 0, (unsigned char*)testbuffer, TRUE, TRUE);
				KRIL_SetInMeasureReportHandler( FALSE );
			}
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
/*-20110418 BCOM PATCH FOR DebugScreen*/


//++ JSHAN_GPRS_Attach_Mode
static void KRIL_SRIL_requestOemSvcGPRSAttachMode_Get(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	ClientInfo_t clientInfo;

	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_requestOemSvcGPRSAttachMode \n");

	switch (pdata->handler_state)
	{
		case BCM_SendCAPI2Cmd:
		{
	        	CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
	        	CAPI2_MsDbApi_GetElement ( &clientInfo, MS_LOCAL_PHCTRL_ELEM_ATTACH_MODE);
	        	pdata->handler_state = BCM_RESPCAPI2Cmd;
	        	break;
		}
		
		case BCM_RESPCAPI2Cmd:
		{
			CAPI2_MS_Element_t* rsp = (CAPI2_MS_Element_t*)capi2_rsp->dataBuf;

    			if (!rsp)
    			{
        			KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        			pdata->handler_state = BCM_ErrorCAPI2Cmd;
        			return;
    			}
			if (rsp->inElemType != MS_LOCAL_PHCTRL_ELEM_ATTACH_MODE)
			{
				KRIL_DEBUG(DBG_ERROR,"Attach mode : inElemType Error!! inElemType:%d\n",rsp->inElemType);
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
        			return;
			}
			
			switch(rsp->data_u.u8Data) //UInt8 ( ATTACH_MODE_GSM_GPRS=1,ATTACH_MODE_GSM_ONLY=2,ATTACH_MODE_GPRS_ONLY=3)
			{
				case 1:
					sprintf(testbuffer[0]," GPRS Attach Mode : GSM_GPRS");
					break;
				case 2:
					sprintf(testbuffer[0]," GPRS Attach Mode : GSM_ONLY");
					break;
				case 3:
					sprintf(testbuffer[0]," GPRS Attach Mode : GPRS_ONLY");
					break;
				default:
					sprintf(testbuffer[0]," GPRS Attach Mode : Error %d" , rsp->data_u.u8Data );
					break;
			}

			sprintf(testbuffer[1]," [1] Set to Auto Attach");
			sprintf(testbuffer[2]," [2] Set to Manual Attach");
			sprintf(testbuffer[3],"");
			sprintf(testbuffer[4],"Change Apply to next Power ON");

			bGprsAttach = 1;

			KRIL_SRIL_svc_update_screen(ril_cmd, 5, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			
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


static void KRIL_SRIL_requestOemSvcGPRSAttachMode_Key(void *ril_cmd, unsigned char iKey, Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	UInt8 attach_mode = 0;
	if (iKey == 49)
	{
		attach_mode = 1;
		KRIL_SRIL_requestOemSvcGPRSAttachMode_Set(ril_cmd, capi2_rsp, attach_mode );
	}
	else if (iKey == 50)
	{
		attach_mode = 2;
		KRIL_SRIL_requestOemSvcGPRSAttachMode_Set(ril_cmd, capi2_rsp, attach_mode );
	}

	return;
	
}


static void KRIL_SRIL_requestOemSvcGPRSAttachMode_Set(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp, UInt8 attach_mode)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	CAPI2_MS_Element_t *inElemData;
	ClientInfo_t clientInfo;
//++ JSHAN Attach for next power on
	char tmpStrGPRSattch[5];
//-- JSHAN Attach for next power on
	KRIL_DEBUG(DBG_ERROR,"User Press GPRSAttachMode_Set \n");

	switch (pdata->handler_state)
	{
		case BCM_SendCAPI2Cmd:
		{
			inElemData = kmalloc(sizeof(CAPI2_MS_Element_t), GFP_KERNEL);
	        	inElemData->inElemType = MS_LOCAL_PHCTRL_ELEM_ATTACH_MODE;
			inElemData->data_u.u8Data = attach_mode;
			CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
			CAPI2_MsDbApi_SetElement ( &clientInfo,inElemData);
	        	pdata->handler_state = BCM_RESPCAPI2Cmd;
	        	break;
		}
		
		case BCM_RESPCAPI2Cmd:
		{
			if (attach_mode == 1)
				sprintf(testbuffer[0]," You Select Auto Atttach Menu");
			else if (attach_mode ==  2)
				sprintf(testbuffer[0]," You Select Manual Atttach Menu");
			sprintf(testbuffer[1],"");
			
			if(capi2_rsp->result != RESULT_OK)
			{
				sprintf(testbuffer[2],"Set to Attach Mode Fail !!");
			}
			else
			{
				sprintf(testbuffer[2],"Set to Attach Mode Success^^");
				vGprsAttachMode = attach_mode;

			}
			kfree(inElemData);
			KRIL_SRIL_svc_update_screen(ril_cmd, 3, 0, (unsigned char*)testbuffer, TRUE, TRUE);
			break;
		}
		
		default:
		{
                        kfree(inElemData);
			KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			break;
		}
	}
}



//-- JSHAN_GPRS_Attach_Mode


/* sample code*/

/*

	sprintf(testbuffer[0]," MAIN MENU");

	sprintf(testbuffer[1]," [1] DEBUG SCREEN");

	sprintf(testbuffer[2]," [2] VERSION INFORMATION");

	sprintf(testbuffer[3]," [3] UMTS RF NV");

	sprintf(testbuffer[4]," [4] GSM RF NV");

	sprintf(testbuffer[5]," [5] AUDIO");

	sprintf(testbuffer[6]," [6] COMMON");

	KRIL_SRIL_svc_update_screen(ril_cmd, 7, 0, (unsigned char*)testbuffer, TRUE, TRUE);

*/



extern void KRIL_SRIL_svc_update_screen(void *ril_cmd, UInt32 total_line, byte line, byte *txt, bool reverse, bool is_full_update)

{

	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;



	OemSvcModeRsp * pSvcRsp = NULL;

	OemSvcModeRsp * pTmpRsp = NULL;

	int numLine;;

	int j;



	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_svc_update_screen \n");



	if(is_full_update == TRUE)

	{

		numLine = MAX_SVCMENU_LINE;

	}

	else 

	{

		numLine = total_line;

	}



	pSvcRsp =  kmalloc(sizeof(OemSvcModeRsp) * numLine, GFP_KERNEL);

	if (!pSvcRsp)

	{

		pdata->handler_state = BCM_ErrorCAPI2Cmd;

		return;

	}  

	memset(pSvcRsp, 0, sizeof(OemSvcModeRsp) * numLine);



	for(j=0; j<total_line; j++)

	{	

		if(j==0)

			pTmpRsp = pSvcRsp;	// Line[0]

		else 

			pTmpRsp++;



		pTmpRsp->line = j;

		pTmpRsp->reverse = 0;

		memcpy(pTmpRsp->string, (byte *) (txt+MAX_SVCSTR_PER_LINE*j), MAX_SVCSTR_PER_LINE);

		KRIL_DEBUG(DBG_ERROR, "for Multiple line string  %s \n", pTmpRsp->string);

	}



	pdata->bcm_ril_rsp = pSvcRsp;

	pdata->handler_state = BCM_FinishCAPI2Cmd;

	pdata->rsp_len = sizeof(OemSvcModeRsp) * numLine;

}

