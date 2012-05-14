/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/char/broadcom/i2s_drv_osdal.c
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

/**
*
*   @file   i2s_drv_osdal.c
*   @brief  This file defines the I2S Driver API using New DMA driver
*
*****************************************************************************/

/**
*   @defgroup   I2SDRVOSDALGroup   I2S Driver API using New DMA driver
*   @brief      This group defines the I2S Driver API
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <asm/sizes.h>
#include <mach/hardware.h>
#include <mach/memory.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <plat/bcm_i2sdai.h>
#include <plat/cpu.h>
#include "i2s.h"
#include <plat/dma.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include <plat/mobcom_types.h>
#include <plat/dma_drv.h>

/**
 * I2S Device structure
 */
struct i2s_dev {
	I2sEndCb endCallback;
	bool callback_status;
	I2S_STATE state;
	int sampleRate;
	bool stereo;
	I2S_RX_CHAN_t mono_from;	/* only needed for RX */
	UInt32 dmaChanNum;
	Dma_Chan_Info dmaChanInfo;
	bool get_chan;
	int bufCnt;
	unsigned int src_addr, len, dest_addr;
	int tx_buf_needs_unmap, rx_buf_needs_unmap;
};

/**
 * I2S block is an I2S controller which has a tx device and a rx device
 */
struct i2s_block {
	struct i2s_dev dev[2];
	bool initialized;
	I2S_BLK_ID_t id;
	bool master;
	bool circular;
	bool loopback;
	struct clk *dam_clk;
	/* magic number is the last member of the object */
	unsigned int magic_number;
	struct i2s_device *i2s_handle;
};
static struct i2s_block i2s_blk[NUM_I2S_BLOCK];

static Dma_Buffer_List dma_buf_list_tx, dma_buf_list_rx;
static Dma_Data dma_data_tx, dma_data_rx;
static struct device *pdevice;
struct i2s_block *tx_handle, *rx_handle;

#define TX_INDEX    0
#define RX_INDEX    1
#define I2S_BLOCK_MAGIC_NUMBER   'I2S'

/** @addtogroup I2SDRVOSDALGroup
    @{
*/

/**
* @brief	DMA Interrupt handler for I2S TX DMA channel
*
* @param Err	DMA driver callback status
*/
static void i2s_dma_handler_tx(DMADRV_CALLBACK_STATUS_t Err)
{
	/* Unmap the DMA TX buffer */
	if (tx_handle->dev[TX_INDEX].tx_buf_needs_unmap)
		dma_unmap_single(pdevice, tx_handle->dev[TX_INDEX].src_addr,
				 tx_handle->dev[TX_INDEX].len, DMA_TO_DEVICE);

	tx_handle->dev[TX_INDEX].tx_buf_needs_unmap = 0;

	/* Call End of Transfer callback for the client to synchronize */
	if (tx_handle->dev[TX_INDEX].endCallback)
		tx_handle->dev[TX_INDEX].endCallback(true);

	 tx_handle->dev[TX_INDEX].bufCnt--;
		if (tx_handle->circular != TRUE) {
			if (tx_handle->dev[TX_INDEX].bufCnt == 0) {
				tx_handle->dev[TX_INDEX].state = I2STST_READY;
			}
        }

	return;
}

/**
* @brief 	DMA Interrupt handler for I2S RX DMA channel
*
* @param Err	DMA driver callback status
*/
static void i2s_dma_handler_rx(DMADRV_CALLBACK_STATUS_t Err)
{
	if (rx_handle->dev[RX_INDEX].rx_buf_needs_unmap)
		dma_unmap_single(pdevice, rx_handle->dev[RX_INDEX].dest_addr,
				 rx_handle->dev[RX_INDEX].len, DMA_FROM_DEVICE);

	rx_handle->dev[RX_INDEX].rx_buf_needs_unmap = 0;

	/* Call End of Transfer callback for the client to synchronize */
	if (rx_handle->dev[RX_INDEX].endCallback)
		rx_handle->dev[RX_INDEX].endCallback(true);

		rx_handle->dev[RX_INDEX].bufCnt--;
		if (rx_handle->circular != TRUE) {
			if (rx_handle->dev[RX_INDEX].bufCnt == 0) {
	        		rx_handle->dev[RX_INDEX].state = I2STST_READY;
			}
		}
	return;
}

/**
* @brief  Obtain I2S Handle
*
* @param id     I2S block enum that is to be initialized
*
* @return       I2S handle, NULL if error
*/
I2S_HANDLE I2SDRV_Get_handle(I2S_BLK_ID_t id)
{
	struct i2s_block *block;
	I2S_HANDLE handle;

	pr_debug("I2SDRV_Get_handle: block %d\r\n", id);

	if (id >= NUM_I2S_BLOCK) {
		pr_debug("I2SDRV_Get_handle: bad block id\r\n");
		return NULL;
	}

	block = &i2s_blk[id];
	handle = (I2S_HANDLE)block;

	return handle;
}

/**
* @brief    Initialize the I2S Driver, should be the first function to call
*
* @param id     I2S block enum that is to be initialized
*
* @return       I2S handle, NULL if error
*/
I2S_HANDLE I2SDRV_Init(I2S_BLK_ID_t id)
{
	struct i2s_block *block;

	if (id >= NUM_I2S_BLOCK) {
		pr_info("I2SDRV_Init: bad block id\n");
		goto err_return;
	}

	block = &i2s_blk[id];

	if (block->initialized) {

		/* force READY states */
		pr_debug
		    ("I2SDRV_Init: already init'ed (%lu for TX, %lu for RX)\r\n",
		     block->dev[TX_INDEX].dmaChanNum,
		     block->dev[RX_INDEX].dmaChanNum);
		block->dev[TX_INDEX].state = I2STST_READY;
		block->dev[RX_INDEX].state = I2STST_READY;
		goto err_return;
	}

	block->dam_clk = clk_get(NULL, "damck");
	if (IS_ERR(block->dam_clk)) {
		pr_info("Cannot obtain DAM clock for I2S!\n");
		return (void *)PTR_ERR(block->dam_clk);
	}
	block->master = false;
	block->id = id;
	block->magic_number = (int)"I2S_BLOCK_MAGIC_NUMBER" + id;
	block->initialized = true;
	block->dev[RX_INDEX].bufCnt = 0;
	block->dev[TX_INDEX].bufCnt = 0;

	block->i2s_handle = use_i2s_device();

	pr_debug("I2SDRV_Init: dma channel %lu for TX, and %lu for RX\r\n",
		 block->dev[TX_INDEX].dmaChanNum,
		 block->dev[RX_INDEX].dmaChanNum);
	block->dev[TX_INDEX].state = I2STST_READY;
	block->dev[RX_INDEX].state = I2STST_READY;

	return (I2S_HANDLE)block;

err_return:
	return NULL;
}

/**
* @brief    De-initialze the I2S Driver, should be the last function to call
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Cleanup(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (!handle)
		return 1;
	if (block->dev[TX_INDEX].state != I2STST_READY ||
	    block->dev[RX_INDEX].state != I2STST_READY) {
		pr_debug("I2SDRV_cleanup: error! tx_state=%d, rx_state=%d\r\n",
			 block->dev[TX_INDEX].state,
			 block->dev[RX_INDEX].state);
		return I2S_WRONG_STATE;
	}

	DMADRV_Release_Channel(block->dev[RX_INDEX].dmaChanNum);
	DMADRV_Release_Channel(block->dev[TX_INDEX].dmaChanNum);

	release_i2s_device(handle);
	block->dev[TX_INDEX].state = I2STST_NOT_INIT;
	block->dev[RX_INDEX].state = I2STST_NOT_INIT;

	block->dev[RX_INDEX].get_chan = false;
	block->dev[TX_INDEX].get_chan = false;
	block->initialized = false;

	return 0;
}

/**
* @brief    Configure I2S Master/Slave Mode
*
* @param handle     I2S handle
* @param master     Master/Slave mode
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Set_mode(I2S_HANDLE handle, bool master)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	clk_enable(block->dam_clk);
	bcm_i2s_sofreset_dai(block->i2s_handle);

	bcm_i2s_flush_fifo(block->i2s_handle, CAPTURE);

	if (bcm_i2s_interface_mode(block->i2s_handle, master, 0)) {
		pr_debug
		    ("I2SDRV_set_mode: bcm_i2s_interface_mode failed! \r\n");
	}
	if (bcm_i2s_config_dma(block->i2s_handle, PLAYBACK, TXRXDMASIZE_32))
		pr_debug("I2SDRV_set_mode:bcm_i2s_config_dma \
			for PLAYBACK failed! \r\n");
	if (bcm_i2s_config_dma(block->i2s_handle, CAPTURE, TXRXDMASIZE_32))
		pr_debug("I2SDRV_set_mode: bcm_i2s_config_dma \
			for CAPTURE failed! \r\n");
	bcm_i2s_txrx_fifo_enable(block->i2s_handle, 1);
	clk_disable(block->dam_clk);
	return 0;

}

/**
* @brief    Configure I2S transmit (TX) format, for Playback
*
* @param handle         I2S handle
* @param sampleRate     Sampling rate
* @param stereo         stereo/mono selection
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Set_tx_format(I2S_HANDLE handle, int sampleRate,
				   bool stereo)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	if (block->dev[TX_INDEX].state != I2STST_READY) {
		pr_debug("I2SDRV_Set_tx_format: wrong state tx_state=%d\r\n",
			 block->dev[TX_INDEX].state);
		return I2S_WRONG_STATE;
	}
	if (bcm_i2s_sample_rate_div(block->i2s_handle, sampleRate, PLAYBACK)) {
		pr_debug
		    ("I2SDRV_set_mode:bcm_i2s_sample_rate_div failed! \r\n");
	}
	if (bcm_i2s_interface_mode(block->i2s_handle, 1, stereo)) {
		pr_debug("I2SDRV_set_mode:bcm_i2s_interface_mode failed! \r\n");
	}
	return 0;

}

/**
* @brief    Configure I2S Receive (RX) format, for Capture
*
* @param handle         I2S handle
* @param sampleRate     Sampling rate
* @param stereo         stereo/mono selection
* @param mono_from      where mono data is from. only used for mono mode
*
* @return               0 for success, i2s error status on error
*/
unsigned char I2SDRV_Set_rx_format(I2S_HANDLE handle, int sampleRate,
				   bool stereo, I2S_RX_CHAN_t mono_from)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	clk_enable(block->dam_clk);
	if (block->dev[RX_INDEX].state != I2STST_READY) {
		pr_debug("I2SDRV_Set_rx_format: wrong state! rx_state=%d\r\n",
			 block->dev[RX_INDEX].state);
		return I2S_WRONG_STATE;
	}
	if (bcm_i2s_sample_rate_div(block->i2s_handle, sampleRate, CAPTURE))
		pr_debug("I2SDRV_set_mode:bcm_i2s_sample_rate_div \
			failed!\r\n");
	if (bcm_i2s_interface_mode(block->i2s_handle, 0, stereo))
		pr_debug("I2SDRV_set_mode:bcm_i2s_interface_mode failed!\r\n");

 	if (!stereo) {
	if (bcm_i2s_setup_mono_record(block->i2s_handle, mono_from, stereo, 0))
		pr_debug("I2SDRV_set_mode:bcm_i2s_setup_mono_record \
			failed! \r\n");
 	}
	clk_disable(block->dam_clk);
	return 0;

}

/**
* @brief     Queue a DMA buffer for Transmit (TX)
*
* @param handle     I2S handle
* @param buf        DMA buffer
* @param len        Length of buffer in 16-bit I2S words
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Queue_tx_buf(I2S_HANDLE handle, unsigned short *buf,
				  unsigned int len)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	pr_debug("I2SDRV_queue_tx_buf, bufCnt=%d, state=%d\r\n",
		 block->dev[TX_INDEX].bufCnt, block->dev[TX_INDEX].state);
	if (block->dev[TX_INDEX].state == I2STST_NOT_INIT)
		return I2S_WRONG_STATE;

	block->dev[TX_INDEX].src_addr = dma_map_single(pdevice,
						       buf, len, DMA_TO_DEVICE);
	if (dma_mapping_error(pdevice, block->dev[TX_INDEX].src_addr)) {
		pr_info("%s(): Failed to map DMA buffer\n", __func__);
		return -1;
	}

	/* Make the memory consistent, invalidate/writeback the cache */
	__dma_single_cpu_to_dev(buf, len, DMA_TO_DEVICE);
	block->dev[TX_INDEX].tx_buf_needs_unmap = 1;
	tx_handle = block;

	block->dev[TX_INDEX].len = len;
	block->dev[TX_INDEX].bufCnt++;

	return 0;
}

/**
* @brief     Queue a DMA buffer for Receive (Rx)
*
* @param handle     I2S handle
* @param buf        DMA buffer
* @param len        Length of buffer in 16-bit I2S words
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Queue_rx_buf(I2S_HANDLE handle, unsigned short *buf,
				  unsigned int len)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}

	pr_debug("I2SDRV_queue_rx_buf, bufCnt=%d, state=%d\r\n",
		 block->dev[RX_INDEX].bufCnt, block->dev[RX_INDEX].state);

	if (block->dev[RX_INDEX].state == I2STST_NOT_INIT)
		return I2S_WRONG_STATE;
	block->dev[RX_INDEX].dest_addr = dma_map_single(pdevice,
							buf, len,
							DMA_FROM_DEVICE);
	if (dma_mapping_error(pdevice, block->dev[RX_INDEX].dest_addr)) {
		pr_info("%s(): Failed to map DMA buffer\n", __func__);
		return -1;
	}

	/* Make the memory consistent, invalidate/writeback the cache */
	__dma_single_cpu_to_dev(buf, len, DMA_FROM_DEVICE);
	block->dev[RX_INDEX].rx_buf_needs_unmap = 1;
	rx_handle = block;

	block->dev[RX_INDEX].len = len;
	block->dev[RX_INDEX].bufCnt++;
	return 0;
}

/**
* @brief    Start I2S Transmit (TX) in DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Start_tx(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;

	/*Request DMA channel for I2S_TX */
	if (DMADRV_Obtain_Channel(DMA_CLIENT_MEMORY, DMA_CLIENT_I2S_TX,
				  (DMA_CHANNEL *) & block->
				  dev[TX_INDEX].dmaChanNum) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Obtain_Channel failed for I2S TX channel\n");
		goto tx_error1;
	}

	clk_enable(block->dam_clk);

	block->dev[TX_INDEX].dmaChanInfo.srcID = DMA_CLIENT_MEMORY;
	block->dev[TX_INDEX].dmaChanInfo.dstID = DMA_CLIENT_I2S_TX;
	block->dev[TX_INDEX].dmaChanInfo.type = DMA_FCTRL_MEM_TO_PERI;
	block->dev[TX_INDEX].dmaChanInfo.alignment = DMA_ALIGNMENT_32;
	block->dev[TX_INDEX].dmaChanInfo.srcBstSize = DMA_BURST_SIZE_16;
	block->dev[TX_INDEX].dmaChanInfo.dstBstSize = DMA_BURST_SIZE_16;
	block->dev[TX_INDEX].dmaChanInfo.srcDataWidth = DMA_DATA_SIZE_16BIT;
	block->dev[TX_INDEX].dmaChanInfo.dstDataWidth = DMA_DATA_SIZE_16BIT;
	block->dev[TX_INDEX].dmaChanInfo.incMode = DMA_INC_MODE_SRC;
	block->dev[TX_INDEX].dmaChanInfo.xferCompleteCb = (DmaDrv_Callback) i2s_dma_handler_tx;
	block->dev[TX_INDEX].dmaChanInfo.freeChan = TRUE;
	block->dev[TX_INDEX].dmaChanInfo.priority = 0;
	block->dev[TX_INDEX].dmaChanInfo.bCircular = FALSE;

	if (DMADRV_Config_Channel(block->dev[TX_INDEX].dmaChanNum, &block->dev[TX_INDEX].dmaChanInfo)
	    != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Config_Channel Failed for I2S TX channel\n");
		goto tx_error2;
	}

	bcm_i2s_enable_clk(block->i2s_handle, I2S_EXT_CLK, 0);

	bcm_i2s_txrx_fifo_enable(block->i2s_handle, 0);

	bcm_i2s_interface_enable(block->i2s_handle, true, false, 0);

	pr_debug("I2SDRV_Start_tx, bufCnt=%d, state=%d\r\n",
		 block->dev[RX_INDEX].bufCnt, block->dev[RX_INDEX].state);

	dma_buf_list_tx.buffers[0].srcAddr = block->dev[TX_INDEX].src_addr;
	dma_buf_list_tx.buffers[0].length = block->dev[TX_INDEX].len;
	dma_buf_list_tx.buffers[0].destAddr =
	    (int)(io_v2p( (int) bcm_i2s_get_base_addr()+ 0x0008));
	dma_buf_list_tx.buffers[0].bRepeat = 0;
	dma_buf_list_tx.buffers[0].interrupt = 1;
	dma_data_tx.numBuffer = 1;
	dma_data_tx.pBufList = (Dma_Buffer_List *) & dma_buf_list_tx;

	if (DMADRV_Bind_Data(block->dev[TX_INDEX].dmaChanNum,
			     &dma_data_tx) != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Bind_Data Failed for I2S TX channel\n");
		goto tx_error2;
	}

	if (DMADRV_Start_Transfer(block->dev[TX_INDEX].dmaChanNum) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Start_Transfer Failed for I2S TX channel\n");
		goto tx_error2;
	}
	block->dev[TX_INDEX].state = I2STST_STREAMING;
	return 0;

tx_error2:
	DMADRV_Release_Channel(block->dev[TX_INDEX].dmaChanNum);
tx_error1:
	return I2S_DMA_SETUP_ERROR;
}

/**
* @brief    Start I2S Receive (RX) in DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Start_rx(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;

	/*Request the  DMA channel for I2S_RX */
	if (DMADRV_Obtain_Channel(DMA_CLIENT_I2S_RX, DMA_CLIENT_MEMORY,
				  (DMA_CHANNEL *) & block->
				  dev[RX_INDEX].dmaChanNum) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Obtain_Channel failed for I2S RX channel\n");
		goto rx_error1;
	}
	clk_enable(block->dam_clk);

	block->dev[RX_INDEX].dmaChanInfo.srcID = DMA_CLIENT_I2S_RX;
	block->dev[RX_INDEX].dmaChanInfo.dstID = DMA_CLIENT_MEMORY;
	block->dev[RX_INDEX].dmaChanInfo.type = DMA_FCTRL_PERI_TO_MEM;
	block->dev[RX_INDEX].dmaChanInfo.alignment = DMA_ALIGNMENT_32;
	block->dev[RX_INDEX].dmaChanInfo.srcBstSize = DMA_BURST_SIZE_16;
	block->dev[RX_INDEX].dmaChanInfo.dstBstSize = DMA_BURST_SIZE_16;
	block->dev[RX_INDEX].dmaChanInfo.srcDataWidth = DMA_DATA_SIZE_16BIT;
	block->dev[RX_INDEX].dmaChanInfo.dstDataWidth = DMA_DATA_SIZE_16BIT;
	block->dev[RX_INDEX].dmaChanInfo.incMode = DMA_INC_MODE_DST;
	block->dev[RX_INDEX].dmaChanInfo.xferCompleteCb = (DmaDrv_Callback) i2s_dma_handler_rx;
	block->dev[RX_INDEX].dmaChanInfo.freeChan = TRUE;
	block->dev[RX_INDEX].dmaChanInfo.priority = 0;
	block->dev[RX_INDEX].dmaChanInfo.bCircular = FALSE;

	if (DMADRV_Config_Channel(block->dev[RX_INDEX].dmaChanNum, &block->dev[RX_INDEX].dmaChanInfo)
	    != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Config_Channel Failed for I2S RX channel\n");
		goto rx_error2;
	}

	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}

	bcm_i2s_txrx_fifo_enable(block->i2s_handle, 1);

	bcm_i2s_enable_clk(block->i2s_handle, I2S_EXT_CLK, 0);

	bcm_i2s_interface_enable(block->i2s_handle, false, true, 0);

	pr_debug("I2SDRV_Start_rx, bufCnt=%d, state=%d\r\n",
		 block->dev[RX_INDEX].bufCnt, block->dev[RX_INDEX].state);

	dma_buf_list_rx.buffers[0].srcAddr =
	    (int)(io_v2p((int)bcm_i2s_get_base_addr()+ 0x0008));
	dma_buf_list_rx.buffers[0].length = block->dev[RX_INDEX].len;
	dma_buf_list_rx.buffers[0].destAddr = block->dev[RX_INDEX].dest_addr;
	dma_buf_list_rx.buffers[0].bRepeat = 0;
	dma_buf_list_rx.buffers[0].interrupt = 1;
	dma_data_rx.numBuffer = 1;
	dma_data_rx.pBufList = (Dma_Buffer_List *) & dma_buf_list_rx;

	if (DMADRV_Bind_Data(block->dev[RX_INDEX].dmaChanNum, &dma_data_rx) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Bind_Data Failed for I2S RX channel\n");
		goto rx_error2;
	}

	if (DMADRV_Start_Transfer(block->dev[RX_INDEX].dmaChanNum) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Start_Transfer Failed for I2S RX channel\n");
		goto rx_error2;
	}
	block->dev[RX_INDEX].state = I2STST_STREAMING;
	return 0;

rx_error2:
	DMADRV_Release_Channel(block->dev[RX_INDEX].dmaChanNum);
rx_error1:
	return I2S_DMA_SETUP_ERROR;
}

/**
* @brief    Start I2S Receive (RX) in DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned int I2SDRV_Start_rxDMA(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;

        if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id)
        {
     	    pr_debug("I2SDRV_Start_rxDMA: bad handle\r\n");
            return I2S_BAD_HANDLE;
        }

	bcm_i2s_flush_fifo(block->i2s_handle,CAPTURE);
	if (DMADRV_Start_Transfer(block->dev[RX_INDEX].dmaChanNum) !=
	    DMADRV_STATUS_OK) {
		pr_info("DMADRV_Start_Transfer Failed for I2S RX channel\n");
		return -1;
	}
	block->dev[RX_INDEX].state = I2STST_STREAMING;
	return 0;
}

/**
* @brief    Stop I2S Receive (RX) in DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned int I2SDRV_Stop_rxDMA(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;

        if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id)
        {
     	    pr_debug("I2SDRV_Stop_rxDMA: bad handle\r\n");
            return I2S_BAD_HANDLE;
        }

	pr_debug("I2SDRV_Stop_rxDMA bufCnt=%d, state=%d\r\n",
		 block->dev[RX_INDEX].bufCnt, block->dev[RX_INDEX].state);

	if (block->circular == TRUE)
        DMADRV_Stop_Transfer(block->dev[RX_INDEX].dmaChanNum);
	/* abort any any on-going DMA, make sure bufCnt is 0 */
	else
	if (block->dev[RX_INDEX].bufCnt != 0) {
		/* stopping */
		 DMADRV_Stop_Transfer(block->dev[RX_INDEX].dmaChanNum);
		block->dev[RX_INDEX].bufCnt = 0;
	}
//	bcm_i2s_flush_fifo(block->i2s_handle,CAPTURE);
	bcm_i2s_disable_clk(block->i2s_handle, I2S_INT_CLK);
	block->dev[RX_INDEX].state = I2STST_RUNNING_NO_DMA;
	return 0;
}

/**
* @brief     Queue a circular DMA buffer for Transmit (TX)
*
* @param handle     I2S handle
* @param circular        DMA buffer
*
* @return           0 for success, i2s error status on error
*/
unsigned int I2SDRV_Cir_tx_buf(I2S_HANDLE handle, I2S_Cir_t *circular)
{
	struct i2s_block *block = (struct i2s_block*)handle;
	int i, current_buf;
	Dma_Buffer_List *DataBufferList;
	Dma_Data DataPacket;

        pr_debug("I2SDRV_Cir_tx_buf\n");

        if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id)
        {
     	    pr_debug("I2SDRV_Cir_tx_buf: bad handle\r\n");
            return I2S_BAD_HANDLE;
        }
	if (block->dev[TX_INDEX].get_chan == false){
		if(DMADRV_Obtain_Channel(DMA_CLIENT_MEMORY, DMA_CLIENT_I2S_TX,
                                       (DMA_CHANNEL *) & block->dev[TX_INDEX].
                                       dmaChanNum) != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Obtain_Channel failed for I2S TX channel\n");
		return -1;
		}
		block->dev[TX_INDEX].get_chan = true;
	}

	pr_debug("I2SDRV_Cir_tx_buf, bufCnt=%d, state=%d\r\n",
        block->dev[TX_INDEX].bufCnt, block->dev[TX_INDEX].state);

	if (block->dev[TX_INDEX].state == I2STST_NOT_INIT)
		return I2S_WRONG_STATE;

	block->circular = TRUE;

	DataBufferList = (void *)kzalloc(circular->bufferNum * sizeof(Dma_Buffer_List), GFP_KERNEL);

	current_buf = circular->startAddr;
	for (i = 0; i < circular->bufferNum; i++)
	{
		DataBufferList[i].buffers[0].srcAddr = (UInt32)current_buf;
		DataBufferList[i].buffers[0].destAddr = (int)(io_v2p((int)bcm_i2s_get_base_addr() + 0x0008));
		DataBufferList[i].buffers[0].length = circular->bufferSize;
		DataBufferList[i].buffers[0].bRepeat = 0;
		DataBufferList[i].buffers[0].interrupt = 1;

		current_buf += circular->bufferSize;
	}

	DataPacket.numBuffer = circular->bufferNum;

	block->dev[TX_INDEX].dmaChanInfo.srcID = DMA_CLIENT_MEMORY;
	block->dev[TX_INDEX].dmaChanInfo.dstID = DMA_CLIENT_I2S_TX;
	block->dev[TX_INDEX].dmaChanInfo.type = DMA_FCTRL_MEM_TO_PERI;
	block->dev[TX_INDEX].dmaChanInfo.alignment = DMA_ALIGNMENT_32;
	block->dev[TX_INDEX].dmaChanInfo.srcBstSize = DMA_BURST_SIZE_16;
	block->dev[TX_INDEX].dmaChanInfo.dstBstSize = DMA_BURST_SIZE_16;
	block->dev[TX_INDEX].dmaChanInfo.srcDataWidth = DMA_DATA_SIZE_16BIT;
	block->dev[TX_INDEX].dmaChanInfo.dstDataWidth = DMA_DATA_SIZE_16BIT;
	block->dev[TX_INDEX].dmaChanInfo.incMode = DMA_INC_MODE_SRC;
	block->dev[TX_INDEX].dmaChanInfo.xferCompleteCb = (DmaDrv_Callback) i2s_dma_handler_tx;
	block->dev[TX_INDEX].dmaChanInfo.freeChan = FALSE;
	block->dev[TX_INDEX].dmaChanInfo.priority = 0;
	block->dev[TX_INDEX].dmaChanInfo.bCircular = TRUE;

	DataPacket.pBufList = (Dma_Buffer_List *)DataBufferList;

	DMADRV_Config_Channel(block->dev[TX_INDEX].dmaChanNum,
                          &block->dev[TX_INDEX].dmaChanInfo);

	/* prepare data to be transfered*/
	DMADRV_Bind_Data(block->dev[TX_INDEX].dmaChanNum, &DataPacket);
	kfree(DataBufferList);
	tx_handle = block;
	block->dev[TX_INDEX].bufCnt++;

	pr_debug("I2SDRV_Cir_tx_buf exiting, bufCnt=%d\r\n",
	    block->dev[TX_INDEX].bufCnt);

	return 0;
}

/**
* @brief     Queue a circular DMA buffer for Receive (RX)
*
* @param handle     I2S handle
* @param circular        DMA buffer
*
* @return           0 for success, i2s error status on error
*/
unsigned int I2SDRV_Cir_rx_buf(I2S_HANDLE handle, I2S_Cir_t *circular)
{
	struct i2s_block *block = (struct i2s_block*)handle;
	int i, current_buf;
	Dma_Buffer_List *DataBufferList;
	Dma_Data DataPacket;

        pr_debug("I2S Driver: I2SDRV_Cir_rx_buf\n");

        if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id)
        {
     	    pr_debug("I2SDRV_Cir_rx_buf: bad handle\r\n");
            return I2S_BAD_HANDLE;
        }
	if (block->dev[RX_INDEX].get_chan == false){
		if (DMADRV_Obtain_Channel(DMA_CLIENT_I2S_RX, DMA_CLIENT_MEMORY,
				  (DMA_CHANNEL *) & block->dev[RX_INDEX].
				  dmaChanNum) != DMADRV_STATUS_OK) {
		pr_info("DMADRV_Obtain_Channel failed for I2S RX channel\n");
		return -1;
		}
		block->dev[RX_INDEX].get_chan = true;
	}

	pr_debug("I2SDRV_Cir_rx_buf, bufCnt=%d, state=%d\r\n",
        block->dev[RX_INDEX].bufCnt, block->dev[RX_INDEX].state);

	if (block->dev[RX_INDEX].state == I2STST_NOT_INIT)
		return I2S_WRONG_STATE;

	block->circular = TRUE;

	DataBufferList = (void *)kzalloc(circular->bufferNum * sizeof(Dma_Buffer_List), GFP_KERNEL);

	bcm_i2s_enable_clk(block->i2s_handle, I2S_INT_CLK, RATE_12_MHZ);
	current_buf = circular->startAddr;
	for (i = 0; i < circular->bufferNum; i++)
	{
		DataBufferList[i].buffers[0].srcAddr = (int)(io_v2p( (int )bcm_i2s_get_base_addr() + 0x0008));
		DataBufferList[i].buffers[0].destAddr = (UInt32)current_buf;
		DataBufferList[i].buffers[0].length = circular->bufferSize;
		DataBufferList[i].buffers[0].bRepeat = 0;
		DataBufferList[i].buffers[0].interrupt = 1;

		current_buf += circular->bufferSize;
	}

	DataPacket.numBuffer = circular->bufferNum;

	block->dev[RX_INDEX].dmaChanInfo.srcID = DMA_CLIENT_I2S_RX;
	block->dev[RX_INDEX].dmaChanInfo.dstID = DMA_CLIENT_MEMORY;
	block->dev[RX_INDEX].dmaChanInfo.type = DMA_FCTRL_PERI_TO_MEM;
	block->dev[RX_INDEX].dmaChanInfo.alignment = DMA_ALIGNMENT_32;
	block->dev[RX_INDEX].dmaChanInfo.srcBstSize = DMA_BURST_SIZE_16;
	block->dev[RX_INDEX].dmaChanInfo.dstBstSize = DMA_BURST_SIZE_16;
	block->dev[RX_INDEX].dmaChanInfo.srcDataWidth = DMA_DATA_SIZE_16BIT;
	block->dev[RX_INDEX].dmaChanInfo.dstDataWidth = DMA_DATA_SIZE_16BIT;
	block->dev[RX_INDEX].dmaChanInfo.incMode = DMA_INC_MODE_DST;
	block->dev[RX_INDEX].dmaChanInfo.xferCompleteCb = (DmaDrv_Callback) i2s_dma_handler_rx;
	block->dev[RX_INDEX].dmaChanInfo.freeChan = FALSE;
	block->dev[RX_INDEX].dmaChanInfo.priority = 0;
	block->dev[RX_INDEX].dmaChanInfo.bCircular = TRUE;

	DataPacket.pBufList = (Dma_Buffer_List *)DataBufferList;

	DMADRV_Config_Channel(block->dev[RX_INDEX].dmaChanNum,
                          &block->dev[RX_INDEX].dmaChanInfo);

	/* prepare data to be transfered*/
	DMADRV_Bind_Data(block->dev[RX_INDEX].dmaChanNum, &DataPacket);
	kfree(DataBufferList);
	rx_handle = block;
	block->dev[RX_INDEX].bufCnt++;

	pr_debug("I2SDRV_Cir_rx_buf exiting, bufCnt=%d\r\n",
	    block->dev[RX_INDEX].bufCnt);

	return 0;
}
/**
* @brief    Start I2S Receive (RX) in non DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Start_rx_noDMA(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_Start_rx_noDMA: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	clk_enable(block->dam_clk);
	bcm_i2s_enable_clk(block->i2s_handle, I2S_EXT_CLK, 0);

	pr_debug("I2SDRV_Start_rx_noDMA, bufCnt=%d, state=%d\r\n",
		 block->dev[RX_INDEX].bufCnt, block->dev[RX_INDEX].state);
	if (bcm_i2s_txrx_fifo_enable(block->i2s_handle, CAPTURE))
		pr_debug("I2SDRV_Start_rx_noDMA:bcm_i2s_txrx_fifo_enable \
			failed! \r\n");
	if (bcm_i2s_interface_enable(block->i2s_handle, false, true, 0))
		pr_debug("I2SDRV_Start_rx_noDMA:bcm_i2s_interface_enable \
			failed! \r\n");
	block->dev[RX_INDEX].state = I2STST_RUNNING_NO_DMA;
	return 0;
}

/**
* @brief Stop I2S Receive (TX) in non DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Stop_rx_noDMA(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	pr_debug("I2SDRV_Start_tx_noDMA, bufCnt=%d, state=%d\r\n",
		 block->dev[RX_INDEX].bufCnt, block->dev[RX_INDEX].state);
	if (bcm_i2s_interface_disable(block->i2s_handle, false, true))
		pr_debug("I2SDRV_Stop_rx_noDMA:bcm_i2s_interface_disable \
			failed! \r\n");
	if (bcm_i2s_disable_clk(block->i2s_handle, I2S_EXT_CLK))
		pr_debug("I2SDRV_Stop_rx_noDMA :bcm_i2s_disable_clk \
			failed! \r\n");
	block->dev[RX_INDEX].state = I2STST_READY;
	clk_disable(block->dam_clk);
	return 0;
}

/**
* @brief    Stop I2S Transmit (TX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Stop_tx(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;

	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	pr_debug("I2SDRV_Stop_tx, bufCnt=%d, state=%d\r\n",
		 block->dev[RX_INDEX].bufCnt, block->dev[RX_INDEX].state);
	if (block->dev[TX_INDEX].bufCnt != 0) {
		/* stopping */
		if (bcm_i2s_interface_disable(block->i2s_handle, true, false)) {
			pr_debug
			    ("I2SDRV_Stop_tx:bcm_i2s_interface_disable  failed! \r\n");
		}
		if (bcm_i2s_disable_clk(block->i2s_handle, I2S_EXT_CLK)) {
			pr_debug
			    ("I2SDRV_Stop_tx:bcm_i2s_disable_clk  failed! \r\n");
		}
		block->dev[TX_INDEX].bufCnt = 0;
	}
	block->dev[TX_INDEX].state = I2STST_READY;
	clk_disable(block->dam_clk);
	return 0;

}

/**
* @brief    Stop I2S Receive (RX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Stop_rx(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;

	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	pr_debug("I2SDRV_Stop_tx, bufCnt=%d, state=%d\r\n",
		 block->dev[RX_INDEX].bufCnt, block->dev[RX_INDEX].state);
	/* abort any any on-going DMA, make sure bufCnt is 0 */
	if (block->dev[RX_INDEX].bufCnt != 0) {
		/* stopping */

		block->dev[RX_INDEX].bufCnt = 0;
		bcm_i2s_interface_disable(block->i2s_handle, false, true);
		bcm_i2s_disable_clk(block->i2s_handle, I2S_EXT_CLK);
		block->dev[RX_INDEX].state = I2STST_READY;
	}
	clk_disable(block->dam_clk);
	return 0;

}

/**
* @brief    Register an end-of-transfer callback function for Transmit (TX)
*
* @param handle     I2S handle
* @param endCb      Pointer to callback function
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Register_tx_Cb(I2S_HANDLE handle, I2sEndCb endCb)
{
	struct i2s_block *block = (struct i2s_block *)handle;

	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	pr_debug("I2SDRV_register_tx_Cb, old=%x, new=%x\r\n",
		 (unsigned int)block->dev[TX_INDEX].endCallback,
		 (unsigned int)endCb);

	/* Register API's End-Call-back function */
	if (block->dev[TX_INDEX].endCallback != endCb) {
		block->dev[TX_INDEX].endCallback = endCb;
	}
	return 0;
}

/**
* @brief    Register an end-of-transfer callback function for Receive (RX)
*
* @param handle     I2S handle
* @param endCb      Pointer to callback function
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Register_rx_Cb(I2S_HANDLE handle, I2sEndCb endCb)
{
	struct i2s_block *block = (struct i2s_block *)handle;

	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	pr_debug("I2SDRV_register_rx_Cb, old=%x, new=%x\r\n",
		 (unsigned int)block->dev[RX_INDEX].endCallback,
		 (unsigned int)endCb);
	/* Register API's End-Call-back function */
	if (block->dev[RX_INDEX].endCallback != endCb) {
		block->dev[RX_INDEX].endCallback = endCb;
	}
	return 0;
}

/**
* @brief    Get the I2S device Transmit (TX) state
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
I2S_STATE I2SDRV_Get_tx_state(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	return block->dev[TX_INDEX].state;

}

/**
* @brief    Get the I2S device Receive (RX) state
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
I2S_STATE I2SDRV_Get_rx_state(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	return block->dev[RX_INDEX].state;
}

/**
* @brief    Pause I2S Transmission (TX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Pause_tx(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;

	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}

	if (block->dev[TX_INDEX].state != I2STST_STREAMING) {
		pr_debug("I2SDRV:i2s_pause_tx: error! can't pause at %d\r\n",
			 block->dev[TX_INDEX].state);
		return (unsigned char)I2S_WRONG_STATE;
	}
	bcm_i2s_interface_disable(block->i2s_handle, true, false);
	bcm_i2s_txrx_fifo_disable(block->i2s_handle, PLAYBACK);
	block->dev[TX_INDEX].state = I2STST_PAUSING;
	pr_debug("I2SDRV_Pause_tx : Pausing TX.. \r\n");
	return 0;

}

/**
* @brief    Pause I2S Reception (RX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Pause_rx(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;

	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}

	if (block->dev[RX_INDEX].state != I2STST_STREAMING) {
		pr_debug("I2SDRV:i2s_pause_rx: error! can't pause at %d\r\n",
			 block->dev[RX_INDEX].state);
		return (unsigned char)I2S_WRONG_STATE;
	}
	bcm_i2s_interface_disable(block->i2s_handle, false, true);
	bcm_i2s_txrx_fifo_disable(block->i2s_handle, CAPTURE);
	block->dev[RX_INDEX].state = I2STST_PAUSING;
	pr_debug("I2SDRV_Pause_tx : Pausing RX.. \r\n");
	return 0;
}

/**
* @brief    Resume previously 'paused' I2S Transmission (TX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Resume_tx(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}

	if (block->dev[TX_INDEX].state != I2STST_PAUSING) {
		pr_debug("I2SDRV_resume_tx: error! can't resume %d\r\n",
			 block->dev[TX_INDEX].state);
		return (unsigned char)I2S_WRONG_STATE;
	}

	bcm_i2s_interface_enable(block->i2s_handle, true, false, 0);
	bcm_i2s_txrx_fifo_enable(block->i2s_handle, PLAYBACK);
	block->dev[TX_INDEX].state = I2STST_STREAMING;
	pr_debug("I2SDRV_Resume_tx : Resuming TX.. \r\n");
	return 0;
}

/**
* @brief    Resume previously 'paused' I2S Reception (RX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Resume_rx(I2S_HANDLE handle)
{
	struct i2s_block *block = (struct i2s_block *)handle;

	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}

	if (block->dev[RX_INDEX].state != I2STST_PAUSING) {
		pr_debug("I2SDRV_resume_rx: error! can't resume %d\r\n",
			 block->dev[RX_INDEX].state);
		return (unsigned char)I2S_WRONG_STATE;
	}

	bcm_i2s_interface_enable(block->i2s_handle, false, true, 0);
	bcm_i2s_txrx_fifo_enable(block->i2s_handle, CAPTURE);
	block->dev[RX_INDEX].state = I2STST_STREAMING;
	pr_debug("I2SDRV_Resume_rx : Resuming RX.. \r\n");
	return 0;
}

/**
* @brief    Enable/Disable I2S internal loopback for testing
*
* @param handle     I2S handle
* @param enable     enable (true/false)
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Enable_Loopback(I2S_HANDLE handle, bool enable)
{
	struct i2s_block *block = (struct i2s_block *)handle;
	if (block->magic_number != (int)"I2S_BLOCK_MAGIC_NUMBER" + block->id) {
		pr_debug("I2SDRV_set_mode: bad handle\r\n");
		return I2S_BAD_HANDLE;
	}
	bcm_i2s_loopback_cfg(block->i2s_handle, enable);
	return 0;

}
/**
 * I2S Driver module Init function
 */
static int __init i2s_init(void)
{
	struct clk *damck = NULL;

	pr_info("i2s module loaded successfully\n");

	damck = clk_get(NULL, "damck");

	/*Set to slave mode @ init to prevent current leakage when BT is ON */
	if (IS_ERR(damck)) {
		pr_info("Cannot obtain DAM clock for I2S!\n");
		return PTR_ERR(damck);
	} else {
		clk_enable(damck);
		writel(readl(bcm_i2s_get_base_addr()+ 0x000c) | 0x1,
		       bcm_i2s_get_base_addr() + 0x000c);
		clk_disable(damck);
	}
	return 0;
}

/**
 * I2S Driver module exit function
 */
static void __exit i2s_exit(void)
{
	pr_info("dma_i2s module unloaded successfully\n");
}

/** @} */

EXPORT_SYMBOL(I2SDRV_Get_handle);
EXPORT_SYMBOL(I2SDRV_Init);
EXPORT_SYMBOL(I2SDRV_Set_tx_format);
EXPORT_SYMBOL(I2SDRV_Set_mode);
EXPORT_SYMBOL(I2SDRV_Cleanup);
EXPORT_SYMBOL(I2SDRV_Set_rx_format);
EXPORT_SYMBOL(I2SDRV_Queue_tx_buf);
EXPORT_SYMBOL(I2SDRV_Queue_rx_buf);
EXPORT_SYMBOL(I2SDRV_Start_tx);
EXPORT_SYMBOL(I2SDRV_Start_rx);
EXPORT_SYMBOL(I2SDRV_Start_rx_noDMA);
EXPORT_SYMBOL(I2SDRV_Stop_rx_noDMA);
EXPORT_SYMBOL(I2SDRV_Stop_tx);
EXPORT_SYMBOL(I2SDRV_Stop_rx);
EXPORT_SYMBOL(I2SDRV_Start_rxDMA);
EXPORT_SYMBOL(I2SDRV_Stop_rxDMA);
EXPORT_SYMBOL(I2SDRV_Cir_tx_buf);
EXPORT_SYMBOL(I2SDRV_Cir_rx_buf);
EXPORT_SYMBOL(I2SDRV_Register_tx_Cb);
EXPORT_SYMBOL(I2SDRV_Register_rx_Cb);
EXPORT_SYMBOL(I2SDRV_Get_tx_state);
EXPORT_SYMBOL(I2SDRV_Get_rx_state);
EXPORT_SYMBOL(I2SDRV_Pause_tx);
EXPORT_SYMBOL(I2SDRV_Pause_rx);
EXPORT_SYMBOL(I2SDRV_Resume_tx);
EXPORT_SYMBOL(I2SDRV_Resume_rx);
EXPORT_SYMBOL(I2SDRV_Enable_Loopback);

module_init(i2s_init);
module_exit(i2s_exit);
