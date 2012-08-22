/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/mm.c
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
 * memory map definitions
 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/bootmem.h>
#include <mach/setup.h>
#include <asm/setup.h>
#include <linux/dma-mapping.h>

#include <asm/mach/map.h>
#include <mach/hardware.h>
#include <mach/sec_debug.h>


#define __IO_DEV_DESC(x, sz)	{		\
	.virtual	= IO_ADDRESS(x),	\
	.pfn		= __phys_to_pfn(x),	\
	.length		= sz,			\
	.type		= MT_DEVICE,		\
}

#define __IO_DEV_DESC_TEMP(x, sz)	{		\
	.virtual	= 0xE0000000,	\
	.pfn		= __phys_to_pfn(x),	\
	.length		= sz,			\
	.type		= MT_DEVICE,		\
}

/* minimum static i/o mapping required to boot BCM21553 platforms */
static struct map_desc bcm21553_io_desc[] __initdata = {
	/*
	 * TODO It turns out that the hardware is capable of mapping 1Mb blocks
	 * by using the first level mapping. So making the I/O space be one
	 * large space is better from a performance standpoint than making
	 * several spaces, each less than 1Mb.
	 */
	__IO_DEV_DESC(BCM21553_ONENAND_BASE, SZ_1M),
	__IO_DEV_DESC(BCM21553_MHARB_BASE, SZ_1M - SZ_64K),
	__IO_DEV_DESC(BCM21553_SDIO1_BASE, SZ_1M - SZ_64K),
	__IO_DEV_DESC(BCM21553_USB_HSOTG_BASE, SZ_1M),
	__IO_DEV_DESC(BCM21553_USB_FSHOST_BASE, SZ_1M),
	__IO_DEV_DESC(BCM21553_SDIO3_BASE, SZ_1M),
	__IO_DEV_DESC(BCM21553_SLPTMR1_BASE, SZ_1M),
	__IO_DEV_DESC(BCM21553_MSPRO_BASE, SZ_1M),
	__IO_DEV_DESC(BCM21553_VEC_BASE, SZ_1M),
	__IO_DEV_DESC(BCM21553_HTM0_BASE, SZ_1M),
	__IO_DEV_DESC(BCM21553_CRYPTO_BASE, SZ_512K),
	__IO_DEV_DESC(BCM21553_AHB_ARM_DSP_BASE, SZ_4M + SZ_512K),
	__IO_DEV_DESC_TEMP(BCM21553_SCRATCHRAM_BASE, SZ_64K)
};

/* Return the address of last bank - size for the non-highmem region */
unsigned long get_mmpool_base(unsigned long size)
{
	int i;

	for (i = (meminfo.nr_banks - 1); i >= 0; ++i) {
		if (!(meminfo.bank[i].highmem) &&
			(meminfo.bank[i].size >= size)) {
			return (meminfo.bank[i].start +
				meminfo.bank[i].size - size);
		}
	}

	return 0;
}

#ifdef CONFIG_BRCM_V3D
#include <linux/broadcom/v3d.h>
void *v3d_mempool_base;
unsigned long v3d_mempool_size = V3D_MEMPOOL_SIZE;
#endif

#if defined (CONFIG_BMEM)
#include <linux/broadcom/bmem_wrapper.h>
void *bmem_mempool_base;
uint32_t bmem_phys_base = CONFIG_MM_MEMPOOL_BASE_ADDR;
#else
#ifdef CONFIG_GE_WRAP
#include <linux/broadcom/bcm_gememalloc_wrapper.h>
void *ge_mempool_base;
uint32_t ge_mem_phys_base = CONFIG_MM_MEMPOOL_BASE_ADDR;
#endif
#endif

#ifdef CONFIG_HANTRO_WRAP
#include <linux/broadcom/bcm_memalloc_wrapper.h>
void *memalloc_mempool_base;
#endif
void *cam_mempool_base;
/* MM here stands for multi-media */
void bcm21553_mm_mem_init(void)
{
	int ret, size;
	uint32_t v3d_mem_phys_base = CONFIG_MM_MEMPOOL_BASE_ADDR;

#if (CONFIG_MM_MEMPOOL_BASE_ADDR <= 0)
#if defined (CONFIG_BMEM)
	bmem_phys_base = get_mmpool_base(BMEM_SIZE);
#else
#ifdef CONFIG_GE_WRAP
	ge_mem_phys_base = get_mmpool_base(gememalloc_SIZE);
#endif
#endif
#ifdef CONFIG_BRCM_V3D
	size = v3d_mempool_size;
#if defined (CONFIG_BMEM)
	size += BMEM_SIZE;
#else
#ifdef CONFIG_GE_WRAP
	size += gememalloc_SIZE;
#endif
#endif
	v3d_mem_phys_base = get_mmpool_base(size);
#endif
#else
#if defined(CONFIG_BRCM_V3D)
#if defined (CONFIG_BMEM)
	bmem_phys_base += v3d_mempool_size;
#else
#if defined(CONFIG_GE_WRAP)
	ge_mem_phys_base += v3d_mempool_size;
#endif
#endif
#endif
#endif

#ifdef CONFIG_BRCM_V3D
	if (v3d_mempool_size) {
	ret = reserve_bootmem(v3d_mem_phys_base, v3d_mempool_size, BOOTMEM_EXCLUSIVE);
	if (ret < 0) {
		printk(KERN_ERR "Failed to allocate memory for v3d\n");
		return;
	}

	v3d_mempool_base = phys_to_virt(v3d_mem_phys_base);
		pr_info("v3d phys[0x%08x] virt[0x%08x] size[0x%08x] \n",
			v3d_mem_phys_base, (uint32_t)v3d_mempool_base, (int)v3d_mempool_size);
	} else {
		v3d_mempool_base = NULL;
		v3d_mem_phys_base = 0;
	}
#endif

#if defined (CONFIG_BMEM)
	ret = reserve_bootmem(bmem_phys_base, BMEM_SIZE, BOOTMEM_EXCLUSIVE);
	if (ret < 0) {
		printk(KERN_ERR "Failed to allocate memory for ge\n");
		return;
	}

	bmem_mempool_base = phys_to_virt(bmem_phys_base);
	pr_info("bmem phys[0x%08x] virt[0x%08x] size[0x%08x] \n",
		bmem_phys_base, (uint32_t)bmem_mempool_base, BMEM_SIZE);
#else
#ifdef CONFIG_GE_WRAP
	ret = reserve_bootmem(ge_mem_phys_base, gememalloc_SIZE, BOOTMEM_EXCLUSIVE);
	if (ret < 0) {
		printk(KERN_ERR "Failed to allocate memory for ge\n");
		return;
	}

	ge_mempool_base = phys_to_virt(ge_mem_phys_base);
	pr_info("ge phys[0x%08x] virt[0x%08x] size[0x%08x] \n",
		ge_mem_phys_base, (uint32_t)ge_mempool_base, gememalloc_SIZE);
#endif
#endif

#if defined (CONFIG_BMEM)
#ifdef CONFIG_HANTRO_WRAP
	memalloc_mempool_base = alloc_bootmem_low_pages(2 * PAGE_SIZE);
	pr_info("memalloc(hantro) phys[0x%08x] virt[0x%08x] size[0x%08x] \n",
		(uint32_t)virt_to_phys(memalloc_mempool_base), (uint32_t)memalloc_mempool_base,
		(uint32_t)(2 * PAGE_SIZE));
#endif
	cam_mempool_base = alloc_bootmem_low_pages(2 * PAGE_SIZE);
	pr_info("pmem(camera) phys[0x%08x] virt[0x%08x] size[0x%08x] \n",
		(uint32_t)virt_to_phys(cam_mempool_base), (uint32_t)cam_mempool_base,
		(uint32_t)(2 * PAGE_SIZE));
#else
#ifdef CONFIG_HANTRO_WRAP
	memalloc_mempool_base = alloc_bootmem_low_pages(MEMALLOC_SIZE + SZ_2M);
#endif
	cam_mempool_base = alloc_bootmem_low_pages(1024 * 1024 * 8);
#endif
}

#ifdef CONFIG_BCM21553_L2_EVCT
u32 l2_evt_virt_buf;
unsigned long l2_evt_phys_buf;
int bcm21553_l2_evt_buf_init(void)
{
	void *temp;

	/* Allocate one word of memory with the following property:
	 * TEX,C,B = 100,0,0
	 * This memory is used for sw work-around for L2 quiesce.
	 */
	temp = dma_alloc_coherent(NULL, SZ_1K,
				 (dma_addr_t *)&l2_evt_phys_buf, GFP_ATOMIC);
	if (!temp) {
		pr_err("%s: dma_alloc_coherent for L2 evict buf failed\n",
			__func__);
		return -ENOMEM;
	}

	l2_evt_virt_buf = (u32)ioremap_wc(l2_evt_phys_buf, 4);
	if (!l2_evt_virt_buf) {
		pr_err("%s: ioremap_wc for L2 evict buf failed\n",
			__func__);
		return -ENOMEM;
	}

	return 0;
}
#endif

void __init bcm21553_map_io(void)
{
	iotable_init(bcm21553_io_desc, ARRAY_SIZE(bcm21553_io_desc));
	bcm21553_mm_mem_init();

	/* as soon as INFORM3 is visible, sec_debug is ready to run */
	sec_debug_init();
	
}
