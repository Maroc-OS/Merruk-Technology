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
*   @file   stk_api_atc.h
*
*   @brief  This file contains SIM Application ToolKit APIs. (used by ATC or xScript only)
*
****************************************************************************/
#ifndef _STK_API__ATC_H_
#define _STK_API_ATC_H_


/**
	This is the API function to return the SIM Application Toolkit Terminal Profile data. 
	
	If the data has been set by SATK_SetTermProfile() before, it returns that data. Otherwise
	it returns the Terminal Profile data in Sys Parm. 

    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		length (out) Length of Terminal Profile data returned. 

	@return		Pointer to the Terminal Profile data. 
**/
const UInt8*  SatkApi_GetTermProfile(ClientInfo_t* inClientInfoPtr, UInt8 *length);


#endif  // _STK_API_ATC_H_

