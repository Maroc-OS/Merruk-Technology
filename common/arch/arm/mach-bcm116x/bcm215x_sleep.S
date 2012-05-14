/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm116x/bcm215x_sleep.S
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
 *   @file   linux/arch/arm/mach-bcm116x/bcm2153_sleep.S
 *
 *   @brief  Implements  bcm2153_sleep() function
 *
 ****************************************************************************/

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <mach/reg_clkpwr.h>

#define ADDR_IRQ_IMR		(HW_IRQ_BASE + 0x0000)
#define IMR_WFI_EN		0x00008000



/* Delay loop.... Loop count in r0*/
ENTRY(DelayLoop)
    subs	r0, r0, #1
    bne		DelayLoop
    bx		lr


/*
 *void	bcm215x_sleep(u32 bMaySleep)
 *
 *	Idle the processor (eg, wait for interrupt).
 *
 *	IRQs are already disabled.
 */
ENTRY(bcm215x_sleep)
    stmfd sp !,{r0 - r12, lr}			/*save registers on stack */
    mov r5,r0  @ save param  - bMaySleep

    ldr r2, =ADDR_CLKPWR_CLK_SEL_MODE    /* save dual PLL mode before sleep for two purposes  */
    ldr r2,[r2]                         /*1. avoid TLB walk-up after wake up */
    and r2, r2,#3					    /*2. avoid fetch wrong value after wake up*/

    /*ARM has to drain write buffer before stopping clock*/
	mov		r0, #0
	mcr		p15,0,r0,c7,c10,4

	/*Set WFI enable bit*/
	ldr	 r1, =ADDR_IRQ_IMR
	ldr	 r0, [r1]
	orr  r0, r0,#IMR_WFI_EN
	str  r0, [r1]

    mov r1,#0
    mcr p15, 0, r1, c7, c10, 4 @ DWB - WFI may enter a low - power mode
    mcr p15, 0, r1, c7, c0, 4 @ wait for interrupt

    nop
    nop
    nop
    ldr r0, = 0xFFF
    bl DelayLoop

    ldr r0, = 0xFFF
    bl DelayLoop

    cmp r5,#0   /*no sleep ?*/
    beq post_wakeup_delay_end
    ldr r0, = 0x3840
    bl DelayLoop

post_wakeup_delay_end:
    mov r1,#0
    mcr p15, 0, r1, c7, c10, 4
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    cmp r5,#0   /*no sleep ?*/
    beq poll_dual_clock_end

    cmp r2,#2 /* clk sel == Apps PLL ?? */
    bne poll_dual_clock_end

poll_dual_clock:
ldr r1, = ADDR_CLKPWR_CLK_PLL_STATUS
    ldr r1,[r1]
    tst r1,#4
    bne poll_dual_clock_end
    mov r0,#150
    bl DelayLoop   @ Delay to lock PLL
    b poll_dual_clock

poll_dual_clock_end:

    b first_jump
first_jump:
    b second_jump
second_jump:
    ldmfd sp !,{r0 - r12, pc}		/*restore regs and return */

