/******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/bcm215xx_cp15_debug.S
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

#include <linux/linkage.h>
#include <asm/assembler.h>

    /* Buffers used to store the cp15 regs  before and after suspend states */
    .global cp15regs_before_slp
    .global cp15regs_after_slp
    .global cp15dbg_reg_cnt

/*
 * void cp15dbg_save_regs(u32 *buffer);
 *
 * Take a snapshot of the CP15 registers into the input buffer.
 *
 */
ENTRY(cp15dbg_save_regs)
    str     sp,cp15dbg_sp_store
    adr     sp,cp15dbg_temp_stack_end
    stmfd   sp!,{r1-r12,lr}            @ Save context

    @ backup the buffer address
    str     r0,cp15dbg_save_buffer_addr
    mov     r1,r0

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @             Save CP15 registers into the input buffer                 @
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    @ Save TLB lockdown register, then force page table walks to
    @ write into the associative region of the TLB, to avoid
    @ changing the lockdown region
    mrc     p15,0,r2,c10,c0,0          @ TLB Lockdown
    mrc     p15,0,r3,c10,c2,0          @ Primary Region Remap
    mrc     p15,0,r4,c10,c2,1          @ Normal Memory Remap
    stmia   r1!,{r2-r4}                @ Save

    mrc     p15,0,r2,c1,c0,0           @ Control register
    mrc     p15,0,r3,c1,c0,1           @ Auxiliary Control register
    mrc     p15,0,r4,c1,c0,2           @ Co-processor Access Control register
    stmia   r1!,{r2-r4}                @ Save

    mrc     p15,0,r2,c2,c0,0           @ TTBR0
    mrc     p15,0,r3,c2,c0,1           @ TTBR1
    mrc     p15,0,r4,c2,c0,2           @ TTBCR
    mrc     p15,0,r5,c3,c0,0           @ DAC
    stmia   r1!,{r2-r5}                @ Save

    mrc     p15,0,r2,c5,c0,0           @ DFSR
    mrc     p15,0,r3,c5,c0,1           @ IFSR
    mrc     p15,0,r4,c6,c0,0           @ FAR
    mrc     p15,0,r5,c6,c0,1           @ IFAR
    stmia   r1!,{r2-r5}                @ Save

    mrc     p15,0,r2,c9,c0,0           @ D Cache Lockdown
    mrc     p15,0,r3,c9,c0,1           @ I Cache Lockdown
    mrc     p15,0,r4,c9,c1,0           @ D-TCM region
    mrc     p15,0,r5,c9,c1,1           @ I-TCM region
    stmia   r1!,{r2-r5}                @ Save

    @ ARM1136 rev1 has three extra thread/process ID registers
    mrc     p15,0,r2,c13,c0,0          @ FCSE PID
    mrc     p15,0,r3,c13,c0,1          @ Context ID
    mrc     p15,0,r4,c13,c0,2          @ User R/W Thread/PID
    mrc     p15,0,r5,c13,c0,3          @ User RO Thread/PID
    mrc     p15,0,r6,c13,c0,4          @ Priv Only Thread/PID
    stmia   r1!,{r2-r6}                @ Save

    mrc     p15,0,r2,c15,c2,0          @ Data Mem Remap
    mrc     p15,0,r3,c15,c2,1          @ Instr Mem Remap
    mrc     p15,0,r4,c15,c2,2          @ DMA Mem Remap
    mrc     p15,0,r5,c15,c2,4          @ PP Mem Remap
    mrc     p15,0,r6,c15,c12,0         @ Performance monitor control register
    mrc     p15,0,r7,c15,c12,1         @ Cycle Counter register
    mrc     p15,0,r8,c15,c12,2         @ Count register 0
    mrc     p15,0,r9,c15,c12,3         @ Count register 1
    mrc     p15,0,r10,c11,c0,0         @ DMA identification and status register
    stmia   r1!,{r2-r10}               @ Save

    @ Cache debug operations register
    mrc     p15,3,r2,c15,c0,0          @ Data debug cache register
    mrc     p15,3,r3,c15,c0,1          @ Instruction debug cache register
    mrc     p15,7,r4,c15,c0,0          @ Cache Debug Control
    stmia   r1!,{r2-r4}                @ Save

    @ MMU debug operations register
    mrc      p15,5,r2,c15,c4,0         @ Data MicroTLB Index Register
    mrc      p15,5,r3,c15,c4,1         @ Instruction MicroTLB Index Register
    mrc      p15,7,r4,c15,c1,0         @ TLB Debug Control
    stmia   r1!,{r2-r4}                @ Save

    @ Save lockdown region of TLB
    ldr     r0,=0x80000000
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
    stmia   r1!,{r2-r10}               @ Save

    add     r0,r0,#1
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 3
    mrc     p15,5,r10,c15,c5,2         @ Read Main TLB VA
    mrc     p15,5,r11,c15,c6,2         @ Read Main TLB PA
    mrc     p15,5,r12,c15,c7,2         @ Read Main TLB Attr
    stmia   r1!,{r10-r12}              @ Save

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
    stmia   r1!,{r2-r10}               @ Save

    add     r0,r0,#1
    mcr     p15,5,r0,c15,c4,2          @ Read lockdown entry 7
    mrc     p15,5,r10,c15,c5,2         @ Read Main TLB VA
    mrc     p15,5,r11,c15,c6,2         @ Read Main TLB PA
    mrc     p15,5,r12,c15,c7,2         @ Read Main TLB Attr
    stmia   r1!,{r10-r12}              @ Save

    @ Save the cache master valid registers
    @ D cache master valid bits
    mrc     p15,3,r2,c15,c12,0         @ Dcache master valid 0
    mrc     p15,3,r3,c15,c12,1         @ Dcache master valid 1
    mrc     p15,3,r4,c15,c12,2         @ Dcache master valid 2
    mrc     p15,3,r5,c15,c12,3         @ Dcache master valid 3
    stmia   r1!,{r2-r5}                @ Save

    @ Data smartcache master valid bits
    mrc     p15,3,r2,c15,c14,0         @ Data smartcache master valid 0
    mrc     p15,3,r3,c15,c14,1         @ Data smartcache master valid 1
    mrc     p15,3,r4,c15,c14,2         @ Data smartcache master valid 2
    mrc     p15,3,r5,c15,c14,3         @ Data smartcache master valid 3
    stmia   r1!,{r2-r5}                @ Save

    @ I cache master valid bits
    mrc     p15,3,r2,c15,c8,0          @ Icache master valid 0
    mrc     p15,3,r3,c15,c8,1          @ Icache master valid 1
    mrc     p15,3,r4,c15,c8,2          @ Icache master valid 2
    mrc     p15,3,r5,c15,c8,3          @ Icache master valid 3
    stmia   r1!,{r2-r5}                @ Save

    @ Instruction smartcache master valid bits
    mrc     p15,3,r2,c15,c10,0         @ Instr smartcache master valid 0
    mrc     p15,3,r3,c15,c10,1         @ Instr smartcache master valid 1
    mrc     p15,3,r4,c15,c10,2         @ Instr smartcache master valid 2
    mrc     p15,3,r5,c15,c10,3         @ Instr smartcache master valid 3
    stmia   r1!,{r2-r5}                @ Save

    @ TLB master valid bits
    mrc     p15,5,r2,c15,c14,0         @ TLB master valid 0
    mrc     p15,5,r3,c15,c14,1         @ TLB master valid 1
    stmia   r1!,{r2-r3}                @ Save

    @ calculate the number of registers stored in the buffer and
    @ return this in r0
    ldr     r0,cp15dbg_save_buffer_addr
    sub     r0,r1,r0
    mov     r0,r0,lsr #2               @ r0 /= 4, to convert bytes to words
    @ save the number of registers
    str     r0,cp15dbg_reg_cnt

    ldmfd   sp!,{r1-r12,lr}            @ Restore context
    ldr     sp,cp15dbg_sp_store
    mov     pc,lr

/****************************************************************************
 *                               DATA STORE                                 *
 ****************************************************************************/

    .ltorg
cp15dbg_sp_store:
    .word    0
cp15dbg_save_buffer_addr:
    .word    0
cp15dbg_temp_stack:
    .skip    256
cp15dbg_temp_stack_end:
    .word    0
cp15regs_before_slp:
    .skip    512
cp15regs_after_slp:
    .skip    512
cp15dbg_reg_cnt:
    .word    0

/****************************************************************************
 *                               END OF FILE                                *
 ****************************************************************************/
