/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/misc/ckblock_reader.c
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/mtd.h>
#include <linux/broadcom/ckblock_reader.h>

/* Macros for debugging and unit testing */
/* #define _CKB_DEBUG	(1) */

struct ckblock_reader_data {
	struct mtd_info		*mtd;
	/* More information can be added in future */
};

static struct ckblock_reader_data drv_ctx;
static DEFINE_MUTEX(drv_mutex);

/**
 * @brief 	readCKDataBlock : Reads the CK Data block from the MTD device.
 * @param	buff 	: Kernel Space buffer to which the data will be read. 
 * @param	len     : number of bytes to read.
 * @return	Returns -1 on error. On success returns the number of bytes 
 *		actually read 
 *
 * @description	This function reads the CK data block from the MTD device.
 * It returns -1 on error. On success returns the number of bytes actually
 * read from the MTD device.
 */
int readCKDataBlock (char *buff, int len)
{
	int n;
	int found;
	int ret = -1;
	size_t ret_len;
	loff_t offset;

	if ((NULL == buff) || (0 == len)) {
		printk("readCKDataBlock: buffer or length invalid \r\n");
		return -1;
	}

	mutex_lock(&drv_mutex);

	if (NULL == drv_ctx.mtd) {
		printk("readCKDataBlock: mtd is NULL, possible that partition not found\r\n");
		return -1;
	}

	if (len > drv_ctx.mtd->size) {
		printk("readCKDataBlock: requested %d greater than %ld \r\n", len, drv_ctx.mtd->size);
		goto err_out;
	}

	/*
	 * The one shot downloader does BBT mgmt for this.
	 * Here we need to check the next available good block and read the 
	 * same.
	 */

	/* Iterate till you find a good block to read */
	offset = 0;
	found = 0;
	while (offset < drv_ctx.mtd->size) {
		if(drv_ctx.mtd->block_isbad(drv_ctx.mtd,offset) != 0)
			offset += drv_ctx.mtd->erasesize;
		else {
			found = 1;
			break;
		}
	}

	if (!found) {
		printk("readCKDataBlock: All the blocks in the partition are bad \r\n");
		goto err_out;
	}

	/* 
	 * Now, if the bytes requested to read is more than the available blocks
	 * reduce the size to available bytes
	 */
	if (len > (drv_ctx.mtd->size - offset)) 
		len = drv_ctx.mtd->size - offset;

	ret_len = 0;
	n = drv_ctx.mtd->read(drv_ctx.mtd,
                offset, (size_t)len, &ret_len, buff);

        if ((n == -EUCLEAN) || (n == -EBADMSG)) {
                printk("readCKDataBlock: mtd->read returned %d \r\n", n);
                goto err_out;
        }
        ret = ret_len;		

#ifdef _CKB_DEBUG
	printk("readCKDataBlock: returned %d and bytes read is %d \r\n", n, ret_len);
#endif

err_out:
	mutex_unlock(&drv_mutex);
	return ret;
}
EXPORT_SYMBOL(readCKDataBlock);

/* 
 * Call back function that is invoked by the mtd layer
 * when an mtd partition is added
 */
static void mtd_notify_add(struct mtd_info *mtd)
{
	/* Return if its not CK Block partition */
	if (strcmp(mtd->name, CONFIG_BRCM_CKBLOCK_NAME))
		return;

	/* otherwise store the mtd context in the device structure */
	drv_ctx.mtd = mtd;

	printk(KERN_INFO "ckblock_reader: Bound to mtd partition '%s'\n", mtd->name);

	return;
}


/*
 * Call back function that is invoked by the mtd layer
 * when an mtd partition is removed
 */
static void mtd_notify_remove(struct mtd_info *mtd)
{
	if (mtd == drv_ctx.mtd) {
		drv_ctx.mtd = NULL;
		printk(KERN_INFO "clkblock_reader: Unbound from %s\n", mtd->name);
	}
}

static struct mtd_notifier mtd_ckblock_notifier = {
	.add	= mtd_notify_add,
	.remove	= mtd_notify_remove,
};

/* Init function called during kernel startup */
int __init ckblock_reader_init(void)
{
	register_mtd_user(&mtd_ckblock_notifier);
	memset(&drv_ctx, 0, sizeof(drv_ctx));
	printk(KERN_INFO "####### CK Data block reader initialized (bind=%s)\n",
	       CONFIG_BRCM_CKBLOCK_NAME);
	return 0;
}
module_init(ckblock_reader_init);
