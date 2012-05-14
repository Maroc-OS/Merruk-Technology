/**
 * COPYRIGHT (C)  SAMSUNG Electronics CO., LTD (Suwon, Korea). 2009           
 * All rights are reserved. Reproduction and redistiribution in whole or 
 * in part is prohibited without the written consent of the copyright owner.
 */

/**
 * @file	u_sril.h
 *
 * @author	Sungdeuk Park (seungdeuk.park @samsung.com)
 *
 * @brief	Request from RIL deamon
 */

#ifndef _U_SRIL_H
#define _U_SRIL_H

/*
 * OEM function IDs
 */
#define OEM_FUNCTION_ID_SVC_MODE			0x01
#define OEM_FUNCTION_ID_NETWORK				0x02
#define OEM_FUNCTION_ID_SS					0x03
#define OEM_FUNCTION_ID_PERSONALIZATION		0x04
#define OEM_FUNCTION_ID_POWER				0x05
#define OEM_FUNCTION_ID_IMEI				0x06
#define OEM_FUNCTION_ID_SYSDUMP				0x07
#define OEM_FUNCTION_ID_SOUND				0x08
#define OEM_FUNCTION_ID_GPRS				0x09
#define OEM_FUNCTION_ID_OMADM				0x0A
#define OEM_FUNCTION_ID_CALL				0x0B
#define OEM_FUNCTION_ID_CONFIGURATION		0x0C
#define OEM_FUNCTION_ID_DATA				0x0D
#define OEM_FUNCTION_ID_GPS					0x0E
#define OEM_FUNCTION_ID_PHONE				0x10
#define OEM_FUNCTION_ID_MISC				0x11
#define OEM_FUNCTION_ID_FACTORY				0x12
#define OEM_FUNCTION_ID_RFS					0x13
#define OEM_FUNCTION_ID_SAP					0x14
#define OEM_FUNCTION_ID_AUTH				0x15
#define OEM_FUNCTION_ID_DATAROUTER          0x16

/* KEY_CODE  field */
/* key value which is used in PDA keypad */

/*
 * OEM sub-function IDs
 */
// Service mode
#define	OEM_SVC_ENTER_MODE_MESSAGE			0x01
#define	OEM_SVC_END_MODE_MESSAGE			0x02
#define	OEM_SVC_PROCESS_KEY_MESSAGE			0x03
#define	OEM_SVC_GET_DISPLAY_DATA_MESSAGE	0x04
#define	OEM_SVC_QUERY_DISPLAY_DATA_MESSAGE	0x05
#define OEM_SVC_DEBUG_DUMP_MESSAGE			0x06
#define OEM_SVC_DEBUG_STRING_MESSAGE		0x07

// Network
#define OEM_NWK_GET_AVAILABLE_NETWORK		0x01
#define OEM_NWK_SET_MANUAL_SELECTION		0x02
#define OEM_NWK_CANCEL_AVAILABLE_NETWORK	0x03

// SS
#define OEM_SS_CHANGE_CB_PWD				0x01

// Personalization

// Power
#define OEM_PWK_SET_RESET					0x01
#define OEM_PWK_GET_LAST_PHONE_FATAL_RSN	0x02
#define OEM_PWK_SET_PHONE_STATE				0x03

// IMEI
//#define OEM_IMEI_START					0x01
#define OEM_IMEI_SET_PRECONFIGURAION		0x01
#define OEM_IMEI_SET_WRITE_ITEM				0X02
#define OEM_IMEI_GET_WRITE_ITEM				0x03
#define OEM_IMEI_RESP_FILE_NUM				0x04
#define OEM_IMEI_EVENT_START_IMEI			0x05
#define OEM_IMEI_EVENT_VERIFY_COMPARE		0x06
#define OEM_IMEI_SET_UPDATE_ITEM			0x07
#define OEM_IMEI_CFRM_UPDATE_ITEM			0x08
#define OEM_IMEI_GET_PRECONFIGURAION		0x09
#define OEM_IMEI_SIM_OUT				0x0A

//SYSDUMP
#define	OEM_LOGCAT_MAIN						0x01
#define OEM_LOGCAT_RADIO					0x02
#define OEM_DUMPSTATE						0x03
#define OEM_KERNEL_LOG						0x04
#define OEM_LOGCAT_CLEAR					0x05
#define OEM_SYSDUMP_DBG_STATE_GET			0x06
#define OEM_SYSDUMP_ENABLE_LOG				0x07
#define OEM_IPC_DUMP_LOG					0x08
#define OEM_IPC_DUMP_BIN					0x09
#define OEM_RAMDUMP_MODE					0x0A
#define OEM_RAMDUMP_STATE_GET				0x0B
#define OEM_START_RIL_LOG					0x0C
#define OEM_DEL_RIL_LOG						0x0D

#define OEM_TCPDUMP_START				0x15

#define OEM_TCPDUMP_STOP					0x16



//SOUND
#define OEM_SOUND_SET_MINUTE_ALERT			0x01
#define OEM_SOUND_GET_MINUTE_ALERT			0x02
#define OEM_SOUND_SET_VOLUME_CTRL			0x03
#define OEM_SOUND_GET_VOLUME_CTRL			0x04
#define OEM_SOUND_SET_AUDIO_PATH_CTRL		0x05
#define OEM_SOUND_GET_AUDIO_PATH_CTRL		0x06
#define OEM_SOUND_SET_VIDEO_CALL_CTRL		0x07
#define OEM_SOUND_SET_LOOPBACK_CTRL         0x08
#define OEM_SOUND_SET_VOICE_RECORDING_CTRL  0x09
#define OEM_SOUND_SET_CLOCK_CTRL            0x0A

// Hidden Menu for cdma
#define OEM_HIDDEN_AKEY_VERIFY				0x01
#define OEM_HIDDEN_GET_MSL					0x02
#define OEM_HIDDEN_GET_ESNMEID				0x03
#define OEM_HIDDEN_SET_TEST_CALL			0x04
#define OEM_HIDDEN_END_TEST_CALL			0x09
#define OEM_HIDDEN_GET_RC_DATA				0x05
#define OEM_HIDDEN_SET_RC_DATA				0x06
#define OEM_HIDDEN_GET_HYBRID_MODE			0x07
#define OEM_HIDDEN_SET_HYBRID_MODE			0x08
#define OEM_HIDDEN_GET_LIFETIMECALL			0x0A
#define OEM_HIDDEN_GET_LIFEBYTE				0x0B
#define OEM_HIDDEN_GET_RECONDITIONED		0x0C
#define OEM_HIDDEN_GET_ACTIVATIONDATE 		0x0D
#define OEM_HIDDEN_GET_MOBILEIPNAI	 		0x0E
#define OEM_HIDDEN_SET_MOBILEIPNAI	 		0x0F
#define OEM_HIDDEN_GET_MODEM_NAI			0x10
#define OEM_HIDDEN_SET_MODEM_NAI			0x11
#define OEM_HIDDEN_GET_KOREA_MODE	 		0x12
#define OEM_HIDDEN_SET_KOREA_MODE 			0x13

//OMADM
#define OEM_OMADM_START_CIDC				0x01
#define OEM_OMADM_START_CIFUMO				0x02
#define OEM_OMADM_START_CIPRL				0x03
#define OEM_OMADM_START_HFA					0x04
#define OEM_OMADM_START_REG_HFA				0x05
#define OEM_OMADM_SETUP_SESSION				0x06
#define OEM_OMADM_SERVER_START_SESSION		0x07
#define OEM_OMADM_CLIENT_START_SESSION		0x08
#define OEM_OMADM_SEND_DATA					0x09
#define OEM_OMADM_EANBLE_HFA				0x19

// GPRS
#define OEM_GPRS_SET_DORMANCY				0x01
#define OEM_GPRS_EXEC_DUN_PIN_CTRL			0x02
#define OEM_GPRS_DISCONNECT_DUN				0x03 // ActiveSync for internet sharing

//CALL
#define OEM_CALL_SEND_DTMF_STRING			0x01
#define OEM_CALL_E911CB_MODE				0x02
#define OEM_CALL_SET_DTMFLENGTH				0x07
#define OEM_CALL_GET_DTMFLENGTH				0x08
#define OEM_CALL_GET_LIFETIMECALL			0x0A

#if defined(CONFIG_SAMSUNG_LTN_COMMON)
#define OEM_CALL_SET_LIFETIMECALL			0x0B // jkjo@latinMW:2010.06.08 - implement_TotallCallTime
#endif

//CONFIGURATION
#define OEM_CFG_EXEC_DEFAULT					0x01
#define OEM_CFG_CFRM_WIFI_TEST					0x02
#define OEM_CFG_CFRM_BT_FACTORY_TEST			0x03
#define OEM_CFG_SET_AUDIO_LOOPBACK_TEST			0x04
#define OEM_CFG_GET_DGS_UNIQUENUMBER			0x05
#define OEM_CFG_CFRM_ACCELERATION_SENSOR_TEST	0x06
#define OEM_CFG_SIO_MODE_SETTING				0X08
#define OEM_CFG_CDMA_TEST_SYS					0X10
#define OEM_CFG_SET_1X_EVDO_DIVERSITY_CONFIG	0x22
#define OEM_CFG_SET_USER_LOCK_CODE_STATUS		0x24
#define OEM_CFG_SET_DEVICE_MAC_ADDRESS			0x30

// DATA
#define OEM_DATA_TE2_STATUS					0x01
#define OEM_DATA_RESTORE_NAI				0x0d

//PHONE
#define OEM_PHONE_RAM_DUMP					0x01
#define OEM_PHONE_RESET						0x02
#define OEM_PHONE_MAKE_CRASH				0x03
#define OEM_PHONE_DUMP_MODE					0x04

#ifdef CONFIG_RTC_CP_BACKUP_FEATURE
// MISC
#define OEM_MISC_GET_TIME					0x01
#define OEM_MISC_SET_TIME					0x02
#endif
#define OEM_MISC_GET_SERIALNUMBER			0x11
#define OEM_MISC_GET_MANUFACTUREDATE        0x12
#define OEM_MISC_GET_BARCODE                0x13

//FACTORY
#if 1//FEATURE_SAMSUNG_IPC41_NEW_FACTORY
//#define OEM_FACTORY_DEVICE_TEST				0x01
//#define OEM_FACTORY_OMISSION_AVOIDANCE_TEST	0x02
//#define OEM_FACTORY_DFT_TEST				0x03
//#define OEM_FACTORY_MISCELLANEOUS_TEST		0x04

#define OEM_FACTORY_EVENT					0x01
#define OEM_FACTORY_CFRM					0x02
#define OEM_OMISSION_GET					0x03
#define OEM_OMISSION_SET					0x04
#define OEM_DFT_EVENT						0x05
#define OEM_DFT_CFRM						0x06
#define OEM_MISCELLANEOUS_EVENT				0x07
#define OEM_MISCELLANEOUS_CFRM				0x08

#else
#define OEM_FACTORY_CFRM_SD_CHECK			0x01
#define OEM_FACTORY_CFRM_SHIPMENT_TEST		0x02
#define OEM_FACTORY_CFRM_G_SENSOR_TEST		0x03 //Acceleration , 3AXIS
#define OEM_FACTORY_SET_PROCESS_TEST		0x04
#define OEM_FACTORY_GET_PROCESS_TEST		0x05
#define OEM_FACTORY_CFRM_WIFI_TEST			0x06
#define OEM_FACTORY_CFRM_CAMERA_TEST		0x07
#endif

// AUTH
#define OEM_AUTH_GSM_CONTEXT				0x06
#define OEM_AUTH_3G_CONTEXT					0x07

//RFS
#define OEM_RFS_NV_MOBILE_TRACKER 	0x01
typedef struct _OemReqMsgHdr
{
	unsigned char funcId;
	unsigned char subfId;
	unsigned short len;
}OemReqMsgHdr;

typedef struct _OemReqMsg
{
	OemReqMsgHdr hdr;
	char * payload;
}OemReqMsg;

#endif	//_U_SRIL_H