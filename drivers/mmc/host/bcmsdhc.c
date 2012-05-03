/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/mmc/host/bcmsdhc.c
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
#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/card.h>
#include <linux/clk.h>
#include <linux/slab.h>
/* #include <asm/mach/mmc.h> */
#include <linux/init.h>
#include <linux/pm_qos_params.h>
#include <linux/regulator/consumer.h>

#include <linux/mfd/max8986/max8986.h>

#ifndef CONFIG_ARCH_BCM215XX
#include <mach/reg_sdio.h>
#else
#include <mach/sdio.h>
#endif

#include "bcmsdhc.h"

#define DRIVER_NAME "bcm_sdhc"

#define DBG(f, x...) \
	pr_debug(DRIVER_NAME " [%s()]: " f, __func__, ## x)
static void bcmsdhc_prepare_data(struct bcmsdhc_host *, struct mmc_data *);
static void bcmsdhc_finish_data(struct bcmsdhc_host *);

static void bcmsdhc_send_command(struct bcmsdhc_host *, struct mmc_command *);
static void bcmsdhc_finish_command(struct bcmsdhc_host *);
static void bcmsdhc_clock_on(struct bcmsdhc_host *host);
static void bcmsdhc_clock_off(struct bcmsdhc_host *host);
extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int bcm_gpio_set_db_val(unsigned int gpio, unsigned int db_val);

void bcmsdhc_sdio_host_force_scan(struct platform_device *pdev, bool on);

int sdhc_update_qos_req(struct bcmsdhc_host *host, s32 value);
int sdio_add_qos_req(struct bcmsdhc_host *host);
int sdio_remove_qos_req(struct bcmsdhc_host *host);


bool sd_inserted = 0;

struct regulator_dev {
	struct regulator_desc *desc;
	int use_count;
	int open_count;
	int exclusive;

	/* lists we belong to */
	struct list_head list; /* list of all regulators */
	struct list_head slist; /* list of supplied regulators */

	/* lists we own */
	struct list_head consumer_list; /* consumers we supply */
	struct list_head supply_list; /* regulators we supply */

	struct blocking_notifier_head notifier;
	struct mutex mutex; /* consumer lock */
	struct module *owner;
	struct device dev;
	struct regulation_constraints *constraints;
	struct regulator_dev *supply;	/* for tree */

	void *reg_data;		/* regulator_dev data */
};

struct regulator {
	struct device *dev;
	struct list_head list;
	int uA_load;
	int min_uV;
	int max_uV;
	char *supply_name;
	struct device_attribute dev_attr;
	struct regulator_dev *rdev;
};


/* *************************************************************************************************** */
/* Function Name: bcmsdhc_dumpregs */
/* Description: This function is for logging register of SDHC */
/* *************************************************************************************************** */

static void bcmsdhc_dumpregs(struct bcmsdhc_host *host)
{
	pr_debug(DRIVER_NAME ": ============== REGISTER DUMP ==============\n");

	pr_debug(DRIVER_NAME ": Sys addr: 0x%08x | Version:  0x%08x\n",
		 readl(host->ioaddr + SDHC_SYSTEMADDRESS_LO),
		 readw(host->ioaddr + SDHC_HOST_CONTROLLER_VER));
	pr_debug(DRIVER_NAME ": Blk size: 0x%08x | Blk cnt:  0x%08x\n",
		 readw(host->ioaddr + SDHC_BLOCKSIZE),
		 readw(host->ioaddr + SDHC_BLOCKCOUNT));
	pr_debug(DRIVER_NAME ": Argument: 0x%08x | Trn mode: 0x%08x\n",
		 readl(host->ioaddr + SDHC_ARGUMENT_0),
		 readw(host->ioaddr + SDHC_TRANSFERMODE));
	pr_debug(DRIVER_NAME ": Present:  0x%08x | Host ctl: 0x%08x\n",
		 readl(host->ioaddr + SDHC_PRESENT_STATE),
		 readb(host->ioaddr + SDHC_HOST_CONTROL));
	pr_debug(DRIVER_NAME ": Power:    0x%08x | Blk gap:  0x%08x\n",
		 readb(host->ioaddr + SDHC_POWER_CONTROL),
		 readb(host->ioaddr + SDHC_BLOCK_GAP_CONTROL));
	pr_debug(DRIVER_NAME ": Wake-up:  0x%08x | Clock:    0x%08x\n",
		 readb(host->ioaddr + SDHC_WAKEUP_CONTROL),
		 readw(host->ioaddr + SDHC_CLOCK_CONTROL));
	pr_debug(DRIVER_NAME ": Timeout:  0x%08x | Int stat: 0x%08x\n",
		 readb(host->ioaddr + SDHC_TIMEOUT_CONTROL),
		 readl(host->ioaddr + SDHC_NORMAL_INT_STATUS));
	pr_debug(DRIVER_NAME ": Int enab: 0x%08x | Sig enab: 0x%08x\n",
		 readl(host->ioaddr + SDHC_NORMAL_INT_STATUS_ENABLE),
		 readl(host->ioaddr + SDHC_NORMAL_INT_SIGNAL_ENABLE));
	pr_debug(DRIVER_NAME ": AC12 err: 0x%08x | Slot int: 0x%08x\n",
		 readw(host->ioaddr + SDHC_AUTOCMD12_ERROR_STATUS),
		 readw(host->ioaddr + SDHC_SLOT_INT_STATUS));
	pr_debug(DRIVER_NAME ": Caps:     0x%08x | Max curr: 0x%08x\n",
		 readl(host->ioaddr + SDHC_CAPABILITIES),
		 readl(host->ioaddr + SDHC_MAX_CURRENT_CAPABILITIES));

#if (defined(CONFIG_ARCH_BCM282X) || defined(CONFIG_ARCH_BCM116X) || defined(CONFIG_ARCH_BCM215XX))
	pr_debug(DRIVER_NAME ": COM:     0x%08x | resp0: 0x%08x\n",
		 readw(host->ioaddr + SDHC_COMMAND),
		 readl(host->ioaddr + SDHC_R0));
#endif
	pr_debug(DRIVER_NAME ": ADMA Err: 0x%08x | ADMA Ptr: 0x%08x\n",
	       readl(host->ioaddr + SDHC_ADMA_ERR_STAT),
	       readl(host->ioaddr + SDHC_ADMA_ADDRESS));

	pr_debug(DRIVER_NAME ": ===========================================\n");
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_reset */
/* Description: SD host reset */
/* *************************************************************************************************** */

static void bcmsdhc_reset(struct bcmsdhc_host *host, u8 mask)
{
	unsigned long timeout;

	if (host->card_present == false) {
		return;
	}

	writeb(mask, host->ioaddr + SDHC_SOFT_RESET);

	if (mask & SOFT_RESET_ALL)
		host->clock = 0;

	/* Wait max 100 ms */
	timeout = 100;

	/* hw clears the bit when it's done */
	while (readb(host->ioaddr + SDHC_SOFT_RESET) & mask) {
		if (timeout == 0) {
			pr_err("%s: Reset 0x%x never completed.\n",
			       mmc_hostname(host->mmc), (int)mask);
/*                      bcmsdhc_dumpregs(host); */
			return;
		}
		timeout--;
		mdelay(1);
	}
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_init */
/* Description: initiate the bcm sd host. */
/* *************************************************************************************************** */

static void bcmsdhc_init(struct bcmsdhc_host *host, u32 mask)
{
	u32 intmask;

	if (host->bcm_plat->cfg_card_detect(host->ioaddr, host->sdhc_slot))
		return;

	bcmsdhc_reset(host, mask);

	/* Enable error interrupt status and signals for all but the vendor
	   errors. This allows any normal error to generate an interrupt. */
	writew((0xFFFF & ~ERR_INT_STATUS_VENDOR),
	       host->ioaddr + SDHC_ERROR_INT_STATUS_ENABLE);
	writew((0xFFFF & ~ERR_INT_STATUS_VENDOR),
	       host->ioaddr + SDHC_ERROR_INT_SIGNAL_ENABLE);

	writew(0, host->ioaddr + SDHC_NORMAL_INT_SIGNAL_ENABLE);
	
	intmask =
	    (NORMAL_INT_ENABLE_CMD_COMPLETE | NORMAL_INT_ENABLE_TRX_COMPLETE |
	     NORMAL_INT_ENABLE_BLOCK_GAP | NORMAL_INT_ENABLE_DMA |
	     NORMAL_INT_ENABLE_BUF_WRITE_RDY | NORMAL_INT_ENABLE_BUF_READ_RDY |
	     NORMAL_INT_ENABLE_CARD_INT);

	/* Enable all interrupt signals. During execution, we will enable
	   and disable interrupt statuses as desired. */

	writew(intmask, host->ioaddr + SDHC_NORMAL_INT_SIGNAL_ENABLE);

	/* Pass through interrupts from IP core to interrupt controller */
	host->bcm_plat->enable_int(host->ioaddr, host->sdhc_slot);

	pr_debug
	    ("%s :Normal int signal enable =0x%x ,normal int status enable=0x%x\n",
	     __func__, readw(host->ioaddr + SDHC_NORMAL_INT_SIGNAL_ENABLE),
	     readw(host->ioaddr + SDHC_NORMAL_INT_STATUS_ENABLE));
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_activate_led */
/* Description: active  Led */
/* *************************************************************************************************** */

static void bcmsdhc_activate_led(struct bcmsdhc_host *host)
{
	u8 ctrl;

	ctrl = readb(host->ioaddr + SDHC_HOST_CONTROL);
	ctrl |= HOSTCTL_LED_CONTROL;
	writeb(ctrl, host->ioaddr + SDHC_HOST_CONTROL);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_deactivate_led */
/* Description: deactive Led */
/* *************************************************************************************************** */

static void bcmsdhc_deactivate_led(struct bcmsdhc_host *host)
{
	u8 ctrl;

	ctrl = readb(host->ioaddr + SDHC_HOST_CONTROL);
	ctrl &= ~HOSTCTL_LED_CONTROL;
	writeb(ctrl, host->ioaddr + SDHC_HOST_CONTROL);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_led_control */
/* Description: control SDHC Led */
/* *************************************************************************************************** */

#ifdef CONFIG_LEDS_CLASS
static void bcmsdhc_led_control(struct led_classdev *led,
				enum led_brightness brightness)
{
	struct bcmsdhc_host *host = container_of(led, struct bcmsdhc_host, led);
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);

	if (brightness == LED_OFF)
		bcmsdhc_deactivate_led(host);
	else
		bcmsdhc_activate_led(host);

	spin_unlock_irqrestore(&host->lock, flags);
}
#endif

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_read_block_pio */
/* Description: */
/* ************************************************************************************************** */

static void bcmsdhc_read_block_pio(struct bcmsdhc_host *host)
{
	unsigned long flags;
	size_t blksize, len, chunk;
	u32 scratch, i = 0;
	u8 *buf;

	DBG("PIO reading\n");

	blksize = host->data->blksz;
	chunk = 0;

	local_irq_save(flags);

	while (blksize) {
		if (!sg_miter_next(&host->sg_miter))
			BUG();

		len = min(host->sg_miter.length, blksize);

		i++;

		blksize -= len;
		host->sg_miter.consumed = len;

		buf = host->sg_miter.addr;

		while (len) {
			if (chunk == 0) {
				scratch =
				    readl(host->ioaddr +
					  SDHC_BUFFER_DATA_PORT_0);
				chunk = 4;
			}

			*buf = scratch & 0xFF;

			buf++;
			scratch >>= 8;
			chunk--;
			len--;
		}
	}

	sg_miter_stop(&host->sg_miter);

	local_irq_restore(flags);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_write_block_pio */
/* Description: */
/* ************************************************************************************************** */

static void bcmsdhc_write_block_pio(struct bcmsdhc_host *host)
{
	unsigned long flags;
	size_t blksize, len, chunk;
	u32 scratch;
	u8 *buf;

	DBG("PIO writing\n");

	blksize = host->data->blksz;
	chunk = 0;
	scratch = 0;

	local_irq_save(flags);

	while (blksize) {
		if (!sg_miter_next(&host->sg_miter))
			BUG();

		len = min(host->sg_miter.length, blksize);

		blksize -= len;
		host->sg_miter.consumed = len;

		buf = host->sg_miter.addr;

		while (len) {
			scratch |= (u32) * buf << (chunk * 8);

			buf++;
			chunk++;
			len--;

			if ((chunk == 4) || ((len == 0) && (blksize == 0))) {
				writel(scratch,
				       host->ioaddr + SDHC_BUFFER_DATA_PORT_0);
				chunk = 0;
				scratch = 0;
			}
		}
	}

	sg_miter_stop(&host->sg_miter);

	local_irq_restore(flags);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_transfer_pio */
/* Description: */
/* ************************************************************************************************** */

static void bcmsdhc_transfer_pio(struct bcmsdhc_host *host)
{
	u32 mask;

	BUG_ON(!host->data);

	if (host->blocks == 0) {
		return;
	}
	if (host->data->flags & MMC_DATA_READ) {
		mask = STATE_BUF_READ_ENABLE;
	} else {
		mask = STATE_BUF_WRITE_ENABLE;
	}

	while (readl(host->ioaddr + SDHC_PRESENT_STATE) & mask) {
		if (host->data->flags & MMC_DATA_READ) {
			bcmsdhc_read_block_pio(host);
		} else {
			bcmsdhc_write_block_pio(host);
		}

		host->blocks--;
		if (host->blocks == 0) {
			break;
		}
	}

	DBG("PIO transfer complete.\n");
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_kmap_atomic */
/* Description: */
/* ************************************************************************************************** */

static char *bcmsdhc_kmap_atomic(struct scatterlist *sg, unsigned long *flags)
{
	local_irq_save(*flags);
	return kmap_atomic(sg_page(sg), KM_BIO_SRC_IRQ) + sg->offset;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_kunmap_atomic */
/* Description: */
/* ************************************************************************************************** */

static void bcmsdhc_kunmap_atomic(void *buffer, unsigned long *flags)
{
	kunmap_atomic(buffer, KM_BIO_SRC_IRQ);
	local_irq_restore(*flags);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_adma_table_pre */
/* Description: */
/* ************************************************************************************************** */

static int bcmsdhc_adma_table_pre(struct bcmsdhc_host *host,
				  struct mmc_data *data)
{
	int direction;

	u8 *desc;
	u8 *align;
	dma_addr_t addr;
	dma_addr_t align_addr;
	int len, offset;

	struct scatterlist *sg;
	int i;
	char *buffer;
	unsigned long flags;

	/*
	 * The spec does not specify endianness of descriptor table.
	 * We currently guess that it is LE.
	 */

	if (data->flags & MMC_DATA_READ)
		direction = DMA_FROM_DEVICE;
	else
		direction = DMA_TO_DEVICE;

	/*
	 * The ADMA descriptor table is mapped further down as we
	 * need to fill it with data first.
	 */

	host->align_addr = dma_map_single(mmc_dev(host->mmc),
					  host->align_buffer, 128 * 4,
					  direction);
	if (dma_mapping_error(mmc_dev(host->mmc), host->align_addr))
		goto fail;
	BUG_ON(host->align_addr & 0x3);

	host->sg_count = dma_map_sg(mmc_dev(host->mmc),
				    data->sg, data->sg_len, direction);
	if (host->sg_count == 0)
		goto unmap_align;

	desc = host->adma_desc;
	align = host->align_buffer;

	align_addr = host->align_addr;

	for_each_sg(data->sg, sg, host->sg_count, i) {
		addr = sg_dma_address(sg);
		len = sg_dma_len(sg);

		/*
		 * The SDHCI specification states that ADMA
		 * addresses must be 32-bit aligned. If they
		 * aren't, then we use a bounce buffer for
		 * the (up to three) bytes that screw up the
		 * alignment.
		 */
		offset = (4 - (addr & 0x3)) & 0x3;
		if (offset) {
			if (data->flags & MMC_DATA_WRITE) {
				buffer = bcmsdhc_kmap_atomic(sg, &flags);
				WARN_ON(((long)buffer & PAGE_MASK) >
					(PAGE_SIZE - 3));
				memcpy(align, buffer, offset);
				pr_info("memcpy of size %d\n", offset);
				bcmsdhc_kunmap_atomic(buffer, &flags);
			}

			desc[7] = (align_addr >> 24) & 0xff;
			desc[6] = (align_addr >> 16) & 0xff;
			desc[5] = (align_addr >> 8) & 0xff;
			desc[4] = (align_addr >> 0) & 0xff;

			BUG_ON(offset > 65536);

			desc[3] = (offset >> 8) & 0xff;
			desc[2] = (offset >> 0) & 0xff;

			desc[1] = 0x00;
			desc[0] = 0x21;	/* tran, valid */

			align += 4;
			align_addr += 4;

			desc += 8;

			addr += offset;
			len -= offset;
		}

		desc[7] = (addr >> 24) & 0xff;
		desc[6] = (addr >> 16) & 0xff;
		desc[5] = (addr >> 8) & 0xff;
		desc[4] = (addr >> 0) & 0xff;

		BUG_ON(len > 65536);

		desc[3] = (len >> 8) & 0xff;
		desc[2] = (len >> 0) & 0xff;

		desc[1] = 0x00;
#ifdef CONFIG_MMC_ARASAN_HOST_FIX
		/*
		 * When doing hardware scatter/gather, can not use nop in any entry.
		 * so add an "end" here if this is the last entry and do not add
		 * an extra terminating entry.
		 */
		if (host->sg_count == i + 1) {
			desc[0] = 0x23;	/* tran, end, valid */
		} else {
			desc[0] = 0x21;	/* tran, valid */
		}
#else
		desc[0] = 0x21;	/* tran, valid */
#endif
		desc += 8;

		/*
		 * If this triggers then we have a calculation bug
		 * somewhere. :/
		 */
		WARN_ON((desc - host->adma_desc) > (128 * 2 + 1) * 4);
	}

#ifndef CONFIG_MMC_ARASAN_HOST_FIX
	/*
	 * Add a terminating entry.
	 */
	desc[7] = 0;
	desc[6] = 0;
	desc[5] = 0;
	desc[4] = 0;

	desc[3] = 0;
	desc[2] = 0;

	desc[1] = 0x00;
	desc[0] = 0x03;		/* nop, end, valid */
#endif

	/*
	 * Resync align buffer as we might have changed it.
	 */
	if (data->flags & MMC_DATA_WRITE) {
		dma_sync_single_for_device(mmc_dev(host->mmc),
					   host->align_addr, 128 * 4,
					   direction);
	}

	host->adma_addr = dma_map_single(mmc_dev(host->mmc),
					 host->adma_desc, (128 * 2 + 1) * 4,
					 DMA_TO_DEVICE);
	if (dma_mapping_error(mmc_dev(host->mmc), host->adma_addr))
		goto unmap_entries;
	BUG_ON(host->adma_addr & 0x3);

	return 0;

unmap_entries:
	dma_unmap_sg(mmc_dev(host->mmc), data->sg, data->sg_len, direction);
unmap_align:
	dma_unmap_single(mmc_dev(host->mmc), host->align_addr,
			 128 * 4, direction);
fail:
	return -EINVAL;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_adma_table_post */
/* Description: */
/* ************************************************************************************************** */

static void bcmsdhc_adma_table_post(struct bcmsdhc_host *host,
				    struct mmc_data *data)
{
	int direction;

	struct scatterlist *sg;
	int i, size;
	u8 *align;
	char *buffer;
	unsigned long flags;

	if (data->flags & MMC_DATA_READ)
		direction = DMA_FROM_DEVICE;
	else
		direction = DMA_TO_DEVICE;

	dma_unmap_single(mmc_dev(host->mmc), host->adma_addr, (128 * 2 + 1) * 4,
			 DMA_TO_DEVICE);

	dma_unmap_single(mmc_dev(host->mmc), host->align_addr, 128 * 4,
			 direction);

	if (data->flags & MMC_DATA_READ) {
		dma_sync_sg_for_cpu(mmc_dev(host->mmc), data->sg, data->sg_len,
				    direction);

		align = host->align_buffer;

		for_each_sg(data->sg, sg, host->sg_count, i) {
			if (sg_dma_address(sg) & 0x3) {
				size = 4 - (sg_dma_address(sg) & 0x3);

				buffer = bcmsdhc_kmap_atomic(sg, &flags);
				WARN_ON(((long)buffer & PAGE_MASK) >
					(PAGE_SIZE - 3));
				memcpy(buffer, align, size);
				bcmsdhc_kunmap_atomic(buffer, &flags);

				align += 4;
			}
		}
	}

	dma_unmap_sg(mmc_dev(host->mmc), data->sg, data->sg_len, direction);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_calc_timeout */
/* Description: calc. timeout value */
/* ************************************************************************************************** */

static u8 bcmsdhc_calc_timeout(struct bcmsdhc_host *host, struct mmc_data *data)
{
	u8 count;
	unsigned target_timeout, current_timeout;

	/*
	 * If the host controller provides us with an incorrect timeout
	 * value, just skip the check and use 0xE.  The hardware may take
	 * longer to time out, but that's much better than having a too-short
	 * timeout value.
	 */
	/* if ((host->quirks & SDHCI_QUIRK_BROKEN_TIMEOUT_VAL)) */
	/*              return 0xE; */

	/* timeout in us */
	target_timeout =
	    data->timeout_ns / 1000 + data->timeout_clks / host->clock;

	/*
	 * Figure out needed cycles.
	 * We do this in steps in order to fit inside a 32 bit int.
	 * The first step is the minimum timeout, which will have a
	 * minimum resolution of 6 bits:
	 * (1) 2^13*1000 > 2^22,
	 * (2) host->timeout_clk < 2^16
	 *     =>
	 *     (1) / (2) > 2^6
	 */
	count = 0;
	current_timeout = (1 << 13) * 1000 / host->timeout_clk;
	while (current_timeout < target_timeout) {
		count++;
		current_timeout <<= 1;
		if (count >= 0xF)
			break;
	}

	if (count >= 0xF) {
#if (!defined(CONFIG_ARCH_BCM282X) && !defined(CONFIG_ARCH_BCM116X) && \
		!defined(CONFIG_ARCH_BCMRING) && !defined(CONFIG_ARCH_BCM215XX))
		pr_warning("%s: Too large timeout requested!\n",
			   mmc_hostname(host->mmc));
#endif
		count = 0xE;
	}
#if (defined(CONFIG_ARCH_BCM282X) || defined(CONFIG_ARCH_BCM116X) || \
		defined(CONFIG_ARCH_BCMRING) || defined(CONFIG_ARCH_BCM215XX))
	/*Some cards require unusually large timeouts */
	count = 0xE;
#endif

	return count;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_prepare_data */
/* Description: prepare data for transfering */
/* ************************************************************************************************** */

static void bcmsdhc_prepare_data(struct bcmsdhc_host *host,
				 struct mmc_data *data)
{
	u8 count;
	u8 ctrl;
	int ret;

	WARN_ON(host->data);

	if (data == NULL)
		return;

	/* Sanity checks */
	BUG_ON(data->blksz * data->blocks > 524288);
	BUG_ON(data->blksz > host->mmc->max_blk_size);
	BUG_ON(data->blocks > 65535);
	DBG(KERN_INFO "bcmsdhc_prepare_data:max_blk_size=%d blksz=%d\n",
	    host->mmc->max_blk_size, data->blksz);

	host->data = data;

	host->command_complete = 0;

	count = bcmsdhc_calc_timeout(host, data);
	writeb(count, host->ioaddr + SDHC_TIMEOUT_CONTROL);

	if (host->flags & SDHCI_USE_DMA) {
		host->flags |= SDHCI_REQ_USE_DMA;
	}

	if (host->flags & SDHCI_REQ_USE_DMA) {
		if (host->flags & SDHCI_USE_ADMA) {
			ret = bcmsdhc_adma_table_pre(host, data);
			if (ret) {
				/*
				 * This only happens when someone fed
				 * us an invalid request.
				 */
				WARN_ON(1);
				host->flags &= ~SDHCI_REQ_USE_DMA;
			} else {
				writel(host->adma_addr,
				       host->ioaddr + SDHC_ADMA_ADDRESS);
			}
		} else {
			int sg_cnt;

			sg_cnt = dma_map_sg(mmc_dev(host->mmc),
					    data->sg, data->sg_len,
					    (data->flags & MMC_DATA_READ) ?
					    DMA_FROM_DEVICE : DMA_TO_DEVICE);
			if (sg_cnt == 0) {
				/*
				 * This only happens when someone fed
				 * us an invalid request.
				 */
				WARN_ON(1);
				host->flags &= ~SDHCI_REQ_USE_DMA;
			} else {
				WARN_ON(sg_cnt != 1);
				writel(sg_dma_address(data->sg),
				       host->ioaddr + SDHC_SYSTEMADDRESS_LO);
			}
		}
	}

	/*
	 * Always adjust the DMA selection as some controllers
	 * (e.g. JMicron) can't do PIO properly when the selection
	 * is ADMA.
	 */
	if (host->version >= SDHC_SPEC_200) {
		ctrl = readb(host->ioaddr + SDHC_HOST_CONTROL);
		ctrl &= ~HOSTCTL_CTRL_DMA_MASK;
		if ((host->flags & SDHCI_REQ_USE_DMA)
		    && (host->flags & SDHCI_USE_ADMA)) {
			ctrl |= HOSTCTL_CTRL_ADMA32;
		} else {
			ctrl |= HOSTCTL_CTRL_SDMA;
		}
		writeb(ctrl, host->ioaddr + SDHC_HOST_CONTROL);
	}

	if (!(host->flags & SDHCI_REQ_USE_DMA)) {
		sg_miter_start(&host->sg_miter, data->sg, data->sg_len,
			       SG_MITER_ATOMIC);
		host->blocks = data->blocks;
	}

	/* We do not handle DMA boundaries, so set it to max (512 KiB) */
	writew(SDHCI_MAKE_BLKSZ(7, data->blksz), host->ioaddr + SDHC_BLOCKSIZE);
	writew(data->blocks, host->ioaddr + SDHC_BLOCKCOUNT);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_set_transfer_mode */
/* Description: Set data transfer mode */
/* *************************************************************************************************** */

static bool bcmsdhc_set_transfer_mode(struct bcmsdhc_host *host,
				      struct mmc_command *cmd)
{
	u16 mode = 0;
	u32 mask = 0;
	unsigned long timeout = 1000;

	if (cmd->data == NULL) {
		cmd->error = -EINVAL;
		tasklet_schedule(&host->finish_tasklet);
		return false;
	}
	WARN_ON(!host->data);

	if (cmd->data->blocks > 1) {
		mode |= (TXN_MODE_MULTI_BLOCK | TXN_MODE_BLOCK_COUNT_ENABLE);
	}

	if (cmd->data->flags & MMC_DATA_READ) {
		mode |= TXN_MODE_DATA_DIRECTION_READ;
	}

	if (host->flags & SDHCI_REQ_USE_DMA) {
		mode |= TXN_MODE_DMA;
	}

	if (host->flags & SD_AUTO_ISSUE_CMD12) {	/* Workaround for  BCM2152 */
		mode |= TXN_MODE_AUTO_CMD12;
	}

	if (host->mrq->data && (cmd == host->mrq->data->stop)) {
		mask &= ~STATE_DAT_INHIBIT;
	} else {
		mask |= STATE_DAT_INHIBIT;
	}

	while (readl(host->ioaddr + SDHC_PRESENT_STATE) & mask) {
		if (timeout == 0) {
			pr_err
			    ("%s: Controller never released inhibit bit(s).\n",
			     mmc_hostname(host->mmc));
/*                      bcmsdhc_dumpregs(host); */
			cmd->error = -EIO;
			tasklet_schedule(&host->finish_tasklet);
			return false;
		}
		timeout--;
		mdelay(1);
	}

	writew(mode, host->ioaddr + SDHC_TRANSFERMODE);
	return true;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_finish_data */
/* Description: */
/* *************************************************************************************************** */

static void bcmsdhc_finish_data(struct bcmsdhc_host *host)
{
	struct mmc_data *data;
	u32 hc = 0;

	BUG_ON(!host->data);

	data = host->data;
	host->data = NULL;

	if (host->flags & SDHCI_REQ_USE_DMA) {
		if (host->flags & SDHCI_USE_ADMA) {
			bcmsdhc_adma_table_post(host, data);
		} else {
			dma_unmap_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
				     (data->flags & MMC_DATA_READ) ?
				     DMA_FROM_DEVICE : DMA_TO_DEVICE);
		}
	}
	if (!data->error && (host->flags & SD_AUTO_ISSUE_CMD12)) {
		host->m_fAutoCMD12Success = true;
	}

	/*
	 * The specification states that the block count register must
	 * be updated, but it does not specify at what point in the
	 * data flow. That makes the register entirely useless to read
	 * back so we have to assume that nothing made it to the card
	 * in the event of an error.
	 */
	if (data->error) {
		data->bytes_xfered = 0;
	} else {
		data->bytes_xfered = data->blksz * data->blocks;
	}

	if (data->stop) {
		/*
		 * The controller needs a reset of internal state machines
		 * upon error conditions.
		 */
		if (data->error) {
			hc = readl(host->ioaddr + SDHC_HOST_CONTROL);

			if (host->bcm_plat->external_reset)
				host->bcm_plat->external_reset(host->ioaddr, host->sdhc_slot);
			bcmsdhc_init(host, SOFT_RESET_CMD | SOFT_RESET_DAT);

			writel(hc, host->ioaddr + SDHC_HOST_CONTROL);
		}

		bcmsdhc_send_command(host, data->stop);
	} else {
		tasklet_schedule(&host->finish_tasklet);
	}
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_send_command */
/* Description: prepare and Send SDHC command */
/* *************************************************************************************************** */

static void bcmsdhc_send_command(struct bcmsdhc_host *host,
				 struct mmc_command *cmd)
{
	unsigned long timeout;
	u16 int_status_en = 0, reg_command = 0;

	WARN_ON(host->cmd);

	host->cmd = cmd;
	timeout = 1000;
	host->flags &= ~SD_AUTO_ISSUE_CMD12;
	DBG(KERN_INFO
	    "Slot[%d]=> CMD:[%d]  SDHC_PRESENT_STATE=0x%x; Command Arg=0x%x\n",
	    host->sdhc_slot, cmd->opcode,
	    readl(host->ioaddr + SDHC_PRESENT_STATE), cmd->arg);

	if (cmd->opcode == MMC_STOP_TRANSMISSION) {
		if (host->m_fAutoCMD12Success) {
			bcmsdhc_finish_command(host);
			goto EXIT;
		}
	}

	if (cmd->opcode == MMC_READ_MULTIPLE_BLOCK) {
		host->flags |= SD_AUTO_ISSUE_CMD12;
	}

	host->m_fAutoCMD12Success = false;

	reg_command = (cmd->opcode << CMD_INDEX_SHIFT) & CMD_INDEX_MASK;

	switch (mmc_resp_type(cmd)) {
	case MMC_RSP_NONE:
		break;
	case MMC_RSP_R1_R5_R6_R7:
		reg_command |= CMD_RESPONSE_R1_R5_R6_R7;
		break;
	case MMC_RSP_R1B:
		reg_command |= CMD_RESPONSE_R1B_R5B;
		break;
	case MMC_RSP_R2:
		reg_command |= CMD_RESPONSE_R2;
		break;
	case MMC_RSP_R3_R4:
		reg_command |= CMD_RESPONSE_R3_R4;
		break;
	default:
		pr_emerg("mmc: unhandled response type %02x\n",
			 mmc_resp_type(cmd));
		tasklet_schedule(&host->finish_tasklet);
		goto EXIT;
	}

	while (readl(host->ioaddr + SDHC_PRESENT_STATE) & STATE_CMD_INHIBIT) {
		if (timeout == 0) {
			pr_err
			    ("%s: Controller never released inhibit bit(s).\n",
			     mmc_hostname(host->mmc));
/*                      bcmsdhc_dumpregs(host); */
			cmd->error = -EIO;
			tasklet_schedule(&host->finish_tasklet);
			goto EXIT;
		}
		timeout--;
		mdelay(1);
	}

	mod_timer(&host->timer, jiffies + 10 * HZ);

	int_status_en = readw(host->ioaddr + SDHC_NORMAL_INT_STATUS_ENABLE);

	int_status_en &=
	    ~(NORMAL_INT_ENABLE_CMD_COMPLETE | NORMAL_INT_ENABLE_TRX_COMPLETE |
	      NORMAL_INT_ENABLE_BLOCK_GAP | NORMAL_INT_ENABLE_DMA |
	      NORMAL_INT_ENABLE_BUF_WRITE_RDY | NORMAL_INT_ENABLE_BUF_READ_RDY|
	      NORMAL_INT_ENABLE_BTACKRXEN	|	NORMAL_INT_ENABLE_BTIRQ	);

	if (cmd->data == NULL) {
		int_status_en |= NORMAL_INT_ENABLE_CMD_COMPLETE;
		/* The following is required for the Pegasus. If it is not present, */
		/* command-only transfers will sometimes fail (especially R1B and R5B). */
		writel(0, host->ioaddr + SDHC_SYSTEMADDRESS_LO);
		writew(0, host->ioaddr + SDHC_BLOCKSIZE);
		writew(0, host->ioaddr + SDHC_BLOCKCOUNT);

	} else {
		int_status_en |=
		    (NORMAL_INT_ENABLE_CMD_COMPLETE |
		     NORMAL_INT_ENABLE_TRX_COMPLETE |
		     NORMAL_INT_ENABLE_BLOCK_GAP | NORMAL_INT_ENABLE_DMA |
		     NORMAL_INT_ENABLE_BUF_WRITE_RDY |
		     NORMAL_INT_ENABLE_BUF_READ_RDY);

		reg_command |= CMD_DATA_PRESENT;
		bcmsdhc_prepare_data(host, cmd->data);

		if (!bcmsdhc_set_transfer_mode(host, cmd)) {
			goto EXIT;
		}

	}

	writel(cmd->arg, host->ioaddr + SDHC_ARGUMENT_0);

	/* Turn the clock on.It is turned off in .bcmsdhc_tasklet_finish. */

	bcmsdhc_clock_on(host);

	writew(int_status_en, host->ioaddr + SDHC_NORMAL_INT_STATUS_ENABLE);

	/* Writing the upper byte of the command register starts the command. */
	/* All register initialization must already be complete by this point. */
	writew(reg_command, host->ioaddr + SDHC_COMMAND);

	return;
EXIT:
	return;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_finish_command */
/* Description: */
/* *************************************************************************************************** */

static void bcmsdhc_finish_command(struct bcmsdhc_host *host)
{
	int i;

	BUG_ON(host->cmd == NULL);

	if (host->cmd->flags & MMC_RSP_PRESENT) {
		if (host->cmd->flags & MMC_RSP_136) {
			/* CRC is stripped so we need to do some shifting. */
			for (i = 0; i < 4; i++) {
				host->cmd->resp[i] =
				    readl(host->ioaddr + SDHC_R0 +
					  (3 - i) * 4) << 8;
				if (i != 3) {
					host->cmd->resp[i] |=
					    readb(host->ioaddr + SDHC_R0 +
						  (3 - i) * 4 - 1);
				}
			}
		} else {
			host->cmd->resp[0] = readl(host->ioaddr + SDHC_R0);
		}
	}

	host->cmd->error = 0;

	if (!host->cmd->data) {
		tasklet_schedule(&host->finish_tasklet);
	}
	host->cmd = NULL;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_set_clock */
/* Description: Set the host clock rate */
/* *************************************************************************************************** */

static void bcmsdhc_set_clock(struct bcmsdhc_host *host, unsigned int clock)
{
	int div;
	u16 clk;

	if (clock == host->clock)
		return;

	writew(0, host->ioaddr + SDHC_CLOCK_CONTROL);

	if (clock == 0) {
		goto out;
	}

	for (div = 1; div < 256; div *= 2) {
		if ((host->max_clk / div) <= clock) {
			break;
		}
	}
	div >>= 1;

	clk = div << SDHCI_DIVIDER_SHIFT;
	clk |= CLOCK_INTERNAL_ENABLE;
	host->clk_control = clk;

out:
	host->clock = clock;

}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_clock_on */
/* Description:  Turn on SD host clock */
/* *************************************************************************************************** */

static void bcmsdhc_clock_on(struct bcmsdhc_host *host)
{
	u16 clk;
	unsigned long timeout;

	writew(host->clk_control, host->ioaddr + SDHC_CLOCK_CONTROL);

	timeout = 100;
	while (!
	       ((clk =
		 readw(host->ioaddr + SDHC_CLOCK_CONTROL)) & CLOCK_STABLE)) {
		if (timeout == 0) {
			pr_err("%s: Internal clock never stabilised.\n",
			       mmc_hostname(host->mmc));
			return;
		}
		timeout--;
		mdelay(1);
	}

	writew((host->clk_control | CLOCK_ENABLE),
	       host->ioaddr + SDHC_CLOCK_CONTROL);
	/* Wait max 100 ms */
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_clock_off */
/* Description: Turn off SD host clock */
/* *************************************************************************************************** */

static void bcmsdhc_clock_off(struct bcmsdhc_host *host)
{
	writew(host->clk_control, host->ioaddr + SDHC_CLOCK_CONTROL);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_set_power */
/* Description: Set SD host power voltage */
/* *************************************************************************************************** */

static void bcmsdhc_set_power(struct bcmsdhc_host *host, unsigned short power)
{
	u8 pwr;

	if (host->power == power)
		return;

	if (power == (unsigned short)-1) {
		writeb(0, host->ioaddr + SDHC_POWER_CONTROL);
		goto out;
	}
	/* SD Bus Power must be initially set to 0 when changing voltages */
	writeb(0, host->ioaddr + SDHC_POWER_CONTROL);

	pwr = SDBUS_POWER_ON;

	switch (1 << power) {
	case MMC_VDD_165_195:
		pwr |= SDBUS_VOLTAGE_SELECT_1_8V;
		break;
	case MMC_VDD_29_30:
	case MMC_VDD_30_31:
		pwr |= SDBUS_VOLTAGE_SELECT_3_0V;
		break;
	case MMC_VDD_32_33:
	case MMC_VDD_33_34:
		pwr |= SDBUS_VOLTAGE_SELECT_3_3V;
		break;
	default:
		BUG();
	}

	writeb(pwr, host->ioaddr + SDHC_POWER_CONTROL);

out:
	host->power = power;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_request */
/* Description: MMC callback */
/* *************************************************************************************************** */

static void bcmsdhc_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct bcmsdhc_host *host;
	unsigned long flags;
	u8 set_voltage;

	host = mmc_priv(mmc);
	spin_lock_irqsave(&host->lock, flags);

	WARN_ON(host->mrq != NULL);

#ifndef CONFIG_LEDS_CLASS
	bcmsdhc_activate_led(host);
#endif

	host->mrq = mrq;
	clk_enable(host->bus_clk);
	sdhc_update_qos_req(host, 0);

#ifdef CONFIG_HAS_WAKELOCK
	/* Check for the CMD type (expect response or not) */
	if (mmc_resp_type(mrq->cmd) == MMC_RSP_NONE) {
		/* Not valid to use wake_lock_timeout when CMD with no response expected */
		wake_lock(&host->sdhc_wakelock);
	} else {
		/*Wakelock for the bcm_sdhc should be active for the longer period when CMD expects response from the card */
		wake_lock_timeout(&host->sdhc_wakelock, msecs_to_jiffies(500));
	}
#endif

#if defined(CONFIG_ARCH_BCM116X)  || defined(CONFIG_ARCH_BCM215XX)
	/* For SD card power saving */
	set_voltage = readb(host->ioaddr + SDHC_POWER_CONTROL);
	set_voltage |= SDBUS_POWER_ON;
	writeb(set_voltage, host->ioaddr + SDHC_POWER_CONTROL);
#endif
	/* BUG: Comment pulling down the dat/clk lines on finish */
/*	if (host->bcm_plat->syscfg_interface(SYSCFG_SDHC1 +
		host->sdhc_slot - 1, SYSCFG_ENABLE))
		return; */

	/* bcmsdhc_dumpregs(host); */
	if (host->card_present == false) {
		host->mrq->cmd->error = -ENOMEDIUM;
		tasklet_schedule(&host->finish_tasklet);
	} else {
		bcmsdhc_send_command(host, mrq->cmd);
	}
	/* Unlock the wakelock immediately after sending the command with no response
	   On PM suspend case, deselcting card CMD will be send with no response */
#ifdef CONFIG_HAS_WAKELOCK
	if (mmc_resp_type(mrq->cmd) == MMC_RSP_NONE) {
		/* unlock after CMD transaction */
		wake_unlock(&host->sdhc_wakelock);
	}
#endif
	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_set_ios */
/* Description:MMC callback */
/* *************************************************************************************************** */

static void bcmsdhc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct bcmsdhc_host *host;
	unsigned long flags;
	u8 ctrl;
	int result = 0;

	host = mmc_priv(mmc);

	DBG(KERN_INFO "SDHC:Slot[%d]=> set_ios,power_mode=%d;bus_width=%d;"
	    "timing=%d;clock=%d;vdd=%d\n", host->sdhc_slot, ios->power_mode,
	    ios->bus_width, ios->timing, ios->clock, ios->vdd);

	switch (ios->power_mode) {
	case MMC_POWER_OFF:
#ifdef CONFIG_REGULATOR
		if(host->vcc) {
			/* Disable regulator here */
			printk(KERN_INFO "SDHC:Slot[%d]=> regulator_disable\n", host->sdhc_slot);
			/* regulator_disable(host->vcc); */
		}
#endif
		break;
	case MMC_POWER_UP:
#ifdef CONFIG_REGULATOR
		if (host->vcc) {
			printk(KERN_INFO "SDHC:Slot[%d]=> regulator_enable\n", host->sdhc_slot);
			result = regulator_enable(host->vcc);
		}
#endif
		break;
	}

	spin_lock_irqsave(&host->lock, flags);

	clk_enable(host->bus_clk);
	sdhc_update_qos_req(host, 0);
	/*
	 * Reset the chip on each power off.
	 * Should clear out any weird states.
	 */
	if (ios->power_mode == MMC_POWER_OFF) {
		writel(0, host->ioaddr + SDHC_NORMAL_INT_SIGNAL_ENABLE);
		bcmsdhc_init(host, SOFT_RESET_ALL);
	}

	bcmsdhc_set_clock(host, ios->clock);

	if (ios->power_mode == MMC_POWER_OFF) {
		bcmsdhc_set_power(host, -1);
	} else {
		bcmsdhc_set_power(host, ios->vdd);
	}
	ctrl = readb(host->ioaddr + SDHC_HOST_CONTROL);

	if (ios->bus_width == MMC_BUS_WIDTH_4) {
		ctrl |= HOSTCTL_DAT_4BIT_WIDTH;
		host->bus_4bit_mode = true;
	} else {
		ctrl &= ~HOSTCTL_DAT_4BIT_WIDTH;
		host->bus_4bit_mode = false;
	}
	if ((ios->timing == MMC_TIMING_SD_HS)
	    || (ios->timing == MMC_TIMING_MMC_HS)) {
		ctrl |= HOSTCTL_HIGH_SPEED;
	} else {
		ctrl &= ~HOSTCTL_HIGH_SPEED;
	}
	writeb(ctrl, host->ioaddr + SDHC_HOST_CONTROL);

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);

	clk_disable(host->bus_clk);
	sdhc_update_qos_req(host, PM_QOS_DEFAULT_VALUE);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_get_ro */
/* Description: MMC callback */
/* *************************************************************************************************** */

static int bcmsdhc_get_ro(struct mmc_host *mmc)
{
	struct bcmsdhc_host *host;
	unsigned long flags;
	int present;

	host = mmc_priv(mmc);

	spin_lock_irqsave(&host->lock, flags);

	clk_enable(host->bus_clk);
	sdhc_update_qos_req(host, 0);
	if (host->card_present == false) {
		present = 0;
	} else {
		present = readl(host->ioaddr + SDHC_PRESENT_STATE);
	}

	clk_disable(host->bus_clk);
	spin_unlock_irqrestore(&host->lock, flags);
	sdhc_update_qos_req(host, PM_QOS_DEFAULT_VALUE);
	return !(present & STATE_WRITE_PROTECT);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_enable_sdio_irq */
/* Description:The function is for Sdio interrupt enable/disable */
/* *************************************************************************************************** */

static void bcmsdhc_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct bcmsdhc_host *host;
	unsigned long flags;
	u32 ier, tmp2;

	host = mmc_priv(mmc);

	spin_lock_irqsave(&host->lock, flags);

	if (host->card_present == false) {

		goto out;
	}

	ier = readl(host->ioaddr + SDHC_NORMAL_INT_STATUS_ENABLE);

	ier &= ~(NORMAL_INT_ENABLE_CARD_INT|NORMAL_INT_ENABLE_BTACKRXEN|NORMAL_INT_ENABLE_BTIRQ);			
	host->enable_sdio_interrupt = false;

	tmp2 = readl(host->ioaddr + SDHC_NORMAL_INT_SIGNAL_ENABLE);
	if (enable) {
		ier |= NORMAL_INT_ENABLE_CARD_INT;
		host->enable_sdio_interrupt = true;

		tmp2 |= NORMAL_INT_ENABLE_CARD_INT;
	}

	writel(tmp2, host->ioaddr + SDHC_NORMAL_INT_SIGNAL_ENABLE);
	writel(ier, host->ioaddr + SDHC_NORMAL_INT_STATUS_ENABLE);

out:
	mmiowb();

	spin_unlock_irqrestore(&host->lock, flags);
}

int bcmsdhc_get_cd(struct mmc_host *mmc_host)
{
	struct bcmsdhc_host *host;
	int state;

	host = mmc_priv(mmc_host);

	//check GPIO value connected to SD
	if(host->irq_cd>0)
	{
		state = gpio_get_value(host->irq_cd);	
		if (state != (host->bcm_plat->flags & SDHC_CARD_DETECT_ACTIVE_HIGH)) 
		{
			pr_info("%s: %s: SD Card removed physically <-->host->card_present:%d!\n",__func__, mmc_hostname(host->mmc),host->card_present);
		} 
		else 
		{
			pr_info("%s: %s: SD Card Inserted physically<-->host->card_present:%d\n",__func__, mmc_hostname(host->mmc),host->card_present);
		}	
	}

	if (host->card_present == false) {
		return 0; /* Card removed */
	}

	return 1; /* Card inserted */
}

static const struct mmc_host_ops bcmsdhc_ops = {
	.request = bcmsdhc_request,
	.set_ios = bcmsdhc_set_ios,
	.get_ro = bcmsdhc_get_ro,
	.get_cd = bcmsdhc_get_cd,
	.enable_sdio_irq = bcmsdhc_enable_sdio_irq,
};

//sysfs - for sd h/w detect

static ssize_t show_sddet(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", sd_inserted);
}

static DEVICE_ATTR(sd_det, 0444, show_sddet, NULL);


/* *************************************************************************************************** */
/* Function Name: bcmsdhc_tasklet_card */
/* Description:The tasklet is for card insert/remove */
/* *************************************************************************************************** */

static void bcmsdhc_tasklet_card(unsigned long param)
{
	struct bcmsdhc_host *host;
	unsigned long flags;
	int state;
	int result = 0;

	host = (struct bcmsdhc_host *)param;
	state = gpio_get_value(host->irq_cd);
	spin_lock_irqsave(&host->lock, flags);

	if (state != (host->bcm_plat->flags & SDHC_CARD_DETECT_ACTIVE_HIGH)) {
		host->card_present = false;
		if (host->mrq) {
			pr_info(" %s: Card removed during transfer!\n",
				mmc_hostname(host->mmc));
			bcmsdhc_reset(host, SOFT_RESET_CMD);
			bcmsdhc_reset(host, SOFT_RESET_DAT);

			host->mrq->cmd->error = -ENOMEDIUM;
			tasklet_schedule(&host->finish_tasklet);
		}
		pr_info(" %s: SD Card removed !\n", mmc_hostname(host->mmc));
	} else {
		pr_info("%s: SD Card Insert !\n", mmc_hostname(host->mmc));

		host->card_present = true;
	}
	sd_inserted = host->card_present;

	spin_unlock_irqrestore(&host->lock, flags);

	// When the card is rejrcted, the sd power should turn on!
	// struct regulator *regulator_get(struct device *dev, const char *id)
	if(!host->card_present)
	{
		if(!strcmp(mmc_hostname(host->mmc), "mmc1"))
		{
			if(host->vcc)
			{
			/* 	-------------------------------------------------------------------------------------------- */
			/*	printk(KERN_INFO "%s's vreg: Name %s / Use Count %d / Open Count %d\n", mmc_hostname(host->mmc), 
						host->vcc->supply_name, host->vcc->rdev->use_count, host->vcc->rdev->open_count); */
			/*	-------------------------------------------------------------------------------------------- */
				result = regulator_enable(host->vcc);
				printk(KERN_INFO "%s's vreg[Turn On]: Name %s / Use Count %d / Open Count %d / Result %d\n", mmc_hostname(host->mmc),
							host->vcc->supply_name, host->vcc->rdev->use_count, host->vcc->rdev->open_count, result);

			if(host->vcc->rdev->use_count > 65535) 
				host->vcc->rdev->use_count = 0;
			}
		}
	}

	mmc_detect_change(host->mmc, msecs_to_jiffies(500));
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_tasklet_finish */
/* Description: */
/* *************************************************************************************************** */

static void bcmsdhc_tasklet_finish(unsigned long param)
{
	struct bcmsdhc_host *host;
	unsigned long flags;
	struct mmc_request *mrq;
	u8 set_voltage;

	host = (struct bcmsdhc_host *)param;

	spin_lock_irqsave(&host->lock, flags);

	del_timer(&host->timer);

	mrq = host->mrq;

	if ((host->enable_sdio_interrupt && host->bus_4bit_mode) == false) {
		bcmsdhc_clock_off(host);
	}
	/* else need to leave clock on in order to receive interrupts in 4 bit mode */

	/* bcmsdhc_clock_off(host); */

	writew(0, host->ioaddr + SDHC_NORMAL_INT_STATUS_ENABLE);

	set_voltage = readb(host->ioaddr + SDHC_POWER_CONTROL);
	set_voltage &= ~SDBUS_POWER_ON;
	writeb(set_voltage, host->ioaddr + SDHC_POWER_CONTROL);

	/* BUG: Comment pulling down the dat/clk lines on finish */
/*	if (host->bcm_plat->syscfg_interface(SYSCFG_SDHC1 +
		host->sdhc_slot - 1, SYSCFG_DISABLE))
		return; */

	if (mrq->cmd->error ||
	    (mrq->data && (mrq->data->error ||
			   (mrq->data->stop && mrq->data->stop->error)))) {
		/* The controller needs a reset upon error conditions */
		bcmsdhc_reset(host, SOFT_RESET_CMD);
		bcmsdhc_reset(host, SOFT_RESET_DAT);
	}

	host->mrq = NULL;
	host->cmd = NULL;
	host->data = NULL;

#ifndef CONFIG_LEDS_CLASS
	bcmsdhc_deactivate_led(host);
#endif

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);

	mmc_request_done(host->mmc, mrq);
	clk_disable(host->bus_clk);
	sdhc_update_qos_req(host, PM_QOS_DEFAULT_VALUE);
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_timeout_timer */
/* Description: handle the interrupt timeout after sending command */
/* *************************************************************************************************** */

static void bcmsdhc_timeout_timer(unsigned long data)
{
	struct bcmsdhc_host *host;
	unsigned long flags;

	host = (struct bcmsdhc_host *)data;

	spin_lock_irqsave(&host->lock, flags);

	if (host->mrq) {
		/*
		 * If timeout, re-send the command for maximum 5 retries. Refers to
		 * the entry E5_2820A2 of BCM2820_A2_Errata_v1.1_CUSTOMER.doc
		 */
		if (host->cmd == NULL) {
			pr_err
			    ("%s: Timeout waiting for transfer complete interrupt.\n",
			     mmc_hostname(host->mmc));
		} else {
			if (host->card_present == true) {
				if (host->cmd->retries >= 1) {
					struct mmc_command *cmd;
					host->cmd->retries--;
					cmd = host->cmd;
					host->cmd = NULL;
					bcmsdhc_send_command(host, cmd);
					spin_unlock_irqrestore(&host->lock,
							       flags);
					return;
				} else {
					pr_info
					    ("%s: Timeout waiting for command complete interrupt after 5 retries.\n",
					     mmc_hostname(host->mmc));
				}
			}
		}

		pr_info("%s: Timeout waiting for hardware interrupt.\n",
			mmc_hostname(host->mmc));
/*              bcmsdhc_dumpregs(host); */

		if (host->data) {
			host->data->error = -ETIMEDOUT;
			bcmsdhc_finish_data(host);
		} else {
			if (host->cmd) {
				host->cmd->error = -ETIMEDOUT;
			} else {
				host->mrq->cmd->error = -ETIMEDOUT;
			}

			tasklet_schedule(&host->finish_tasklet);
		}
	}

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);
}

static void sdhci_show_adma_error(struct bcmsdhc_host *host)
{
	const char *name = mmc_hostname(host->mmc);
	u8 *desc = host->adma_desc;
	__le32 *dma;
	__le16 *len;
	u8 attr;

	bcmsdhc_dumpregs(host);

	while (true) {
		dma = (__le32 *)(desc + 4);
		len = (__le16 *)(desc + 2);
		attr = *desc;

		DBG("%s: %p: DMA 0x%08x, LEN 0x%04x, Attr=0x%02x\n",
		    name, desc, le32_to_cpu(*dma), le16_to_cpu(*len), attr);

		desc += 8;

		if (attr & 2)
			break;
	}
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_handle_errors */
/* Description: Handle host Errors */
/* *************************************************************************************************** */

static void bcmsdhc_handle_errors(struct bcmsdhc_host *host)
{
	u16 wErrorStatus = 0, wErrIntSignal;
	wErrorStatus = readw(host->ioaddr + SDHC_ERROR_INT_STATUS);
	DBG(KERN_INFO "%s :Slot[%d]=>SDHC_ERROR_INT_STATUS=%x\n", __func__,
	    host->sdhc_slot, wErrorStatus);

	if (wErrorStatus & ERR_INT_STATUS_CMD_MASK) {
		if (!host->cmd) {
			/* clear all error status */
			writew(wErrorStatus & ERR_INT_STATUS_CMD_MASK,
					host->ioaddr + SDHC_ERROR_INT_STATUS);

			printk(KERN_ERR "%s: Got command interrupt 0x%08x even "
				"though no command operation was in progress.\n",
				mmc_hostname(host->mmc),
				(unsigned)(wErrorStatus & ERR_INT_STATUS_CMD_MASK));
			bcmsdhc_dumpregs(host);
			return;
		}
	}

	if (wErrorStatus & ERR_INT_STATUS_DAT_MASK) {
		if (!host->data) {
			/* clear all error status */
			writew(wErrorStatus & ERR_INT_STATUS_DAT_MASK,
					host->ioaddr + SDHC_ERROR_INT_STATUS);

			printk(KERN_ERR "%s: Got data interrupt 0x%08x even "
				"though no data operation was in progress.\n",
				mmc_hostname(host->mmc),
				(unsigned)(wErrorStatus & ERR_INT_STATUS_DAT_MASK));
			bcmsdhc_dumpregs(host);
			return;
		}
	}

	if (wErrorStatus) {
		if (wErrorStatus & ERR_INT_STATUS_CMD_TIMEOUT) {
			host->cmd->error = -ETIMEDOUT;
		}

		if (wErrorStatus & ERR_INT_STATUS_CMD_CRC) {
			host->cmd->error = -EILSEQ;
		}

		if (wErrorStatus & ERR_INT_STATUS_CMD_ENDBIT) {
			host->cmd->error = -EILSEQ;
		}

		if (wErrorStatus & ERR_INT_STATUS_CMD_INDEX) {
			host->cmd->error = -EILSEQ;
		}

		if (wErrorStatus & ERR_INT_STATUS_DAT_TIMEOUT) {
			host->data->error = -ETIMEDOUT;
		}

		if (wErrorStatus & ERR_INT_STATUS_DAT_CRC) {
			host->data->error = -EILSEQ;
		}

		if (wErrorStatus & ERR_INT_STATUS_DAT_ENDBIT) {
			host->data->error = -EILSEQ;
		}

		if (wErrorStatus & ERR_INT_STATUS_BUS_POWER) {
			host->data->error = -EHOSTDOWN;
		}

		if (wErrorStatus & ERR_INT_STATUS_AUTOCMD12) {
			host->data->error = -EILSEQ;
		}

		if (wErrorStatus & ERR_INT_STATUS_ADMAERR) {
			host->data->error = -EIO;
			sdhci_show_adma_error(host);
		}
		/* Perform basic error recovery */
		wErrIntSignal =
		    readw(host->ioaddr + SDHC_ERROR_INT_SIGNAL_ENABLE);
		writew(0, host->ioaddr + SDHC_ERROR_INT_SIGNAL_ENABLE);

		if (IS_CMD_LINE_ERROR(wErrorStatus)) {
			/* Reset CMD line */
			bcmsdhc_reset(host, SOFT_RESET_CMD);
			mdelay(2);
			while (readb(host->ioaddr + SDHC_SOFT_RESET)) {
				/* Wait for softreset completes */
			}
		}

		if (IS_DAT_LINE_ERROR(wErrorStatus)) {
			/* Reset DAT line */
			bcmsdhc_reset(host, SOFT_RESET_DAT);

			if (host->data->error) {
				host->data->bytes_xfered = 0;
			}
			mdelay(2);
			while (readb(host->ioaddr + SDHC_SOFT_RESET)) {
				/* Wait for softreset completes */
			}
		}

		mdelay(1);

		/* clear all error status */
		writew(wErrorStatus, host->ioaddr + SDHC_ERROR_INT_STATUS);

		/* re-enable error interrupt signals */
		writew(wErrIntSignal,
		       host->ioaddr + SDHC_ERROR_INT_SIGNAL_ENABLE);

		if (host->data && host->data->error) {
			bcmsdhc_finish_data(host);
		} else {
			/* complete the request */
			tasklet_schedule(&host->finish_tasklet);
		}
	}

	return;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_cd */
/* Description: ISR for the CardDetect Pin */
/* *************************************************************************************************** */

static irqreturn_t bcmsdhc_cd(int irq, void *dev_id)
{
	struct bcmsdhc_host *host = dev_id;
	disable_irq_nosync(irq);
	tasklet_schedule(&host->card_tasklet);
	enable_irq(irq);
	return IRQ_HANDLED;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_irq */
/* Description: ISR for SD host interrupt */
/* *************************************************************************************************** */

static irqreturn_t bcmsdhc_irq(int irq, void *dev_id)
{

	irqreturn_t result;
	struct bcmsdhc_host *host = dev_id;
	u16 wIntStatus;

	int cardint = 0;

	spin_lock(&host->lock);

	/* Enable the clock here in case we receive an interrupt after
	 * timeout_timer is called, a very corner case. We call disable
	 * at the end of ISR to balance this.
	 */
	clk_enable(host->bus_clk);

	wIntStatus = readw(host->ioaddr + SDHC_NORMAL_INT_STATUS);

	if (wIntStatus != 0) {
		DBG(KERN_INFO
			"Slot[%d]=>NORMAL_INT_STATUS=%x; ERROR_INT_STATUS=%x\n",
			host->sdhc_slot, (unsigned)wIntStatus,
			readw(host->ioaddr + SDHC_ERROR_INT_STATUS));

		if (wIntStatus & NORMAL_INT_STATUS_ERROR_INT) {
			wIntStatus &= ~NORMAL_INT_STATUS_ERROR_INT;
			bcmsdhc_handle_errors(host);
		}

		if (wIntStatus &
			(NORMAL_INT_STATUS_CARD_INSERTION |
			 NORMAL_INT_STATUS_CARD_REMOVAL)) {
			writew(wIntStatus &
				   (NORMAL_INT_STATUS_CARD_INSERTION |
				NORMAL_INT_STATUS_CARD_REMOVAL),
				   host->ioaddr + SDHC_NORMAL_INT_STATUS);
			wIntStatus &=
				~(NORMAL_INT_STATUS_CARD_INSERTION |
				  NORMAL_INT_STATUS_CARD_REMOVAL);
		}

		if (wIntStatus & NORMAL_INT_STATUS_CMD_COMPLETE) {
			writew(NORMAL_INT_STATUS_CMD_COMPLETE,
				   host->ioaddr + SDHC_NORMAL_INT_STATUS);
			wIntStatus &= ~(NORMAL_INT_STATUS_CMD_COMPLETE);

			host->command_complete = 1;
			if (!host->cmd) {
				pr_err
					("%s: Got command interrupt 0x%08x even though no command operation was in progress.\n",
					 mmc_hostname(host->mmc),
					 (unsigned)wIntStatus);
				bcmsdhc_dumpregs(host);
			} else {
				bcmsdhc_finish_command(host);
				if (!host->data) {
					wIntStatus &=
						~NORMAL_INT_STATUS_TRX_COMPLETE;
					host->command_complete = 0;
				}

			}
		}
		/* Sometimes at the lowest clock rate, the Read/WriteBufferReady */
		/* interrupt actually occurs before the CommandComplete interrupt. */
		/* This confuses our debug validation code and could potentially */
		/* cause problems. This is why we will verify that the CommandComplete */
		/* occurred before processing any data transfer interrupts. */

		if (host->command_complete == 1) {
			/* Buffer Read Ready handling */
			if (wIntStatus & NORMAL_INT_STATUS_BUF_READ_RDY) {
				/* Clear status */
				writew(NORMAL_INT_STATUS_BUF_READ_RDY,
					   host->ioaddr + SDHC_NORMAL_INT_STATUS);
				wIntStatus &= ~(NORMAL_INT_STATUS_BUF_READ_RDY);
				if (!host->data) {
					pr_err("%s: Got data interrupt 0x%08x even "
						"though no data operation was in progress.\n",
						mmc_hostname(host->mmc),
						(unsigned)wIntStatus);
					bcmsdhc_dumpregs(host);
				} else {
					bcmsdhc_transfer_pio(host);
				}
				/* do not break here. Continue to check TransferComplete. */
			}
			/* Buffer Write Ready handling */
			if (wIntStatus & NORMAL_INT_STATUS_BUF_WRITE_RDY) {
				/* Clear status */
				writew(NORMAL_INT_STATUS_BUF_WRITE_RDY,
					   host->ioaddr + SDHC_NORMAL_INT_STATUS);
				wIntStatus &=
					~(NORMAL_INT_STATUS_BUF_WRITE_RDY);
				if (!host->data) {
					pr_err("%s: Got data interrupt 0x%08x even "
					   "though no data operation was in progress.\n",
					   mmc_hostname(host->mmc),
					   (unsigned)wIntStatus);
					bcmsdhc_dumpregs(host);
				} else {
					bcmsdhc_transfer_pio(host);
				}
				/* do not break here. Continue to check TransferComplete. */
			}

			if (wIntStatus & NORMAL_INT_STATUS_DMA) {

				writew(NORMAL_INT_STATUS_DMA,
					   host->ioaddr + SDHC_NORMAL_INT_STATUS);
				wIntStatus &= ~(NORMAL_INT_STATUS_DMA);
				/* do not break here. Continue to check TransferComplete. */
			}
		} else {
			/* We received data transfer interrupt before command */
			/* complete interrupt. Wait for the command complete before */
			/* processing the data interrupt. */
		}

		/* Transfer Complete handling */
		if (wIntStatus & NORMAL_INT_STATUS_TRX_COMPLETE) {
			/* Clear status */
			writew((wIntStatus &
				(NORMAL_INT_STATUS_TRX_COMPLETE |
				 NORMAL_INT_STATUS_DMA)),
				   host->ioaddr + SDHC_NORMAL_INT_STATUS);
			wIntStatus &=
				~(NORMAL_INT_STATUS_TRX_COMPLETE |
				  NORMAL_INT_STATUS_DMA);
			if (!host->data) {
				pr_err("%s: Got data interrupt 0x%08x even "
					"though no data operation was in progress.\n",
					mmc_hostname(host->mmc),
					(unsigned)wIntStatus);
				bcmsdhc_dumpregs(host);
			} else {
				bcmsdhc_finish_data(host);
			}
		}

		if (wIntStatus & NORMAL_INT_STATUS_CARD_INT) {
			cardint = 1;
			wIntStatus &= ~NORMAL_INT_STATUS_CARD_INT;
		}

		if (wIntStatus) {
			pr_info("%s: Unexpected interrupt 0x%08x.\n",
				 mmc_hostname(host->mmc), wIntStatus);
			bcmsdhc_dumpregs(host);
			
			if((wIntStatus&NORMAL_INT_ENABLE_BTACKRXEN) ||(wIntStatus&NORMAL_INT_ENABLE_BTIRQ)){	   
				u16 int_status_en = 0;
				int i;
				char sd_reg_num=32;

				pr_info("SD[%d]-slot Register Dump - start\n", host->sdhc_slot);
				
				for(i=0;i<sd_reg_num;i++){
					pr_info("[0x%x]:0x%x\n",(unsigned int)(host->ioaddr+i*4),readl(host->ioaddr+i*4));
				}
				
				pr_info("SD Register Dump - complete\n");

				
				int_status_en = readw(host->ioaddr + SDHC_NORMAL_INT_STATUS_ENABLE);
				int_status_en &= ~(NORMAL_INT_ENABLE_BTACKRXEN|NORMAL_INT_ENABLE_BTIRQ);
				writew(int_status_en, host->ioaddr + SDHC_NORMAL_INT_STATUS_ENABLE);
				bcmsdhc_reset(host, SOFT_RESET_CMD);
				bcmsdhc_reset(host, SOFT_RESET_DAT);
			}
			writel(wIntStatus,
				   host->ioaddr + SDHC_NORMAL_INT_STATUS);
		}
	} else {
		result = IRQ_NONE;
		goto out;
	}

	result = IRQ_HANDLED;
	mmiowb();
out:

	clk_disable(host->bus_clk);
	spin_unlock(&host->lock);

	/*
	 * We have to delay this as it calls back into the driver.
	 */
	if (cardint)
		mmc_signal_sdio_irq(host->mmc);

	return result;
}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_probe */
/* Description: probe SD host */
/* *************************************************************************************************** */

static int __init bcmsdhc_probe(struct platform_device *pdev)
{
	struct bcmsdhc_platform_data *bcm_plat = pdev->dev.platform_data;
	struct mmc_host *mmc;
	struct bcmsdhc_host *host;
	char drv_name[20];

	unsigned int caps = 0;
	int ret = 0, i;

	struct resource *irqres = NULL;
	struct resource *memres = NULL;
	/* must have platform data */

	if (!bcm_plat) {
		pr_err("%s: Platform data not available\n", __func__);
		return -EINVAL;
	}

	if (pdev->resource == NULL || pdev->num_resources < 2) {
		pr_err("%s:SDHC: Invalid resource and num_resources\n",
		       __func__);
		return -ENXIO;
	}

	for (i = 0; i < pdev->num_resources; i++) {
		if (pdev->resource[i].flags & IORESOURCE_MEM) {
			memres = &pdev->resource[i];
		}
		if (pdev->resource[i].flags & IORESOURCE_IRQ) {
			irqres = &pdev->resource[i];
		}
	}
	if (!irqres || !memres) {
		pr_err("%s:SDHC: Invalid irq and memery\n", __func__);
		return -ENXIO;
	}

	mmc = mmc_alloc_host(sizeof(struct bcmsdhc_host), &pdev->dev);
	if (!mmc) {
		pr_err("failed to mmc_alloc_host\n");
		return -ENOMEM;
	}

	//sysfs for sd hw detect
	if(device_create_file(&pdev->dev, &dev_attr_sd_det)<0)
		pr_err("failed to create sd_det sysfs file.\n");
	host = mmc_priv(mmc);
	host->sdhc_slot = pdev->id;
	host->mmc = mmc;
	host->bcm_plat = bcm_plat;
	host->irq_cd = bcm_plat->irq_cd;

	host->irq = irqres->start;

	host->ioaddr = (void __iomem *)memres->start;
	pr_info("sdhc[%d], regulator: %s, start %x - end %x, irq %d cd %d %p\n",
		host->sdhc_slot, bcm_plat->regl_id ,memres->start, memres->end,
		irqres->start, bcm_plat->irq_cd, host->ioaddr);
	if (!host->ioaddr) {
		pr_err("failed to remap registers\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, host);

	host->clk_control = 0;
	host->card_present = true;
	host->m_fAutoCMD12Success = false;
	host->enable_sdio_interrupt = false;
	host->bus_4bit_mode = false;

	/* Reset the entire host controller core */
	if (host->bcm_plat->external_reset &&
	    host->bcm_plat->external_reset(host->ioaddr, host->sdhc_slot)) {
		goto host_free;
	}

	/* Configure system IOCR2 SD registry */
	ret = host->bcm_plat->syscfg_interface(SYSCFG_SDHC1 +
					       host->sdhc_slot - 1,
					       SYSCFG_INIT);
	if (ret)
		goto host_free;

	sprintf(drv_name, "%s.%d", pdev->name, pdev->id);
	host->bus_clk = clk_get(NULL, drv_name);
	pr_info("Clock name %s %p\n", drv_name, host->bus_clk);

	if (IS_ERR(host->bus_clk)) {
		ret = PTR_ERR(host->bus_clk);
		goto host_free;
	}
	ret = clk_enable(host->bus_clk);
	if (ret)
		goto bus_clk_put;

	ret = clk_set_rate(host->bus_clk, host->bcm_plat->base_clk);
	if (ret)
		goto bus_clk_disable;

	host->version = readw(host->ioaddr + SDHC_HOST_CONTROLLER_VER);
	host->version =
	    (host->version & SDHC_SPEC_VER_MASK) >> SDHC_SPEC_VER_SHIFT;
	if (host->version > SDHC_SPEC_201) {
		pr_err("%s: Unknown controller version (%d). "
		       "You may experience problems.\n", mmc_hostname(mmc),
		       host->version);
	}
	/* 0x69ef30b0 */
	caps = readl(host->ioaddr + SDHC_CAPABILITIES);
	pr_info("SDHC:slot[%d]=>Host version = %d ,Host capabilities =0x%x\n",
		host->sdhc_slot, readw(host->ioaddr + SDHC_HOST_CONTROLLER_VER),
		caps);

	if (caps & SDHC_CAN_DO_DMA) {
		host->flags |= SDHCI_USE_DMA;
	}

	if ((host->version >= SDHC_SPEC_200) && (caps & SDHC_CAN_DO_ADMA2))
		host->flags |= SDHCI_USE_ADMA;

	if (host->flags & SDHCI_USE_ADMA) {
		/*
		 * We need to allocate descriptors for all sg entries
		 * (128) and potentially one alignment transfer for
		 * each of those entries.
		 */
		host->adma_desc = kmalloc((128 * 2 + 1) * 4, GFP_KERNEL);
		host->align_buffer = kmalloc(128 * 4, GFP_KERNEL);
		if (!host->adma_desc || !host->align_buffer) {
			kfree(host->adma_desc);
			kfree(host->align_buffer);
			pr_warning("%s: Unable to allocate ADMA "
				   "buffers. Falling back to standard DMA.\n",
				   mmc_hostname(mmc));
			host->flags &= ~SDHCI_USE_ADMA;
		}
	}

	/*
	 * If we use DMA, then it's up to the caller to set the DMA
	 * mask, but PIO does not need the hw shim so we set a new
	 * mask here in that case.
	 */
	if (!(host->flags & SDHCI_USE_DMA)) {
		host->dma_mask = DMA_BIT_MASK(64);
		mmc_dev(host->mmc)->dma_mask = &host->dma_mask;
	}
	host->max_clk = host->bcm_plat->base_clk;
/*
	host->max_clk =
		(caps & SDHCI_CLOCK_BASE_MASK) >> SDHCI_CLOCK_BASE_SHIFT;
	if (host->max_clk == 0) {
		host->max_clk = 52; 52Mhz
	}
	host->max_clk *= 1000000;
*/
	host->timeout_clk =
	    (caps & SDHC_TIMEOUT_CLK_MASK) >> SDHC_TIMEOUT_CLK_SHIFT;
	if (host->timeout_clk == 0) {
		host->timeout_clk = host->bcm_plat->base_clk / 1000;
	}
	if (caps & SDHC_TIMEOUT_CLK_UNIT)
		host->timeout_clk *= 1000;

	host->max_block = (caps & SDHC_MAX_BLOCK_MASK) >> SDHC_MAX_BLOCK_SHIFT;
	if (host->max_block > 3) {
		/* 0:512 Bytes; 1:1024 bytes; 2:2048 bytes;3:4096 bytes */
		host->max_block = 3;
	}
	host->max_block = 512 << host->max_block;

	DBG(KERN_INFO "\nthe host supports block size=%d\n", host->max_block);

	/*
	 * Set host parameters.
	 */
	mmc->ops = &bcmsdhc_ops;
	mmc->f_min = host->max_clk / 256;
	mmc->f_max = host->max_clk;
	mmc->caps = MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ;
	if (host->bcm_plat->flags & SDHC_MANUAL_SUSPEND_RESUME)
		mmc->bus_resume_flags = MMC_BUSRESUME_MANUAL_RESUME |
				MMC_BUSRESUME_NEEDS_RESUME;

	if (caps & SDHC_CAN_DO_HISPD) {
		mmc->caps |= MMC_CAP_SD_HIGHSPEED;
		mmc->caps |= MMC_CAP_MMC_HIGHSPEED;
	}
#ifdef CONFIG_REGULATOR
	/* If we're using the regulator framework, try to fetch a regulator */
	if (host->bcm_plat->regl_id) {
		host->vcc = regulator_get(&pdev->dev, host->bcm_plat->regl_id);
		if (IS_ERR(host->vcc)) {
			host->vcc = NULL;
		} else {
			/* Set voltage only to override the default regulator voltage */
			if (host->bcm_plat->reg_minuv && host->bcm_plat->reg_maxuv)
				regulator_set_voltage(host->vcc, host->bcm_plat->reg_minuv,
						host->bcm_plat->reg_maxuv);
			regulator_enable(host->vcc);
		}
	}
#endif

	mmc->ocr_avail = 0;
	if (caps & SDHC_CAN_VDD_330) {
		mmc->ocr_avail |= MMC_VDD_32_33 | MMC_VDD_33_34;
	}
	if (caps & SDHC_CAN_VDD_300) {
		mmc->ocr_avail |= MMC_VDD_29_30 | MMC_VDD_30_31;
	}
	if (caps & SDHC_CAN_VDD_180) {
		mmc->ocr_avail |= MMC_VDD_165_195;
	}

	if (mmc->ocr_avail == 0) {
		pr_err("%s: Hardware doesn't report any support voltages.\n",
		       mmc_hostname(mmc));
		return -ENODEV;
	}

	spin_lock_init(&host->lock);

	/*
	 * Maximum number of segments. Depends on if the hardware
	 * can do scatter/gather or not.
	 */
	if (host->flags & SDHCI_USE_ADMA) {
		mmc->max_hw_segs = 128;
	} else if (host->flags & SDHCI_USE_DMA) {
		mmc->max_hw_segs = 1;
		/* mmc->max_hw_segs = 128; */
	} else {		/* PIO */

		mmc->max_hw_segs = 128;
	}
	mmc->max_phys_segs = 128;

	/*
	 * Maximum number of sectors in one transfer. Limited by DMA boundary
	 * size (512KiB).
	 */
	mmc->max_req_size = 524288;

	/*
	 * Maximum segment size. Could be one segment with the maximum number
	 * of bytes. When doing hardware scatter/gather, each entry cannot
	 * be larger than 64 KiB though.
	 */
	if (host->flags & SDHCI_USE_ADMA) {
		/*
		 * For Arasan's host, when doing hardware scatter/gather, each entry cannot
		 * be larger than or equal to 64 KiB.
		 */
		mmc->max_seg_size = 65535;
	} else {
		mmc->max_seg_size = mmc->max_req_size;
	}
	/*
	 * Maximum block size. This varies from controller to controller and
	 * is specified in the capabilities register.
	 */
	mmc->max_blk_size = host->max_block;
	/*
	 * Maximum block count.
	 */
	mmc->max_blk_count = 65535;

	/*
	 * Init tasklets.
	 */
	tasklet_init(&host->card_tasklet, bcmsdhc_tasklet_card,
		     (unsigned long)host);

	tasklet_init(&host->finish_tasklet, bcmsdhc_tasklet_finish,
		     (unsigned long)host);

	setup_timer(&host->timer, bcmsdhc_timeout_timer, (unsigned long)host);

	ret =
	    request_irq(host->irq, bcmsdhc_irq, IRQF_SHARED, mmc_hostname(mmc),
			host);

	if (ret) {
		goto untasklet;
	}

	if (host->irq_cd >= 0) {
		pr_info("Configuring card detect\n");

		if ((bcm_plat->cd_pullup_cfg & SDCD_UPDOWN_ENABLE)
		    == SDCD_UPDOWN_ENABLE)
			bcm_gpio_pull_up_down_enable(host->irq_cd, true);
		else
			bcm_gpio_pull_up_down_enable(host->irq_cd, false);

		if ((bcm_plat->cd_pullup_cfg & SDCD_PULLUP) == SDCD_PULLUP)
			bcm_gpio_pull_up(host->irq_cd, true);
		else
			bcm_gpio_pull_up(host->irq_cd, false);
		bcm_gpio_set_db_val(host->irq_cd, 0xf);

		gpio_request(host->irq_cd, "mmc card detect");
		gpio_direction_input(host->irq_cd);
		ret = request_irq(GPIO_TO_IRQ(host->irq_cd), bcmsdhc_cd,
				  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
				  mmc_hostname(mmc), host);
		if (ret) {
			goto release_irq;
			pr_err("can't get card detect irq.\n");
		}

		if (gpio_get_value(host->irq_cd) != (host->bcm_plat->flags
					& SDHC_CARD_DETECT_ACTIVE_HIGH)) {
			host->card_present = false;
		}
	} else {
		if (!(host->bcm_plat->flags & SDHC_CARD_ALWAYS_PRESENT))
			host->card_present = false;
	}

	sd_inserted = host->card_present;
	
	bcmsdhc_init(host, SOFT_RESET_ALL);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&host->sdhc_wakelock, WAKE_LOCK_SUSPEND,
		       dev_name(&pdev->dev));
#endif

#ifdef CONFIG_MMC_DEBUG
/*      bcmsdhc_dumpregs(host); */
#endif

#ifdef CONFIG_LEDS_CLASS
	host->led.name = mmc_hostname(mmc);
	host->led.brightness = LED_OFF;
	host->led.default_trigger = mmc_hostname(mmc);
	host->led.brightness_set = bcmsdhc_led_control;

	ret = led_classdev_register(mmc_dev(mmc), &host->led);
	if (ret)
		goto reset;
#endif

	mmiowb();

	mmc_add_host(mmc);
	pr_info("SDHCI controller on DASH\n");
	pr_info("%s: SDHCI controller on [%s] using %s%s\n", mmc_hostname(mmc),
		/*      host->hw_name, */
		dev_name(mmc_dev(mmc)),
		(host->flags & SDHCI_USE_ADMA) ? "A" : "",
		(host->flags & SDHCI_USE_DMA) ? "DMA" : "PIO");
	clk_disable(host->bus_clk);

	if (host->bcm_plat->syscfg_interface(SYSCFG_SDHC1 +
					     host->sdhc_slot - 1,
					     SYSCFG_ENABLE))
		goto release_irq;

	if (host->bcm_plat->flags & SDHC_DISABLE_PED_MODE)
		sdio_add_qos_req(host);
	return 0;

#ifdef CONFIG_LEDS_CLASS
reset:
	bcmsdhc_reset(host, SOFT_RESET_ALL);
	free_irq(GPIO_TO_IRQ(host->irq_cd), host);
#endif
release_irq:
	free_irq(host->irq, host);
untasklet:
	tasklet_kill(&host->card_tasklet);
	tasklet_kill(&host->finish_tasklet);
bus_clk_disable:
	clk_disable(host->bus_clk);
bus_clk_put:
/*      clk_put(host->bus_clk); */
host_free:
	mmc_free_host(mmc);
	return ret;

}

/* *************************************************************************************************** */
/* Function Name: bcmsdhc_remove */
/* Description: remove SD host */
/* *************************************************************************************************** */

static int bcmsdhc_remove(struct platform_device *pdev)
{
	struct bcmsdhc_host *host = platform_get_drvdata(pdev);

	pr_debug("bcmsdhc_remove\n");
	if (!host)
		return -ENXIO;

	mmc_remove_host(host->mmc);

#ifdef CONFIG_LEDS_CLASS
	led_classdev_unregister(&host->led);
#endif

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&host->sdhc_wakelock);
#endif

	sdio_remove_qos_req(host);

	bcmsdhc_reset(host, SOFT_RESET_ALL);

	free_irq(GPIO_TO_IRQ(host->irq_cd), host);

	free_irq(host->irq, host);

	del_timer_sync(&host->timer);

	tasklet_kill(&host->card_tasklet);
	tasklet_kill(&host->finish_tasklet);

	kfree(host->adma_desc);
	kfree(host->align_buffer);

	host->adma_desc = NULL;
	host->align_buffer = NULL;

	/* release bus clock */
	/*if (host->bus_clk && !IS_ERR(host->bus_clk))
	 { */
	/*   clk_put(host->bus_clk); */
	/* }
	 */
	mmc_free_host(host->mmc);

	return 0;
}

/* *************************************************************************************************** */
/* Function Name: sdio_host_force_scan */
/* Description: This function is for Initiate the scan */
/* *************************************************************************************************** */
void bcmsdhc_sdio_host_force_scan(struct platform_device *pdev, bool on)
{

	struct bcmsdhc_host *data = platform_get_drvdata(pdev);

	data->card_present = on;

	mmc_detect_change(data->mmc, 0);
	if (data->card_present == false) {
		pr_info(": Card present == FALSE.\n");

	} else {
		pr_info(": Card present == TRUE.\n");
	}

}

#ifdef CONFIG_PM
static int bcmsdhc_suspend(struct platform_device *pdev, pm_message_t mesg)
{

	struct bcmsdhc_host *host = platform_get_drvdata(pdev);
	int ret = 0;

	pr_debug(" bcmsdhc_suspend\n");

	if (host) {
		if (host->mmc->card && host->mmc->card->type != MMC_TYPE_SDIO) {
			ret = mmc_suspend_host(host->mmc);
		}
		if (host->bcm_plat->syscfg_interface(SYSCFG_SDHC1 +
						     host->sdhc_slot - 1,
						     SYSCFG_DISABLE))
			return -EINVAL;

		free_irq(host->irq, host);
	}
	return ret;

}

static int bcmsdhc_resume(struct platform_device *pdev)
{

	struct bcmsdhc_host *host = platform_get_drvdata(pdev);
	int ret = 0;
	pr_debug(" bcmsdhc_resume\n");

	if (host) {
		ret =
		    request_irq(host->irq, bcmsdhc_irq, IRQF_SHARED,
				mmc_hostname(host->mmc), host);
		if (ret)
			return ret;

		if (host->bcm_plat->syscfg_interface(SYSCFG_SDHC1 +
						     host->sdhc_slot - 1,
						     SYSCFG_ENABLE))
			return -EINVAL;

		if((host->mmc->card &&
			host->mmc->card->type != MMC_TYPE_SDIO)) {
			bcmsdhc_init(host, (host->mmc->pm_flags & MMC_PM_KEEP_POWER) ?
					(SOFT_RESET_CMD | SOFT_RESET_DAT) : SOFT_RESET_ALL);
			mmiowb();
			ret = mmc_resume_host(host->mmc);
			if (ret)
				return ret;
		}
	}
	return ret;
}


int sdio_add_qos_req(struct bcmsdhc_host *host)
{
	host->sdio_driver_pm_qos_req =
		pm_qos_add_request(PM_QOS_CPU_DMA_LATENCY, PM_QOS_DEFAULT_VALUE);
	pr_debug("%s: ADD Qos\n",__FUNCTION__);

	return 0;
}

int sdio_remove_qos_req(struct bcmsdhc_host *host)
{
	if (host->sdio_driver_pm_qos_req) {
		pm_qos_remove_request(host->sdio_driver_pm_qos_req);
		pr_debug("%s: DELETE QOS\n",	__FUNCTION__);
		host->sdio_driver_pm_qos_req = NULL;
	}

	return 0;
}

int sdhc_update_qos_req(struct bcmsdhc_host *host, s32 value)
{
	if(host->sdio_driver_pm_qos_req) {
		pm_qos_update_request(host->sdio_driver_pm_qos_req, value);
		pr_debug("%s: UPDATE QOS %x\n",	__FUNCTION__, value);
	}

	return 0;
}

#else
#define bcmsdhc_suspend  	NULL
#define bcmsdhc_resume		NULL
#endif

static struct platform_driver bcm_sdhc_driver = {
	.probe = bcmsdhc_probe,
	.remove = bcmsdhc_remove,
	.suspend = bcmsdhc_suspend,
	.resume = bcmsdhc_resume,
	.driver = {
		   .name = DRIVER_NAME,
		   .owner = THIS_MODULE,
		   },
};

static int __init bcm_sdhc_init(void)
{
	return platform_driver_register(&bcm_sdhc_driver);
}

static void __exit bcm_sdhc_exit(void)
{
	platform_driver_unregister(&bcm_sdhc_driver);
}

module_init(bcm_sdhc_init);
module_exit(bcm_sdhc_exit);

MODULE_AUTHOR("Pierre Ossman <drzeus@drzeus.cx>");
MODULE_DESCRIPTION("Secure Digital Host Controller Interface core driver");
MODULE_LICENSE("GPL");
