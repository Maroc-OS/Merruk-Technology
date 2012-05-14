/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/char/broadcom/i2s_test_osdal.c
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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <plat/bcm_i2sdai.h>
#include <plat/dma.h>
#include <linux/dma-mapping.h>
#include <mach/hardware.h>

#include <plat/mobcom_types.h>
#include <plat/dma_drv.h>

/*#define I2S_TEST_PRINT_BUFFERS		1*/

unsigned int intCount = 0, transfer_size = 4096 * 4;
static struct dma_i2s *dev_tx, *dev_rx;

static Dma_Chan_Info test_dma_info_tx, test_dma_info_rx;
static Dma_Buffer_List test_dma_buf_list_tx, test_dma_buf_list_rx;
static Dma_Data test_dma_data_tx, test_dma_data_rx;

/***************************************************************************
*
* Function Name:       cleanup_rx()
*
* Description:         Free all the allocated channels and memories.
*
* Parameters:          handle dma_i2s device Instance
*
* Return:              returns 0 on success; -EINVAL otherwise
*
****************************************************************************/
void cleanup_rx(struct dma_i2s *dma_i2s_dev)
{

	/*DMADRV_Release_Channel(dma_i2s_dev->rx_channel); */

	if (dma_i2s_dev->dma_buf.virt_ptr != NULL)
		dma_free_coherent(NULL, transfer_size,
				  dma_i2s_dev->dma_buf.virt_ptr,
				  dma_i2s_dev->dma_buf.phys_ptr);
}

/***************************************************************************
*
* Function Name:       cleanup_tx()
*
* Description:         Free all the allocated channels and memories.
*
* Parameters:          handle dma_i2s device Instance
*
* Return:              returns 0 on success; -EINVAL otherwise
*
****************************************************************************/
void cleanup_tx(struct dma_i2s *dma_i2s_dev)
{
	/*DMADRV_Release_Channel(dma_i2s_dev->tx_channel); */

	if (dma_i2s_dev->dma_buf.virt_ptr != NULL)
		dma_free_coherent(NULL, transfer_size,
				  dma_i2s_dev->dma_buf.virt_ptr,
				  dma_i2s_dev->dma_buf.phys_ptr);
}

/*Irq handler for tx side dma*/
static void i2s_dma_handler_tx(DMADRV_CALLBACK_STATUS_t Err)
{
	pr_info("\n--->i2s_dma_handler_tx()\n");
	intCount++;
	return;
}

/*Irq handler for rx side dma*/
static void i2s_dma_handler_rx(DMADRV_CALLBACK_STATUS_t Err)
{
	pr_info("\n-->i2s_dma_handler_rx()\n");
	intCount++;
	return;
}

/**************************************************************************
*
*Function Name:       i2s_setup_dma_rx()
*
*Description:         Configure the DMA transfer for RX side
*
*Parameters:          handle          dma_i2s device Instance
*
*Return:              returns 0 on success; -EINVAL otherwise
*
***************************************************************************/
int i2s_setup_dma_rx(struct dma_i2s *dma_i2s_dev)
{
	dma_i2s_dev->dma_buf.virt_ptr =
	    dma_alloc_coherent(NULL, transfer_size,
			       &dma_i2s_dev->dma_buf.phys_ptr, GFP_KERNEL);
	if (dma_i2s_dev->dma_buf.virt_ptr == NULL) {
		pr_info
		    ("I2S_RX - Failed to allocate memory for I2S RX buffer\n");
		return -ENOMEM;
	}

	/*Request the channel */
	if (DMADRV_Obtain_Channel(DMA_CLIENT_I2S_RX, DMA_CLIENT_MEMORY,
				  (DMA_CHANNEL *) & dma_i2s_dev->rx_channel) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Obtain_Channel failed for I2S RX channel\n");
		goto rx_error1;
	}

	test_dma_info_rx.srcID = DMA_CLIENT_I2S_RX;
	test_dma_info_rx.dstID = DMA_CLIENT_MEMORY;
	test_dma_info_rx.type = DMA_FCTRL_PERI_TO_MEM;
	test_dma_info_rx.alignment = DMA_ALIGNMENT_32;
	test_dma_info_rx.srcBstSize = DMA_BURST_SIZE_16;
	test_dma_info_rx.dstBstSize = DMA_BURST_SIZE_16;
	test_dma_info_rx.srcDataWidth = DMA_DATA_SIZE_16BIT;
	test_dma_info_rx.dstDataWidth = DMA_DATA_SIZE_16BIT;
	test_dma_info_rx.incMode = DMA_INC_MODE_DST;
	test_dma_info_rx.xferCompleteCb = (DmaDrv_Callback) i2s_dma_handler_rx;
	test_dma_info_rx.freeChan = TRUE;
	test_dma_info_rx.priority = 0;
	test_dma_info_rx.bCircular = FALSE;

	if (DMADRV_Config_Channel(dma_i2s_dev->rx_channel, &test_dma_info_rx)
	    != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Config_Channel Failed for I2S RX channel\n");
		goto rx_error2;
	}
	pr_info("\nRX Channel num=%d", dma_i2s_dev->rx_channel);
	return 0;

rx_error2:
	DMADRV_Release_Channel(dma_i2s_dev->rx_channel);
rx_error1:
	if (dma_i2s_dev->dma_buf.virt_ptr != NULL)
		dma_free_coherent(NULL, transfer_size,
				  dma_i2s_dev->dma_buf.virt_ptr,
				  dma_i2s_dev->dma_buf.phys_ptr);
	return -1;
}

/**************************************************************************
*
*Function Name:       i2s_setup_dma_tx()
*
*Description:         Configure the DMA  transfer for TX side
*
*Parameters:          handle          dma_i2s device Instance
*
*Return:              returns 0 on success; -EINVAL otherwise
*
***************************************************************************/
int i2s_setup_dma_tx(struct dma_i2s *dma_i2s_dev)
{
	dma_i2s_dev->dma_buf.virt_ptr =
	    dma_alloc_coherent(NULL, transfer_size,
			       &dma_i2s_dev->dma_buf.phys_ptr, GFP_KERNEL);
	if (dma_i2s_dev->dma_buf.virt_ptr == NULL) {
		pr_info
		    ("I2S_TX - Failed to allocate memory for I2S TX buffer\n");
		return -ENOMEM;
	}

	/*Request the channel */
	if (DMADRV_Obtain_Channel(DMA_CLIENT_MEMORY, DMA_CLIENT_I2S_TX,
				  (DMA_CHANNEL *) & dma_i2s_dev->tx_channel) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Obtain_Channel failed for I2S TX channel\n");
		goto tx_error1;
	}

	test_dma_info_tx.srcID = DMA_CLIENT_MEMORY;
	test_dma_info_tx.dstID = DMA_CLIENT_I2S_TX;
	test_dma_info_tx.type = DMA_FCTRL_MEM_TO_PERI;
	test_dma_info_tx.alignment = DMA_ALIGNMENT_32;
	test_dma_info_tx.srcBstSize = DMA_BURST_SIZE_16;
	test_dma_info_tx.dstBstSize = DMA_BURST_SIZE_16;
	test_dma_info_tx.srcDataWidth = DMA_DATA_SIZE_16BIT;
	test_dma_info_tx.dstDataWidth = DMA_DATA_SIZE_16BIT;
	test_dma_info_tx.incMode = DMA_INC_MODE_SRC;
	test_dma_info_tx.xferCompleteCb = (DmaDrv_Callback) i2s_dma_handler_tx;
	test_dma_info_tx.freeChan = TRUE;
	test_dma_info_tx.priority = 0;
	test_dma_info_tx.bCircular = FALSE;

	if (DMADRV_Config_Channel(dma_i2s_dev->tx_channel, &test_dma_info_tx)
	    != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Config_Channel Failed for I2S TX channel\n");
		goto tx_error2;
	}

	pr_info("\nTX Channel num=%d", dma_i2s_dev->tx_channel);
	return 0;

tx_error2:
	DMADRV_Release_Channel(dma_i2s_dev->tx_channel);
tx_error1:
	if (dma_i2s_dev->dma_buf.virt_ptr != NULL)
		dma_free_coherent(NULL, transfer_size,
				  dma_i2s_dev->dma_buf.virt_ptr,
				  dma_i2s_dev->dma_buf.phys_ptr);
	return -1;
}

/**************************************************************************
*
*Function Name:       i2s_start_dma_rx()
*
*Description:             Start the dma transfer of RX side
*
*Parameters:          handle          dma_i2s  device Instance
*
*
***************************************************************************/
void i2s_start_dma_rx(struct dma_i2s *dma_i2s_dev,
		      struct i2s_device *i2s_tst_dev)
{
	test_dma_buf_list_rx.buffers[0].srcAddr =
	    (int)(BCM21553_I2S_BASE + 0x0008);
	test_dma_buf_list_rx.buffers[0].destAddr =
	    dma_i2s_dev->dma_buf.phys_ptr;
	test_dma_buf_list_rx.buffers[0].length = transfer_size;
	test_dma_buf_list_rx.buffers[0].bRepeat = 0;
	test_dma_buf_list_rx.buffers[0].interrupt = 1;
	test_dma_data_rx.numBuffer = 1;
	test_dma_data_rx.pBufList = (Dma_Buffer_List *) & test_dma_buf_list_rx;

	if (DMADRV_Bind_Data(dma_i2s_dev->rx_channel, &test_dma_data_rx) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Bind_Data Failed for I2S RX channel\n");
		return;
	}

	if (DMADRV_Start_Transfer(dma_i2s_dev->rx_channel) != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Start_Transfer Failed for I2S RX channel\n");
		return;
	}
}

/**************************************************************************
*
*Function Name:       i2s_start_dma_tx()
*
*Description:             Start the dma transfer of TX side
*
*Parameters:          handle          dma_i2s  device Instance
*
*
***************************************************************************/
void i2s_start_dma_tx(struct dma_i2s *dev, struct i2s_device *i2s_tst_dev)
{
	test_dma_buf_list_tx.buffers[0].srcAddr = dev->dma_buf.phys_ptr;
	test_dma_buf_list_tx.buffers[0].destAddr =
	    (int)(BCM21553_I2S_BASE + 0x0008);
	test_dma_buf_list_tx.buffers[0].length = transfer_size;
	test_dma_buf_list_tx.buffers[0].bRepeat = 0;
	test_dma_buf_list_tx.buffers[0].interrupt = 1;
	test_dma_data_tx.numBuffer = 1;
	test_dma_data_tx.pBufList = (Dma_Buffer_List *) & test_dma_buf_list_tx;

	if (DMADRV_Bind_Data(dev->tx_channel, &test_dma_data_tx) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Bind_Data Failed for I2S TX channel\n");
		return;
	}

	if (DMADRV_Start_Transfer(dev->tx_channel) != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Start_Transfer Failed for I2S TX channel\n");
		return;
	}
}

/**************************************************************************
*
*Function Name:       i2s_internal_loopback_test()
*
*Description:         I2S internal loopback mode using DMA transfer.
*
*Return:              returns 0 on success; -1 otherwise
***************************************************************************/
int i2s_internal_loopback_test(void)
{
	int ret = -1, errFlag = 0;
	int i = 0;
	struct i2s_device *i2s_tst_dev = NULL;
	struct clk *i2s_clk_dam;
	intCount = 0;

	/*Enable DAM clk  */
	i2s_clk_dam = clk_get(NULL, "DAM");
	clk_enable(i2s_clk_dam);
	msleep(5);

	/*Get the I2S Device */
	if (i2s_tst_dev == NULL)
		i2s_tst_dev = use_i2s_device();

	if (i2s_tst_dev == NULL) {
		pr_info("I2S Test: I2S device in use, can't get device!!!!\n");
		return -1;
	}

	/*Enable Softreset on the I2S and flush all the registers */
	bcm_i2s_sofreset_dai(i2s_tst_dev);

	bcm_i2s_flush_fifo(i2s_tst_dev, PLAYBACK);
	bcm_i2s_flush_fifo(i2s_tst_dev, CAPTURE);

	/*Enable the RX and TX fifos for both Playback and recording */
	if (bcm_i2s_txrx_fifo_enable(i2s_tst_dev, PLAYBACK) < 0) {
		pr_info("bcm_i2s_txrx_fifo_enable(i2s_tst_dev, 0) failed \n");
		return -1;
	}

	if (bcm_i2s_txrx_fifo_enable(i2s_tst_dev, CAPTURE) < 0) {
		pr_info("bcm_i2s_txrx_fifo_enable(i2s_tst_dev, 1) failed \n");
		return -1;
	}

	/*Enable Stereo/Mono */
	if (bcm_i2s_interface_mode(i2s_tst_dev, 1, 0) < 0) {
		pr_info("bcm_i2s_interface_mode failed \n");
		return -1;
	}

	/*Enable I2S DMA-sample rate */
	if (bcm_i2s_config_dma(i2s_tst_dev, PLAYBACK, SAMPLE_32) < 0) {
		pr_info
		    ("bcm_i2s_config_dma(i2s_tst_dev, 0, SAMPLE_16) failed \n");
		return -1;
	}
	if (bcm_i2s_config_dma(i2s_tst_dev, CAPTURE, SAMPLE_32) < 0) {
		pr_info
		    ("bcm_i2s_config_dma(i2s_tst_dev, 1, SAMPLE_16) failed \n");
		return -1;
	}

	/*Set the sample rate */
	if (bcm_i2s_sample_rate_div
	    (i2s_tst_dev, I2S_SAMPLERATE_48000HZ, PLAYBACK) < 0) {
		pr_info
		    ("bcm_i2s_sample_rate_div(i2s_tst_dev, BCLK_DIV_1500, 0) failed \n");
		return -1;
	}
	if (bcm_i2s_sample_rate_div
	    (i2s_tst_dev, I2S_SAMPLERATE_48000HZ, CAPTURE) < 0) {
		pr_info
		    ("bcm_i2s_sample_rate_div(i2s_tst_dev, BCLK_DIV_1500, 1) failed \n");
		return -1;
	}

	/*Enable the internal loopback mode */
	if (bcm_i2s_loopback_cfg(i2s_tst_dev, true) < 0) {
		pr_info("bcm_i2s_loopback_cfg failed \n");
		return -1;
	}

	/*Allocate memory for TX/RX side buffers */
	dev_tx = kzalloc(sizeof(struct dma_i2s), GFP_KERNEL);
	dev_rx = kzalloc(sizeof(struct dma_i2s), GFP_KERNEL);

	/*Enable both TX/RX for transmiision and receive */
	bcm_i2s_interface_enable(i2s_tst_dev, true, true, 0);

	/*Setup RX side DMA */
	ret = i2s_setup_dma_rx(dev_rx);
	if (ret < 0) {
		pr_info("i2s_setup_dma_rx failed\n");
		return ret;
	}

	/*Setup TX side DMA */
	ret = i2s_setup_dma_tx(dev_tx);
	if (ret < 0) {
		pr_info("i2s_setup_dma_tx failed\n");
		cleanup_rx(dev_rx);
		return ret;
	}

	/*Intailize the I2S_dma buffer */
	for (i = 0; i < transfer_size / 4; i++) {
		dev_tx->dma_buf.virt_ptr[i] = 0x12345000 + (i + 1);
		dev_rx->dma_buf.virt_ptr[i] = 0xDEADBEAF;
	}

#ifdef I2S_TEST_PRINT_BUFFERS
	pr_info("\nTransmit(TX) buffer before DMA=>\n");
	for (i = 0; i < transfer_size / 4; i++)
		printk("0x%x  ", dev_tx->dma_buf.virt_ptr[i]);
#endif

	/*Start the RX side DMA */
	i2s_start_dma_rx(dev_rx, i2s_tst_dev);

	/*Start the TX side DMA */
	i2s_start_dma_tx(dev_tx, i2s_tst_dev);

	/* Enable the I2S clock:
	 * I2S clock is enabled only after starting DMA in case of loopback test.
	 * If enabled before DMA, I2S will start receiving the data on SDI
	 * and fills RX FIFO with stale data or zeros, resulting in mismatch
	 * between DMA TX and RX buffers.
	 *
	 */
	if (bcm_i2s_enable_clk(i2s_tst_dev, I2S_INT_CLK, RATE_12_MHZ) < 0) {
		pr_info("bcm_i2s_enable_clk failed \n");
		return -1;
	}

	/*Wait for DMA interrupts */
	while (intCount < 2) {
		msleep(10);
	}

#ifdef I2S_TEST_PRINT_BUFFERS
	/*Print the destination buffer */
	pr_info("\nReceive(RX) buffer after DMA=>\n");
	for (i = 0; i < (transfer_size / 4); i++)
		printk("0x%x  ", dev_rx->dma_buf.virt_ptr[i]);
#endif

	printk("\nVerifying the integrity of DMA data...\n");
	/* Verify data  on Athena Platform */
	for (i = 6; i < (transfer_size / 4); i++) {
		if (dev_tx->dma_buf.virt_ptr[i] != dev_rx->dma_buf.virt_ptr[i])
			errFlag++;
	}

	/*Cleanup the allocated memories */
	cleanup_tx(dev_tx);
	cleanup_rx(dev_rx);

	/*Release the I2S handle */
	release_i2s_device(i2s_tst_dev);

	/*Print the test result */
	if (errFlag == 0)
		pr_info
		    ("\n --->i2s_internal_loopback_test: DMA TEST PASSED!\n");
	else
		pr_info
		    ("\n --->i2s_internal_loopback_test: DMA TEST FAILED!\n");

	return 0;
}

/*
 * i2s_init_module
 *      set the owner of i2s_fops, register the module
 *      as a char device, and perform any necessary
 *      initialization
 */
static int i2s_init_module(void)
{
	int rc;
	rc = i2s_internal_loopback_test();
	return 0;
}

/*
 * i2s_exit_module
 *      unregister the device and any necessary
 *      operations to close devices
 */
static void i2s_exit_module(void)
{
	pr_info("bcm_i2s module unloaded successfully\n");
}

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("bcm_i2s_test module");
MODULE_LICENSE("GPL");
module_init(i2s_init_module);
module_exit(i2s_exit_module);
