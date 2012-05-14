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
*   @file   engmode_api.h
*
*   @brief  This file defines the interface for diagnostics functions.
*
****************************************************************************/
/**
*   @defgroup   EngineeringModeGroup   Engineering Mode API
*	@ingroup	Debug
*
*   @brief      This is the Engineering Mode API
*	
****************************************************************************/

#ifndef _DIAGNOST_H_
#define _DIAGNOST_H_

/**
 * @addtogroup EngineeringModeGroup
 * @{
 */

//***************************************************************************************
/**
    This function requests that the protocol stack to return test 
	parameters (e.g. measurement report)

	@param	inPeriodicReport (in) True to request stack to report TestParam periodically
	@param	inTimeInterval	 (in) The time interval between peiodic reports.

	The function will cause a ::MSG_MEASURE_REPORT_PARAM_IND message to be sent 
	every inTimeInterval.  Included in the data buffer of the message will be the 
	test parameters in the structure ::MS_RxTestParam_t.
	The client might add this message to their message handling like this:

	\code
	case MSG_MEASURE_REPORT_PARAM_IND:
 			HandleMeasReportParamInd(inMsg);
			break;
	\endcode

and type cast the data buffer of the message like this:

	\code
	MS_MMParam_t*	mmParamPtr = &((MS_RxTestParam_t*)inMsgPtr->dataBuf)->mm_param;
	MS_GSMParam_t*	gsmParamPtr = &((MS_RxTestParam_t*)inMsgPtr->dataBuf)->gsm_param;
	MS_UMTSParam_t*	umtsParamPtr = &((MS_RxTestParam_t*)inMsgPtr->dataBuf)->umts_param;
	\endcode

	@internal	there is a third parameter MN_ReportRxTestParam.  It is an internal
				callback function to receive the TestParam report.
	
**/	
void DIAG_ApiMeasurmentReportReq(Boolean inPeriodicReport, UInt32 inTimeInterval);

void DiagApi_MeasurmentReportReq(ClientInfo_t* inClientInfoPtr, Boolean inPeriodicReport, UInt32 inTimeInterval);



//***************************************************************************************
/**
    This function enables or disables the Cell Lock Feature.

	@param	cell_lockEnable (in) TRUE to enable Cell Lock Feature, FALSE otherwise
	@return	void

	
**/	
void DIAG_ApiCellLockReq(Boolean cell_lockEnable);

//***************************************************************************************
/**
    This function queries the Cell Lock Feature staus.

	@return	Boolean
	@note
	The return value will be TRUE if Cell Lock Feature was enabled , FALSE otherwise.
	
**/
Boolean DIAG_ApiCellLockStatus(void);


//***************************************************************************************
/**
    This function checks if we need to enable engineering mode with stack

	@param	clientInfo
	@return	void
**/

void Diag_CheckEM(ClientInfo_t *clientInfoPtr);

/** @} */

#endif // _DIAGNOST_H_

