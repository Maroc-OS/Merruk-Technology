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
*   @file   ms_sysutil.h
*
*   @brief  This file defines the utility functions for MS.
*
****************************************************************************/

void MS_SetGsmRxLevThresold(UInt8 Thresold);
void MS_SetGsmRxQualThresold(UInt8 Thresold);
void MS_SetUmtsRscpThresold(UInt8 Thresold);
void MS_SetUmtsEcioThresold(UInt8 Thresold);
UInt8 MS_GetRxSignalLev( void );
UInt8 MS_GetRxSignalQual( void );

Boolean				MS_IsRegisteredHomePLMN(void);
void				MS_SetPlmnMCC(UInt16 mcc);
void				MS_SetPlmnMNC(UInt8 mnc);

void				MS_StatisticInfoReport(void);
Boolean				MS_GetInvalidSimMsMe(void);
void				SYS_SetRxSignalInfo(UInt8 RxLev, Boolean RxQualValid, UInt8 RxQual);
void				SYS_SetGSMRegistrationStatus(RegisterStatus_t status);
void				SYS_SetGPRSRegistrationStatus(RegisterStatus_t status);
void				SYS_SetGSMRegistrationCause( PCHRejectCause_t cause );
void				SYS_SetGPRSRegistrationCause( PCHRejectCause_t cause );
PCHRejectCause_t	SYS_GetGSMRegistrationCause(void);

//**************************************************************************************
/**
	This function is used to notify to all the registered clients of a database element
	content update..

	@param		inClientInfoPtr (in) The Client Information.
	@param		inElemType (in) The database element type.

	@return		Result_t
**/
void MS_SendLocalElemNotifyInd( ClientInfo_t*	inClientInfoPtr,
								MS_Element_t	inElemType);



/**
	The following APIs are obsolete. Please use MS_GetElement() and MS_SetElement() with element valuee
	set to MS_NETWORK_ELEM_GPRSINVALID_SIM_MS_ME_STATUS and MS_NETWORK_ELEM_CSINVALID_SIM_MS_ME_STATUS
**/
RegisterStatus_t	MS_GetCsInvalidSimMsMe(void);

RegisterStatus_t	MS_GetGprsInvalidSimMsMe(void);

Boolean  MS_IsClientGeneratedRingTone(void);

Boolean MS_IsPendingDetachCnf(SimNumber_t sim_id);

void MS_SetPendingDetachCnf(SimNumber_t sim_id, Boolean flag);

void MS_SetMsClass(SimNumber_t sim_id, MSClass_t ms_class);

MSClass_t MS_GetMsClass(SimNumber_t sim_id);

void MS_SetTestSIMInserted(Boolean is_test_sim);

Boolean MS_IsTestSIMInserted(void);



