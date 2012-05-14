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
*
*  @file   meas_mgr.h
*
*  @brief  This file contains the public information related to the meas_mgr
*          component.
*	     
*
*  @note  
*         
*         
*
****************************************************************************/
//! Configure and status info
typedef struct
{
	UInt16 	adc_ch; 	///< ADC channel
	UInt16	adc_trg;	///< ADC sample trigger
	UInt16	o_rd_data;	///< output ADC read data
}MeasMngrCnfg_t;

//! Function Eror results 
typedef enum
{
	MEAS_RESULT_PENDING,	///< Pending result.
	MEAS_RESULT_COMPLETE,	///< Complete result
	MEAS_RESULT_ERROR		///< Error result
} MeasMgrResult_t;

//! Function Error for Database
typedef enum
{
	MEAS_DB_RESULT_OK,		///< Complete/OK result
	MEAS_DB_RESULT_INIT,	///< Init State result
	MEAS_DB_RESULT_TIMEOUT,	///< Timed Out result
	MEAS_DB_RESULT_ERROR	///< Error result
} MeasMgrDbaseResult_t;


//***************************************************************************
/**
 *  Function Name:   MEASMGR_Init
 *   Description: 	Initialize the Measurement Manager Task, DEEP_SLEEP.
 *	 @return  void
 * 			
 */
void MEASMGR_Init(void);
//***************************************************************************
/**
 *  Function Name:   MEASMGR_ShutDown
 *   Description: 	Disables the Measurement Manager Task, DEEP_SLEEP.
 *	 @return  void
 * 			
 */
void MEASMGR_ShutDown(void);
//*********************************************************************
/**
*
*   Description: MEASMGR_WakeMeasService is called by the RF DRX signal & 
*	expiring_of_5sec_timer.
*	This function retrieves the Wakes the Measurement Manager Task.
*   @param void
*
*   @return   Result_t          Function status
*   @note      Releases the completed semaphore to allow the Measurement Manager to
*			run.
*
**********************************************************************/
void MEASMGR_WakeMeasService( void );
//*********************************************************************
/**
*
*   Description: MEASMGR_RunMeas performs the MeasureMent Manager Service.
*	This function retrieves the results of the required ADCs and  calls the basic services.
*   @param void
*
*   @param    void
*   @return   Result_t           Result_ok,ERROR
*   @note      
*
**********************************************************************/
void MEASMGR_Task(void);

//*********************************************************************
/**
*
*   Description: MEASMGR_GetDataB_Adc API is called by the user to retrieve the output 
*		ADC read data for each channel that was updated by the Measurement manager.
*
*	This function is ran after the DRX signal is received from the CP.
*   @param void
*
*   @return   MeasMgrDbaseResult_t          Function status
*   @param1    HAL_ADC_rd_cnfg_st_t		pointer to ADC read configuration
*   @note      Releases the completed semaphore to allow the Measurement Manager to
*			run.
*
**********************************************************************/

MeasMgrDbaseResult_t MEASMGR_GetDataB_Adc(MeasMngrCnfg_t* pIoAdc);



