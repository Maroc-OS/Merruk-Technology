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
*   @file   ms_database_api.h
*
*   @brief  This file defines the global information for MS.
*
****************************************************************************/
/**
*
*   @defgroup   MSDatabaseGroup	Mobile Station Database APIs
*   @ingroup    SystemGroup				
*
*   @brief      This group allows getting and setting the MS Database elements.
*
	\n Use the link below to navigate back to the main page. \n
    \li \ref index
****************************************************************************/

#ifndef __MS_DATABASE_API_H__
#define __MS_DATABASE_API_H__


//=========================================================================
// MS Database API's for accessing of the elements 
//=========================================================================

/**
 * @addtogroup MSDatabaseGroup
 * @{
 */ 

//**************************************************************************************
/**
	This function is a generic interface that will be used by any clients (external/
	internal) to update the MS Database elements. This function will copy the element
	passed in the third argument in to the database.
	Note: After proper range checks in neccessary the value will be updated.

	@param      clientInfoPtr	(in) Client information
	@param		inElemType		(in) The database element type.
	@param		inElemPtr		(in) A pointer to the location of the new value of the element.

	@return		Result_t
**/
Result_t MsDbApi_SetElement(	ClientInfo_t	*clientInfoPtr,
								MS_Element_t	inElemType,
								void*			inElemPtr);

//**************************************************************************************
/**
	This function is a generic interface that will be used by any clients (external/
	internal) to read any MS Database elements. This function will copy the contents of
	the database value to the memory location passed in by the last argument.  
	The calling entity should know what will be the typecast used to retreive the element.

	@param      clientInfoPtr	(in) Client information
	@param		inElemType		(in) The database element type.
	@param		inElemPtr		(in) The address where the element shall be copied to

	@return		Result_t
**/
Result_t MsDbApi_GetElement(	ClientInfo_t	*clientInfoPtr,
								MS_Element_t	inElemType,
								void*			inElemPtr);

Result_t MsDbApi_InitCallCfg(ClientInfo_t* inClientInfoPtr);

Boolean MsDbApi_IsCallActive(void);


void	MsDbApi_SYS_EnableCellInfoMsg(ClientInfo_t* inClientInfoPtr, Boolean inEnableCellInfoMsg);


/** @} */

#endif	//	__MS_DATABASE_API_H__

