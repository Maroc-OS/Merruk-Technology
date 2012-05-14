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
/**
*
*   @file   common_struct.h
*
*   @brief  This file defines structures from INC folder used by both CAPI and CAPI2
*
****************************************************************************/
#ifndef _COMMON_MSTRUCT_H_
#define _COMMON_MSTRUCT_H_


#ifdef __cplusplus
extern "C" {
#endif
   



//******************************************************************************
//
// MNATDS Message Types
//
//******************************************************************************

//******************************************************************************
//
// Message Parameters
//
//******************************************************************************

typedef struct
{
	T_SYSTEM_MODE		system_mode;
	T_DATA_RATE			data_rate;
	T_SYNCHRON_TYPE		synchron_type;
	T_CONN_ELEMENT		conn_element;
	T_SERVICE_MODE		service_mode;
    // 8/18/2004 From Comneon code (atc_setup.c), Instead of a list of possible RLP, it is now simplified:
    // if GEM, then version 2, if compression then version 1, otherwise, version 0.
	T_RLP_PAR			rlp_par;
	T_EST_CAUSE			est_cause;
	T_CALLING_SUBADDR	calling_subaddr;
	T_TEL_NUMBER		tel_number;
	T_CALLED_SUBADDR	called_subaddr;
	T_MN_CLIR			mn_clir;
	T_MN_CUG			mn_cug;
	SDL_Boolean			autocall;
	T_DC_PAR			ds_datacomp;
	T_DC_PAR			ds_hw_datacomp;
	T_DC_NEGOTIATION	ds_datacomp_neg;
    // 8/18/2004 yuanliu - this is for AT+ETBM. If not supported, then all values in the struct should be set to 0.
	T_ETBM				etbm;
} MNATDSParmSetupReq_t;


typedef struct
{
	T_AT_CAUSE			at_cause;
	T_TI_PD				ti_pd;
} MNATDSParmReleaseReq_t;

typedef struct
{
	T_SYSTEM_MODE		system_mode;
	T_RLP_PAR			rlp_par;
    T_TI_PD		  	   	ti_pd;
    T_CONN_SUBADDR  	conn_subaddr;
	T_DC_PAR			ds_datacomp;
	T_DC_PAR			ds_hw_datacomp;
	T_DC_NEGOTIATION	ds_datacomp_neg;
	T_ETBM				etbm;
} MNATDSParmSetupResp_t;


typedef struct
{
	T_DATA_RATE			data_rate;
	T_SERVICE_MODE		service_mode;
	T_CONN_ELEMENT		conn_element;
	T_TI_PD				ti_pd;
	T_TEL_NUMBER		tel_number;
	T_CALLING_SUBADDR	calling_subaddr;
	T_CALLED_SUBADDR	called_subaddr;
	Boolean				ds_datacomp_ind;
} MNATDSParmSetupInd_t;


typedef struct
{
	T_TI_PD				ti_pd;
	T_DC_PAR			ds_neg_datacomp;
} MNATDSParmConnectInd_t;

typedef struct
{
	T_TI_PD				ti_pd;
	T_SERVICE_MODE		service_mode;
} MNATDSParmServiceInd_t;

typedef struct
{ 
	T_TI_PD				ti_pd;
	T_TEL_NUMBER		tel_number;
	T_CONN_SUBADDR		conn_subaddr;
	T_DC_PAR			ds_neg_datacomp;
} MNATDSParmSetupCnf_t;

typedef struct
{
    T_MN_CAUSE  		mn_cause;
} MNATDSParmSetupRej_t;

typedef struct
{
    T_TI_PD  			ti_pd;
    T_MN_CAUSE  		mn_cause;
} MNATDSParmReleaseInd_t;

typedef struct
{
	T_TI_PD				ti_pd;
} MNATDSParmReleaseCnf_t;

typedef struct
{
	T_LINESTATE			linestate1;
	T_LINESTATE			linestate2;
	T_LINESTATE			linestate3;
} MNATDSParmStatusInd_t;

typedef struct 
{
	UInt8	ppm; 
} MNATDSParmFET_t;

typedef union
{
	MNATDSParmSetupReq_t		setup_req;
	MNATDSParmReleaseReq_t		release_req;
	MNATDSParmSetupResp_t		setup_resp;

	MNATDSParmSetupInd_t		setup_ind;
	MNATDSParmConnectInd_t		connect_ind;
	MNATDSParmServiceInd_t		service_ind;
	MNATDSParmSetupCnf_t		setup_cnf;
	MNATDSParmSetupRej_t		setup_rej;
	MNATDSParmReleaseInd_t		release_ind;
	MNATDSParmReleaseCnf_t		release_cnf;
	MNATDSParmStatusInd_t		status_ind;
	MNATDSParmFET_t				fet_ind;
} MNATDSMsgParm_t;						// MNATDS message parameters

typedef struct
{
	MNATDSMsgType_t 	type;			// MNATDS Message Type
	ClientInfo_t		clientInfo;		///<ClientInfo
	MNATDSMsgParm_t     parm;			// MNATDS Message Parameters
} MNATDSMsg_t;	

#ifdef __cplusplus
}
#endif


#endif  //_COMMON_MSTRUCT_H_

