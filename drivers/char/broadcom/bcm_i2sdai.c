/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/char/broadcom/bcm_i2sdai.c
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
*   @file   bcm_i2sdai.c
*   @brief  This file defines the I2S Digital Audio Interface
*
*****************************************************************************/

/**
*   @defgroup   I2SDAIGroup   I2S Digital Audio Interface API's
*   @brief      This group defines the I2S Digital Audio Interface API's
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/sizes.h>
#include <mach/hardware.h>
#include <mach/memory.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <plat/bcm_i2sdai.h>
#include <plat/cpu.h>

/**
 * I2S Register offsets
 */
#define I2S_DATXCTRL_REG	0x0000
#define I2S_DARXCTRL_REG	0x0004
#define	I2S_DAFIFO_DATA_REG	0x0008
#define	I2S_DAI2S_REG		0x000c
#define	I2S_DAI2SRX_REG		0x0010
#define	I2S_DADMACTRL_REG	0x0014
#define	I2S_DASTA_REG		0x001c

#if (!cpu_is_bcm2153())
/**
 * These set of regs are not present on BCM2153
 */
#define	I2S_DAMSCR_REG		0x002c
#define	I2S_DATLCSR_REG		0x0030
#define	I2S_DATRCSR_REG		0x0034
#define	I2S_DARLCSR_REG		0x0038
#define	I2S_DARRCSR_REG		0x003c
#define	I2S_DADDR_REG		0x0040
#define	I2S_DATXDDVR_REG	0x0044
#define	I2S_DARXDDVR_REG	0x0048
#define	I2S_DASCKSTOP_REG	0x0050
#define	I2S_DASPDIF_REG		0x0080
#define	I2S_DASIDETONE_REG	0x0100
#endif

/**
 * Bit Shifts in Registers
 */

/*DATX-RXCTRL*/
#define I2S_TX_RX_FIFO_ENABLE 	0
#define	I2S_TX_RX_FIFO_FLUSH	1
#define I2S_SOFT_RESET		3
#define I2S_TX_RX_FIFO_STAT	4
#define I2S_TX_RX_FIFO_INT_EN	5
#define I2S_TX_RX_INT_STAT	6
#define I2S_TX_RX_INT_EN	7
#define I2S_TX_RX_INT_PERIOD	8

#define I2S_TX_RX_INT_PERIOD_MASK 0xFF00

/*I2S_DATXI2S*/
#define I2S_INTERFACE_MODE	 0
#define I2S_PLAYBACK_MODE	 1
#define I2S_SCK_STOP		 3
#define I2S_RXEN		 4
#define I2S_TXEN		 5
#define I2S_TX_START_RIGHT	 6
#define I2S_TX_RX_SRDIV		 8
#define I2S_DAOD_CTRL           13
#define I2S_DAOLR_CTRL          14

#define I2S_TX_RX_SRDIV_MASK	0xF00

#define I2S_DAOD_CTRL_MASK	0xC000

/*DARXI2S*/
#define I2S_CHANNEL_MODE	 0
#define I2S_RECORD_CHANNELS	 1
#define I2S_MONO_AVG_REC	 2
#define I2S_TXLRC4RX		 3
#define I2S_RX_START_RIGHT	 6
#define I2S_LOOP_EN		 7
#define I2S_DAID_CTRL		 12
#define I2S_DAILR_CTRL		 14

#define I2S_DAID_CTRL_MASK	0xC000

/*DMA_CTRL*/
#define I2S_TX_DMA_SIZE		 0
#define I2S_TX_DMA_EN		 7
#define I2S_RX_DMA_SIZE		 8
#define I2S_RX_DMA_EN		 15

#define I2S_RX_DMA_SIZE_MASK	0x700
#define I2S_TX_DMA_SIZE_MASK	0x7

/*I2S_DASTA*/
#define I2S_TX_STA		 5
#define I2S_RX_STA		 4

#define I2S_TX_RX_LR_CHANNEL_SIZE	0
#define I2S_TX_RX_LR_CHANNEL_SIZE_MASK 0x3FF

#define I2S_TX_DATA_DEPT_H_MOD	 0
#define I2S_TX_JUSTISFIED_MOD	 3
#define I2S_TX_16BIT_PACKED	 4
#define I2s_TX_16BIT_MSHW_LEFT	 5
#define I2S_TX_DATA_MSB_ALIGN	 7
#define I2S_RX_DATA_DEPT_H_MOD	 8
#define I2S_RX_JUSTISFIED_MOD	 11
#define I2S_RX_16BIT_PACKED	 12
#define I2s_RX_16BIT_MSHW_LEFT	 13
#define I2S_RX_DATA_MSB_ALIGN	 15

#define I2S_16BIT_PACK_MASK      0x3FF

#define I2S_TX_DATA_DEPT_H_MOD_MASK 0x7
#define I2S_RX_DATA_DEPT_H_MOD_MASK 0x700

 /*DASCKSTOP*/
#define SCK_RUN_CNT 		 0
#define SCK_PRERUN_CNT 		 10
#define SCK_RUN_CNT_MASK 	0x3FF
#define SCK_PRERUN_CNT_MASK	0xFFC00
     /*DASPDIF*/
#define I2S_SPDIF_MODE		0
#define I2S_SPDIF_CH_CODE 	1
#define DAOLR_UP 3
#define DAID_UP  3
#define ENABLE_FIELD		0x1
#define REG_RESET_VAL		0x00000000
#define REG_DATXCTRL_RESET_VAL	0x0000C010
#define REG_DARXCTRL_RESET_VAL	0x0000C000
    enum HANDLE_STATE {
	HANDLE_FREE,		/*!< Handle is available for use. */
	HANDLE_IN_USE		/*!< Handle is currently in use.  */
};

/**
 * Local i2s device Instance
 */
static struct i2s_device bcm_i2s = {
	.sample_rate_div = BCLK_DIV_1500,
	.i2s_handle = 0,
	.i2s_handle_state = HANDLE_FREE,
	.channels = 0,
	.tx_op_mode = 0,
	.rx_op_mode = 0,
	.tx_active = 0,
	.rx_active = 0,
};

static void __iomem *i2s_base_addr;

/*****************************************************************************
* Local Functions for Register Access
*****************************************************************************/
static inline unsigned long get_register(unsigned int offset)
{
	return readl(i2s_base_addr + offset);
}

static inline void set_register(unsigned int data, unsigned int offset)
{
	writel(data, i2s_base_addr + offset);
}

/*****************************************************************************
* Exported Functions
*****************************************************************************/

/** @addtogroup I2SDAIGroup
    @{
*/

/**
* Request for I2S device
*
* @return 	i2s device instance if not in use; NULL if in use
*/
struct i2s_device *use_i2s_device(void)
{
	struct i2s_device *i2s_handle;
	unsigned long flags = 0;

	spin_lock_irqsave(&bcm_i2s.i2s_lock, flags);
	if (bcm_i2s.i2s_handle_state != HANDLE_IN_USE) {
		i2s_handle = (struct i2s_device *)&bcm_i2s;
		bcm_i2s.i2s_handle_state = HANDLE_IN_USE;
		spin_unlock_irqrestore(&bcm_i2s.i2s_lock, flags);
		pr_debug("[bcm_i2s] %s: obtained handle for i2s\r\n",
			 __FUNCTION__);
		return i2s_handle;
	} else
		spin_unlock_irqrestore(&bcm_i2s.i2s_lock, flags);
	return NULL;
}

/**
* Release i2S device
*
* @param i2s_handle	I2S device Instance
*/
void release_i2s_device(struct i2s_device *i2s_handle)
{
	unsigned long flags = 0;

	if (i2s_handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return;
	}

	spin_lock_irqsave(&bcm_i2s.i2s_lock, flags);
	bcm_i2s.i2s_handle_state = HANDLE_FREE;
	bcm_i2s.sample_rate_div = BCLK_DIV_1500;
	bcm_i2s.channels = 0;
	bcm_i2s.tx_op_mode = 0;
	bcm_i2s.rx_op_mode = 0;
	bcm_i2s.tx_active = 0;
	bcm_i2s.rx_active = 0;
	i2s_handle = NULL;
	spin_unlock_irqrestore(&bcm_i2s.i2s_lock, flags);
	/*Write reset values */
	pr_debug("[bcm_i2s] %s: released i2s handle \r\n", __FUNCTION__);
}

/**
* Configure Playback/Record Interrupts and enable TX/Rx FIFO
*
* @param handle			i2S device Instance
* @param int_period 	DMA interrupt period for Playback/Capture
* @param direction		Direction (Playback/Capture)
* @param en_dis			Enable/Disable the FIFO
*
* @return 				0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_cfg_play_rec_interrupt(struct i2s_device *handle,
					    unsigned int int_period,
					    int direction, bool en_dis)
{
	unsigned int value = 0;
	unsigned int offset = 0;

	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}

	if (direction == PLAYBACK) {
		offset = I2S_DATXCTRL_REG;
	} else if (direction == CAPTURE) {
		offset = I2S_DARXCTRL_REG;
	} else
		return -EINVAL;
	value = get_register(offset);
	if (en_dis) {
		value &= (~I2S_TX_RX_INT_PERIOD_MASK);
		value |= (ENABLE_FIELD << I2S_TX_RX_FIFO_ENABLE) |
		    (int_period << I2S_TX_RX_INT_PERIOD) |
		    (ENABLE_FIELD << I2S_TX_RX_INT_EN) | (ENABLE_FIELD << 2);
	} else {
		value &= (~I2S_TX_RX_INT_PERIOD_MASK);
		value &= ~(ENABLE_FIELD << I2S_TX_RX_FIFO_ENABLE);
	}
	set_register(value, offset);
	return 0;
}

/**
* Read the status of Interrupts
*
* @param handle 	i2S device Instance
* @param direction	Direction (Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_get_interrupt_status(struct i2s_device *handle,
					  unsigned int direction)
{
	unsigned int value = 0;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}

	if (direction == PLAYBACK)
		value = get_register(I2S_DATXCTRL_REG);
	else if (direction == CAPTURE)
		value = get_register(I2S_DARXCTRL_REG);
	return (value & 0x40) >> I2S_TX_RX_INT_STAT;
}

/**
* Enable Interrupts when TX/Rx FIFO is empty
*
* @param handle		i2S device Instance
* @param direction	Direction (Playback/Capture)
* @param en_dis		Enable/Disable the FIFO
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_fifo_empty_int_cfg(struct i2s_device *handle,
					unsigned int direction, bool en_dis)
{
	unsigned int value = 0;
	unsigned int offset = 0;

	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}

	if (direction == PLAYBACK) {
		offset = I2S_DATXCTRL_REG;
	} else if (direction == CAPTURE) {
		offset = I2S_DARXCTRL_REG;
	} else
		return -EINVAL;
	value = get_register(offset);
	if (en_dis) {
		value |= (ENABLE_FIELD << I2S_TX_RX_FIFO_INT_EN);
		pr_debug("[bcm_i2s] %s: enable FIFO empty interrupt \r\n",
			 __FUNCTION__);
	} else {
		value &= ~(ENABLE_FIELD << I2S_TX_RX_FIFO_INT_EN);
		pr_debug("[bcm_i2s] %s: disable FIFO empty interrupt \r\n",
			 __FUNCTION__);
	}
	set_register(value, offset);
	return 0;
}

/**
* Enable Transmit/Receive (TX/Rx) FIFO
*
* @param handle		i2S device Instance
* @param direction	Direction (Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_txrx_fifo_enable(struct i2s_device *handle,
				      unsigned int direction)
{
	unsigned int value;

	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	if (direction == PLAYBACK) {
		value = get_register(I2S_DATXCTRL_REG);
		value |= (ENABLE_FIELD << I2S_TX_RX_FIFO_ENABLE);
		set_register(value, I2S_DATXCTRL_REG);
		pr_debug("[bcm_i2s] %s: enable TX FIFO \r\n", __FUNCTION__);
	} else {
		value = get_register(I2S_DARXCTRL_REG);
		value |= (ENABLE_FIELD << I2S_TX_RX_FIFO_ENABLE);
		set_register(value, I2S_DARXCTRL_REG);
		pr_debug("[bcm_i2s] %s: enable RX FIFO \r\n", __FUNCTION__);
	}
	return 0;
}

/**
* Disable Transmit/Receive (TX/Rx) FIFO
*
* @param handle		i2S device Instance
* @param direction	Direction(Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_txrx_fifo_disable(struct i2s_device *handle,
				       unsigned int direction)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DATXCTRL_REG);
	if (direction == PLAYBACK) {
		value &= ~(ENABLE_FIELD << I2S_TX_RX_FIFO_ENABLE);
		set_register(value, I2S_DATXCTRL_REG);
		pr_debug("[bcm_i2s] %s: disable TX FIFO \r\n", __FUNCTION__);
	} else {
		value &= ~(ENABLE_FIELD << I2S_TX_RX_FIFO_ENABLE);
		set_register(value, I2S_DARXCTRL_REG);
		pr_debug("[bcm_i2s] %s: disable RX FIFO \r\n", __FUNCTION__);
	}
	return 0;
}

/**
* Flush Transmit/Receive (TX/Rx) FIFO
*
* @param handle		i2S device Instance
* @param direction	Direction(Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_flush_fifo(struct i2s_device *handle,
				unsigned int direction)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	if (direction == PLAYBACK) {
		value = get_register(I2S_DATXCTRL_REG);
		value |= (ENABLE_FIELD << I2S_TX_RX_FIFO_FLUSH);
		set_register(value, I2S_DATXCTRL_REG);
		pr_debug("[bcm_i2s] %s: Flush TX FIFO \r\n", __FUNCTION__);
	} else {
		value = get_register(I2S_DARXCTRL_REG);
		value |= (ENABLE_FIELD << I2S_TX_RX_FIFO_FLUSH);
		set_register(value, I2S_DARXCTRL_REG);
		value &= ~(ENABLE_FIELD << I2S_TX_RX_FIFO_FLUSH);
		set_register(value, I2S_DARXCTRL_REG);
		pr_debug("[bcm_i2s] %s: Flush RX FIFO \r\n", __FUNCTION__);
	}
	return 0;
}

/**
* Configure Sample Rate Divider ratio
*
* @param handle		i2S device Instance
* @param rate		sample rate ratio
* @param direction	Direction(Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_sample_rate_div(struct i2s_device *handle,
				     int rate, int direction)
{
	unsigned int value;
#if !(cpu_is_bcm2153() || cpu_is_bcm215xx())
	unsigned int clock = 0, tx_lsize = 0, tx_rsize = 0;
#endif
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
#if (cpu_is_bcm2153() || cpu_is_bcm215xx())
	set_register(0x0, I2S_DAMSCR_REG);
	if (direction == PLAYBACK) {
		value = get_register(I2S_DAI2S_REG);
		value =
		    (value & ~(I2S_TX_RX_SRDIV_MASK)) | (rate <<
							 I2S_TX_RX_SRDIV);
		set_register(value, I2S_DAI2S_REG);
	} else {
		value = get_register(I2S_DAI2SRX_REG);
		value =
		    (value & ~(I2S_TX_RX_SRDIV_MASK)) | (rate <<
							 I2S_TX_RX_SRDIV);
		set_register(value, I2S_DAI2SRX_REG);
	}
#else
	switch (rate) {
	case I2S_SAMPLERATE_8000HZ:
		tx_lsize = 16;
		tx_rsize = 16;
		clock = 256000;
		break;
	case I2S_SAMPLERATE_11030HZ:
		tx_lsize = 68;
		tx_rsize = 68;
		clock = 1500000;
		break;
	case I2S_SAMPLERATE_12000HZ:
		tx_lsize = 64;
		tx_rsize = 64;
		clock = 1536000;
		break;
	case I2S_SAMPLERATE_16000HZ:
		tx_lsize = 16;
		tx_rsize = 16;
		clock = 512000;
		break;
	case I2S_SAMPLERATE_22060HZ:
		tx_lsize = 34;
		tx_rsize = 34;
		clock = 1500000;
		break;
	case I2S_SAMPLERATE_24000HZ:
		tx_lsize = 32;
		tx_rsize = 32;
		clock = 1536000;
		break;
	case I2S_SAMPLERATE_32000HZ:
		tx_lsize = 24;
		tx_rsize = 24;
		clock = 1536000;
		break;
	case I2S_SAMPLERATE_48000HZ:
		tx_lsize = 16;
		tx_rsize = 16;
		clock = 1536000;
		pr_debug("[bcm_i2s] %s:Sample rate set to %d @ %d\r\n",
			 __FUNCTION__, rate, clock);
		break;
	case I2S_SAMPLERATE_44100HZ:
		tx_lsize = 17;
		tx_rsize = 17;
		clock = 1500000;
		break;
	case I2S_SAMPLERATE_96000HZ:
		tx_lsize = 64;
		tx_rsize = 64;
		clock = 12000000;
		break;
	}
	value = get_register(I2S_DADDR_REG);
	if (direction == PLAYBACK) {
		value |= 0x7 << I2S_TX_DATA_DEPT_H_MOD;
		set_register(tx_lsize, I2S_DATLCSR_REG);
		set_register(tx_rsize, I2S_DATRCSR_REG);
	} else {
		value |= 0x7 << I2S_RX_DATA_DEPT_H_MOD;
		set_register(tx_lsize, I2S_DARLCSR_REG);
		set_register(tx_rsize, I2S_DARRCSR_REG);
	}

	set_register(value, I2S_DADDR_REG);
	clk_set_rate(bcm_i2s.i2s_clk_int, clock);
	pr_debug("[bcm_i2s] %s: Sample rate set to %d\r\n", __FUNCTION__, rate);
#endif
	bcm_i2s.sample_rate_div = rate;
	return 0;
}

/**
* Configure Master/Slave mode for playback
*
* @param handle			i2S device Instance
* @param interface_mode	Master/Slave mode for playback
* @param channels		Number of channels in audio stream
*
* @return 				0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_interface_mode(struct i2s_device *handle,
				    unsigned int interface_mode,
				    unsigned int channels)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DAI2S_REG);

	if (interface_mode)
		value &= ~(ENABLE_FIELD << I2S_INTERFACE_MODE);
	else
		value |= (ENABLE_FIELD << I2S_INTERFACE_MODE);

	if (channels)
		value &= ~(1 << I2S_PLAYBACK_MODE);
	else
		value |= (channels << I2S_PLAYBACK_MODE);
	set_register(value, I2S_DAI2S_REG);
	bcm_i2s.tx_op_mode = interface_mode;

	pr_debug("bcm_i2s_interface_mode: i2S to %s mode \n",
		 interface_mode ? "master" : "slave");
	return 0;
}

/**
* Enable I2S TX/RX interface for transmission or receive
*
* @param handle		i2S device Instance
* @param play		Enable Playback
* @param rec		Enable Capture
* @param sample_dir	Order of samples in FIFO
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_interface_enable(struct i2s_device *handle, bool play,
				      bool rec, int sample_dir)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	if (play) {
		value = get_register(I2S_DAI2S_REG);
		value |= (ENABLE_FIELD << I2S_TXEN) |
		    (sample_dir << I2S_TX_START_RIGHT);
		set_register(value, I2S_DAI2S_REG);
		pr_debug("[bcm_i2s] %s: enable TX interface\r\n", __FUNCTION__);
		bcm_i2s.tx_active = 1;
	}
	if (rec) {

		value = get_register(I2S_DAI2SRX_REG);
		value |= (ENABLE_FIELD << I2S_TXLRC4RX);
		set_register(value, I2S_DAI2SRX_REG);
		value = get_register(I2S_DAI2S_REG);
		value |= (ENABLE_FIELD << I2S_RXEN);
		set_register(value, I2S_DAI2S_REG);
		pr_debug("[bcm_i2s] %s: enable RX interface\r\n", __FUNCTION__);
		bcm_i2s.rx_active = 1;
	}
	return 0;
}

/**
* Disable transmission/receive on I2S interface
*
* @param handle		i2S device Instance
* @param play		Disable Playback
* @param rec		Disable Capture
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_interface_disable(struct i2s_device *handle, bool play,
				       bool rec)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	if (play) {
		value = get_register(I2S_DAI2S_REG);
		value &=
		    ~((ENABLE_FIELD << I2S_TXEN) &
		      (ENABLE_FIELD << I2S_TX_START_RIGHT));
		set_register(value, I2S_DAI2S_REG);
		pr_debug("[bcm_i2s] %s: disable TX interface\r\n",
			 __FUNCTION__);
	}
	if (rec) {
		value = get_register(I2S_DAI2S_REG);
		value &= ~(ENABLE_FIELD << I2S_TXEN);
		set_register(value, I2S_DAI2S_REG);

		value = get_register(I2S_DAI2SRX_REG);
		value &= (ENABLE_FIELD << I2S_RX_START_RIGHT);
		set_register(value, I2S_DAI2SRX_REG);
		pr_debug("[bcm_i2s] %s: disable TX interface\r\n",
			 __FUNCTION__);
	}
	return 0;
}

/**
* Enable/Disable Loopback mode of I2S. Used only for debugging
*
* @param handle		i2S device Instance
* @param en_dis		Enable or disable Loopback
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_loopback_cfg(struct i2s_device *handle, bool en_dis)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DAI2SRX_REG);
	if (en_dis) {
		value |= (en_dis << I2S_LOOP_EN);
		set_register(value, I2S_DAI2SRX_REG);
		pr_debug("[bcm_i2s] %s: enable Loopback\r\n", __FUNCTION__);
	} else {
		value |= (en_dis << I2S_LOOP_EN);
		set_register(value, I2S_DAI2SRX_REG);
		pr_debug("[bcm_i2s] %s: disable Loopback\r\n", __FUNCTION__);
	}
	return 0;
}

/**
* Configure Mono Record Mode
*
* @param handle		i2S device Instance
* @param channel	Specify Mono Record Channel
* @param rec_mode	Number of channels in audio stream
* @param avg_mode	Averaging or Normal mode for recording
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_setup_mono_record(struct i2s_device *handle, int channel,
				       int rec_mode, int avg_mode)
{
	unsigned int value = 0;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	if (rec_mode) {
		value &= ~(rec_mode << I2S_RECORD_CHANNELS);
	} else {
		value = get_register(I2S_DAI2SRX_REG);
		value |= (channel << I2S_CHANNEL_MODE) |
		    (rec_mode << I2S_RECORD_CHANNELS) | (avg_mode <<
							 I2S_MONO_AVG_REC);
	}
	set_register(value, I2S_DAI2SRX_REG);
	pr_debug("[bcm_i2s] %s: mode is %d\r\n", __FUNCTION__, rec_mode);
	bcm_i2s.rx_op_mode = avg_mode;
	return 0;
}

/**
* Use i2S in DMA Mode
*
* @param handle		i2S device Instance
* @param direction	Direction(Playback/Capture)
* @param threshold	FIFO threshold levels in DMA Mode
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_config_dma(struct i2s_device *handle,
				unsigned int direction, enum DMA_SIZE threshold)
{
	unsigned int value;
	if (handle == NULL)
		return -EINVAL;
	value = get_register(I2S_DADMACTRL_REG);
	if (direction == PLAYBACK) {
		value &= ~I2S_TX_DMA_SIZE_MASK;
		value |= (ENABLE_FIELD << I2S_TX_DMA_EN) |
		    (threshold << I2S_TX_DMA_SIZE);
	} else {
		value &= ~I2S_RX_DMA_SIZE_MASK;
		value |= (ENABLE_FIELD << I2S_RX_DMA_EN) |
		    (threshold << I2S_RX_DMA_SIZE);
	}
	set_register(value, I2S_DADMACTRL_REG);
	pr_debug("[bcm_i2s] %s: FIFO threshold val is %d\r\n",
		 __FUNCTION__, threshold);
	return 0;
}

/**
* Disable DMA Mode
*
* @param handle		i2S device Instance
* @param direction	Direction(Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_disable_dma(struct i2s_device *handle,
				 unsigned int direction)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DADMACTRL_REG);
	if (direction == PLAYBACK) {
		value &= ~(ENABLE_FIELD << I2S_TX_DMA_EN);
		value &= ~(I2S_TX_DMA_SIZE_MASK);
	} else {
		value &= ~(ENABLE_FIELD << I2S_RX_DMA_EN) &
		    ~(I2S_RX_DMA_SIZE_MASK);
	}
	set_register(value, I2S_DADMACTRL_REG);
	pr_debug("[bcm_i2s] %s: disable dma\r\n", __FUNCTION__);
	return 0;
}

/**
* Get current status of the DAI
*
* @param handle		i2S device Instance
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_get_channel_status(struct i2s_device *handle)
{
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	handle->channels = bcm_i2s.channels;
	handle->tx_op_mode = bcm_i2s.tx_op_mode;
	handle->rx_op_mode = bcm_i2s.rx_op_mode;
	handle->tx_active = bcm_i2s.tx_active;
	handle->sample_rate_div = bcm_i2s.sample_rate_div;
	handle->rx_active = bcm_i2s.rx_active;

	pr_debug("[bcm_i2s] %s: DASTA_REG is %x\r\n", __FUNCTION__,
		 (unsigned int)get_register(I2S_DASTA_REG));
	return get_register(I2S_DASTA_REG);
}

/**
* Issue Soft Reset to I2S DAI
*
* @param handle	 i2S device Instance
*/
void bcm_i2s_sofreset_dai(struct i2s_device *handle)
{
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return;
	}
	/*Preferably use this along with Flush FIFO */
	set_register(get_register(I2S_DATXCTRL_REG) |
		     (ENABLE_FIELD << I2S_SOFT_RESET), I2S_DATXCTRL_REG);
	set_register(get_register(I2S_DATXCTRL_REG) &
		     (~ENABLE_FIELD << I2S_SOFT_RESET), I2S_DATXCTRL_REG);
	pr_debug("[bcm_i2s] %s: Softreset DAI \r\n", __FUNCTION__);
}

/**
* Enable Internal/External clock for I2S module
*
* @param handle		i2S device Instance
* @param clk		Internal/External Clock
* @param rate		Rate of Internal Clock
*
* @return 			0 on success; -EINVAL on FAILURE
*/
int bcm_i2s_enable_clk(struct i2s_device *handle, enum CLK_SEL clk,
		       enum INT_CLK_RATE rate)
{
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	if ((clk == I2S_INT_CLK)
	    && (rate < RATE_INVALID)) {
#if (cpu_is_bcm2153() || cpu_is_bcm215xx())
		clk_set_rate(bcm_i2s.i2s_clk_int, rate);
#endif
		clk_enable(bcm_i2s.i2s_clk_int);
	} else if (clk == I2S_EXT_CLK) {
		bcm_i2s.i2s_clk_ext = clk_get(NULL, "i2s_ext");
		clk_enable(bcm_i2s.i2s_clk_ext);
	}
	pr_debug("[bcm_i2s] %s: done \r\n", __FUNCTION__);
	return 0;
}

/**
*  Disable Internal/External clock for I2S module
*
* @param handle	i2S device Instance
* @param clk	Internal/External Clock
*
* @return		0 on success; -EINVAL on FAILURE
*/
int bcm_i2s_disable_clk(struct i2s_device *handle, enum CLK_SEL clk)
{
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}

	if (clk == I2S_INT_CLK) {
		clk_disable(bcm_i2s.i2s_clk_int);
	} else if (clk == I2S_EXT_CLK) {
		clk_disable(bcm_i2s.i2s_clk_ext);
	}
	pr_debug("[bcm_i2s] %s:\r\n", __FUNCTION__);
	return 0;
}

#if (!cpu_is_bcm2153())
/**
* Configure pad DAOLR when it's not in use
*
* @param handle	 	i2S device Instance
* @param lr_mode	DAOLR_CTRL value
* @param ctrl		DAOD_CTRL value
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_config_data_out(struct i2s_device *handle,
				     unsigned int lr_mode, unsigned int ctrl)
{
	unsigned int value;

	if (handle == NULL || (lr_mode > DAOLR_UP)) {
		pr_debug("[bcm_i2s] %s: bad param! \r\n", __FUNCTION__);
		return -EINVAL;
	}

	value = get_register(I2S_DAI2S_REG);
	value &= ~I2S_DAOD_CTRL_MASK;
	value |= (ctrl << I2S_DAOD_CTRL) | (lr_mode << I2S_DAOLR_CTRL);
	set_register(value, I2S_DAI2S_REG);

	pr_debug("[bcm_i2s] %s: DAOLR_CTRL is %d, DAOD_CTRL is %d \r\n",
		 __FUNCTION__, lr_mode, ctrl);
	return 0;
}

/**
* Configure pad DAILR when it's not in use
*
* @param handle		i2S device Instance
* @param lr_mode	DAILR_CTRL value
* @param ctrl		DAID_CTRL value
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_config_data_in(struct i2s_device *handle,
				    unsigned int lr_mode, unsigned int ctrl)
{
	unsigned int value;

	if (lr_mode > DAOLR_UP || ctrl > DAID_UP || handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad param! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DAI2SRX_REG);
	value &= ~I2S_DAID_CTRL_MASK;
	value |= (ctrl << I2S_DAID_CTRL) | (lr_mode << I2S_DAILR_CTRL);
	set_register(value, I2S_DAI2SRX_REG);
	pr_debug("[bcm_i2s] %s: DAILR_CTRL is %d, DAID_CTRL is %d \r\n",
		 __FUNCTION__, lr_mode, ctrl);
	return 0;
}

/**
* 	configure Tx/Rx channel size
*
* @param handle	 	i2S device Instance
* @param direction	Direction(Playback/Capture)
* @param size		Channel size
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_txrx_chnl_size(struct i2s_device *handle,
				    unsigned int direction, unsigned int size)
{
	unsigned int value = 0;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}

	if (direction == PLAYBACK) {
		value =
		    ((size & (I2S_16BIT_PACK_MASK)) <<
		     I2S_TX_RX_LR_CHANNEL_SIZE);
		set_register(value, I2S_DATLCSR_REG);
		set_register(value, I2S_DATRCSR_REG);
		pr_debug("[bcm_i2s] %s: TX chnl size is %d, \r\n",
			 __FUNCTION__, size);

	} else {
		value =
		    ((size & (I2S_16BIT_PACK_MASK)) <<
		     I2S_TX_RX_LR_CHANNEL_SIZE);
		set_register(value, I2S_DARLCSR_REG);
		set_register(value, I2S_DARRCSR_REG);
		pr_debug("[bcm_i2s] %s: RX chnl size is %d, \r\n",
			 __FUNCTION__, size);
	}
	return 0;
}

/**
* Configure Audio Data Depth
*
* @param handle		i2S device Instance
* @param direction	Direction(Playback/Capture)
* @param data_depth	Set data depth
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_data_depth_mod(struct i2s_device *handle,
				    unsigned int direction, int data_depth)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DADDR_REG);

	if (direction == PLAYBACK) {

		value &= ~(I2S_TX_DATA_DEPT_H_MOD_MASK);
		value |= data_depth << I2S_TX_DATA_DEPT_H_MOD;
		set_register(data_depth, I2S_DADDR_REG);
		pr_debug("[bcm_i2s] %s: TX data depth is %d, \r\n",
			 __FUNCTION__, data_depth);
	} else {
		value &= ~(I2S_RX_DATA_DEPT_H_MOD_MASK);
		value |= data_depth << I2S_RX_DATA_DEPT_H_MOD;
		set_register(data_depth, I2S_DADDR_REG);
		pr_debug("[bcm_i2s] %s: RX data depth is %d, \r\n",
			 __FUNCTION__, data_depth);
	}
	return 0;
}

/**
* Configure the Audio Data Depth Value
*
* @param handle			i2S device Instance
* @param direction		Direction(Playback/Capture)
* @param custom_depth	Set custom data depth
*
* @return 				0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_data_depth_cfg(struct i2s_device *handle,
				    unsigned int direction, int custom_depth)
{
	if (handle == NULL || !(get_register(I2S_DADDR_REG) & 0x7)) {
		pr_debug("[bcm_i2s] %s: bad param \r\n", __FUNCTION__);
		return -EINVAL;
	}
	if (direction == PLAYBACK) {
		set_register(custom_depth, I2S_DATXDDVR_REG);
		pr_debug("[bcm_i2s] %s: TX data depth is %d, \r\n",
			 __FUNCTION__, custom_depth);
	} else {
		set_register(custom_depth, I2S_DARXDDVR_REG);
		pr_debug("[bcm_i2s] %s: RX data depth is %d, \r\n",
			 __FUNCTION__, custom_depth);
	}
	return 0;
}

/**
* Enable MSB alignment of TX/RX FIFO data
*
* @param handle		i2S device Instance
* @param direction	Direction(Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_data_align_enable(struct i2s_device *handle,
				       unsigned int direction)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DADDR_REG);
	if (direction == PLAYBACK) {
		value |= (ENABLE_FIELD << I2S_TX_DATA_MSB_ALIGN);
		pr_debug("[bcm_i2s] %s: Enable TX MSB align of data\r\n",
			 __FUNCTION__);
	} else {
		value |= (ENABLE_FIELD << I2S_RX_DATA_MSB_ALIGN);
		pr_debug("[bcm_i2s] %s: Disable RX MSB align of data\r\n",
			 __FUNCTION__);
	}
	set_register(value, I2S_DADDR_REG);
	return 0;
}

/**
* Disable MSB alignment of TX/RX FIFO data
*
* @param handle		i2S device Instance
* @param direction	Direction(Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_data_align_disable(struct i2s_device *handle,
					unsigned int direction)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DADDR_REG);
	if (direction == PLAYBACK) {
		value &= ~(ENABLE_FIELD << I2S_TX_DATA_MSB_ALIGN);
		pr_debug("[bcm_i2s] %s: Disable TX MSB align of data\r\n",
			 __FUNCTION__);
	} else {
		value &= ~(ENABLE_FIELD << I2S_RX_DATA_MSB_ALIGN);
		pr_debug("[bcm_i2s] %s: Disable RX MSB align of data\r\n",
			 __FUNCTION__);
	}
	set_register(value, I2S_DADDR_REG);
	return 0;
}

/**
* Configure Justified I2S mode
*
* @param handle		i2S device Instance
* @param direction	Direction(Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_just_mode_en(struct i2s_device *handle,
				  unsigned int direction)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DADDR_REG);
	if (direction == PLAYBACK) {
		value |= (ENABLE_FIELD << I2S_TX_JUSTISFIED_MOD);
		pr_debug("[bcm_i2s] %s: Enable TX Justified I2S mode\r\n",
			 __FUNCTION__);
	} else {
		value |= (ENABLE_FIELD << I2S_RX_JUSTISFIED_MOD);
		pr_debug("[bcm_i2s] %s: Enable RX Justified I2S mode\r\n",
			 __FUNCTION__);
	}
	set_register(value, I2S_DADDR_REG);
	return 0;
}

/**
* Configure normal I2S mode
*
* @param handle		i2S device Instance
* @param direction	Direction(Playback/Capture)
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_just_mode_dis(struct i2s_device *handle,
				   unsigned int direction)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DADDR_REG);
	if (direction == PLAYBACK) {
		value &= ~(ENABLE_FIELD << I2S_TX_JUSTISFIED_MOD);
		pr_debug("[bcm_i2s] %s: config TX normal I2S mode\r\n",
			 __FUNCTION__);
	} else {
		value &= ~(ENABLE_FIELD << I2S_RX_JUSTISFIED_MOD);
		pr_debug("[bcm_i2s] %s: config RX normal I2S mode\r\n",
			 __FUNCTION__);
	}
	set_register(value, I2S_DADDR_REG);
	return 0;
}

/**
* Configure Audio SCLK STOP Register
*
* @param handle			i2S device Instance
* @param sck_run_cnt 	No. of cycle SCK will run AFTER channel started
* @param prerun_cnt		No. of cycle that SCK will run BEFORE channel started.
*
* @return 				0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_sclk_stop(struct i2s_device *handle, int sck_run_cnt,
			       int prerun_cnt)
{
	unsigned int value;
	if ((prerun_cnt <= 1) || (handle == NULL)) {
		pr_debug("[bcm_i2s] %s: bad param/handle \r\n", __FUNCTION__);
		return -EINVAL;
	} else {
		value = get_register(I2S_DASCKSTOP_REG);
		value &= ~(SCK_RUN_CNT_MASK);
		value |= (prerun_cnt << SCK_PRERUN_CNT);
		/* Make sure SCK Stop in DAI2S is also set. Else configuring here is meaningless */
		value &= ~(SCK_PRERUN_CNT_MASK);
		value |= (sck_run_cnt << SCK_RUN_CNT);
		set_register(value, I2S_DASCKSTOP_REG);
		pr_debug("[bcm_i2s] %s: \r\n", __FUNCTION__);
	}
	return 0;
}

/**
* Enable spdif mode
*
* @param handle		i2S device Instance
* @param spdif_ch	Set SPDIF Preambles Channel Code
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_spdif_enable(struct i2s_device *handle, int spdif_ch)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DASPDIF_REG);
	value |= (spdif_ch << I2S_SPDIF_CH_CODE);

	set_register(value, I2S_DASPDIF_REG);
	pr_debug("[bcm_i2s] %s: \r\n", __FUNCTION__);
	return 0;
}

/**
* Set SPDIF mode PREAMBLE MAP
*
* @param handle		i2S device Instance
* @param preamble	Preable map
*
* @return 			0 on success; -EINVAL on FAILURE
*/
unsigned int bcm_i2s_spdif_set_mode(struct i2s_device *handle,
				    enum SPDIF_PREAMBLE preamble)
{
	unsigned int value;
	if (handle == NULL) {
		pr_debug("[bcm_i2s] %s: bad handle! \r\n", __FUNCTION__);
		return -EINVAL;
	}
	value = get_register(I2S_DASPDIF_REG);

	value |= (preamble << I2S_SPDIF_MODE);
	set_register(value, I2S_DASPDIF_REG);
	pr_debug("[bcm_i2s] %s: \r\n", __FUNCTION__);
	return 0;
}
#endif

/**
* Get base addr of I2S block
*
* @return  I2S base addr
*/
void __iomem * bcm_i2s_get_base_addr(void)
{
	return i2s_base_addr;
}

/**
 * Probe function for I2S DAI
 */
static int __devinit bcm_i2s_probe(struct platform_device *pdev)
{
	int ret = -1;
	struct resource *res;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"can't get platform resource -  bcm_i2s_\n");
		ret = -ENOMEM;
		goto err;
	}
	i2s_base_addr = (void __iomem *)res->start;
	bcm_i2s.i2s_clk_int = clk_get(NULL, "i2s_int");
	spin_lock_init(&bcm_i2s.i2s_lock);
	pr_info("bcm_i2s %d module loaded successfully\n", pdev->id + 1);
	return 0;
err:
	return -1;
}

/**
 * Remove function for I2S DAI
 */
static int __devexit bcm_i2s_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver bcm_i2s_driver = {
	.probe = bcm_i2s_probe,
	.remove = bcm_i2s_remove,
	.driver = {
		   .name = "bcm_i2sdai",
		   },
};

/**
 *  I2S DAI module init routine
 */
static int __init bcm_i2s_init(void)
{
	return platform_driver_register(&bcm_i2s_driver);
}

/**
 *  I2S DAI module exit routine
 */
static void __exit bcm_i2s_exit(void)
{
	platform_driver_unregister(&bcm_i2s_driver);
	pr_info("bcm_i2s module unloaded successfully\n");
}

/** @} */

/* Exports */
EXPORT_SYMBOL(use_i2s_device);
EXPORT_SYMBOL(release_i2s_device);
EXPORT_SYMBOL(bcm_i2s_cfg_play_rec_interrupt);
EXPORT_SYMBOL(bcm_i2s_get_interrupt_status);
EXPORT_SYMBOL(bcm_i2s_fifo_empty_int_cfg);
EXPORT_SYMBOL(bcm_i2s_txrx_fifo_enable);
EXPORT_SYMBOL(bcm_i2s_txrx_fifo_disable);
EXPORT_SYMBOL(bcm_i2s_flush_fifo);
EXPORT_SYMBOL(bcm_i2s_sample_rate_div);
EXPORT_SYMBOL(bcm_i2s_interface_mode);
EXPORT_SYMBOL(bcm_i2s_interface_enable);
EXPORT_SYMBOL(bcm_i2s_interface_disable);
EXPORT_SYMBOL(bcm_i2s_loopback_cfg);
EXPORT_SYMBOL(bcm_i2s_setup_mono_record);
EXPORT_SYMBOL(bcm_i2s_config_dma);
EXPORT_SYMBOL(bcm_i2s_disable_dma);
EXPORT_SYMBOL(bcm_i2s_get_channel_status);
EXPORT_SYMBOL(bcm_i2s_sofreset_dai);
EXPORT_SYMBOL(bcm_i2s_enable_clk);
EXPORT_SYMBOL(bcm_i2s_disable_clk);
EXPORT_SYMBOL(bcm_i2s_get_base_addr);

module_init(bcm_i2s_init);
module_exit(bcm_i2s_exit);
