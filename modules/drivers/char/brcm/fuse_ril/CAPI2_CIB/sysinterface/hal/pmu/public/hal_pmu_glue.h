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
***************************************************************************/
/**
*	file   hal_pmu_glue.h
*
*   brief  
*
****************************************************************************/
/*
*   defgroup   HAL_PMU   HAL PMU Glue layer header file
*	
*   brief		Temporary API declaration for glue layer. After system integration done, 
*				this will be deleted. Intended to provide sample codes to use new HAL.
*
****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
#include "i2c_drv.h"
#include "pmu.h"
#include "hal_pmu.h"
*/
void PMU_Init ( void );
void PMU_Init_I2C_Err_Recover( void );
void PMU_ClientPowerDown( void );
void PMU_GetPowerupCause( PMU_Powerup_Cb_t PowerupCb );
void PMU_ActivateSIM( PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt );
void PMU_SetVoltageInActiveMode( void );
void PMU_IsChargerPresent ( PMU_IsChargerPresent_Cb_t pmuCb, PMU_I2CDirectCall_t directI2Ccall  );
Boolean PMU_IsSIMReady( PMU_SIMLDO_t simldo );

#if defined ( PMU_BCM59001 ) || defined ( PMU_BCM59035 ) || defined( PMU_BCM59036 ) || defined(PMU_BCM59038)
void PMU_TurnOnMicBias( void );
void PMU_TurnOffMicBias( void );
#endif

#ifdef PMU_PCF50612
void PMU_SetGPOutput(PMU_GPOpin_t GPOpin, PMU_GPOutput_t OutputVal);
#endif

