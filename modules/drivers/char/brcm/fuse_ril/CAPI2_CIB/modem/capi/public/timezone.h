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
*   @file   timezone.h
*
*   @brief  This file defines the interface for TimeZone API.
*
****************************************************************************/
/**

*   @defgroup   TIMEZONE_APIGroup   Time Zone
*   @ingroup    TIMEZONEGroup
*
*   @brief      This group defines the interfaces and the utilities related to
				time zone management on the MS.
*	
****************************************************************************/

#ifndef _TIMEZONE_H_
#define _TIMEZONE_H_

//******************************************************************************
//							Include block
//******************************************************************************


//******************************************************************************
//							Definition block
//******************************************************************************
#define SEMI_OCTET_DECODE(x)	((((x)&0x0f)<<4)+(((x)>>4)&0x0f))
#define TIMEZONE_DECODE(x)		(((((x)&0x07)<<4)+(((x)>>4)&0x0f))|(((x)&0x08)<<4))


/** @} */
		 
#endif	// _TIMEZONE_H_
		 
		 

