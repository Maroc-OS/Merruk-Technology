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
* @file   bm_power_manager.h
* @brief  B-Modem Power Manager API
*
*****************************************************************************/

/**
*
* @defgroup Modem-CellularStack
*
* @brief    Modem Team
*
* @ingroup  MPG
*****************************************************************************/

#ifndef	__bm_pm_H__
#define	__bm_pm_H__

/**
*
* @addtogroup CellularStackL1
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/**

API Summary

Initialization
==============
void BMPM_init(void);


B-Modem Power State Management 
===================================================
PM_STATUS BMPM_set_state_sync(PM_CLIENT_ID client_id, PM_STATE_ID pm_state_id, Boolean deep_sleep_enable);
PM_STATUS BMPM_set_state_async(PM_CLIENT_ID client_id, PM_STATE_ID pm_state_id, Boolean deep_sleep_enable, UInt16 time_out_us);

PM_STATE_t *PM_STATE BMPM_get_pm_state(void);


Sleep Management
=========================================
PM_STATUS BMPM_disable_deep_sleep(PM_CLIENT_ID client_id);
PM_STATUS BMPM_enable_deep_sleep(PM_CLIENT_ID client_id);

**/

typedef enum
{
	PM_CLIENT_PM, 
	PM_CLIENT_L1G,               
	PM_CLIENT_L1U,                            
	PM_CLIENT_FW,               
	PM_CLIENT_SIMIO,                
	PM_CLIENT_EXT,              
	PM_CLIENT_OTHER,               
} PM_CLIENT_ID;


typedef enum
{
	PM_STATE_IDLE,			
	PM_STATE_2G_SEARCH,
	PM_STATE_2G_PCH,
	PM_STATE_2G_CS,
	PM_STATE_2G_PS,
	PM_STATE_3G_SEARCH,
	PM_STATE_3G_PCH,
	PM_STATE_3G_DCH,
	PM_STATE_3G_HSPA
} PM_STATE_ID;



typedef enum
{
	PM_STATUS_SUCCESS,                              
	PM_STATUS_TIMEOUT,                              
	PM_STATUS_FAIL                       
} PM_STATUS;


typedef enum
{
	PM_FREQ_ID_LOW,                              
	PM_FREQ_ID_MID,                              
	PM_FREQ_ID_HIGH,                              
	PM_FREQ_ID_TURBO                              
} PM_FREQ_ID;


typedef struct
{
	UInt32					starting_time;
	PM_STATE_ID				pm_state_id; 	

	PM_FREQ_ID				pm_run_state_freq_id; 	
	PM_FREQ_ID				pm_suspend_state_freq_id; 	

	Boolean					deep_sleep_allowed;	
} PM_STATE_t;




/**
*  @brief  Initialize B-Modem PM.This API should be called once during boot
*
*  @param  
*
*  @return 
*
*  @note   
*
****************************************************************************/
void BMPM_init(void);

/**
*  @brief  set power state (synchronous mode)
*
*  @param  client_id			(in) client ID 
*  @param  pm_state_id  		(in) power state set to
*
*  @return PM_STATUS_SUCCESS or error code
*
*  @note   
****************************************************************************/
PM_STATUS BMPM_set_state_sync(PM_CLIENT_ID client_id, PM_STATE_ID pm_state_id);


/**
*  @brief  set power state (asynchronous mode)
*
*  @param  client_id			(in) client ID 
*  @param  pm_state_id  		(in) power state set to
*  @param  time_out_us		(in) timeout value in us (rounded up to 32KHz ticks)
*
*  @return PM_STATUS_SUCCESS or error code
*
*  @note   
****************************************************************************/
PM_STATUS BMPM_set_state_async(PM_CLIENT_ID client_id, PM_STATE_ID pm_state_id, UInt16 time_out_us);




/**
*  @brief  get PM power state
*
*  @param  
*
*  @return PM_STATE ptr
*
*  @note   
****************************************************************************/
PM_STATE_t *BMPM_get_pm_state(void);



/**
*  @brief  disable deep sleep (overide default power state setting)
*
*  @param  client_id			(in) client ID 
*
*  @return PM_STATUS_SUCCESS or error code
*
*  @note   
****************************************************************************/
PM_STATUS BMPM_disable_deep_sleep(PM_CLIENT_ID client_id);



/**
*  @brief  enable deep sleep (restore default power state setting)
*
*  @param  client_id			(in) client ID 
*
*  @return PM_STATUS_SUCCESS or error code
*
*  @note   
****************************************************************************/
PM_STATUS BMPM_enable_deep_sleep(PM_CLIENT_ID client_id);


#ifdef __cplusplus
}
#endif

#endif // __bm_pm_H__
