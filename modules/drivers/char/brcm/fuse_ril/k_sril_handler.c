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
#include "bcm_kril_common.h"
#include "bcm_kril_capi2_handler.h"
#include "bcm_kril_cmd_handler.h"

#include "bcm_cp_cmd_handler.h"
#include "capi2_gen_msg.h"
#include "capi2_sms_ds.h"
#include "capi2_sms_api.h"
#include "k_sril_handler.h"
#include "k_sril_svcmode_handler.h"
#include "k_sril_personalization_handler.h" 
#include "k_sril_gprs_handler.h"
#include "k_sril_imei_handler.h"

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @ SRIL RIL_REQUEST_LOCK_INFO
//
// @comm None
//
// -----------------------------------------------------------------------------
#define BCM_SRIL_CHECK_PIN_STATUS  	0x500#define BCM_SRIL_CHECK_PIN2_FDN		0x501

#define SI_CHV_NOT_AVAILABLE 0
#define SI_CHV_NEEDED 1
#define SI_CHV_NOT_NEEDED 2
#define SI_CHV_BLOCKED 3
#define SI_CHV_UNBL_BLOCKED 4
#define SI_CHV_VERIFIED 5
#define SI_CHV_REJECTED 6
#define SI_CHV_IN_CONTRADICTION_WITH_CHV_STATUS 7
#define SI_CHV1_DISABLE_NOT_ALLOWED 8
#define SI_CHV_MEMORY_PROBLEM 9

/// CHV Status
typedef enum
{
	CHVSTATUS_CHV_NOT_AVAILABLE = SI_CHV_NOT_AVAILABLE,		///< CHV is not available
	CHVSTATUS_CHV_NEEDED 		= SI_CHV_NEEDED,			///< CHV available and must be entered
	CHVSTATUS_CHV_NOT_NEEDED 	= SI_CHV_NOT_NEEDED,		///< CHV available, but no need
	CHVSTATUS_CHV_BLOCKED 		= SI_CHV_BLOCKED,			///< blocked, but unblocking is possible
	CHVSTATUS_CHV_UNBL_BLOCKED 	= SI_CHV_UNBL_BLOCKED,		///< unblocking is not possible
	CHVSTATUS_CHV_VERIFIED 		= SI_CHV_VERIFIED,			///< CHV has been verified
	CHVSTATUS_CHV_REJECTED 		= SI_CHV_REJECTED,			///< CHV vlaue just presented is incorrect
	CHVSTATUS_CHV_CONTRADICTION = SI_CHV_IN_CONTRADICTION_WITH_CHV_STATUS, ///<STK++ SI_CHV_CONTRADICTION is changed to SI_CHV_IN_CONTRADICTION_WITH_CHV_STATUS STK
	CHVSTATUS_CHV1_DISABLE_NOT_ALLOWED = SI_CHV1_DISABLE_NOT_ALLOWED, ///< Disabling CHV1 is not allowed in EF-SST of 2G SIM 
	CHVSTATUS_CHV_MEMORY_PROBLEM = SI_CHV_MEMORY_PROBLEM	///< Memory problem status word returned by SIM 
} KrilCHVStatus_t;

extern CallIndex_t  gUssdID;
extern CallIndex_t  gPreviousUssdID;

PBK_Id_t fileid_to_pbk_id(int fileid)
{
    PBK_Id_t pbk_id;

    /*
    int fileid : EF ID (Elementary File ID) is specified in TS 51.011.

    To use +CPBx command,
    <fileid> parameter can be convert to phonebook storage.

    SIM File ID           Phonebook Stroage ID
    ------------------    -------------------------------
    EF_ADN    = 0x6F3A    3GSIM = 0x0C (3G SIM)
                          SIM   = 0x0A (2G SIM and others)
    EF_FDN    = 0x6F3B    FD    = 0x03
    EF_SDN    = 0x6F49    SDN   = 0x0B
    EF_MSISDN = 0x6F40    ON    = 0x08
    OTHERS                SIM   = 0x0A (2G SIM and others)
    */
    // FixMe: need to avoid use hard-coded constant such as 0x01 by adding Samsung PB_DC etc into ril.h
    switch(fileid)
    {
        case 0x6F3B:
        //PB_FD    0x03 SIM fixed-dialing phonebook
        //PB_FD-----> PB_FDN
            pbk_id = PB_FDN;
            break;
        case 0x6F49:
        //PB_SDN   0x0B Service Dialing Number
        //PB_SDN ----> PB_SDN
            pbk_id = PB_SDN;
            break;
        case 0x6F40:
        //PB_ON    0x08 SIM(or ME) own numbers ( MSISDNs) list
        //PB_ON----> PB_MSISDN
            pbk_id = PB_MSISDN;
            break;
        case 0x6F3A:    // FixMe -- only 2G supported
        default:
        //PB_3GSIM ----> PB_3G_GLOBAL or PB_3G_LOCAL. You have to see global or local pbk exists to decide based on response to PbkApi_SendInfoReq() API
        //PB_SIM   0x0A 2G SIM phonebook
        //PB_SIM ---> PB_ADN
            pbk_id = PB_ADN;
            break;
    }

    return pbk_id;
}

int Usim_pbkid_to_int(USIM_PBK_ID_t usim_pbk_id)
{
	int type = 0x00;
	switch(usim_pbk_id)
	{
		case PBK_3G_NAME : 
			type = 0x01; break;
		case PBK_3G_NUMBER :
			type = 0x02; break;
		case PBK_3G_ANR :
			type = 0x03; break;
		case PBK_3G_EMAIL :
			type = 0x04; break;
		case PBK_3G_SNE :
			type = 0x05; break;
		case PBK_3G_GRP :
			type = 0x06; break;
		case PBK_3G_PBC :
			type = 0x07; break;
		case PBK_3G_ANR_A :
			type = 0x08; break;
		case PBK_3G_ANR_B :
			type = 0x09; break;
		case PBK_3G_ANR_C :
			type = 0x0A; break;
		case PBK_3G_EMAIL_A :
			type = 0x0B; break;
		case PBK_3G_EMAIL_B :
			type = 0x0C; break;
		case PBK_3G_EMAIL_C :
			type = 0x0D; break;
		default:
	                KRIL_DEBUG(DBG_ERROR, "Usim_pbkid_to_int : invalid case : %d \n", usim_pbk_id);		
			break;
	}

        KRIL_DEBUG(DBG_ERROR, "Usim_pbkid_to_int : type : %d \n", type);		

	return type;
}



void KRIL_SRIL_SetCellBroadcastHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "KRIL_SRIL_SetCellBroadcastHandler_state:0x%04X::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            Kril_Cell_Broadcast_config *tdata = (Kril_Cell_Broadcast_config *)pdata->ril_cmd->data;

            UInt8* pchnlIDs = (UInt8 *)(tdata->msgIDs);
            UInt8  mode =0;
			int i = 0;

            if (tdata->bCBEnabled == 1) {
                // turn on CB monitoring
                if( tdata->selectedId == 1 )	//all channels
					mode = 2;
                else /* if( tdata->selectedId == 2 ) */	//my channels
					mode = 0;
            }
            else {
                // turn off CB monitoring
                mode = 1;
                pchnlIDs = NULL;
            }

            KRIL_DEBUG(DBG_INFO, "[SET_CB]BCM_SendCAPI2Cmd: bCBEnabled[%d], mode[%d]\n", tdata->bCBEnabled, mode);
            KRIL_DEBUG(DBG_INFO, "[SET_CB]BCM_SendCAPI2Cmd: selectedId[%d], msgIdMaxCount[%d], msgIdCount[%d]\n", 
				tdata->selectedId, tdata->msgIdMaxCount, tdata->msgIdCount);

            if (NULL != pchnlIDs)
            {
                KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "[SET_CB]BCM_SendCAPI2Cmd: pchnlIDs[%s]\n", pchnlIDs);
            }

            if ( 0 == mode ) //Active (My channels)
            {
                // Always remove all existing CBMI IDs from the search list and from SIM EF_CBMI first.
                // The actual CBMI activation will be done in the next state 'BCM_RemoveCBMI'.
                CAPI2_SMS_RemoveAllCBChnlFromSearchList(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_RemoveCBMI;
            }
            else if ( 2 == mode ) //Active (All channels)
            {
                CAPI2_SMS_SetCellBroadcastMsgTypeReq(GetNewTID(), GetClientID(), 2, NULL, NULL);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }			
            else //turn-off
            {
                CAPI2_SMS_SetCellBroadcastMsgTypeReq(GetNewTID(), GetClientID(), 1, NULL, NULL);
                pdata->handler_state = BCM_SMS_SetCBOff;
            }
            break;
        }

        case BCM_RemoveCBMI:
        {
            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                KRIL_DEBUG(DBG_INFO, "[SET_CB]KRIL_SRIL_SetCellBroadcastHandler BCM_RemoveCBMI Error state[0x%04X]\n", pdata->handler_state);
                return;
            }
			
            Kril_Cell_Broadcast_config *tdata = (Kril_Cell_Broadcast_config *)pdata->ril_cmd->data;
            UInt8* pchnlIDs = (UInt8 *)(tdata->msgIDs);

            //tdata->bCBEnabled must be 1 at this state. Otherwise it's an error.
            if( tdata->bCBEnabled!=1 )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                KRIL_DEBUG(DBG_INFO, "[SET_CB]KRIL_SRIL_SetCellBroadcastHandler BCM_RemoveCBMI Error tdata->bCBEnabled[0x%04X]\n", tdata->bCBEnabled);
                return;
            }
		    
            //call CAPI2 API to activate the CBMI channel list.  <coding> is always NULL because SRIL RIL_REQUEST_SET_CELL_BROADCAST_CONFIG 
            // doesn't provide it. <mode> must =='0' at this state since we are to activate pchnIDs. 
            CAPI2_SMS_SetCellBroadcastMsgTypeReq(GetNewTID(), GetClientID(), 0, pchnlIDs, NULL);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_SMS_SetCBOff:
        {
            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
	   			KRIL_DEBUG(DBG_INFO, "[SET_CB]KRIL_SRIL_SetCellBroadcastHandler_return: state[0x%04X]\n", pdata->handler_state);
                return;
            }
            CAPI2_SMS_StopReceivingCellBroadcastReq(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
	   			KRIL_DEBUG(DBG_INFO, "[SET_CB]KRIL_SRIL_SetCellBroadcastHandler_return: state[0x%04X]\n", pdata->handler_state);
                return;
            }
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

    KRIL_DEBUG(DBG_INFO, "[SET_CB]KRIL_SRIL_SetCellBroadcastHandler_return: state[0x%04X]\n", pdata->handler_state);

}


void KRIL_SRIL_GetCellBroadcastHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "KRIL_SRIL_GetCellBroadcastHandler_state:0x%04X::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(Kril_Cell_Broadcast_config), GFP_KERNEL);
            pdata->rsp_len = sizeof(Kril_Cell_Broadcast_config);
            CAPI2_SMS_GetCBMI(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_SMS_GetCBMI;

                KRIL_DEBUG(DBG_INFO, "[GET_CB]BCM_SendCAPI2Cmd: pdata->rsp_len[%d]\n", pdata->rsp_len);
            break;
        }

        case BCM_SMS_GetCBMI:
        {
            //UInt8 i, j;
            int i, j;
            Kril_Cell_Broadcast_config *rdata = (Kril_Cell_Broadcast_config *)pdata->bcm_ril_rsp;
            SMS_CB_MSG_IDS_t *rsp = (SMS_CB_MSG_IDS_t *)capi2_rsp->dataBuf;
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);

            if(capi2_rsp->result == 147 /*SMS_CB_MIDS_DOES_NOT_EXIST*/ )
            {
                KRIL_DEBUG(DBG_ERROR, "[GET_CB]BCM_SMS_GetCBMI: SMS_CB_MIDS_DOES_NOT_EXIST(%d)\n", capi2_rsp->result);
				
                rdata->msgIdMaxCount = 50;	//for Phone memory
				
                KRIL_DEBUG(DBG_INFO, "[GET_CB]BCM_RESPCAPI2Cmd: bCBEnabled[%d], msgIdCount[%d], msgIdMaxCount[%d], selectedId[%d]\n", 
				                rdata->bCBEnabled, rdata->msgIdCount, rdata->msgIdMaxCount, rdata->selectedId);

                pdata->handler_state = BCM_FinishCAPI2Cmd;
                break;
            }
            else if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                KRIL_DEBUG(DBG_INFO, "[GET_CB]KRIL_SRIL_GetCellBroadcastHandler_return: state[0x%04X]\n", pdata->handler_state);
                return;
            }

            rdata->msgIdCount = 0;

            // Because SRIL requires an int *msgIDs, convert rdata->msgIDs from char * to int* so that 
            // CAPI2's return list SMS_CB_MSG_IDS_t *rsp can be copied directly to rdata->msgIDs. 
            int * midList = (int *)(rdata->msgIDs);

            KRIL_DEBUG(DBG_INFO, "[GET_CB]BCM_SendCAPI2Cmd: rsp->nbr_of_msg_id_ranges[%d]\n", rsp->nbr_of_msg_id_ranges);

            for(i = 0 ; i < rsp->nbr_of_msg_id_ranges && rdata->msgIdCount < (CHNL_IDS_SIZE/sizeof(int)); i++)
            {
                KRIL_DEBUG(DBG_INFO, "[GET_CB]BCM_SendCAPI2Cmd: A[%d].start_pos= %d, A[%d].stop_pos= %d\n", 
                            i, rsp->msg_id_range_list.A[i].start_pos, i, rsp->msg_id_range_list.A[i].stop_pos);

                if(rsp->msg_id_range_list.A[i].start_pos == rsp->msg_id_range_list.A[i].stop_pos){
                    if( rsp->msg_id_range_list.A[i].start_pos >= 0 && rsp->msg_id_range_list.A[i].start_pos < 1000 ){
                        midList[rdata->msgIdCount] = rsp->msg_id_range_list.A[i].start_pos;
                        //KRIL_DEBUG(DBG_INFO, "[GET_CB]BCM_SendCAPI2Cmd: midList[%d]= %d\n", rdata->msgIdCount, midList[rdata->msgIdCount]);
                        rdata->msgIdCount ++;
                    }

                }else if(rsp->msg_id_range_list.A[i].start_pos < rsp->msg_id_range_list.A[i].stop_pos){
                    for (j = rsp->msg_id_range_list.A[i].start_pos; j <= rsp->msg_id_range_list.A[i].stop_pos; j++)
                    {
                        if( j >= 0 && j < 1000 ){
                            midList[rdata->msgIdCount] = j;
                            //KRIL_DEBUG(DBG_INFO, "[GET_CB]BCM_SendCAPI2Cmd: midList[%d]= %d\n", rdata->msgIdCount, midList[rdata->msgIdCount]);
                            rdata->msgIdCount ++;
                        }
                    }
                }
            }
			
            if(rdata->msgIdCount >= (CHNL_IDS_SIZE/sizeof(int)) ){
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                KRIL_DEBUG(DBG_INFO, "[GET_CB]KRIL_SRIL_GetCellBroadcastHandler_return: state[0x%04X]\n", pdata->handler_state);
                return;
            }

            CAPI2_SMS_RetrieveMaxCBChnlLength(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd :
        {
            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                KRIL_DEBUG(DBG_INFO, "[GET_CB]KRIL_SRIL_GetCellBroadcastHandler_return: state[0x%04X]\n", pdata->handler_state);
                return;
            }

            Kril_Cell_Broadcast_config *rdata = (Kril_Cell_Broadcast_config *)pdata->bcm_ril_rsp;
            // API CAPI2_SMS_RetrieveMaxCBChnlLength returns a (UInt8 *)
            UInt8 * rsp = (UInt8 *)capi2_rsp->dataBuf;
            UInt8 i = 0;
			
            rdata->msgIdMaxCount =  (int)(rsp[0]);
			
            KRIL_DEBUG(DBG_INFO, "[GET_CB]BCM_RESPCAPI2Cmd: bCBEnabled[%d], msgIdCount[%d], msgIdMaxCount[%d], selectedId[%d]\n", 
								rdata->bCBEnabled, rdata->msgIdCount, rdata->msgIdMaxCount, rdata->selectedId);
            for(i = 0; i < rdata->msgIdCount; i++)
                KRIL_DEBUG(DBG_INFO, "[GET_CB]BCM_RESPCAPI2Cmd: msgIDs[%d]=%d\n", i, rdata->msgIDs[i]);

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
	
    KRIL_DEBUG(DBG_INFO, "[GET_CB]KRIL_SRIL_GetCellBroadcastHandler_return: state[0x%04X]\n", pdata->handler_state);

}

#if 1


//******************************************************************************
//
// Function Name:	KRIL_USSDSeptet2Octet
//
// Description:	This function converts Septet formatted data to Octet format
//				in USSD.
//				This function returns the number of bytes that was required
//				storage the convert data.
//
// Notes:
//
//******************************************************************************

UInt16 KRIL_USSDSeptet2Octet(
	UInt8 *p_src,
	UInt8 *p_dest,
	UInt16 num_of_Septets
	)
{
	UInt16			num_of_Octets;
	UInt16			avail_cha_cnt;
	UInt8			septet_mod;
	UInt8			*p_temp;


	num_of_Octets = (num_of_Septets * 8) / 7;
	septet_mod = 0;
	avail_cha_cnt = 0;
	p_temp = p_dest;
	for(; num_of_Octets > 0; num_of_Octets-- )
	{
		switch( septet_mod )
		{
			case 0:
				*p_temp = (p_src[0] & 0x7F);
				break;
			case 1:
				*p_temp = (((p_src[0] & 0x80) >> 7) | ((p_src[1] & 0x3F) << 1));
				break;
			case 2:
				*p_temp = (((p_src[1] & 0xC0) >> 6) | ((p_src[2] & 0x1F) << 2));
				break;
			case 3:
				*p_temp = (((p_src[2] & 0xE0) >> 5) | ((p_src[3] & 0x0F) << 3));
				break;
			case 4:
				*p_temp = (((p_src[3] & 0xF0) >> 4) | ((p_src[4] & 0x07) << 4));
				break;
			case 5:
				*p_temp = (((p_src[4] & 0xF8) >> 3) | ((p_src[5] & 0x03) << 5));
				break;
			case 6:
				*p_temp = (((p_src[5] & 0xFC) >> 2) | ((p_src[6] & 0x01) << 6));
				break;
			case 7:
				*p_temp = ((p_src[6] & 0xFE) >> 1);
				break;
		}
		p_temp++;
		avail_cha_cnt++;
		if(septet_mod == 7)	p_src = &p_src[7];
		septet_mod = ((septet_mod + 1) & 0x07);
	}

	// per GSM03.38, if the last 7bit char is on 8bit boundary and it is 
	// a <CR> char, it will need to be removed. 
	if( (septet_mod == 0) && (*(p_temp-1) == 0x0D) ){
		KRIL_DEBUG(DBG_INFO, "USSD: 7bit boundary matches 8bit boundary, extra <CR> removed..%d", p_temp);
		p_temp--;
		avail_cha_cnt--;
	}
	KRIL_DEBUG(DBG_INFO, "avail_cha_cnt:0x%lX \n", avail_cha_cnt);
	// reuse the variable 'num_of_septets'
	num_of_Octets = avail_cha_cnt;
	// now convert all Default GSM Alphabet '@' to MTI version
	// 0x00 -> CODE_PAGE_MTI_AT_SIGN
	for(p_temp = p_dest; num_of_Octets > 0; num_of_Octets--, p_temp++ )
	{
		if( *p_temp == 0x00 )
		{
			*p_temp = 0x80;
		}
	}

	// NULL terminate the string
	*p_temp = 0x00;
	return( avail_cha_cnt );
}


/// Enum : Coding group : "Data Coding Scheme GSM 3.38, section 5"
typedef enum
{
	CODING_GROUP_0_LANGUAGE_GSM_7_BIT_ALPHABET,			///< German, English, Italian ....
	CODING_GROUP_1_MESSAGE_PROTECTED_BY_LANGUAGE_IND,	///< GSM 7 Bit Default
	CODING_GROUP_2_LANGUAGE_GSM_7_BIT_ALPHABET,			///< Czech, Hebrew, Arabic, ....
	CODING_GROUP_3_LANGUAGE_GSM_7_BIT_ALPHABET,			///< Languages using the GSM 7 bit default alphabet
	CODING_GROUP_4_GENERAL_TEXT_UNCOMPRESSED = 0x04,	///< General, Text Uncompressed
	CODING_GROUP_5_GENERAL_TEXT_UNCOMPRESSED,			///< General, Text Uncompressed, have message class meaning
	CODING_GROUP_6_GENERAL_TEXT_COMPRESSED,				///< General, Text Compressed
	CODING_GROUP_7_GENERAL_TEXT_COMPRESSED,				///< General, Text Compressed, have message class meaning
	CODING_GROUP_9_MSG_WITH_USER_DATA_HEADER = 0x09,	///< Message with User Data Header (UDH)
	CODING_GROUP_E_WAP_RELATED = 0x0E,					///< Refer to "Wireless Datagram Protocol Specification", Wireless Application Protocol Forum Ltd.
	CODING_GROUP_F_DATA_CODING_MSG_HANDLING				///< Data Coding/Message Handling
} CodingGroup_t;										///< SS Data Coding Scheme Type

/**
Enum :	Alphabet: "Data Codin Scheme GSM 3.38, section 5"
		Coding group 4 : CODING_GROUP_4_GENERAL_TEXT_UNCOMPRESSED
		Coding group 5 : CODING_GROUP_5_GENERAL_TEXT_UNCOMPRESSED
		Coding group 6 : CODING_GROUP_6_GENERAL_TEXT_COMPRESSED
		Coding group 7 : CODING_GROUP_7_GENERAL_TEXT_COMPRESSED
		Coding group 9 : CODING_GROUP_9_MSG_WITH_USER_DATA_HEADER
**/
typedef enum
{
	CHARACTER_SET_GSM_7_BIT_DEFAULT,			///< GSM 7 bit default alphabet
	CHARACTER_SET_8_BIT_DATA,					///< 8 bit data
	CHARACTER_SET_UCS2_16_BIT,					///< UCS2 16 bit
	CHARACTER_SET_RESERVED						///< Reserved
} CharacterSet_t;								///< Character Set


//******************************************************************************
// Function Name:	Util_GetCharacterSet()
//
// Description:		This function returns Character Set. Refer to GSM 3.38
// Arguments:		inDcs (in) Data Coding Scheme
//******************************************************************************
CharacterSet_t Util_GetCharacterSet(UInt8 inDcs)
{
	CharacterSet_t charSet = CHARACTER_SET_8_BIT_DATA;

	switch (inDcs >> 4) //Coding Group, refere to CodingGroup_t
	{
		case CODING_GROUP_0_LANGUAGE_GSM_7_BIT_ALPHABET:
		case CODING_GROUP_2_LANGUAGE_GSM_7_BIT_ALPHABET:
		case CODING_GROUP_3_LANGUAGE_GSM_7_BIT_ALPHABET:
			charSet = CHARACTER_SET_GSM_7_BIT_DEFAULT;
			break;

		case CODING_GROUP_1_MESSAGE_PROTECTED_BY_LANGUAGE_IND:
			if (inDcs & 0x01)	//UCS2
			{
				charSet = CHARACTER_SET_UCS2_16_BIT;
			}
			else //GSM 7 bit default alphabet
			{
				charSet = CHARACTER_SET_GSM_7_BIT_DEFAULT;
			}
			break;

		case CODING_GROUP_4_GENERAL_TEXT_UNCOMPRESSED:
		case CODING_GROUP_5_GENERAL_TEXT_UNCOMPRESSED:
		case CODING_GROUP_9_MSG_WITH_USER_DATA_HEADER:
			charSet = (CharacterSet_t)((inDcs >> 2) & 0x03);
			break;

		case CODING_GROUP_6_GENERAL_TEXT_COMPRESSED:
		case CODING_GROUP_7_GENERAL_TEXT_COMPRESSED:
			break;

		case CODING_GROUP_E_WAP_RELATED:
			break;

		case CODING_GROUP_F_DATA_CODING_MSG_HANDLING:
			if (!((inDcs >> 2) & 0x01))
			{
				charSet = CHARACTER_SET_GSM_7_BIT_DEFAULT;
			}
			break;

		default:
			charSet = CHARACTER_SET_RESERVED;
			break;
	}

	return(charSet);
}

void KRIL_SRIL_SendEncodeUSSDHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilSendUSSDInfo_t *tdata = (KrilSendUSSDInfo_t *)pdata->ril_cmd->data;
			USSDString_t *ussd = kmalloc(sizeof(USSDString_t), GFP_KERNEL);
			memset (ussd, 0x00, sizeof(USSDString_t));
            if(tdata->StringSize <= 0)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
				kfree(ussd);
                return;
            }
			
            KRIL_DEBUG(DBG_INFO, "pdata->ril_cmd->CmdID:%d pdata->cmd 0x%0x dcs 0x%0x\n",pdata->ril_cmd->CmdID, pdata->cmd, tdata->dcs);
            KRIL_DEBUG(DBG_INFO, "gUssdID:%d m_USSDString:%s used_size:%d\n", gUssdID, tdata->USSDString, tdata->StringSize);
            if(gUssdID == CALLINDEX_INVALID) //new USSD request
            {
                pdata->bcm_ril_rsp = kmalloc(sizeof(KrilReceiveUSSDInfo_t), GFP_KERNEL);
                pdata->rsp_len = sizeof(KrilReceiveUSSDInfo_t);
                memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
                ussd->used_size = tdata->StringSize;
				ussd->dcs = tdata->dcs;

				if(Util_GetCharacterSet(tdata->dcs) == CHARACTER_SET_GSM_7_BIT_DEFAULT){
					ussd->used_size = KRIL_USSDSeptet2Octet(tdata->USSDString,ussd->string,tdata->StringSize);
					KRIL_DEBUG(DBG_INFO, "KRIL_SendUSSDHandler USSDSeptet2Octet : %d m_USSDString:%s used_size:%d\n", gUssdID, ussd->string, ussd->used_size);
				}else{
               		memcpy((void*)ussd->string, (void*)tdata->USSDString, ussd->used_size);
				}
				
				CAPI2_SS_SendUSSDConnectReq(GetNewTID(), GetClientID(), ussd);
            }
            else
            {
	            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilReceiveUSSDInfo_t), GFP_KERNEL);
                pdata->rsp_len = sizeof(KrilReceiveUSSDInfo_t);
                memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
				if(Util_GetCharacterSet(tdata->dcs) == CHARACTER_SET_GSM_7_BIT_DEFAULT){
					ussd->used_size = KRIL_USSDSeptet2Octet(tdata->USSDString,ussd->string,tdata->StringSize);
					CAPI2_SS_SendUSSDData(GetNewTID(), GetClientID(), gUssdID, tdata->dcs, ussd->used_size, (UInt8 *) ussd->string);
					KRIL_DEBUG(DBG_INFO, "KRIL_SendUSSDHandler USSDSeptet2Octet : %d m_USSDString:%s used_size:%d\n", gUssdID, ussd->string, ussd->used_size);
											
				}else
                	CAPI2_SS_SendUSSDData(GetNewTID(), GetClientID(), gUssdID, tdata->dcs, tdata->StringSize, (UInt8 *) tdata->USSDString);
            }
			kfree(ussd);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                int i;
                USSDataInfo_t *rsp = (USSDataInfo_t *) capi2_rsp->dataBuf;
                KrilReceiveUSSDInfo_t *rdata = (KrilReceiveUSSDInfo_t *) pdata->bcm_ril_rsp;

                KRIL_DEBUG(DBG_INFO, "call_index:%d service_type:%d oldindex:%d newindex:%d prob_tag:%d prob_code:%d err_code:%d code_type:0x%x used_size:%d\n", rsp->call_index,rsp->ussd_data.service_type,rsp->ussd_data.oldindex,rsp->ussd_data.newindex,rsp->ussd_data.prob_tag,
                rsp->ussd_data.prob_code,rsp->ussd_data.err_code,rsp->ussd_data.code_type,rsp->ussd_data.used_size);

                if (USSD_REQUEST == rsp->ussd_data.service_type || 
                	   USSD_REGISTRATION == rsp->ussd_data.service_type)
                {
                    rdata->type = 1;
                }
                else if (USSD_NOTIFICATION == rsp->ussd_data.service_type)
                {
                    rdata->type = 0;
                }
                else if (USSD_RELEASE_COMPLETE_RETURN_RESULT == rsp->ussd_data.service_type)
                {
                    rdata->type = 2;
                    if (rsp->ussd_data.used_size < 0)
                    {
                        pdata->handler_state = BCM_FinishCAPI2Cmd;
                        return;
                    }
                }
                else if (USSD_FACILITY_RETURN_RESULT == rsp->ussd_data.service_type)
                {
                    if (rsp->ussd_data.used_size > 0)
                    {
                        rdata->type = 1;
                    }
                    if (rsp->ussd_data.used_size < 0)
                    {
                        rdata->type = 0;
                        memset(rdata->USSDString, 0, PHASE1_MAX_USSD_STRING_SIZE+1);
                        pdata->handler_state = BCM_FinishCAPI2Cmd;
                return;
            }
                }
                else if (USSD_FACILITY_REJECT == rsp->ussd_data.service_type ||
                            USSD_RELEASE_COMPLETE_REJECT == rsp->ussd_data.service_type)
                {
                    gUssdID = CALLINDEX_INVALID;
                    rdata->type = 4;
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                    return;
                }
                else if (USSD_FACILITY_RETURN_ERROR == rsp->ussd_data.service_type ||
                            USSD_RELEASE_COMPLETE_RETURN_ERROR == rsp->ussd_data.service_type)
                {
                    gUssdID = CALLINDEX_INVALID;
                    rdata->type = 5;
                }
                else if (USSD_RESEND == rsp->ussd_data.service_type)
                {
                    if (gUssdID == rsp->ussd_data.oldindex)
                    {
                        KrilSendUSSDInfo_t *tdata = (KrilSendUSSDInfo_t *)pdata->ril_cmd->data;
                        gUssdID = rsp->ussd_data.newindex;
                        CAPI2_SS_SendUSSDData(GetNewTID(), GetClientID(), gUssdID, 0x0F, tdata->StringSize, (UInt8 *) tdata->USSDString);
                    }
            else
            {
                        gUssdID = CALLINDEX_INVALID;
                        gPreviousUssdID = CALLINDEX_INVALID;
                        pdata->handler_state = BCM_ErrorCAPI2Cmd;
                   }
                    return;
                }
                rdata->Length = rsp->ussd_data.used_size;
                rdata->codetype = rsp->ussd_data.code_type;

                for (i = 0 ; i < rdata->Length ; i++)
                    KRIL_DEBUG(DBG_TRACE2, "string:0x%x\n", rsp->ussd_data.string[i]);

                memset(rdata->USSDString, 0, PHASE1_MAX_USSD_STRING_SIZE+1);
                memcpy(rdata->USSDString, rsp->ussd_data.string, rdata->Length);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}
#endif
void KRIL_SRIL_PhoneBookStorageInfoHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    KRIL_DEBUG(DBG_ERROR,"pdata->handler_state:0x%lX\n", pdata->handler_state);

    if ((capi2_rsp == NULL)&& (pdata->handler_state==BCM_RESPCAPI2Cmd))
    {
        KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_STORAGE_INFO: CAPI2 response failed: capi2_rsp is empty");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }

    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            SRIL_Phonebk_Storage_Info_request *cmd_data = (SRIL_Phonebk_Storage_Info_request *)pdata->ril_cmd->data;
            PBK_Id_t pbk_id;

            pdata->bcm_ril_rsp = kmalloc(sizeof(RIL_Phonebk_Storage_Info), GFP_KERNEL);

            if (!pdata->bcm_ril_rsp)
            {
                KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            pdata->rsp_len = sizeof(RIL_Phonebk_Storage_Info) ;
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);

            // Samsung to Broadcom phonebook storage type mapping
            // FixMe: need to avoid use hard-coded constant such as 0x01 by adding Samsung PB_DC etc into ril.h
            // FixMe: not a good practice to do in-place mapping. An additional data structure shall be defined
            switch (cmd_data->data)
            {
                case 0x02:
                //PB_EN    0x02 SIM(or ME) emergency number
                //PB_EN----> PB_EN
                    pbk_id = PB_EN;
                    break;
                case 0x03:
                //PB_FD    0x03 SIM fixed-dialing phonebook
                //PB_FD-----> PB_FDN
                    pbk_id = PB_FDN;
                    break;
                case 0x04:
                //PB_LD    0x04 SIM last-dialing phonebook
                //PB_LD----> PB_LND
                    pbk_id = PB_LND;
                    break;
                    break;
                    break;
                case 0x08:
                //PB_ON    0x08 SIM(or ME) own numbers ( MSISDNs) list
                //PB_ON----> PB_MSISDN
                    pbk_id = PB_MSISDN;
                    break;
                    break;
                case 0x0A:
                //PB_SIM   0x0A 2G SIM phonebook
                //PB_SIM ---> PB_ADN
                    pbk_id = PB_ADN;
                    break;
                case 0x0B:
                //PB_SDN   0x0B Service Dialing Number
                //PB_SDN ----> PB_SDN
                    pbk_id = PB_SDN;
                    break;
                case 0x0C:        //FixMe
                //PB_3GSIM 0x0C 3G SIM phonebook
                //PB_3GSIM ----> PB_3G_GLOBAL or PB_3G_LOCAL.
                //You have to see global or local pbk exists to decide based on response to PbkApi_SendInfoReq() API
                    pbk_id = PB_3G_LOCAL;
                    break;

                case 0x01:
                //PB_DC    0x01 ME dialed calls list stored in NV
                //PB_DC-----> not supported. ME PBK is supported by AP MMI.
                case 0x05:
                //PB_MC    0x05 ME missed calls list
                //PB_MC-----> not supported
                case 0x06:    // FixMe
                //PB_ME    0x06 ME phonebook
                //PM_ME----> not supported. ME PBK is supported by AP MMI.
                case 0x07:
                //PB_MT    0x07 Combined ME and SIM phonebook
                //PB_MT---->not supported
                case 0x09:
                //PB_RC    0x09 ME received calls list stored in NV
                //PB_RC--->not supported
                case 0x0D:
                //PB_ICI   0x0D Incoming Call Information
                //PB_ICI ----> not supported
                case 0x0E:
                //PB_OCI   0x0E Outgoing Call Information
                //PB_OCI-----> not supported
                case 0x0F:    // FixMe
                //PB_AAS   0x0F Additional Number Alpha String
                //PB_AAS and PB_GAS  -----> not supported directly. Supported indirectly through PB_3G_GLOBAL or PB_3G_LOCAL.
                case 0x10:    // FixMe
                //PB_GAS   0x10 Grouping Information Alpha String
                //PB_AAS and PB_GAS  -----> not supported directly. Supported indirectly through PB_3G_GLOBAL or PB_3G_LOCAL.
                default:
                // FixMe: add code here for handling cases not supported by CP
                    KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_STORAGE_INFO: cases not supported by CP!\n");
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
        }

            // handle those supported by CP
            CAPI2_PBK_SendInfoReq(GetNewTID(), GetClientID(), pbk_id);

            pdata->handler_state = BCM_RESPCAPI2Cmd;
        break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            RIL_Phonebk_Storage_Info *ril_rsp = (RIL_Phonebk_Storage_Info *)pdata->bcm_ril_rsp;;
            PBK_INFO_RSP_t *rsp = (PBK_INFO_RSP_t *)capi2_rsp->dataBuf;

            if (MSG_GET_PBK_INFO_RSP != capi2_rsp->msgType)
            {
                KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_STORAGE_INFO: Receive error MsgType:0x%x...!\n", capi2_rsp->msgType);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            if (rsp == NULL)
            {
                KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_STORAGE_INFO: capi2_rsp->dataBuf is empty, no information available...!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }

            ril_rsp->total = (int)rsp->total_entries;
            
            if((int)rsp->free_entries > (int)rsp->total_entries)
			ril_rsp->used = 0;
            else
	            ril_rsp->used = (int)(rsp->total_entries - rsp->free_entries);
	            
            ril_rsp->first_id = (int)rsp->first_used_entry ;	//HJKIM_ADN

            if(rsp->mx_alpha_size>40)
            	ril_rsp->max_text = 40;	
            else if ( rsp->mx_alpha_size < 0)
            	ril_rsp->max_text = 0;
            else
            	ril_rsp->max_text = (int)rsp->mx_alpha_size;

            ril_rsp->max_num = (int)rsp->mx_digit_size;
            

            pdata->handler_state = BCM_FinishCAPI2Cmd;
        break;
        }

        default:
        {
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

void KRIL_SRIL_GetPhoneBookEntryHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    KRIL_DEBUG(DBG_ERROR,"pdata->handler_state:0x%lX\n", pdata->handler_state);

    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {

            RIL_SIM_IO *cmd_data = (RIL_SIM_IO *)pdata->ril_cmd->data;

            UInt16 start_index = cmd_data->p1;

            UInt16 end_index = 0;

            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilPhonebookGetEntry_t ), GFP_KERNEL);
		pdata->rsp_len = sizeof(KrilPhonebookGetEntry_t );
		end_index = start_index;
            PBK_Id_t pbk_id = fileid_to_pbk_id(cmd_data->fileid);

            if (pbk_id == 0)
            {
                // FixMe: add code here for handling cases not supported by CP
                KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: phonebook storage type %x not supported!!\n", cmd_data->fileid);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
        }

            // int command: one of the commands listed for TS 27.007 +CRSM
            // <command> support only following value in 3GPP TS 11.11 9.1.
            // READ_RECORD = 0xB2 (178)
            // FixMe: need to avoid use hard-coded constant such as 0xB2 by adding READ_RECORD etc into ril.h
            if (cmd_data->command != 0xB2)
            {
                KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: command %x not supported!!\n", cmd_data->command);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }


            // handle those supported by CP
            CAPI2_PBK_SendReadEntryReq(GetNewTID(), GetClientID(), pbk_id, start_index, end_index);

            pdata->handler_state = BCM_RESPCAPI2Cmd;
        break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KrilPhonebookGetEntry_t  *ril_rsp;

		PBK_ENTRY_DATA_RSP_t *rsp = (PBK_ENTRY_DATA_RSP_t *)capi2_rsp->dataBuf;
		PBK_Record_t *pbk_rec = (PBK_Record_t *)&rsp->pbk_rec;
		USIM_PBK_EXT_DATA_t	*pbk_etc_rec = (USIM_PBK_EXT_DATA_t *)&rsp->usim_adn_ext;
		int temp_length = 0;

                KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_GET_PHONEBOOK_ENTRY:BCM_RESPCAPI2Cmd!!\n");
            if (MSG_PBK_ENTRY_DATA_RSP != capi2_rsp->msgType)	//HJKIM_ADN
            {
                KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: Receive error MsgType:0x%x...!\n", capi2_rsp->msgType);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }


            memset(pdata->bcm_ril_rsp, 0,pdata->rsp_len);

            if (!pdata->bcm_ril_rsp)
            {
                KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: Allocate bcm_ril_rsp memory failed!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            


            {
                PBK_ENTRY_DATA_RESULT_t data_result = (PBK_ENTRY_DATA_RESULT_t)rsp->data_result;

            ril_rsp = (KrilPhonebookGetEntry_t  *)pdata->bcm_ril_rsp;


                KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_GET_PHONEBOOK_ENTRY: data_result : %d\n", (int) data_result);


                switch(data_result)
                {
			case PBK_ENTRY_VALID_NOT_LAST:
				ril_rsp->index = pbk_rec->location;
				ril_rsp->next_index = rsp->next_valid_index;    // assuming continuous

                    break;
                    
			case PBK_ENTRY_VALID_IS_LAST:
				KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_GET_PHONEBOOK_ENTRY: PBK_ENTRY_VALID_IS_LAST\n");
				ril_rsp->index = pbk_rec->location;

				if(rsp->next_valid_index == 0 )
				ril_rsp->next_index = 0xffff;    // no next
				else
				ril_rsp->next_index = rsp->next_valid_index ;    

			break;
			
			case PBK_ENTRY_INVALID_IS_LAST:
				KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: PBK_ENTRY_INVALID_IS_LAST!!\n");
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
			return;
			
			case PBK_SIM_BUSY:
				KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: PBK_SIM_BUSY!!\n");
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
			return;
			
			case PBK_ENTRY_INDEX_INVALID:
				KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: PBK_ENTRY_INDEX_INVALID!!\n");
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
			return;
			
			case PBK_ENTRY_NOT_ACCESSIBLE:
				KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: PBK_ENTRY_NOT_ACCESSIBLE!!\n");
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
			return;
			
			default:
				KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: invalid data_result %d!!\n", data_result);
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
			return;
                }
            }
            
            {
		PBK_API_Name_t *alpha_data = (PBK_API_Name_t *)&pbk_rec->alpha_data;

		if((int)alpha_data->alpha_size> PB_ALPHA_MAX-2)
		{
			alpha_data->alpha_size = PB_ALPHA_MAX-2;
			memset(ril_rsp->name, 0, PB_ALPHA_MAX);
		}
		else
			memset(ril_rsp->name, 0, sizeof(alpha_data->alpha) + 2);


                ril_rsp->length_name = (int)alpha_data->alpha_size + 1;


                switch(alpha_data->alpha_coding)
                {
			case ALPHA_CODING_UCS2_80:
				memcpy(ril_rsp->name +1, alpha_data->alpha, alpha_data->alpha_size);
				ril_rsp->name[0] = 0x80;
				temp_length = strlen(alpha_data->alpha);
				if(temp_length > PB_ALPHA_MAX-2 )
					temp_length = PB_ALPHA_MAX-2 ;
				ril_rsp->name[temp_length+1]='\0';
				ril_rsp->name_datatpye = 0x03;
				ril_rsp->length_name = (int)alpha_data->alpha_size + 1;
			break;

			case ALPHA_CODING_UCS2_81:
				memcpy(ril_rsp->name +1, alpha_data->alpha, alpha_data->alpha_size);
				ril_rsp->name[0] = 0x81;

				temp_length = strlen(alpha_data->alpha);
				if(temp_length > PB_ALPHA_MAX-2 )
					temp_length = PB_ALPHA_MAX-2 ;
				ril_rsp->name[temp_length+1]='\0';

				ril_rsp->name_datatpye = 0x03;
				ril_rsp->length_name = (int)alpha_data->alpha_size + 1;
			break;

			case ALPHA_CODING_UCS2_82:
				memcpy(ril_rsp->name +1, alpha_data->alpha, alpha_data->alpha_size);
				ril_rsp->name[0] = 0x82;

				temp_length = strlen(alpha_data->alpha);
				if(temp_length > PB_ALPHA_MAX-2 )
					temp_length = PB_ALPHA_MAX-2 ;
				ril_rsp->name[temp_length+1]='\0';

				ril_rsp->name_datatpye = 0x03;
				ril_rsp->length_name = (int)alpha_data->alpha_size + 1;
			break;

			case ALPHA_CODING_GSM_ALPHABET:
			default:
				memcpy(ril_rsp->name, alpha_data->alpha, ril_rsp->length_name);
				ril_rsp->name[strlen(alpha_data->alpha)]='\0';
				ril_rsp->name_datatpye = 0x02;
				ril_rsp->length_name = (int)alpha_data->alpha_size ;

			break;
                }

            }
//Phone Number            
                {
			int num_len = strlen(pbk_rec->number);
			ril_rsp->length_number = num_len;


			memset( ril_rsp->number, 0, sizeof(num_len) + 1);

			if (ril_rsp->number == NULL)
			{
				KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_GET_PHONEBOOK_ENTRY: Allocate numbers[0] memory failed!!\n");
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
				return;
			}

			memcpy(ril_rsp->number, (char *)pbk_rec->number, num_len);

			}
			KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_GET_PHONEBOOK_ENTRY:pbk_etc_rec->num_of_email: %d\n", pbk_etc_rec->num_of_email);

//Email
	if((pbk_etc_rec->num_of_email >= 1 ) && (pbk_etc_rec->email[0].alpha_size > 0) &&( rsp->pbk_id ==PB_3G_GLOBAL))
            {
                PBK_API_Name_t *email_data = (PBK_API_Name_t *)&pbk_etc_rec->email[0];
		int	iter = 0;
		Boolean email_valid = false ;


		for(iter= 0; iter <  sizeof(email_data->alpha) ; iter++ )
		{
			if(email_data->alpha[iter] != 0x00 )
			{
				email_valid = true;
				break;
			}
			
		}

		if((int)email_data->alpha_size> PB_EMAIL_MAX-2)
		{
			email_data->alpha_size = PB_EMAIL_MAX-2;
			memset(ril_rsp->email, 0,PB_EMAIL_MAX);
		}
		else
			memset(ril_rsp->email, 0, sizeof(email_data->alpha) + 2);

		if( email_valid )
		{
			switch(email_data->alpha_coding)
	                {
				case ALPHA_CODING_UCS2_80:
				memcpy(ril_rsp->email +1, email_data->alpha, email_data->alpha_size);
				ril_rsp->email[0] = 0x80;

				temp_length = strlen(email_data->alpha);
				if(temp_length > PB_EMAIL_MAX-2 )
					temp_length = PB_EMAIL_MAX-2 ;
				ril_rsp->email[temp_length+1]='\0';

				ril_rsp->email_datatpye = 0x03;
				ril_rsp->length_email = (int)email_data->alpha_size + 1;
				break;

				case ALPHA_CODING_UCS2_81:
				memcpy(ril_rsp->email +1, email_data->alpha,  email_data->alpha_size);
				ril_rsp->email[0] = 0x81;

				temp_length = strlen(email_data->alpha);
				if(temp_length > PB_EMAIL_MAX-2 )
					temp_length = PB_EMAIL_MAX-2 ;
				ril_rsp->email[temp_length+1]='\0';

				ril_rsp->email_datatpye = 0x03;
				ril_rsp->length_email = (int)email_data->alpha_size + 1;
				break;

				case ALPHA_CODING_UCS2_82:
				memcpy(ril_rsp->email +1, email_data->alpha, email_data->alpha_size);
				ril_rsp->email[0] = 0x82;

				temp_length = strlen(email_data->alpha);
				if(temp_length > PB_EMAIL_MAX-2 )
					temp_length = PB_EMAIL_MAX-2 ;
				ril_rsp->email[temp_length+1]='\0';

				ril_rsp->email_datatpye = 0x03;
				ril_rsp->length_email = (int)email_data->alpha_size + 1;
				break;

				case ALPHA_CODING_GSM_ALPHABET:
				default:
				memcpy(ril_rsp->email, email_data->alpha, email_data->alpha_size);
				ril_rsp->email[strlen(email_data->alpha)]='\0';
				ril_rsp->email_datatpye = 0x02;
				ril_rsp->length_email = (int)email_data->alpha_size ;

				break;
	            }
            }
            else
            {
		KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_GET_PHONEBOOK_ENTRY:  invalid email\n");
		memset(ril_rsp->email, 0, sizeof(email_data->alpha) + 2);
		ril_rsp->length_email = 0;
		ril_rsp->email_datatpye = 0;


            }
		
        }
            pdata->handler_state = BCM_FinishCAPI2Cmd;

                KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_GET_PHONEBOOK_ENTRY: pdata->handler_state : %d\n", pdata->handler_state);

        break;
        }

        default:
        {
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

void KRIL_SRIL_AccessPhoneBookEnteryHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    KRIL_DEBUG(DBG_ERROR,"pdata->handler_state:0x%lX\n", pdata->handler_state);

    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilPhonebookAccess_t *cmd_data_fdn = (KrilPhonebookAccess_t *)pdata->ril_cmd->data;
		if((cmd_data_fdn->fileid == 0x6F3B)&&(cmd_data_fdn->pin2  != NULL))
		{

	            	CAPI2_SIM_SendVerifyChvReq(GetNewTID(), GetClientID(), CHV2, cmd_data_fdn->pin2);
	            	pdata->handler_state = BCM_SRIL_CHECK_PIN2_FDN;
			break;
		}
		else if((cmd_data_fdn->fileid == 0x6F3B)&&(cmd_data_fdn->pin2  == NULL))
		{
		        KRIL_DEBUG(DBG_ERROR,"HJKIM > CAPI2 KRIL_SRIL_AccessPhoneBookEnteryHandler pincheck:else if case \n" );
	                pdata->handler_state = BCM_ErrorCAPI2Cmd;
	                return;
		}
		        KRIL_DEBUG(DBG_ERROR,"HJKIM > CAPI2 KRIL_SRIL_AccessPhoneBookEnteryHandler pincheck:go Through \n" );

        }
        
        case BCM_SRIL_CHECK_PIN2_FDN:
        {

            KrilPhonebookAccess_t *cmd_data = (KrilPhonebookAccess_t *)pdata->ril_cmd->data;

            // CAPI2_PBK_SendUpdateEntryReq parameters
            Boolean special_fax_num = false;
            UInt16 index = cmd_data->index;
            UInt8 type_of_number = UNKNOWN_TON_UNKNOWN_NPI; // FixMe
		int i=0;

            ALPHA_CODING_t alpha_coding = (cmd_data->alphaTagDCS == 0x03) ? ALPHA_CODING_UCS2_80 : ALPHA_CODING_GSM_ALPHABET;
            ALPHA_CODING_t emailalpha_coding = (cmd_data->emailTagDCS == 0x03) ? ALPHA_CODING_UCS2_80 : ALPHA_CODING_GSM_ALPHABET;

            UInt8 alpha_size = cmd_data->alphaTagLength;
            UInt8 num_length = cmd_data->numberLength;

            USIM_PBK_EXT_DATA_t *usim_adn_ext_data = NULL;    // FixMe


            PBK_Id_t pbk_id = fileid_to_pbk_id(cmd_data->fileid);
		pdata->bcm_ril_rsp = kmalloc(sizeof(int), GFP_KERNEL);
		if (pbk_id == 0)
		{
			// FixMe: add code here for handling cases not supported by CP
			KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: phonebook storage type %x not supported!!\n", cmd_data->fileid);
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			return;
		}



            switch(cmd_data->command)
            {

            case 0x01:	//ADD
                index = FIRST_PHONEBK_FREE_INDEX;

	//	alpha_coding = ALPHA_CODING_UCS2_80;

		if(cmd_data->emailLength > 0)
		{
			usim_adn_ext_data = kmalloc(sizeof(USIM_PBK_EXT_DATA_t),GFP_KERNEL);
			memset(usim_adn_ext_data , 0x00 , sizeof(USIM_PBK_EXT_DATA_t));
			usim_adn_ext_data->num_of_email=1;
			usim_adn_ext_data->email[0].alpha_size = cmd_data->emailLength;
			usim_adn_ext_data->email[0].alpha_coding = emailalpha_coding;
			memcpy(usim_adn_ext_data->email[0].alpha , cmd_data->email , cmd_data->emailLength);


			
		}
		else
		{
	                KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: EDIT :email deleted: %d \n");

			usim_adn_ext_data = kmalloc(sizeof(USIM_PBK_EXT_DATA_t),GFP_KERNEL);
			memset(usim_adn_ext_data , 0x00 , sizeof(USIM_PBK_EXT_DATA_t));
			usim_adn_ext_data->num_of_email=0;
			usim_adn_ext_data->email[0].alpha_size = 0;
			usim_adn_ext_data->email[0].alpha_coding = 0;
		}
		CAPI2_PBK_SendUpdateEntryReq(GetNewTID(), GetClientID(), pbk_id,
			special_fax_num, index, type_of_number,
			(char *)cmd_data->number, alpha_coding, alpha_size, (UInt8 *)cmd_data->alphaTag,
			usim_adn_ext_data);



            pdata->handler_state = BCM_RESPCAPI2Cmd;

                break;

            case 0x02:	//Delete
                KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: Delete \n");
                KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: index %d \n", index);


                KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: DELETE  \n");
 
		usim_adn_ext_data = kmalloc(sizeof(USIM_PBK_EXT_DATA_t),GFP_KERNEL);
		memset(usim_adn_ext_data , 0x00 , sizeof(USIM_PBK_EXT_DATA_t));
		usim_adn_ext_data->num_of_email=0;
		usim_adn_ext_data->email[0].alpha_size = 0;
		usim_adn_ext_data->email[0].alpha_coding = 0;
           
            CAPI2_PBK_SendUpdateEntryReq(GetNewTID(), GetClientID(), pbk_id,
                special_fax_num, index, 0,
                NULL, 0, 0, NULL,
                usim_adn_ext_data);



            pdata->handler_state = BCM_RESPCAPI2Cmd;

                break;

            case 0x03:	//EDIT
	//	alpha_coding = ALPHA_CODING_UCS2_80;

		if(cmd_data->emailLength > 0)
		{
			usim_adn_ext_data = kmalloc(sizeof(USIM_PBK_EXT_DATA_t),GFP_KERNEL);
			memset(usim_adn_ext_data , 0x00 , sizeof(USIM_PBK_EXT_DATA_t));
			usim_adn_ext_data->num_of_email=1;
			usim_adn_ext_data->email[0].alpha_size = cmd_data->emailLength;
			usim_adn_ext_data->email[0].alpha_coding = emailalpha_coding;
			memcpy(usim_adn_ext_data->email[0].alpha , cmd_data->email , cmd_data->emailLength);
		}
		else
		{
	                KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: EDIT :email deleted: %d \n");

			usim_adn_ext_data = kmalloc(sizeof(USIM_PBK_EXT_DATA_t),GFP_KERNEL);
			memset(usim_adn_ext_data , 0x00 , sizeof(USIM_PBK_EXT_DATA_t));
			usim_adn_ext_data->num_of_email=0;
			usim_adn_ext_data->email[0].alpha_size = 0;
			usim_adn_ext_data->email[0].alpha_coding = 0;
		}


            CAPI2_PBK_SendUpdateEntryReq(GetNewTID(), GetClientID(), pbk_id,
                special_fax_num, index, type_of_number,
                (char *)cmd_data->number, alpha_coding, alpha_size, (UInt8 *)cmd_data->alphaTag,
                usim_adn_ext_data);



            pdata->handler_state = BCM_RESPCAPI2Cmd;

                /* EDIT   = 0x03: */
                break;
            default:
                KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: command %x not supported!!\n", cmd_data->command);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }


        break;
        }

        case BCM_RESPCAPI2Cmd:
        {
		int *ril_rsp;
		int *rsp;
		PBK_WRITE_ENTRY_RSP_t *rsp_result;
		
		if (MSG_WRT_PBK_ENTRY_RSP != capi2_rsp->msgType)
		{
			KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: Receive error MsgType:0x%x...!\n", capi2_rsp->msgType);
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			return;
		}

            
/*
		if (NULL == pdata->bcm_ril_rsp)
		{
			KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: Allocate bcm_ril_rsp memory failed!!\n");
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			return;
		}
*/
		rsp_result = (PBK_WRITE_ENTRY_RSP_t *)capi2_rsp->dataBuf;
		if (rsp_result == NULL || rsp_result->write_result != PBK_WRITE_SUCCESS)
		{
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			return;
		}	
		rsp =&(rsp_result->index);

            pdata->rsp_len = sizeof(int) ;
            ril_rsp = (int *)pdata->bcm_ril_rsp;
	     *ril_rsp = *rsp;

            pdata->handler_state = BCM_FinishCAPI2Cmd;
		KRIL_DEBUG(DBG_ERROR,"HJKIM > RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY: rsp [0] : %d!\n",rsp[0]);
        break;


        }

        default:
        {
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}



void KRIL_SRIL_DialVideoCallHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

void KRIL_SRIL_CallDeflectionHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


extern UInt16 Select_Mf_Path;
#define SIM_GetMfPathLen() ( sizeof(Select_Mf_Path) / sizeof(UInt16) )
#define SIM_GetMfPath() (&Select_Mf_Path)

extern void CAPI2_SMS_GetSMSStorageStatus(UInt32 tid, UInt8 clientId, SmsStorage_t storageType);
extern void CAPI2_MS_GetElement(UInt32 tid, UInt8 clientId, MS_Element_t inElemType);

static unsigned char sms_rec_no = 0; 

void KRIL_SRIL_ReadSMSFromSIMHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

	if (capi2_rsp != NULL)
	{
		 KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
		 if(capi2_rsp->result != RESULT_OK)
		 {
			 pdata->result = RILErrorResult(capi2_rsp->result);
			 pdata->handler_state = BCM_ErrorCAPI2Cmd;
			 return;
		 }
	}

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int *recno = (int *)(pdata->ril_cmd->data);
			pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSMSfromSIM), GFP_KERNEL);
			if (NULL == pdata->bcm_ril_rsp)
			{
					KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_READ_SMS_FROM_SIM: Allocate bcm_ril_rsp memory failed!!\n");
					pdata->handler_state = BCM_ErrorCAPI2Cmd;
					return;
			}
			pdata->rsp_len = sizeof(KrilSMSfromSIM);
			memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
			sms_rec_no = *recno;
			KRIL_DEBUG(DBG_ERROR,"[READ_SMS] sms_rec_no: %d\n", sms_rec_no);
		    CAPI2_SIM_SendRecordEFileReadReq(GetNewTID() ,GetClientID(), APDUFILEID_EF_SMS, (KRIL_GetSimAppType() == SIM_APPL_2G)?APDUFILEID_DF_TELECOM : APDUFILEID_USIM_ADF, *recno, SIM_SMS_REC_LEN, SIM_GetMfPathLen(), SIM_GetMfPath());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
		{
			
			SIM_EFILE_DATA_t* pSimResult = (SIM_EFILE_DATA_t*)capi2_rsp->dataBuf;

			if (pSimResult->result == SIMACCESS_SUCCESS)
			{
			   	KrilSMSfromSIM *rdata = (KrilSMSfromSIM *)pdata->bcm_ril_rsp;
				int i = 0, index = 0;

				memset(rdata->pdu, 0, sizeof(rdata->pdu));
				for(i=1;i<(int)pSimResult->data_len;i++)
				{
					sprintf((char *)&rdata->pdu[index++*2],"%02x",(unsigned char)pSimResult->ptr[i]);
				}
				rdata->length = pSimResult->data_len*2;
				
			    KRIL_DEBUG(DBG_ERROR, "[READ_SMS2] rdata->length: %d, rdata->pdu: %s, \n", rdata->length, rdata->pdu);

				if( pSimResult->ptr[1] == 0xff )
				{
					SetSMSMesgStatus(sms_rec_no, SIMSMSMESGSTATUS_FREE);
				}
				else
				{
					SetSMSMesgStatus(sms_rec_no, SIMSMSMESGSTATUS_UNREAD);
				}
				pdata->handler_state = BCM_FinishCAPI2Cmd;
			}
			else
			{
			    pdata->result = RILErrorSIMResult(pSimResult->result);
			    KRIL_DEBUG(DBG_ERROR, "pSimResult: %d\n", pSimResult->result);
			    pdata->handler_state = BCM_ErrorCAPI2Cmd;
			}
        }
		break;
		
        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

#define BCM_SendCAPI2Cmd_Continue	0x01
PBK_Id_t g_pbk_id = PB_3G_GLOBAL;	// start from the first
void KRIL_SRIL_USIM_PB_CAPAHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
    PBK_Id_t pbk_id;

    KRIL_DEBUG(DBG_ERROR,"pdata->handler_state:0x%lX\n", pdata->handler_state);

    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
			g_pbk_id = PB_3G_GLOBAL;	// start from the first and circle through all 13 defined by Samsung
			pbk_id = g_pbk_id;	// start from the first and circle through all 13 defined by Samsung
			
			pdata->bcm_ril_rsp = kmalloc(sizeof(RIL_Usim_PB_Capa), GFP_KERNEL);
			if (NULL == pdata->bcm_ril_rsp)
			{
				KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_USIM_PB_CAPA: Allocate bcm_ril_rsp memory failed!!\n");
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
				return;
			}
            memset(pdata->bcm_ril_rsp, 0, sizeof(RIL_Usim_PB_Capa));
            pdata->rsp_len = sizeof(RIL_Usim_PB_Capa) ;
			CAPI2_PBK_SendInfoReq(GetNewTID(), GetClientID(), pbk_id);
			pdata->handler_state = BCM_RESPCAPI2Cmd;
			break;
			// break;	// it's not a missing. no break; hear
		}
        case BCM_RESPCAPI2Cmd:
        {
			UInt8 i,j;
			UInt8  a,b;
			PBK_INFO_RSP_t *rsp_result;
			USIM_PBK_INFO_t		*rsp;
			USIM_PBK_CONFIG_LIST_t *rsp_config_info;
			RIL_Usim_PB_Capa *ril_rsp;
			USIM_PBK_CONFIG_t* pbk_config;
            if (MSG_GET_PBK_INFO_RSP != capi2_rsp->msgType)
            {
                KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_USIM_PB_CAPA: Receive error MsgType:0x%x...!\n", capi2_rsp->msgType);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
			rsp_result = (PBK_INFO_RSP_t *)capi2_rsp->dataBuf;
			if (rsp_result == NULL)
			{
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
				return;
			}	
			rsp =&(rsp_result->usim_adn_info);
			rsp_config_info = (USIM_PBK_CONFIG_LIST_t *)(&(rsp->config_info));
			ril_rsp = (RIL_Usim_PB_Capa *)pdata->bcm_ril_rsp;
			pbk_config = &rsp_config_info->configInfo[0][0];


		if(rsp_result->usim_adn_info_exist)
		{
			for (i = 0; i < MAX_3GPP_TYPE ; i++)
			{
				ril_rsp->response[i][0] = Usim_pbkid_to_int(pbk_config[i].pbkId);
				for(j=0;j<MAX_NUM_OF_ADN_SET;j++)
					ril_rsp->response[i][1] += rsp_config_info->configInfo[j][i].numOfTotalEntry ;//(int)pbk_config[j][i].numOfTotalEntry;
				if(ril_rsp->response[i][0]  ==  0x04 )	//Temporary_gcf
				{
					if (pbk_config[i].entryLength > 40 )
						ril_rsp->response[i][2] = 40;
					else
						ril_rsp->response[i][2] = (int)pbk_config[i].entryLength;
				}
				else
				ril_rsp->response[i][2] = (int)pbk_config[i].entryLength;
				for(j=0;j<MAX_NUM_OF_ADN_SET;j++)
					ril_rsp->response[i][3] += rsp_config_info->configInfo[j][i].numOfUsedEntry;//(int)pbk_config[j][i].numOfUsedEntry;
			}
		}
		else
		{

	                KRIL_DEBUG(DBG_ERROR,"RIL_REQUEST_USIM_PB_CAPA: Usim without ADN case ");


			for (i = 0; i < MAX_3GPP_TYPE ; i++)
			{
				ril_rsp->response[i][0] =i+1;
				if(i==0)		//name 
				{
					ril_rsp->response[i][1]=(int)rsp_result->total_entries;	//totoal
					if((int)rsp_result->mx_alpha_size >40)
					ril_rsp->response[i][2]=	40;//length
					else
					ril_rsp->response[i][2]=	(int)rsp_result->mx_alpha_size;//length
					ril_rsp->response[i][3]=	 (int)(rsp_result->total_entries - rsp_result->free_entries);//used
				}
				else if(i ==1 )		//number
				{
					ril_rsp->response[i][1]=	(int)rsp_result->total_entries;//totoal
					ril_rsp->response[i][2]=	(int)rsp_result->mx_digit_size;//length
					ril_rsp->response[i][3]=	 (int)(rsp_result->total_entries - rsp_result->free_entries);//used
				}
				else
				{
					for(j=0 ; j<MAX_DATA_LEN;j++)
					{
						ril_rsp->response[i][j] =0;
					}
				}
				
			}
	
			}
			pdata->handler_state = BCM_FinishCAPI2Cmd;
        break;
        }

        default:
        {
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}



extern void CAPI2_MS_GetElement(UInt32 tid, UInt8 clientId, MS_Element_t inElemType);

void KRIL_SRIL_GetStorageMSGCountHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilStoredMsgCount), GFP_KERNEL);
            if (!pdata->bcm_ril_rsp)
            {
                KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            memset(pdata->bcm_ril_rsp, 0, sizeof(KrilStoredMsgCount));
            pdata->rsp_len = sizeof(KrilStoredMsgCount);
            
			CAPI2_SMS_GetSMSStorageStatus(GetNewTID(), GetClientID(), 0); // SIM_STORAGE is 0
            pdata->handler_state = BCM_RESPCAPI2Cmd;
			break;
        }

        case BCM_RESPCAPI2Cmd:			
        {			
            KrilStoredMsgCount *rdata = (KrilStoredMsgCount *)pdata->bcm_ril_rsp;
			if(RESULT_OK != capi2_rsp->result)
			{
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
				return;
			}
			if (capi2_rsp->msgType == MSG_SMS_GETSMSSTORAGESTATUS_RSP)
			{
				KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
				if(NULL != capi2_rsp->dataBuf)
				{
					CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t* presult = (CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t*) capi2_rsp->dataBuf;
					rdata->total_cnt = presult->NbFree + presult->NbUsed;
					rdata->used_cnt = presult->NbUsed;
					KRIL_DEBUG(DBG_INFO, "Storage free:%d ,  Storage used:%d\n", presult->NbFree, presult->NbUsed);
					pdata->handler_state = BCM_FinishCAPI2Cmd;
				}
				else
				{
					pdata->handler_state = BCM_ErrorCAPI2Cmd;
				}
				
			}
			else
			{
				KRIL_DEBUG(DBG_ERROR,"Receive error MsgType:0x%x...!\n", capi2_rsp->msgType);
				pdata->handler_state = BCM_ErrorCAPI2Cmd;
			}
            break;
			
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

#if 0
void KRIL_SRIL_DialEmergencyHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}
#endif


void KRIL_SRIL_LockInfoHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_ERROR, "KRIL_SRIL_LockInfoHandler: handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilLockInfo_t *rdata;
            KrilLockType_t *tdata = (KrilLockType_t *)pdata->ril_cmd->data;

            KRIL_DEBUG(DBG_ERROR, "Lock Type:%d \n", *tdata);
			// init local variable
			// prepare response mempry
            rdata = pdata->bcm_ril_rsp = kmalloc(sizeof(KrilLockInfo_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilLockInfo_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
			// send request to CP through CAPI2
			rdata->lock_type = *tdata;
			if(rdata->lock_type == KRIL_LOCK_PIN1 ||  rdata->lock_type == KRIL_LOCK_PIN2 ||  rdata->lock_type == KRIL_LOCK_PUK2)
			{
            	CAPI2_MS_GetElement(GetNewTID(), 
            						GetClientID(), 
            						(rdata->lock_type == KRIL_LOCK_PIN1)?MS_SIM_ELEM_PIN1_STATUS:MS_SIM_ELEM_PIN2_STATUS);
            	pdata->handler_state = BCM_SRIL_CHECK_PIN_STATUS;
			}
			else
			{
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
			}
        	break;

        }

        case BCM_SRIL_CHECK_PIN_STATUS:
		{
            KrilLockInfo_t *rdata = (KrilLockInfo_t *)pdata->bcm_ril_rsp;
            CAPI2_MS_Element_t *rsp = (CAPI2_MS_Element_t *) capi2_rsp->dataBuf;
			KrilCHVStatus_t chv_status = (KrilCHVStatus_t)rsp->data_u.u8Data;

            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            	KRIL_DEBUG(DBG_ERROR, "BCM_SRIL_CHECK_PIN_STATUS result error:%lu error...!\n", pdata->handler_state);
                return;
            }
       		KRIL_DEBUG(DBG_ERROR, "Lock info - CHV status from CP: lock type=%d, chv_status=%d \n", rdata->lock_type, chv_status);

			switch(chv_status)
			{
				case CHVSTATUS_CHV_NOT_NEEDED:
				case CHVSTATUS_CHV_VERIFIED:
					// prepare resp data
					rdata->lock_status = KRIL_PIN_NOT_NEED;
					rdata->remaining_attempt = 3;
            		pdata->handler_state = BCM_FinishCAPI2Cmd;
            		KRIL_DEBUG(DBG_ERROR, "Lock info - 1: lock type=%d,lock status=%d \n", rdata->lock_type, rdata->lock_status);
					return;
					
				case CHVSTATUS_CHV_BLOCKED:
					rdata->lock_status = (rdata->lock_type == KRIL_LOCK_PIN1)?KRIL_PUK:KRIL_PUK2;
            		KRIL_DEBUG(DBG_ERROR, "Lock info - 2: lock type=%d,lock status=%d \n", rdata->lock_type, rdata->lock_status);
					break;

				case CHVSTATUS_CHV_UNBL_BLOCKED:
					// prepare resp data
					rdata->lock_status = KRIL_PERM_BLOCKED;
					rdata->remaining_attempt = 0;
            		pdata->handler_state = BCM_FinishCAPI2Cmd;
            		KRIL_DEBUG(DBG_ERROR, "Lock info - 3: lock type=%d,lock status=%d \n", rdata->lock_type, rdata->lock_status);
					return;
				case CHVSTATUS_CHV_NOT_AVAILABLE:
				case CHVSTATUS_CHV_NEEDED:
				case CHVSTATUS_CHV_REJECTED:
				case CHVSTATUS_CHV_CONTRADICTION:
				case CHVSTATUS_CHV1_DISABLE_NOT_ALLOWED:
				default:
					rdata->lock_status = (rdata->lock_type == KRIL_LOCK_PIN1)?KRIL_PIN:KRIL_PIN2;
            		KRIL_DEBUG(DBG_ERROR, "Lock info - 4: lock type=%d,lock status=%d \n", rdata->lock_type, rdata->lock_status);
					break;
			}
			// Call CAPI API to get PIN1 remaining attempt
            CAPI2_SIM_SendRemainingPinAttemptReq(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
			break;
		}	
			
        case BCM_RESPCAPI2Cmd:
        {
            KrilLockInfo_t *rdata = (KrilLockInfo_t *)pdata->bcm_ril_rsp;
    		PIN_ATTEMPT_RESULT_t *rsp = (PIN_ATTEMPT_RESULT_t*)capi2_rsp->dataBuf;

            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            	KRIL_DEBUG(DBG_ERROR, "BCM_RESPCAPI2Cmd result error:%lu error...!\n", pdata->handler_state);
                return;
            }
			if(rdata->lock_type == KRIL_LOCK_PIN1)
			{
				if(rdata->lock_status == KRIL_PIN)
				{
					rdata->remaining_attempt = rsp->pin1_attempt_left;
            		KRIL_DEBUG(DBG_ERROR, "Lock info - 5: lock type=%d,lock status=%d, attempt=%d \n", rdata->lock_type, rdata->lock_status, rdata->remaining_attempt);
					if(!rsp->pin1_attempt_left)
					{
						// PIN is blocked
						rdata->lock_status = KRIL_PUK;
						rdata->remaining_attempt = rsp->puk1_attempt_left;
            			KRIL_DEBUG(DBG_ERROR, "Lock info - 6: lock type=%d,lock status=%d, attempt=%d \n", rdata->lock_type, rdata->lock_status, rdata->remaining_attempt);
					}
				}
				else
				{
					rdata->remaining_attempt = rsp->puk1_attempt_left;
            		KRIL_DEBUG(DBG_ERROR, "Lock info - 61: lock type=%d,lock status=%d, attempt=%d \n", rdata->lock_type, rdata->lock_status, rdata->remaining_attempt);
				}
			}
			else if(rdata->lock_type == KRIL_LOCK_PIN2 || rdata->lock_type == KRIL_LOCK_PUK2)
			{
				if(rdata->lock_status == KRIL_PIN2)
				{
					rdata->remaining_attempt = rsp->pin2_attempt_left;
            		KRIL_DEBUG(DBG_ERROR, "Lock info - 7: lock type=%d,lock status=%d, attempt=%d \n", rdata->lock_type, rdata->lock_status, rdata->remaining_attempt);
					if(!rsp->pin2_attempt_left)
					{
						// PIN2 is blocked
						rdata->lock_status = KRIL_PUK2;
						rdata->remaining_attempt = rsp->puk2_attempt_left;
            			KRIL_DEBUG(DBG_ERROR, "Lock info - 8: lock type=%d,lock status=%d, attempt=%d \n", rdata->lock_type, rdata->lock_status, rdata->remaining_attempt);
					}
				}
				else
				{
					rdata->remaining_attempt = rsp->puk2_attempt_left;
            		KRIL_DEBUG(DBG_ERROR, "Lock info - 81: lock type=%d,lock status=%d, attempt=%d \n", rdata->lock_type, rdata->lock_status, rdata->remaining_attempt);
				}
			}
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
}

extern void KRIL_SRIL_requestSvcModeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp);

void KRIL_SRIL_OemHookRawHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;
	char* rawdata = NULL;

	rawdata = (char*)pdata->ril_cmd->data;

	UInt8 msgid;

	msgid = (UInt8)rawdata[0];
	KRIL_DEBUG(DBG_ERROR,"msgid:%d\n", msgid);

	switch (msgid)
	{
		case OEM_FUNCTION_ID_SVC_MODE:
			KRIL_SRIL_requestSvcModeHandler(ril_cmd, capi2_rsp);
		break;

		case OEM_FUNCTION_ID_PERSONALIZATION:
			KRIL_SRIL_PersonalizationHandler(ril_cmd, capi2_rsp);
		break;			

		case OEM_FUNCTION_ID_GPRS:
			KRIL_SRIL_requestGprsHandler(ril_cmd, capi2_rsp);
		break;

		case OEM_FUNCTION_ID_IMEI:
			KRIL_SRIL_requestImeiHandler(ril_cmd, capi2_rsp);
		break;
#if 0
		case OEM_FUNCTION_ID_POWER:
#endif
		case OEM_FUNCTION_ID_NETWORK:
		case OEM_FUNCTION_ID_SYSDUMP:
		case OEM_FUNCTION_ID_SOUND:
		case OEM_FUNCTION_ID_CONFIGURATION:
		case OEM_FUNCTION_ID_CALL:
		case OEM_FUNCTION_ID_GPS:
		case OEM_FUNCTION_ID_PHONE:
#ifdef CONFIG_RTC_CP_BACKUP_FEATURE
		case OEM_FUNCTION_ID_MISC:
#endif
		case OEM_FUNCTION_ID_FACTORY:
		case OEM_FUNCTION_ID_RFS:
		case OEM_FUNCTION_ID_SAP:
		case OEM_FUNCTION_ID_AUTH:	
		case OEM_FUNCTION_ID_DATAROUTER:
		//case BRIL_HOOK_SET_PREFDATA:
		//   break;
		default:
			KRIL_DEBUG(DBG_ERROR,"Unsupported msgtype:%d Error!!!\n", msgid);
			pdata->handler_state = BCM_ErrorCAPI2Cmd;
			break;
	}

}
/*+ Band Selection sh0515.lee +*/

void KRIL_SRIL_SetBandHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp, KrilSrilSetBand_t *tdata)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
    ClientInfo_t clientInfo;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "KRIL_SRIL_SetBandHandler: handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            //KrilSrilSetBand_t *tdata = (KrilSrilSetBand_t *)pdata->ril_cmd->data;

            KRIL_DEBUG(DBG_INFO, "band: %d, curr_rat: %d\n", tdata->new_band, tdata->curr_rat);

	    CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
	    CAPI2_NetRegApi_SetSupportedRATandBand ( &clientInfo,(RATSelect_t) tdata->curr_rat, (BandSelect_t) tdata->new_band, GSM_ONLY, BAND_NULL);
       

			/*CAPI2_MS_SetSupportedRATandBand(GetNewTID(), 
											GetClientID(), 
											(RATSelect_t) tdata->curr_rat, 
											(BandSelect_t) tdata->new_band
											);*/
            		pdata->handler_state = BCM_RESPCAPI2Cmd;
			break;
		}	
		case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK )
            {
            	KRIL_DEBUG(DBG_ERROR, "handler_state -1:%lu error...!\n", pdata->handler_state);
              pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
			else
			{
            			pdata->handler_state = BCM_FinishCAPI2Cmd;
			}
			break;
		}
        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state - 2:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
	}
}/*- Band Selection sh0515.lee -*/
/*+ Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee +*/

void KRIL_SRIL_SetStackClasssMarkHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp, KrilStackNvramClassMark_t *tdata)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
    ClientInfo_t clientInfo;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "KRIL_SRIL_SetStackClasssMarkHandler: handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            //KrilStackNvramClassMark_t *tdata = (KrilStackNvramClassMark_t *)pdata->ril_cmd->data;
            CAPI2_MS_Element_t data;

			KRIL_DEBUG(DBG_INFO, "SetStackClasssMarkHandler - classmark id:%d \n", tdata->classmark_id);
			// init local variable
            		memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));

			switch(tdata->classmark_id)
			{
				case KRIL_CIPHERING_MODE:
            				data.inElemType = MS_STACK_ELEM_NVRAM_CLASSMARK;
            				data.data_u.stackClassmark.uasConfigParams.ciphering_updated = TRUE;
					data.data_u.stackClassmark.uasConfigParams.ciphering_support = tdata->data_u.is_supported;
					break;
				case KRIL_INTEGRATE_MODE:
            				data.inElemType = MS_STACK_ELEM_NVRAM_CLASSMARK;
            				data.data_u.stackClassmark.uasConfigParams.integrity_updated = TRUE;
					data.data_u.stackClassmark.uasConfigParams.integrity_support = tdata->data_u.is_supported;
					break;
				default:
		            		KRIL_DEBUG(DBG_ERROR, "wrong classmark id:%lu error...!\n", tdata->classmark_id);
		            		pdata->handler_state = BCM_ErrorCAPI2Cmd;
					return;
			}

                        CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                        CAPI2_MsDbApi_SetElement (&clientInfo, &data);
			//CAPI2_MS_SetElement(GetNewTID(),GetClientID(), &data);
            		pdata->handler_state = BCM_RESPCAPI2Cmd;
			break;
		}
        case BCM_RESPCAPI2Cmd:
        {
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

}/*- new Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee -*/CAPI2_MS_Element_t KRIL_SRIL_GetStackClasssMarkHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp, KrilStackNvramClassMark_t *tdata){	KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;	ClientInfo_t clientInfo;	CAPI2_MS_Element_t data;	KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_GetStackClasssMarkHandler \n");	switch (pdata->handler_state)	{		case BCM_SendCAPI2Cmd:		{	        	CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());	        	CAPI2_MsDbApi_GetElement ( &clientInfo, MS_STACK_ELEM_NVRAM_CLASSMARK);	        	pdata->handler_state = BCM_RESPCAPI2Cmd;	        	break;		}				case BCM_RESPCAPI2Cmd:		{			CAPI2_MS_Element_t* rsp = (CAPI2_MS_Element_t*)capi2_rsp->dataBuf;    			if (!rsp)    			{        			KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");        			pdata->handler_state = BCM_ErrorCAPI2Cmd;        			return data;    			}			if (rsp->inElemType != MS_STACK_ELEM_NVRAM_CLASSMARK)			{				KRIL_DEBUG(DBG_ERROR,"KRIL_SRIL_GetStackClasssMarkHandler : inElemType Error!! inElemType:%d\n",rsp->inElemType);				pdata->handler_state = BCM_ErrorCAPI2Cmd;        			return data;			}			switch(tdata->classmark_id)			{				case KRIL_CIPHERING_MODE:
            			//data.inElemType = MS_STACK_ELEM_NVRAM_CLASSMARK;
            			data.data_u.stackClassmark.uasConfigParams.ciphering_updated = rsp->data_u.stackClassmark.uasConfigParams.ciphering_updated;
					data.data_u.stackClassmark.uasConfigParams.ciphering_support = rsp->data_u.stackClassmark.uasConfigParams.ciphering_support;					return data;
				case KRIL_INTEGRATE_MODE:
            			//data.inElemType = MS_STACK_ELEM_NVRAM_CLASSMARK;
            			data.data_u.stackClassmark.uasConfigParams.integrity_updated = rsp->data_u.stackClassmark.uasConfigParams.integrity_updated;
					data.data_u.stackClassmark.uasConfigParams.integrity_support = rsp->data_u.stackClassmark.uasConfigParams.integrity_support;
					return data;
				default:
		            		KRIL_DEBUG(DBG_ERROR, "wrong classmark id:%lu error...!\n", tdata->classmark_id);
		            		pdata->handler_state = BCM_ErrorCAPI2Cmd;
					return data;						}					}		default:		{	        	KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);	        	pdata->handler_state = BCM_ErrorCAPI2Cmd;	        	return data;
		}	}	return data;}

/*- Ciphering Mode sh0515.lee /  Integrity Mode sh0515.lee -*/
