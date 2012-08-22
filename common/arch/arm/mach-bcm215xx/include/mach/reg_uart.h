/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/include/mach/reg_uart.h
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

#ifndef __REG_BCM215XX_UART_H__
#define __REG_BCM215XX_UART_H__

#include <mach/io.h>
#include <mach/hardware.h>

#define HW_UARTA_BASE         IO_ADDRESS(BCM21553_UART_A_BASE)
#define HW_UARTB_BASE         IO_ADDRESS(BCM21553_UART_B_BASE)
#define HW_UARTC_BASE         IO_ADDRESS(BCM21553_UART_C_BASE)

/* UART register mapping */
#define ADDR_UARTA_UCR        (HW_UARTA_BASE + 0x100)
#define ADDR_UARTA_LSR        (HW_UARTA_BASE + 0x014)
#define ADDR_UARTA_THR        (HW_UARTA_BASE + 0x000)
#define ADDR_UARTB_UCR        (HW_UARTB_BASE + 0x100)
#define ADDR_UARTC_UCR        (HW_UARTC_BASE + 0x100)

#define ADDR_UARTA_UCR_PHYS   (BCM21553_UART_A_BASE + 0x100)
#define ADDR_UARTA_LSR_PHYS   (BCM21553_UART_A_BASE + 0x014)
#define ADDR_UARTA_THR_PHYS   (BCM21553_UART_A_BASE + 0x000)

/* Register bit defines */
#define ADDR_UART_PWR_DIS     (1<<6)

#endif /*__REG_BCM215XX_UART_H__ */
