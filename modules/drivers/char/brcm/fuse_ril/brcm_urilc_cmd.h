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
*  @file   bcm_urilc_cmd.h
*
*  @brief  Defines the constants used for IPC between the AT server and URIL & KRIL.
*
* This file is a exact copy of brcm\public\bcm_urilc_cmd.h
*  and it should be kept in sync with abovementioned file. 
*
*
*
*IMPORTANT NOTE: The value of constants defined here should range between 
*					{URILC_REQUEST_BASE+1....URILC_REQUEST_BASE+99} in order to
*					avoid conflict with constants defined in ril.h
****************************************************************************/
#ifndef BCM_URILC_CMD_DEF_H__
#define BCM_URILC_CMD_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define URILC_REQUEST_BASE  (BRCM_RIL_UNSOL_RESPONSE_BASE - 100)



/***********************************************************************/

/**
 * URILC_REQUEST_DATA_STATE
 *
 * Request to enter data state
 * 
 *
 * "data" is l2p & cid parameters
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */

#define URILC_REQUEST_DATA_STATE (URILC_REQUEST_BASE+1)



/***********************************************************************/

/**
 * URILC_REQUEST_SEND_DATA
 *
 * Request to send specified bytes of data
 * 
 *
 * "data" is number of bytes & cid parameters
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_SEND_DATA (URILC_REQUEST_BASE+2)


/***********************************************************************/

/**
 * URILC_REQUEST_GET_SMS_SIM_MAX_CAPACITY
 *
 * Request to get the maximum capacity of SMS mesasges on the SIM.
 *
 *
 * "data" is NULL
 * "response" is KrilSimSmsCapacity_t*, a structure which contains
 * the number of used slots, and the maximum number of slots on the SIM.
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_GET_SMS_SIM_MAX_CAPACITY (URILC_REQUEST_BASE+3)


/***********************************************************************/

/**
 * URILC_REQUEST_READ_SMS_ON_SIM
 *
 * Request to read the contents of the SIM SMS message at the given record
 * index.
 *
 *
 * "data" is int, the record number to read
 * "response" is KrilReadMsgRsp_t *
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_READ_SMS_ON_SIM (URILC_REQUEST_BASE+4)


/***********************************************************************/

/**
 * URILC_REQUEST_LIST_SMS_ON_SIM
 *
 * Request to list the SIM SMS messages.
 * Each SMS message's contents are broadcast one by one via
 * URILC_UNSOL_RESP_LIST_SMS_ON_SIM.
 *
 * "data" is int, the status of messages to list.
 * See TS 27.005 3.1, "<stat>":
 *                        0 = "REC UNREAD"
 *                        1 = "REC READ"
 *                        2 = "STO UNSENT"
 *                        3 = "STO SENT"
 * or
 *                        4 = all messages
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_LIST_SMS_ON_SIM (URILC_REQUEST_BASE+5)


/***********************************************************************/

/**
 * URILC_REQUEST_SEND_STORED_SMS
 *
 * Request to send a stored SMS message.
 *
 *
 * "data" is int, the record number to send
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_SEND_STORED_SMS (URILC_REQUEST_BASE+6)


/***********************************************************************/

/**
 * URILC_REQUEST_GET_ELEM_CSCS
 *
 * Request to get the CSCS value.
 *
 *
 * "data" is NULL
 * "response" is char[10], the character set to be used for conversion
 * of char between TE and MT.
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_GET_ELEM_CSCS (URILC_REQUEST_BASE+7)

/***********************************************************************/

/**
 * URILC_REQUEST_GET_SMS_ELEM_MORE_MSG_TO_SEND
 *
 * Request to get the MS_LOCAL_SMS_ELEM_MORE_MESSAGE_TO_SEND value.
 *
 *
 * "data" is NULL
 * "response" is UInt8, the More Message to Send flag.
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_GET_SMS_ELEM_MORE_MSG_TO_SEND (URILC_REQUEST_BASE+8)

/***********************************************************************/

/**
 * URILC_REQUEST_SET_SMS_ELEM_MORE_MSG_TO_SEND
 *
 * Request to set the MS_LOCAL_SMS_ELEM_MORE_MESSAGE_TO_SEND value.
 *
 *
 * "data" is UInt8, the More Message to Send flag.
 * "response" is NULL.
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_SET_SMS_ELEM_MORE_MSG_TO_SEND (URILC_REQUEST_BASE+9)


/***********************************************************************/

/**
 * URILC_REQUEST_START_MULTI_SMS_TX_REQ
 *
 * Request to start a multi SMS transfer.
 *
 * "data" is NULL.
 * "response" is NULL.
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_START_MULTI_SMS_TX_REQ (URILC_REQUEST_BASE+10)


/***********************************************************************/

/**
 * URILC_REQUEST_STOP_MULTI_SMS_TX_REQ
 *
 * Request to stop a multi SMS transfer.
 *
 * "data" is NULL.
 * "response" is NULL.
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_STOP_MULTI_SMS_TX_REQ (URILC_REQUEST_BASE+11)




#define URILC_UNSOL_RESPONSE_BASE	(BRCM_RIL_UNSOL_RESPONSE_BASE + 500)

/***********************************************************************/

/**
 * URILC_UNSOL_RESP_LIST_SMS_ON_SIM
 *
 * Unsolicited response to list the SIM SMS messages.
 *
 *
 * "data" is KrilReadMsgRsp_t *
 * The response is sent for each message on the SIM card.
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_UNSOL_RESP_LIST_SMS_ON_SIM (URILC_UNSOL_RESPONSE_BASE+1)


#endif //BCM_URILC_CMD_DEF_H__
