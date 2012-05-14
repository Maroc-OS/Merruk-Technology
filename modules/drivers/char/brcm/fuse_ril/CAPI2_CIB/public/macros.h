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
*   @file   macros.h
*
*   @brief  This file contains generic macro function definitions.
*
****************************************************************************/

#ifndef _INC_MACROS_H_
#define _INC_MACROS_H_


#ifndef MAX
#define MAX( a, b )				( (a)>(b) ? (a) : (b) )
#endif

#ifndef MIN
#define MIN( a, b )				( (a)<(b) ? (a) : (b) )
#endif 

#ifndef ABS
#define ABS( a )				( (a)>0 ? (a) : -(a) )
#endif

#ifndef MIN_3_ARG
#define MIN_3_ARG( a, b, c ) MIN( MIN((a), (b)), (c) )
#endif

#ifndef MAX_3_ARG
#define MAX_3_ARG( a, b, c ) MAX( MAX((a), (b)), (c) )
#endif

// Return TRUE if passed digit is between 0-9 
#define IsDigitValue(a)         ( ((a) >= 0) && ((a) <= 9) ) 

#ifdef MTI_PADDING_WORD
#error	MTI_PADDING_WORD already defined!
#endif

#define MTI_PADDING_WORD(n)		(((n) % sizeof(UInt32)) ?					\
								 sizeof(UInt32) - ((n) % sizeof(UInt32)) :	\
								 sizeof(UInt32))

#ifdef MTI_CHECK_STRUCT_SIZE
#error	MTI_CHECK_STRUCT_SIZE already defined!
#endif

#define MTI_CHECK_STRUCT_SIZE(T)	typedef struct {				\
									UInt8 padding1[1 - sizeof(T) %	\
												  sizeof(UInt32)];	\
									UInt8 padding2[2 - sizeof(T) %	\
												  sizeof(UInt32)];	\
									UInt8 padding3[3 - sizeof(T) %	\
												  sizeof(UInt32)];	\
									} Check_ ## T

// Macros for register control
#define  SetReg16Bit(reg, x)    *(volatile UInt16*)(reg) = (*(UInt16*)(reg) | (1<<(x)))
#define  SetReg32Bit(reg, x)    *(volatile UInt32*)(reg) = (*(UInt32*)(reg) | (1<<(x)))
#define  SetReg16Mask(reg, x)   *(volatile UInt16*)(reg) = (*(UInt32*)(reg) | (x))
#define  SetReg32Mask(reg, x)   *(volatile UInt32*)(reg) = (*(UInt32*)(reg) | (x))

#define  ClrReg16Bit(reg, x)    *(volatile UInt16*)(reg) = (*(UInt16*)(reg) & (~(1<<(x))))
#define  ClrReg32Bit(reg, x)    *(volatile UInt32*)(reg) = (*(UInt32*)(reg) & (~(1<<(x))))
#define  ClrReg16Mask(reg, x)   *(volatile UInt16*)(reg) = (*(UInt16*)(reg) & (~(x)))
#define  ClrReg32Mask(reg, x)   *(volatile UInt32*)(reg) = (*(UInt32*)(reg) & (~(x)))


#endif
