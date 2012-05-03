/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*      @file  drivers/mtd/onenand/bcm_xboard.c
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

/*
 *  Overview:
 *   This is a device driver for the OneNAND flash for Broadcom X-boards.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <asm/mach/flash.h>

#define DRIVER_NAME	"onenand"


#ifdef CONFIG_MTD_PARTITIONS
static const char *part_probes[] = { "cmdlinepart", NULL,  };
#endif

struct onenand_info {
	struct mtd_info		mtd;
	struct mtd_partition	*parts;
	struct onenand_chip	onenand;
};

/**
 * onenand_bufferram_offset - [DEFAULT] BufferRAM offset
 * @param mtd		MTD data structure
 * @param area		BufferRAM area
 * @return		offset given area
 *
 * Return BufferRAM offset given area
 */
static inline int onenand_bufferram_offset(struct mtd_info *mtd, int area)
{
	struct onenand_chip *this = mtd->priv;

	if (ONENAND_CURRENT_BUFFERRAM(this)) {
		/* Note: the 'this->writesize' is a real page size */
		if (area == ONENAND_DATARAM)
			return this->writesize;
		if (area == ONENAND_SPARERAM)
			return mtd->oobsize;
	}

	return 0;
}

/**
 * bcm_onenand_read_bufferram - Read the bufferram area
 * @param mtd		MTD data structure
 * @param area		BufferRAM area
 * @param buffer	the databuffer to put/get data
 * @param offset	offset to read from or write to
 * @param count		number of bytes to read/write
 *
 * Read the BufferRAM area
 */
static int bcm_onenand_read_bufferram(struct mtd_info *mtd, int area,
		unsigned char *buffer, int offset, size_t count)
{
	struct onenand_chip *this = mtd->priv;
	void __iomem *bufferram;

	bufferram = this->base + area;

	bufferram += onenand_bufferram_offset(mtd, area);

	if (ONENAND_CHECK_BYTE_ACCESS(count)) {
		unsigned short word;

		/* Align with word(16-bit) size */
		count--;

		/* Read word and save byte */
		word = this->read_word(bufferram + offset + count);
		buffer[count] = (word & 0xff);
	}

	memcpy(buffer, bufferram + offset, count);

	return 0;
}

/**
 * bcm_onenand_sync_read_bufferram - Read the bufferram area with Sync. Burst mode
 * @param mtd		MTD data structure
 * @param area		BufferRAM area
 * @param buffer	the databuffer to put/get data
 * @param offset	offset to read from or write to
 * @param count		number of bytes to read/write
 *
 * Read the BufferRAM area with Sync. Burst Mode
 */
static int bcm_onenand_sync_read_bufferram(struct mtd_info *mtd, int area,
		unsigned char *buffer, int offset, size_t count)
{
	struct onenand_chip *this = mtd->priv;
	void __iomem *bufferram;

	bufferram = this->base + area;

	bufferram += onenand_bufferram_offset(mtd, area);

	this->mmcontrol(mtd, ONENAND_SYS_CFG1_SYNC_READ);

	if (ONENAND_CHECK_BYTE_ACCESS(count)) {
		unsigned short word;

		/* Align with word(16-bit) size */
		count--;

		/* Read word and save byte */
		word = this->read_word(bufferram + offset + count);
		buffer[count] = (word & 0xff);
	}

	memcpy(buffer, bufferram + offset, count);

	this->mmcontrol(mtd, 0);

	return 0;
}

/**
 * bcm_onenand_write_bufferram - Write the bufferram area
 * @param mtd		MTD data structure
 * @param area		BufferRAM area
 * @param buffer	the databuffer to put/get data
 * @param offset	offset to read from or write to
 * @param count		number of bytes to read/write
 *
 * Write the BufferRAM area
 */
static int bcm_onenand_write_bufferram(struct mtd_info *mtd, int area,
		const unsigned char *buffer, int offset, size_t count)
{
	struct onenand_chip *this = mtd->priv;
	void __iomem *bufferram;

	bufferram = this->base + area;

	bufferram += onenand_bufferram_offset(mtd, area);

	if (ONENAND_CHECK_BYTE_ACCESS(count)) {
		unsigned short word;
		int byte_offset;

		/* Align with word(16-bit) size */
		count--;

		/* Calculate byte access offset */
		byte_offset = offset + count;

		/* Read word and save byte */
		word = this->read_word(bufferram + byte_offset);
		word = (word & ~0xff) | buffer[count];
		this->write_word(word, bufferram + byte_offset);
	}

	memcpy(bufferram + offset, buffer, count);

	return 0;
}

static int bcm_onenand_probe(struct platform_device *pdev)
{
	struct onenand_info *info;
	struct flash_platform_data *pdata = pdev->dev.platform_data;
	struct resource *res = pdev->resource;
	int err;

	info = kzalloc(sizeof(struct onenand_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->onenand.base = (void __iomem *)res->start;
	if (!info->onenand.base) {
		err = -ENOMEM;
		goto out_free;
	}

	info->onenand.mmcontrol = 0; /*pdata->mmcontrol*/
	info->onenand.irq = platform_get_irq(pdev, 0);
	if (!info->onenand.mmcontrol)
		info->onenand.read_bufferram = bcm_onenand_read_bufferram;
	else
		info->onenand.read_bufferram = bcm_onenand_sync_read_bufferram;
	info->onenand.write_bufferram = bcm_onenand_write_bufferram;

	info->mtd.name = "bcm_umi-nand";
	info->mtd.priv = &info->onenand;
	info->mtd.owner = THIS_MODULE;

	if (onenand_scan(&info->mtd, 1)) {
		err = -ENXIO;
		pr_err("onenand_scan failed: %d\n", err);
		goto out_free;
	}

#ifdef CONFIG_MTD_PARTITIONS
	err = parse_mtd_partitions(&info->mtd, part_probes, &info->parts, 0);
	if (err > 0)
		add_mtd_partitions(&info->mtd, info->parts, err);
	else if (err <= 0 && pdata->parts)
		add_mtd_partitions(&info->mtd, pdata->parts, pdata->nr_parts);
	else
#endif
		err = add_mtd_device(&info->mtd);

	dev_set_drvdata(&pdev->dev, info);

	return 0;

out_free:
	kfree(info);
	return err;
}

static int __devexit bcm_onenand_remove(struct platform_device *pdev)
{
	struct onenand_info *info = dev_get_drvdata(&pdev->dev);

	dev_set_drvdata(&pdev->dev, NULL);

	if (info) {
		if (info->parts)
			del_mtd_partitions(&info->mtd);
		else
			del_mtd_device(&info->mtd);

		onenand_release(&info->mtd);
		kfree(info);
	}

	return 0;
}
static struct platform_driver bcm_onenand_driver = {
	.driver = {
		.name		= DRIVER_NAME,
		.owner		= THIS_MODULE,
	},
	.probe		= bcm_onenand_probe,
	.remove		= __devexit_p(bcm_onenand_remove),
};

MODULE_ALIAS(DRIVER_NAME);

static int __init bcm_onenand_init(void)
{
//++ ASW3.SOS.jinuk : block the MTD initialization
//	return platform_driver_register(&bcm_onenand_driver);
//-- ASW3.SOS.jinuk : block the MTD initialization
}

static void __exit bcm_onenand_exit(void)
{
	platform_driver_unregister(&bcm_onenand_driver);
}

module_init(bcm_onenand_init);
module_exit(bcm_onenand_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Glue layer for OneNAND flash on BCM X-boards");
