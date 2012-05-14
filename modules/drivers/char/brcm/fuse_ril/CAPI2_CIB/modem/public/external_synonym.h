/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
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
/* 
** TR721 - mjoang 07/21/04
** Use external synonym so that codegen is not needed when # of instances changes. 
** TR721 - wtsai 9/10/03 
** For TC 45.2.2, only one PDP context activation on MS side is supported.
*/
/* For MO - UE Initiated PDP Context Connections */
#define ATC_INSTANCES		2 /* changed from 2 */
#define MMI_INSTANCES		3 /* changed from 2, total number of WAP connections over IPRELAY for CQ63247*/
#define SM_INSTANCES		ATC_INSTANCES+MMI_INSTANCES 

/* For MT - NET_INITIATED PDP Context Connections */
#define ATC_NET_INSTANCES	2
#define MMI_NET_INSTANCES	2
#define SM_NET_INSTANCES	ATC_NET_INSTANCES+MMI_NET_INSTANCES 

#define MS_NSAPI_INSTANCES	SM_INSTANCES

#define SNPD_NO_CONTEXT		MS_NSAPI_INSTANCES 
 

/** 
MAX_DATA_CONNECTIONS defines the total number of (DUN + IPRELAY + WLAN + IP_LPBK) connections by adding the following macros:

1. ATC_INSTANCES for DUN connection.  
2. MMI_INSTANCES for WAP connections over IPRELAY
3. WLAN_INSTANCES connection if any, e.g. 1 for 21331/2153
4. IPLPBK_INSTANCES connection if any, e.g. 1 for 21331/2153
**/
 
#define WLAN_INSTANCES		0
#define IPLPBK_INSTANCES	0 
 
#define MAX_DATA_CONNECTIONS  (ATC_INSTANCES+MMI_INSTANCES+WLAN_INSTANCES+IPLPBK_INSTANCES) 


#define SI_ALL_IND 			4294967295UL  /* MobC00096237: fix the compilation warning */

#define INVALID_CELL_IDENTITY	4294967295UL /* MobC00096237: fix the compilation warning */

/* GNATS TR3147 yaxin 1/24/05
We can extract some SDL definition to this external header file to 
have flexibility for different builds. Also, we can change constant
definition here without doing code-gen */


#ifdef SDL_MODULE_TEST 
#define UMTS_BAND_LIST_LENGTH 10
#elif defined URRC_R5_UNIT_TESTING
#define UMTS_BAND_LIST_LENGTH 10
#elif defined UMTS
#define UMTS_BAND_LIST_LENGTH 10
#else		/* To avoid log decoding error in classmark, change it to 10. Tony 07/01/08 */
#define UMTS_BAND_LIST_LENGTH 10

#endif


/* BRCM/lchan 05/12/2006 (CSP 34572): define the maximum number of retries we do for L3 refresh. Two tries
 * have 1.5s interval in between. For example, "L3_REFRESH_MAX_RETRY = 6" means we will retry for a total 
 * of 6*1.5=9s before we time out and send terminal response with failure status to SIM. 
 */
#define L3_REFRESH_MAX_RETRY 6


/* Number of retry attempts for SIMN_SMS_SEND_PDU_REQ signal */
#define L3_SEND_PDU_MAX_RETRY 2


