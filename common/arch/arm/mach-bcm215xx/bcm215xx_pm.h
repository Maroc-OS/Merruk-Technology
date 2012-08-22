/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/bcm215xx_pm.h
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

#ifndef __BCM215XX_PM_H__
#define __BCM215XX_PM_H__

#include <asm/sizes.h>

/* Enable/disable dormant mode time profiling code, define this as follows:
 * 1 - to enable profiling code in dormant sequence
 * 0 - to disable profiling code in dormant sequence
 */
#define BCM_PM_DORMANT_PROFILING      0

#define GPIO_IOTR1_OFF                4

#if defined(CONFIG_BOARD_THUNDERBIRD_EDN31)
#define BCM_PM_DORMANT_DEBUG_GPIO     22
#elif defined(CONFIG_BOARD_THUNDERBIRD_EDN5x)
#define BCM_PM_DORMANT_DEBUG_GPIO     22
#endif

/* L210 controller defines */
#if defined(CONFIG_ARCH_BCM215XX)
#define L2CACHE_READREG_OFFSET        0x1000
#else
#define L2CACHE_READREG_OFFSET        0x0
#endif

#define L210_CONTROLREG_CACHE_ENABLE  ((u32)1)

/*
 * Indices into the dormant context save buffer. Any update to this list
 * must reflect in the following declarations:
 *
 * 1. struct bcm_pm_sleep (in this header file)
 * 2. static const char *slpbuf_field_names[] (in bcm215xx_pm.c)
 */
#define PM_DEEPSLEEP_ALLOWED          (0  << 2)
#define PM_DORMANT_ALLOWED            (1  << 2)
#define PM_PEDESTAL_ALLOWED           (2  << 2)
#define PM_DEEPSLEEP_COUNT            (3  << 2)
#define PM_DORMANT_COUNT              (4  << 2)
#define PM_PEDESTAL_COUNT             (5  << 2)
#define PM_DORM_PEDESTAL_COUNT        (6  << 2)
#define PM_DORM_DEEPSLEEP_COUNT       (7  << 2)
#define PM_PEDESTAL_TIME              (8  << 2)
#define PM_DORMANT_VERIFY             (9  << 2)
#define PM_CHECKSUM_ENTER             (10 << 2)
#define PM_CHECKSUM_EXIT              (11 << 2)
#define PM_CHECKSUM_COMPARE           (12 << 2)
#define PM_PRE_DORMANT_CPSR           (13 << 2)
#define PM_DORMANT_SAVE_STAT          (14 << 2)
#define PM_DORMANT_EXIT_STAT          (15 << 2)
#define PM_DORMANT_SAVE_CNT           (16 << 2)
#define PM_DORMANT_EXIT_CNT           (17 << 2)
#define PM_DORMANT_INT_CNT            (18 << 2)
#define PM_DORMANT_FAIL_CNT           (19 << 2)
#define PM_DORMANT_FAIL_STAT          (20 << 2)
#define PM_DORMANT_SEQUENCE_DBG       (21 << 2)
#define PM_DORMANT_STORE_END          (22 << 2)
#define PM_L2CACHE_INIT_FLAG          (23 << 2)
#define PM_L2_EVICT_BUF               (24 << 2)
#define PM_DORMANT_SCRATCH1           (25 << 2)
#define PM_DORMANT_SCRATCH2           (26 << 2)
#define PM_DORMANT_SCRATCH3           (27 << 2)
/* This must be the last element of the above list */
#define PM_NUM_SLEEP_ELEMENTS         (28)

#define DOR_BUF_SZ     (SZ_1K)

/* Scratch ram related defines for the checksum function. The last two
 * words of the CRC SRAM area is reserved for storing the checksums
 * calculated before and after dormant entry/exit.
 */
#define CRC_DDR_SZ     (SZ_1M * 220) /* Size of DDR to be checksummed       */
#define CRC_SRAM_SZ    (SZ_1K)       /* Size of SRAM used for CRC           */
#define CRC_SRAM_START (0)           /* Offset of CRC code in SRAM          */
#define CRC_DORM_ENTRY (SZ_1K - 8)   /* Offset to store dorm entry checksum */
#define CRC_DORM_EXIT  (SZ_1K - 4)   /* Offset to store dorm exit checksum  */
/*
 * Set this to 1 to enable DDR CRC check code in dormant sequence
 */
#define BCM_PM_DORMANT_VERIFY_CRC      0

#ifndef __ASSEMBLY__

/*
 * Dormant and sleep mode context buffer. This buffer also contains
 * fields to trace the sequence (for debugging).
 */
struct bcm_pm_sleep {
	/*
	 * Sleep state control
	 */
	u32 deepsleep_allowed;
	u32 dormant_allowed;
	u32 pedestal_allowed;
	/*
	 * Counters to debug sleep mode states
	 */
	u32 deepsleep_count;
	u32 dormant_count;
	u32 pedestal_count;
	u32 dorm_pedestal_count;
	u32 dorm_deepsleep_count;
	u32 pedestal_time;
	/*
	 * Checksum verification status
	 */
	u32 verify;
	u32 chksum_enter;
	u32 chksum_exit;
	u32 chksum_compare;
	/*
	 * Save CPSR of current state. Dormant state context save happens
	 * in SYSTEM mode. Hence the previous mode CPSR is saved here
	 * before the dormant mode sequence begins.
	 */
	u32 pre_dormant_cpsr;
	/*
	 * These variables are used to trace the dormant mode flow. Examine
	 * these locations once control comes out of dormant to get an idea
	 * of how the dormant flow executed.
	 */
	u32 save_stat;
	u32 exit_stat;
	u32 save_cnt;
	u32 exit_cnt;
	u32 int_cnt;
	u32 fail_cnt;
	u32 fail_stat;
	/*
	 * Set to 'true' to enable dormant mode save-restore sequence.
	 * If true, then WFI instruction is bypassed and control is
	 * transferred to the dormant restore sequence.
	 */
	u32 dormant_sequence_dbg;
	/*
	 * Stores the offset of the end of dormant context save buffer after
	 * the save sequence is completed. This offset is added in a position
	 * independent way during the resume sequence to restore the context.
	 */
	u32 dormant_store_end_offset;
	/* Scratch locations to save registers temporarily during dormant
	 * sequence.
	 */
	u32 l2cache_init_flag;
	u32 l2_evict_buf;
	u32 dormant_scratch1;
	u32 dormant_scratch2;
	u32 dormant_scratch3;
	u32 dormant_store[DOR_BUF_SZ - (PM_NUM_SLEEP_ELEMENTS * sizeof(u32))];
};

/***************************************************************************
 * Assembly routines to move apps processor to sleep states.
 ****************************************************************************/
extern u32 *bcm215xx_dorm_wake_handler(void);
extern void bcm215xx_sleep(struct bcm_pm_sleep *pm_sleep);
extern void bcm215xx_dorm_exception_handler(void);
extern void bcm215xx_pabt_handler(void);
extern void bcm215xx_dabt_handler(void);
extern void bcm215xx_enter_idle(int cpu_state);
extern void flush_caches(void);

#if (BCM_PM_DORMANT_VERIFY_CRC == 1)
/* Exports for CRC calculation */
extern const u32 bcm_crc32_sz;
extern u32 bcm_crc32(u32 *buffer, u32 size, u32 *table);
#endif

extern u32 l2_evt_virt_buf;

#endif /* !__ASSEMBLY__ */

#endif /* __BCM215XX_PM_H__ */
