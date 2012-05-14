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
*   @file   util_lswap.h
*
*   @brief  This file prototypes the utility lswap function used by the platform.
*
*****************************************************************************/
#ifndef _UTIL_LSWAP_H_
#define _UTIL_LSWAP_H_
 


#ifndef __BIG_ENDIAN
__inline unsigned long lswap( unsigned long l )
{
	typedef struct {
		unsigned char byte0;
		unsigned char byte1;
		unsigned char byte2;
		unsigned char byte3;
	} little_e;

	typedef struct {
		unsigned char byte3;
		unsigned char byte2;
		unsigned char byte1;
		unsigned char byte0;
	} big_e;

	union {
		little_e	le;
		unsigned long	l;
	} lu;

	union {
		big_e		be;
		unsigned long	l;
	} bu;

	bu.l = l;
	lu.le.byte0 = bu.be.byte0;
	lu.le.byte1 = bu.be.byte1;
	lu.le.byte2 = bu.be.byte2;
	lu.le.byte3 = bu.be.byte3;
	return lu.l;
} // lswap
#endif //__BIG_ENDIAN


#endif //_UTIL_LSWAP_H_

