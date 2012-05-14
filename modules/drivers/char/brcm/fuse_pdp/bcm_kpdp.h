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
*  @file   bcm_kpdp.h
*
*  @brief  Defines the message id and data structure used for userspace app & KPDP.
*
* This file is a exact copy of bcm_kpdp.h in the userspace
*  and it should be kept in sync with abovementioned file.
*
****************************************************************************/
#ifndef _BCM_KPDP_H
#define _BCM_KPDP_H

#ifdef __cplusplus
extern "C" {
#endif


//ioctl command
#define BCM_KPDPIO_SET_COMMAND 100
#define BCM_KPDPIO_GET_RESPONSE 101

//pdp command
#define PDP_REQUEST_SETUP_DATA_CALL 1
#define PDP_REQUEST_DEACTIVATE_DATA_CALL 2

//pdp notify >=1000
#define PDP_UNSOL_DATA_CALL_LIST_CHANGED 1001
#define PDP_DEACTIVATION_IND 1002

// Structure data
#define RESULT_BUFFER_LEN_MAX 1000//byte

// for PDP
#define PDP_TYPE_LEN_MAX     10
#define PDP_ADDRESS_LEN_MAX  20
#define PDP_APN_LEN_MAX      101
#define PDP_USERNAME_LEN_MAX 65
#define PDP_PASSWORD_LEN_MAX 65
#define L2P_STRING_LENGTH_MAX 6		//possible l2p values: PPP, M-xxxx, BRCM supports only PPP

#define PDP_CHALLENGE_LEN_MAX (MAX_CHALLENGE_LENGTH+MAXSECRETLEN+CHAP_HEADERLEN+1)
#define PDP_RESPONSE_LEN_MAX PDP_CHALLENGE_LEN_MAX

// for Notification


/**
     @struct  KPDP_Command_t

     @ingroup KPDP_PROXY_COMMAND
 */
typedef struct
{
    unsigned long CmdID;    //PDP Command ID
    void *data;             // pointer to user buffer
    size_t datalen;         // length of user buffer
} KPDP_Command_t;


typedef struct 
{
    unsigned long CmdID;   //pdp Command ID
    Result_t result;      //Response result
    void *data;            //pointer to user buffer
    size_t datalen;        //length of user buffer
} KPDP_Response_t;


//pdp related
typedef enum
{
    AUTH_NONE = 0,
    AUTH_PAP,
    AUTH_CHAP,
    AUTH_BOTH
} Kpdp_AuthType;


typedef struct
{
    unsigned char cid;
    unsigned char chall_len;
    unsigned char resp_len;
    char apn[PDP_APN_LEN_MAX];
    char username[PDP_USERNAME_LEN_MAX];
    char password[PDP_PASSWORD_LEN_MAX];
    char challenge[PDP_CHALLENGE_LEN_MAX];
    char response[PDP_RESPONSE_LEN_MAX];
    Kpdp_AuthType authtype;
} KpdpPdpContext_t;


typedef struct
{
    unsigned char cid;        /* Context ID */
    char pdpAddress[PDP_ADDRESS_LEN_MAX];
    unsigned long priDNS;
    unsigned long secDNS;
    int cause;
} KpdpPdpData_t;


typedef struct {
    int  cid;        /* Context ID */
    int  active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
    char type[PDP_TYPE_LEN_MAX];       /* X.25, IP, IPV6, etc. */
    char apn[PDP_APN_LEN_MAX];
    char address[PDP_ADDRESS_LEN_MAX];
} KpdpDataCallResponse_t;

typedef struct {
    char l2p[L2P_STRING_LENGTH_MAX];
    int  cid;

} KpdpDataState_t;

//Used by Kpdp_SendDataHandler
typedef struct {
    int numberBytes;
    int cid;
}KpdpSendData_t;


#ifdef __cplusplus
}
#endif

#endif //_BCM_KPDP_H
