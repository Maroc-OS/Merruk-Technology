/*
 * Broadcom ARM PrimeCell PL353 controller (NAND controller) driver
 *
 * Copyright (c) 2011-2012 Broadcom Corporation
 *
 * Authors: Sundar Jayakumar Dev <sundarjdev@broadcom.com>
 */

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/mtd/nand/nand_brcm_nvsram.c
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

#include <mach/reg_nvsram.h>

#include <plat/dma.h>

#define NAND_ECC_NUM_BYTES	3
#define NAND_ECC_PAGE_SIZE 512

static struct mtd_info *board_mtd; 
static void __iomem *bcm_nand_io_base;
static nvsram_cmd addr;
static uint8_t chip_sel;

#ifdef CONFIG_MTD_PARTITIONS
const char *part_probes[] = { "cmdlinepart", NULL };
#endif

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

#if defined (CONFIG_MTD_USE_NAND_FLASH_ON_DIE_ECC)
/*
 * OOB layout for Micron on-die ECC
 */
static struct nand_ecclayout micron_nand_on_die_eccoob_2048 = {
	.eccbytes = 32,
	.eccpos = {
			8,  9,  10, 11, 12, 13, 14, 15,
			24, 25, 26, 27, 28, 29, 30, 31,
			40, 41, 42, 43, 44, 45, 46, 47,
			56, 57, 58, 59, 60, 61, 62, 63
		},
	/* Reserve 0 as BI indicator */
	.oobavail = 16,
	.oobfree = {
		    {.offset = 4, .length = 4},
		    {.offset = 20, .length = 4},
		    {.offset = 36, .length = 4},
		    {.offset = 52, .length = 4},
	   }
};
#endif

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

static uint8_t brcm_nand_read_byte(struct mtd_info *mtd)
{
	return readb(addr.data_phase_addr);
}

static uint8_t brcm_nand_read_byte16(struct mtd_info *mtd)
{
	return (uint8_t) cpu_to_le16(readw(addr.data_phase_addr));
}

static uint16_t brcm_nand_read_word(struct mtd_info *mtd)
{
	return readw(addr.data_phase_addr);
}

static void brcm_nand_write_buf16(struct mtd_info *mtd,
				const u_char *buf, int len)
{
	int i;
	u16 *p = (u16 *) buf;
	len >>= 1;

	for (i = 0; i < len; i++)
		writew(p[i], addr.data_phase_addr);
}

static void brcm_nand_write_buf(struct mtd_info *mtd,
				const u_char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		writeb(buf[i], addr.data_phase_addr);
}

extern void	FastNANDAXIReadData(uint32_t *, int, uint32_t);

static void brcm_nand_read_buf16(struct mtd_info *mtd,
				u_char *buf, int len)
{
#if 0
	int words;
	int i;
	u16 *p = (u16 *) buf;

	words = (len + 1) / 2;	/* e.g len may be odd. 1 byte must do 1 word read */

	for (i = 0; i < words; i++) {
		p[i] = readw(addr.data_phase_addr);
	}
#endif

	uint32_t *p = (uint32_t *) buf;
	FastNANDAXIReadData(p, len, addr.data_phase_addr);
}

static void brcm_nand_read_buf(struct mtd_info *mtd,
				u_char *buf, int len)
{
#if 0
	int i;

	for (i = 0; i < len; i++)
		buf[i] = readb(addr.data_phase_addr);
#endif

	uint32_t *p = (uint32_t *) buf;
	FastNANDAXIReadData(p, len, addr.data_phase_addr);
}

static int brcm_nand_verify_buf16(struct mtd_info *mtd,
				const u_char *buf, int len)
{
	int i;
	u16 *p = (u16 *) buf;
	len >>= 1;

	for (i = 0; i < len; i++)
		if (p[i] != readw(addr.data_phase_addr))
			return -EFAULT;
	return 0;
}

static int brcm_nand_verify_buf(struct mtd_info *mtd,
				const u_char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		if (buf[i] != readb(addr.data_phase_addr)) {
			return -EFAULT;
		}
	}
	return 0;
}

static void brcm_nand_wait_until_ready(uint32_t busy_wait_us)
{
	uint32_t status = 0;
	unsigned long timeo = jiffies;

	timeo += (HZ * busy_wait_us) / 1000;

	/*
	 * First wait for the NAND device to go busy because R/#B
	 * signal may be slower than fast CPU and still be ready.
	 */
	while (time_before(jiffies, timeo)) {
		status = readl(NVSRAM_REG_BASE + NVSRAM_MEMC_STATUS_OFFSET);
		/* NAND R/#B monitored by NVSRAM interface-1 raw interrupt status */
		if (!(status & NVSRAM_MEMC_STATUS_RAW_INT_STATUS_MASK))
			break;
		cond_resched();
	}

	/* Then wait for the NAND device to go ready */
	do {
		status = readl(NVSRAM_REG_BASE + NVSRAM_MEMC_STATUS_OFFSET);
	} while (!(status & NVSRAM_MEMC_STATUS_RAW_INT_STATUS_MASK));

	/* Clear raw interrupt */
	writel(NVSRAM_MEMC_CFG_CLR_NAND_INT_CLR_MASK,
			NVSRAM_REG_BASE + NVSRAM_MEMC_CFG_CLR_OFFSET);
}

/*
 * brcm_nand_cmdfunc:
 * Our implementation of cmdfunc for NVSRAM NAND controller.
 *
 * Used by the upper layer to write command to NAND Flash for
 * different operations to be carried out on NAND Flash.
 */
static void brcm_nand_cmdfunc(struct mtd_info *mtd, unsigned int command,
				int column, int page_addr)
{
	struct nand_chip *chip = mtd->priv;
	int status, count;

	/* Initialize cmd and data phase addresses */
	addr.cmd_phase_addr = (uint32_t)bcm_nand_io_base;
	addr.data_phase_addr = (uint32_t)bcm_nand_io_base;

	pr_debug("brcm_nvsram_cmdfunc (cmd = 0x%x, col = 0x%x, page = 0x%x)\n",
	      command, column, page_addr);

	udelay(200);

	/* Command pre-processing step */
	switch (command) {
	case NAND_CMD_RESET:
		/* Construct cmd_phase_addr */
		addr.cmd_phase_addr |= 1 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE	|
								0x00 << NVSRAM_SMC_NAND_END_CMD		|
								NAND_CMD_RESET << NVSRAM_SMC_NAND_START_CMD;

		writel(0x0, addr.cmd_phase_addr);

		break;

	case NAND_CMD_STATUS:
		/* Construct cmd_phase_addr */
		addr.cmd_phase_addr |= 1 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE	|
								0x00 << NVSRAM_SMC_NAND_END_CMD		|
								NAND_CMD_STATUS << NVSRAM_SMC_NAND_START_CMD;

		writel(0x0, addr.cmd_phase_addr);

		/* Construct data_phase_addr */
		addr.data_phase_addr |= NVSRAM_SMC_CLEAR_CS 	|
								NVSRAM_SMC_RESERVED;

		break;

	case NAND_CMD_READOOB:
		/* Emulate NAND_CMD_READOOB as NAND_CMD_READ0 */
		column += mtd->writesize;
		command = NAND_CMD_READ0;

		/* Fall through */
	case NAND_CMD_READ0:
		/* Construct cmd_phase_addr */
		addr.cmd_phase_addr |= 5 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE				|
								NVSRAM_SMC_NAND_END_CMD_REQ						|
								NAND_CMD_READSTART << NVSRAM_SMC_NAND_END_CMD	|
								NAND_CMD_READ0 << NVSRAM_SMC_NAND_START_CMD;

		/*
		 * Input the address to the controller. 4 address cycles fit in
		 * 32-bit a double word and the last address cycle is sent as a
		 * seperate 32-bit double word. The first 2 cycles are column
		 * addresses and the next 3 cycles are page addresses.
		 *
		 * Read PL353 controller spec for more info.
		 *
		 * NOTE: the fifth address cycle is only for devices > 128MiB.
		 */
		if (column != -1) {
			/* Adjust columns for 16 bit buswidth */
			if (chip->options & NAND_BUSWIDTH_16)
				column >>= 1;
		}
		if (page_addr != -1) {
			writel((page_addr << 16) | column, addr.cmd_phase_addr);
			/* One more address cycle for devices > 128MiB */
			if (chip->chipsize > (128 << 20))
				writel(page_addr >> 16, addr.cmd_phase_addr);
		}

#if defined (CONFIG_MTD_USE_HW_NAND_CONTROLLER_ECC)
		/* Poll memc_status register for R/#B interrupt */
		brcm_nand_wait_until_ready(100);
#elif defined (CONFIG_MTD_USE_NAND_FLASH_ON_DIE_ECC)
		/* Micron NAND: Wait for STATUS_READY; tR_ECC max */
		for (count = 0; count < 70; count++) {
			ndelay(1000);	/* 1 micro sec delay */

			chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
			status = chip->read_byte(mtd);
			if (status & NAND_STATUS_READY)
				break;
		}

		/* Check for ECC errors */
		if (status & NAND_STATUS_FAIL) {
			pr_warn("ECC error on READ operation: 0x%02x\n", status);
			/* Increment error stats */
			mtd->ecc_stats.failed++;
		}

		/* Return the device to READ mode following a STATUS command */
		addr.cmd_phase_addr = (uint32_t)bcm_nand_io_base 						|
								1 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE				|
								NVSRAM_SMC_CLEAR_CS								|
								0x00 << NVSRAM_SMC_NAND_END_CMD					|
								NAND_CMD_READ0 << NVSRAM_SMC_NAND_START_CMD;

		writel(0x0, addr.cmd_phase_addr);
#endif

		/* Construct data_phase_addr */
		addr.data_phase_addr |= NVSRAM_SMC_CLEAR_CS	| NVSRAM_SMC_RESERVED;

		break;

	case NAND_CMD_SEQIN:
		/* Construct cmd_phase_addr */
		addr.cmd_phase_addr |= 5 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE				|
								0x0 << NVSRAM_SMC_NAND_END_CMD	|
								NAND_CMD_SEQIN << NVSRAM_SMC_NAND_START_CMD;

		/* Read comment above (READ0 command) */
		if (column != -1) {
			/* Adjust columns for 16 bit buswidth */
			if (chip->options & NAND_BUSWIDTH_16)
				column >>= 1;
		}
		if (page_addr != -1) {
			writel((page_addr << 16) | column, addr.cmd_phase_addr);
			/* One more address cycle for devices > 128MiB */
			if (chip->chipsize > (128 << 20))
				writel(page_addr >> 16, addr.cmd_phase_addr);
		}

		/* Construct data_phase_addr */
		addr.data_phase_addr |= NVSRAM_SMC_RESERVED;
		break;

	case NAND_CMD_PAGEPROG:
		/* Send NAND_CMD_PAGEPROG to end write transaction */
		/* Construct cmd_phase_addr */
		addr.cmd_phase_addr |= 1 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE	|
								NVSRAM_SMC_CLEAR_CS					|
								0x00 << NVSRAM_SMC_NAND_END_CMD		|
								NAND_CMD_PAGEPROG << NVSRAM_SMC_NAND_START_CMD;

		writel(0x0, addr.cmd_phase_addr);

		/* Poll memc_status register for R/#B interrupt */
		brcm_nand_wait_until_ready(100);
		break;

	case NAND_CMD_READID:
		/* Construct cmd_phase_addr */
		addr.cmd_phase_addr |= 1 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE	|
								NVSRAM_SMC_CLEAR_CS					|
								0x00 << NVSRAM_SMC_NAND_END_CMD		|
								NAND_CMD_READID << NVSRAM_SMC_NAND_START_CMD;

		writel(0x0, addr.cmd_phase_addr);

		/* Construct data_phase_addr */
		addr.data_phase_addr |= NVSRAM_SMC_RESERVED;

		break;

	case NAND_CMD_ERASE1:
		/* Construct cmd_phase_addr */
		addr.cmd_phase_addr |= 3 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE	|
								0x00 << NVSRAM_SMC_NAND_END_CMD		|
								NAND_CMD_ERASE1 << NVSRAM_SMC_NAND_START_CMD;

		/* Send page_addr(PA0 - PA17) of the block to be erased to SMC */
		writel(page_addr & 0x3ffff, addr.cmd_phase_addr);

		/* Poll memc_status register for interrupt */
		//brcm_nand_wait_until_ready(1000);

		break;

	case NAND_CMD_ERASE2:
		/* Construct cmd_phase_addr */
		addr.cmd_phase_addr |= 1 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE	|
								0x00 << NVSRAM_SMC_NAND_END_CMD		|
								NAND_CMD_ERASE2 << NVSRAM_SMC_NAND_START_CMD;

		writel(0x0, addr.cmd_phase_addr);
		break;

	case NAND_CMD_RNDOUT:
		/* Construct cmd_phase_addr */
		/* RNDOUT is sub-page read; so column addr (2 cycles) is sufficient */
		addr.cmd_phase_addr |= 2 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE				|
								NVSRAM_SMC_NAND_END_CMD_REQ						|
								NAND_CMD_RNDOUTSTART << NVSRAM_SMC_NAND_END_CMD	|
								NAND_CMD_RNDOUT << NVSRAM_SMC_NAND_START_CMD;

		/* Sending only column address cycle */
		if (column != -1) {
			/* Adjust columns for 16 bit buswidth */
			if (chip->options & NAND_BUSWIDTH_16)
				column >>= 1;

			writel(column && 0xffff, addr.cmd_phase_addr);
		}

		/* Construct data_phase_addr */
		addr.data_phase_addr |= NVSRAM_SMC_CLEAR_CS	| NVSRAM_SMC_RESERVED;

		break;

	case NAND_CMD_RNDIN:
		/* Construct cmd_phase_addr */
		/* RNDIN is sub-page write; so column addr (2 cycles) is sufficient */
		addr.cmd_phase_addr |= 2 << NVSRAM_SMC_CMD_NUM_ADDR_CYCLE	|
								0x00 << NVSRAM_SMC_NAND_END_CMD		|
								NAND_CMD_RNDIN << NVSRAM_SMC_NAND_START_CMD;

		/* Sending only column address cycle */
		if (column != -1) {
			/* Adjust columns for 16 bit buswidth */
			if (chip->options & NAND_BUSWIDTH_16)
				column >>= 1;

			writel(column && 0xffff, addr.cmd_phase_addr);
		}

		/* Construct data_phase_addr */
		addr.data_phase_addr |= NVSRAM_SMC_RESERVED;

		break;

	default:
		pr_err("NAND_ERROR: Un-supported command\n");
		return;
	}
}

/**
 * brcm_nand_select_chip - Dummy place holder 
 * @mtd:	MTD device structure
 * @chipnr:	chipnumber to select, -1 for deselect
 *
 * Default select function for 1 chip devices.
 */
static void brcm_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	switch (chipnr) {
	case -1:
		break;
	case 0:
		break;

	default:
		BUG();
	}
}

static uint32_t brcm_nand_get_hw_ecc(int page)
{
	uint32_t dwEcc = 0;

	switch (page)
	{
		case 0:
			dwEcc = readl(NVSRAM_REG_BASE + NVSRAM_ECC_VALUE0_OFFSET);
			break;

		case 1:
			dwEcc = readl(NVSRAM_REG_BASE + NVSRAM_ECC_VALUE1_OFFSET);
			break;

		case 2:
			dwEcc = readl(NVSRAM_REG_BASE + NVSRAM_ECC_VALUE2_OFFSET);
			break;

		case 3:
			dwEcc = readl(NVSRAM_REG_BASE + NVSRAM_ECC_VALUE3_OFFSET);
			break;

		default:
			pr_err("brcm_nand_get_hw_ecc: Invalid page number for ECC read\n");
			break;
	}

	/* Return the 3-byte ECC */
	return (dwEcc & 0x00ffffff);
}

#define TRUE 1
#define FALSE 0

static int brcm_nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int page)
{
	int i, j, steps, eccsize = chip->ecc.size;
	bool skip_ecc_check = TRUE;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint32_t ecc_calc[4];	// TODO: change to dynamic allocation for fwd compatibility for > 2048 byte pages
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	for (i = 0, steps = 0; steps < eccsteps; steps++, i++, p += eccsize) {
		/* Read main area */
		chip->read_buf(mtd, p, eccsize);

		/* Get h/w ecc */
		ecc_calc[i] = brcm_nand_get_hw_ecc(steps);
	}
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	/*
	 * For blank/erased good pages, the HW ECC is 0x0 and the spare area is 0xff
	 * therefore, ignore the ECC check for blank/erased good pages.
	 */
	for (j = 0; j < mtd->oobsize ; j++) {
		if (chip->oob_poi[j] != 0xff)
		{
			/* not a blank page, check the ecc */
			skip_ecc_check = FALSE;
			break;
		}
	}

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	eccsteps = chip->ecc.steps;
	p = buf;

    if (!skip_ecc_check) {
		for (i = 0, j = 0; eccsteps; eccsteps--, i += eccbytes, j++, p += eccsize) {
			int stat;

			stat = nand_correct_data512(mtd, p, &ecc_code[i], (uint8_t *)&ecc_calc[j]);
			if (stat < 0)
				mtd->ecc_stats.failed++;
			else
				mtd->ecc_stats.corrected += stat;
		}
	}
	return 0;
}

static void brcm_nand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf)
{
	int i, j, steps;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint32_t ecc_calc[4];	// TODO: dynamic allocation
	const uint8_t *p = buf;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t ecc_ptr[12];

	/* Write main area */
	chip->write_buf(mtd, p, mtd->writesize);

	for (i = 0, steps = 0; steps < eccsteps; steps++, i++) {
		/* Get h/w ecc */
		ecc_calc[i] = brcm_nand_get_hw_ecc(steps);
	}

	/* Copy the 12-byte ECC values */
	for (i = 0, j = 0; i < steps; i++, j += eccbytes)
		memcpy(&ecc_ptr[j], &ecc_calc[i], eccbytes);

	/* Rewrite new h/w ecc into oob on the fly */
	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_ptr[i];

	/* Write the oob last */
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
}

static int __devinit bcm_nand_probe(struct platform_device *pdev)
{
	struct nand_chip *chip;
	struct resource *r;
	uint32_t base_addr;
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
	bcm_nand_io_base = ioremap(r->start, r->end - r->start + 1);

	if (!bcm_nand_io_base) {
		pr_err("ioremap to access BCM NVSRAM NAND chip failed\n");
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

	/* Get chip_sel and NAND bus width for appropriate chip_sel */
	base_addr = r->start;
	switch(base_addr)
	{
		case CS0_BASE_ADDR:
			chip_sel = 0;
			chip->options |=
				(readl(NVSRAM_REG_BASE + NVSRAM_NAND_OPMODE_CS0_OFFSET) &
					NVSRAM_NAND_OPMODE_CS0_MW_MASK) ? NAND_BUSWIDTH_16 : 0;
			break;
		case CS1_BASE_ADDR:
			chip_sel = 1;
			chip->options |=
				(readl(NVSRAM_REG_BASE + NVSRAM_NAND_OPMODE_CS1_OFFSET) &
					NVSRAM_NAND_OPMODE_CS1_MW_MASK) ? NAND_BUSWIDTH_16 : 0;
			break;
		case CS2_BASE_ADDR:
			chip_sel = 2;
			chip->options |=
				(readl(NVSRAM_REG_BASE + NVSRAM_NAND_OPMODE_CS2_OFFSET) &
					NVSRAM_NAND_OPMODE_CS2_MW_MASK) ? NAND_BUSWIDTH_16 : 0;
			break;
		case CS3_BASE_ADDR:
			chip_sel = 3;
			chip->options |=
				(readl(NVSRAM_REG_BASE + NVSRAM_NAND_OPMODE_CS3_OFFSET) &
					NVSRAM_NAND_OPMODE_CS3_MW_MASK) ? NAND_BUSWIDTH_16 : 0;
			break;
		default:
			pr_err("NAND_ERROR: Wrong base address\n");
			iounmap(bcm_nand_io_base);
			kfree(board_mtd);
			return -EINVAL;
	}

	busw = chip->options & NAND_BUSWIDTH_16;

#if 0
	/*
	 * Do NVSRAM controller initializations. Will be moved to boot1/2.
	 *
	 * NOTE: Since it will be removed once boot1/2 is in place 
	 * i'm leaving it "very very" hard-coded!
	 */
	uint32_t val;

	/* Set NVSRAM MEMIF configurations */
    writel(0x00001E1F, NVSRAM_REG_BASE + NVSRAM_MEMIF_CFG_OFFSET);
    writel(0x10FF440D, NVSRAM_REG_BASE + NVSRAM_CONF_OFFSET);

	/* Set NVSRAM NAND configurations */
	if (busw & NAND_BUSWIDTH_16) {
		writel(1, NVSRAM_REG_BASE + NVSRAM_SET_OPMODE_OFFSET);
	} else {
		writel(0, NVSRAM_REG_BASE + NVSRAM_SET_OPMODE_OFFSET);
	}
    writel(0x00092Aff, NVSRAM_REG_BASE + NVSRAM_SET_CYCLES_OFFSET);
    val = ((chip_sel << NVSRAM_DIRECT_CMD_CHIP_SELECT_SHIFT) &
			NVSRAM_DIRECT_CMD_CHIP_SELECT_MASK) | NVSRAM_SMC_CS_BASE_ADDR;
    writel((0x2c00000 & ~(NVSRAM_DIRECT_CMD_CHIP_SELECT_MASK)) | val,
			NVSRAM_REG_BASE + NVSRAM_DIRECT_CMD_OFFSET);
#endif

	/* Set command delay time, see datasheet for correct value */
	chip->chip_delay = 0;

	/* Fill nand_chip structure */
	chip->read_byte = busw ? brcm_nand_read_byte16 : brcm_nand_read_byte;
	chip->read_word = brcm_nand_read_word;
	chip->write_buf = busw ? brcm_nand_write_buf16 : brcm_nand_write_buf;
	chip->read_buf = busw ? brcm_nand_read_buf16 : brcm_nand_read_buf;
	chip->verify_buf = busw ? brcm_nand_verify_buf16 : brcm_nand_verify_buf;
	chip->cmdfunc = brcm_nand_cmdfunc;
	chip->select_chip = brcm_nand_select_chip;

	/* Clear Raw interrupts */
	writel((unsigned long)(1 << NVSRAM_MEMC_CFG_CLR_NAND_INT_CLR_SHIFT),
			NVSRAM_REG_BASE + NVSRAM_MEMC_CFG_CLR_OFFSET);

#if defined (CONFIG_MTD_USE_HW_NAND_CONTROLLER_ECC)
	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.size = NAND_ECC_PAGE_SIZE;
	chip->ecc.bytes = NAND_ECC_NUM_BYTES;
	chip->ecc.read_page = brcm_nand_read_page_hwecc;
	chip->ecc.write_page = brcm_nand_write_page_hwecc;
#elif defined (CONFIG_MTD_USE_NAND_FLASH_ON_DIE_ECC)
	chip->ecc.mode = NAND_ECC_NONE;
#endif

	err = nand_scan_ident(board_mtd, 1, NULL);
	if (err) {
		pr_err("nand_scan failed: %d\n", err);
		iounmap(bcm_nand_io_base);
		kfree(board_mtd);
		return err;
	}

	/* Now that we know the nand size, we can and setup the ECC layout */

	switch (board_mtd->writesize)
	{
		case 4096:
			chip->ecc.layout = &nand_hw_eccoob_4096;
			break;
		case 2048:
#if defined (CONFIG_MTD_USE_HW_NAND_CONTROLLER_ECC)
			chip->ecc.layout = &nand_hw_eccoob_2048;
#elif defined (CONFIG_MTD_USE_NAND_FLASH_ON_DIE_ECC)
			chip->ecc.layout = &micron_nand_on_die_eccoob_2048;
#endif
			break;
		case 512:
			chip->ecc.layout = &nand_hw_eccoob_512;
			break;
		default:
			pr_err("NAND - Unrecognized pagesize: %d\n",
			       board_mtd->writesize);
			return -EINVAL;
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
		iounmap(bcm_nand_io_base);
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
			iounmap(bcm_nand_io_base);
			kfree(board_mtd);
			return -EIO;
		}

		add_mtd_partitions(board_mtd, partition_info, nr_partitions);
	}

	/* Return happy */
	return 0;
}

static int bcm_nand_remove(struct platform_device *pdev)
{
	/* Release resources, unregister device */
	nand_release(board_mtd);

	/* unmap physical address */
	iounmap(bcm_nand_io_base);

	/* Free the MTD device structure */
	kfree(board_mtd);

	return 0;
}

#ifdef CONFIG_PM
static int bcm_nand_suspend(struct platform_device *pdev,
				pm_message_t state)
{
	return 0;
}

static int bcm_nand_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define bcm_nand_suspend   NULL
#define bcm_nand_resume    NULL
#endif

static struct platform_driver nand_driver = {
	.driver = {
		   .name = "bcm-nand",
		   .owner = THIS_MODULE,
		   },
	.probe = bcm_nand_probe,
	.remove = bcm_nand_remove,
	.suspend = bcm_nand_suspend,
	.resume = bcm_nand_resume,
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
