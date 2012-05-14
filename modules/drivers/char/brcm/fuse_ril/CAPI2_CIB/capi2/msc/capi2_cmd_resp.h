/*********************************************************************
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*     
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*    
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent. 
*   
***************************************************************************/
/**
*
*   @file   capi2_cmd_resp.h
*
*   @brief  This file dispatches the CAPI2 request messages.
*
****************************************************************************/
#ifndef _CAPI2_CMD_RESP
#define _CAPI2_CMD_RESP


typedef struct
{
	UInt32 tid;
	UInt8 clientID;
	MsgType_t reqId;
	MsgType_t rspId;
	UInt8 callIndex;
	SimNumber_t	simId;
}CAPI2_CmdRespInfo_t;

void CAPI2_CMD_RSP_IntCmdRespTable(void);

Boolean CAPI2_CMD_RSP_AddCmdRespEntry(CAPI2_CmdRespInfo_t *info);

Boolean CAPI2_CMD_RSP_FindCmdRespMapping(MsgType_t rspId, 
										 CAPI2_CmdRespInfo_t *info,
										 Boolean removeEntry);

Boolean CAPI2_CMD_RSP_RemoveCmdRespMapping(MsgType_t rspId);
Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCmd(MsgType_t reqId, CAPI2_CmdRespInfo_t *info, Boolean removeEntry);
Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCallIndex(UInt8 callIndex, CAPI2_CmdRespInfo_t *info, Boolean removeEntry);
Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCallIndexRespId(UInt8 callIndex, MsgType_t rspId, CAPI2_CmdRespInfo_t *info, Boolean removeEntry);
Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCmdRangeCallIndex( UInt8 callIndex, MsgType_t StartreqId, MsgType_t EndreqId, CAPI2_CmdRespInfo_t *info, Boolean removeEntry);

Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCmdRange(MsgType_t StartreqId, MsgType_t EndreqId, CAPI2_CmdRespInfo_t *info, Boolean removeEntry);

Boolean CAPI2_CMD_RSP_CheckDupCmdRespEntry( MsgType_t rspId, SimNumber_t simId);
Boolean CAPI2_CC_CMD_RSP_CheckDupCmdRespEntry( MsgType_t rspId, UInt8 callIndex, SimNumber_t simId);

UInt32 CAPI2_CMD_RSP_GetCmdRespMappingbyCallIndexRespId( UInt8 callIndex, MsgType_t rspId, SimNumber_t simId);
void CAPI2_CSD_RegisterClient(UInt8 ClientID);;

#endif
