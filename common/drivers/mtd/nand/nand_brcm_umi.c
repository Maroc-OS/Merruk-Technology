/*
 *    This driver was based on the sharpl NAND driver written by
 *    Richard Purdie.  Their copyright is below.
 *    As such, this driver falls under the GPL license also below.
 *
 *  Copyright (C) 2004 Richard Purdie
 *
 *  Based on Sharp's NAND driver sharp_sl.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/mtd/nand/nand_bcm_umi.c
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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc512.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include <asm/io.h>
#include <asm/mach-types.h>

#include <mach/reg_nand.h>
#include <mach/reg_umi.h>

#include <plat/dma.h>

#define USE_DMA			1
#define NAND_ECC_NUM_BYTES	3

/*  Register offsets */
#define REG_NAND_CMD_OFFSET     (0x0)
#define REG_NAND_ADDR_OFFSET    (0x4)
#define REG_NAND_DATA_OFFSET	(0x8)

/*
 * MTD structure for BCM UMI
 */
typedef struct _tagOOB {
	union {
		struct {
			uint32_t dwReserved1;
			uint8_t bReserved2;
			uint8_t bBlockStatus;
			uint8_t ecc0;
			uint8_t ecc1;
			uint8_t ecc2;
			uint8_t s_ecc[2];
			uint8_t dwReserved3[5];
		} s;
		uint8_t dummy[16];
	} u;
} NAND_OOB_S;

static struct mtd_info *board_mtd, *smtd;
static void __iomem *bcm_umi_io_base;

static uint8_t scan_ff_pattern[] = { 0xff, 0xff };

static struct nand_bbt_descr smallpage_bbt = {
	.options = NAND_BBT_SCAN2NDPAGE,
	.offs = 5,
	.len = 1,
	.pattern = scan_ff_pattern
};

static struct nand_bbt_descr largepage_bbt = {
	.options = 0,
	.offs = 0,
	.len = 1,
	.pattern = scan_ff_pattern
};

/*
 * nand_hw_eccoob
 * New oob placement block for use with hardware ecc generation.
 */
static struct nand_ecclayout nand_hw_eccoob_512 = {
	.eccbytes = 3,
	.eccpos = {6, 7, 8},
	/* Reserve 5 for BI indicator */
	.oobavail = 10,
	.oobfree = {
		    {.offset = 0, .length = 4},
		    {.offset = 10, .length = 6}
		    }
};

/*
** We treat the OOB for a 2K page as if it were 4 512 byte oobs,
** except the BI is at byte 0.
*/
static struct nand_ecclayout nand_hw_eccoob_2048 = {
	.eccbytes = 12,
	.eccpos = {8, 9, 10, 24, 25, 26, 40, 41, 42, 56, 57, 58},
	/* Reserve 0 as BI indicator */
	.oobavail = 44,
	.oobfree = {
		    {.offset = 2, .length = 4},
		    {.offset = 12, .length = 12},
		    {.offset = 28, .length = 12},
		    {.offset = 44, .length = 12},
		    {.offset = 60, .length = 4},
		    }
};

/* We treat the OOB for a 4K page as if it were 8 512 byte oobs,
 * except the BI is at byte 0. */
static struct nand_ecclayout nand_hw_eccoob_4096 = {
	.eccbytes = 24,
	.eccpos = {8, 9, 10, 24, 25, 26, 40, 41, 42, 56, 57, 58,
		   72, 73, 74, 88, 89, 90, 104, 105, 106, 120, 121, 122},
	/* Reserve 0 as BI indicator */
	.oobavail = 88,
	.oobfree = {
		    {.offset = 2, .length = 4},
		    {.offset = 12, .length = 12},
		    {.offset = 28, .length = 12},
		    {.offset = 44, .length = 12},
		    {.offset = 60, .length = 12},
		    {.offset = 76, .length = 12},
		    {.offset = 92, .length = 12},
		    {.offset = 108, .length = 12},
		    }
};

/**
 * yaf_nand_oob_64 - oob info for large (2KB) page
 */
static struct nand_ecclayout yaf_nand_oob_64 = {
	.eccbytes = 40,
	.eccpos = {
		   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
		   20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		   46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
		   },
	.oobavail = 16,
	.oobfree = {
		    {.offset = 30, .length = 16},
		    }
};

/* Preallocate a buffer to avoid having to do this every dma operation. */
/* This is the size of the preallocated coherent DMA buffer. */
#if USE_DMA
#define DMA_MIN_BUFLEN          512
#define DMA_MAX_BUFLEN          PAGE_SIZE
#define USE_DIRECT_IO(len)      \
	(((len) < DMA_MIN_BUFLEN) || ((len) > DMA_MAX_BUFLEN))
#define NAND_DMA_CHAN   3

/* Global config is common for both directions. */
#define DMA_CFG                                 \
    (REG_DMA_CHAN_CFG_TC_INT_ENABLE            \
    | REG_DMA_CHAN_CFG_ERROR_INT_ENABLE         \
    | REG_DMA_CHAN_CFG_FLOW_CTL_MEM_TO_MEM_DMA  \
    | REG_DMA_CHAN_CFG_ENABLE)

/* Common transfer widths in bits (typically 8, 16, or 32) */
#define DMA_WIDTH(dstwidth, srcwidth)           \
    (REG_DMA_CHAN_CTL_DEST_WIDTH_##dstwidth    \
    | REG_DMA_CHAN_CTL_SRC_WIDTH_##srcwidth)

/* Common burst sizes - typically 4 */
#define DMA_BURST(width)                        \
    (REG_DMA_CHAN_CTL_DEST_BURST_SIZE_##width  \
    | REG_DMA_CHAN_CTL_SRC_BURST_SIZE_##width)

/* DMA settings for copying from NAND to SDRAM */
#define DMA_CTRL_NAND_TO_SDRAM(dstwidth, srcwidth, bytes)    \
    (REG_DMA_CHAN_CTL_TC_INT_ENABLE            \
    | REG_DMA_CHAN_CTL_DEST_INCR                \
    | DMA_BURST(4)                              \
    | DMA_WIDTH(dstwidth, srcwidth)             \
    | (bytes * 8 / srcwidth))

/* DMA settings for copying from SDRAM to NAND */
#define DMA_CTRL_SDRAM_TO_NAND(dstwidth, srcwidth, bytes)    \
    (REG_DMA_CHAN_CTL_TC_INT_ENABLE            \
    | REG_DMA_CHAN_CTL_SRC_INCR                 \
    | DMA_BURST(4)                              \
    | DMA_WIDTH(dstwidth, srcwidth)             \
    | (bytes * 8 / srcwidth))

static void *virtPtr;
static dma_addr_t physPtr;

static int nand_dma_init(void)
{
	dma_request_chan(NAND_DMA_CHAN, "nand");

	virtPtr =
	    dma_alloc_coherent(NULL, DMA_MAX_BUFLEN, &physPtr, GFP_KERNEL);
	if (virtPtr == NULL) {
		pr_info("NAND - Failed to allocate memory for DMA buffer\n");
		return -ENOMEM;
	}

	return 0;
}

static void nand_dma_term(void)
{
	dma_free_chan(NAND_DMA_CHAN);

	if (virtPtr != NULL) {
		dma_free_coherent(NULL, DMA_MAX_BUFLEN, virtPtr, physPtr);
	}
}

static void nand_dma_read(void *buf, int len)
{
	int dmactrl;
	dmactrl =
	    NAND_BUS_16BIT() ? DMA_CTRL_NAND_TO_SDRAM(32, 16,
						     len) :
	    DMA_CTRL_NAND_TO_SDRAM(32, 8, len);
	dma_init_chan(NAND_DMA_CHAN);
	dma_setup_chan(NAND_DMA_CHAN,
		       NAND_BUS_16BIT() ? REG_NAND_DATA16_PADDR :
		       REG_NAND_DATA8_PADDR, (int)physPtr, 0, dmactrl, DMA_CFG);
	dma_poll_chan(NAND_DMA_CHAN);
	if (buf != NULL) {
		memcpy(buf, virtPtr, len);
	}
}

static void nand_dma_write(const void *buf, int len)
{
	int dmactrl;
	memcpy(virtPtr, buf, len);
	dmactrl =
	    NAND_BUS_16BIT() ? DMA_CTRL_SDRAM_TO_NAND(16, 32,
						     len) :
	    DMA_CTRL_SDRAM_TO_NAND(8, 32, len);
	dma_init_chan(NAND_DMA_CHAN);
	dma_setup_chan(NAND_DMA_CHAN,
		       (int)physPtr,
		       NAND_BUS_16BIT() ? REG_NAND_DATA16_PADDR :
		       REG_NAND_DATA8_PADDR, 0, dmactrl, DMA_CFG);
	dma_poll_chan(NAND_DMA_CHAN);
}

#else
#define USE_DIRECT_IO(len)      1
#endif

static int nand_dev_ready(struct mtd_info *mtd)
{
	return REG_UMI_NAND_RCSR & REG_UMI_NAND_RCSR_RDY;
}

/* Used to turn latch the proper register for access. */
static void bcm_umi_nand_hwcontrol(struct mtd_info *mtd, int cmd,
				   unsigned int ctrl)
{
	/* send command to hardware */
	struct nand_chip *chip = mtd->priv;
	if (ctrl & NAND_CTRL_CHANGE) {
		if (ctrl & NAND_CLE) {
			chip->IO_ADDR_W = bcm_umi_io_base + REG_NAND_CMD_OFFSET;
			goto CMD;
		}
		if (ctrl & NAND_ALE) {
			chip->IO_ADDR_W =
			    bcm_umi_io_base + REG_NAND_ADDR_OFFSET;
			goto CMD;
		}
		chip->IO_ADDR_W = bcm_umi_io_base + REG_NAND_DATA_OFFSET;
	}

CMD:
	/* Send command to chip directly */
	if (cmd != NAND_CMD_NONE)
		writeb(cmd, chip->IO_ADDR_W);
}

static int bcm_umi_nand_get_hw_ecc(struct mtd_info *mtd,
				   const u_char *dat, u_char *ecc_code)
{
	unsigned long ecc;
	while (!(REG_UMI_NAND_ECC_CSR & REG_UMI_NAND_ECC_CSR_ECCINT_RAW))
		udelay(1);
	REG_UMI_NAND_ECC_CSR |= REG_UMI_NAND_ECC_CSR_ECCINT_RAW;
	ecc = REG_UMI_NAND_ECC_DATA;
	ecc_code[2] = (ecc >> 16) & 0xff;
	ecc_code[1] = (ecc >> 8) & 0xff;
	ecc_code[0] = (ecc >> 0) & 0xff;

	return 0;
}

static void bcm_umi_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	/* Disable and reset ECC, 512 byte page */
	REG_UMI_NAND_ECC_CSR &= ~(REG_UMI_NAND_ECC_CSR_ECC_ENABLE
				  | REG_UMI_NAND_ECC_CSR_256BYTE
				  | REG_UMI_NAND_ECC_CSR_RBINT_RAW
				  | REG_UMI_NAND_ECC_CSR_ECCINT_RAW);
	/* Enable ECC */
	REG_UMI_NAND_ECC_CSR |= REG_UMI_NAND_ECC_CSR_ECC_ENABLE
	    | REG_UMI_NAND_ECC_CSR_ECCINT_ENABLE;
}

#ifdef CONFIG_MTD_PARTITIONS
const char *part_probes[] = { "cmdlinepart", NULL };
#endif

/**
 * nand_write_buf - write buffer to chip
 * @mtd:	MTD device structure
 * @buf:	data buffer
 * @len:	number of bytes to write
 *
 * Default write function for 8bit buswith
 */
static void bcm_umi_nand_write_buf(struct mtd_info *mtd, const u_char * buf,
				   int len)
{
	if (USE_DIRECT_IO(len)) {
		/* Do it the old way if the buffer is small or too large.
		 * Probably quicker than starting and checking dma. */
		int i;
		struct nand_chip *this = mtd->priv;

		for (i = 0; i < len; i++)
			writeb(buf[i], this->IO_ADDR_W);
	}
#if USE_DMA
	else {
		nand_dma_write(buf, len);
	}
#endif
}

/**
 * nand_read_buf - read chip data into buffer
 * @mtd:	MTD device structure
 * @buf:	buffer to store date
 * @len:	number of bytes to read
 *
 * Default read function for 8bit buswith
 */
static void bcm_umi_nand_read_buf(struct mtd_info *mtd, u_char * buf, int len)
{
	if (USE_DIRECT_IO(len)) {
		int i;
		struct nand_chip *this = mtd->priv;

		for (i = 0; i < len; i++)
			buf[i] = readb(this->IO_ADDR_R);
	}
#if USE_DMA
	else {
		nand_dma_read(buf, len);
	}
#endif
}

/**
 * bcm_umi_nand_verify_buf - Verify chip data against buffer
 * @mtd:	MTD device structure
 * @buf:	buffer containing the data to compare
 * @len:	number of bytes to compare
 *
 * Default verify function for 8bit buswith
 */
static int bcm_umi_nand_verify_buf(struct mtd_info *mtd, const u_char * buf,
				   int len)
{
	if (USE_DIRECT_IO(len)) {
		int i;
		struct nand_chip *this = mtd->priv;
		for (i = 0; i < len; i++) {
			if (buf[i] != readb(this->IO_ADDR_R)) {
				return -EFAULT;
			}
		}
	}
#if USE_DMA
	else {
		nand_dma_read(NULL, len);
		if (memcmp(buf, virtPtr, len) != 0) {
			return -EFAULT;
		}
	}
#endif
	return 0;
}

/**
 * bcm_umi_nand_write_buf16 - write buffer to chip
 * @mtd:	MTD device structure
 * @buf:	data buffer
 * @len:	number of bytes to write
 *
 * Default write function for 16bit buswith
 */
static void bcm_umi_nand_write_buf16(struct mtd_info *mtd, const u_char * buf,
				     int len)
{
	if (USE_DIRECT_IO(len)) {
		int i;
		struct nand_chip *this = mtd->priv;
		u16 *p = (u16 *) buf;
		len >>= 1;

		for (i = 0; i < len; i++)
			writew(p[i], this->IO_ADDR_W);
	}
#if USE_DMA
	else {
		nand_dma_write(buf, len);
	}
#endif
}

/**
 * bcm_umi_nand_read_buf16 - read chip data into buffer
 * @mtd:	MTD device structure
 * @buf:	buffer to store date
 * @len:	number of bytes to read
 *
 * Default read function for 16bit buswith
 */
static void bcm_umi_nand_read_buf16(struct mtd_info *mtd, u_char * buf, int len)
{
	if (USE_DIRECT_IO(len)) {
		int words;
		int i;
		struct nand_chip *this = mtd->priv;
		u16 *p = (u16 *) buf;

		words = (len + 1) / 2;	/* e.g len may be odd. 1 byte must do 1 word read */

		for (i = 0; i < words; i++) {
			p[i] = readw(this->IO_ADDR_R);
		}
	}
#if USE_DMA
	else {
		nand_dma_read(buf, len);
	}
#endif
}

/**
 * bcm_umi_nand_verify_buf16 - Verify chip data against buffer
 * @mtd:	MTD device structure
 * @buf:	buffer containing the data to compare
 * @len:	number of bytes to compare
 *
 * Default verify function for 16bit buswith
 */
static int bcm_umi_nand_verify_buf16(struct mtd_info *mtd, const u_char * buf,
				     int len)
{
	if (USE_DIRECT_IO(len)) {
		int i;
		struct nand_chip *this = mtd->priv;
		u16 *p = (u16 *) buf;
		len >>= 1;

		for (i = 0; i < len; i++)
			if (p[i] != readw(this->IO_ADDR_R))
				return -EFAULT;
	}
#if USE_DMA
	else {
		nand_dma_read(NULL, len);
		if (memcmp(buf, virtPtr, len) != 0) {
			return -EFAULT;
		}
	}
#endif
	return 0;
}

void bcm_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	uint32_t l = 0, loopCount = 0;

	/* temp buffer for storing the spare info read from NAND device */
	NAND_OOB_S temp;
	struct mtd_oob_ops kops;
	uint32_t *pTag = NULL;
	kops.mode = MTD_OOB_RAW;
	kops.len = 0;
	kops.ooblen = 16;
	kops.ooboffs = 0;
	kops.datbuf = NULL;

	/* cast the spare buffer into 32-bit word for easy access */
	/* the tag info */
	pTag = (uint32_t *) ops->oobbuf;

	/* this is small page NAND device */
	if (board_mtd->writesize == 0x200) {
		loopCount = 4;
	}
	for (l = 0; l < loopCount; l++) {
		/* reset the buffer */
		memset((uint8_t *) &temp, 0, sizeof(NAND_OOB_S));
		kops.oobbuf = (char *)&temp;
		/* board level NAND device read */
		board_mtd->read_oob(board_mtd, from, &kops);
		pTag[l] = temp.u.s.dwReserved1;
		/* move to next page */
		from += board_mtd->writesize;	/* assume the write size is 512 bytes */
	}
}

void bcm_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	char temp[16], l;
	struct mtd_oob_ops kops;
	kops.mode = MTD_OOB_RAW;
	kops.len = 0;
	kops.ooblen = 16;
	kops.ooboffs = 0;
	kops.datbuf = NULL;
	for (l = 0; l < 4; l++) {
		memcpy(temp, ops->oobbuf, sizeof(unsigned long));
		kops.oobbuf = temp;
		board_mtd->write_oob(board_mtd, to, &kops);
		to += board_mtd->writesize;
		ops->oobbuf += sizeof(unsigned long);
	}
}

static int yaf_nand_read_oob(struct mtd_info *mtd, loff_t from,
			     struct mtd_oob_ops *ops)
{
	int retlen;

	if (ops->len != 0 && ops->datbuf != NULL) {
		board_mtd->read(board_mtd, from, ops->len, &retlen,
				ops->datbuf);
	}

	bcm_read_oob(mtd, from, ops);
	return 0;
}

static int yaf_nand_write_oob(struct mtd_info *mtd, loff_t to,
			      struct mtd_oob_ops *ops)
{
	int retlen;

	if (ops->len != 0 && ops->datbuf != NULL) {
		board_mtd->write(board_mtd, to, ops->len, &retlen, ops->datbuf);
	}
	bcm_write_oob(mtd, to, ops);
	return 0;
}

/*
 * The generic flash bbt decriptors overlap with our ecc
 * hardware, so define some Broadcom specific ones.
 */
static uint8_t bbt_pattern[] = { 'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = { '1', 't', 'b', 'B' };

static struct nand_bbt_descr bbt_main_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
	    | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs = 10,
	.len = 4,
	.veroffs = 14,
	.maxblocks = 4,
	.pattern = bbt_pattern,
};

static struct nand_bbt_descr bbt_mirror_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
	    | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs = 10,
	.len = 4,
	.veroffs = 14,
	.maxblocks = 4,
	.pattern = mirror_pattern,
};

static int __devinit bcm_umi_nand_probe(struct platform_device *pdev)
{
	struct nand_chip *chip;
	struct resource *r;
	int err = 0;
	int busw;

	/* Allocate memory for MTD device structure and private data */
	board_mtd = kmalloc(sizeof(struct mtd_info)
			    + sizeof(struct nand_chip), GFP_KERNEL);
	if (!board_mtd) {
		pr_warning(
		       "Unable to allocate NAND MTD device structure.\n");
		return -ENOMEM;
	}

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r)
		return -ENXIO;

	/* map physical address */
	bcm_umi_io_base = ioremap(r->start, r->end - r->start + 1);

	if (!bcm_umi_io_base) {
		pr_err("ioremap to access BCM UMI NAND chip failed\n");
		kfree(board_mtd);
		return -EIO;
	}

	/* Get pointer to private data */
	chip = (struct nand_chip *)(&board_mtd[1]);

	/* Initialize structures */
	memset((char *)board_mtd, 0, sizeof(struct mtd_info));
	memset((char *)chip, 0, sizeof(struct nand_chip));

	/* Link the private data with the MTD structure */
	board_mtd->priv = chip;

	/* Set address of NAND IO lines */
	chip->IO_ADDR_W = bcm_umi_io_base + REG_NAND_DATA_OFFSET;
	chip->IO_ADDR_R = bcm_umi_io_base + REG_NAND_DATA_OFFSET;

	/* Set command delay time, see datasheet for correct value */
	chip->chip_delay = 0;
	/* Assign the device ready function, if available */
	chip->dev_ready = nand_dev_ready;
	chip->options = (NAND_BUS_16BIT()) ? NAND_BUSWIDTH_16 : 0;

	busw = chip->options & NAND_BUSWIDTH_16;
	chip->write_buf = busw ? bcm_umi_nand_write_buf16
	    : bcm_umi_nand_write_buf;
	chip->read_buf = busw ? bcm_umi_nand_read_buf16 : bcm_umi_nand_read_buf;
	chip->verify_buf = busw ? bcm_umi_nand_verify_buf16
	    : bcm_umi_nand_verify_buf;

	chip->cmd_ctrl = bcm_umi_nand_hwcontrol;
	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.size = 512;
	chip->ecc.bytes = NAND_ECC_NUM_BYTES;
	chip->ecc.correct = nand_correct_data512;
	chip->ecc.calculate = bcm_umi_nand_get_hw_ecc;
	chip->ecc.hwctl = bcm_umi_nand_enable_hwecc;

#if USE_DMA
	err = nand_dma_init();
	if (err != 0)
		return err;
#endif

	err = nand_scan_ident(board_mtd, 1, NULL);
	if (err) {
		pr_err("nand_scan failed: %d\n", err);
		iounmap(bcm_umi_io_base);
		kfree(board_mtd);
		return err;
	}

	/* Now that we know the nand size, we can setup the ECC layout */

	switch (board_mtd->writesize) {	/* writesize is the pagesize */
	case 4096:
		chip->ecc.layout = &nand_hw_eccoob_4096;
		break;
	case 2048:
		chip->ecc.layout = &nand_hw_eccoob_2048;
		break;
	case 512:
		chip->ecc.layout = &nand_hw_eccoob_512;
		break;
	default:
		{
			pr_err("NAND - Unrecognized pagesize: %d\n",
			       board_mtd->writesize);
			return -EINVAL;
		}
	}
	if (board_mtd->writesize > 512) {
		bbt_main_descr.offs = 2;
		bbt_main_descr.veroffs = 6;
		bbt_mirror_descr.offs = 2;
		bbt_mirror_descr.veroffs = 6;
		chip->badblock_pattern = &largepage_bbt;
	} else {
		chip->badblock_pattern = &smallpage_bbt;
	}

	chip->bbt_td = &bbt_main_descr;
	chip->bbt_md = &bbt_mirror_descr;
	chip->options |= NAND_USE_FLASH_BBT;
	/* Now finish off the scan, now that ecc has been initialized. */
	err = nand_scan_tail(board_mtd);
	if (err) {
		pr_err("nand_scan failed: %d\n", err);
		iounmap(bcm_umi_io_base);
		kfree(board_mtd);
		return err;
	}

	/* Register the partitions */
	{
		int nr_partitions;
		struct mtd_partition *partition_info;

		board_mtd->name = "bcm_umi-nand";
		nr_partitions = parse_mtd_partitions(board_mtd, part_probes,
						     &partition_info, 0);

		if (nr_partitions <= 0) {
			pr_err("BCM NAND: Too few partitions-%d\n",
			       nr_partitions);
			iounmap(bcm_umi_io_base);
			kfree(board_mtd);
			return -EIO;
		}

		smtd = kmalloc(sizeof(struct mtd_info)
			       + sizeof(struct nand_chip), GFP_KERNEL);

		memcpy(smtd, board_mtd,
		       sizeof(struct mtd_info) + sizeof(struct nand_chip));

		if (board_mtd->writesize == 512) {
			smtd->writesize = 2048;
			smtd->oobsize = yaf_nand_oob_64.eccbytes
			    + yaf_nand_oob_64.oobavail;
			smtd->oobavail = yaf_nand_oob_64.oobavail;

			/* Fill in remaining MTD driver data */
			smtd->read_oob = yaf_nand_read_oob;
			smtd->write_oob = yaf_nand_write_oob;
		}

		add_mtd_partitions(smtd, partition_info, nr_partitions);
	}
	/* Return happy */
	return 0;
}

static int bcm_umi_nand_remove(struct platform_device *pdev)
{
#if USE_DMA
	nand_dma_term();
#endif
	/* Release resources, unregister device */
	nand_release(board_mtd);

	/* unmap physical address */
	iounmap(bcm_umi_io_base);

	/* Free the MTD device structure */
	kfree(board_mtd);

	return 0;
}

#ifdef CONFIG_PM
static int bcm_umi_nand_suspend(struct platform_device *pdev,
				pm_message_t state)
{
	return 0;
}

static int bcm_umi_nand_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define bcm_umi_nand_suspend   NULL
#define bcm_umi_nand_resume    NULL
#endif

static struct platform_driver nand_driver = {
	.driver = {
		   .name = "bcm-nand",
		   .owner = THIS_MODULE,
		   },
	.probe = bcm_umi_nand_probe,
	.remove = bcm_umi_nand_remove,
	.suspend = bcm_umi_nand_suspend,
	.resume = bcm_umi_nand_resume,
};

static int __init nand_init(void)
{
	return platform_driver_register(&nand_driver);
}

static void __exit nand_exit(void)
{
	platform_driver_unregister(&nand_driver);
}

module_init(nand_init);
module_exit(nand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("BCM UMI MTD NAND driver");
