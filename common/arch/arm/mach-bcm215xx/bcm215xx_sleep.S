/******************************************************************************
 * (C) COPYRIGHT 2007 ARM Limited.
 *       ALL RIGHTS RESERVED
 *
 * This software is provided 'as-is', without any express or implied warranties
 * including the implied warranties of satisfactory quality, fitness for
 * purpose or non infringement.  In no event will ARM be liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use, copy and modify this software for
 * any purpose, and to redistribute the software, subject to the following
 * restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in a
 *    product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 ******************************************************************************/

/* Portions of this software are Copyright 2011 Broadcom Corporation */

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/reg_irq.h>
#include <mach/reg_clkpwr.h>
#include <mach/reg_uart.h>
#include <mach/reg_syscfg.h>
#include <mach/hardware.h>
#include <mach/memory.h>
#include "bcm215xx_pm.h"
#include <mach/reg_emi.h>

/****************************************************************************
 *      Configuration and constants
 ****************************************************************************/

/* Specify whether the Main TLB RAM remains powered up in Dormant mode */
#define TLB_POWERED                    0

/* CP15 Control register bits */
#define CP15_CTRL_ICACHE_BIT           (1<<12)
#define CP15_CTRL_BPRED_BIT            (1<<11)
#define CP15_CTRL_DCACHE_BIT           (1<<2)
#define CP15_CTRL_MMU_BIT              (1<<0)

#define FPEXC_EN                       0x40000000
#define CACHE_LINE_SIZE                0x20 /* in bytes */

/*
 * Specify whether Dormant mode entry/exit code uses its own static memory
 * setup i.e. after saving memory setup state it changes to a memory setup
 * which can be statically restored from constants after reset, in which
 * case the original memory setup state can be saved into cache
 */
#define DM_STATICMEM                   0

/*
 * While stepping through the dormant sequence using a debugger, set this
 * to 1.
 */
#define DISABLE_DEBUG_REGS_SAVE        0

/* Set this to 1 to enable saving of cache master valid regs */
#define SAVE_CACHE_MASTER_VALID_REGS   0

/* Set this to 1 to save VFP context */
#define FPU_ENABLE                     0

/* Aliases for mode encodings */
#define MODE_USR    0x10
#define MODE_FIQ    0x11
#define MODE_IRQ    0x12
#define MODE_SVC    0x13
#define MODE_ABT    0x17
#define MODE_UND    0x1B
#define MODE_SYS    0x1F

#define CPSR_MODE_BITS_MASK            0x1F

/*
 * The dormant context buffer is placed as the last element of the sleep
 * store structure. PM_DORMANT_STORE is the offset in the sleep store
 * to the start of the context buffer.
 */
#define PM_DORMANT_STORE               (PM_NUM_SLEEP_ELEMENTS << 2)

#define MEM_V2P(v)    v - CONFIG_PAGE_OFFSET + CONFIG_SDRAM_BASE_ADDR
#define MEM_P2V(p)    p - CONFIG_SDRAM_BASE_ADDR + CONFIG_PAGE_OFFSET

/* L210 controller base address */
#define p2v(x) ((x) - BCM21553_IO_PHYSICAL + BCM21553_IO_VIRTUAL)
#define HW_L210_BASE                   p2v(BCM21553_L210_BASE)

#ifdef CONFIG_CACHE_L2X0
#define ENABLE_L2X0_CODE               1
#else
#define ENABLE_L2X0_CODE               0
#endif

/****************************************************************************
 *      Reenable cache and MMU
 ****************************************************************************/

/*
 * void    bcm215xx_pm_delay_loop(u32 count)
 *
 *    Busy-wait loop
 */
ENTRY(bcm215xx_pm_delay_loop)
    subs    r0,r0,#1
    bne     bcm215xx_pm_delay_loop
    bx      lr

/*
 * void    bcm215xx_sleep(struct bcm215xx_pm_sleep *pm_sleep)
 *
 *    Idle the processor (eg, wait for interrupt).
 *
 *    IRQs are already disabled.
 */
ENTRY(bcm215xx_sleep)
    stmfd   sp!,{r0-r12,lr}                  @ Save registers on stack
    ldr     r1,[r0,#PM_DORMANT_ALLOWED]      @ Is dormant allowed?
    cmp     r1,#0
    beq     skip_dorm
    /*
     * At this point lr contains the caller return address.
     * bcm215xx_dormant_entry directly returns to the caller
     * without coming here. Hence 'bne' is used here.
     */
    bl      bcm215xx_dormant_entry
    b       first_jump
skip_dorm:
    mov     r8,r0                            @ save dormant buf addr

    mrc     p15,0,r3,c1,c0,0                 @ Read ctrl reg
    bic     r0,r3,#CP15_CTRL_BPRED_BIT       @ Disable branch prediction
    mcr     p15,0,r0,c1,c0,0                 @ Write ctrl reg

.if ENABLE_L2X0_CODE==1
    ldr     r1,l210_base_wo_v
    ldr     r4,=L2X0_CACHE_SYNC
    add     r6,r4,r1                         @ L2X0_CACHE_SYNC reg addr
#ifdef CONFIG_BCM21553_L2_EVCT
    ldr     r10,[r8,#PM_L2_EVICT_BUF]
#endif
.endif /* ENABLE_L2X0_CODE */

    mov     r0,#0
    mcr     p15,0,r0,c7,c10,4                @ DSB

    /* Preload idle code into L1 I-cache */
    adr     r0,idle_save_preload_start
    adr     r1,idle_save_preload_end
    bic     r0,r0,#0x1F
idle_l1_prefetch_loop:
    mcr     p15,0,r0,c7,c13,1                @ Prefetch Instruction Cache Line
    add     r0,r0,#CACHE_LINE_SIZE
    cmp     r0,r1
    blt     idle_l1_prefetch_loop

idle_save_preload_start:
#ifdef CONFIG_BCM21553_L2_EVCT
    @ Dummy reads
    ldr     r0,[r10]                         @ dummy read from L2 eviction address
    ldr     r0,[r6]                          @ dummy read of l2 cache sync WO reg
#endif

.if ENABLE_L2X0_CODE==1
    mov     r0,#0
    mcr     p15,0,r0,c7,c10,4                @ DSB
    str     r0,[r6]                          @ L2 Cache Sync
    mcr     p15,0,r0,c7,c10,4                @ DSB
    ldr     r0,[r10]                         @ Dummy read to assure data ejected by L2 sync arrive at DRAM
    mov	    r0,#0
    mcr     p15,0,r0,c7,c10,4                @ DSB
.endif

#ifdef CONFIG_BCM21553_L2_EVCT
    mov     r0,#0
    str     r0,[r10]                         @ Clean L2 buffer eviction buffer
    mcr     p15,0,r0,c7,c10,4                @ DSB
    ldr     r0,[r10]                         @ Dummy read to assure data ejected by L2 eviction arrive at DRAM
    mov     r0, #0
    mcr     p15,0,r0,c7,c10,4                @ DSB
#endif

/* Adding delay before entering WFI mode */
   mov     r0,#150
   mov     r0,r0,lsl #3
kernel_delaybyloop1:
   subs    r0, r0, #1
   bne     kernel_delaybyloop1

    /* ARM has to drain write buffer before stopping clock */
    mov     r0,#0
    mcr     p15,0,r0,c7,c10,4                @ DSB
    mcr     p15,0,r0,c7,c0,4                 @ Wait-For-Interrupt

    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1

    mrc     p15,0,r1,c1,c0,0                 @ Read ctrl reg
    orr     r0,r1,#CP15_CTRL_BPRED_BIT       @ Enable branch prediction
    mcr     p15,0,r0,c1,c0,0                 @ Write ctrl reg

    b       first_jump
first_jump:
    b       second_jump
second_jump:
    ldmfd   sp!,{r0-r12,pc}                  @ restore regs and return
    .align 5
idle_save_preload_end:

/****************************************************************************
 *      Dormant mode entry
 ****************************************************************************/

/*
 * void bcm215xx_dormant_entry(struct bcm215xx_pm_sleep *pm_sleep);
 *
 * This code assumes that the dormant mode entry code is called using an
 * instruction of the form:
 *
 *     bl dormant_entry
 *
 * as the dormant mode exit code returns using an instruction of the form
 *
 *     mov pc,r14
 */

ENTRY(bcm215xx_dormant_entry)
    stmfd   sp!,{lr}

.if BCM_PM_DORMANT_PROFILING==1
    /* toggle BCM_PM_DORMANT_DEBUG_GPIO */
    ldr     r1,gpor0
    ldr     r2,[r1]
    eor     r2,r2,#(1 << BCM_PM_DORMANT_DEBUG_GPIO)
    str     r2,[r1]
.endif

    /* Enter dormant mode */
    bl      bcm215xx_dormant_sleep

.if BCM_PM_DORMANT_PROFILING==1
    /* toggle BCM_PM_DORMANT_DEBUG_GPIO */
    ldr     r1,gpor0
    ldr     r2,[r1]
    eor     r2,r2,#(1 << BCM_PM_DORMANT_DEBUG_GPIO)
    str     r2,[r1]
.endif

    ldmfd   sp!,{pc}

/*
 * The dormant mode entry code saves all required state in the correct order.
 * You must make some modifications to this code as explained by the comments
 * within it.  The most important change is to add your system-dependent
 * method of setting up your system's power controller so that it will enter
 * dormant mode when it sees the macrocell output STANDBYWFI go high.
 * Full details are given in the comments.
 */
    .extern bcm_pm_sleep_buf
    .extern bcm_pm_sleep_buf_phys

bcm215xx_dormant_sleep:
    /* Save a copy of the context buffer address at a known
     * offset in the SDRAM.
     */
    ldr     r0,=bcm_pm_sleep_buf
    ldr     r0,[r0]

    mov     r2,#0x00
    str     r2,[r0,#PM_DORMANT_EXIT_STAT]
    ldr     r1,[r0,#PM_DORMANT_SAVE_CNT]@ Increment save count
    add     r1,r1,#1
    str     r1,[r0,#PM_DORMANT_SAVE_CNT]

    @ Save SYS mode registers
    @ Save general purpose registers (SYS) and CPSR
    @ BRCM customization: we don't care r0 value; otherwise
    @ r0_store must be close to PC,
    @ requiring code area to be R/W and causing dirty cache.
    @ Hui Luo, 1/4/11
    cpsid   if,#MODE_SYS
    add     r0,r0,#PM_DORMANT_STORE    @ Get state save addr
    str     r1,[r0,#4]                 @ Save r1
    mov     r1,r0                      @ Get state save addr
    str     r0,[r1],#8                 @ Save r0
    stmia   r1!,{r2-r14}               @ Save r2-r14 (SYS/USR)
    mrs     r0,cpsr                    @ Get CPSR
    str     r0,[r1],#4                 @ Save CPSR

    @ Interrupts are already disabled at this point. Save
    @ regs of all the modes.

    @ Save FIQ mode registers and SPSR
    cps     #MODE_FIQ
    stmia   r1!,{r8-r14}               @ Save r8-r14 (FIQ)
    mrs     r0,spsr                    @ Get SPSR
    str     r0,[r1],#4                 @ Save SPSR

    @ Save IRQ mode registers and SPSR
    cps     #MODE_IRQ
    stmia   r1!,{r13-r14}              @ Save r13-r14 (IRQ)
    mrs     r0,spsr                    @ Get SPSR
    str     r0,[r1],#4                 @ Save SPSR

    @ Save Abort mode registers and SPSR
    cps     #MODE_ABT
    stmia   r1!,{r13-r14}              @ Save r13-r14 (Abort)
    mrs     r0,spsr                    @ Get SPSR
    str     r0,[r1],#4                 @ Save SPSR

    @ Save Supervisor mode registers and SPSR
    cps     #MODE_SVC
    stmia   r1!,{r13-r14}              @ Save r13-r14 (SVC)
    mrs     r0,spsr                    @ Get SPSR
    str     r0,[r1],#4                 @ Save SPSR

    @ Save Undefined mode registers and SPSR
    cps     #MODE_UND
    stmia   r1!,{r13-r14}              @ Save r13-r14 (Undef)
    mrs     r0,spsr                    @ Get SPSR
    str     r0,[r1],#4                 @ Save SPSR

    @ Save the rest of the context in SVC mode. If any exception occurs
    @ in this sequence, the Linux handlers will catch it.
    cps     #MODE_SVC

    @ Save TLB lockdown register, then force page table walks to
    @ write into the associative region of the TLB, to avoid
    @ changing the lockdown region
    mrc     p15,0,r2,c10,c0,0          @ TLB Lockdown
    str     r2,[r1],#4                 @ Save
    bic     r2,r2,#1                   @ Clear P bit
    mcr     p15,0,r2,c10,c0,0          @ Write TLB Lockdown

  @ If Dormant mode has its own static memory setup, save memory system
  @ control registers then change to the static memory setup.
  @ This is only worth doing if both the original memory setup and the
  @ static setup enable write-back into the data cache.
.if DM_STATICMEM==1
    mrc     p15,0,r2,c1,c0,0           @ Control
    mrc     p15,0,r3,c2,c0,0           @ TTBR0
    mrc     p15,0,r4,c2,c0,1           @ TTBR1
    mrc     p15,0,r5,c2,c0,2           @ TTBCR
    mrc     p15,0,r6,c3,c0,0           @ DAC
    mrc     p15,0,r7,c13,c0,0          @ FCSE PID
    mrc     p15,0,r8,c13,c0,1          @ Context ID
    stmia   r1!,{r2-r8}                @ Save
    mrc     p15,0,r2,c15,c2,0          @ Data Mem Remap
    mrc     p15,0,r3,c15,c2,1          @ Instr Mem Remap
    mrc     p15,0,r4,c15,c2,2          @ DMA Mem Remap
    mrc     p15,0,r5,c15,c2,4          @ PP Mem Remap
    mrc     p15,7,r6,c15,c0,0          @ Cache Debug Control
    mrc     p15,7,r7,c15,c1,0          @ TLB Debug Control
    mrc     p15,0,r8,c9,c0,0           @ D Cache Lockdown
    mrc     p15,0,r9,c9,c0,1           @ I Cache Lockdown
    stmia   r1!,{r2-r9}                @ Save
    @ ARM1136 rev1 has two extra memory system registers
    mrc     p15,0,r0,c0,c0,0           @ Read ID code
    ands    r0,r0,#0xF << 20           @ Is this a rev0 device?
    beq     rev0_skip1                 @ Yes: skip
    mrc     p15,0,r2,c10,c2,0          @ Primary Region Remap
    mrc     p15,0,r3,c10,c2,1          @ Normal Memory Remap
    stmia   r1!,{r2-r3}                @ Save
rev0_skip1:

  @ Now set up the static memory configuration for Dormant mode.
  @ This may involve changing TTBR regs to use new page tables, or disabling
  @ the MMU and remapping memory to write-back. If you disable the MMU here,
  @ write 0x00000000 to the FCSE PID register first.
.endif /* DM_STATICMEM */

  @ Note: if you do not have a separate static memory setup for dormant mode,
  @ the above state will be saved later after disabling the caches & MMU.

    @ Save CP15 control & fault registers
    mrc     p15,0,r2,c5,c0,0           @ DFSR
    mrc     p15,0,r3,c5,c0,1           @ IFSR
    mrc     p15,0,r4,c6,c0,0           @ FAR
    mrc     p15,0,r5,c6,c0,1           @ IFAR
    mrc     p15,0,r6,c1,c0,1           @ Auxiliary Control
    mrc     p15,0,r7,c1,c0,2           @ Coproc Access Control
    stmia   r1!,{r2-r7}                @ Save
    @ ARM1136 rev1 has three extra thread/process ID registers
    mrc     p15,0,r0,c0,c0,0           @ Read ID code
    ands    r0,r0,#0xF << 20           @ Is this a rev0 device?
    beq     rev0_skip2                 @ Yes: skip
    mrc     p15,0,r2,c13,c0,2          @ User R/W Thread/PID
    mrc     p15,0,r3,c13,c0,3          @ User RO Thread/PID
    mrc     p15,0,r4,c13,c0,4          @ Priv Only Thread/PID
    stmia   r1!,{r2-r4}                @ Save
rev0_skip2:

    @ If we have a DMA, save DMA state (both channels) and
    @ stop running and queued DMA channels.
    @ If either channel has stopped with an error, we cannot
    @ restore this error status. This code will abandon entry to
    @ dormant mode and return. Alternatively you can change this
    @ code to process the DMA error(s) and continue.
    @ This code checks if the DMA is present by reading the DMA
    @ ID & Status reg.  If your code runs on a system that never
    @ has an internal DMA engine you can remove this code section.
    mrc     p15,0,r0,c11,c0,0          @ Read DMA ID & Status
    cmp     r0,#3                      @ Do we have a DMA?
    bne     no_dma                     @ No - skip DMA save

    mrc     p15,0,r2,c11,c1,0          @ User Accessibility Reg
    mrc     p15,0,r3,c11,c2,0          @ Channel Number

    mov     r0,#0
    mcr     p15,0,r0,c11,c2,0          @ Select Channel 0
    mrc     p15,0,r4,c11,c8,0          @ DMA channel status
    cmp     r4,#3                      @ Is there an error?
    bhi     quit_dormant_entry         @ Yes: quit
    mcr     p15,0,r0,c11,c3,0          @ No: stop the channel

    mov     r0,#1
    mcr     p15,0,r0,c11,c2,0          @ Select Channel 1
    mrc     p15,0,r10,c11,c8,0         @ DMA channel status
    cmp     r10,#3                     @ Is there an error?
    bhi     quit_dormant_entry         @ Yes: quit
    mcr     p15,0,r0,c11,c3,0          @ No: stop the channel

    mov     r0,#0
    mcr     p15,0,r0,c11,c2,0          @ Select Channel 0
wait_dma0_idle:
    mrc     p15,0,r0,c11,c8,0          @ DMA channel status
    ands    r0,r0,#3                   @ Is the channel idle
    cmpne   r0,#3                      @ or Complete/Error?
    bne     wait_dma0_idle             @ Poll until it is
    mrc     p15,0,r5,c11,c4,0          @ Control Register
    mrc     p15,0,r6,c11,c5,0          @ Internal Start Address
    mrc     p15,0,r7,c11,c6,0          @ External Start Address
    mrc     p15,0,r8,c11,c7,0          @ Internal End Address
    mrc     p15,0,r9,c11,c15,0         @ Context ID
    stmia   r1!,{r2-r9}                @ Save
    mcr     p15,0,r0,c11,c3,2          @ Clear channel 0

    mov     r0,#1
    mcr     p15,0,r0,c11,c2,0          @ Select Channel 1
    mov     r4,r10                     @ Prior channel status
wait_dma1_idle:
    mrc     p15,0,r0,c11,c8,0          @ DMA channel status
    ands    r0,r0,#3                   @ Is the channel idle
    cmpne   r0,#3                      @ or Complete/Error?
    bne     wait_dma1_idle             @ Poll until it is
    mrc     p15,0,r5,c11,c4,0          @ Control Register
    mrc     p15,0,r6,c11,c5,0          @ Internal Start Address
    mrc     p15,0,r7,c11,c6,0          @ External Start Address
    mrc     p15,0,r8,c11,c7,0          @ Internal End Address
    mrc     p15,0,r9,c11,c15,0         @ Context ID
    stmia   r1!,{r4-r9}                @ Save
    mcr     p15,0,r0,c11,c3,2          @ Clear channel 1
no_dma:

    @ NB -  Jazelle-DBX registers (CP14) were not forgotten.
    @       They should not be saved or restored by this routine.
    @       The Virtual Machine should be relied upon to
    @       re-initialize Jazelle-DBX state.

    @ Save lockdown region of TLB
    mov     r0,#0x80000000
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 0
    mrc     p15,5,r2,c15,c5,2          @ Read Main TLB VA
    mrc     p15,5,r3,c15,c6,2          @ Read Main TLB PA
    mrc     p15,5,r4,c15,c7,2          @ Read Main TLB Attr

    add     r0,r0,#1
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 1
    mrc     p15,5,r5,c15,c5,2          @ Read Main TLB VA
    mrc     p15,5,r6,c15,c6,2          @ Read Main TLB PA
    mrc     p15,5,r7,c15,c7,2          @ Read Main TLB Attr

    add     r0,r0,#1
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 2
    mrc     p15,5,r8,c15,c5,2          @ Read Main TLB VA
    mrc     p15,5,r9,c15,c6,2          @ Read Main TLB PA
    mrc     p15,5,r10,c15,c7,2         @ Read Main TLB Attr

    add     r0,r0,#1
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 3
    mrc     p15,5,r11,c15,c5,2         @ Read Main TLB VA
    mrc     p15,5,r12,c15,c6,2         @ Read Main TLB PA
    mrc     p15,5,r13,c15,c7,2         @ Read Main TLB Attr

    stmia   r1!,{r2-r13}               @ Save

    add     r0,r0,#1
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 4
    mrc     p15,5,r2,c15,c5,2          @ Read Main TLB VA
    mrc     p15,5,r3,c15,c6,2          @ Read Main TLB PA
    mrc     p15,5,r4,c15,c7,2          @ Read Main TLB Attr

    add     r0,r0,#1
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 5
    mrc     p15,5,r5,c15,c5,2          @ Read Main TLB VA
    mrc     p15,5,r6,c15,c6,2          @ Read Main TLB PA
    mrc     p15,5,r7,c15,c7,2          @ Read Main TLB Attr

    add     r0,r0,#1
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 6
    mrc     p15,5,r8,c15,c5,2          @ Read Main TLB VA
    mrc     p15,5,r9,c15,c6,2          @ Read Main TLB PA
    mrc     p15,5,r10,c15,c7,2         @ Read Main TLB Attr

    add     r0,r0,#1
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 7
    mrc     p15,5,r11,c15,c5,2         @ Read Main TLB VA
    mrc     p15,5,r12,c15,c6,2         @ Read Main TLB PA
    mrc     p15,5,r13,c15,c7,2         @ Read Main TLB Attr

    stmia   r1!,{r2-r13}               @ Save

    @ Save the state of any memory-mapped peripherals that are
    @ powered down in dormant mode.
    @ For example: the validation trickbox registers
    @ Replace this code with your own code to save the state of
    @ your memory-mapped peripherals.
@TB_BASE         equ     0x13000000    @ Trickbox base address
@    ldr    r0,=TB_BASE
@    ldr    r2,[r0,#0x64]              @ Trickbox abort addr
@    ldr    r3,[r0,#0x44]              @ Trickbox abort range 1
@    ldr    r4,[r0,#0x40]              @ Trickbox abort range 2
@    stmia  r1!,{r2-r4}                @ Save

    @ Enable access to all coprocessors to save coproc & VFP state
    @ You can omit this if you have no coprocessors and no VFP
    mov     r0,#0xFFFFFFFF             @ Enable all coprocs
    mcr     p15,0,r0,c1,c0,2           @ Write coproc acc ctrl

    @ If we have a VFP coprocessor, save the VFP system regs and
    @ register bank.
.if FPU_ENABLE==1
    @ The VFP will always be disabled here for lazy context switching,
    @ but if it has been used by any thread it will contain state which
    @ would be lost by a shift into dormant mode. So must enable it and
    @ save the registers.
    mrc     p15,0,r0,c1,c0,2           @ r0 = Access Control Register
    orr     r0,r0,#(0x5<<20)           @ enable priv access for p10,11
    mcr     p15,0,r0,c1,c0,2           @ Access Control Register = r0
    mov     r0,#0
    mcr     p15,0,r0,c7,c5,4           @ flush prefetch buffer because of FMXR
                                       @ below and CP 10 & 11 were only just
                                       @ enabled
    @ Then enable the VFP
    fmrx    r0,fpexc                   @ r0 = fpexc
    orr     r0,r0,#FPEXC_EN
    fmxr    fpexc,r0                   @ fpexc = r0

    fmrx    r2,fpexc                   @ VFP Exception
    fmxr    fpexc,r0                   @ Write VFP Exception
    fmrx    r3,fpscr                   @ VFP Status & Control

    @ This is done for cases where a pending VFP exception is taken on the
    @ previous "FMRX    r3,FPSCR" instruction. If exception is taken, the
    @ old value in FPEXC is cleared. So reading FPEXC again.
    fmrx    r6,fpexc
    and     r2,r2,r6

    fmrx    r4,fpinst                 @ VFP Instruction
    fmrx    r5,fpinst2                @ VFP Instruction 2
    stmia   r1!,{r2-r5}               @ Save
    fstmiax r1!,{d0-d15}              @ Save VFP register bank
.endif /* FPU_ENABLE */

    @ Save the registers of all other coprocessors.
    @ As all coprocessors are different, we cannot write generic
    @ code here to save the registers of any coprocessor. If you
    @ have coprocessors other than the VFP, write your own code to
    @ save all of their registers to memory. Otherwise, remove this
    @ section of code.
    @ The example code here saves the validation generic
    @ coprocessors p4 & p5.
    @ Note: you can test for the presence of coprocessors by reading
    @ the appropriate bits of the coprocessor access control reg,
    @ but you still need to know what state must be saved for each
    @ individual coprocessor present in the system.
@    stc     p4,c0,[r1],#4              @ Save coproc p4 regs
@    stc     p4,c1,[r1],#4
@    stc     p4,c2,[r1],#4
@    stc     p4,c3,[r1],#4
@    stc     p4,c4,[r1],#4
@    stc     p4,c5,[r1],#4
@    stc     p4,c6,[r1],#4
@    stc     p4,c7,[r1],#4
@    stc     p4,c8,[r1],#4
@    stc     p4,c9,[r1],#4
@    stc     p4,c10,[r1],#4
@    stc     p4,c11,[r1],#4
@    stc     p4,c12,[r1],#4
@    stc     p4,c13,[r1],#4
@    stc     p4,c14,[r1],#4
@    stc     p4,c15,[r1],#4
@    stc     p5,c0,[r1],#4              @ Save coproc p5 regs
@    stc     p5,c1,[r1],#4
@    stc     p5,c2,[r1],#4
@    stc     p5,c3,[r1],#4
@    stc     p5,c4,[r1],#4
@    stc     p5,c5,[r1],#4
@    stc     p5,c6,[r1],#4
@    stc     p5,c7,[r1],#4
@    stc     p5,c8,[r1],#4
@    stc     p5,c9,[r1],#4
@    stc     p5,c10,[r1],#4
@    stc     p5,c11,[r1],#4
@    stc     p5,c12,[r1],#4
@    stc     p5,c13,[r1],#4
@    stc     p5,c14,[r1],#4
@    stc     p5,c15,[r1],#4

    @ Save the Performance Monitor registers
    @ Note that performance monitor counts will be restarted from
    @ these saved values on exiting dormant mode, but not
    @ immediately, so the counts of events may not be correct.
    mrc     p15,0,r2,c15,c12,0         @ Performance mon ctrl
    mrc     p15,0,r3,c15,c12,1         @ Cycle counter
    mrc     p15,0,r4,c15,c12,2         @ Count register 0
    mrc     p15,0,r5,c15,c12,3         @ Count register 1
    stmia   r1!,{r2-r5}                @ Save

.if DISABLE_DEBUG_REGS_SAVE==0
    @ Save the Debug registers
    mrc     p14,0,r2,c0,c1,0           @ DSCR
    orr     r0,r2,#1 << 15             @ Select & enable Debug
    bic     r0,r0,#1 << 14             @ monitor mode
    mcr     p14,0,r0,c0,c1,0           @ Write DSCR
    mrc     p14,0,r3,c0,c0,4           @ BVR0
    mrc     p14,0,r4,c0,c1,4           @ BVR1
    mrc     p14,0,r5,c0,c2,4           @ BVR2
    mrc     p14,0,r6,c0,c3,4           @ BVR3
    mrc     p14,0,r7,c0,c4,4           @ BVR4
    mrc     p14,0,r8,c0,c5,4           @ BVR5
    mrc     p14,0,r9,c0,c0,5           @ BCR0
    mrc     p14,0,r10,c0,c1,5          @ BCR1
    mrc     p14,0,r11,c0,c2,5          @ BCR2
    mrc     p14,0,r12,c0,c3,5          @ BCR3
    mrc     p14,0,r13,c0,c4,5          @ BCR4
    mrc     p14,0,r14,c0,c5,5          @ BCR5
    stmia   r1!,{r2-r14}               @ Save
    mrc     p14,0,r2,c0,c0,6           @ WVR0
    mrc     p14,0,r3,c0,c1,6           @ WVR1
    mrc     p14,0,r4,c0,c0,7           @ WCR0
    mrc     p14,0,r5,c0,c1,7           @ WCR1
    mrc     p14,0,r6,c0,c7,0           @ VTR
    stmia   r1!,{r2-r6}                @ Save
.endif /* DISABLE_DEBUG_REGS_SAVE */

    @ BRCM customization
    @ 1. Disable branch prediction to avoid unexpected prefetch from external
    @    RAM
    @ 2. Prefetch remaining dormant entry code and some dormant abort exit
    @    code into L1 I-cache.
    @ 3. Clean L1 D-cache, which will be powered down in dormant mode, but
    @    leaving L1 I-cache on. Do not save and restore L1 I/D cache's valid
    @    bits. L1 I/D cache should be invalidated at dormant exit.
    @ 4. Dummy write to non-cacheable normal memory to fix L210 eviction buffer
    @    problem; L2 cache sync; and disable L2 cache it is OK to not clean L2
    @    cache because its content is maintained in dormant mode.
    @ Hui Luo, 1/4/11

    @ Disable branch prediction
    mrc     p15,0,r2,c1,c0,0           @ Read ctrl reg
    bic     r0,r2,#CP15_CTRL_BPRED_BIT
    mcr     p15,0,r0,c1,c0,0           @ Write ctrl reg

    @ Preload rest of the dormant code (in virtual space) into L1 cache
    mov     r3,#0
    adr     r4,dormant_save_preload_start
    adr     r5,dormant_save_preload_end
    bic     r4,r4,#0x1F
l1_prefetch_loop:
    mcr     p15,0,r4,c7,c13,1          @ Prefetch Instruction Cache Line
    add     r4,r4,#CACHE_LINE_SIZE
    cmp     r4,r5
    blt     l1_prefetch_loop

    @.align 5
dormant_save_preload_start:

    @ BRCM customization: Clean the entire data cache
    @ Hui Luo, 1/4/11
.if ENABLE_L2X0_CODE==1
    ldr     r3,l210_base_ro_v          @ may cause L1/L2 data cache churning
    ldr     r4,l210_base_wo_v
#ifdef CONFIG_BCM21553_L2_EVCT
    ldr     r0,=bcm_pm_sleep_buf       @ may cause L1/L2 data cache churning
    ldr     r5,[r0]                    @ may cause L1/L2 data cache churning
    ldr     r6,[r5,#PM_L2_EVICT_BUF]   @ may cause L1/L2 data cache churning
    ldr     r0,=bcm_pm_sleep_buf_phys  @ may cause L1/L2 data cache churning
    ldr     r8,[r0]                    @ may cause L1/L2 data cache churning
    ldr     r9,page_offset             @ may cause L1/L2 data cache churning
    ldr     r10,phys_offset            @ may cause L1/L2 data cache churning

    @ Dummy reads to preload MMU descriptor into MMU TLB cache
    ldr     r0,[r6]                    @ dummy read from L2 eviction address
    ldr     r0,[r4,#L2X0_CACHE_SYNC]   @ dummy read of l2 cache sync reg
#endif
.endif /* ENABLE_L2X0_CODE */

    mov     r0,#0                      @ SBZ
    mcr     p15,0,r0,c7,c10,4          @ DSB
    mcr     p15,0,r0,c7,c10,0          @ Clean L1 D
    mcr     p15,0,r0,c7,c10,4          @ DSB
    @ From here to the point disabling D-cache, L1 D-cache cannot be made dirty

.if ENABLE_L2X0_CODE==1
    ldr     r2,[r3,#L2X0_CTRL]
    ands    r2,r2,#1
    str     r2,[r5,#PM_L2CACHE_INIT_FLAG]
    beq	    l2_clean_skipped
    @ Dummy write to non-cacheable memory to force eviction buffer
    @ to get drained to main memory
    @
    mov     r2,#0
    mcr     p15,0,r2,c7,c10,4          @ DSB
    str     r2,[r4,#L2X0_CACHE_SYNC]   @ L2 Cache sync
    mcr     p15,0,r2,c7,c10,4          @ DSB
    ldr     r7,[r6]                    @ Dummy read to assure data ejected by L2 sync arrive at DRAM
    mcr     p15,0,r2,c7,c10,4          @ DSB
#ifdef CONFIG_BCM21553_L2_EVCT
    mov     r7,#0
    str     r7,[r6]                    @ Clean L2 buffer eviction buffer
    mcr     p15,0,r2,c7,c10,4          @ DSB
    ldr     r7,[r6]                    @ Dummy read to assure data ejected by L2 eviction arrive at DRAM
    mcr     p15,0,r2,c7,c10,4          @ DSB
#endif

    @ Delay 12us by loop with branch prediction disabled.
    @ One loop @832MHz takes 10ns (10560us / 1048576)
    mov     r2,#150
    mov     r2,r2,lsl #3
l2_clean_wait2:
    subs    r2,r2,#1
    bne     l2_clean_wait2
    mov     r2, #0
    str     r2,[r4,#L2X0_CTRL]         @ disable l2 cache
    mcr     p15,0,r2,c7,c10,4          @ DSB
l2_clean_skipped:
.endif /* ENABLE_L2X0_CODE */

    @ Disable caches to save state that must be restored
    @ before the MMU and caches can be enabled.
    @ This ensures that this state is not saved into the cache.
    @ First, ensure there are no block transfer operations ongoing
wait_block:
    mrc     p15,0,r0,c7,c12,4          @ Block transfer status
    cmp     r0,#0                      @ Block prefetch going?
    bne     wait_block                 @ Yes - wait

    @ Since MMU is going to be disabled, from this point forward r1
    @ should have physical address of the dormant buffer
    mov     r0,r5                      @ r5 = bcm_pm_sleep_buf

    @ Get the current offset of the current location pointed by
    @ R1 in dormant buffer.
    add     r0,r0,#PM_DORMANT_STORE    @ Start of dormant store area virt
    sub     r0,r1,r0                   @ Offset = Current - Start
    mov     r1,r8                      @ r8 = bcm_pm_sleep_buf_phys
    add     r1,r1,#PM_DORMANT_STORE    @ Start of dormant store area phys
    add     r1,r1,r0                   @ Current dormant buf address phys

    @ Load the phys address of bcm215xx_mmu_off in r3
    adr     r3,bcm215xx_mmu_off        @ Virtual address in r3
    mov     r2,r9                      @ r9  = *(u32*)page_offset
    mov     r0,r10                     @ r10 = *(u32*)phys_offset
    sub     r3,r3,r2
    add     r3,r3,r0                   @ Physical address in r3

    @ Disable only L1 D-cache. L1 I-cache remains powered on in dormant
    @ mode. It is kept on here to ensure that no bus transactions occur
    @ after WFI for atleast 80ns.
    @ BRCM customization: L1 I-cache must be on to avoid bus transactions
    @ after WFI for 80ns
    @ After this step, no data cache activity because L1 D-cache and
    @ L2 cache is disabled
    @ No instruction cache activity either because instructions are
    @ loaded into L1 I-cache, which is still on.
    @ Hui Luo, 1/4/11
    mrc     p15,0,r2,c1,c0,0           @ Read ctrl reg
    bic     r10,r2,#CP15_CTRL_DCACHE_BIT@ r2 is saved further on, hence it is
                                       @ not modified here.
    bic     r10,r10,#CP15_CTRL_MMU_BIT
    b       bcm215xx_turn_off_mmu

    .align 5
bcm215xx_turn_off_mmu:
    mov     r4,r4
    mcr     p15,0,r10,c1,c0,0          @ Disable D and MMU
    mrc     p15,0,r4,c0,c0,0           @ Read id reg
    mov     r4,r4
    mov     r4,r4
    mov     pc,r3

bcm215xx_mmu_off:
     mov     r4,r4
     mov     r4,r4
     mov     r4,r4

    @ Preload rest of the dormant code (in physical space) into L1 cache
    mov     r3,#0
    adr     r4,static_mem_setup
    adr     r5,dormant_save_preload_end
    bic     r4,r4,#0x1F
l1_prefetch_loop_phys:
    mcr     p15,0,r4,c7,c13,1          @ Prefetch Instruction Cache Line
    add     r4,r4,#CACHE_LINE_SIZE
    cmp     r4,r5
    blt     l1_prefetch_loop_phys

static_mem_setup:
  @ If Dormant mode does not have its own static memory setup, save memory
  @ system control registers to main memory.
  @ If it does have its own setup, we've already saved these registers.
.if DM_STATICMEM==0
    @ We read the Control reg into r2 before modifying it above.
    mrc     p15,0,r3,c2,c0,0           @ TTBR0
    mrc     p15,0,r4,c2,c0,1           @ TTBR1
    mrc     p15,0,r5,c2,c0,2           @ TTBCR
    mrc     p15,0,r6,c3,c0,0           @ DAC
    mrc     p15,0,r7,c13,c0,0          @ FCSE PID
    mrc     p15,0,r8,c13,c0,1          @ Context ID
    stmia   r1!,{r2-r8}                @ Save
    mrc     p15,0,r2,c15,c2,0          @ Data Mem Remap
    mrc     p15,0,r3,c15,c2,1          @ Instr Mem Remap
    mrc     p15,0,r4,c15,c2,2          @ DMA Mem Remap
    mrc     p15,0,r5,c15,c2,4          @ PP Mem Remap
    mrc     p15,7,r6,c15,c0,0          @ Cache Debug Control
    mrc     p15,7,r7,c15,c1,0          @ TLB Debug Control
    mrc     p15,0,r8,c9,c0,0           @ D Cache Lockdown
    mrc     p15,0,r9,c9,c0,1           @ I Cache Lockdown
    stmia   r1!,{r2-r9}                @ Save
    @ ARM1136 rev1 has two extra memory system registers
    mrc     p15,0,r0,c0,c0,0           @ Read ID code
    ands    r0,r0,#0xF << 20           @ Is this a rev0 device?
    beq     rev0_skip3                 @ Yes: skip
    mrc     p15,0,r2,c10,c2,0          @ Primary Region Remap
    mrc     p15,0,r3,c10,c2,1          @ Normal Memory Remap
    stmia   r1!,{r2-r3}                @ Save
rev0_skip3:
.endif /* DM_STATICMEM */

.if SAVE_CACHE_MASTER_VALID_REGS==1
    @ Save the cache master valid registers
    @ Number of registers to save depends on the cache size:
    @ 4k cache: save reg 0
    @ 8k      :          0
    @ 16k     :          0-1
    @ 32k     :          0-3
    @ 64k     :          0-7
    @ Here we get the cache sizes from the cache type reg.
    @ If you know that your cache sizes are fixed you can remove
    @ the checking and just save the regs you need.
    @ Alternatively you can save all regs 0-7 regardless of the
    @ cache size - restoring unused regs has no effect. However,
    @ this impacts the performance due to extra MRCs and stores.
    @ D cache master valid bits
    mrc     p15,0,r0,c0,c0,1           @ Cache type
    and     r10,r0,#7 << 18            @ Dcache size in [20:18]
    mrc     p15,3,r2,c15,c12,0         @ Dcache master valid 0
    str     r2,[r1],#4                 @ Save
    cmp     r10,#5 << 18               @ Dcache >= 16k?
    mrchs   p15,3,r3,c15,c12,1         @ Dcache master valid 1
    strhs   r3,[r1],#4                 @ Save
    cmp     r10,#6 << 18               @ Dcache >= 32k?
    mrchs   p15,3,r4,c15,c12,2         @ Dcache master valid 2
    mrchs   p15,3,r5,c15,c12,3         @ Dcache master valid 3
    stmhsia r1!,{r4-r5}                @ Save
    cmp     r10,#7 << 18               @ Dcache = 64k?
    mrchs   p15,3,r6,c15,c12,4         @ Dcache master valid 4
    mrchs   p15,3,r7,c15,c12,5         @ Dcache master valid 5
    mrchs   p15,3,r8,c15,c12,6         @ Dcache master valid 6
    mrchs   p15,3,r9,c15,c12,7         @ Dcache master valid 7
    stmhsia r1!,{r6-r9}                @ Save

    @ I cache master valid bits
    and     r10,r0,#7 << 6             @ Icache size in [8:6]
    mrc     p15,3,r2,c15,c8,0          @ Icache master valid 0
    str     r2,[r1],#4                 @ Save
    cmp     r10,#5 << 6                @ Icache >= 16k?
    mrchs   p15,3,r3,c15,c8,1          @ Icache master valid 1
    strhs   r3,[r1],#4                 @ Save
    cmp     r10,#6 << 6                @ Icache >= 32k?
    mrchs   p15,3,r4,c15,c8,2          @ Icache master valid 2
    mrchs   p15,3,r5,c15,c8,3          @ Icache master valid 3
    stmhsia r1!,{r4-r5}                @ Save
    cmp     r10,#7 << 6                @ Icache = 64k?
    mrchs   p15,3,r6,c15,c8,4          @ Icache master valid 4
    mrchs   p15,3,r7,c15,c8,5          @ Icache master valid 5
    mrchs   p15,3,r8,c15,c8,6          @ Icache master valid 6
    mrchs   p15,3,r9,c15,c8,7          @ Icache master valid 7
   stmhsia r1!,{r6-r9}                @ Save

    @ If TCMs are enabled as SmartCache, save the SmartCache
    @ master valid bits and the TCM region registers.
    @ If TCMs are not enabled as SmartCache, we do not need to
    @ save the SmartCache master valid bits, just the TCM
    @ region regs. This code checks if TCMs are enabled as
    @ SmartCache. If your code runs on a system that never has
    @ TCMs, you can remove this code@ if your system has TCMs
    @ and always uses SmartCache, you can remove the check
    @ if your system has TCMs
    @ and never uses SmartCache, you can remove this code except
    @ for the saving of the TCM region regs.
    @ Note: saving and restoring the TCM region regs on a system
    @ with no TCMs has no effect, and is easier and quicker than
    @ testing if we have TCMs.
    @ Save the SmartCache master valid registers
    @ Number of registers to save depends on the TCM size:
    @ 4k TCM  : save reg 0
    @ 8k      :          0
    @ 16k     :          0-1
    @ 32k     :          0-3
    @ 64k     :          0-7
    @ Here we get the TCM sizes from the TCM region reg.
    @ If you know that your TCM sizes are fixed you can remove
    @ the checking and just save the regs you need.
    @ Alternatively you can save all regs 0-7 regardless of the
    @ TCM size - restoring unused regs has no effect. However,
    @ this impacts the performance due to extra MRCs and stores
    @ to main memory.
    @ D SmartCache master valid bits
    mrc     p15,0,r10,c9,c1,0          @ D TCM Region
    tst     r10,#1 << 1                @ SmartCache?
    beq     no_dsc
    and     r0,r10,#7 << 2             @ D TCM size in [4:2]
    mrc     p15,3,r2,c15,c14,0         @ D SC master valid 0
    str     r2,[r1],#4                 @ Save
    cmp     r0,#5 << 2                 @ D TCM >= 16k?
    mrchs   p15,3,r3,c15,c14,1         @ D SC master valid 1
    strhs   r3,[r1],#4                 @ Save
    cmp     r0,#6 << 2                 @ D TCM >= 32k?
    mrchs   p15,3,r4,c15,c14,2         @ D SC master valid 2
    mrchs   p15,3,r5,c15,c14,3         @ D SC master valid 3
    stmhsia r1!,{r4-r5}                @ Save
    cmp     r0,#7 << 2                 @ D TCM = 64k?
    mrchs   p15,3,r6,c15,c14,4         @ D SC master valid 4
    mrchs   p15,3,r7,c15,c14,5         @ D SC master valid 5
    mrchs   p15,3,r8,c15,c14,6         @ D SC master valid 6
    mrchs   p15,3,r9,c15,c14,7         @ D SC master valid 7
    stmhsia r1!,{r6-r9}                @ Save
no_dsc:
    str     r10,[r1],#4                @ Save D TCM Region

    @ I SmartCache master valid bits
    mrc     p15,0,r10,c9,c1,1          @ I TCM Region
    tst     r10,#1 << 1                @ SmartCache?
    beq     no_isc
    and     r0,r10,#7 << 2             @ I TCM size in [4:2]
    mrc     p15,3,r2,c15,c10,0         @ I SC master valid 0
    str     r2,[r1],#4                 @ Save
    cmp     r0,#5 << 2                 @ I TCM >= 16k?
    mrchs   p15,3,r3,c15,c10,1         @ I SC master valid 1
    strhs   r3,[r1],#4                 @ Save
    cmp     r0,#6 << 2                 @ I TCM >= 32k?
    mrchs   p15,3,r4,c15,c10,2         @ I SC master valid 2
    mrchs   p15,3,r5,c15,c10,3         @ I SC master valid 3
    stmhsia r1!,{r4-r5}                @ Save
    cmp     r0,#7 << 2                 @ I TCM = 64k?
    mrchs   p15,3,r6,c15,c10,4         @ I SC master valid 4
    mrchs   p15,3,r7,c15,c10,5         @ I SC master valid 5
    mrchs   p15,3,r8,c15,c10,6         @ I SC master valid 6
    mrchs   p15,3,r9,c15,c10,7         @ I SC master valid 7
    stmhsia r1!,{r6-r9}                @ Save
no_isc:
    str     r10,[r1],#4                @ Save I TCM Region
.endif /* SAVE_CACHE_MASTER_VALID_REGS */

  @ It is optional whether the Main TLB RAM remains powered up in Dormant
  @ mode. If it is powered up in Dormant mode in your implementation, save
  @ the Main TLB master valid bits.
.if TLB_POWERED==1
    @ The Main TLB is fixed size so the number of TLB master
    @ valid bits is fixed at 64 i.e. 2 registers.
    mrc     p15,5,r2,c15,c14,0         @ TLB master valid 0
    mrc     p15,5,r3,c15,c14,1         @ TLB master valid 1
    stmia   r1!,{r2-r3}                @ Save
.endif

    @ Save the offset at the end of the state saving.
    @ Save this at a known fixed address so we can restore from
    @ it.
    ldr     r0,=bcm_pm_sleep_buf_phys     @ Virt addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r3,page_offset                @ Convert it to phys
    ldr     r2,phys_offset
    sub     r0,r0,r3
    add     r0,r0,r2                      @ Phys addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r0,[r0]                       @ Get the dorm buffer phys addr

    add     r2,r0,#PM_DORMANT_STORE
    sub     r1,r1,r2
    str     r1,[r0,#PM_DORMANT_STORE_END]

    @ Set up your power controller so that it will enter dormant
    @ mode when it next sees STANDBYWFI macrocell output go high.
    @ When this occurs, the power controller must drive the
    @ ARM1136's reset inputs low and hold them low while it
    @ removes the power supply to the logic, but not the RAMs.
    @ When the power controller wishes to exit dormant mode, it
    @ must drive the ARM1136's reset inputs low and hold them low
    @ while it restores the power supply to the ARM1136.  When it
    @ de-asserts the reset inputs, the reset code must identify
    @ that it is coming out of dormant mode and branch to the
    @ dormant mode restore code to restore system state.

    @ The validation world uses a memory-mapped trickbox feature
    @ to model the power controller:
@    ldr     r1,=TB_BASE               @ Set up the dormant
@    ldr     r0,=0x3000                @ mode trickbox feature
@    str     r0,[r1,#0xA4]

    @ Clean the entire data cache
@    mov     r1,#0
@    mcr     p15,0,r1,c7,c10,0

dormant_setup:
    @ All registers are now saved, if enabled perform checksum
    @ on the saved data
    ldr     r1,[r0,#PM_DORMANT_VERIFY]
    cmp     r1,#0
    beq     no_checksum

    @ Start address of save buffer
    add     r1,r0,#PM_DORMANT_STORE

    @ End address of save buffer
    ldr     r4,[r0,#PM_DORMANT_STORE_END]
    add     r4,r4,r1

    mov     r2,#0                      @ Clear running sum
checksum:
    ldr     r3,[r1],#0x04
    add     r2,r2,r3
    cmp     r4,r1
    bne     checksum
    str     r2,[r0,#PM_CHECKSUM_ENTER] @ Save checksum for
                                       @ comparison in exit routine
no_checksum:
    ldr     r1,[r0,#PM_DORMANT_SAVE_STAT]
    orr     r1,r1,#0x10
    str     r1,[r0,#PM_DORMANT_SAVE_STAT]

    @ Check for pending interrupts before entering into dormant
    @ mode
@    ldr     r1,=(ADDR_IRQ_IMR+4)
@    ldr     r2,[r1]
@    ldr     r1,=ADDR_IRQ_IMR
@    ldr     r3,[r1]
@    ands    r3,r3,r2
@    bne     resume_int_pending
@    ldr     r1,=(ADDR_IRQ_IMR1+4)
@    ldr     r2,[r1]
@    ldr     r1,=ADDR_IRQ_IMR1
@    ldr     r3,[r1]
@    ands    r3,r3,r2
@    bne     resume_int_pending
@    ldr     r1,=(ADDR_IRQ_IMR2+4)
@    ldr     r2,[r1]
@    ldr     r1,=ADDR_IRQ_IMR2
@    ldr     r3,[r1]
@    ands    r3,r3,r2
@    bne     resume_int_pending

.if BCM_PM_DORMANT_VERIFY_CRC==1
    @ Drain the write buffer
    mov     r0,#0
    mcr     p15,0,r0,c7,c10,4

    @ Calculate DDR data checksum
    ldr     r0,crc_ddr_start_p
    ldr     r1,crc_ddr_sz
    ldr     r2,crc_table_p
    ldr     r3,crc_sram_base_p
    ldr     r3,[r3]
    blx     r3

    @ Store the checksum in SRAM itself (it is powered ON in dormant and
    @ it is non-cached memory
    ldr     r1,crc_sram_base_p
    ldr     r1,[r1]
    ldr     r2,crc_dorm_entry
    add     r1,r1,r2
    str     r0,[r1]
.endif /* BCM_PM_DORMANT_VERIFY_CRC */

    /*
     * The following three instructions are added to debug the dormant
     * save-restore sequence.
     *
     * If bcm215xx_pm_sleep_buffer.dormant_sequence_debug is 1, then
     * WFI instruction is bypassed and control is transferred directly
     * to the dormant restore sequence.
     *
     * If bcm215xx_pm_sleep_buffer.dormant_sequence_debug is 0, then
     * WFI instruction is executed.
     */
    ldr     r0,=bcm_pm_sleep_buf_phys     @ Virt addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r3,page_offset                @ Convert it to phys
    ldr     r2,phys_offset
    sub     r0,r0,r3
    add     r0,r0,r2                      @ Phys addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r0,[r0]
    ldr     r1,[r0,#PM_DORMANT_SEQUENCE_DBG]
    cmp     r1,#1
    beq     resume_wfi_fail

.if BCM_PM_DORMANT_PROFILING==1
    /* toggle BCM_PM_DORMANT_DEBUG_GPIO */
    ldr     r1,gpor0_phys
    ldr     r2,[r1]
    eor     r2,r2,#(1 << BCM_PM_DORMANT_DEBUG_GPIO)
    str     r2,[r1]
.endif

    mov     r1,r0                         @ Save dorm buf address
    ldr     r2,[r1,#PM_L2_EVICT_BUF]      @ evict buffer address

    mov     r0,#0
    str     r0,[r2]                       @ Dummy write
    mcr     p15,0,r0,c7,c10,4             @ DSB

    @ Adding delay before entering WFI mode
    mov     r0,#150
    mov     r0,r0,lsl #3
kernel_delaybyloop2:
    subs    r0, r0, #1
    bne     kernel_delaybyloop2

    mov     r0,#0
    ldr     r3,[r2]                       @ Dummy read
    mcr     p15,0,r0,c7,c10,4             @ DSB

    @ Wait for interrupt (asserts STANDBYWFI macrocell output)
    mcr     p15,0,r0,c7,c0,4

    @ In case of dormant mode abort, the following code runs in L1 I-cache,
    @ thus no bus transactions. A reset will happen during the following NOP
    @ code.
    @ Hui Luo, 1/4/11
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1
    mov     r1,r1

    b       resume_wfi_fail

    .align 5
dormant_save_preload_end:

  @ We come here if there are any pending interrupts. MMU is still
  @ disabled. Branch to dormant restore function using physical
  @ address.
resume_int_pending:
@    ldr     r0,=bcm_pm_sleep_buf_phys     @ Virt addr of bcm_pm_sleep_buf_phys
@                                          @ variable
@    ldr     r3,page_offset                @ Convert it to phys
@    ldr     r2,phys_offset
@    sub     r0,r0,r3
@    add     r0,r0,r2                      @ Phys addr of bcm_pm_sleep_buf_phys
@                                          @ variable
@    ldr     r0,[r0]

@    ldr     r2,[r0,#PM_DORMANT_INT_CNT]
@    add     r2,r2,#1
@    str     r2,[r0,#PM_DORMANT_INT_CNT]

resume_wfi_fail:
    adr     r1,bcm215xx_dormant_wake_mmu
    blx     r1

quit_dormant_entry:
    @ We come here if either DMA channel has stopped with an error.
    @ Restore any registers we have changed and return.
    ldr     r0,=bcm_pm_sleep_buf
    ldr     r0,[r0]
    ldr     r2,[r0,#PM_DORMANT_SAVE_STAT]
    orr     r2,r2,#0x800
    str     r2,[r0,#PM_DORMANT_SAVE_STAT]

    mcr     p15,0,r3,c11,c2,0             @ Restore DMA channel no
    add     r1,r0,#PM_DORMANT_STORE       @ Get state save addr
    ldr     r2,[r1,#0x90]                 @ Load TLB lockdown
    mcr     p15,0,r2,c10,c0,0             @ Restore TLB lockdown
@    ldr     r2,[r1,#0x3C]                 @ Load CPSR
@    msr     CPSR_cxsf,r2                  @ Restore CPSR
@    ldmia   r1,{r0-r14}                   @ Restore r0-r14 (SYS)
    mov     pc,r14                        @ Return

/* END OF DORMANT MODE ENTRY CODE */

.if BCM_PM_DORMANT_VERIFY_CRC==1
ENTRY(crc32_test)
    stmfd   sp!,{r1-r12,lr}
    ldr     r3,crc_sram_base_v
    ldr     r3,[r3]
    blx     r3
    ldmfd   sp!,{r1-r12,pc}

/* Function to checksum the DDR contents. Following are the
 * requirements for the CRC function:
 * 1. This piece of code runs from SRAM. So all data loads
 *    and stores must be pc-relative.
 * 2. The implementation must be fast, hence look-up table
 *    method is adopted.
 * 3. The computed checksum must be stored to non-cached memory
 *    by the caller.
 *
 * Expects:
 * R0 - Start address of memory to be checksummed
 * R1 - Size of memory in bytes
 * R2 - CRC32 table pointer
 * LR - Function return address
 *
 * Returns:
 * R0 - Computed checksum
 *
 * Modifies:
 * R1-R5
 */
    .global bcm_crc32
    .global bcm_crc32_sz
    .align 5
bcm_crc32:
    cmp     r1,#0
    ble     2f
    mov     r4,r2                         @ crc_table
    mov     r2,#0                         @ i - buffer index
    mvn     r3,r2                         @ c = 0xFFFFFFFF
1:
    ldrb    r5,[r0,r2]                    @ buf[i]
    add     r2,r2,#1                      @ i++
    eor     r5,r3,r5                      @ c ^ buf[i]
    and     r5,r5,#0xFF                   @ (c ^ buf[i]) & 0xFF
    ldr     r5,[r4,r5,lsl #2]             @ b = crc_table[(c ^ buf[i]) & 0xFF]
    eor     r3,r5,r3,lsr #8               @ c = b ^ (c >> 8)
    cmp     r2,r1
    bne     1b
2:
    mov     r0,r3
    mov     pc,lr

    .ltorg
/* CRC checksum calculation related data */
crc_ddr_sz:
    .word    CRC_DDR_SZ
crc_sram_start:
    .word    CRC_SRAM_START
crc_dorm_entry:
    .word    CRC_DORM_ENTRY
crc_dorm_exit:
    .word    CRC_DORM_EXIT
crc_ddr_start_p:
    .word    CONFIG_SDRAM_BASE_ADDR
crc_ddr_start_v:
    .word    CONFIG_PAGE_OFFSET
crc_sram_base_p:
    .word    BCM21553_SCRATCHRAM_BASE
crc_sram_base_v:
    .word    bcm_crc32_sram_base
crc_table_v:
    .word    bcm_crc32_table
crc_table_p:
    .word    MEM_V2P(bcm_crc32_table)
bcm_crc32_sz:
    .word    . - bcm_crc32
.endif /* BCM_PM_DORMANT_VERIFY_CRC */

    .ltorg
.if ENABLE_L2X0_CODE==1
l210_config:
    .word    BCM21553_L210_CONFIG
l210_base_ro_v:
    .word    (HW_L210_BASE + 0x1000)
l210_base_wo_v:
    .word    (HW_L210_BASE)
l210_base_ro_phy:
    .word    (0x84a1000 + 0x1000)
l210_base_wo_phy:
    .word    0x84a1000

.endif /* ENABLE_L2X0_CODE */

gpor0:
    .word    (HW_GPIO_BASE + GPIO_GPOPS0_OFF)
gpor0_phys:
    .word    (BCM21553_GPIO_BASE + GPIO_GPOPS0_OFF)
page_offset:
    .word    CONFIG_PAGE_OFFSET
phys_offset:
    .word    CONFIG_SDRAM_BASE_ADDR

/****************************************************************************
 *      Get address of dormant wakeup function
 ****************************************************************************/

ENTRY(bcm215xx_dorm_wake_handler)
    adr     r0,bcm215xx_dormant_wake
    mov     pc,r14

/****************************************************************************
 *      Dormant mode exit
 ****************************************************************************/

/* This code assumes that the reset exception handler determines (by whatever
 * means) that the reset is due to exiting dormant mode, and the reset handler
 * then branches (by any means) to this dormant mode exit code.  The reset
 * handler does not need to configure the ARM1136 in any way before this.  The
 * dormant mode exit code assumes that it is entered in a privileged mode.
 *
 * The dormant mode exit code restores all required state in the correct order
 * (generally the opposite order from that in which it was saved).
 * You must make some modifications to this code as explained by the comments
 * within it.  When all state is restored, the dormant mode exit code restores
 * the original CPSR, thus entering System mode (as dormant mode entry was
 * called in System mode), and returns to the instruction after the the
 * original call to the dormant mode entry code, using the following
 * instruction:
 *     MOV pc,r14
 *
 * This assumes that the dormant mode entry code was called in System mode
 * using an instruction of the form:
 *     BL dormant_entry
 *
 * NOTE: dormant_wake is not defined as a function. Otherwise the linker
 * removes this code.
 */

/* Information about the dormant mode entry code is given at the start of that
 * code.
 */
/*
 * MMU is disabled at this point. So restore state using
 * the physical address of the restore buffer.
 */
    .extern bcm_pm_sleep_buf
    .extern bcm_pm_sleep_buf_phys

bcm215xx_dormant_wake:
.if BCM_PM_DORMANT_PROFILING==1
    /* toggle BCM_PM_DORMANT_DEBUG_GPIO */
    ldr     r1,gpor0_phys
    ldr     r2,[r1]
    eor     r2,r2,#(1 << BCM_PM_DORMANT_DEBUG_GPIO)
    str     r2,[r1]
.endif

.if BCM_PM_DORMANT_VERIFY_CRC==1
    @ Calculate DDR contents checksum
    ldr     r0,crc_ddr_start_p
    ldr     r1,crc_ddr_sz
    ldr     r2,crc_table_p
    ldr     r3,crc_sram_base_p
    ldr     r4,crc_sram_start
    add     r3,r3,r4
    blx     r3

    @ Store the checksum in SRAM itself (it is powered ON in dormant and
    @ it is non-cached memory)
    ldr     r1,crc_sram_base_p
    ldr     r2,crc_dorm_exit
    add     r1,r1,r2
    str     r0,[r1]
.endif /* BCM_PM_DORMANT_VERIFY_CRC */

    ldr     r0,=bcm_pm_sleep_buf_phys     @ Virt addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r1,page_offset                @ Convert it to phys
    ldr     r2,phys_offset
    sub     r0,r0,r1
    add     r0,r0,r2                      @ Phys addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r0,[r0]                       @ Get the dorm buffer phys addr

    @ Increment the sleep_mode_count field of the sleep_buffer
    @ structure to indicate successful etry-exit from dormant
    @ mode
    ldr     r1,[r0,#PM_DORMANT_COUNT]
    add     r1,r1,#1
    str     r1,[r0,#PM_DORMANT_COUNT]

    b       dormant_restore

    @ Control reaches here if AP comes out of WFI for reasons
    @ other than normal dormant wakeup sequence or if dormant sequence
    @ debug is turned ON. In this case the MMU is still on and the
    @ context buffer must be accessed using virtual address.
bcm215xx_dormant_wake_mmu:
    ldr     r0,=bcm_pm_sleep_buf_phys     @ Virt addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r1,page_offset                @ Convert it to phys
    ldr     r2,phys_offset
    sub     r0,r0,r1
    add     r0,r0,r2                      @ Phys addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r0,[r0]                       @ Get the dorm buffer phys addr

dormant_restore:
    @ Take a backup of r0 (the dormant buffer address)
    str     r0,r0_store

    @ Assume we are in a privileged mode.
    @ Enter system mode and ensure interrupts are disabled
    cpsid   aif,#MODE_SYS

    ldr     r1,[r0,#PM_DORMANT_EXIT_STAT]
    cmp     r1,#0
    beq     failtst
    str     r1,[r0,#PM_DORMANT_FAIL_STAT]
    ldr     r1,[r0,#PM_DORMANT_FAIL_CNT]
    add     r1,r1,#1
    str     r1,[r0,#PM_DORMANT_FAIL_CNT]
failtst:
    mov     r1,#0x02
    str     r1,[r0,#PM_DORMANT_EXIT_STAT]

    ldr     r1,[r0,#PM_DORMANT_EXIT_CNT]  @ Increment wake count
    add     r1,r1,#1
    str     r1,[r0,#PM_DORMANT_EXIT_CNT]

checksum_verify_rst:
    @ If enabled perform checksum on the
    @ saved off data
    @ Check to see if verify is necessary
    ldr     r1,[r0,#PM_DORMANT_VERIFY]
    cmp     r1,#0
    beq     no_checksum_rst

    @ Start address of save buffer
    add     r1,r0,#PM_DORMANT_STORE

    @ End address of save buffer
    ldr     r4,[r0,#PM_DORMANT_STORE_END]
    add     r4,r4,r1

    mov     r2,#0                         @ Clear running sum
checksum_rst:
    ldr     r3,[r1],#0x04
    add     r2,r2,r3
    cmp     r1,r4
    bne     checksum_rst
    str     r2,[r0,#PM_CHECKSUM_EXIT]     @ Save exit checksum

    @ Compare start and exit checksums
    ldr     r3,[r0,#PM_CHECKSUM_ENTER]
    cmp     r2,r3
    beq     no_checksum_rst

    @ Increment failed checksum count
    ldr     r2,[r0,#PM_CHECKSUM_COMPARE]
    add     r2,r2,#0x1
    str     r2,[r0,#PM_CHECKSUM_COMPARE]

no_checksum_rst:
    @ Load the address at the end of the state save memory region
    add     r1,r0,#PM_DORMANT_STORE
    ldr     r4,[r0,#PM_DORMANT_STORE_END]
    add     r1,r1,r4

  @ It is optional whether the Main TLB RAM remains powered up in Dormant
  @ mode. If it is powered up in Dormant mode in your implementation, restore
  @ the Main TLB master valid bits.
.if TLB_POWERED==1
    @ The Main TLB is fixed size so the number of TLB master
    @ valid bits is fixed at 64 i.e. 2 registers.
    ldmdb   r1!,{r2-r3}                   @ Load
    mcr     p15,5,r3,c15,c14,1            @ TLB master valid 1
    mcr     p15,5,r2,c15,c14,0            @ TLB master valid 0
.endif
    mov     r2,#0
    mcr     p15,0,r2,c8,c5,0              @ Invalidate instruction TLB
    mcr     p15,0,r2,c8,c6,0              @ Invalidate data TLB
    mcr     p15,0,r2,c8,c7,0              @ Invalidate unified TLB

.if SAVE_CACHE_MASTER_VALID_REGS==1
    @ If TCMs were enabled as SmartCache, restore the SmartCache
    @ master valid bits and the TCM region registers.
    @ If TCMs are not enabled as SmartCache, we do not need to
    @ restore the SmartCache master valid bits, just the TCM
    @ region regs.
    @ This code checks if TCMs were enabled as SmartCache.
    @ If your code runs on a system that never has TCMs, you can
    @ remove this code@ if your system has TCMs and always uses
    @ SmartCache, you can remove the check@ if your system has
    @ TCMs and never uses SmartCache, you can remove this code
    @ except for the restoring of the TCM region regs.
    @ Note: saving and restoring the TCM region regs on a system
    @ with no TCMs has no effect, and is easier and quicker than
    @ testing if we have TCMs.
    @ Restore the SmartCache master valid registers
    @ Number of registers to restore depends on the TCM size:
    @ 4k TCM  : restore reg 0
    @ 8k      :             0
    @ 16k     :             0-1
    @ 32k     :             0-3
    @ 64k     :             0-7
    @ Here we get the TCM sizes from the restored TCM region reg.
    @ If you know that your TCM sizes are fixed you can remove
    @ the checking and just restore the regs you need.
    @ Alternatively you can restore all regs 0-7 regardless of the
    @ TCM size - restoring unused regs has no effect. However,
    @ this impacts the performance due to extra MCRs and reads.
    @ Note: we must restore the TCM region regs before the
    @ SmartCache master valid bits, as a write to a TCM region reg
    @ automatically clears all SmartCache master valid bits for
    @ that TCM.
    @ I SmartCache master valid bits
    ldr     r10,[r1,#-4]!                 @ Load I TCM Region
    mcr     p15,0,r10,c9,c1,1             @ I TCM Region
    tst     r10,#1 << 1                   @ SmartCache?
    beq     no_isc_rst                    @ No: skip valid bits
    and     r0,r10,#7 << 2                @ I TCM size in [4:2]
    cmp     r0,#7 << 2                    @ I TCM = 64k?
    ldmhsdb r1!,{r6-r9}                   @ Load
    mcrhs   p15,3,r9,c15,c10,7            @ I SC master valid 7
    mcrhs   p15,3,r8,c15,c10,6            @ I SC master valid 6
    mcrhs   p15,3,r7,c15,c10,5            @ I SC master valid 5
    mcrhs   p15,3,r6,c15,c10,4            @ I SC master valid 4
    cmp     r0,#6 << 2                    @ I TCM >= 32k?
    ldmhsdb r1!,{r4-r5}                   @ Load
    mcrhs   p15,3,r5,c15,c10,3            @ I SC master valid 3
    mcrhs   p15,3,r4,c15,c10,2            @ I SC master valid 2
    cmp     r0,#5 << 2                    @ I TCM >= 16k?
    ldrhs   r3,[r1,#-4]!                  @ Load
    mcrhs   p15,3,r3,c15,c10,1            @ I SC master valid 1
    ldr     r2,[r1,#-4]!                  @ Load
    mcr     p15,3,r2,c15,c10,0            @ I SC master valid 0
no_isc_rst:
    @ D SmartCache master valid bits
    ldr     r10,[r1,#-4]!                 @ Load D TCM Region
    mcr     p15,0,r10,c9,c1,0             @ D TCM Region
    tst     r10,#1 << 1                   @ SmartCache?
    beq     no_dsc_rst                    @ No: skip valid bits
    and     r0,r10,#7 << 2                @ D TCM size in [4:2]
    cmp     r0,#7 << 2                    @ D TCM = 64k?
    ldmhsdb r1!,{r6-r9}                   @ Load
    mcrhs   p15,3,r9,c15,c14,7            @ D SC master valid 7
    mcrhs   p15,3,r8,c15,c14,6            @ D SC master valid 6
    mcrhs   p15,3,r7,c15,c14,5            @ D SC master valid 5
    mcrhs   p15,3,r6,c15,c14,4            @ D SC master valid 4
    cmp     r0,#6 << 2                    @ D TCM >= 32k?
    ldmhsdb r1!,{r4-r5}                   @ Load
    mcrhs   p15,3,r5,c15,c14,3            @ D SC master valid 3
    mcrhs   p15,3,r4,c15,c14,2            @ D SC master valid 2
    cmp     r0,#5 << 2                    @ D TCM >= 16k?
    ldrhs   r3,[r1,#-4]!                  @ Load
    mcrhs   p15,3,r3,c15,c14,1            @ D SC master valid 1
    ldr     r2,[r1,#-4]!                  @ Load
    mcr     p15,3,r2,c15,c14,0            @ D SC master valid 0
no_dsc_rst:

    @ Restore the cache master valid registers
    @ Number of registers to restore depends on the cache size:
    @ 4k cache: restore reg 0
    @ 8k      :             0
    @ 16k     :             0-1
    @ 32k     :             0-3
    @ 64k     :             0-7
    @ Here we get the cache sizes from the cache type reg.
    @ If you know that your cache sizes are fixed you can remove
    @ the checking and just restore the regs you need.
    @ Alternatively you can restore all regs 0-7 regardless of the
    @ cache size - restoring unused regs has no effect. However,
    @ this impacts the performance due to extra MRCs and reads.
    @ I cache master valid bits
    mrc     p15,0,r0,c0,c0,1              @ Cache type
    and     r10,r0,#7 << 6                @ Icache size in [8:6]
    cmp     r10,#7 << 6                   @ Icache = 64k?
    ldmhsdb r1!,{r6-r9}                   @ Load
    mcrhs   p15,3,r9,c15,c8,7             @ Icache master valid 7
    mcrhs   p15,3,r8,c15,c8,6             @ Icache master valid 6
    mcrhs   p15,3,r7,c15,c8,5             @ Icache master valid 5
    mcrhs   p15,3,r6,c15,c8,4             @ Icache master valid 4
    cmp     r10,#6 << 6                   @ Icache >= 32k?
    ldmhsdb r1!,{r4-r5}                   @ Load
    mcrhs   p15,3,r5,c15,c8,3             @ Icache master valid 3
    mcrhs   p15,3,r4,c15,c8,2             @ Icache master valid 2
    cmp     r10,#5 << 6                   @ Icache >= 16k?
    ldrhs   r3,[r1,#-4]!                  @ Load
    mcrhs   p15,3,r3,c15,c8,1             @ Icache master valid 1
    ldr     r2,[r1,#-4]!                  @ Load
    mcr     p15,3,r2,c15,c8,0             @ Icache master valid 0

    @ D cache master valid bits
    and     r10,r0,#7 << 18               @ Dcache size in [8:6]
    cmp     r10,#7 << 18                  @ Dcache = 64k?
    ldmhsdb r1!,{r6-r9}                   @ Load
    mcrhs   p15,3,r9,c15,c12,7            @ Dcache master valid 7
    mcrhs   p15,3,r8,c15,c12,6            @ Dcache master valid 6
    mcrhs   p15,3,r7,c15,c12,5            @ Dcache master valid 5
    mcrhs   p15,3,r6,c15,c12,4            @ Dcache master valid 4
    cmp     r10,#6 << 18                  @ Dcache >= 32k?
    ldmhsdb r1!,{r4-r5}                   @ Load
    mcrhs   p15,3,r5,c15,c12,3            @ Dcache master valid 3
    mcrhs   p15,3,r4,c15,c12,2            @ Dcache master valid 2
    cmp     r10,#5 << 18                  @ Dcache >= 16k?
    ldrhs   r3,[r1,#-4]!                  @ Load
    mcrhs   p15,3,r3,c15,c12,1            @ Dcache master valid 1
    ldr     r2,[r1,#-4]!                  @ Load
    mcr     p15,3,r2,c15,c12,0            @ Dcache master valid 0
.endif /* SAVE_CACHE_MASTER_VALID_REGS */

  @ If Dormant mode does not have its own static memory setup, restore memory
  @ system control registers from main memory.
  @ If it does have its own setup, set it up now.
.if DM_STATICMEM==0
    @ ARM1136 rev1 has two extra memory system registers
    mrc     p15,0,r0,c0,c0,0              @ Read ID code
    ands    r0,r0,#0xF << 20              @ Is this a rev0 device?
    beq     rev0_skip4                    @ Yes: skip
    ldmdb   r1!,{r2-r3}                   @ Load
    mcr     p15,0,r3,c10,c2,1             @ Normal Memory Remap
    mcr     p15,0,r2,c10,c2,0             @ Primary Region Remap
rev0_skip4:
    @ BRCM customization: before enabling MMU and L1 I/D cache,
    @ invalidate them entirely
    @ Hui Luo, 1/4/11
    mov     r0, #0
    mcr     p15,0,r0,c7,c7,0              @ Invalidate entire I/D-cache;
                                          @ must be done before enabling MMU

    ldmdb   r1!,{r2-r9}                   @ Load
    mcr     p15,0,r9,c9,c0,1              @ I Cache Lockdown
    mcr     p15,0,r8,c9,c0,0              @ D Cache Lockdown
    mcr     p15,7,r7,c15,c1,0             @ TLB Debug Control
    mcr     p15,7,r6,c15,c0,0             @ Cache Debug Control
    mcr     p15,0,r5,c15,c2,4             @ PP Mem Remap
    mcr     p15,0,r4,c15,c2,2             @ DMA Mem Remap
    mcr     p15,0,r3,c15,c2,1             @ Instr Mem Remap
    mcr     p15,0,r2,c15,c2,0             @ Data Mem Remap
    ldmdb   r1!,{r2-r8}                   @ Load
    mcr     p15,0,r8,c13,c0,1             @ Context ID
    mcr     p15,0,r7,c13,c0,0             @ FCSE PID
    mcr     p15,0,r6,c3,c0,0              @ DAC
    mcr     p15,0,r5,c2,c0,2              @ TTBCR
    mcr     p15,0,r4,c2,c0,1              @ TTBR1
    mcr     p15,0,r3,c2,c0,0              @ TTBR0

    orr     r2,r2,#CP15_CTRL_MMU_BIT      @ Control reg in r2
    orr     r2,r2,#CP15_CTRL_DCACHE_BIT
    orr     r2,r2,#CP15_CTRL_ICACHE_BIT

.if ENABLE_L2X0_CODE==1
    @ BRCM customization: restore L2 cache and re-enable branch prediction
    @ Re-programming L2 cache is needed because it is reset
    @ Hui Luo, 1/4/11
    ldr     r0,=bcm_pm_sleep_buf_phys     @ Virt addr of bcm_pm_sleep_buf_phys variable
    ldr     r3,page_offset                @ Convert it to phys
    sub     r0,r0,r3
    ldr     r3,phys_offset
    add     r0,r0,r3                      @ Phys addr of bcm_pm_sleep_buf_phys variable
    ldr     r0,[r0]                       @ Get the dorm buffer phys addr
    ldr     r0,[r0,#PM_L2CACHE_INIT_FLAG]
    cmp     r0,#0
    beq     l2_invalidate_skipped
    ldr     r0,l210_base_wo_phy
    ldr     r3,l210_config
    str     r3,[r0,#L2X0_AUX_CTRL]        @ Config L2; load MMU descriptor for L2 control register in TLB cache
    mov     r3,#1
    str     r3,[r0,#L2X0_CTRL]            @ Enable L2
l2_invalidate_skipped:
.endif /* ENABLE_L2X0_CODE */

    @ Load the virt address of ret_from_func in r3
    adr     r3,ret_from_dormant           @ Physical address of ret_from_dormant in r3
    ldr     r0,phys_offset
    sub     r3,r3,r0
    ldr     r0,page_offset
    add     r3,r3,r0                      @ Virtual address of ret_from-dormant in r3

    @ Turn on mmu
    @ Sequence from arch/arm/kernel/head.S
    b       bcm215xx_turn_on_mmu

    .align 5
bcm215xx_turn_on_mmu:
    @ The following NOPs are required for the enable MMU sequence
    @ to work correctly. arch/arm/kernel/head.S uses only one NOP.
    mov     r2,r2
    mcr     p15,0,r2,c1,c0,0              @ Write control reg
    mrc     p15,0,r2,c0,c0,0              @ Read id reg
    mov     r2,r2
    mov     r2,r2
    mov     pc,r3                         @ jump to virtual address of ret_from_dormant
    mov     r2,r2
    mov     r2,r2

ret_from_dormant:
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2

    mov     r2,#0
    mcr     p15,0,r2,c7,c10,4             @ DSB
    mcr     p15,0,r2,c7,c5,6              @ Flush entire branch prediction cache
    mcr     p15,0,r2,c7,c5,4              @ Flush prefetch buffer

    @ Since MMU is enabled, from this point forward r1
    @ should have virtual address of the dormant buffer
    ldr     r0,=bcm_pm_sleep_buf_phys
    ldr     r0,[r0]
    @ Get the current offset of the current location pointed by
    @ R1 in dormant buffer.
    add     r0,r0,#PM_DORMANT_STORE    @ Start of dormant store area phys
    sub     r0,r1,r0                   @ Offset = Current - Start
    ldr     r1,=bcm_pm_sleep_buf
    ldr     r1,[r1]
    add     r1,r1,#PM_DORMANT_STORE    @ Start of dormant store area virt
    add     r1,r1,r0                   @ Current dormant buf address virt

    mrc     p15,0,r0,c1,c0,0              @ Read ctrl reg
    orr     r2,r0,#CP15_CTRL_BPRED_BIT    @ Enable branch prediction
    mcr     p15,0,r2,c1,c0,0              @ Write ctrl reg

  @ Now set up the static memory configuration for Dormant mode.
  @ This may involve writing TTBR regs to use new page tables, or remapping
  @ memory to write-back.
.endif /* DM_STATICMEM */

  @ Note: if you have a separate static memory setup for dormant mode,
  @ the above state will be restored later.
.if DISABLE_DEBUG_REGS_SAVE==0
    @ Restore the Debug registers
    mrc     p14,0,r0,c0,c1,0              @ Read DSCR
    orr     r0,r0,#1 << 15                @ Select & enable debug
    bic     r0,r0,#1 << 14                @ monitor mode
    mcr     p14,0,r0,c0,c1,0              @ Write DSCR
    ldmdb   r1!,{r2-r6}                   @ Load
    mcr     p14,0,r6,c0,c7,0              @ VTR
    mcr     p14,0,r5,c0,c1,7              @ WCR1
    mcr     p14,0,r4,c0,c0,7              @ WCR0
    mcr     p14,0,r3,c0,c1,6              @ WVR1
    mcr     p14,0,r2,c0,c0,6              @ WVR0
    ldmdb   r1!,{r2-r14}                  @ Load
    mcr     p14,0,r14,c0,c5,5             @ BCR5
    mcr     p14,0,r13,c0,c4,5             @ BCR4
    mcr     p14,0,r12,c0,c3,5             @ BCR3
    mcr     p14,0,r11,c0,c2,5             @ BCR2
    mcr     p14,0,r10,c0,c1,5             @ BCR1
    mcr     p14,0,r9,c0,c0,5              @ BCR0
    mcr     p14,0,r8,c0,c5,4              @ BVR5
    mcr     p14,0,r7,c0,c4,4              @ BVR4
    mcr     p14,0,r6,c0,c3,4              @ BVR3
    mcr     p14,0,r5,c0,c2,4              @ BVR2
    mcr     p14,0,r4,c0,c1,4              @ BVR1
    mcr     p14,0,r3,c0,c0,4              @ BVR0
    mcr     p14,0,r2,c0,c1,0              @ DSCR
.endif /* DISABLE_DEBUG_REGS_SAVE */

    @ Enable interrupts.
    @ This assumes that you have valid interrupt handlers in memory.
    @ Memory has not changed in dormant mode so this is a reasonable
    @ assumption.  However, you may have some other reason why you
    @ do not wish to enable interrupts here, in which case you can
    @ move this code to later in the dormant mode restore code.
    @ If you have a Vectored Interrupt Controller (VIC) in your
    @ system, you can set it up here.  If the configuration of the
    @ VIC is not static, you can save its state with the state of
    @ other memory-mapped peripherals, and restore it later together
    @ with restoring the state of other memory-mapped peripherals.
@    cpsie   aif

    @ Restore the Performance Monitor registers
    @ Note: performance monitor counts will now restart from
    @ their saved values.  They have not counted events between
    @ the time these registers were saved and this point in time,
    @ so the counts of events may not be correct.
    @ Note: Bits [10:8] of the performance monitor control register
    @ are overflow/interrupt flags.  These flags cannot be saved
    @ and restored correctly, and the flags will all be cleared
    @ after restoring these registers.
    ldmdb   r1!,{r2-r5}                   @ Load
    mcr     p15,0,r5,c15,c12,3            @ Count register 1
    mcr     p15,0,r4,c15,c12,2            @ Count register 0
    mcr     p15,0,r3,c15,c12,1            @ Cycle counter
    mcr     p15,0,r2,c15,c12,0            @ Performance mon ctrl

    @ Enable access to all coprocessors to restore coproc & VFP
    @ state
    @ You can omit this if you have no coprocessors and no VFP
    mov     r0,#0xFFFFFFFF                @ Enable all coprocs
    mcr     p15,0,r0,c1,c0,2              @ Write coproc acc ctrl

    @ Restore the registers of all coprocessors except the VFP.
    @ As all coprocessors are different, we cannot write generic
    @ code here to restore the registers of any coprocessor. If you
    @ have coprocessors other than the VFP, write your own code to
    @ restore all of their registers to memory. Otherwise, remove
    @ this section of code.
    @ The example code here restores the validation generic
    @ coprocessors p4 & p5.
    @ Note: you can test for the presence of coprocessors by reading
    @ the appropriate bits of the coprocessor access control reg,
    @ but you still need to know what state must be restored for
    @ each individual coprocessor present in the system.
@    ldc     p5,c15,[r1,#-4]!             @ Restore coproc p5 regs
@    ldc     p5,c14,[r1,#-4]!
@    ldc     p5,c13,[r1,#-4]!
@    ldc     p5,c12,[r1,#-4]!
@    ldc     p5,c11,[r1,#-4]!
@    ldc     p5,c10,[r1,#-4]!
@    ldc     p5,c9,[r1,#-4]!
@    ldc     p5,c8,[r1,#-4]!
@    ldc     p5,c7,[r1,#-4]!
@    ldc     p5,c6,[r1,#-4]!
@    ldc     p5,c5,[r1,#-4]!
@    ldc     p5,c4,[r1,#-4]!
@    ldc     p5,c3,[r1,#-4]!
@    ldc     p5,c2,[r1,#-4]!
@    ldc     p5,c1,[r1,#-4]!
@    ldc     p5,c0,[r1,#-4]!
@    ldc     p4,c15,[r1,#-4]!             @ Restore coproc p4 regs
@    ldc     p4,c14,[r1,#-4]!
@    ldc     p4,c13,[r1,#-4]!
@    ldc     p4,c12,[r1,#-4]!
@    ldc     p4,c11,[r1,#-4]!
@    ldc     p4,c10,[r1,#-4]!
@    ldc     p4,c9,[r1,#-4]!
@    ldc     p4,c8,[r1,#-4]!
@    ldc     p4,c7,[r1,#-4]!
@    ldc     p4,c6,[r1,#-4]!
@    ldc     p4,c5,[r1,#-4]!
@    ldc     p4,c4,[r1,#-4]!
@    ldc     p4,c3,[r1,#-4]!
@    ldc     p4,c2,[r1,#-4]!
@    ldc     p4,c1,[r1,#-4]!
@    ldc     p4,c0,[r1,#-4]!

    @ If we have a VFP coprocessor, restore the VFP system regs and
    @ register bank.
.if FPU_ENABLE==1
    @ The VFP will always be enabled by dormant entry code so restore context
    @ and disable here again for lazy context switching.
    fmrx    r0,fpexc          @ VFP Exception
    orr     r0,r0,#1<<30      @ Enable VFP
    fmxr    fpexc,r0          @ Write VFP Exception
    fldmdbx r1!,{d0-d15}      @ Restore VFP reg bank
    ldmdb   r1!,{r2-r5}       @ Restore
    fmxr    fpinst2,r5        @ VFP Instruction 2
    fmxr    fpinst,r4         @ VFP Instruction
    fmxr    fpscr,r3          @ VFP Status & Control
    fmxr    fpexc,r2          @ Write VFP Exception

    @ This may be called when FPU is already disabled and so CP access may
    @ already be disabled
    mrc     p15,0,r0,c1,c0,2  @ r0 = Access Control Register
    ands    r0,r0,#(0x5<<20)
    beq     fpu_disabled
    @ Disable VFP
    fmrx    r0,fpexc
    bic     r0,r0,#FPEXC_EN
    fmxr    fpexc,r0
    mrc     p15,0,r0,c1,c0,2  @ r0 = Access Control Register
    bic     r0,r0,#(0x5<<20)  @ disable priv access for p10,11
    mcr     p15,0,r0,c1,c0,2
fpu_disabled:
.endif /* FPU_ENABLE */

    @ Restore the state of any memory-mapped peripherals that are
    @ powered down in dormant mode.
    @ For example: the validation trickbox registers
    @ Replace this code with your own code to restore the state of
    @ your memory-mapped peripherals.
@    ldmdb   r1!,{r2-r4}       @ Load
@    ldr     r0,=TB_BASE       @ Trickbox base address
@    str     r2,[r0,#0x64]     @ Trickbox abort addr
@    str     r3,[r0,#0x44]     @ Trickbox abort range 1
@    str     r4,[r0,#0x40]     @ Trickbox abort range 2

    @ Restore lockdown region of TLB
    ldmdb   r1!,{r2-r13}                  @ Load

    mov     r0,#0x80000000
    add     r0,r0,#7
    mcr     p15,5,r0,c15,c4,4             @ Write lockdown entry 7
    mcr     p15,5,r13,c15,c7,2            @ Write Main TLB Attr
    mcr     p15,5,r12,c15,c6,2            @ Write Main TLB PA
    mcr     p15,5,r11,c15,c5,2            @ Write Main TLB VA

    sub     r0,r0,#1
    mcr     p15,5,r0,c15,c4,4             @ Write lockdown entry 6
    mcr     p15,5,r10,c15,c7,2            @ Write Main TLB Attr
    mcr     p15,5,r9,c15,c6,2             @ Write Main TLB PA
    mcr     p15,5,r8,c15,c5,2             @ Write Main TLB VA

    sub     r0,r0,#1
    mcr     p15,5,r0,c15,c4,4             @ Write lockdown entry 5
    mcr     p15,5,r7,c15,c7,2             @ Write Main TLB Attr
    mcr     p15,5,r6,c15,c6,2             @ Write Main TLB PA
    mcr     p15,5,r5,c15,c5,2             @ Write Main TLB VA

    sub     r0,r0,#1
    mcr     p15,5,r0,c15,c4,4             @ Write lockdown entry 4
    mcr     p15,5,r4,c15,c7,2             @ Write Main TLB Attr
    mcr     p15,5,r3,c15,c6,2             @ Write Main TLB PA
    mcr     p15,5,r2,c15,c5,2             @ Write Main TLB VA

    ldmdb   r1!,{r2-r13}                  @ Load

    sub     r0,r0,#1
    mcr     p15,5,r0,c15,c4,4             @ Write lockdown entry 3
    mcr     p15,5,r13,c15,c7,2            @ Write Main TLB Attr
    mcr     p15,5,r12,c15,c6,2            @ Write Main TLB PA
    mcr     p15,5,r11,c15,c5,2            @ Write Main TLB VA

    sub     r0,r0,#1
    mcr     p15,5,r0,c15,c4,4             @ Write lockdown entry 2
    mcr     p15,5,r10,c15,c7,2            @ Write Main TLB Attr
    mcr     p15,5,r9,c15,c6,2             @ Write Main TLB PA
    mcr     p15,5,r8,c15,c5,2             @ Write Main TLB VA

    sub     r0,r0,#1
    mcr     p15,5,r0,c15,c4,4             @ Write lockdown entry 1
    mcr     p15,5,r7,c15,c7,2             @ Write Main TLB Attr
    mcr     p15,5,r6,c15,c6,2             @ Write Main TLB PA
    mcr     p15,5,r5,c15,c5,2             @ Write Main TLB VA

    sub     r0,r0,#1
    mcr     p15,5,r0,c15,c4,4             @ Write lockdown entry 0
    mcr     p15,5,r4,c15,c7,2             @ Write Main TLB Attr
    mcr     p15,5,r3,c15,c6,2             @ Write Main TLB PA
    mcr     p15,5,r2,c15,c5,2             @ Write Main TLB VA


    @ If we have a DMA, restore DMA state (both channels) and
    @ restart any previously running and queued DMA channels
    @ This code checks if the DMA is present by reading the DMA ID
    @ & Status reg.  If your code runs on a system that never has
    @ an internal DMA engine you can remove this code section.
    mrc     p15,0,r0,c11,c0,0             @ Read DMA ID & Status
    cmp     r0,#3                         @ Do we have a DMA?
    bne     no_dma_rst                    @ No - skip DMA restore

    ldmdb   r1!,{r2-r13}                  @ Load
    mov     r0,#1
    mcr     p15,0,r0,c11,c2,0             @ Select Channel 1
    mcr     p15,0,r13,c11,c15,0           @ Context ID
    mcr     p15,0,r12,c11,c7,0            @ Internal End Address
    mcr     p15,0,r11,c11,c6,0            @ External Start Address
    mcr     p15,0,r10,c11,c5,0            @ Internal Start Address
    mcr     p15,0,r9,c11,c4,0             @ Control Register
    cmp     r8,#3                         @ Was channel 1 complete?
    mcreq   p15,0,r0,c11,c3,1             @ Yes: start the channel
    @ If the channel was queued or running, start it after channel 0.

    mov     r0,#0
    mcr     p15,0,r0,c11,c2,0             @ Select Channel 0
    mcr     p15,0,r7,c11,c15,0            @ Context ID
    mcr     p15,0,r6,c11,c7,0             @ Internal End Address
    mcr     p15,0,r5,c11,c6,0             @ External Start Address
    mcr     p15,0,r4,c11,c5,0             @ Internal Start Address
    mcr     p15,0,r3,c11,c4,0             @ Control Register
    tst     r2,#2                         @ Was channel 0 complete
                                          @  or running?
    mcrne   p15,0,r0,c11,c3,1             @ Yes: start the channel

    mov     r0,#1
    cmp     r8,#1                         @ Was channel 1 queued?
    cmpne   r8,#2                         @ No: was it running?
    mcreq   p15,0,r0,c11,c2,0             @ Yes: select channel 1@
    mcreq   p15,0,r0,c11,c3,1             @ start the channel

    mov     r0,#0
    cmp     r2,#1                         @ Was channel 0 queued?
    mcreq   p15,0,r0,c11,c2,0             @ Yes: select channel 0@
    mcreq   p15,0,r0,c11,c3,1             @ start the channel

    ldmdb   r1!,{r2-r3}                   @ Load
    mcr     p15,0,r3,c11,c2,0             @ Channel Number
    mcr     p15,0,r2,c11,c1,0             @ User Accessibility Reg
no_dma_rst:

    @ Restore CP15 control & fault registers
    @ ARM1136 rev1 has three extra thread/process ID registers
    mrc     p15,0,r0,c0,c0,0              @ Read ID code
    ands    r0,r0,#0xF << 20              @ Is this a rev0 device?
    beq     rev0_skip5                    @ Yes: skip
    ldmdb   r1!,{r2-r4}                   @ Load
    mcr     p15,0,r4,c13,c0,4             @ Priv Only Thread/PID
    mcr     p15,0,r3,c13,c0,3             @ User RO Thread/PID
    mcr     p15,0,r2,c13,c0,2             @ User R/W Thread/PID
rev0_skip5:
    ldmdb   r1!,{r2-r7}                   @ Load
    mcr     p15,0,r7,c1,c0,2              @ Coproc Access Control
    mcr     p15,0,r6,c1,c0,1              @ Auxiliary Control
    mcr     p15,0,r5,c6,c0,1              @ IFAR
    mcr     p15,0,r4,c6,c0,0              @ FAR
    mcr     p15,0,r3,c5,c0,1              @ IFSR
    mcr     p15,0,r2,c5,c0,0              @ DFSR

  @ If Dormant mode has its own static memory setup, restore the original memory
  @ system control registers.
  @ Note: if you do not have a separate static memory setup for dormant mode,
  @ this state has already been restored.
.if DM_STATICMEM==1
    @ ARM1136 rev1 has two extra memory system registers
    mrc     p15,0,r0,c0,c0,0              @ Read ID code
    ands    r0,r0,#0xF << 20              @ Is this a rev0 device?
    beq     rev0_skip6                    @ Yes: skip
    ldmdb   r1!,{r2-r3}                   @ Load
    mcr     p15,0,r3,c10,c2,1             @ Normal Memory Remap
    mcr     p15,0,r2,c10,c2,0             @ Primary Region Remap
rev0_skip6:
    ldmdb   r1!,{r2-r9}                   @ Load
    mcr     p15,0,r9,c9,c0,1              @ I Cache Lockdown
    mcr     p15,0,r8,c9,c0,0              @ D Cache Lockdown
    mcr     p15,7,r7,c15,c1,0             @ TLB Debug Control
    mcr     p15,7,r6,c15,c0,0             @ Cache Debug Control
    mcr     p15,0,r5,c15,c2,4             @ PP Mem Remap
    mcr     p15,0,r4,c15,c2,2             @ DMA Mem Remap
    mcr     p15,0,r3,c15,c2,1             @ Instr Mem Remap
    mcr     p15,0,r2,c15,c2,0             @ Data Mem Remap
    ldmdb   r1!,{r2-r8}                   @ Load
    mcr     p15,0,r8,c13,c0,1             @ Context ID
    mcr     p15,0,r7,c13,c0,0             @ FCSE PID
    mcr     p15,0,r6,c3,c0,0              @ DAC
    mcr     p15,0,r5,c2,c0,2              @ TTBCR
    mcr     p15,0,r4,c2,c0,1              @ TTBR1
    mcr     p15,0,r3,c2,c0,0              @ TTBR0

    orr     r2,r2,#CP15_CTRL_MMU_BIT      @ Control reg in r2
    orr     r2,r2,#CP15_CTRL_DCACHE_BIT
    orr     r2,r2,#CP15_CTRL_ICACHE_BIT

.if ENABLE_L2X0_CODE==1
    @ BRCM customization: restore L2 cache and re-enable branch prediction
    @ Re-programming L2 cache is needed because it is reset
    @ Hui Luo, 1/4/11
    ldr     r0,=bcm_pm_sleep_buf_phys     @ Virt addr of bcm_pm_sleep_buf_phys variable
    ldr     r3,page_offset                @ Convert it to phys
    sub     r0,r0,r3
    ldr     r3,phys_offset
    add     r0,r0,r3                      @ Phys addr of bcm_pm_sleep_buf_phys variable
    ldr     r0,[r0]                       @ Get the dorm buffer phys addr
    ldr     r0,[r0,#PM_L2CACHE_INIT_FLAG]
    cmp     r0,#0
    beq     l2_invalidate_skipped
    ldr     r0,l210_base_wo_phy
    ldr     r3,l210_config
    str     r3,[r0,#L2X0_AUX_CTRL]        @ Config L2; load MMU descriptor for L2 control register in TLB cache
    mov     r3,#1
    str     r3,[r0,#L2X0_CTRL]            @ Enable L2
l2_invalidate_skipped:
.endif /* ENABLE_L2X0_CODE */

    @ Load the virt address of ret_from_func in r3
    adr     r3,ret_from_dormant           @ Physical address of ret_from_dormant in r3
    ldr     r0,phys_offset
    sub     r3,r3,r0
    ldr     r0,page_offset
    add     r3,r3,r0                      @ Virtual address of ret_from-dormant in r3

    @ Turn on mmu
    @ Sequence from arch/arm/kernel/head.S
    b       bcm215xx_turn_on_mmu

    .align 5
bcm215xx_turn_on_mmu:
    @ The following NOPs are required for the enable MMU sequence
    @ to work correctly. arch/arm/kernel/head.S uses only one NOP.
    mov     r2,r2
    mcr     p15,0,r2,c1,c0,0              @ Write control reg
    mrc     p15,0,r2,c0,c0,0              @ Read id reg
    mov     r2,r2
    mov     r2,r2
    mov     pc,r3                         @ jump to virtual address of ret_from_dormant
    mov     r2,r2
    mov     r2,r2

ret_from_dormant:
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2
    mov     r2,r2

    mov     r2,#0
    mcr     p15,0,r2,c7,c10,4             @ DSB
    mcr     p15,0,r2,c7,c5,6              @ Flush entire branch prediction cache
    mcr     p15,0,r2,c7,c5,4              @ Flush prefetch buffer

    @ Since MMU is enabled, from this point forward r1
    @ should have virtual address of the dormant buffer
    ldr     r0,=bcm_pm_sleep_buf_phys
    ldr     r0,[r0]
    @ Get the current offset of the current location pointed by
    @ R1 in dormant buffer.
    add     r0,r0,#PM_DORMANT_STORE    @ Start of dormant store area phys
    sub     r0,r1,r0                   @ Offset = Current - Start
    ldr     r1,=bcm_pm_sleep_buf
    ldr     r1,[r1]
    add     r1,r1,#PM_DORMANT_STORE    @ Start of dormant store area virt
    add     r1,r1,r0                   @ Current dormant buf address virt

    mrc     p15,0,r0,c1,c0,0              @ Read ctrl reg
    orr     r2,r0,#CP15_CTRL_BPRED_BIT    @ Enable branch prediction
    mcr     p15,0,r2,c1,c0,0              @ Write ctrl reg

.endif /* DM_STATICMEM */

    @ Restore TLB lockdown register
    ldr     r2,[r1,#-4]!                  @ Load
    mcr     p15,0,r2,c10,c0,0             @ TLB Lockdown

    @ Disable interrupts while we restore mode SPSRs and regs
    @ Also change to Undefined mode for next section
    cpsid   if,#MODE_UND

    @ Restore Undefined mode SPSR and registers
    ldr     r0,[r1,#-4]!                  @ Load SPSR
    msr     spsr_cxsf,r0                  @ Restore SPSR
    ldmdb   r1!,{r13-r14}                 @ Restore r13-r14 (UND)

    @ Restore Supervisor mode SPSR and registers
    cps     #MODE_SVC
    ldr     r0,[r1,#-4]!                  @ Load SPSR
    msr     spsr_cxsf,r0                  @ Restore SPSR
    ldmdb   r1!,{r13-r14}                 @ Restore r13-r14 (SVC)

    @ Restore Abort mode SPSR and registers
    cps     #MODE_ABT
    ldr     r0,[r1,#-4]!                  @ Load SPSR
    msr     spsr_cxsf,r0                  @ Restore SPSR
    ldmdb   r1!,{r13-r14}                 @ Restore r13-r14 (Abt)

    @ Restore IRQ mode SPSR and registers
    cps     #MODE_IRQ
    ldr     r0,[r1,#-4]!                  @ Load SPSR
    msr     spsr_cxsf,r0                  @ Restore SPSR
    ldmdb   r1!,{r13-r14}                 @ Restore r13-r14 (IRQ)

    @ Restore FIQ mode SPSR and registers
    cps     #MODE_FIQ
    ldr     r0,[r1,#-4]!                  @ Load SPSR
    msr     spsr_cxsf,r0                  @ Restore SPSR
    ldmdb   r1!,{r8-r14}                  @ Restore r8-r14 (FIQ)

    @ Restore CPSR (puts us into System mode) and restore
    @ general purpose registers (SYS)
    ldr     r0,[r1,#-4]!                  @ Load CPSR
    msr     cpsr_cxsf,r0                  @ Restore CPSR

    @ Indicate normal dormant exit status
    ldr     r0,=bcm_pm_sleep_buf
    ldr     r0,[r0]
    ldr    r2,[r0,#PM_DORMANT_EXIT_STAT]
    orr    r2,r2,#0x20
    str    r2,[r0,#PM_DORMANT_EXIT_STAT]

    @ Restore System mode regs
    ldmdb   r1,{r0-r14}                   @ Restore r0-r14 (SYS)

    cps     #MODE_SVC

    @ Return to the caller.
    mov     pc,r14

@ END OF DORMANT MODE EXIT CODE

@****************************************************************************
@*      Data for dormant code
@****************************************************************************

    .ltorg
/* Space to store ARM registers temporarily */
r0_store:
    .word    0
r1_store:
    .word    0
r2_store:
    .word    0
r3_store:
    .word    0
dorm_debug_mode:
    .word    0
ENTRY(bcm215xx_dormant_entry_sz)
    .word    . - bcm215xx_dormant_entry

@****************************************************************************
@*      Code to debug exceptions during dormant wakeup
@****************************************************************************

/*
 * This code is required because the Linux exception handlers are not
 * available yet after dormant wakeup.
 */

/* Macros to identify the abort type */
#define PREFETCH_ABORT                0
#define DATA_ABORT                    1

/*
 * Offsets of register addresses in the tables below. If this list is
 * updated, corresponding changes are to be made in the following
 * tables defined below:
 *
 * 1. reg_table_phys_addr
 */
#define SYSCFG_IOCR3                  (0 << 2)
#define SYSCFG_AHB_CLK_GATE_MASK      (1 << 2)
#define SYSCFG_UARTA_AHB_CLK_EN       (2 << 2)
#define IRQ_IMR                       (3 << 2)
#define CLKPWR_CLK_PM_SLEEP_REQ_FORCE (4 << 2)
#define CLKPWR_CLK_AP_POWER_MODES     (5 << 2)
#define CLKPWR_CLK_POWER_MODES        (6 << 2)
#define CLKPWR_CLK_UARTA_ENABLE       (7 << 2)
#define UARTA_UCR                     (8 << 2)
#define UARTA_LSR                     (9 << 2)
#define UARTA_THR                     (10 << 2)

/*
 * void bcm215xx_tx_char(u32 char, u32 table_base_addr)
 *
 * Description: Transmit the character passed in r0 over UARTA
 *
 * Inputs:
 *   r0 - Character to be transmitted
 *   r1 - Base address of the register address table
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_tx_char)
    stmfd   sp!,{r0,r2,r3,lr}
    ldr     r3,[r1,#UARTA_LSR]            @ Read LSR
wait_on_thre:
    ldr     r2,[r3]
    and     r2,r2,#(0x01 << 5)            @ Extract THRE bit
    cmp     r2,#0x00
    beq     wait_on_thre
    and     r0,r0,#0xFF
    ldr     r3,[r1,#UARTA_THR]
    str     r0,[r3]
    ldmfd   sp!,{r0,r2,r3,pc}             @ restore regs and return

/*
 * void bcm215xx_enable_console(void)
 *
 * Description: Re-enable UARTA for printing debug messages.
 *  Since UARTs were not turned off during the dormant mode, it still
 *  holds the configurations. Hence no reconfiguration is done here.
 *
 * Inputs:
 *   None.
 *
 * Expects:
 *   table_base_addr to point to correct register table
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_enable_console)
    stmfd   sp!,{r0-r3,lr}

    @ Load the register table base address
    ldr     r1,table_base_addr

    @ Clear APP sleep bit IMR
    mov     r3,#0x1
    ldr     r0,[r1,#IRQ_IMR]
    ldr     r2,[r0]
    and     r2,r2,r3
    str     r2,[r0]

    @ Turn on AHB clock requests for UARTs
    ldr    r2,=0xFE3FFFFF
    ldr    r0,[r1,#SYSCFG_AHB_CLK_GATE_MASK]
    str    r2,[r0]

    @ CLK_PM_SLEEP_REQ_FORCE
    mov    r2,#0x0
    ldr    r0,[r1,#CLKPWR_CLK_PM_SLEEP_REQ_FORCE]
    str    r2,[r0]

    @ CLK_AP_POWER_MODES
    mov    r2,#0x0
    ldr    r0,[r1,#CLKPWR_CLK_AP_POWER_MODES]
    str    r2,[r0]

    @ CLK_POWER_MODES
    mov    r2,#0x0
    ldr    r0,[r1,#CLKPWR_CLK_POWER_MODES]
    str    r2,[r0]

    @ Turn on UART AHB clock gates
    mov    r2,#0x01
    ldr    r0,[r1,#SYSCFG_UARTA_AHB_CLK_EN]
    str    r2,[r0]

    @ Enable UART clocks
    ldr    r0,[r1,#CLKPWR_CLK_UARTA_ENABLE]
    str    r2,[r0]

    @ Disable power down in UCR regs
    mov    r2,#0x30
    ldr    r0,[r1,#UARTA_UCR]
    str    r2,[r0]

    @ Enable UARTC_DIS in SYSCFG_IOCR3
    ldr    r0,[r1,#SYSCFG_IOCR3]
    ldr    r3,=(1 << 22)
    and    r2,r2,r3
    str    r2,[r0]

    ldmfd   sp!,{r0-r3,pc}                @ restore regs and return

/*
 * void bcm215xx_print_string(const u32 *str)
 *
 * Description: Print the input NULL terminated ASCII string onto
 *  UARTA console.
 *
 * Inputs:
 *   r0 - Pointer to the string
 *
 * Expects:
 *   table_base_addr to point to correct register table
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_print_string)
    stmfd   sp!,{r0-r4,lr}

    @ Load the register table base address
    ldr     r1,table_base_addr

    mov     r4,r0                         @ Index variable
get_next_byte:
    ldrb    r0,[r4],#1
    cmp     r0,#0                         @ NULL?
    beq     exit_print_string
    bl      bcm215xx_tx_char
    b       get_next_byte

exit_print_string:
    ldmfd   sp!,{r0-r4,pc}                @ restore regs and return

/*
 * void bcm215xx_print_byte(u32 data)
 *
 * Description: Convert the input byte into ASCII string and print
 *  it on UARTA console.
 *
 * Inputs:
 *   r0 - Byte to be printed in LSB
 *
 * Expects:
 *   table_base_addr to point to correct register table
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_print_byte)
    stmfd   sp!,{r0,r1,r4-r6,lr}

    @ Save input byte
    mov     r6,r0

    @ Load the register table base address
    ldr     r1,table_base_addr

    @ Load address of ascii table
    adr     r4,ascii_table

    @ Print upper nibble
    and     r5,r6,#0xF0                   @ Extract upper nibble
    mov     r5,r5,lsr #4
    ldrb    r0,[r4,r5]                    @ Load the corresponding ascii
                                          @ character.
    bl      bcm215xx_tx_char

    @ Print lower nibble
    and     r5,r6,#0x0F                   @ Extract lower nibble
    ldrb    r0,[r4,r5]                    @ Load the corresponding ascii
                                          @ character.
    bl      bcm215xx_tx_char

    ldmfd   sp!,{r0,r1,r4-r6,pc}          @ restore regs and return

    .ltorg
ascii_table: .asciz  "0123456789abcdef"
    .align

/*
 * void bcm215xx_print_word(u32 word)
 *
 * Description: Convert the input word into ASCII string and print
 *  it on UARTA console.
 *
 * Inputs:
 *   r0 - Word to be printed
 *
 * Expects:
 *   table_base_addr to point to correct register table
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_print_word)
    stmfd   sp!,{r0,r7,lr}

    @ Save input word
    mov     r7,r0

    @ Print most significant byte
    mov     r0,r7,lsr #24
    bl      bcm215xx_print_byte

    @ Print next significant byte
    mov     r0,r7,lsl #8
    mov     r0,r0,lsr #24
    bl      bcm215xx_print_byte

    @ Print next significant byte
    mov     r0,r7,lsl #16
    mov     r0,r0,lsr #24
    bl      bcm215xx_print_byte

    @ Print least significant byte
    mov     r0,r7,lsl #24
    mov     r0,r0,lsr #24
    bl      bcm215xx_print_byte

    ldmfd   sp!,{r0,r7,pc}                @ restore regs and return

/*
 * void bcm215xx_print_newline(void)
 *
 * Description: Send '\r\n' sequence over UARTA
 *
 * Inputs:
 *   None.
 *
 * Expects:
 *   None.
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_print_newline)
    stmfd   sp!,{r0,lr}

    adr     r0,newline_str
    bl      bcm215xx_print_string

    ldmfd   sp!,{r0,pc}                   @ restore regs and return

    .ltorg
/*
 * Set of strings for the exception dump
 */
newline_str: .asciz  "\r\n"
    .align

/*
 * void bcm215xx_print_tab(void)
 *
 * Description: Send '\t' character over UARTA
 *
 * Inputs:
 *   None.
 *
 * Expects:
 *   None.
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_print_tab)
    stmfd   sp!,{r0,lr}
    adr     r0,tab_str
    bl      bcm215xx_print_string
    ldmfd   sp!,{r0,pc}                   @ restore regs and return

    .ltorg
/*
 * Set of strings for the exception dump
 */
tab_str: .asciz  "\t"
    .align

/*
 * void bcm215xx_print_space(void)
 *
 * Description: Send ' ' character over UARTA
 *
 * Inputs:
 *   None.
 *
 * Expects:
 *   None.
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_print_space)
    stmfd   sp!,{r0,lr}
    adr     r0,space_str
    bl      bcm215xx_print_string
    ldmfd   sp!,{r0,pc}                   @ restore regs and return

    .ltorg
/*
 * Set of strings for the exception dump
 */
space_str: .asciz  " "
    .align

/*
 * void bcm215xx_print_exception_mode(u32 word)
 *
 * Description: Print the mode name string corresponding to the input
 *   ARM mode number (from the CPSR mode bits).
 *
 * Inputs:
 *   r0 - Mode number
 *
 * Expects:
 *   table_base_addr to point to correct register table
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_print_exception_mode)
    stmfd   sp!,{r0,r1,lr}

    @ Save the mode number
    mov    r1,r0
    /*
     * Print the exception mode name
     */
    adr     r0,cur_mode_str
    bl      bcm215xx_print_string

    cmp     r1,#MODE_USR
    adreq   r0,usr_mode_str
    cmp     r1,#MODE_FIQ
    adreq   r0,fiq_mode_str
    cmp     r1,#MODE_IRQ
    adreq   r0,irq_mode_str
    cmp     r1,#MODE_SVC
    adreq   r0,svc_mode_str
    cmp     r1,#MODE_ABT
    adreq   r0,abt_mode_str
    cmp     r1,#MODE_UND
    adreq   r0,und_mode_str
    cmp     r1,#MODE_SYS
    adreq   r0,sys_mode_str

    bl      bcm215xx_print_string

    /* If the mode is abort, then further print the type of abort */
    cmp     r1,#MODE_ABT
    bne     1f
    ldr     r0,abort_type
    cmp     r0,#PREFETCH_ABORT
    adreq   r0,pbt_mode_str
    adrne   r0,abt_mode_str
    bl      bcm215xx_print_string
1:
    ldmfd   sp!,{r0,r1,pc}                @ restore regs and return

    .ltorg
/*
 * Set of strings for the exception dump
 */
cur_mode_str: .asciz  "\r\nException mode: "
    .align
usr_mode_str: .asciz  "USR\r\n"
    .align
fiq_mode_str: .asciz  "FIQ\r\n"
    .align
irq_mode_str: .asciz  "IRQ\r\n"
    .align
svc_mode_str: .asciz  "SVC\r\n"
    .align
abt_mode_str: .asciz  "ABT"
    .align
und_mode_str: .asciz  "UND\r\n"
    .align
sys_mode_str: .asciz  "SYS\r\n"
    .align
dbt_mode_str: .asciz  " : DATA\r\n"
    .align
pbt_mode_str: .asciz  " : PREFETCH\r\n"
    .align

/*
 * void bcm215xx_print_exception_buffer(void)
 *
 * Description: Print the exception buffer contents onto UARTA console.
 *
 * Inputs:
 *   None
 *
 * Expects:
 *   table_base_addr to point to correct register table
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_print_exception_buffer)
    stmfd   sp!,{r0-r3,lr}

    adr     r1,exception_buffer
    ldr     r0,[r1],#4
    bl      bcm215xx_print_exception_mode

    /*
     * Print SYS mode regs
     */
    adr     r0,sys_mode_regs
    bl      bcm215xx_print_string

    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab

    mov     r2,#4                         @ Outer loop counter
1:
    mov     r3,#4                         @ Inner loop counter
2:
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    subs    r3,r3,#1
    bne     2b
    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab
    subs    r2,r2,#1
    bne     1b

    /*
     * Print FIQ mode regs
     */
    adr     r0,fiq_mode_regs
    bl      bcm215xx_print_string

    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab

    mov     r2,#2                         @ Outer loop counter
1:
    mov     r3,#4                         @ Inner loop counter
2:
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    subs    r3,r3,#1
    bne     2b
    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab
    subs    r2,r2,#1
    bne     1b

    /*
     * Print IRQ mode regs
     */
    adr     r0,irq_mode_regs
    bl      bcm215xx_print_string

    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab

    @ r13_irq
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    @ r14_irq
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    @ spsr_irq
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_newline

    /*
     * Print ABT mode regs
     */
    adr     r0,abt_mode_regs
    bl      bcm215xx_print_string

    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab

    @ r13_abt
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    @ r14_abt
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    @ spsr_abt
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_newline

    /*
     * Print SVC mode regs
     */
    adr     r0,svc_mode_regs
    bl      bcm215xx_print_string

    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab

    @ r13_svc
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    @ r14_svc
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    @ spsr_svc
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_newline

    /*
     * Print UND mode regs
     */
    adr     r0,und_mode_regs
    bl      bcm215xx_print_string

    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab

    @ r13_und
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    @ r14_und
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    @ spsr_und
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_newline

    ldmfd   sp!,{r0-r3,pc}                @ restore regs and return

    .ltorg
usr_mode_regs: .asciz  "\r\nUSR Mode Registers:"
    .align
fiq_mode_regs: .asciz  "\r\nFIQ Mode Registers:"
    .align
irq_mode_regs: .asciz  "\r\nIRQ Mode Registers:"
    .align
svc_mode_regs: .asciz  "\r\nSVC Mode Registers:"
    .align
abt_mode_regs: .asciz  "\r\nABT Mode Registers:"
    .align
und_mode_regs: .asciz  "\r\nUND Mode Registers:"
    .align
sys_mode_regs: .asciz  "\r\nSYS Mode Registers:"
    .align

/* Macros to control the way in which dormant buffer is
 * printed on the console.
 */
#define DORMANT_BUFFER_SIZE_BYTES       1024
#define NUMBER_OF_WORDS_PER_LINE        8
#define NUMBER_OF_BYTES_PER_LINE        (NUMBER_OF_WORDS_PER_LINE * 4)
#define NUMBER_OF_LINES                 \
	(DORMANT_BUFFER_SIZE_BYTES / NUMBER_OF_BYTES_PER_LINE)

/*
 * void bcm215xx_print_dormant_buffer(void)
 *
 * Description: Print the dormant buffer contents onto UARTA console.
 *
 * Inputs:
 *   None
 *
 * Expects:
 *   r1 - dorm buffer address.
 *
 * Modifies:
 *   None.
 */
ENTRY(bcm215xx_print_dormant_buffer)
    stmfd   sp!,{r0-r3,lr}

    @ Load dormant buffer address
    ldr     r0,=bcm_pm_sleep_buf_phys     @ Virt addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r3,page_offset                @ Convert it to phys
    ldr     r2,phys_offset
    sub     r0,r0,r3
    add     r0,r0,r2                      @ Phys addr of bcm_pm_sleep_buf_phys
                                          @ variable
    ldr     r0,[r0]                       @ Get the dorm buffer phys addr

    adr     r0,dormant_buffer_str
    bl      bcm215xx_print_string

    @ Print dormant buffer address
    mov     r0,r1
    bl      bcm215xx_print_word

    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab

    ldr     r2,=NUMBER_OF_LINES           @ Outer loop counter
1:
    mov     r3,#NUMBER_OF_WORDS_PER_LINE  @ Inner loop counter
2:
    ldr     r0,[r1],#4
    bl      bcm215xx_print_word
    bl      bcm215xx_print_space
    subs    r3,r3,#1
    bne     2b
    bl      bcm215xx_print_newline
    bl      bcm215xx_print_tab
    subs    r2,r2,#1
    bne     1b

    ldmfd   sp!,{r0-r3,pc}                @ restore regs and return

    .ltorg
dormant_buffer_str: .asciz  "\r\nDormant buffer: "
    .align

/****************************************************************************
 *      Dormant mode exception handler
 ****************************************************************************/

ENTRY(bcm215xx_pabt_handler)
    str     r0,excep_r0_store             @ Temporary store
    mov     r0,#PREFETCH_ABORT
    str     r0,abort_type
    ldr     r0,excep_r0_store             @ Restore r0
    b       bcm215xx_dorm_exception_handler

ENTRY(bcm215xx_dabt_handler)
    str     r0,excep_r0_store             @ Temporary store
    mov     r0,#DATA_ABORT
    str     r0,abort_type
    ldr     r0,excep_r0_store             @ Restore r0
    b       bcm215xx_dorm_exception_handler

ENTRY(bcm215xx_dorm_exception_handler)
    /*
     * Identify the current processor mode
     */
    str     r0,excep_r0_store             @ Temporary store
    str     r1,excep_r1_store             @ Temporary store

    adr     r1,exception_buffer

    mrs     r0,cpsr
    and     r0,r0,#CPSR_MODE_BITS_MASK
    str     r0,[r1],#4                    @ Save current mode

    cpsid   aif,#MODE_SYS                 @ Switch to system mode

    @ Save general purpose registers (SYS) and CPSR
    ldr     r0,excep_r0_store             @ Restore r0
    str     r0,[r1],#4                    @ Save r0
    ldr     r0,excep_r1_store             @ Restore r1
    str     r0,[r1],#4                    @ Save r1
    stmia   r1!,{r2-r14}                  @ Save r2-r14 (SYS/USR)
    mrs     r0,cpsr                       @ Get CPSR
    str     r0,[r1],#4                    @ Save CPSR

    @ Interrupts are already disabled at this point. Save
    @ regs of all the modes.

    @ Save FIQ mode registers and SPSR
    cps     #MODE_FIQ
    stmia   r1!,{r8-r14}                  @ Save r8-r14 (FIQ)
    mrs     r0,spsr                       @ Get SPSR
    str     r0,[r1],#4                    @ Save SPSR

    @ Save IRQ mode registers and SPSR
    cps     #MODE_IRQ
    stmia   r1!,{r13-r14}                 @ Save r13-r14 (IRQ)
    mrs     r0,spsr                       @ Get SPSR
    str     r0,[r1],#4                    @ Save SPSR

    @ Save Abort mode registers and SPSR
    cps     #MODE_ABT
    stmia   r1!,{r13-r14}                 @ Save r13-r14 (Abort)
    mrs     r0,spsr                       @ Get SPSR
    str     r0,[r1],#4                    @ Save SPSR

    @ Save Supervisor mode registers and SPSR
    cps     #MODE_SVC
    stmia   r1!,{r13-r14}                 @ Save r13-r14 (SVC)
    mrs     r0,spsr                       @ Get SPSR
    str     r0,[r1],#4                    @ Save SPSR

    @ Save Undefined mode registers and SPSR
    cps     #MODE_UND
    stmia   r1!,{r13-r14}                 @ Save r13-r14 (Undef)
    mrs     r0,spsr                       @ Get SPSR
    str     r0,[r1],#4                    @ Save SPSR

    @ Move to System mode
    cpsid   aif,#MODE_SYS

    @ Save the total number of words stored in exception buffer.
    adr     r0,exception_buffer
    sub     r1,r1,r0
    adr     r0,total_words_saved
    str     r1,[r0]

    /*
     * Setup the stack pointer to the exception stack for function calls
     */
    adr    sp,exception_stack_end

    /*
     * At this point all relevant regs are saved in the exception buffer
     */

    @ Identify whether we are running from virtual or physical space
    adr     r1,reg_table_phys_addr
    str     r1,table_base_addr            @ Save a copy of the table base addr

    @ Enable UARTA to print the exception buffer
    bl      bcm215xx_enable_console

    @ Print the exception buffer
    bl      bcm215xx_print_exception_buffer

    @ Print the dormant buffer
    bl      bcm215xx_print_dormant_buffer

wait_for_rescue:
    b       wait_for_rescue

@****************************************************************************
@*      Data for dormant exception handler
@****************************************************************************

    .ltorg

/*
 * Temporary space to enable saving the ARM regs
 */
exception_buffer:
    .skip    256
total_words_saved:
    .word    0
excep_r0_store:
    .word    0
excep_r1_store:
    .word    0
/* This locations is used to differentiate between abort types as follows:
 *
 * PREFETCH_ABORT - Prefetch abort
 * DATA_ABORT     - Data abort
 */
abort_type:
    .word    0
table_base_addr:
    .word    0

reg_table_phys_addr:
    .word    ADDR_SYSCFG_IOCR3_PHYS
    .word    ADDR_SYSCFG_AHB_CLK_GATE_MASK_PHYS
    .word    ADDR_SYSCFG_UARTA_AHB_CLK_EN_PHYS
    .word    ADDR_IRQ_IMR_PHYS
    .word    ADDR_CLKPWR_CLK_PM_SLEEP_REQ_FORCE_PHYS
    .word    ADDR_CLKPWR_CLK_AP_POWER_MODES_PHYS
    .word    ADDR_CLKPWR_CLK_POWER_MODES_PHYS
    .word    ADDR_CLKPWR_CLK_UARTA_ENABLE_PHYS
    .word    ADDR_UARTA_UCR_PHYS
    .word    ADDR_UARTA_LSR_PHYS
    .word    ADDR_UARTA_THR_PHYS

exception_stack:
    .skip    256
exception_stack_end:
    .word    0

/*Util function to set AHB mode
 * void bcm215xx_set_armahb_mode(u32 mode)
 */
ENTRY(bcm215xx_set_armahb_mode)
		stmfd   sp!,{r0-r12,lr}
		ldr		r3, =ADDR_CLKPWR_CLK_ARMAHB_MODE
		ldr		r2, [r3]
		and		r2, r2, #0x0F
		and		r0, r0, #0x0F
		cmp		r0, r2
		beq		set_armahb_mode_ret

		mrs		r6, cpsr				@  r6 saves cpsr
		orr		r4, r6, #0xc0
		msr		cpsr_c, r4
		mrc     p15,0,r4,c1,c0,0		@  read ctrl reg
		bic     r5,r4,#1<<11			@  disable branch prediction
		mcr     p15,0,r5,c1,c0,0		@  write ctrl reg
		eor		r5, r5, r4				@  r5 saves current branch prediction setting

		ldr		r8, =ADDR_EMI_REQUEST_MASK
		ldr		r4, [r8]				@ dummy read to assure MMU descriptor is in TLB cache
		ldr		r4, [r3]				@ dummy read to assure MMU descriptor is in TLB cache

		adr		r4, set_armahb_mode_incache_begin
		adr		r7, set_armahb_mode_incache_end
		bic		r4, r4, #0x1f
set_armahb_mode_prefetch:
		mcr		p15, 0, r4, c7, c13, 1	@  prefetch instruction cache line
		add		r4, r4, #0x20
		cmp		r4, r7
		blt		set_armahb_mode_prefetch

		mov     r4,#0
		mcr     p15,0,r4,c7,c10,4		@ DSB

set_armahb_mode_incache_begin:
		mov		r4, #300				@ arm11 delay 12us @416mhz or 6us @832mhz
set_armahb_mode_delay0:
		subs	r4, r4, #1
		bne		set_armahb_mode_delay0

		B		set_armahb_mode_get_semaphore2
set_armahb_mode_get_semaphore:
		MOV		r4, #50
set_armahb_mode_get_semaphore1:
		SUBS	r4, r4, #1
		BNE		set_armahb_mode_get_semaphore1
set_armahb_mode_get_semaphore2:
		LDR		r9, [r8]
		ANDS	r7, r9, #0x0F
		BNE		set_armahb_mode_get_semaphore
		AND		r9, r9, #0xF0
		ORR		r10, r9, #4
		STR		r10, [r8]
		MOV		r4, #50
set_armahb_mode_get_semaphore3:
		SUBS	r4, r4, #1
		BNE		set_armahb_mode_get_semaphore3
		LDR		r9, [r8]
		AND		r7, r9, #0x0F
		CMP		r7, #4
		BNE		set_armahb_mode_get_semaphore
				 @semaphore has been obtained
		AND		r9, r9, #0xF0
		ORR		r10, r9, #0x0F

		str		r10, [r8]				@  block all emi ports
		mov		r4, #100				@  arm11 delay 4us @416mhz or 2us @832mhz
set_armahb_mode_delay1:
		subs	r4, r4, #1
		bne		set_armahb_mode_delay1
		str		r0, [r3]				@  change armahb
		mov		r4, #100				@  arm11 delay 4us @416mhz or 2us @832mhz
set_armahb_mode_delay2:
		subs	r4, r4, #1
		bne		set_armahb_mode_delay2
		str		r9, [r8]				@  unblock all emi ports
		mov		r4, #100				@  arm11 delay 4us @416mhz or 2us @832mhz
set_armahb_mode_delay3:
		subs	r4, r4, #1
		bne		set_armahb_mode_delay3

		mov		r4, #0x00
		mov		r4, #0x01
		mov		r4, #0x02
		mov		r4, #0x03
		mov		r4, #0x04
		mov		r4, #0x05
		mov		r4, #0x06
		mov		r4, #0x07
		mov		r4, #0x08
		mov		r4, #0x09
		mov		r4, #0x0A
		mov		r4, #0x0B
		mov		r4, #0x0C
		mov		r4, #0x0D
		mov		r4, #0x0E
		mov		r4, #0x0F
		mov		r4, #0x10
		mov		r4, #0x11
		mov		r4, #0x12
		mov		r4, #0x13
		mov		r4, #0x14
		mov		r4, #0x15
		mov		r4, #0x16
		mov		r4, #0x17
		mov		r4, #0x18
		mov		r4, #0x19
		mov		r4, #0x1A
		mov		r4, #0x1B
		mov		r4, #0x1C
		mov		r4, #0x1D
		mov		r4, #0x1E
		mov		r4, #0x1F

set_armahb_mode_incache_end:
		mrc     p15,0,r4,c1,c0,0		@  read ctrl reg
		bic     r4,r4,#1<<11			@  disable branch prediction
		orr		r4,r4,r5
		mcr     p15,0,r4,c1,c0,0		@  write ctrl reg
		msr		cpsr_c, r6
set_armahb_mode_ret:
	   ldmfd   sp!,{r0-r12,pc}                  @ restore regs and return


/*Util function to set AHB mode
 * void bcm215xx_set_appll_enable(u32 enable)
 */
ENTRY(bcm215xx_set_appll_enable)
		stmfd   sp!,{r0-r12,lr}
		ldr		r3, =ADDR_CLKPWR_CLK_APPSPLL_ENABLE
		ldr		r2, [r3]
		and		r2, r2, #1
		and		r0, r0, #1
		cmp		r0, r2
		beq		set_appll_enable_ret

		mrs		r6, cpsr				@  r6 saves cpsr
		orr		r4, r6, #0xc0
		msr		cpsr_c, r4
		mrc     p15,0,r4,c1,c0,0		@  read ctrl reg
		bic     r5,r4,#1<<11			@  disable branch prediction
		mcr     p15,0,r5,c1,c0,0		@  write ctrl reg
		eor		r5, r5, r4				@  r5 saves current branch prediction setting

		ldr		r2, =ADDR_CLKPWR_CLK_SYSCLK_DEBUG_MON2
		ldr		r8, =ADDR_EMI_REQUEST_MASK
		ldr		r4, [r8]			@ dummy read to assure MMU descriptor is in TLB cache
		ldr		r4, [r2]				@ dummy read to assure MMU descriptor is in TLB cache
		ldr		r4, [r3]				@ dummy read to assure MMU descriptor is in TLB cache

		adr		r4, set_appll_enable_incache_begin
		adr		r7, set_appll_enable_incache_end
		bic		r4, r4, #0x1f
set_appll_enable_prefetch:
		mcr		p15, 0, r4, c7, c13, 1	@  prefetch instruction cache line
		add		r4, r4, #0x20
		cmp		r4, r7
		blt		set_appll_enable_prefetch

		mov     r4,#0
		mcr     p15,0,r4,c7,c10,4		@ DSB

set_appll_enable_incache_begin:
		mov		r4, #300				@ arm11 delay 12us @416mhz or 6us @832mhz
set_appll_enable_delay0:
		subs	r4, r4, #1
		bne		set_appll_enable_delay0

		B		set_appll_enable_get_semaphore2
set_appll_enable_get_semaphore:
		MOV		r4, #50
set_appll_enable_get_semaphore1:
		SUBS	r4, r4, #1
		BNE		set_appll_enable_get_semaphore1
set_appll_enable_get_semaphore2:
		LDR		r9, [r8]
		ANDS	r7, r9, #0x0F
		BNE		set_appll_enable_get_semaphore
		AND		r9, r9, #0xF0
		ORR		r10, r9, #4
		STR		r10, [r8]
		MOV		r4, #50
set_appll_enable_get_semaphore3:
		SUBS	r4, r4, #1
		BNE		set_appll_enable_get_semaphore3
		LDR		r9, [r8]
		AND		r7, r9, #0x0F
		CMP		r7, #4
		BNE		set_appll_enable_get_semaphore
					@ semaphore has been obtained
		AND		r9, r9, #0xF0
		ORR		r10, r9, #0x0F

		str		r10, [r8]				@  block all emi ports
		mov		r4, #100				@  arm11 delay 4us @416mhz or 2us @832mhz
set_appll_enable_delay1:
		subs	r4, r4, #1
		bne		set_appll_enable_delay1
		str		r0, [r3]				@  change apll_enable
set_appll_enable_loop:
		mov		r4, #100				@  arm11 delay 4us @416mhz or 2us @832mhz
set_appll_enable_delay2:
		subs	r4, r4, #1
		bne		set_appll_enable_delay2
		cmp		r0, #0
		beq		set_appll_enable_unblock
		ldr		r1, [r2]
		and		r1, r1, #0xf0
		cmp		r1, #0x50
		bne		set_appll_enable_loop
set_appll_enable_unblock:
		str		r9, [r8]				@  unblock all emi ports
		mov		r4, #100				@  arm11 delay 4us @416mhz or 2us @832mhz
set_appll_enable_delay3:
		subs	r4, r4, #1
		bne		set_appll_enable_delay3

		mov		r4, #0x00
		mov		r4, #0x01
		mov		r4, #0x02
		mov		r4, #0x03
		mov		r4, #0x04
		mov		r4, #0x05
		mov		r4, #0x06
		mov		r4, #0x07
		mov		r4, #0x08
		mov		r4, #0x09
		mov		r4, #0x0A
		mov		r4, #0x0B
		mov		r4, #0x0C
		mov		r4, #0x0D
		mov		r4, #0x0E
		mov		r4, #0x0F
		mov		r4, #0x10
		mov		r4, #0x11
		mov		r4, #0x12
		mov		r4, #0x13
		mov		r4, #0x14
		mov		r4, #0x15
		mov		r4, #0x16
		mov		r4, #0x17
		mov		r4, #0x18
		mov		r4, #0x19
		mov		r4, #0x1A
		mov		r4, #0x1B
		mov		r4, #0x1C
		mov		r4, #0x1D
		mov		r4, #0x1E
		mov		r4, #0x1F

set_appll_enable_incache_end:
		mrc     p15,0,r4,c1,c0,0		@  read ctrl reg
		bic     r4,r4,#1<<11			@  disable branch prediction
		orr		r4,r4,r5
		mcr     p15,0,r4,c1,c0,0		@  write ctrl reg
		msr		cpsr_c, r6
set_appll_enable_ret:
	   ldmfd   sp!,{r0-r12,pc}                  @ restore regs and return


@****************************************************************************
@*      End of code
@****************************************************************************
