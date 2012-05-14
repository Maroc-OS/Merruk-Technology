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
*   @file   bandselect.h
*
*   @brief  This file contains all functions that is related to band selection.
*
****************************************************************************/


#ifndef _BANDSELECT_H_
#define _BANDSELECT_H_



// functions for internal platform use only
void			InitBand( void );
void			SwitchBand( void );
void			SaveLastGoodBand( void );
//void			initLastBandInfo( void );
Boolean			IsBandSupported( BandSelect_t band );
Boolean			IsBandSwitchNeeded(void);
void			SetBandSwitchNeeded(void);
Boolean			MS_SetRatMode(USIM_RAT_MODE_t rat_mode);
void			InitSystemRATandBand (void);
void			ReSendChangeBand(void);
Result_t		ForceSelectBand( BandSelect_t bandSelect );
void			SetBandSelectPending(Boolean);
BandSelect_t	GetLastBandSelect(void);
void			SendChangeBand(BandSelect_t band_select, RATSelect_t rat_select); 
BandSelect_t	MS_GetCurrentBandSelect(void);
RATSelect_t		MS_GetCurrentRatSelect(void);
void 			MS_ResetCurrentBandSelect(void);

//Functions for bandswitch to other RAT due to emergency call.
Boolean	SYS_CheckBandSwitchNeededForCall(void);
Boolean	SYS_IsCallBandSwitchvalid(void);
void 	SYS_RevertCallBandSwitch(void);
Result_t NetRegApi_CellLock(ClientInfo_t *clientInfoPtr, Boolean Enable, BandSelect_t lockband, UInt8 lockrat, UInt16 lockuarfcn, UInt16 lockpsc);
void NetReg_CheckCellLockEnabled(void);

/**
 * @addtogroup PhoneControlAPIGroup
 * @{
 */

/////////////// API functions related to band selection ///////////////


/** @} */


// Internal Functions.
Result_t SYS_DoBandSelect(ClientInfo_t *clientInfoPtr,BandSelect_t bandSelect, Boolean disable_fast_band_select, Boolean cellLockProcedure);

#endif

