/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/plat-bcmap/include/plat/nand_otp.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef __NAND_OTP_H__
#define __NAND_OTP_H__

/* Use below flags to differentiate OTP on different flash chips */
enum {
	NAND_MICRON_OTP = 0x1,
	NAND_HYNIX_OTP,
	/* Always add entries before this macro */
	NAND_OTP_NONE
};

/*
 * struct nand_otp - OTP related info for NAND flash chips that support OTP
 *
 * @maf_id:		NAND Manufacture ID
 * @otp_start:	OTP start address
 * @otp_end:	OTP end address
 * @flags:		OTP FLAG
 */
struct nand_otp {
	int maf_id;
	unsigned long otp_start;
	unsigned long otp_end;
	int flags;
};

#endif /*__NAND_OTP_H__*/
