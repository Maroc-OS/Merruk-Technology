/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/cpu-bcm21553.c
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
 * BCM21553 SoC specific driver definitions
 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/broadcom/bcm_fuse_memmap.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/broadcom/bcm_reset_utils.h>

#include <asm/mach/map.h>
#include <asm/cacheflush.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/hardware.h>
#include <mach/setup.h>
#include <plat/syscfg.h>

#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
void __iomem *sync_flag_base;
#define SCRATCHRAM_V3D_SYNCVAR_OFFSET         0x00007000
#endif

#define CAM_PRIO_HIGH                         0x00011111

extern int bcm21553_l2_evt_buf_init(void);
struct bcm_gpio_port gpio_port = {
	.base = (void __iomem *)(IO_ADDRESS(BCM21553_GPIO_BASE)),
	.irq = IRQ_GPIO,
};

void __init bcm21553_init_irq(void)
{
	bcm_intc_init(io_p2v(BCM21553_INTC_BASE + 0x0),
		      IRQ_INTC_START + 0, BCM21553_VALID_SRC0, 0);
	bcm_intc_init(io_p2v(BCM21553_INTC_BASE + 0x100),
		      IRQ_INTC_START + 32, BCM21553_VALID_SRC1, 0);
	bcm_intc_init(io_p2v(BCM21553_INTC_BASE + 0x180),
		      IRQ_INTC_START + 64, BCM21553_VALID_SRC2, 0);

	bcm_gpio_init(&gpio_port);
}

int apps_vpsharedmem_init(void)
{
	void __iomem *cp_dsp_shmem;

	cp_dsp_shmem = ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);

	if (!cp_dsp_shmem) {
		pr_err("%s: ioremap cp_dsp_shmem failed\n", __func__);
		return -1;
	}
	memset (cp_dsp_shmem, 0, AP_SH_SIZE);
	iounmap(cp_dsp_shmem);	
	return 0;
}

static void __init bcm21553_start_cp(void)
{
	void __iomem *cp_boot_base;
	void __iomem *apcp_shmem;
#ifdef CONFIG_DPRAM
        void __iomem *ssapcp_shmem;
#endif

	cp_boot_base = ioremap(CP_BOOT_BASE_PHYS, CP_BOOT_BASE_SIZE);
	if (!cp_boot_base) {
		pr_err("%s: ioremap error\n", __func__);
		return;
	}
	apcp_shmem = ioremap_nocache(IPC_BASE, IPC_SIZE);
	if (!apcp_shmem) {
		pr_err("%s: ioremap shmem failed\n", __func__);
		iounmap(cp_boot_base);
		return;
	}
	/* clear IPC shared memory */
	memset(apcp_shmem, 0, IPC_SIZE);
#ifdef CONFIG_DPRAM
        ssapcp_shmem = ioremap_nocache(SSIPC_BASE, SSIPC_SIZE);
        if (!ssapcp_shmem) {
                pr_err("%s: ioremap shmem failed\n", __func__);
                iounmap(cp_boot_base);
                iounmap(apcp_shmem);
                return;
        }
        /* clear Samsung IPC shared memory */
        memset(ssapcp_shmem, 0, SSIPC_SIZE);
#endif

	if (apps_vpsharedmem_init() < 0) {
		iounmap(cp_boot_base);
		iounmap(apcp_shmem);
#ifdef CONFIG_DPRAM
                iounmap(ssapcp_shmem);
#endif
		return;
	}

	/* Start the CP, Code taken from Nucleus BSP */
	writel(CP_START_ADDR, ((unsigned long)cp_boot_base) + 0x20);
	board_sysconfig(SYSCFG_CP_START, SYSCFG_INIT);
	writel(0, io_p2v(BCM21553_INTC_BASE + 0x38));

	iounmap(cp_boot_base);
	iounmap(apcp_shmem);
#ifdef CONFIG_DPRAM
        iounmap(ssapcp_shmem);
#endif
	pr_info("%s: BCM_FUSE CP Started....\n", __func__);

	return;
}

/*
 * This function is called from the machine init
 * This function gives camera a higher priority compared to SD/USB
 */
void __init bcm21553_cam_lmatrix_prio()
{
	u32 val = ioread32(io_p2v(MATRIX_LARBITER_BASE + 0x108));
	val |= CAM_PRIO_HIGH;
	iowrite32(val,io_p2v((MATRIX_LARBITER_BASE + 0x108)));
}

/*
 * This function is called from the board init
 */
void __init bcm21553_platform_init(void)
{

#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
	sync_flag_base = ioremap_nocache(BCM21553_SCRATCHRAM_BASE +
					SCRATCHRAM_V3D_SYNCVAR_OFFSET, SZ_1K);
	if (sync_flag_base == NULL) {
		pr_err("sync_flag_base ioremap failed\n");
	} else {
		iowrite32(0, sync_flag_base + 0);
		iowrite32(0, sync_flag_base + 4);
		iowrite32(0, sync_flag_base + 8);
	}
#ifndef CONFIG_BRCM_V3D
	iounmap(sync_flag_base);
#endif
#endif /* end of CONFIG_BCM21553_V3D_SYNC_ENABLE specific changes */

	if (get_ap_boot_mode() != AP_ONLY_BOOT)
	bcm21553_start_cp();

#ifdef CONFIG_BCM21553_L2_EVCT
	bcm21553_l2_evt_buf_init();
#endif
#ifdef CONFIG_CACHE_L2X0
	/*
	 * 31:25 RESERVED
	 * 24    EXCLUSIVE_ABORT_DISABLE
	 * 23    WRITE_ALLOCATE_OVERRIDE
	 * 22    SHARED_OVERRIDE_ENABLE
	 * 21    PARITY_ENABLE
	 * 20    EVENT_BUS_ENABLE
	 * 19:17 WAY_SIZE - Set 16KB
	 * 16:13 ASSOCIATIVITY - Set 8-way
	 * 12    WRAP_ACCESS_DISABLE
	 * 11:09 DIRTY_RAM_LATENCY - Set 1 cycle
	 * 08:06 TAG_RAM_LATENCY - Set 1 cycle
	 * 05:03 DATA_RAM_WRITE_LATENCY - Set 3 cycle
	 * 02:00 DATA_RAM_READ_LATENCY - Set 3 cycle
	 */
	l2x0_init(io_p2v(BCM21553_L210_BASE), BCM21553_L210_CONFIG,
		  BCM21553_L210_CONFIG);
#endif
	return;
}
