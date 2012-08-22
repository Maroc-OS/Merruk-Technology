/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/include/mach/hardware.h
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
 * This file contains BCM21553 hardware definitions.
 */
#ifndef __MACH_HARDWARE_H
#define __MACH_HARDWARE_H

/*  */
#define BCM21553_L210_CONFIG	0x00030012

/* TODO Few modules have confusing base + offset addresses Relook a must */

/* BCM21553 registers live from 0x0800.0000 to 0x0890.0000 -- currently */
#define BCM21553_IO_VIRTUAL	0xf0000000
#define BCM21553_IO_PHYSICAL	0x00000000

#define __REG32(x)   (*((volatile u32 *)(x)))
#define __REG16(x)   (*((volatile u16 *)(x)))
#define __REG8(x) (*((volatile u8  *)(x)))

/* used in C code, so cast to proper type */
#define io_p2v(x) (__io(x) - BCM21553_IO_PHYSICAL + BCM21553_IO_VIRTUAL)
#define io_v2p(x) (__io(x) - BCM21553_IO_VIRTUAL + BCM21553_IO_PHYSICAL)

/* used in asm code, so no casts */
#define IO_ADDRESS(x) ((x) - BCM21553_IO_PHYSICAL + BCM21553_IO_VIRTUAL)

#define BCM21553_ONENAND_BASE		0x00000000	/* ONENAND Device Ctlr */
#define BCM21553_NAND_BASE		0x02000000	/* NAND Device Ctlr */
#define BCM21553_MHARB_BASE		0x08010000	/* AHB Matrix HArbiter */
#define BCM21553_MLARB_BASE		0x08018000	/* AHB Matrix LArbiter */
#define BCM21553_DMAC_BASE		0x08020000	/* DMA Controller */
#define BCM21553_LCDC_BASE		0x08030000	/* LCD Controller */
#define BCM21553_MPHI_BASE		0x08050000	/* MPHI Co-Processor */
#define BCM21553_EDGE_MP_BASE		0x08070000	/* Edge Message Processor */
#define BCM21553_NVSRAM_BASE		0x08090000	/* Nor/NAND/pSram Controller
							   (NV_SRAM_IF) Control */
#define BCM21553_DATAPACKER_BASE	0x080A0000	/* Data Packer */
#define BCM21553_DES_ENGINE_BASE	0x080B0000	/* DES Engine */
#define BCM21553_SDIO1_BASE		0x08110000	/* SDIO 1 */
#define BCM21553_SDIO2_BASE		0x08120000	/* SDIO 2 */
#define BCM21553_CLKPWR_BASE		0x08140000	/* Clock/Power Management */
#define BCM21553_CIPHER_BASE		0x08150000	/* WCDMA Ciphering Engine */
#define BCM21553_CRC_BASE		0x08180000	/* CRC Base */
#define BCM21553_HANTRO_DEC_BASE	0x081A0000	/* Hantro Decoder */
#define BCM21553_HANTRO_ENC_BASE	0x081A8000	/* Hantro Encoder */
#define BCM21553_ECT0P0_BASE		0x081C0000	/* ECT0_Port0 */
#define BCM21553_ECT0P1_BASE		0x081C1000	/* ECT0_Port1 */
#define BCM21553_ECT1P0_BASE		0x081C2000	/* ECT1_Port0 */
#define BCM21553_ECT1P1_BASE		0x081C3000	/* ECT1_Port1 */
#define BCM21553_USB_HSOTG_BASE		0x08200000	/* USB HS OTG */
#define BCM21553_USB_HSOTGCTRL_BASE	0x08280000	/* USB HS OTGHS Control */
#define BCM21553_USB_FSHOST_BASE	0x08300000	/* USB FS Host Device */
#define BCM21553_USB_FSHOSTCTRL_BASE	0x08380000	/* USB FS Host Control */
#define BCM21553_SDIO3_BASE		0x08400000	/* SDIO 3 */
#define BCM21553_EMI_BASE		0x08420000	/* ExternalDRAM Controller */
#define BCM21553_CAMINTF_BASE		0x08440000	/* Camera Interface */
#define BCM21553_HUCM_BASE		0x08480000	/* HSUPA Cipher/Mux Module */
#define BCM21553_DISPC_BASE		0x08490000	/* TV Out TV Encoder */
#define BCM21553_DSI_BASE		0x084A0000	/* Display Serial Interface */
#define BCM21553_L210_BASE		0x084A1000	/* ARM L2 Cache Controller */
#define BCM21553_SLPTMR1_BASE		0x08800000	/* Sleep Timer 1 */
#define BCM21553_SLPTMR2_BASE		0x08800100	/* Sleep Timer 2 */
#define BCM21553_INTC_BASE		0x08810000	/* Interrupt controller */
#define BCM21553_UART_A_BASE		0x08820000	/* UART A base */
#define BCM21553_UART_B_BASE		0x08821000	/* UART B base */
#define BCM21553_UART_C_BASE		0x08822000	/* UART C base */
#define BCM21553_AUXADC_BASE		0x08830020	/* Auxiliary ADC control */
#define BCM21553_GPTIMER_BASE		0x08830100	/* General purpose timer */
#define BCM21553_BB2PMU_ADCSYNC_BASE	0x08830200	/* BB-to-PMU Aux ADC Sync */
#define BCM21553_SIM1_BASE		0x08860000	/* SIM interface 1 */
#define BCM21553_SLOWCLK_BASE		0x08870010	/* Slow clock calibration */
#define BCM21553_SYSCFG_BASE		0x08880000	/* System Configuration */
#define BCM21553_GEA_BASE		0x08890000	/* GPRS encryption */
#define BCM21553_WDT_BASE		0x088A0000	/* Watchdog timer 1 */
#define BCM21553_WDT_BASE1		0x088A0010	/* Watchdog timer 2 */
#define BCM21553_I2C1_BASE		0x088A0020	/* I2C interface 1 */
#define BCM21553_I2C2_BASE		0x088B0020	/* I2C interface 2 */
#define BCM21553_I2S_BASE		0x088C0000	/* I2S interface (aka
							   Digital Audio) */
#define BCM21553_GPIO_BASE		0x088CE000	/* GPIO interface */
#define BCM21553_KEYPAD_BASE		0x088CE080	/* Keypad control */
#define BCM21553_SPI1_BASE		0x088D0000	/* SPI interface 1 */
#define BCM21553_SOFTRST_BASE		0x088E0000	/* Soft Reset */
#define BCM21553_I2C3_BASE		0x088F0020	/* I2C interface 3 */
#define BCM21553_MSPRO_BASE		0x08900000	/* Memory Stick Pro */
#define BCM21553_AUXMIC_BASE		0x08911000	/* Auxillary Microphone */
#define BCM21553_SPI2_BASE		0x08920000	/* SPI interface 2 */
#define BCM21553_SIM2_BASE		0x08930000	/* SIM interface 2 */
#define BCM21553_PWM_TOP_BASE		0x08940000	/* Pulse Width Modulator */
#define BCM21553_V3D_BASE		0x08950000	/* 3D Graphics Engine(V3D) */
#define BCM21553_VEC_BASE		0x08B00000	/* TVOut Encoder Registers */
#define BCM21553_HTM0_BASE		0x08C00000	/* AHB Trace Modulde 0 */
#define BCM21553_HTM1_BASE		0x08C01000	/* AHB Trace Modulde 1 */
#define BCM21553_TFUNNEL_BASE		0x08C02000	/* Trace Funnel Register */
#define BCM21553_TPIU_BASE		0x08C03000	/* HTM Trace Port I/F Unit */
#define BCM21553_APPS_ETM_BASE		0x08C04000	/* Apps ARM ETM Conf */
#define BCM21553_MDM_ETM_BASE		0x08C05000	/* Modem ARM ETM Conf */
#define BCM21553_CRYPTO_BASE		0x0C080000	/* Crypto Engine */
#define BCM21553_PKI_BASE		0x0C0C8000	/* Public Key Authenticate */
#define BCM21553_OTP_BASE		0x0C0C9000	/* One-Time Programmable */
#define BCM21553_RNG_BASE		0x0C0CA000	/* Random Number Generator */
#define BCM21553_SCRATCHRAM_BASE	0x28000000	/* Scratch ram base */
#define MATRIX_HARBITER_BASE            0x08010000      /* Matrix High speed arbiter */
#define MATRIX_LARBITER_BASE            0x08018000      /* Matrix Low speed arbiter  */
#define BCM21553_AHB_ARM_DSP_BASE	0x30400000	/* ARM accessesible DSP Registers Domain*/

/* BCM21553_WCDMA_BOOT_BASE cannot use io_p2v as it conflicts with Linux
 * high vectors @0xFFFF0000
 */
#define BCM21553_WCDMA_BOOT_BASE	0xffff0000	/* WCDMA Boot Module base */

#endif /* __MACH_HARDWARE_H */
