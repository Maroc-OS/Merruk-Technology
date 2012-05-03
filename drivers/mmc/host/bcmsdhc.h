/*
 *  linux/drivers/mmc/host/sdhci.h - Secure Digital Host Controller Interface driver
 *
 *  Copyright (C) 2005-2008 Pierre Ossman, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/mmc/host/bcmsdhc.h
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

#include <linux/scatterlist.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
/*
 * Controller registers
 */

/* Standard host controller registers */
#define SDHC_SYSTEMADDRESS_LO         0x00
#define SDHC_SYSTEMADDRESS_HI         0x02
#define SDHC_BLOCKSIZE                0x04
#define SDHC_BLOCKCOUNT               0x06
#define SDHC_ARGUMENT_0               0x08
#define SDHC_ARGUMENT_1               0x0A
#define SDHC_TRANSFERMODE             0x0C
#define SDHC_COMMAND                  0x0E
#define SDHC_R0                       0x10
#define SDHC_R1                       0x12
#define SDHC_R2                       0x14
#define SDHC_R3                       0x16
#define SDHC_R4                       0x18
#define SDHC_R5                       0x1A
#define SDHC_R6                       0x1C
#define SDHC_R7                       0x1E
#define SDHC_BUFFER_DATA_PORT_0       0x20
#define SDHC_BUFFER_DATA_PORT_1       0x22
#define SDHC_PRESENT_STATE            0x24
#define SDHC_HOST_CONTROL             0x28
#define SDHC_POWER_CONTROL            0x29
#define SDHC_BLOCK_GAP_CONTROL        0x2A
#define SDHC_WAKEUP_CONTROL           0x2B
#define SDHC_CLOCK_CONTROL            0x2C
#define SDHC_TIMEOUT_CONTROL          0x2E
#define SDHC_SOFT_RESET               0x2F
#define SDHC_NORMAL_INT_STATUS        0x30
#define SDHC_ERROR_INT_STATUS         0x32
#define SDHC_NORMAL_INT_STATUS_ENABLE 0x34
#define SDHC_ERROR_INT_STATUS_ENABLE  0x36
#define SDHC_NORMAL_INT_SIGNAL_ENABLE 0x38
#define SDHC_ERROR_INT_SIGNAL_ENABLE  0x3A
#define SDHC_AUTOCMD12_ERROR_STATUS   0x3C
#define SDHC_Reserved1                0x3E
#define SDHC_CAPABILITIES             0x40
#define SDHC_Reserved2                0x44
#define SDHC_MAX_CURRENT_CAPABILITIES 0x48
#define SDHC_Reserved3                0x4C
/* #define   SDHC_Reserved4[43]       50H */
#define SDHC_ADMA_ERR_STAT            0x54
#define SDHC_ADMA_ADDRESS             0x58
#define SDHC_SLOT_INT_STATUS          0xFC
#define SDHC_HOST_CONTROLLER_VER      0xFE

/* Transfer Mode Register */
#define TXN_MODE_DMA                    0x0001
#define TXN_MODE_BLOCK_COUNT_ENABLE     0x0002
#define TXN_MODE_AUTO_CMD12             0x0004
#define TXN_MODE_DATA_DIRECTION_READ    0x0010
#define TXN_MODE_MULTI_BLOCK            0x0020

/* Command Register (3.2.6) */
#define CMD_NO_RESPONSE                 0x0000
#define CMD_RESPONSE_LENGTH_136         0x0001
#define CMD_RESPONSE_LENGTH_48          0x0002
#define CMD_RESPONSE_LENGTH_48_BUSY     0x0003
#define CMD_CRC_CHECK                   0x0008
#define CMD_INDEX_CHECK                 0x0010
#define CMD_DATA_PRESENT                0x0020
#define CMD_TYPE_NORMAL                 0x0000
#define CMD_TYPE_RESUME                 0x0040
#define CMD_TYPE_SUSPEND                0x0080
#define CMD_TYPE_ABORT                  0x00c0

#define CMD_INDEX_MASK                  0x3F00
#define CMD_INDEX_SHIFT                 8
#define CMD_RESPONSE_R2                 (CMD_RESPONSE_LENGTH_136 | CMD_CRC_CHECK)
#define CMD_RESPONSE_R3_R4              CMD_RESPONSE_LENGTH_48
#define CMD_RESPONSE_R1_R5_R6_R7           (CMD_RESPONSE_LENGTH_48 | CMD_CRC_CHECK | CMD_INDEX_CHECK)
#define CMD_RESPONSE_R1B_R5B            (CMD_RESPONSE_LENGTH_48_BUSY | CMD_CRC_CHECK | CMD_INDEX_CHECK)

/* Present State Register (3.2.9) */
#define STATE_CMD_INHIBIT               0x00000001
#define STATE_DAT_INHIBIT               0x00000002
#define STATE_DAT_ACTIVE                0x00000004
#define STATE_WRITE_ACTIVE              0x00000100
#define STATE_READ_ACTIVE               0x00000200
#define STATE_BUF_WRITE_ENABLE          0x00000400
#define STATE_BUF_READ_ENABLE           0x00000800
#define STATE_CARD_INSERTED             0x00010000
#define STATE_CARD_STATE_STABLE         0x00020000
#define STATE_CARD_DETECT_SIGNAL        0x00040000
#define STATE_WRITE_PROTECT             0x00080000
#define STATE_DAT_LINE_SIGNAL           0x00F00000
#define STATE_CMD_LINE_SIGNAL           0x01000000

/* Host Control Register (3.2.10) */
#define HOSTCTL_LED_CONTROL             0x01
#define HOSTCTL_DAT_4BIT_WIDTH          0x02
#define HOSTCTL_HIGH_SPEED              0x04
#define HOSTCTL_CTRL_DMA_MASK	        0x18
#define HOSTCTL_CTRL_SDMA	            0x00
#define HOSTCTL_CTRL_ADMA1         	0x08
#define HOSTCTL_CTRL_ADMA32   	        0x10
#define HOSTCTL_CTRL_ADMA64	            0x18

/* Power Control Register (3.2.11) */
#define SDBUS_POWER_ON                  0x01
#define SDBUS_VOLTAGE_SELECT_3_3V       0x0E
#define SDBUS_VOLTAGE_SELECT_3_0V       0x0C
#define SDBUS_VOLTAGE_SELECT_1_8V       0x0A

/* Wakeup Control Register (3.2.13) */
#define WAKEUP_INTERRUPT                0x01
#define WAKEUP_INSERTION                0x02
#define WAKEUP_REMOVAL                  0x04

#define WAKEUP_ALL_SOURCES              (WAKEUP_INTERRUPT | WAKEUP_INSERTION | WAKEUP_REMOVAL)

/* Clock Control Register (3.2.14) */
#define CLOCK_INTERNAL_ENABLE           0x0001
#define CLOCK_STABLE                    0x0002
#define CLOCK_ENABLE                    0x0004
#define  SDHCI_DIVIDER_SHIFT	8

/* Timeout Control Register (3.2.15) */
#define TIME_DATA_TIMEOUT_MASK          0x000F

/* Soft Reset Register  (3.2.16) */
#define SOFT_RESET_ALL                  0x0001
#define SOFT_RESET_CMD                  0x0002
#define SOFT_RESET_DAT                  0x0004

/* Normal Interrupt status register (3.2.17) */
#define NORMAL_INT_STATUS_CMD_COMPLETE   0x0001
#define NORMAL_INT_STATUS_TRX_COMPLETE   0x0002
#define NORMAL_INT_STATUS_BLOCK_GAP      0x0004
#define NORMAL_INT_STATUS_DMA            0x0008
#define NORMAL_INT_STATUS_BUF_WRITE_RDY  0x0010
#define NORMAL_INT_STATUS_BUF_READ_RDY   0x0020
#define NORMAL_INT_STATUS_CARD_INSERTION 0x0040
#define NORMAL_INT_STATUS_CARD_REMOVAL   0x0080
#define NORMAL_INT_STATUS_CARD_INT       0x0100
#define NORMAL_INT_STATUS_ERROR_INT      0x8000

/* Error Interrupt Status Register (3.2.18) */
#define ERR_INT_STATUS_CMD_TIMEOUT       0x0001
#define ERR_INT_STATUS_CMD_CRC           0x0002
#define ERR_INT_STATUS_CMD_CONFLICT      (ERR_INT_STATUS_CMD_TIMEOUT | ERR_INT_STATUS_CMD_CRC_ERR)
#define ERR_INT_STATUS_CMD_ENDBIT        0x0004
#define ERR_INT_STATUS_CMD_INDEX         0x0008
#define ERR_INT_STATUS_DAT_TIMEOUT       0x0010
#define ERR_INT_STATUS_DAT_CRC           0x0020
#define ERR_INT_STATUS_DAT_ENDBIT        0x0040
#define ERR_INT_STATUS_BUS_POWER         0x0080
#define ERR_INT_STATUS_AUTOCMD12         0x0100
#define ERR_INT_STATUS_ADMAERR        	 0x0200
#define ERR_INT_STATUS_VENDOR            0xF000

#define IS_CMD_LINE_ERROR(x)    (((x) & 0x0F) != 0)
#define IS_DAT_LINE_ERROR(x)    (((x) & 0x70) != 0)

#define ERR_INT_STATUS_CMD_MASK		(ERR_INT_STATUS_CMD_TIMEOUT | \
									 ERR_INT_STATUS_CMD_CRC | \
									 ERR_INT_STATUS_CMD_ENDBIT | \
									 ERR_INT_STATUS_CMD_INDEX)

#define ERR_INT_STATUS_DAT_MASK		(ERR_INT_STATUS_DAT_TIMEOUT | \
									 ERR_INT_STATUS_DAT_CRC | \
									 ERR_INT_STATUS_DAT_ENDBIT | \
									 ERR_INT_STATUS_BUS_POWER | \
									 ERR_INT_STATUS_AUTOCMD12 | \
									 ERR_INT_STATUS_ADMAERR)

/* Normal interrupt status enable register (3.2.19) */
#define NORMAL_INT_ENABLE_CMD_COMPLETE   0x0001
#define NORMAL_INT_ENABLE_TRX_COMPLETE   0x0002
#define NORMAL_INT_ENABLE_BLOCK_GAP      0x0004
#define NORMAL_INT_ENABLE_DMA            0x0008
#define NORMAL_INT_ENABLE_BUF_WRITE_RDY  0x0010
#define NORMAL_INT_ENABLE_BUF_READ_RDY   0x0020
#define NORMAL_INT_ENABLE_CARD_INSERTION 0x0040
#define NORMAL_INT_ENABLE_CARD_REMOVAL   0x0080
#define NORMAL_INT_ENABLE_CARD_INT       0x0100

//add 
#define NORMAL_INT_ENABLE_BTACKRXEN       	0x0200
#define NORMAL_INT_ENABLE_BTIRQ				0x0400


/* Error Interrupt status enable register (3.2.20) */
#define ERR_INT_ENABLE_CMD_TIMEOUT       0x0001
#define ERR_INT_ENABLE_CMD_CRC           0x0002
#define ERR_INT_ENABLE_CMD_ENDBIT        0x0004
#define ERR_INT_ENABLE_CMD_INDEX         0x0008
#define ERR_INT_ENABLE_DAT_TIMEOUT       0x0010
#define ERR_INT_ENABLE_DAT_CRC           0x0020
#define ERR_INT_ENABLE_DAT_ENDBIT        0x0040
#define ERR_INT_ENABLE_BUS_POWER         0x0080
#define ERR_INT_ENABLE_AUTOCMD12         0x0100

/* Normal interrupt signal enable register (3.2.21) */
#define NORMAL_INT_SIGNAL_CMD_COMPLETE   0x0001
#define NORMAL_INT_SIGNAL_TRX_COMPLETE   0x0002
#define NORMAL_INT_SIGNAL_BLOCK_GAP      0x0004
#define NORMAL_INT_SIGNAL_DMA            0x0008
#define NORMAL_INT_SIGNAL_BUF_WRITE_RDY  0x0010
#define NORMAL_INT_SIGNAL_BUF_READ_RDY   0x0020
#define NORMAL_INT_SIGNAL_CARD_INSERTION 0x0040
#define NORMAL_INT_SIGNAL_CARD_REMOVAL   0x0080
#define NORMAL_INT_SIGNAL_CARD_INT       0x0100

/* Error Interrupt signal enable register (3.2.22) */
#define ERR_INT_SIGNAL_CMD_TIMEOUT       0x0001
#define ERR_INT_SIGNAL_CMD_CRC           0x0002
#define ERR_INT_SIGNAL_CMD_ENDBIT        0x0004
#define ERR_INT_SIGNAL_CMD_INDEX         0x0008
#define ERR_INT_SIGNAL_DAT_TIMEOUT       0x0010
#define ERR_INT_SIGNAL_DAT_CRC           0x0020
#define ERR_INT_SIGNAL_DAT_ENDBIT        0x0040
#define ERR_INT_SIGNAL_BUS_POWER         0x0080
#define ERR_INT_SIGNAL_AUTOCMD12         0x0100

#define CMD_RESPONSE_R2                 (CMD_RESPONSE_LENGTH_136 | CMD_CRC_CHECK)
#define CMD_RESPONSE_R3_R4              CMD_RESPONSE_LENGTH_48
#define CMD_RESPONSE_R1_R5_R6_R7           (CMD_RESPONSE_LENGTH_48 | CMD_CRC_CHECK | CMD_INDEX_CHECK)
#define CMD_RESPONSE_R1B_R5B            (CMD_RESPONSE_LENGTH_48_BUSY | CMD_CRC_CHECK | CMD_INDEX_CHECK)

#define MMC_RSP_R1_R5_R6_R7	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R3_R4	(MMC_RSP_PRESENT)

/* macros to split the RW_Direct argument up again */
#define IO_RW_DIRECT_ARG_RW(Arg)    (((Arg)>>31)&1)
#define IO_RW_DIRECT_ARG_RAW(Arg)   (((Arg)>>27)&1)
#define IO_RW_DIRECT_ARG_FUNC(Arg)  ((UCHAR)(((Arg)>>28)&0x7))
#define IO_RW_DIRECT_ARG_ADDR(Arg)  (((Arg)>>9)&0x1FFFF)
#define IO_RW_DIRECT_ARG_DATA(Arg)  ((UCHAR)((Arg)&0xFF))

#define SD_IO_READ   0
#define SD_IO_WRITE  1

/* bit definition for Interrupt enable register */
#define SD_IO_INT_ENABLE_MASTER_ENABLE       (1 << 0)
#define SD_IO_INT_ENABLE_ALL_FUNCTIONS       0xFE

/* Capabilities Register */
#define  SDHC_TIMEOUT_CLK_MASK	0x0000003F
#define  SDHC_TIMEOUT_CLK_SHIFT 0
#define  SDHC_TIMEOUT_CLK_UNIT	0x00000080
#define  SDHC_CLOCK_BASE_MASK	0x00003F00
#define  SDHC_CLOCK_BASE_SHIFT	8
#define  SDHC_MAX_BLOCK_MASK	0x00030000
#define  SDHC_MAX_BLOCK_SHIFT  16
#define  SDHC_CAN_DO_ADMA2	0x00080000
#define  SDHC_CAN_DO_ADMA1	0x00100000
#define  SDHC_CAN_DO_HISPD	0x00200000
#define  SDHC_CAN_DO_DMA	0x00400000
#define  SDHC_CAN_VDD_330	0x01000000
#define  SDHC_CAN_VDD_300	0x02000000
#define  SDHC_CAN_VDD_180	0x04000000
#define  SDHC_CAN_64BIT	0x10000000

/* SD HOST Controller Version */
#define  SDHC_SPEC_VER_MASK	            0x00FF
#define  SDHC_SPEC_VER_SHIFT	        0
#define  SDHC_SPEC_100                  0x0
#define  SDHC_SPEC_200	                 0x1
#define  SDHC_SPEC_201	                 0x2

#define  SDHCI_MAKE_BLKSZ(dma, blksz) (((dma & 0x7) << 12) | (blksz & 0xFFF))
/* struct bcmsdhc_ops; */

struct bcmsdhc_host {
	/* Data set by hardware interface driver */
	/* const char    *hw_name;  Hardware bus name */
	int irq;		/* Device IRQ */
	int irq_cd;
	void __iomem *ioaddr;	/* Mapped address */

	/* const struct bcmsdhc_ops      *ops; Low level hw interface */

	/* Internal data */
	struct mmc_host *mmc;	/* MMC structure */
	u64 dma_mask;		/* custom DMA mask */

#ifdef CONFIG_LEDS_CLASS
	struct led_classdev led;	/* LED control */
#endif

	spinlock_t lock;	/* Mutex */

	int flags;		/* Host attributes */
#define SDHCI_USE_DMA		(1<<0)	/* Host is DMA capable */
#define SDHCI_USE_ADMA		(1<<1)	/* Host is ADMA capable */
#define SDHCI_REQ_USE_DMA	(1<<2)	/* Use DMA for this req. */
#define SD_AUTO_ISSUE_CMD12	(1<<3)	/* Use Auto CMD12 */

	unsigned int version;	/* SDHCI spec. version */
	unsigned int max_clk;	/* Max possible freq (MHz) */
	unsigned int timeout_clk;	/* Timeout freq (KHz) */

	unsigned int max_block;	/* Max block size (bytes) */

	unsigned int clock;	/* Current clock (MHz) */
	unsigned short power;	/* Current voltage */

	struct mmc_request *mrq;	/* Current request */
	struct mmc_command *cmd;	/* Current command */
	struct mmc_data *data;	/* Current data request */
	unsigned int data_early:1;	/* Data finished before cmd */
	unsigned int command_complete;	/* Data finished before cmd */
	bool m_fAutoCMD12Success;	/* Auto CMD 12 enable */
	bool enable_sdio_interrupt;	/* Current SDIO interrupt state */
	bool bus_4bit_mode;	/* Current Bus Width 4 bit */
	struct device *dev;

	struct sg_mapping_iter sg_miter;	/* SG state for PIO */
	unsigned int blocks;	/* remaining PIO blocks */

	int sg_count;		/* Mapped sg entries */

	u8 *adma_desc;		/* ADMA descriptor table */
	u8 *align_buffer;	/* Bounce buffer */

	dma_addr_t adma_addr;	/* Mapped ADMA descr. table */
	dma_addr_t align_addr;	/* Mapped bounce buffer */

	struct tasklet_struct card_tasklet;	/* Tasklet structures */
	struct tasklet_struct finish_tasklet;
	struct clk *bus_clk;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock sdhc_wakelock;
#endif
	u16 clk_control;	/*Keep sd host clock register value */

	u8 sdhc_slot;		/* Current host slot */
	bool card_present;
	struct bcmsdhc_platform_data *bcm_plat;

	struct timer_list timer;	/* Timer for timeouts */

	unsigned long private[0] ____cacheline_aligned;
	struct regulator	*vcc;
	struct pm_qos_request_list *sdio_driver_pm_qos_req;
};
