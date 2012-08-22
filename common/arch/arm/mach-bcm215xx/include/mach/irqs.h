/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/include/mach/irqs.h
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
 * This file contains the interrupts numbers on BCM21553 SoC
 */
#ifndef ASM_ARCH_IRQS_H
#define ASM_ARCH_IRQS_H

#include <linux/version.h>
#include <mach/hardware.h>

#define ARCH_NR_GPIOS		64
#define NUM_INTERNAL_IRQS	96
#define NR_IRQS 		(NUM_INTERNAL_IRQS + ARCH_NR_GPIOS)

#define IRQ_INTC_START          0	/* first INTC interrupt is 0 */
#define IRQ_WAKETMR 		0	/* Sleep Mode Timer 1 Wake-Up */
#define IRQ_DSPIRQ 		1	/* RIP(DSP) Communications IRQ */
#define IRQ_UARTA 		2	/* UART-A */
#define IRQ_UARTB 		3	/* UART-B */
#define IRQ_SIM1IF 		4	/* SIM 1 I/F */
#define IRQ_KEYPAD 		5	/* Keypad Key Depressed */
#define IRQ_GPIO 		6	/* GPIO */
#define IRQ_I2C1 		7	/* I2C */
#define IRQ_NANDECC 		8	/* NAND ECC done/ReadyBusy */
#define IRQ_WDOG1 		9	/* Watchdog Timer */
#define IRQ_10			10	/* Reserved */
#define IRQ_GPT1 		11	/* General Purpose Time (A/B) */
#define IRQ_MICON 		12	/* MICON Headset button */
#define IRQ_I2C2 		13	/* I2C2 */
#define IRQ_MP	 		14	/* Message Processor */
#define IRQ_15 			15	/* Reserved (= Sleep Mode control) */
#define IRQ_GRAPHICS 		16	/* Graphics Engine */
#define IRQ_SIM2DP 		17	/* SIM 2 Debounced Presence */
#define IRQ_AUDIO 		18	/* External Audio DAC */
#define IRQ_DSPFIQ 		19	/* RIP(DSP) Communications FIQ */
#define IRQ_DMA 		20	/* DMA */
#define IRQ_USBHSOTG 		21	/* USB 2.0 OTG (HSOTG) */
#define IRQ_SIM2EMERG 		22	/* SIM 2 Emergency Shutdown */
#define IRQ_HSB 		23	/* MICON raw headset button status */
#define IRQ_SPI 		24	/* SPI 1 */
#define IRQ_25	 		25	/* Reserved */
#define IRQ_SDIO2 		26	/* SDIO2 */
#define IRQ_SDIO1 		27	/* SDIO1 */
#define IRQ_GEA3 		28	/* GEA3 */
#define IRQ_I2C3		29	/* I2C3 - From B0 onwards Reserved otherwise */
#define IRQ_MICIN 		30	/* MICIN */
#define IRQ_MSPRO 		31	/* MSPRO */

#define IRQ_WCDMA_L2		32	/* WCDMA layer 2 */
#define IRQ_WCDMA_IRQ		33	/* Modem ARM11 nIRQ */
#define IRQ_WCDMA_FIQ		34	/* Modem ARM11 nFIQ */
#define IRQ_CIPHER		35	/* WCDMA cipher */
#define IRQ_CRYPTO		36	/* Crypto */
#define IRQ_UART_WAKE		37	/* UART power on request to wake up */
#define IRQ_PKE			38	/* Public key encryption block */
#define IRQ_RNG			39	/* Random number generator */
#define IRQ_USBHOST_WAKE 	40	/* USB host device wake */
#define IRQ_HSUPA_CIPHER        41	/* HSUPA cipher */
#define IRQ_HANTRO_ENCODER      42	/* Video encoder */
#define IRQ_HANTRO_DECODER      43	/* Video decoder */
#define IRQ_USB_HOST		44	/* USB Host */
#define IRQ_APPS_DMA            45	/* Apps ARM11 DMA */
#define IRQ_APPS_PMU            46	/* Apps ARM11 PMU */
#define IRQ_PACKER              47	/* Data packer */
#define IRQ_SIM1DP              48	/* SIM Debounced Presence */
#define IRQ_DES                 49	/* DES */
#define IRQ_USBOTG_WAKE         50	/* USBOTG wake */
#define IRQ_HPERF_COUNTER       51	/* Bus Matrix H Performance Counter */
#define IRQ_SIM1EMERG           52	/* SIM 1 Emergency Shutdown */
#define IRQ_DSPBRGERR           53	/* DSP Slave Port Async Bridge Error */
#define IRQ_DSI                 54	/* MIPI DSI */
#define IRQ_VDECBRGERR          55	/* Video Decoder Sync Bridge Error */
#define IRQ_VENCBRGERR          56	/* Video Encoder Sync Bridge Error */
#define IRQ_SDIO3               57	/* SDIO3 */
#ifdef CONFIG_DPRAM
#define IRQ_SSIPC_A2C		58	/* Reserved */
#define IRQ_SSIPC_C2A		59	/* Reserved */
#else
#define IRQ_58			58	/* Reserved */
#define IRQ_59			59	/* Reserved */
#endif
#define IRQ_60			60	/* Reserved */
#define IRQ_61			61	/* Reserved */
#define IRQ_62			62	/* Reserved */
#define IRQ_DRAM		63	/* Mamba DDR DRAM controller */

#define IRQ_GPIO2               64	/* GPIO 2 */
#define IRQ_WAKETMR2            65	/* Sleep Mode Timer 2 Wake-Up */
#define IRQ_TVENC_TOP           66	/* TV Encoder Top */
#define IRQ_TVENC_BOT           67	/* TV Encoder Bottom */
#define IRQ_GPT2                68	/* General Purpose Timer 2 (A/B) */
#define IRQ_WDOG2               69	/* Watchdog Timer 2 */
#define IRQ_MODEM_DMA           70	/* Modem ARM11 DMA */
#define IRQ_MODEM_PMU           71	/* Modem ARM11 PMU */
#define IRQ_UARTC               72	/* UART-C */
#define IRQ_SPI2                73	/* SPI 2 */
#define IRQ_LPERF_COUNTER       74	/* Bus Matrix L Performance Counter */
#define IRQ_DSP2MIRQ            75	/* RIP(DSP) Communications IRQ-Modem */
#define IRQ_DSP2MFIQ            76	/* RIP(DSP) Communications FIQ-Modem */
#define IRQ_AUXADC              77	/* AuxADC */
#define IRQ_USB_HSOTGEND        78	/* USB2.0 OTG (HSOTG) End Multi-Proc */
#define IRQ_UARTB_BAUD          79	/* UART-B Autobaud */
#define IRQ_CAM_CCIR            80	/* Camera Interface CCIR IRQ */
#define IRQ_CAM_CCP2            81	/* Camera Interface CCP2 IRQ */
#define IRQ_CAM_CSI2            82	/* Camera Interface CSI2 IRQ */
#define IRQ_LCD_TEAR            83	/* LCD Controller Tear Effect */
#define IRQ_SIM2IF 	        84	/* SIM 2 I/F  */
#define IRQ_SMC                 85	/* DSP SMC */
#define IRQ_MPHI	        86	/* MPHI Co-Processor Interface */
#define IRQ_AUDVOC              87	/* Audio Vocoder */
#define IRQ_OTGDRIVE	        88	/* USB-OTG Drive VBus */
#define IRQ_OTGCHARGE	        89	/* USB-OTG Charge VBus */
#define IRQ_OTGDISCHARGE        90	/* USB-OTG Discharge VBus */
#define IRQ_MDMABRGERR          91	/* ModemARM DMAPort Sync Bridge Error */
#define IRQ_L2HBRGERR      	92	/* Matrix L to H Sync Bridge Error */
#define IRQ_H2LBRGERR	        93	/* Matrix H to L Sync Bridge Error */
#define IRQ_IPC_A2C	        94	/* Software Interrupt */
#define IRQ_IPC_C2A     	95	/* Software Interrupt */

/* TODO: Check for proper bitmask */
#if defined(CONFIG_ARCH_BCM21553_B0) || defined(CONFIG_ARCH_BCM21553_B1)
#define BCM21553_VALID_SRC0    0xEDB53BEF      /* bitmask of intr[0:31] */
#else
#define BCM21553_VALID_SRC0    0xCDB53BEF      /* bitmask of intr[0:31] */
#endif
#ifdef CONFIG_DPRAM
#define BCM21553_VALID_SRC1 	0x8E4C7D20	/* bitmask of intr[32:63] */
#else
#define BCM21553_VALID_SRC1 	0x824C7D20	/* bitmask of intr[32:63] */
#endif
#define BCM21553_VALID_SRC2 	0xC7CFE33F	/* bitmask of intr[64:95] */

/*
 * Following 3 are used by entry_macro.S,
 * to access our 3-set of INTC registers
 */
#define INTC_REG_IMSR0		0x00c
#define INTC_REG_IMSR1		0x10c
#define INTC_REG_IMSR2		0x18c

/*
 * IRQs 96:159 are mapped onto GPIO lines 0:63
 */
#define  IRQ_GPIO_0		NUM_INTERNAL_IRQS
#define  GPIO_TO_IRQ(gpio)	((gpio) + IRQ_GPIO_0)
#define  IRQ_TO_GPIO(irq)	((irq) - IRQ_GPIO_0)
#define  gpio_to_irq(gpio)	GPIO_TO_IRQ(gpio)
#define  irq_to_gpio(irq)	IRQ_TO_GPIO(irq)

/*
 * In Kernel 2.6.32.9, On system wide suspend context the device
 * drivers are refrained from receiving the interrupts, it masks all the
 * interrupts except for the timer ones and sets the IRQ_DISABLED flag
 * for each of the driver. If the interrupt is a WAKEUP source then there
 * is no way to get out of the cuffs set by kernel as the handle_level_irq()
 * exits if the interrupt status is set to IRQF_DISABLED :-(,
 * In case of 2.6.35.7 this is taken care by defining a new IRQ status flag
 * IRQF_NO_SUSPEND which will ensure that respective IRQ is not disabled.
 * As a work around for this issue in 2.6.32.9 the keypad interrupt WAKEUP
 * source is described as IRQF_TIMER
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35))
	#define IRQF_NO_SUSPEND IRQF_TIMER
#endif
#endif /*ASM_ARCH_IRQS_H */
