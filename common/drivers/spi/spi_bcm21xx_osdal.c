/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/spi/spi_bcm21xx_osdal.c
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
 * Broadcom bcm21xx SPI master controller
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/dma.h>

#include <plat/bcm_spi.h>
#include <plat/dma.h>
#include <linux/clk.h>

#include <plat/mobcom_types.h>
#include <plat/dma_drv.h>


/* Driver data private */
static struct driver_data *drv_data_priv = NULL;

static Dma_Chan_Info dma_info_tx, dma_info_rx;
static Dma_Buffer_List dma_buf_list_tx, dma_buf_list_rx;
static Dma_Data dma_data_tx, dma_data_rx;

static void bcm21xx_spi_dumpregs(struct driver_data *drv_data, char *prompt)
{
	int i;

	pr_info("%s:\n", prompt);
	for (i = 0; i < 0x24; i = i + 8) {
		if (i != SPI_SSPDR) {
			pr_info("SPI[0x%x] = %x, SPI[0x%x] = %x\n",
				i, readw(drv_data->ioaddr + i),
				i + 4, readw(drv_data->ioaddr + (i + 4)));
		} else {
			pr_info("SPI[0x%x] = %x\n",
				i + 4, readw(drv_data->ioaddr + (i + 4)));
		}
	}
}

/* caller already set message->status; dma and pio irqs are blocked */
static void giveback(struct driver_data *drv_data)
{
	struct spi_transfer *last_transfer;
	unsigned long flags;
	struct spi_message *msg;

	spin_lock_irqsave(&drv_data->lock, flags);

	msg = drv_data->cur_msg;
	drv_data->cur_msg = NULL;
	drv_data->cur_transfer = NULL;

	if (drv_data->workqueue)
		queue_work(drv_data->workqueue, &drv_data->pump_messages);
	spin_unlock_irqrestore(&drv_data->lock, flags);

	last_transfer = list_entry(msg->transfers.prev,
				   struct spi_transfer, transfer_list);

	drv_data->cs_control(drv_data, DISABLE_CS);

	msg->state = NULL;
	if (msg->complete)
		msg->complete(msg->context);
}

static void *next_transfer(struct driver_data *drv_data)
{
	struct spi_message *msg = drv_data->cur_msg;
	struct spi_transfer *trans = drv_data->cur_transfer;

	/* Move to next transfer */
	if (trans->transfer_list.next != &msg->transfers) {
		drv_data->cur_transfer =
		    list_entry(trans->transfer_list.next,
			       struct spi_transfer, transfer_list);
		return RUNNING_STATE;
	} else
		return DONE_STATE;
}

static void bcm21xx_setup_transfer(struct driver_data *drv_data)
{
	struct spi_transfer *transfer = NULL;

	transfer = drv_data->cur_transfer;

	drv_data->tx = (void *)transfer->tx_buf;
	drv_data->tx_end = drv_data->tx + transfer->len;
	drv_data->rx = transfer->rx_buf;
	drv_data->rx_end = drv_data->rx + transfer->len;
	drv_data->rx_dma = transfer->rx_dma;
	drv_data->tx_dma = transfer->tx_dma;
	drv_data->len = transfer->len;
	drv_data->cs_change = transfer->cs_change;
}

static int bcm21xx_spi_read(struct driver_data *drv_data)
{
	struct spi_transfer *transfer = NULL;
	struct spi_device *spi = NULL;
	struct spi_message *message = NULL;
	int len = 0;

	transfer = drv_data->cur_transfer;
	message = drv_data->cur_msg;
	spi = message->spi;

	/* If loopback is setup, don't have to write for read */
	if (!(spi->mode & SPI_LOOP)) {
		len = FIFO_DEPTH;
		while ((readw(drv_data->ioaddr + SPI_SSPSR) & SPI_SSPSR_TNF)
		       && --len)
			writew(0xff, drv_data->ioaddr + SPI_SSPDR);
		len = 0;
	}

	while ((readw(drv_data->ioaddr + SPI_SSPSR) & SPI_SSPSR_RNE)
	       && drv_data->len) {
		if (transfer->bits_per_word <= 8)
			(*(u8 *) drv_data->rx) =
			    readw(drv_data->ioaddr + SPI_SSPDR);
		else
			(*(u16 *) drv_data->rx) =
			    readw(drv_data->ioaddr + SPI_SSPDR);

		drv_data->rx += (transfer->bits_per_word >> 3);
		++len;
		drv_data->len--;
	}

	return len;
}

static int bcm21xx_spi_write(struct driver_data *drv_data)
{
	struct spi_transfer *transfer = NULL;
	int len = 0;

	transfer = drv_data->cur_transfer;

	while ((readw(drv_data->ioaddr + SPI_SSPSR) & SPI_SSPSR_TNF)
	       && drv_data->len) {
		if (transfer->bits_per_word <= 8)
			writew(*(u8 *) drv_data->tx,
			       drv_data->ioaddr + SPI_SSPDR);
		else
			writew(*(u16 *) drv_data->tx,
			       drv_data->ioaddr + SPI_SSPDR);

		drv_data->tx += (transfer->bits_per_word >> 3);
		++len;
		--drv_data->len;
	}

	return len;
}

static void spi_dma_isr(DMADRV_CALLBACK_STATUS_t err)
{
	struct driver_data *drv_data = drv_data_priv;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct device *dev;
	u32 status;
	unsigned long limit = loops_per_jiffy << 1;

	message = drv_data->cur_msg;
	dev = &message->spi->dev;
	transfer = drv_data->cur_transfer;

	/* DMA Interrupt can come in before status is updated */
	while (!(status = readw(drv_data->ioaddr + SPI_SSPRIS)) && limit--) {
	}

	if (!status)
		return;

	if (status & SPI_SSPRIS_RORIM) {
		DMADRV_Stop_Transfer(drv_data->tx_channel);
		DMADRV_Stop_Transfer(drv_data->rx_channel);

		writew(SPI_CLEAR_ALL_INTERRUPTS, drv_data->ioaddr + SPI_SSPICR);
		writew(SPI_DISABLE_ALL_INTERRUPTS,
		       drv_data->ioaddr + SPI_SSPIMSC);
		message->state = ERROR_STATE;
	} else {
		if (drv_data->tx) {
			DMADRV_Stop_Transfer(drv_data->tx_channel);
		} else {
			DMADRV_Stop_Transfer(drv_data->rx_channel);
		}

		if (drv_data->cs_change)
			drv_data->cs_control(drv_data, DISABLE_CS);

		writew(SPI_CLEAR_ALL_INTERRUPTS, drv_data->ioaddr + SPI_SSPICR);
		/* End of transfer, update total byte transfered */
		message->actual_length += drv_data->len;

		/* Move to next transfer */
		message->state = next_transfer(drv_data);
	}

	if (drv_data->rx_dma_needs_unmap) {
		dma_unmap_single(dev, drv_data->tx_dma,
				 drv_data->rx_map_len, DMA_FROM_DEVICE);

		drv_data->tx_dma_needs_unmap = 0;
	}

	if (drv_data->tx_dma_needs_unmap) {
		dma_unmap_single(dev, drv_data->tx_dma,
				 drv_data->tx_map_len, DMA_TO_DEVICE);

		drv_data->tx_dma_needs_unmap = 0;
	}

	tasklet_schedule(&drv_data->pump_transfers);

	return;
}

static int bcm21xx_enable_dma(struct driver_data *drv_data)
{
	/* Keep local copy of driver_data for using in
	 * dma callback.
	 */
	drv_data_priv = drv_data;
	return 0;
}

static irqreturn_t bcm21xx_interrupt_handler(int irq, void *arg)
{
	struct driver_data *drv_data = (struct driver_data *)arg;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	u32 status, len;

	message = drv_data->cur_msg;
	transfer = drv_data->cur_transfer;

	status = readw(drv_data->ioaddr + SPI_SSPRIS);

	if (!status)
		return IRQ_NONE;

	if (status & SPI_SSPRIS_RORIM) {

		writew(SPI_CLEAR_ALL_INTERRUPTS, drv_data->ioaddr + SPI_SSPICR);
		writew(SPI_DISABLE_ALL_INTERRUPTS,
		       drv_data->ioaddr + SPI_SSPIMSC);

		message->state = ERROR_STATE;
		tasklet_schedule(&drv_data->pump_transfers);
	} else {
		if (drv_data->tx)
			len = bcm21xx_spi_write(drv_data);
		else
			len = bcm21xx_spi_read(drv_data);

		if (drv_data->cs_change)
			drv_data->cs_control(drv_data, DISABLE_CS);

		writew(SPI_CLEAR_ALL_INTERRUPTS, drv_data->ioaddr + SPI_SSPICR);
		/* End of transfer, update total byte transfered */
		message->actual_length += len;

		if (!drv_data->len) {
			writew(SPI_DISABLE_ALL_INTERRUPTS,
			       drv_data->ioaddr + SPI_SSPIMSC);
			/* Move to next transfer */
			message->state = next_transfer(drv_data);
			tasklet_schedule(&drv_data->pump_transfers);
		}
	}

	return IRQ_HANDLED;
}

static int bcm21xx_config_spi_hw(struct driver_data *drv_data)
{
	char drv_name[32];
	int status;

	sprintf(drv_name, "%s.%d", drv_data->pdev->name, drv_data->pdev->id);
	drv_data->spi_clk = clk_get(NULL, drv_name);

	if (clk_enable(drv_data->spi_clk))
		return -EINVAL;

	/* Disable SPI before configuring */
	writeb(readb(drv_data->ioaddr + SPI_SSPCR1) &
	       ~(SPI_SSPSCR1_SSE), drv_data->ioaddr + SPI_SSPCR1);

	/* Clear all the configration register */
	writew(0, drv_data->ioaddr + SPI_SSPCR0);
	writew(0, drv_data->ioaddr + SPI_SSPCR1);

	if (IS_MODE_SLAVE(drv_data->mode)) {
		writew(readb(drv_data->ioaddr + SPI_SSPCR1) |
		       SPI_SSPSCR1_MS, drv_data->ioaddr + SPI_SSPCR1);
	} else {
		writew(readb(drv_data->ioaddr + SPI_SSPCR1) &
		       ~SPI_SSPSCR1_MS, drv_data->ioaddr + SPI_SSPCR1);
	}

	/* Mask all interrupts */
	writew(0, drv_data->ioaddr + SPI_SSPIMSC);
	writew(SPI_SSPICR_RORIC | SPI_SSPICR_RTIC,
	       drv_data->ioaddr + SPI_SSPICR);

	if (drv_data->enable_dma) {
		if (bcm21xx_enable_dma(drv_data)) {
			pr_err("Error in enabling DMA for SPI\n");
			drv_data->enable_dma = 0;
		}
	}

	/* Enable SPI now */
	writeb(SPI_SSPSCR1_SSE, drv_data->ioaddr + SPI_SSPCR1);

	status = request_irq(drv_data->irq, bcm21xx_interrupt_handler,
			     IRQF_SHARED, "spi_irq", drv_data);
	if (status) {
		pr_err("%s:Error registering spi irq %d %d\n",
		       __func__, status, drv_data->irq);
		return status;
	}

	return 0;
}

static int bcm21xx_set_clock(struct spi_device *spi)
{
	u32 clkdiv, scr, ref_clk;
	struct driver_data *drv_data = spi_master_get_devdata(spi->master);

	if (!drv_data)
		return -EINVAL;

	ref_clk = clk_get_rate(drv_data->spi_clk);

	for (scr = 0; scr <= 255; ++scr) {
		for (clkdiv = 2; clkdiv <= 254; ++clkdiv) {
			/* We program a clock value which is equal to or
			 * slightly less than the requested value */
			if ((spi->max_speed_hz >=
			     (ref_clk / (clkdiv * (1 + scr))))) {
				writew(SPI_SSPCR0_SCR(scr),
				       drv_data->ioaddr + SPI_SSPCR0);
				writew(SPI_SSPCPSR_CPSDVSR(clkdiv),
				       drv_data->ioaddr + SPI_SSPCPSR);
				return 0;
			}
		}
	}

	return -EINVAL;
}

static int bcm21xx_set_mode(struct spi_device *spi)
{
	u16 sspcr0 = 0;
	struct driver_data *drv_data = spi_master_get_devdata(spi->master);

	if (!drv_data)
		return -EINVAL;

	/* FIXME: Datasheet says SPI ctrl only supports MOT, need to confirm */
	sspcr0 =
	    SPI_SSPCR0_FRF_MOT | ((spi->mode & SPI_CPHA) ? SPI_SSPCR0_SPH(1) :
				  0)
	    | ((spi->mode & SPI_CPOL) ? SPI_SSPCR0_SPO(1) : 0)
	    | SPI_SSPCR0_DSS(spi->bits_per_word - 1);

	writew(readw(drv_data->ioaddr + SPI_SSPCR0) | sspcr0,
	       drv_data->ioaddr + SPI_SSPCR0);

	if (spi->mode & SPI_LOOP) {
		writew(readw(drv_data->ioaddr + SPI_SSPCR1) | SPI_SSPSCR1_LBM,
		       drv_data->ioaddr + SPI_SSPCR1);
	}

	return 0;
}

static int bcm21xx_spi_readpoll(struct driver_data *drv_data)
{
	struct spi_transfer *transfer = NULL;

	transfer = drv_data->cur_transfer;

	do {
		bcm21xx_spi_read(drv_data);
	} while (drv_data->len && !((readw(drv_data->ioaddr + SPI_SSPSR)
				     & SPI_SSPSR_RNE)));

	while ((readw(drv_data->ioaddr + SPI_SSPSR) & SPI_SSPSR_BSY)) {
	}

	return 0;
}

static int bcm21xx_spi_writepoll(struct driver_data *drv_data)
{
	struct spi_transfer *transfer = NULL;

	transfer = drv_data->cur_transfer;

	do {
		bcm21xx_spi_write(drv_data);
	} while (drv_data->len && !(readw(drv_data->ioaddr + SPI_SSPSR)
				    & SPI_SSPSR_TNF));

	while ((readw(drv_data->ioaddr + SPI_SSPSR) & SPI_SSPSR_BSY)) {
	}

	return 0;
}

static int bcm21xx_setup_dma(struct driver_data *drv_data)
{
	struct spi_message *message = NULL;
	struct spi_device *spi = NULL;
	struct device *dev;

	/* Get current state information */
	message = drv_data->cur_msg;
	spi = message->spi;
	dev = &message->spi->dev;

	pr_info("%s:Enter\n", __func__);

	/* Setup RX DMA */
	if (drv_data->rx_dma) {
		/* For rx_dma, we need to map the dummy tx buffer, except
		 * in loopback Mode, where we dont use dummy tx buffer.
		 */
		if (!(spi->mode & SPI_LOOP)) {
			drv_data->tx_map_len = sizeof(drv_data->dummy_dma_buf);
			drv_data->tx_dma = dma_map_single(dev,
							  &drv_data->
							  dummy_dma_buf,
							  drv_data->tx_map_len,
							  DMA_TO_DEVICE);

			if (dma_mapping_error(dev, drv_data->tx_dma))
				return -1;

			drv_data->tx_dma_needs_unmap = 1;
			drv_data->tx = NULL;
		}

		/* Map the RX DMA buffer if not mapped by the caller */
		if (message->is_dma_mapped == 0) {
			drv_data->rx_map_len = drv_data->len;
			drv_data->rx_dma = dma_map_single(dev,
							  &drv_data->rx_dma,
							  drv_data->rx_map_len,
							  DMA_FROM_DEVICE);

			if (dma_mapping_error(dev, drv_data->rx_dma))
				return -1;

			drv_data->rx_dma_needs_unmap = 1;
		}
	} else {		/* Setup TX DMA */

		/* Map the TX DMA buffer if not mapped by the caller */
		if (message->is_dma_mapped == 0) {
			drv_data->tx_map_len = drv_data->len;
			drv_data->tx_dma = dma_map_single(dev,
							  &drv_data->tx_dma,
							  drv_data->tx_map_len,
							  DMA_TO_DEVICE);

			if (dma_mapping_error(dev, drv_data->tx_dma))
				return -1;

			drv_data->tx_dma_needs_unmap = 1;
		}
	}
	return 0;
}

static void bcm21xx_start_dmatx(struct driver_data *drv_data)
{
	struct spi_transfer *transfer = NULL;
	struct spi_message *message = NULL;
	struct spi_device *spi = NULL;
	struct device *dev;

	transfer = drv_data->cur_transfer;
	message = drv_data->cur_msg;
	spi = message->spi;
	dev = &message->spi->dev;

	pr_info("%s:Enter\n", __func__);

	/*Request DMA channel for SPI_TX */
	if (DMADRV_Obtain_Channel(DMA_CLIENT_MEMORY, DMA_CLIENT_SPI_TX,
				  (DMA_CHANNEL *) & drv_data->tx_channel) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Obtain_Channel failed for I2S TX channel\n");
		return;
	}

	dma_info_tx.srcID = DMA_CLIENT_MEMORY;
	dma_info_tx.dstID = DMA_CLIENT_SPI_TX;
	dma_info_tx.type = DMA_FCTRL_MEM_TO_PERI;
	dma_info_tx.alignment = DMA_ALIGNMENT_32;
	dma_info_tx.srcBstSize = DMA_BURST_SIZE_4;
	dma_info_tx.dstBstSize = DMA_BURST_SIZE_4;
	if (transfer->bits_per_word <= 8) {
		dma_info_tx.srcDataWidth = DMA_DATA_SIZE_8BIT;
		dma_info_tx.dstDataWidth = DMA_DATA_SIZE_8BIT;
	} else {
		dma_info_tx.srcDataWidth = DMA_DATA_SIZE_16BIT;
		dma_info_tx.dstDataWidth = DMA_DATA_SIZE_16BIT;
	}
	dma_info_tx.incMode = DMA_INC_MODE_SRC;
	dma_info_tx.xferCompleteCb = (DmaDrv_Callback) spi_dma_isr;
	dma_info_tx.freeChan = TRUE;
	dma_info_tx.priority = 0;
	dma_info_tx.bCircular = FALSE;

	if (DMADRV_Config_Channel(drv_data->tx_channel, &dma_info_tx)
	    != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Config_Channel Failed for I2S TX channel\n");
		goto tx_error;
	}

	dma_buf_list_tx.buffers[0].srcAddr = drv_data->tx_dma;
	dma_buf_list_tx.buffers[0].length = drv_data->len;
	dma_buf_list_tx.buffers[0].destAddr =
	    (UInt32) io_v2p((UInt32) drv_data->ioaddr + SPI_SSPDR);
	dma_buf_list_tx.buffers[0].bRepeat = 0;
	dma_buf_list_tx.buffers[0].interrupt = 1;
	dma_data_tx.numBuffer = 1;
	dma_data_tx.pBufList = (Dma_Buffer_List *) & dma_buf_list_tx;

	if (DMADRV_Bind_Data(drv_data->tx_channel, &dma_data_tx)
	    != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Bind_Data Failed for I2S TX channel\n");
		goto tx_error;
	}

	if (DMADRV_Start_Transfer(drv_data->tx_channel) != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Start_Transfer Failed for I2S TX channel\n");
		goto tx_error;
	}

	writew(SPI_SSPDMACR_TXEN, drv_data->ioaddr + SPI_SSPDMACR);
	return;

      tx_error:
	DMADRV_Release_Channel(drv_data->tx_channel);
}

static void bcm21xx_start_dmarx(struct driver_data *drv_data)
{
	struct spi_transfer *transfer = NULL;
	struct spi_message *message = NULL;
	struct spi_device *spi = NULL;
	struct device *dev;

	pr_info("%s:Enter\n", __func__);
	transfer = drv_data->cur_transfer;
	message = drv_data->cur_msg;
	spi = message->spi;
	dev = &message->spi->dev;

	/*Request DMA channel for SPI_TX */
	if (DMADRV_Obtain_Channel(DMA_CLIENT_SPI_RX, DMA_CLIENT_MEMORY,
				  (DMA_CHANNEL *) & drv_data->rx_channel) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Obtain_Channel failed for I2S RX channel\n");
		return;
	}

	dma_info_rx.srcID = DMA_CLIENT_SPI_RX;
	dma_info_rx.dstID = DMA_CLIENT_MEMORY;
	dma_info_rx.type = DMA_FCTRL_PERI_TO_MEM;
	dma_info_rx.alignment = DMA_ALIGNMENT_32;
	dma_info_rx.srcBstSize = DMA_BURST_SIZE_4;
	dma_info_rx.dstBstSize = DMA_BURST_SIZE_4;
	if (transfer->bits_per_word <= 8) {
		dma_info_rx.srcDataWidth = DMA_DATA_SIZE_8BIT;
		dma_info_rx.dstDataWidth = DMA_DATA_SIZE_8BIT;
	} else {
		dma_info_rx.srcDataWidth = DMA_DATA_SIZE_16BIT;
		dma_info_rx.dstDataWidth = DMA_DATA_SIZE_16BIT;
	}
	dma_info_rx.incMode = DMA_INC_MODE_DST;
	dma_info_rx.xferCompleteCb = (DmaDrv_Callback) spi_dma_isr;
	dma_info_rx.freeChan = TRUE;
	dma_info_rx.priority = 0;
	dma_info_rx.bCircular = FALSE;

	if (DMADRV_Config_Channel(drv_data->rx_channel, &dma_info_rx)
	    != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Config_Channel Failed for I2S TX channel\n");
		goto rx_error;
	}

	dma_buf_list_rx.buffers[0].srcAddr =
	    (UInt32) io_v2p((UInt32) drv_data->ioaddr + SPI_SSPDR);
	dma_buf_list_rx.buffers[0].length = drv_data->len;
	dma_buf_list_rx.buffers[0].destAddr = drv_data->rx_dma;
	dma_buf_list_rx.buffers[0].bRepeat = 0;
	dma_buf_list_rx.buffers[0].interrupt = 1;
	dma_data_rx.numBuffer = 1;
	dma_data_rx.pBufList = (Dma_Buffer_List *) & dma_buf_list_rx;

	if (DMADRV_Bind_Data(drv_data->rx_channel, &dma_data_rx)
	    != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Bind_Data Failed for I2S TX channel\n");
		goto rx_error;
	}

	if (DMADRV_Start_Transfer(drv_data->rx_channel) != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Start_Transfer Failed for I2S TX channel\n");
		goto rx_error;
	}

	if (!(spi->mode & SPI_LOOP)) {
		writew(SPI_SSPDMACR_TXEN | SPI_SSPDMACR_RXEN,
		       drv_data->ioaddr + SPI_SSPDMACR);
	} else {
		writew(SPI_SSPDMACR_RXEN, drv_data->ioaddr + SPI_SSPDMACR);
	}
	return;

      rx_error:
	DMADRV_Release_Channel(drv_data->rx_channel);
}

static int bcm21xx_start_dma(struct driver_data *drv_data)
{
	struct spi_message *message = NULL;
	struct spi_device *spi = NULL;

	pr_info("%s:Enter\n", __func__);
	message = drv_data->cur_msg;
	spi = message->spi;

	bcm21xx_setup_transfer(drv_data);
	/* Disable SPI DMA  */
	writew(SPI_DISABLE_ALL_DMA, drv_data->ioaddr + SPI_SSPDMACR);

	/* Start Tx DMA if its a transmit or in case of receive
	 * if loopback is disabled */
	if (drv_data->tx || !(spi->mode & SPI_LOOP))
		bcm21xx_start_dmatx(drv_data);

	if (!drv_data->tx)
		bcm21xx_start_dmarx(drv_data);

	return 0;
}

static int bcm21xx_start_polling(struct driver_data *drv_data)
{
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;

	pr_info("%s:Enter\n", __func__);
	/* Get current state information */
	message = drv_data->cur_msg;

	do {
		bcm21xx_setup_transfer(drv_data);
		transfer = drv_data->cur_transfer;

		if (drv_data->tx)
			bcm21xx_spi_writepoll(drv_data);
		else
			bcm21xx_spi_readpoll(drv_data);

		message->actual_length += transfer->len;
	} while ((drv_data->cur_msg->state =
		  next_transfer(drv_data)) != DONE_STATE);

	message->state = RUNNING_STATE;
	message->status = 0;

	giveback(drv_data);

	return message->status;
}

static int bcm21xx_start_interrupt(struct driver_data *drv_data)
{
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;

	bcm21xx_setup_transfer(drv_data);
	/* Get current state information */
	message = drv_data->cur_msg;
	transfer = drv_data->cur_transfer;
	writew(SPI_ENABLE_ALL_INTERRUPTS, drv_data->ioaddr + SPI_SSPIMSC);

	return 0;
}

static void bcm21xx_pump_transfers(unsigned long data)
{
	struct driver_data *drv_data = (struct driver_data *)data;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;

	pr_info("%s:Enter\n", __func__);
	/* Get current state information */
	message = drv_data->cur_msg;
	transfer = drv_data->cur_transfer;

	/* Handle for abort */
	if (message->state == ERROR_STATE) {
		message->status = -EIO;

		/*SPIPADPULL settings: Set pulldown for all SPI lines */
		writel(readl(drv_data->ioaddr + SPI_SPIPADPULL) & (0x000000aa),
		       drv_data->ioaddr + SPI_SPIPADPULL);

		/* Disable SPI */
		drv_data->syscfg_inf(SYSCFG_SPI1 + drv_data->slot_id,
				     SYSCFG_DISABLE);
		giveback(drv_data);
		return;
	}

	/* Handle end of message */
	if (message->state == DONE_STATE) {
		message->status = 0;

		/* Disable SPI */
		/*SPIPADPULL settings: Set pulldown for all SPI lines */
		writel(readl(drv_data->ioaddr + SPI_SPIPADPULL) & (0x000000aa),
		       drv_data->ioaddr + SPI_SPIPADPULL);

		drv_data->syscfg_inf(SYSCFG_SPI1 + drv_data->slot_id,
				     SYSCFG_DISABLE);
		giveback(drv_data);
		return;
	}

	/* Enable SPI */
	/* SPIPADPULL settings: Set pullup for all SPI lines */
	writel(readl(drv_data->ioaddr + SPI_SPIPADPULL) | (0x000000ff),
	       drv_data->ioaddr + SPI_SPIPADPULL);

	if (drv_data->
	    syscfg_inf(SYSCFG_SPI1 + drv_data->slot_id, SYSCFG_ENABLE)) {
		message->status = -EIO;
		return;
	}
	drv_data->cs_control(drv_data, ENABLE_CS);

	/* Try and enable DMA */
	if (transfer->len >= FIFO_DEPTH) {
		if (drv_data->enable_dma && !bcm21xx_setup_dma(drv_data)) {
			bcm21xx_start_dma(drv_data);
		} else {
			bcm21xx_start_interrupt(drv_data);
		}
	} else {
		bcm21xx_start_polling(drv_data);
	}
}

static void bcm21xx_pump_messages(struct work_struct *work)
{
	struct driver_data *drv_data =
	    container_of(work, struct driver_data, pump_messages);
	unsigned long flags;

	pr_info("%s:Enter\n", __func__);
	/* Lock queue and check for queue work */
	spin_lock_irqsave(&drv_data->lock, flags);
	if (list_empty(&drv_data->queue) || drv_data->run == QUEUE_STOPPED) {
		drv_data->busy = 0;
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return;
	}

	/* Make sure we are not already running a message */
	if (drv_data->cur_msg) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return;
	}

	/* Extract head of queue */
	drv_data->cur_msg = list_entry(drv_data->queue.next,
				       struct spi_message, queue);
	list_del_init(&drv_data->cur_msg->queue);

	/* Initial message state */
	drv_data->cur_msg->state = START_STATE;

	drv_data->cur_transfer = list_entry(drv_data->cur_msg->transfers.next,
					    struct spi_transfer, transfer_list);

	/* Mark as busy and launch transfers */
	tasklet_schedule(&drv_data->pump_transfers);

	drv_data->busy = 1;
	spin_unlock_irqrestore(&drv_data->lock, flags);
}

static int transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct driver_data *drv_data = spi_master_get_devdata(spi->master);
	unsigned long flags;

	spin_lock_irqsave(&drv_data->lock, flags);

	pr_info("%s:Enter\n", __func__);
	if (drv_data->run == QUEUE_STOPPED) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return -ESHUTDOWN;
	}

	msg->actual_length = 0;
	msg->status = -EINPROGRESS;
	msg->state = START_STATE;

	list_add_tail(&msg->queue, &drv_data->queue);

	if (drv_data->run == QUEUE_RUNNING && !drv_data->busy)
		queue_work(drv_data->workqueue, &drv_data->pump_messages);

	spin_unlock_irqrestore(&drv_data->lock, flags);

	return 0;
}

static int setup(struct spi_device *spi)
{
	struct driver_data *drv_data = spi_master_get_devdata(spi->master);

	if (!spi->bits_per_word)
		spi->bits_per_word = 8;

	pr_info("%s:Enter\n", __func__);
	bcm21xx_set_clock(spi);
	bcm21xx_set_mode(spi);
	drv_data->cs_control(drv_data, DISABLE_CS);

	return 0;
}

static int init_queue(struct driver_data *drv_data)
{
	INIT_LIST_HEAD(&drv_data->queue);
	spin_lock_init(&drv_data->lock);

	drv_data->run = QUEUE_STOPPED;
	drv_data->busy = 0;

	tasklet_init(&drv_data->pump_transfers,
		     bcm21xx_pump_transfers, (unsigned long)drv_data);

	INIT_WORK(&drv_data->pump_messages, bcm21xx_pump_messages);
	drv_data->workqueue =
	    create_singlethread_workqueue(dev_name
					  (drv_data->master->dev.parent));
	if (drv_data->workqueue == NULL)
		return -EBUSY;

	return 0;
}

static int start_queue(struct driver_data *drv_data)
{
	unsigned long flags;

	spin_lock_irqsave(&drv_data->lock, flags);

	if (drv_data->run == QUEUE_RUNNING || drv_data->busy) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return -EBUSY;
	}

	drv_data->run = QUEUE_RUNNING;
	drv_data->cur_msg = NULL;
	drv_data->cur_transfer = NULL;
	spin_unlock_irqrestore(&drv_data->lock, flags);

	queue_work(drv_data->workqueue, &drv_data->pump_messages);

	return 0;
}

static int stop_queue(struct driver_data *drv_data)
{
	unsigned long flags;
	unsigned limit = 500;
	int status = 0;

	spin_lock_irqsave(&drv_data->lock, flags);

	/* This is a bit lame, but is optimized for the common execution path.
	 * A wait_queue on the drv_data->busy could be used, but then the common
	 * execution path (pump_messages) would be required to call wake_up or
	 * friends on every SPI message. Do this instead */
	drv_data->run = QUEUE_STOPPED;
	while (!list_empty(&drv_data->queue) && drv_data->busy && limit--) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		msleep(10);
		spin_lock_irqsave(&drv_data->lock, flags);
	}

	if (!list_empty(&drv_data->queue) || drv_data->busy)
		status = -EBUSY;

	spin_unlock_irqrestore(&drv_data->lock, flags);

	return status;
}

static int destroy_queue(struct driver_data *drv_data)
{
	int status;

	status = stop_queue(drv_data);
	if (status != 0)
		return status;

	destroy_workqueue(drv_data->workqueue);

	return 0;
}

static int bcm21xx_spi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct bcm21xx_spi_platform_data *platform_info;
	struct resource *res;
	struct spi_master *master;
	struct driver_data *drv_data = 0;
	int status = 0;

	platform_info = dev->platform_data;

	/* Allocate master with space for drv_data and null dma buffer */
	master = spi_alloc_master(dev, sizeof(struct driver_data));
	if (!master) {
		dev_err(dev, "can not alloc spi_master\n");
		return -ENOMEM;
	}

	drv_data = spi_master_get_devdata(master);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("%s:SPI: No resource for memory\n", __func__);
		status = -ENXIO;
		goto out_error_free_master;
	}

	drv_data->ioaddr = (void __iomem *)res->start;

	drv_data->irq = platform_get_irq(pdev, 0);
	if (!drv_data->irq) {
		pr_err("%s:SPI: No resource for IRQ\n", __func__);
		status = -ENXIO;
		goto out_error_free_master;
	}

	pr_info("ioaddr %p %d\n", drv_data->ioaddr, drv_data->irq);
	drv_data->master = master;
	drv_data->pdev = pdev;
	drv_data->enable_dma = platform_info->enable_dma;
	drv_data->cs_control = platform_info->cs_control;
	drv_data->syscfg_inf = platform_info->syscfg_inf;
	drv_data->mode = platform_info->mode;

	master->bus_num = to_platform_device(dev)->id;
	master->num_chipselect = platform_info->cs_line;
	master->setup = setup;
	master->transfer = transfer;

	/*SPIPADPULL settings: Set pulldown for all SPI lines */
	writel(readl(drv_data->ioaddr + SPI_SPIPADPULL) & (0x000000aa),
	       drv_data->ioaddr + SPI_SPIPADPULL);

	if (drv_data->syscfg_inf(SYSCFG_SPI1 + drv_data->slot_id,
				 SYSCFG_DISABLE))
		return -EINVAL;

	status = bcm21xx_config_spi_hw(drv_data);
	if (status) {
		pr_err("Error configuring SPI hardware\n");
		goto out_error_free_master;
	}

	/* Initial and start queue */
	status = init_queue(drv_data);
	if (status != 0) {
		dev_err(dev, "problem initializing queue\n");
		goto out_error_free_master;
	}

	status = start_queue(drv_data);
	if (status != 0) {
		dev_err(dev, "problem starting queue\n");
		goto out_error_free_master;
	}

	/* Register with the SPI framework */
	platform_set_drvdata(pdev, drv_data);

	status = spi_register_master(master);
	if (status != 0) {
		dev_err(dev, "problem registering spi master\n");
		goto out_error_queue_alloc;
	}

	return status;

      out_error_queue_alloc:
	destroy_queue(drv_data);

      out_error_free_master:
	spi_master_put(master);
	return status;
}

static int bcm21xx_spi_remove(struct platform_device *pdev)
{
	struct driver_data *drv_data = platform_get_drvdata(pdev);
	int status = 0;

	if (!drv_data)
		return 0;

	/* Remove the queue */
	status = destroy_queue(drv_data);
	if (status != 0)
		return status;

	spi_unregister_master(drv_data->master);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static void bcm21xx_spi_shutdown(struct platform_device *pdev)
{
	int status = bcm21xx_spi_remove(pdev);

	if (status != 0)
		dev_err(&pdev->dev, "shutdown failed with %d\n", status);

}

#ifdef CONFIG_PM
static int bcm21xx_spi_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int bcm21xx_spi_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define bcm21xx_spi_suspend     NULL
#define bcm21xx_spi_resume      NULL
#endif

static struct platform_driver bcm21xx_driver = {
	.driver = {
		   .name = "bcm_spi",
		   .owner = THIS_MODULE,
		   },
	.probe = bcm21xx_spi_probe,
	.remove = bcm21xx_spi_remove,
	.shutdown = bcm21xx_spi_shutdown,
	.suspend = bcm21xx_spi_suspend,
	.resume = bcm21xx_spi_resume
};

static int __init bcm21xx_spi_init(void)
{
	pr_info("bcm21xx_spi_init\n");

	return platform_driver_probe(&bcm21xx_driver, bcm21xx_spi_probe);
}

module_init(bcm21xx_spi_init);

static void __exit bcm21xx_spi_exit(void)
{
	pr_info("bcm21xx_spi_exit\n");
	platform_driver_unregister(&bcm21xx_driver);
}

module_exit(bcm21xx_spi_exit);

MODULE_AUTHOR("Broadcom Corporation <@broadcom.com>");
MODULE_DESCRIPTION("bcm21xx SPI Controller");
MODULE_LICENSE("GPL");
