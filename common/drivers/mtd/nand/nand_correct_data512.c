/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/mtd/nand/nand_correct_data512.c
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

/* --------------------------------------------------------------------------- */
/* nand error control correction routines */
/* --------------------------------------------------------------------------- */
#include <linux/types.h>
#include <linux/mtd/nand_ecc512.h>

typedef union {
	unsigned long mylword;
	unsigned short myword[2];
	unsigned char mybyte[4];	/* MSB: byte[0] */
	struct {
		unsigned int bit31:1;
		unsigned int bit30:1;
		unsigned int bit29:1;
		unsigned int bit28:1;
		unsigned int bit27:1;
		unsigned int bit26:1;
		unsigned int bit25:1;
		unsigned int bit24:1;
		unsigned int bit23:1;
		unsigned int bit22:1;
		unsigned int bit21:1;
		unsigned int bit20:1;
		unsigned int bit19:1;
		unsigned int bit18:1;
		unsigned int bit17:1;
		unsigned int bit16:1;
		unsigned int bit15:1;
		unsigned int bit14:1;
		unsigned int bit13:1;
		unsigned int bit12:1;
		unsigned int bit11:1;
		unsigned int bit10:1;
		unsigned int bit9:1;
		unsigned int bit8:1;
		unsigned int bit7:1;
		unsigned int bit6:1;
		unsigned int bit5:1;
		unsigned int bit4:1;
		unsigned int bit3:1;
		unsigned int bit2:1;
		unsigned int bit1:1;
		unsigned int bit0:1;
	} mybits;
} UnionLW;

typedef union {
	unsigned char mybyte;
	struct {
		unsigned int bit7:1;
		unsigned int bit6:1;
		unsigned int bit5:1;
		unsigned int bit4:1;
		unsigned int bit3:1;
		unsigned int bit2:1;
		unsigned int bit1:1;
		unsigned int bit0:1;
	} mybits;
} UnionB;

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      nand_correct_data512                                                */
/* DESCRIPTION                                                               */
/*      This function compares two ECCs and indicates if there is an error.  */
/* PARAMETERS                                                                */
/*      read_ecc           one ECC to be compared                            */
/*      calc_ecc           the other ECC to be compared                      */
/*      dat                content of data page                              */
/* RETURN VALUES                                                             */
/*      Upon successful completion, compare_ecc returns 0.                   */
/*      Otherwise, corresponding error code is returned.                     */
/*                                                                           */
/*****************************************************************************/
int nand_correct_data512(struct mtd_info *mtd, u_char *dat, u_char *read_ecc,
			 u_char *calc_ecc)
{
	unsigned short i, j, row, col;
	UnionLW mylw;
	unsigned short bitlen = 24;	/* e.g. 512 byte pages */

	u_char newval;

	UnionLW orgecc;
	UnionLW newecc;

	orgecc.mylword = 0;
	newecc.mylword = 0;

	/* Hardware register 00 ecc[2] ecc[1] ecc[0],   MSB..LSB */
	/* OOB form  ecc[0] ecc[1] ecc[2] */
	/* u_char form ecc[0] ecc[1] ecc[2] */
	orgecc.mybyte[1] = read_ecc[2];	/* MSB */
	orgecc.mybyte[2] = read_ecc[1];
	orgecc.mybyte[3] = read_ecc[0];	/* LSB */

	newecc.mybyte[1] = calc_ecc[2];
	newecc.mybyte[2] = calc_ecc[1];
	newecc.mybyte[3] = calc_ecc[0];

	mylw.mylword = orgecc.mylword ^ newecc.mylword;

	/* Quick check to avoid for loop below for normal no error case. */
	if (mylw.mylword == 0) {
		return 0;	/* No error */
	}

	j = 0;
	for (i = 0; i < bitlen; i++) {
		if ((mylw.mylword >> i) & 1) {
			j++;
		}
	}
	if (j == 1) {
		return 2;	/* ECC itself in error */
	}
	if (j == (bitlen >> 1)) {
		row = mylw.mybits.bit1 +
		    (mylw.mybits.bit3 << 0) +
		    (mylw.mybits.bit5 << 2) +
		    (mylw.mybits.bit7 << 3) +
		    (mylw.mybits.bit9 << 4) +
		    (mylw.mybits.bit11 << 5) +
		    (mylw.mybits.bit13 << 6) +
		    (mylw.mybits.bit15 << 7) + (mylw.mybits.bit17 << 8);

		col = mylw.mybits.bit19 +
		    (mylw.mybits.bit21 << 1) + (mylw.mybits.bit23 << 2);

		newval = dat[row] ^ (1 << col);
		/* printk("ECC: Replaced at offset 0x%x  old=0x%x new=0x%x\n", row, dat[row], newval); */
		dat[row] = newval;

		return 1;	/* Corrected 1 error */
	}
	return -1;		/* 2 or more errors - uncorrectible */
}
