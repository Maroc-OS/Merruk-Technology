/*
 *  drivers/mtd/nand_ecc512.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file is the header for the ECC algorithm.
 */

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/mtd/nand_ecc512.h
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

#ifndef __MTD_NAND_ECC512_H__
#define __MTD_NAND_ECC512_H__

struct mtd_info;

/*
 * Calculate 3 byte ECC code for 512 byte block
 */
int nand_calculate_ecc512(struct mtd_info *mtd, const u_char *dat,
			  u_char *ecc_code);

/*
 * Detect and correct a 2 bit error for 512 byte block
 */
int nand_correct_data512(struct mtd_info *mtd, u_char *dat, u_char *read_ecc,
			 u_char *calc_ecc);

#endif /* __MTD_NAND_ECC512_H__ */
