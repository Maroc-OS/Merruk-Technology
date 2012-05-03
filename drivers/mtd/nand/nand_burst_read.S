#include <linux/linkage.h>
#include <asm/assembler.h>

/*
 *		FastNANDAXIReadData
 *
 *
 * FastNANDAXIReadData
 * r0 = UInt32 ptr to dst buffer
 * r1 = UInt32 numBytes
 * r2 = UInt32 data phase address
 * Assumes numBytes > 0 and is a multiple of 64.
 *
 * Reminder: r0-3 are arguments or scratch registers, so do not save.
 *          r12 is general scratch register, but only ARM
 *          r14 = lr and return value is do not care.
 */

ENTRY(FastNANDAXIReadData)
		stmfd   sp!,{r4-r8,lr}         @ Save registers on stack

        add     r12, r0, r1
		mov	 	r1, r2

loop_by_64bytes:
        ldm     r1, {r2-r8, lr}
        stm     r0!,{r2-r8, lr}
        ldm     r1, {r2-r8, lr}
        stm     r0!,{r2-r8, lr}
        cmp     r0, r12
        blo     loop_by_64bytes        @ while pDst < pTop

		ldmfd  	sp!,{r4-r8,pc}         @ restore regs and return

ENDPROC(FastNANDAXIReadData)
