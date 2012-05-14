/*
SRIL related defines
*/
typedef void * RIL_Token;

#define CellBroadcast true

#ifdef CellBroadcast

#define GSM_SMS_TPDU_STR_MAX_SIZE 490
#define BCM_RemoveCBMI 0x002

typedef struct {

	int	bCBEnabled;/*< CB service state. If cb_enabled is true then cell broadcast service will be enabled */
	                              /* and underlying modem will enable CB Channel to receiving CB messages. Otherwise CB */    
	                              /* service will be disabled; underlying modem will deactivate the CB channel. */
	                              /* (enabled/disabled)   : Storage : MODEM Flash */ 
								  
	int	selectedId;/* < CBMI Identifier selected (all or some)  Storage : MODEM Flash */

	int	msgIdMaxCount; /*< CB Channel List Max Count , this number is different for 2G & 3G & is SIM specific Storage : MODEM Flash  */

	int	msgIdCount; /**< CB message ID count , My Channels list is updated in  Storage : MODEM Flash */

	char * msgIDs; /* < CB message ID information, */
				   /* My Channels List is updated here. Storage : SIM Memory  */
} RIL_CB_ConfigArgs ;

typedef struct {
	
	int cbType; // 0x01 :GSM , 0x02:UMTS
	int message_length;
	char message[GSM_SMS_TPDU_STR_MAX_SIZE]; //reference : 3GPP TS 23.041 9.4.1, 9.4.2
	
}RIL_CBMsg_Notification;

#endif

typedef struct {				
 int bCBEnabled; /* CB service state */				
 int selectedId; /* CBMI Identifier selected */				
 int msgIdMaxCount; /* CB Channel List Max Count */				
 int msgIdCount; /* CB message ID count */				
 int msgIDs[CHNL_IDS_SIZE]; /* CB message ID information*/
} Kril_Cell_Broadcast_config;	


typedef struct {
	int cbType; // 0x01 :GSM , 0x02:UMTS
	int message_length;
	char message[GSM_SMS_TPDU_STR_MAX_SIZE]; //reference : 3GPP TS 23.041 9.4.1, 9.4.2
} Kril_Cell_Broadcast_message;	

typedef struct { 				
 unsigned long total_cnt; /*  the record index of the  message */				
 unsigned long used_cnt; /*  the record index of the  message */				
}KrilStoredMsgCount;				

typedef struct { 				
 unsigned long length; /*  the record index of the  message */				
 char pdu[180*2];
}KrilSMSfromSIM;				

typedef struct {
    int command;    /* one of the commands listed for TS 27.007 +CRSM*/
    int fileid;     /* EF id */
    char *path;     /* "pathid" from TS 27.007 +CRSM command.
                       Path is in hex asciii format eg "7f205f70"
                       Path must always be provided.
                     */
    int p1;
    int p2;
    int p3;
    char *data;     /* May be NULL*/
    char *pin2;     /* May be NULL*/
} RIL_SIM_IO;

typedef struct { 				
	int 	size;		  // @field structure size in bytes
	int     dataLen;	  // @field data size in bytes		
	int 	params;    // @field indicates valid parameters		
	int 	status; 	// @field additional status for message	
	char    *data;	 // @field message itself		
}RIL_SS_Release_Comp_Msg;

#define RIL_PARAM_SSDI_STATUS	1
#define RIL_PARAM_SSDI_DATA 		2



// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @ SRIL RIL_REQUEST_LOCK_INFO
//
// @comm None
//
// -----------------------------------------------------------------------------

typedef enum
{
	KRIL_LOCK_PIN1,
	KRIL_LOCK_PIN2,
	KRIL_LOCK_PUK2	
}KrilLockType_t;

typedef enum
{
	KRIL_PIN_NOT_NEED = 0x00,
	KRIL_PIN = 0x01,
	KRIL_PUK = 0x02,
	KRIL_PIN2 = 0x03,
	KRIL_PUK2 = 0x04,
	KRIL_PERM_BLOCKED = 0x05,
	KRIL_PIN2_DISABLE = 0x06			 
}KrilLockStatus_t;

typedef struct
{
	int lock_type;
	int lock_status;
	int remaining_attempt;	
}KrilLockInfo_t;

/*+ Ciphering Mode sh0515.lee /Integrity Mode sh0515.lee +*/
typedef enum
{
	KRIL_CIPHERING_MODE,
	KRIL_INTEGRATE_MODE,

	KRIL_CLASSMARK_ID_END	
} KrilClassMarkId_t;

typedef struct
{
	KrilClassMarkId_t classmark_id;
	union
	{
		int					is_supported;
		// can add other ClassMark setting below
	}data_u;
}KrilStackNvramClassMark_t;
/*- Ciphering Mode sh0515.lee /Integrity Mode sh0515.lee -*/


/*+ Band Selection sh0515.lee +*/
typedef struct
{
	int curr_rat;
	int new_band;
}KrilSrilSetBand_t;
/*- Band Selection sh0515.lee -*/

#define	PB_NUM_MAX	42
#define	PB_ALPHA_MAX	82
#define	PB_EMAIL_MAX	82

typedef struct { 				
				
 int data;				
				
}SRIL_Phonebk_Storage_Info_request;

typedef struct { 				
				
  int total;				
  int used;				
  int first_id;				
  int max_text;				
  int max_num;				
  				
}RIL_Phonebk_Storage_Info;

#define MAX_3GPP_TYPE 13
#define MAX_DATA_LEN 4
typedef struct {
 
	int response[MAX_3GPP_TYPE][MAX_DATA_LEN];
	
}RIL_Usim_PB_Capa;

typedef struct 
{
	long			index;
	long			next_index;
	unsigned char	number[PB_NUM_MAX];
	unsigned long   length_number;
	unsigned long	num_datatpye;
	unsigned char	name[ PB_ALPHA_MAX ];
	unsigned long   length_name;
	unsigned long	name_datatpye;
	unsigned char	email[ PB_EMAIL_MAX ];
	unsigned long   length_email;
	unsigned long	email_datatpye;

}KrilPhonebookGetEntry_t;	//HJKIM_ADN

typedef struct 
{
	int command;   
	int fileid;    
	int index;		
	char alphaTag[PB_ALPHA_MAX];   
	int alphaTagDCS;
	int alphaTagLength;
	char number[PB_NUM_MAX];    
	int numberLength;	
	char email[PB_EMAIL_MAX];
	int emailTagDCS;	//HJKIM_emailDCS
	int emailLength;
	char pin2[9];    
} KrilPhonebookAccess_t;	//HJKIM_ADN

//	ycao@032311

/* See RIL_REQUEST_LAST_CALL_FAIL_CAUSE */
typedef enum {
    CALL_FAIL_UNOBTAINABLE_NUMBER = 1,
    CALL_FAIL_NORMAL = 16,
    CALL_FAIL_BUSY = 17,
    CALL_FAIL_CONGESTION = 34,
    CALL_FAIL_ACM_LIMIT_EXCEEDED = 68,
    CALL_FAIL_CALL_BARRED = 240,
    CALL_FAIL_FDN_BLOCKED = 241,
    CALL_FAIL_IMSI_UNKNOWN_IN_VLR = 242,
    CALL_FAIL_IMEI_NOT_ACCEPTED = 243,
    CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE = 1000,
    CALL_FAIL_CDMA_DROP = 1001,
    CALL_FAIL_CDMA_INTERCEPT = 1002,
    CALL_FAIL_CDMA_REORDER = 1003,
    CALL_FAIL_CDMA_SO_REJECT = 1004,
    CALL_FAIL_CDMA_RETRY_ORDER = 1005,
    CALL_FAIL_CDMA_ACCESS_FAILURE = 1006,
    CALL_FAIL_CDMA_PREEMPTED = 1007,
    CALL_FAIL_CDMA_NOT_EMERGENCY = 1008, /* For non-emergency number dialed
                                            during emergency callback mode */
    CALL_FAIL_CDMA_ACCESS_BLOCKED = 1009, /* CDMA network access probes blocked */
    CALL_FAIL_ERROR_UNSPECIFIED = 0xffff
} RIL_LastCallFailCause;


#define     MO_VOICE 0x00
#define     MO_SMS   0x01
#define     MO_SS    0x02
#define     MO_USSD  0x03
#define     PDP_CTXT 0x04    


#define      CALL_CONTROL_NO_CONTROL  0
#define      CALL_CONTROL_ALLOWED_NO_MOD 1
#define      CALL_CONTROL_NOT_ALLOWED 2
#define      CALL_CONTROL_ALLOWED_WITH_MOD 3   



typedef struct { 				
    char call_type;         				
    char control_result;    			
    char alpha_id_present;				
    char alpha_id_len;				
    char alpha_id[64];				
    char call_id;				
    char old_call_type;   				
    char modadd_ton;				
    char modadd_npi;				
    char modadd_len;				
    char modadd[200];				
}RIL_Stk_CallCtrl_Result;

typedef struct {
  BRIL_RadioState   radioState ;    /* radioState*/
  int             simCardType;  /* simCardType */
  int             IsFlightModeOnBoot;  // gearn flihgt mode simcard type
} RIL_SIM_Chaged;  // gearn sim card type

#define RIL_OEM_REQUEST_BASE 10000

#ifdef CellBroadcast
#define RIL_REQUEST_SET_CELL_BROADCAST_CONFIG (RIL_OEM_REQUEST_BASE + 1)
#define RIL_REQUEST_GET_CELL_BROADCAST_CONFIG (RIL_OEM_REQUEST_BASE + 2)  
#define RIL_REQUEST_CRFM_LINE_SMS_COUNT_MSG (RIL_OEM_REQUEST_BASE + 3)
#define RIL_REQUEST_CRFM_LINE_SMS_READ_MSG (RIL_OEM_REQUEST_BASE + 4)
#endif

#define RIL_REQUEST_SEND_ENCODED_USSD (RIL_OEM_REQUEST_BASE + 5)
#define RIL_REQUEST_SET_PDA_MEMORY_STATUS (RIL_OEM_REQUEST_BASE + 6)
#define RIL_REQUEST_GET_PHONEBOOK_STORAGE_INFO (RIL_OEM_REQUEST_BASE + 7) 
#define RIL_REQUEST_GET_PHONEBOOK_ENTRY (RIL_OEM_REQUEST_BASE + 8) 
#define RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY (RIL_OEM_REQUEST_BASE + 9)
#define RIL_REQUEST_DIAL_VIDEO_CALL (RIL_OEM_REQUEST_BASE + 10) 
#define RIL_REQUEST_CALL_DEFLECTION (RIL_OEM_REQUEST_BASE + 11)
#define RIL_REQUEST_READ_SMS_FROM_SIM (RIL_OEM_REQUEST_BASE + 12)
#define RIL_REQUEST_USIM_PB_CAPA (RIL_OEM_REQUEST_BASE + 13)
#define RIL_REQUEST_LOCK_INFO (RIL_OEM_REQUEST_BASE + 14)
#define RIL_REQUEST_SEND_MOBILE_TRACKER_SMS (RIL_OEM_REQUEST_BASE + 15)
#define RIL_REQUEST_DIAL_EMERGENCY_CALL (RIL_OEM_REQUEST_BASE + 16)
#define RIL_REQUEST_GET_STOREAD_MSG_COUNT (RIL_OEM_REQUEST_BASE + 17)
#define RIL_REQUEST_STK_SIM_INIT_EVENT (RIL_OEM_REQUEST_BASE + 18)

#define RIL_OEM_UNSOL_RESPONSE_BASE 11000
#ifdef CellBroadcast
#define RIL_UNSOL_RESPONSE_NEW_CB_MSG  (RIL_OEM_UNSOL_RESPONSE_BASE + 0) //add by jiyoon
#endif
#define RIL_UNSOL_RELEASE_COMPLETE_MESSAGE	(RIL_OEM_UNSOL_RESPONSE_BASE + 1)
#define RIL_UNSOL_STK_SEND_SMS_RESULT (RIL_OEM_UNSOL_RESPONSE_BASE + 2)
#define RIL_UNSOL_STK_CALL_CONTROL_RESULT (RIL_OEM_UNSOL_RESPONSE_BASE + 3)
#define RIL_UNSOL_DUN_CALL_STATUS (RIL_OEM_UNSOL_RESPONSE_BASE + 4)
#define RIL_UNSOL_RESPONSE_LINE_SMS_COUNT (RIL_OEM_UNSOL_RESPONSE_BASE + 5)
#define RIL_UNSOL_RESPONSE_LINE_SMS_READ  (RIL_OEM_UNSOL_RESPONSE_BASE + 6)
#define RIL_UNSOL_O2_HOME_ZONE_INFO (RIL_OEM_UNSOL_RESPONSE_BASE + 7)
#define RIL_UNSOL_DEVICE_READY_NOTI (RIL_OEM_UNSOL_RESPONSE_BASE + 8)
#define RIL_UNSOL_GPS_NOTI	(RIL_OEM_UNSOL_RESPONSE_BASE + 9)
#define RIL_UNSOL_AM	(RIL_OEM_UNSOL_RESPONSE_BASE + 10)
#define RIL_UNSOL_DUN_PIN_CONTROL_SIGNAL (RIL_OEM_UNSOL_RESPONSE_BASE + 11)
#define RIL_UNSOL_DATA_SUSPEND_RESUME (RIL_OEM_UNSOL_RESPONSE_BASE + 12)
#define RIL_UNSOL_SAP (RIL_OEM_UNSOL_RESPONSE_BASE + 13)
#define RIL_UNSOL_RESPONSE_NO_NETWORK_RESPONSE (RIL_OEM_UNSOL_RESPONSE_BASE + 14)
#define RIL_UNSOL_SIM_SMS_STORAGE_AVAILALE (RIL_OEM_UNSOL_RESPONSE_BASE + 15)
