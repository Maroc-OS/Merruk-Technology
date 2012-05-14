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
/**
*
*  @file   bcm_kril.h
*
*  @brief  Defines the message id and data structure used for URIL & KRIL.
*
* This file is a exact copy of hardware/common/bcm_uril/bcm_kril.h
*  and it should be kept in sync with abovementioned file.
*
****************************************************************************/
#ifndef _BCM_KRIL_H
#define _BCM_KRIL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void * BRIL_Token;

// Structure data
#define RESULT_BUFFER_LEN_MAX 4500//byte

// for Request
#define NUMBER_LENGTH_MAX 40
#define BCM_MAXCB_NO      10

#define CHV_MIN_LENGTH         4 ///< Min. number of digits in CHV
#define CHV_MAX_LENGTH         8 ///< Max. number of digits in CHV, PUK
#define PUK_MIN_LENGTH         8 ///< Min. number of digits in PUK
#define PUK_MAX_LENGTH         8 ///< Min. number of digits in PUK

#define MAX_SIMIO_FILE_PATH_LENGTH   5
#define MAX_SIMIO_PIN2_LENGTH        64
#define MAX_SIMIO_CMD_LENGTH         256
#define MAX_SIMIO_RSP_LENGTH         512

#define CHNL_IDS_SIZE           260
#define MAX_CHNLID_LIST_SIZE    10

#define MAX_STK_PROFILE_LEN     20

// for Response
#define BCM_MAX_CALLS_NO        10
#define BCM_MAX_PLMN_SEARCH     10
#define MAX_PW_LENGTH           10
#define BCM_MAX_SS_CLASS_SIZE   15	///< Maximum number of service classes returned by a query.
#define PHONE_NUMBER_LENGTH_MAX 40
#define PLMN_LONG_NAME          30
#define PLMN_SHORT_NAME         20
#define BCM_MAX_DIGITS          80
#define MAX_USSD_STRING         160
#define BCM_SMSMSG_SZ  		   180
#define MAX_SMSC_LENGTH		   20
#define PHASE1_MAX_USSD_STRING_SIZE 200

#define IMSI_DIGITS	             15	            ///< Max. IMSI digits
#define IMEI_DIGITS              15             ///< Number of IMEI digits (14 + check digit)

// for PDP
#define PDP_TYPE_LEN_MAX     10
#define PDP_ADDRESS_LEN_MAX  20
#define PDP_APN_LEN_MAX      101
#define PDP_USERNAME_LEN_MAX 65
#define PDP_PASSWORD_LEN_MAX 65
#define L2P_STRING_LENGTH_MAX 6		//possible l2p values: PPP, M-xxxx, BRCM supports only PPP


// for Android message structure identifier type
typedef enum {
    BCM_E_SUCCESS = 0,
    BCM_E_RADIO_NOT_AVAILABLE = 1,
    BCM_E_GENERIC_FAILURE = 2,
    BCM_E_PASSWORD_INCORRECT = 3,
    BCM_E_SIM_PIN2 = 4,
    BCM_E_SIM_PUK2 = 5,
    BCM_E_REQUEST_NOT_SUPPORTED = 6,
    BCM_E_CANCELLED = 7,
    BCM_E_OP_NOT_ALLOWED_DURING_VOICE_CALL = 8,
    BCM_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW = 9,
    BCM_E_SMS_SEND_FAIL_RETRY = 10,
    BCM_E_SIM_ABSENT = 11,
    BCM_E_SUBSCRIPTION_NOT_AVAILABLE = 12,
    BCM_E_MODE_NOT_SUPPORTED = 13,
    BCM_E_FDN_CHECK_FAILURE = 14,
    BCM_E_ILLEGAL_SIM_OR_ME = 15
} BRIL_Errno;

typedef enum {
    BCM_CALL_ACTIVE = 0,
    BCM_CALL_HOLDING = 1,
    BCM_CALL_DIALING = 2,    /* For MO only */
    BCM_CALL_ALERTING = 3,   /* For MO only */
    BCM_CALL_INCOMING = 4,   /* For MT only */
    BCM_CALL_WAITING = 5     /* For MT only */
} BRIL_CallState;

typedef enum {
    BCM_RADIO_STATE_OFF = 0,
    BCM_RADIO_STATE_UNAVAILABLE = 1,
    BCM_RADIO_STATE_SIM_NOT_READY = 2,
    BCM_RADIO_STATE_SIM_LOCKED_OR_ABSENT = 3,
    BCM_RADIO_STATE_SIM_READY = 4,
    BCM_RADIO_STATE_RUIM_NOT_READY = 5,
    BCM_RADIO_STATE_RUIM_READY = 6,
    BCM_RADIO_STATE_RUIM_LOCKED_OR_ABSENT = 7,
    BCM_RADIO_STATE_NV_NOT_READY = 8,
    BCM_RADIO_STATE_NV_READY = 9
} BRIL_RadioState;

typedef enum {
    BCM_CALL_FAIL_UNOBTAINABLE_NUMBER = 1,
    BCM_CALL_FAIL_NORMAL = 16,
    BCM_CALL_FAIL_BUSY = 17,
    BCM_CALL_FAIL_CONGESTION = 34,
    BCM_CALL_FAIL_ACM_LIMIT_EXCEEDED = 68,
    BCM_CALL_FAIL_CALL_BARRED = 240,
    BCM_CALL_FAIL_FDN_BLOCKED = 241,
    BCM_CALL_FAIL_IMSI_UNKNOWN_IN_VLR = 242,
    BCM_CALL_FAIL_IMEI_NOT_ACCEPTED = 243,
    BCM_CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE = 1000,
    BCM_CALL_FAIL_CDMA_DROP = 1001,
    BCM_CALL_FAIL_CDMA_INTERCEPT = 1002,
    BCM_CALL_FAIL_CDMA_REORDER = 1003,
    BCM_CALL_FAIL_CDMA_SO_REJECT = 1004,
    BCM_CALL_FAIL_CDMA_RETRY_ORDER = 1005,
    BCM_CALL_FAIL_CDMA_ACCESS_FAILURE = 1006,
    BCM_CALL_FAIL_CDMA_PREEMPTED = 1007,
    BCM_CALL_FAIL_CDMA_NOT_EMERGENCY = 1008,
    BCM_CALL_FAIL_CDMA_ACCESS_BLOCKED = 1009,
    BCM_CALL_FAIL_ERROR_UNSPECIFIED = 0xffff
} BRIL_LastCallFailCause;

typedef enum {
    BCM_PDP_FAIL_OPERATOR_BARRED = 0x08,
    BCM_PDP_FAIL_INSUFFICIENT_RESOURCES = 0x1A,
    BCM_PDP_FAIL_MISSING_UKNOWN_APN = 0x1B,
    BCM_PDP_FAIL_UNKNOWN_PDP_ADDRESS_TYPE = 0x1C,
    BCM_PDP_FAIL_USER_AUTHENTICATION = 0x1D,
    BCM_PDP_FAIL_ACTIVATION_REJECT_GGSN = 0x1E,
    BCM_PDP_FAIL_ACTIVATION_REJECT_UNSPECIFIED = 0x1F,
    BCM_PDP_FAIL_SERVICE_OPTION_NOT_SUPPORTED = 0x20,
    BCM_PDP_FAIL_SERVICE_OPTION_NOT_SUBSCRIBED = 0x21,
    BCM_PDP_FAIL_SERVICE_OPTION_OUT_OF_ORDER = 0x22,
    BCM_PDP_FAIL_NSAPI_IN_USE      = 0x23,
    BCM_PDP_FAIL_PROTOCOL_ERRORS   = 0x6F,
    BCM_PDP_FAIL_ERROR_UNSPECIFIED = 0xffff,
    BCM_PDP_FAIL_REGISTRATION_FAIL = -1,
    BCM_PDP_FAIL_GPRS_REGISTRATION_FAIL = -2,
} BRIL_LastDataCallActivateFailCause;

typedef enum {
  BCM_APPTYPE_UNKNOWN = 0,
  BCM_APPTYPE_SIM     = 1,
  BCM_APPTYPE_USIM    = 2,
  BCM_APPTYPE_RUIM    = 3,
  BCM_APPTYPE_CSIM    = 4
} BRIL_AppType;

typedef enum {
    BCM_SIM_FILE_UPDATE = 0,
    BCM_SIM_INIT = 1,
    BCM_SIM_RESET = 2
} BRIL_SimRefreshResult;

#define BCM_RIL_RESTRICTED_STATE_NONE           0x00
#define BCM_RIL_RESTRICTED_STATE_CS_EMERGENCY   0x01
#define BCM_RIL_RESTRICTED_STATE_CS_NORMAL      0x02
#define BCM_RIL_RESTRICTED_STATE_CS_ALL         0x04
#define BCM_RIL_RESTRICTED_STATE_PS_ALL         0x10

// for Notification


/**
     @struct  KRIL_Command_t

     @ingroup KRIL_PROXY_COMMAND
 */
typedef struct
{
    unsigned short client;  //Client ID
    BRIL_Token t;          //TBD
    unsigned long CmdID;    //URIL Command ID
    void *data;             // pointer to user buffer
    size_t datalen;         // length of user buffer
} KRIL_Command_t;


typedef struct 
{
    unsigned short client; //Client ID
    BRIL_Token t;          //TBD
    unsigned long CmdID;   //URIL Command ID
    BRIL_Errno result;     //Response result
    void *data;            //pointer to user buffer
    size_t datalen;        //length of user buffer
} KRIL_Response_t;



// for Request
typedef struct {
    char address[BCM_MAX_DIGITS+1];
    int  clir;
            /* (same as 'n' paremeter in TS 27.007 7.7 "+CLIR"
             * clir == 0 on "use subscription default value"
             * clir == 1 on "CLIR invocation" (restrict CLI presentation)
             * clir == 2 on "CLIR suppression" (allow CLI presentation)
             */
    unsigned char isEmergency;
	unsigned char emergencySvcCat;                    ///< Emergency Service Category Value
    unsigned short isVTcall;
} KrilDial_t;


//Used by Kril_DataStateHandler
typedef struct {
    char l2p[L2P_STRING_LENGTH_MAX];
    int  cid;

} KrilDataState_t;

//Used by Kril_SendDataHandler
typedef struct {
    int numberBytes;
    int cid;
}KrilSendData_t;


typedef struct {
    int  index;
} KrilSeparate_t;


typedef struct {
    char   networkInfo[30];
    int    manualRat;
    int    permanentAutoEnable;
} KrilManualSelectInfo_t;


typedef struct {
    int    bandmode;
} KrilBandModeInfo_t;


typedef struct {
    int    networktype;
} KrilSetPreferredNetworkType_t;


typedef struct {
    int    location_updates;
} KrilLocationUpdates_t;


typedef struct {
    char    smsc[NUMBER_LENGTH_MAX+1];
} KrilSetSMSCAddress_t;


typedef struct {
    unsigned char  Len;
    unsigned char  Toa;
    unsigned char  Val[20]; 
    unsigned char  Length;
    unsigned char  Pdu[BCM_SMSMSG_SZ];
    unsigned char    IsMoreMessage;
} KrilSendMsgInfo_t;


typedef struct {
    unsigned char  MsgState;
    unsigned char  Length;
    unsigned char  MoreSMSToReceive; // for calss 2 SMS 
    unsigned char  Pdu[BCM_SMSMSG_SZ];
    int			   index;	// for replace type
} KrilWriteMsgInfo_t;

typedef struct {
	int recNum;		///< SMS Message index
	unsigned char mesg_data[BCM_SMSMSG_SZ];	///< SMS Message data including status byte
} KrilReadMsgRsp_t;

typedef struct {
    int       AckType;
    int       FailCause;
} KrilMsgAckInfo_t;


typedef struct {
    int       StringSize;
    char    USSDString[MAX_USSD_STRING+1];
	unsigned char	dcs;  // (dcs&0x0F)>>2: 0-Default Alphabet; 1-8bit; 2-UCS2
} KrilSendUSSDInfo_t;


typedef struct {
    int    value;
} KrilCLIRValue_t;


typedef struct {
    int    value;
} KrilCLIPInfo_t;


typedef struct {
    int    mode;
    int    reason;
    int    ss_class;
    char   number[BCM_MAX_DIGITS+1];
    int    timeSeconds;
} KrilCallForwardStatus_t;


typedef struct
{
    int   state;
    int   ss_class;
} KrilCallWaitingInfo_t;


typedef struct
{
    int    fac_id;
    char   OldPasswd[MAX_PW_LENGTH];
    char   NewPasswd[MAX_PW_LENGTH];
    char   NewPassConfirm[MAX_PW_LENGTH];
} KrilCallBarringPasswd_t;


typedef struct {
    char password[CHV_MAX_LENGTH];
    char newpassword[CHV_MAX_LENGTH];
} KrilSimPinNum_t;


typedef enum
{
  FAC_CS,    // lock CoNTRoL surface(e.g. phone keyboard)
  FAC_PS,    // PH-SIM (lock PHone to SIM/UICC card)
  FAC_PF,    // lock Phone to the very First inserted SIM/UICC card
  FAC_SC,    // SIM (lock SIM/UICC card)
  FAC_AO,    // Barr All Outgoing Calls
  FAC_OI,    // Barr Outgoing International Calls
  FAC_OX,    // Barr Outgoing International Calls except to Home Country
  FAC_AI,    // Barr All Incoming Calls
  FAC_IR,    // Barring incoming calls while roaming outside home area 
  FAC_NT,    // Barr incoming calls from numbers Not stored to TA memory 
  FAC_NM,    // Barr incoming calls from numbers Not stored to MT memory
  FAC_NS,    // Barr incoming calls from numbers Not stored to SIM/UICC memory 
  FAC_NA,    // Barr incoming calls from numbers Not stored in Any memory
  FAC_AB,    // All Barring services 
  FAC_AG,    // All outgoing barring services  
  FAC_AC,    // All incoming barring services
  FAC_FD,    // SIM card or active application in the UICC (GSM or USIM) 
             //   fixed dialling memory feature
  FAC_PN,    // Network Personalization
  FAC_PU,    // Network sUbset Personalization
  FAC_PP,    // Service Provider Personalization
  FAC_PC,    // Corporate Personalization
  TOTAL_FAC
} KrilFacility_t;


typedef struct
{
   KrilFacility_t    id;
   char              *name;
} FacTab_t;


typedef struct {
    KrilFacility_t     fac_id;
    char               password[CHV_MAX_LENGTH];
    int                service;
} KrilGetFacLock_t;


typedef struct {
    KrilFacility_t     fac_id;
    int                lock;
    char               password[CHV_MAX_LENGTH];
    int                service;
} KrilSetFacLock_t;


typedef struct {
    int  command;                          /* one of the commands listed for TS 27.007 +CRSM*/
    int  fileid;                           /* EF id */
    int  dfid;                             /* DF id */
    unsigned short path[MAX_SIMIO_FILE_PATH_LENGTH];       /* "pathid" from TS 27.007 +CRSM command.
                                                           Path is in hex asciii format eg "7f205f70" */
    int  pathlen;
    int  p1;
    int  p2;
    int  p3;
    char data[MAX_SIMIO_CMD_LENGTH];       /* May be NULL*/
    char pin2[MAX_SIMIO_PIN2_LENGTH];       /* May be NULL*/
} KrilSimIOCmd_t;


typedef enum {
    BRIL_HOOK_SET_PREFDATA, // To Set the which SIM need to support preferred data connection.
    BRIL_HOOK_QUERY_SIM_PIN_REMAINING,
    BRIL_HOOK_GET_SIM_VOLTAGE,
    BRIL_HOOK_GENERIC_SIM_ACCESS,
    BRIL_HOOK_EAP_SIM_AUTHENTICATION,	// BCM_EAP_SIM
    BRIL_HOOK_EAP_AKA_AUTHENTICATION	 

} HOOK_msgType;


// for response
typedef struct {
    BRIL_CallState  state;
    int             index;      /* GSM Index for use with, eg, AT+CHLD */
    int             toa;        /* type of address, eg 145 = intl */
    char            isMpty;     /* nonzero if is mpty call */
    char            isMT;       /* nonzero if call is mobile terminated */
    char            als;        /* ALS line indicator if available  (0 = line 1) */
    char            isVoice;    /* nonzero if this is is a voice call */
    char            isMTVTcall; /* indicate MT VT call */

    int             codetype;
    char            name[BCM_MAX_DIGITS*2];
    size_t          namelen;
    char            namePresentation;
    char            number[PHONE_NUMBER_LENGTH_MAX];     /* phone number */
    size_t          numlen;     /* phone number length*/
    char            numberPresentation; /* 0 = Allowed, 
                                           1 = Restricted,
                                           2 = Not Specified/Unknown, 
                                           3 = Payphone */
} KrilCallList_t;


typedef struct
{
    int total_call;
    int index;
    KrilCallList_t KRILCallState[BCM_MAX_CALLS_NO];
} KrilCallListState_t;


typedef struct
{
    int failCause;
} KrilLastCallFailCause_t;


typedef struct {
    int              type;
    int              codetype;
    unsigned char    Length;
    unsigned char    USSDString[PHASE1_MAX_USSD_STRING_SIZE+1];
} KrilReceiveUSSDInfo_t;


typedef struct {
    int    value1;
    int    value2;
} KrilCLIRInfo_t;


typedef struct
{
    int     activated;
    int     ss_class;
    int     ton;
    int     npi;
    char    number[BCM_MAX_DIGITS+1];
    int     noReplyTime;
} KrilCallForwardClassInfo_t;


typedef struct
{
    int    reason;
    int    class_size;
    KrilCallForwardClassInfo_t call_forward_class_info_list[BCM_MAX_SS_CLASS_SIZE];
} KrilCallForwardinfo_t;


typedef struct
{
    int     activated;
    int     ss_class;
} KrilCallWaitingClass_t;


typedef struct
{
    int class_size;
    KrilCallWaitingClass_t call_wait_class_info_list[BCM_MAX_SS_CLASS_SIZE];
} KrilCallWaitingClassInfo_t;


typedef struct
{
    unsigned int	    RAT;
    unsigned int	    RxLev;
    unsigned int    RxQual;
} KrilSignalStrength_t;


typedef struct
{
    unsigned char        gsm_reg_state;		///< GSM Registration state
    unsigned char        gprs_reg_state;	///< GPRS Registration state
    unsigned short       mcc;				        ///< MCC in Raw format (may include 3rd MNC digit), e.g. 0x13F0 for AT&T in Sunnyvale, CA
    unsigned char        mnc;				        ///< MNC in Raw format, e.g. 0x71 for AT&T in Sunnyvale, CA
    unsigned short       lac;				        ///< Location Area Code
    unsigned long        cell_id;			     ///< Cell ID
    unsigned char        network_type;	
    unsigned char        band;				       ///< Current band. For possible values see MS_GetCurrentBand()
    unsigned char        cause;
} KrilRegState_t;


typedef struct
{
    char	    numeric[7];
    char      shortname[PLMN_SHORT_NAME];
    char      longname[PLMN_LONG_NAME];
} KrilOperatorInfo_t;


typedef struct
{
   unsigned short       mcc;	
   unsigned char        mnc;
   unsigned char        network_type;
   unsigned char        rat;
   unsigned char        longname[PLMN_LONG_NAME];
   unsigned char        shortname[PLMN_SHORT_NAME];
} KrilAvailablePlmnList_t;


typedef struct
{
    unsigned char          num_of_plmn;
    KrilAvailablePlmnList_t  available_plmn[BCM_MAX_PLMN_SEARCH];
} KrilNetworkList_t;


typedef struct
{
    int    selection_mode;
} KrilSelectionMode_t;


typedef struct
{
    int   band_mode[18];
} KrilAvailableBandMode_t;


typedef struct
{
    int    network_type;
} krilGetPreferredNetworkType_t;


typedef struct
{
    int    cid;
    int    rssi;
} krilGetNeighborCell_t;


typedef struct
{
    int    mode;
} krilQueryTTYModeType_t;


typedef struct
{
    char    version[64];
} krilQueryBaseBandVersion_t;


typedef struct
{
    char    smsc[NUMBER_LENGTH_MAX+1];
} krilGetSMSCAddress_t;


typedef struct
{
    int messageRef;
    int errorCode;
} KrilSendSMSResponse_t;


typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is BCM_RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PIN2 = 4,
    SIM_PUK = 5,
    SIM_PUK2 = 6,
    SIM_NETWORK = 7,
    SIM_NETWORK_SUBSET = 8,
    SIM_CORPORATE = 9,
    SIM_SERVICE_PROVIDER = 10,
    SIM_SIM = 11,
    SIM_NETWORK_PUK = 12,
    SIM_NETWORK_SUBSET_PUK = 13,
    SIM_CORPORATE_PUK = 14,
    SIM_SERVICE_PROVIDER_PUK = 15,
    SIM_SIM_PUK = 16,
    SIM_PUK_BLOCK = 17,
    SIM_PUK2_BLOCK = 18,
    SIM_ERROR = 19
} SIM_Status;


typedef struct
{
	BRIL_Errno result;		    ///< SIM access result
	int remain_attempt;		  ///< Facility lock status.
} KrilSimPinResult_t;

/* Added for RIL_REQUEST_GET_SIM_STATUS. Let KRIL report APP type also. */
typedef struct
{
    SIM_Status         pin_status;
    SIM_Status         pin2_status;
    int                pin1_enable;
    int                pin2_enable;  
    BRIL_AppType       app_type;
} KrilSimStatusResult_t;


typedef struct
{
	BRIL_Errno result;		///< SIM access result
	int lock;		        ///< Facility lock status.
} KrilFacLock_t;


typedef struct {
    BRIL_Errno  result;		///< SIM access result
    int  command;
    int  fileid;
    int  sw1;
    int  sw2;
    char simResponse[MAX_SIMIO_RSP_LENGTH];  /* In hex string format ([a-fA-F0-9]*). */
    int searchcount;
} KrilSimIOResponse_t;


typedef struct
{
    BRIL_Errno result;		                ///< SIM access result
    char imsi[IMSI_DIGITS+1];		  ///< IMSI.
} KrilImsiData_t;


typedef struct
{
    BRIL_Errno result;		                ///< SIM access result
    char imei[IMEI_DIGITS+1];		        ///< IMEI (+1 for null termination)
    unsigned char  imeisv[3];		        ///< IMEISV (+1 for null termination)
} KrilImeiData_t;


typedef struct
{
    char stkprofile[MAX_STK_PROFILE_LEN*2+1];		///< STK profile
} KrilStkprofile_t;


typedef struct
{
    unsigned char usedSlots;	///< Number of used SMS slots on the SIM.
    unsigned char totalSlots;	///< Maximum number of SMS slots on the SIM.  (totalSlots - usedSlots = free slots)
} KrilSimSmsCapacity_t;


//for Notification

typedef struct {
    unsigned char   PDU[BCM_SMSMSG_SZ];
    unsigned short  pduSize;
} KrilMsgPDUInfo_t;


typedef struct {
    BRIL_Errno result;
    unsigned char index;
} KrilMsgIndexInfo_t;


typedef struct
{
    signed char      timeZone;
    unsigned char  dstAdjust;	///< Possible value (0, 1, 2 or "INVALID_DST_VALUE"). "INVALID_DST_VALUE" means network does not pass DST
							                                ///< (Daylight Saving Time) info in MM Information or GMM Information messages. See Section 10.5.3.12 of 3GPP 24.008.
    unsigned char  Sec;  ///< 0-59 seconds
    unsigned char  Min;  ///< 0-59 minutes
    unsigned char  Hour; ///< 0-23 hours
    unsigned char  Week; ///< 0-6==sun-sat week
    unsigned char  Day;  ///< 1-31 day
    unsigned char  Month; ///< 1-12 Month
    unsigned short Year; ///< (RTC_YEARBASE) - (RTC_YEARBASE + 99)
	unsigned short mcc;	///< MCC in Raw format (may include 3rd MNC digit), e.g. 0x13F0 for AT&T in Sunnyvale, CA
	unsigned char  mnc;	///< MNC in Raw format, e.g. 0x71 for AT&T in Sunnyvale, CA
} KrilTimeZoneDate_t;


typedef struct
{
    int notificationType;
    int code;
    int index;
    int type;
    char number[BCM_MAX_DIGITS+1];
} KrilSuppSvcNotification_t;


typedef enum
{
    AUTH_NONE = 0,
    AUTH_PAP,
    AUTH_CHAP,
    AUTH_BOTH
} Kril_AuthType;


typedef struct
{
    unsigned char cid;
    char apn[PDP_APN_LEN_MAX];
    char username[PDP_USERNAME_LEN_MAX];
    char password[PDP_PASSWORD_LEN_MAX];
    Kril_AuthType authtype;
} KrilPdpContext_t;


typedef struct
{
    unsigned char cid;        /* Context ID */
    char pdpAddress[PDP_ADDRESS_LEN_MAX];
    char apn[PDP_APN_LEN_MAX];
    unsigned long priDNS;
    unsigned long secDNS;
    int cause;
} KrilPdpData_t;


typedef struct {
    int  cid;        /* Context ID */
    int  active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up, 3=connecting, 4=disconnectiong */
    char type[PDP_TYPE_LEN_MAX];       /* X.25, IP, IPV6, etc. */
    char apn[PDP_APN_LEN_MAX];
    char address[PDP_ADDRESS_LEN_MAX];
} KrilDataCallResponse_t;


// Struct KrilGsmBroadcastSmsConfigInfo_t is for CBSMS data transfer btw kernel and user space
typedef struct {
    char mids[CHNL_IDS_SIZE];
    char dcss[CHNL_IDS_SIZE];
    unsigned char selected;
} KrilGsmBroadcastSmsConfigInfo_t;


// Struct iKrilGetCBSMSConfigInfo_t is compatible with Android struct
typedef struct {
    int fromServiceId;
    int toServiceId;
    int fromCodeScheme;
    int toCodeScheme;
    unsigned char selected;
} iKrilGetCBSMSConfigInfo_t;


// Struct KrilGsmBroadcastGetSmsConfigInfo_t is for multiple setup of mids/dcss
// The 10 sets of iKrilGetCBSMSConfigInfo_t is limited by CAPI2: MAX_MSG_ID_RANGE_LIST_SIZE
typedef struct {
    iKrilGetCBSMSConfigInfo_t content[MAX_CHNLID_LIST_SIZE];
} KrilGsmBroadcastGetSmsConfigInfo_t;

//	ycao@032311

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
    int alpha_id_present;				
    char alpha_id_len;				
    char alpha_id[64];				
    char call_id;				
   char old_call_type;                                   
    char modadd_ton;				
    char modadd_npi;				
    char modadd_len;				
    char modadd[200];				
}KrilStkCallCtrlResult_t;				

// Broadcom define Message data
//#define BRCM_URIL_REQUEST_BASE  (URILC_REQUEST_BASE - 100)
//#define BRCM_URIL_UNSOLICITED_BASE  (RIL_UNSOL_RESPONSE_BASE + 1000)


typedef struct {
    int    simAppType;
    char   simecclist[BCM_MAX_DIGITS];
} Kril_SIMEmergency;



/**
 *  The data structure carried by BRIL_REQUEST_KRIL_INIT
 *
 *  If is_valid_imei, KRIL will update CP MS element for IMEI.
 *  KRIL will then perform further initializations by calling the other CAPI calls.
 *
 *  The data structure now carries IMEI, and it can be exteneded in the future.
 */
typedef struct {
     int               is_valid_imei;
     char              imei[IMEI_DIGITS+1];
    int               networktype;
    int               band;
    //++ JSHAN Attach for next power on
    int               gprs_attach_init;
    //-- JSHAN Attach for next power on
} KrilInit_t;


typedef struct {
    char*  tlv;
    int    datalen;
} KrilTlvData;

typedef enum {
    BRIL_HOOK_UNSOL_SIM_ERROR,        // Notify user SIM Card is error.
    BRIL_HOOK_UNSOL_SIM_DATA		  //Notify user SIM Data for IMSI/GID1/GID2
} HOOK_unsol_msgType;



/* message id defination for request 
 *
 * Android request base start :: BRCM_RIL_REQUEST_GET_SIM_STATUS (1)
 * Android notification base start :: BRCM_RIL_UNSOL_RESPONSE_BASE (1000)
 * The request and notification value of constants defined are same as \\hardware\ril\include\telephony\ril.h
 *
 * AT request message base start :: URILC_REQUEST_BASE => BRCM_RIL_UNSOL_RESPONSE_BASE - 100  (900)
 * AT notification message base start :: URILC_UNSOL_RESPONSE_BASE => BRCM_RIL_UNSOL_RESPONSE_BASE + 500 (1500)
 * IMPORTANT NOTE: The request and notification value of constants defined here should range between 
 * {URILC_REQUEST_BASE+1....URILC_REQUEST_BASE+99} and {URILC_UNSOL_RESPONSE_BASE+1....URILC_UNSOL_RESPONSE_BASE+99} 
 * in order to avoid conflict with constants defined in \\hardware\ril\include\telephony\ril.h
 *
 * BRCM URIL message base start :: BRCM_URIL_REQUEST_BASE => BRCM_RIL_UNSOL_RESPONSE_BASE - 200 (800)
 * BRCM URIL notification message base start :: BRCM_URIL_UNSOLICITED_BASE => BRCM_RIL_UNSOL_RESPONSE_BASE + 600 (1600)
 * IMPORTANT NOTE: The request and notification value of constants defined here should range between 
 * {BRCM_URIL_REQUEST_BASE+1....BRCM_URIL_REQUEST_BASE+99} and {BRCM_URIL_UNSOLICITED_BASE+1....BRCM_URIL_UNSOLICITED_BASE+99}
 * in order to avoid conflict with constants defined in \\hardware\ril\include\telephony\ril.h
 *
 * VT request message base start :: BRCM_RIL_REQUEST_VT_BASE => BRCM_RIL_UNSOL_RESPONSE_BASE - 300 (700)
 * VT request notification base start :: BRCM_RIL_UNSOL_RESPONSE_VT_BASE => BRCM_RIL_UNSOL_RESPONSE_BASE + 700 (1700)
 * IMPORTANT NOTE: The request and notification value of constants defined here should range between
 * {BRCM_RIL_REQUEST_VT_BASE+1....BRCM_RIL_REQUEST_VT_BASE+99} and {BRCM_RIL_UNSOL_RESPONSE_VT_BASE+1....BRCM_RIL_UNSOL_RESPONSE_VT_BASE+99}
 * in order to avoid conflict with constants defined in \\hardware\ril\include\telephony\ril.h
 *
 * AGPS request message base start :: RIL_REQUEST_AGPS_BASE => BRCM_RIL_UNSOL_RESPONSE_BASE - 400 (600)
 * AGPS notification message base start :: RIL_UNSOL_RESP_AGPS_BASE	 => BRCM_RIL_UNSOL_RESPONSE_BASE + 800 (1800)
 * IMPORTANT NOTE: The request and notification value of constants defined here should range between 
 * {RIL_REQUEST_AGPS_BASE+1....RIL_REQUEST_AGPS_BASE+99} and {RIL_UNSOL_RESP_AGPS_BASE+1....RIL_UNSOL_RESP_AGPS_BASE+99}
 * in order to avoid conflict with constants defined in \\hardware\ril\include\telephony\ril.h
 *
 */


// Android request defination
#define BRCM_RIL_REQUEST_GET_SIM_STATUS 1
#define BRCM_RIL_REQUEST_ENTER_SIM_PIN 2
#define BRCM_RIL_REQUEST_ENTER_SIM_PUK 3
#define BRCM_RIL_REQUEST_ENTER_SIM_PIN2 4
#define BRCM_RIL_REQUEST_ENTER_SIM_PUK2 5
#define BRCM_RIL_REQUEST_CHANGE_SIM_PIN 6
#define BRCM_RIL_REQUEST_CHANGE_SIM_PIN2 7
#define BRCM_RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION 8
#define BRCM_RIL_REQUEST_GET_CURRENT_CALLS 9
#define BRCM_RIL_REQUEST_DIAL 10
#define BRCM_RIL_REQUEST_GET_IMSI 11
#define BRCM_RIL_REQUEST_HANGUP 12
#define BRCM_RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND 13
#define BRCM_RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND 14
#define BRCM_RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE 15
#define BRCM_RIL_REQUEST_SWITCH_HOLDING_AND_ACTIVE 15
#define BRCM_RIL_REQUEST_CONFERENCE 16
#define BRCM_RIL_REQUEST_UDUB 17
#define BRCM_RIL_REQUEST_LAST_CALL_FAIL_CAUSE 18
#define BRCM_RIL_REQUEST_SIGNAL_STRENGTH 19
#define BRCM_RIL_REQUEST_REGISTRATION_STATE 20
#define BRCM_RIL_REQUEST_GPRS_REGISTRATION_STATE 21
#define BRCM_RIL_REQUEST_OPERATOR 22
#define BRCM_RIL_REQUEST_RADIO_POWER 23
#define BRCM_RIL_REQUEST_DTMF 24
#define BRCM_RIL_REQUEST_SEND_SMS 25
#define BRCM_RIL_REQUEST_SEND_SMS_EXPECT_MORE 26
#define BRCM_RIL_REQUEST_SETUP_DATA_CALL 27
#define BRCM_RIL_REQUEST_SIM_IO 28
#define BRCM_RIL_REQUEST_SEND_USSD 29
#define BRCM_RIL_REQUEST_CANCEL_USSD 30
#define BRCM_RIL_REQUEST_GET_CLIR 31
#define BRCM_RIL_REQUEST_SET_CLIR 32
#define BRCM_RIL_REQUEST_QUERY_CALL_FORWARD_STATUS 33
#define BRCM_RIL_REQUEST_SET_CALL_FORWARD 34
#define BRCM_RIL_REQUEST_QUERY_CALL_WAITING 35
#define BRCM_RIL_REQUEST_SET_CALL_WAITING 36
#define BRCM_RIL_REQUEST_SMS_ACKNOWLEDGE  37
#define BRCM_RIL_REQUEST_GET_IMEI 38
#define BRCM_RIL_REQUEST_GET_IMEISV 39
#define BRCM_RIL_REQUEST_ANSWER 40
#define BRCM_RIL_REQUEST_DEACTIVATE_DATA_CALL 41
#define BRCM_RIL_REQUEST_QUERY_FACILITY_LOCK 42
#define BRCM_RIL_REQUEST_SET_FACILITY_LOCK 43
#define BRCM_RIL_REQUEST_CHANGE_BARRING_PASSWORD 44
#define BRCM_RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE 45
#define BRCM_RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC 46
#define BRCM_RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL 47
#define BRCM_RIL_REQUEST_QUERY_AVAILABLE_NETWORKS 48
#define BRCM_RIL_REQUEST_DTMF_START 49
#define BRCM_RIL_REQUEST_DTMF_STOP 50
#define BRCM_RIL_REQUEST_BASEBAND_VERSION 51
#define BRCM_RIL_REQUEST_SEPARATE_CONNECTION 52
#define BRCM_RIL_REQUEST_SET_MUTE 53
#define BRCM_RIL_REQUEST_GET_MUTE 54
#define BRCM_RIL_REQUEST_QUERY_CLIP 55
#define BRCM_RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE 56
#define BRCM_RIL_REQUEST_DATA_CALL_LIST 57
#define BRCM_RIL_REQUEST_RESET_RADIO 58
#define BRCM_RIL_REQUEST_OEM_HOOK_RAW 59
#define BRCM_RIL_REQUEST_OEM_HOOK_STRINGS 60
#define BRCM_RIL_REQUEST_SCREEN_STATE 61
#define BRCM_RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION 62
#define BRCM_RIL_REQUEST_WRITE_SMS_TO_SIM 63
#define BRCM_RIL_REQUEST_DELETE_SMS_ON_SIM 64
#define BRCM_RIL_REQUEST_SET_BAND_MODE 65
#define BRCM_RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE 66
#define BRCM_RIL_REQUEST_STK_GET_PROFILE 67
#define BRCM_RIL_REQUEST_STK_SET_PROFILE 68
#define BRCM_RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND 69
#define BRCM_RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE 70
#define BRCM_RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM 71
#define BRCM_RIL_REQUEST_EXPLICIT_CALL_TRANSFER 72
#define BRCM_RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE 73
#define BRCM_RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE 74
#define BRCM_RIL_REQUEST_GET_NEIGHBORING_CELL_IDS 75
#define BRCM_RIL_REQUEST_SET_LOCATION_UPDATES 76
#define BRCM_RIL_REQUEST_CDMA_SET_SUBSCRIPTION 77
#define BRCM_RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE 78
#define BRCM_RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE 79
#define BRCM_RIL_REQUEST_SET_TTY_MODE 80
#define BRCM_RIL_REQUEST_QUERY_TTY_MODE 81
#define BRCM_RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE 82
#define BRCM_RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE 83
#define BRCM_RIL_REQUEST_CDMA_FLASH 84
#define BRCM_RIL_REQUEST_CDMA_BURST_DTMF 85
#define BRCM_RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY 86
#define BRCM_RIL_REQUEST_CDMA_SEND_SMS 87
#define BRCM_RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE 88
#define BRCM_RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG 89
#define BRCM_RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG 90
#define BRCM_RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION 91
#define BRCM_RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG 92
#define BRCM_RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG 93
#define BRCM_RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION 94
#define BRCM_RIL_REQUEST_CDMA_SUBSCRIPTION 95
#define BRCM_RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM 96
#define BRCM_RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM 97
#define BRCM_RIL_REQUEST_DEVICE_IDENTITY 98
#define BRCM_RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE 99
#define BRCM_RIL_REQUEST_GET_SMSC_ADDRESS 100
#define BRCM_RIL_REQUEST_SET_SMSC_ADDRESS 101
#define BRCM_RIL_REQUEST_REPORT_SMS_MEMORY_STATUS 102
#define BRCM_RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING 103

// Android notification defination
#define BRCM_RIL_UNSOL_RESPONSE_BASE 1000
#define BRCM_RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED 1000
#define BRCM_RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED 1001
#define BRCM_RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED 1002
#define BRCM_RIL_UNSOL_RESPONSE_NEW_SMS 1003
#define BRCM_RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT 1004
#define BRCM_RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM 1005
#define BRCM_RIL_UNSOL_ON_USSD 1006
#define BRCM_RIL_UNSOL_ON_USSD_REQUEST 1007
#define BRCM_RIL_UNSOL_NITZ_TIME_RECEIVED  1008
#define BRCM_RIL_UNSOL_SIGNAL_STRENGTH  1009
#define BRCM_RIL_UNSOL_DATA_CALL_LIST_CHANGED 1010
#define BRCM_RIL_UNSOL_SUPP_SVC_NOTIFICATION 1011
#define BRCM_RIL_UNSOL_STK_SESSION_END 1012
#define BRCM_RIL_UNSOL_STK_PROACTIVE_COMMAND 1013
#define BRCM_RIL_UNSOL_STK_EVENT_NOTIFY 1014
#define BRCM_RIL_UNSOL_STK_CALL_SETUP 1015
#define BRCM_RIL_UNSOL_SIM_SMS_STORAGE_FULL 1016
#define BRCM_RIL_UNSOL_SIM_REFRESH 1017
#define BRCM_RIL_UNSOL_CALL_RING 1018
#define BRCM_RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED 1019
#define BRCM_RIL_UNSOL_RESPONSE_CDMA_NEW_SMS 1020
#define BRCM_RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS 1021
#define BRCM_RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL 1022
#define BRCM_RIL_UNSOL_RESTRICTED_STATE_CHANGED 1023
#define BRCM_RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE 1024
#define BRCM_RIL_UNSOL_CDMA_CALL_WAITING 1025
#define BRCM_RIL_UNSOL_CDMA_OTA_PROVISION_STATUS 1026
#define BRCM_RIL_UNSOL_CDMA_INFO_REC 1027
#define BRCM_RIL_UNSOL_OEM_HOOK_RAW 1028
#define BRCM_RIL_UNSOL_RINGBACK_TONE 1029
#define BRCM_RIL_UNSOL_RESEND_INCALL_MUTE 1030


// URIL request defination
#define BRCM_URIL_REQUEST_BASE  (BRCM_RIL_UNSOL_RESPONSE_BASE - 200)

/**
 * BRIL_REQUEST_KRIL_INIT
 *
 * 
 * URIL reads IMEI with flash OTP library and sends it to KRIL.
 * KRIL updates the IMEI to CP.
 * Also, it will trigger the initialization of KRIL.
 *
 * "data" is char *
 * ((char *)data) is a 15 digit string (ASCII digits '0'-'9') for IMEI
 * An extra byte is allocated for NULL ending string printf debug purpose
 * 
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  GENERIC_FAILURE
 *
 */
#define BRIL_REQUEST_KRIL_INIT (BRCM_URIL_REQUEST_BASE + 1)


// URIL notification defination
#define BRCM_URIL_UNSOLICITED_BASE  (BRCM_RIL_UNSOL_RESPONSE_BASE + 600)

/**
 * BRIL_UNSOL_EMERGENCY_NUMBER
 *
 * Reports emergency number in SIM.
 *
 * "data" is a char number[BCM_MAX_DIGITS]
 *
 */
#define BRIL_UNSOL_EMERGENCY_NUMBER (BRCM_URIL_UNSOLICITED_BASE+1)


#ifdef VIDEO_TELEPHONY_ENABLE
// VT request defination
#define BRCM_RIL_REQUEST_VT_BASE                   BRCM_RIL_UNSOL_RESPONSE_BASE - 300
#define BRCM_RIL_REQUEST_DIAL_VT                   BRCM_RIL_REQUEST_VT_BASE + 1
#define BRCM_RIL_REQUEST_ANSWER_VT                 BRCM_RIL_REQUEST_VT_BASE + 2
#define BRCM_RIL_REQUEST_HANGUP_VT                 BRCM_RIL_REQUEST_VT_BASE + 3
#define BRCM_RIL_REQUEST_MODIFY_VT                 BRCM_RIL_REQUEST_VT_BASE + 4
#define BRCM_RIL_REQUEST_CONFIRM_VT                BRCM_RIL_REQUEST_VT_BASE + 5

// VT notification defination
#define BRCM_RIL_UNSOL_RESPONSE_VT_BASE                               BRCM_RIL_UNSOL_RESPONSE_BASE + 700
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_CALL_CONF               BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 1
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_PROGRESS_INFO_IND       BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 2
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_CONNECT                 BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 3
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_SETUP_IND               BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 4
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_INCOM                   BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 5
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_END                     BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 6
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_MODIFY_IND              BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 7
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_MODIFY_REQ              BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 8
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_MODIFY_COMPLETE_CONF    BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 9
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_MODIFY_RES              BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 10
#define BRCM_RIL_UNSOL_RESPONSE_VT_CALL_EVENT_UNKNOWN                 BRCM_RIL_UNSOL_RESPONSE_VT_BASE + 11
#endif //VIDEO_TELEPHONY_ENABLE



//AGPS
#ifdef BRCM_AGPS_CONTROL_PLANE_ENABLE

#define RIL_REQUEST_AGPS_BASE				BRCM_RIL_UNSOL_RESPONSE_BASE - 400

/**
 * RIL_REQUEST_AGPS_SEND_UPLINK
 *
 * Send UPLINK AGPS Control Plane data to network
 *
 * "data" is const BrcmAgps_CPlaneData *
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
#define RIL_REQUEST_AGPS_SEND_UPLINK				RIL_REQUEST_AGPS_BASE+1
#define RIL_REQUEST_AGPS_RRC_MEAS_CTRL_FAILURE		RIL_REQUEST_AGPS_BASE+2
#define RIL_REQUEST_AGPS_RRC_STATUS					RIL_REQUEST_AGPS_BASE+3

#define RIL_UNSOL_RESP_AGPS_BASE					BRCM_RIL_UNSOL_RESPONSE_BASE + 800

/**
 * RIL_UNSOL_RESP_AGPS_DLINK_DATA_IND
 *
 * Notify AGPS Downlink Data
 *
 * "data" is a const AgpsCp_Data *
 */
#define RIL_UNSOL_RESP_AGPS_DLINK_DATA_IND			RIL_UNSOL_RESP_AGPS_BASE+1

/**
 * RIL_UNSOL_RESP_AGPS_UE_STATE_IND
 *
 * Notify AGPS UE State
 *
 * "data" is a const AgpsCp_UeStateData *
 */
#define RIL_UNSOL_RESP_AGPS_UE_STATE_IND			RIL_UNSOL_RESP_AGPS_BASE+2

/**
 * RIL_UNSOL_RESP_AGPS_RESET_STORED_INFO_IND
 *
 * Notify reset stored AGPS information
 *
 * "data" is a const AgpsCp_Protocol *
 */
#define RIL_UNSOL_RESP_AGPS_RESET_STORED_INFO_IND	RIL_UNSOL_RESP_AGPS_BASE+3

#define BRCM_AGPS_MAX_MESSAGE_SIZE 900
/// RRC UE state types
typedef enum 
{
    AGPS_UE_STATE_CELL_DCH,  /**< Dedicated Channel*/
    AGPS_UE_STATE_CELL_FACH, /**< Forward Access Channel */
    AGPS_UE_STATE_CELL_PCH,  /**< Paging Channel */
    AGPS_UE_STATE_URA_PCH,   /**< UTRAN Registration Area Paging Channel */
    AGPS_UE_STATE_IDLE       /**< Idle */
} AgpsCp_UeState;

typedef enum 
{
	AGPS_PROC_RRLP,
	AGPS_PROC_RRC,
	AGPS_PROC_LTE
} AgpsCp_Protocol;

typedef struct 
{
  AgpsCp_Protocol	protocol;
  int				cPlaneDataLen;
  unsigned char		cPlaneData[BRCM_AGPS_MAX_MESSAGE_SIZE];
} AgpsCp_Data;

typedef struct 
{
  AgpsCp_Protocol	protocol;
  AgpsCp_UeState	ueState;
} AgpsCp_UeStateData;

/**
	Measurement control failure
**/
typedef enum 
{
    AGPS_RRC_invalidConfiguration,                 
    AGPS_RRC_configurationUnsupported,             
    AGPS_RRC_unsupportedMeasurement			                                
} AgpsCp_RrcMcFailureCode;

typedef struct 
{
  unsigned short			transId;
  AgpsCp_RrcMcFailureCode	failureCode;
} AgpsCp_McFailure;

typedef enum 
{
    AGPS_RRC_STATUS_asn1_ViolationOrEncodingError,
    AGPS_RRC_STATUS_messageTypeNonexistent
} AgpsCp_RrcStatus;

#endif //BRCM_AGPS_CONTROL_PLANE_ENABLE

//SAMSUNG_SELLOUT_FEATURE
typedef struct
{
	unsigned char	bSelloutEnable;
	unsigned char	OperateMode;	
	unsigned char	ProdutInfo;
	unsigned char	bSmsSending;
	char			Mcc[4];
	char			Mnc[3];
}SellOutSMS;

#define FLASH_READ_NV_DATA( _Buf_, _Idx_ )		Flash_Read_NV_Data( _Buf_, _Idx_ )
#define FLASH_WRITE_NV_DATA( _Buf_, _Idx_ )	Flash_Write_NV_Data( _Buf_, _Idx_ )
//SAMSUNG_SELLOUT_FEATURE


#ifdef OEM_RIL_ENABLE
#include "bcm_kril_ext.h"
#endif

#ifdef __cplusplus
}
#endif

#endif //_BCM_KRIL_H
