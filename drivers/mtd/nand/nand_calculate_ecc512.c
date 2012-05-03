/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/mtd/nand/nand_calculate_ecc512.c
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
/* nand ecc calculation - only useful for mips cpu's that have no ECC */
/* generation hardware. */
/* --------------------------------------------------------------------------- */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
/* #include <linux/broadcom/knllog.h> */

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      nand_calculate_ecc512                                                         */
/* DESCRIPTION                                                               */
/*		This function generates 3 byte ECC for 512 byte data.                */
/*      (Software ECC)                                                       */
/* PARAMETERS                                                                */
/*      pEcc            the location where ECC should be stored              */
/*      datapage        given data                                           */
/* RETURN VALUES                                                             */
/*		none                                                                 */
/*                                                                           */
/*****************************************************************************/
#define NAND_PG_SIZE 512

/* For efficiency reasons, process the data as 32-bit words, 4 bytes packed. */
/* The words are grouped into blocks which helps us calculate parity regions. */
/* The block size is set to 8 words so that we can calculate the larger */
/* parity regions, which starts at P256, which needs 8 words. So for a */
/* 512 byte page, we have to loop 16 times, 8 words per block to get 128 words, */
/* or 512 bytes. */
#define NAND_PAGE_WORDS (NAND_PG_SIZE / 4)
#define BYTES_PER_BLOCK	32
#define WORDS_PER_BLOCK	(BYTES_PER_BLOCK / 4)
#define BLOCKS (NAND_PAGE_WORDS / WORDS_PER_BLOCK)

/* Patterns which capture blocks of bytes */
/* of different sizes, when layed out as */
/* 32 bit words. This is more efficient */
/* than calculating parity on a byte by */
/* byte basis. */
#define P16_PATTERN		0xffff0000
#define P16P_PATTERN		(~P16_PATTERN)
#define P8_PATTERN		0xff00ff00
#define P8P_PATTERN		(~P8_PATTERN)
#define P4_PATTERN		0xf0f0f0f0
#define P4P_PATTERN		(~P4_PATTERN)
#define P2_PATTERN		0xcccccccc
#define P2P_PATTERN		(~P2_PATTERN)
#define P1_PATTERN		0xaaaaaaaa
#define P1P_PATTERN		(~P1_PATTERN)

/* These are bit positions within the largeBlockParity bitmap */
/* for each corresponding pattern. D0 corresponds to the parity */
/* of the first 32 byte block. D15 corresponds to the parity of */
/* the last 32 byte block. Combinations of these bits can be */
/* used to get the parity of each region. */
/* 0  P256' */
/* 1  P256   P512' */
/* 2  P256' */
/* 3  P256   P512   P1024' */
/* 4  P256' */
/* 5  P256   P512' */
/* 6  P256' */
/* 7  P256   P512'  P1024 */
/* ... */
#define P256_PATTERN   	0xaaaa
#define P512_PATTERN   	0xcccc
#define P1024_PATTERN   0xf0f0
#define P2048_PATTERN   0xff00
#define P256P_PATTERN  	0x5555
#define P512P_PATTERN  	0x3333
#define P1024P_PATTERN 	0x0f0f
#define P2048P_PATTERN 	0x00ff

/* Positions in the final ECC word */
#define P4		0x00800000
#define P4P		0x00400000
#define P2		0x00200000
#define P2P		0x00100000
#define P1		0x00080000
#define P1P		0x00040000
#define P2048	0x00020000
#define P2048P	0x00010000
#define P1024	0x00008000
#define P1024P	0x00004000
#define P512	0x00002000
#define P512P	0x00001000
#define P256	0x00000800
#define P256P	0x00000400
#define P128	0x00000200
#define P128P	0x00000100
#define P64		0x00000080
#define P64P	0x00000040
#define P32		0x00000020
#define P32P	0x00000010
#define P16		0x00000008
#define P16P	0x00000004
#define P8		0x00000002
#define P8P		0x00000001

struct mtd_info;

const u_char parity_nibble[16] = {
	0,			/* 0 */
	1,			/* 1 */
	1,			/* 2 */
	0,			/* 3 */
	1,			/* 4 */
	0,			/* 5 */
	0,			/* 6 */
	1,			/* 7 */
	1,			/* 8 */
	0,			/* 9 */
	0,			/* 0xa */
	1,			/* 0xb */
	0,			/* 0xc */
	1,			/* 0xd */
	1,			/* 0xe */
	0			/* 0xf */
};

typedef struct {
	uint32_t largeBlockParities;	/* bitmap of parities for blocks 0-15 */
	uint32_t pBlock;	/* parity of current block */
	uint32_t result;	/* runnning ecc result */
	uint32_t p16ByteBlockOdd;	/* 16 byte odd block parity */
	uint32_t p8ByteBlockOdd;	/* 8 byte odd block parity */
	uint32_t p4ByteBlockOdd;	/* 4 byte odd block parity */
	uint32_t p16ByteBlockEven;	/* 16 byte even block parity */
	uint32_t p8ByteBlockEven;	/* 8 byte even block parity */
	uint32_t p4ByteBlockEven;	/* 4 byte even block parity */
	uint32_t pFinal;	/* parity of 512 byte packet */
	uint32_t *wordp;	/* ptr to aligned start of packet */
} ECCINFO;

/* Calculate the parity of a word (xor all bits in the word) */
/* Returns parity 0 or 1. */
static inline int wordParity(uint32_t word)
{
	uint32_t val;
	val = (word >> 16) ^ word;
	val = (val >> 8) ^ val;
	val = (val >> 4) ^ val;
	return parity_nibble[val & 0xf];

}

/* Calculate the parity of a word and return the bitmask */
/* if nonzero parity or zero if zero parity. */
static inline uint32_t calcEccBit(uint32_t word, uint32_t bitmask)
{
	return wordParity(word) ? bitmask : 0;
}

/* Main ecc calculation routine */
int nand_calculate_ecc512(struct mtd_info *mtd, const u_char *dat,
			  u_char *ecc_code)
{
	int blockidx;		/* 16 blocks of 8 longwords = 512 bytes */
	int wordidx;		/* index within each block 0-7 */
	uint32_t alignedBuf[NAND_PG_SIZE / sizeof(uint32_t)];
	ECCINFO ecc;

	/* KNLLOG("start\n"); */
	memset(&ecc, 0, sizeof(ecc));

	/* Make sure the data is aligned. */
	ecc.wordp = (uint32_t *) dat;
	if (((uint32_t) dat % 4) != 0) {
		memcpy((char *)alignedBuf, dat, NAND_PG_SIZE);
		ecc.wordp = alignedBuf;	/* 32-bit operations more efficient */
	}
	/* The data needs to be in little-endian order for the ECC calculations */
	/* so we have to swap it for big-endian architectures. */
	for (blockidx = 0; blockidx < BLOCKS; blockidx++) {
		ecc.pBlock = 0;	/* Reset the parity each block */
		for (wordidx = 0; wordidx < WORDS_PER_BLOCK; wordidx++) {
			uint32_t word = cpu_to_le32(*ecc.wordp++);
			ecc.pBlock ^= word;	/* update the running word parity per block */
			ecc.p16ByteBlockEven ^= !(wordidx & 4) ? word : 0;	/* Every even block of 16 bytes */
			ecc.p8ByteBlockEven ^= !(wordidx & 2) ? word : 0;	/* Every even block of 8 bytes */
			ecc.p4ByteBlockEven ^= !(wordidx & 1) ? word : 0;	/* Every even block of 4 bytes */
			ecc.p16ByteBlockOdd ^= (wordidx & 4) ? word : 0;	/* Every odd block of 16 bytes */
			ecc.p8ByteBlockOdd ^= (wordidx & 2) ? word : 0;	/* Every odd block of 8 bytes */
			ecc.p4ByteBlockOdd ^= (wordidx & 1) ? word : 0;	/* Every odd block of 4 bytes */
		}
		/* pFinal is the total parity over all blocks and all data */
		ecc.pFinal ^= ecc.pBlock;

		/* If the word parity is set for this block, then update */
		/* a bitmap that shows the parity over all 16 blocks. */
		if (wordParity(ecc.pBlock)) {
			ecc.largeBlockParities |= (1 << blockidx);
		}
	}
	/* Calculate the 256/512/1024/2048 bit region parities */
	ecc.result |= calcEccBit(ecc.largeBlockParities & P2048_PATTERN, P2048);
	ecc.result |= calcEccBit(ecc.largeBlockParities & P1024_PATTERN, P1024);
	ecc.result |= calcEccBit(ecc.largeBlockParities & P512_PATTERN, P512);
	ecc.result |= calcEccBit(ecc.largeBlockParities & P256_PATTERN, P256);

	ecc.result |=
	    calcEccBit(ecc.largeBlockParities & P2048P_PATTERN, P2048P);
	ecc.result |=
	    calcEccBit(ecc.largeBlockParities & P1024P_PATTERN, P1024P);
	ecc.result |= calcEccBit(ecc.largeBlockParities & P512P_PATTERN, P512P);
	ecc.result |= calcEccBit(ecc.largeBlockParities & P256P_PATTERN, P256P);

	/* Using the 32/64/128 bit region parities */
	ecc.result |= calcEccBit(ecc.p16ByteBlockOdd, P128);
	ecc.result |= calcEccBit(ecc.p8ByteBlockOdd, P64);
	ecc.result |= calcEccBit(ecc.p4ByteBlockOdd, P32);

	ecc.result |= calcEccBit(ecc.p16ByteBlockEven, P128P);
	ecc.result |= calcEccBit(ecc.p8ByteBlockEven, P64P);
	ecc.result |= calcEccBit(ecc.p4ByteBlockEven, P32P);

	/* Calculate the column parities */
	ecc.result |= calcEccBit(ecc.pFinal & P16_PATTERN, P16);
	ecc.result |= calcEccBit(ecc.pFinal & P8_PATTERN, P8);
	ecc.result |= calcEccBit(ecc.pFinal & P4_PATTERN, P4);
	ecc.result |= calcEccBit(ecc.pFinal & P2_PATTERN, P2);
	ecc.result |= calcEccBit(ecc.pFinal & P1_PATTERN, P1);

	ecc.result |= calcEccBit(ecc.pFinal & P16P_PATTERN, P16P);
	ecc.result |= calcEccBit(ecc.pFinal & P8P_PATTERN, P8P);
	ecc.result |= calcEccBit(ecc.pFinal & P4P_PATTERN, P4P);
	ecc.result |= calcEccBit(ecc.pFinal & P2P_PATTERN, P2P);
	ecc.result |= calcEccBit(ecc.pFinal & P1P_PATTERN, P1P);

	/* Invert the result */
	ecc.result = ~ecc.result;

	/* Fill in the return codes */
	ecc_code[0] = ecc.result & 0xff;
	ecc_code[1] = (ecc.result >> 8) & 0xff;
	ecc_code[2] = (ecc.result >> 16) & 0xff;

	/* KNLLOG("done ecc=%x\n", ecc.result & 0xffffff); */
	return 0;
}
