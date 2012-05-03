/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/include/mach/uncompress.h
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
 * Serial port stubs for kernel decompress status messages
 */
#ifndef __ASM_ARCH_UNCOMPRESS_H
#define __ASM_ARCH_UNCOMPRESS_H

#include <linux/types.h>
#include <linux/serial_reg.h>
#include <linux/io.h>
#include <mach/hardware.h>

static volatile u8 *uart_base = (volatile u8 *) BCM21553_UART_A_BASE ; 

/* PORT_8250, in polled non-fifo mode */
static void putc(const char c)
{
	while (!(uart_base[UART_LSR << 2] & UART_LSR_THRE))
		barrier();

	uart_base[UART_TX << 2] = c;
}

static void flush(void)
{
	while (!(uart_base[UART_LSR << 2] & UART_LSR_THRE))
		barrier();
}

#define arch_decomp_setup()	/* nothing to do here */
#define arch_decomp_wdog()	/* nothing to do here */

#endif /* __ASM_ARCH_UNCOMPRESS_H */
