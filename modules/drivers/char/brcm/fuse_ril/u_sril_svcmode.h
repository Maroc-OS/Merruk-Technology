#ifndef _U_SRIL_SVCMODE_H
#define _U_SRIL_SVCMODE_H

#include "u_sril.h"

#define MAX_SVCMENU_DEPTH 8
#define MAX_SVCSTR_PER_LINE	32
#define MAX_SVCMENU_LINE	11

/* CWYoon 110526::Factory test, Auto answering */
int uSril_GetAutoAnswerValue(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);
void uSril_SetOnAutoAnswerValue(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);
void uSril_SetOffAutoAnswerValue(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);

/* Factory test, Sellout SMS : Junhee_110710 */
void uSril_SelloutSms(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);
void uSril_SetSelloutSmsOn(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);
void uSril_SetSelloutSmsOff(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);
void uSril_SetSelloutSmsTestMode(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);
void uSril_SetSelloutSmsProductMode(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);
void uSril_GetSelloutSmsInfo(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);

void uSril_FatalError(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);
void uSril_SWWatchDog(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);
void uSril_HWWatchDog(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);

/* ID_SVCMENU_FACTORY_RTC_READ : JHLee 110808 */
void uSril_GetRTCValue(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);

/* working for debug screen in 197328640 key string : JSHAN */
void uSril_DebugScreen(int request, void *data, size_t datalen, RIL_Token t, unsigned short inClientId);

/* SVC_MODE  field */
/* Service mode 종류 선택 */
typedef enum {
  SVC_MODE_TEST_MANUAL=0x01,     /* 0x01 : Manual test mode */
  SVC_MODE_TEST_AUTO,                  /* 0x02 : Auto test mode */
  SVC_MODE_NAM,                              /* 0x03 : NAM edit mode */
  SVC_MODE_MONITOR,                      /* 0x04 : Monitor screen mode */
  SVC_MODE_PHONE_TEST,                /* 0x05 : Phone test mode ( just for the debugging ) */
  SVC_MODE_OPERATOR_SPECIFIC_TEST,    /*0x06: Specific test mode required by operator*/
  SVC_MODE_MAX
} svc_mode_type;

/* Service mode가 	SVC_MODE_TEST_MANUAL인 경우만 해당 */ 
typedef enum { 
    TST_TESTMODE_ENTER=0x00,                        /* 0x1000 : Testmode enter */
    TST_SW_VERSION_ENTER,                            /* 0x1001 : SW_version enter */
    TST_FTA_SW_VERSION_ENTER,                        /* 0x1002 : FTA SW version enter */
    TST_FTA_HW_VERSION_ENTER,                        /* 0x1003 : FTA HW version enter */
    TST_ALL_VERSION_ENTER,                            /* 0x1004 : All version enter  */
    TST_BATTERY_INFO_ENTER,                            /* 0x1005 : Battery Information enter  */
    TST_CIPHERING_PROTECTION_ENTER,                                    /* 0x1006 : Ciphering protection enter */
    TST_INTEGRITY_PROTECTION_ENTER,                             /* 0x1007 : Integrity protection enter */
    TST_IMEI_READ_ENTER,                            /* 0x1008 : IMEI enter */
    TST_BLUETOOTH_TEST_ENTER,                        /* 0x1009 : Bluetooth test enter */
    TST_VIBRATOR_TEST_ENTER,                        /* 0x100A : Vibrator test enter */
    TST_MELODY_TEST_ENTER,                            /* 0x100B : Melody test enter */
    TST_MP3_TEST_ENTER,                            /* 0x100C : MP3 test enter */
    TST_FACTORY_RESET_ENTER,                        /* 0x100D : Factory test enter */
    TST_FACTORY_PRECONFIG_ENTER,                            /* 0x100E : Factory preconfig enter */
    TST_TFS4_EXPLORE_ENTER,                            /* 0x100F : TFS4 explore enter */
    TST_RTC_TIME_DISPLAY_ENTER,                        /* 0x1010 : RTC time display enter */
    TST_RSC_FILE_VERSION_ENTER,                        /* 0x1011 : RSC file version enter */
    TST_USB_DRIVER_ENTER,                            /* 0x1012 : USB driver enter */
    TST_USB_UART_DIAG_CONTROL_ENTER,                            /* 0x1013 : USB UART diag control enter*/
    TST_RRC_VERSION_ENTER,                            /* 0x1014 : RRC Version enter */ 
    TST_GPSONE_SS_TEST_ENTER,                                   /* 0x1015 : GPSone testmode enter */ 
    TST_BAND_SEL_ENTER,                              /* 0x1016 : Band selection mode enter */
    TST_GCF_TESTMODE_ENTER,                         /* 0x1017 : GCF Test mode enter */
    TST_GSM_FACTORY_AUDIO_LB_ENTER,         /* 0x1018 : GSM Factory Audio Loopback enter */
    TST_FACTORY_VF_TEST_ENTER,                                         /* 0x1019 : VF ADC Test mode enter */
    TST_TOTAL_CALL_TIME_INFO_ENTER,                        /* 0x101A : Display Total Call time info enter */
    TST_SELLOUT_SMS_ENABLE_ENTER,                                         /* 0x101B : Sell_Out_SMS_Enable_enter */
    TST_SELLOUT_SMS_DISABLE_ENTER,                                         /* 0x101C : Sell_Out_SMS_Disable_enter */
    TST_SELLOUT_SMS_TEST_MODE_ON,                                         /* 0x101D : Sell_Out_SMS_Test_Mode_ON */
    TST_SELLOUT_SMS_PRODUCT_MODE_ON,                                  /* 0x101E : Sell_Out_SMS_Product_Mode_ON */
    TST_GET_SELLOUT_SMS_INFO_ENTER,                                      /* 0x101F : GET_SELLOUT_SMS_INFO_ENTER */      
	TST_WCDMA_SET_CHANNEL_ENTER,                                          /* 0x1020 : WCDMA Set Channel Enter */
    TST_GPRS_ATTACH_MODE_ENTER,												/* 0x1021 : GPRS Attach mode Enter */
/*=========================================================================================*/
/* Belows are added for SvcMode implementation and are not exist  in application code      */
/*=========================================================================================*/
	TST_AUTO_ANSWER_ENTER,												/* CWYoon 110526::Auto answer Toggle */

	TST_IMSI_READ_ENTER,
	TST_LOG_KRIL_ENABLE,
	TST_LOG_KRIL_DISABLE,	
	TST_SIM_INFO_ENTER,//Irine_SIMInfo_SvcMenu

    /*+ Band Selection sh0515.lee +*/
    TST_SET_BAND_AUTO,
    TST_SET_BAND_ALL_GSM,
    TST_SET_BAND_GSM_850,
    TST_SET_BAND_GSM_900,
    TST_SET_BAND_DCS_1800,
    TST_SET_BAND_PCS_1900,
    TST_SET_BAND_ALL_UMTS,
    TST_SET_BAND_UMTS_2100,
    TST_SET_BAND_UMTS_1900,
    TST_SET_BAND_UMTS_900,
    TST_SET_BAND_UMTS_850,    
    /*- Band Selection sh0515.lee -*/

   /*+ Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee +*/
   TST_SET_CIPHERING_MODE_ON,	
   TST_SET_CIPHERING_MODE_OFF,	
   TST_GET_CIPHERING_MODE,
   TST_SET_INTEGRATE_MODE_ON,	
   TST_SET_INTEGRATE_MODE_OFF,	
   TST_GET_INTEGRATE_MODE,
   /*- Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee -*/
    TST_SUB_MODE_MAX

}test_mode_sub_type;

/* If Service mode is SVC_MODE_NAM */ 
typedef enum { 
	SVC_NAM_EDIT=0x0001,                           /* 0x0001 : NAM Full Edit Mode */
	SVC_NAM_BASIC_EDIT,                            /* 0x0002 : NAM Basic Edit Mode */
	SVC_NAM_ADVANCED_EDIT,                     /* 0x0003 : NAM Advanced Edit Mode */
	SVC_NAM_MAX
} svc_nam_sub_type;



// for Svcmode display
typedef struct
{
	unsigned char 	line;
	unsigned char	reverse;
	char			string[MAX_SVCSTR_PER_LINE];			//영문 31자, 한글 15자 
} OemSvcModeRsp;

typedef struct
{
	OemReqMsgHdr 	hdr;
	unsigned char	modeType;
	unsigned char	subType;
	unsigned char	query;
} OemSvcEnterModeMsg;

typedef struct
{
	OemReqMsgHdr 	hdr;
	unsigned char	keyCode;
	unsigned char	query;
} OemSvcProcKeyMsg;

typedef struct
{
	OemReqMsgHdr 	hdr;
	unsigned char	modeType;
} OemSvcEndModeMsg;

typedef enum{
    SVC_KEYCODE_SPEAKERPHONE			=0x0A,
    SVC_KEYCODE_OK						=0x0B,
    SVC_KEYCODE_START					=0x0C,
    SVC_KEYCODE_RETURN					=0x0D,
    SVC_KEYCODE_BACKSPACE				=0x0F,
    SVC_KEYCODE_POWER				    =0x1B,

	SVC_KEYCODE_SHARP					=0x23,
	SVC_KEYCODE_STAR					=0x2A,
	SVC_KEYCODE_0						=0x30,
	SVC_KEYCODE_1						=0x31,
	SVC_KEYCODE_2						=0x32,
	SVC_KEYCODE_3						=0x33,
	SVC_KEYCODE_4						=0x34,
	SVC_KEYCODE_5						=0x35,
	SVC_KEYCODE_6						=0x36,
	SVC_KEYCODE_7						=0x37,
	SVC_KEYCODE_8						=0x38,
	SVC_KEYCODE_9						=0x39,


	SVC_KEYCODE_A                       =0x41,
	SVC_KEYCODE_B                       =0x42,
	SVC_KEYCODE_C                       =0x43,
	SVC_KEYCODE_D                       =0x44,
	SVC_KEYCODE_E                       =0x45,
	SVC_KEYCODE_F                       =0x46,

	SVC_KEYCODE_SEND					=0x50,
	SVC_KEYCODE_END						=0x51,
	SVC_KEYCODE_CLEAR					=0x52,
	SVC_KEYCODE_SAVE					=0x53,

	// Side Volume Key
	SVC_KEYCODE_VOLUP					=0x54,
	SVC_KEYCODE_VOLDOWN					=0x55,

	// Soft Key
	SVC_KEYCODE_SOFT_LEFT				=0x5B,
	SVC_KEYCODE_SOFT_RIGHT				=0x5C,

	// Navigation Key
	SVC_KEYCODE_UP						=0x63,
	SVC_KEYCODE_DOWN					=0x64,
	SVC_KEYCODE_LEFT					=0x65,
	SVC_KEYCODE_RIGHT					=0x66,

	// Mits Key
	SVC_KEYCODE_MENU					=0x70,
	SVC_KEYCODE_HOME					=0x71,
	SVC_KEYCODE_FUNC_1					=0x72,
	SVC_KEYCODE_FUNC_2					=0x73,
	SVC_KEYCODE_FUNC_3					=0x74,
	SVC_KEYCODE_FUNC_4					=0x75,
	SVC_KEYCODE_FUNC_5					=0x76,

	// IPC_KEYCODE_FUNC_RESERVED : 0x77 ~ 0x7F
	SVC_KEYCODE_VMEMO					=0x8B,
	SVC_KEYCODE_WAP						=0x8C,
	SVC_KEYCODE_CAMERA					=0x8D,
	SVC_KEYCODE_LOCK                    =0xB0

}snd_key_code_e_type;


/*+20110418 BCOM PATCH FOR DebugScreen*/
/* If Service mode is SVC_MODE_NAM */ 
typedef enum { 
	SVC_MODE_UPDATE_READSWVER=0x0001,
	SVC_MODE_UPDATE_MONITOR
} svc_mode_update_menu;
/*-20110418 BCOM PATCH FOR DebugScreen*/


#endif //_U_SRIL_SVCMODE_H
