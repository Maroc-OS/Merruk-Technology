/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/plat-bcmap/sysfs.c
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
 * SYSFS infrastructure specific Broadcom SoCs
 */
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <plat/syscfg.h>
#include <asm/cacheflush.h>
#include <linux/sched.h>
#include <linux/broadcom/gpt.h>
#include <linux/broadcom/bcm_reset_utils.h>
#include <linux/delay.h>
#include <plat/timer.h>

#include <mach/gpt.h>

struct kobject *bcm_kobj;

static ssize_t
mem_store(struct device *dev, struct device_attribute *attr,
	  const char *buf, size_t n)
{
	uint32_t addr, val, count = 0, loop = 0;
	void __iomem *vaddr;
	char rw;

	if (sscanf(buf, "%c %x %x", &rw, &addr, &val) == 3) {
		pr_info("\n");
		vaddr = ioremap(addr, PAGE_SIZE);
		if (rw == 'W' || rw == 'w') {
			writel(val, vaddr);
			count = 4;
		} else if (rw == 'R' || rw == 'r') {
			count = val;	/* count read in val for simplicity */
			if (count & 0x3)	/* Align to 4 */
				count += (4 - (count & 0x3));
		}
		for (; loop < count; loop += 4) {
			val = readl(vaddr + loop);
			pr_info("[0x%08x] = 0x%08x\n", addr + loop, val);
		}
		iounmap(vaddr);
		return n;
	}
	pr_info("\nUsage: echo <R - Read/W - write> <Physical Address>"
		"<Value(Write)/Count(Read) > /sys/bcm/mem\n"
		"E.g. echo R 0x88CE000 0x40 > /sys/bcm/mem\n"
		"     echo w 0x88CE000 0xDEADBEAF > /sys/bcm/mem\n");
	return -EINVAL;
}

static char *str_reset_reason[] = {
	"power_on_reset",
	"soft_reset",
	"charging",
	"unknown"
};

struct gpt_test {
	uint32_t gpt_num_times;
	uint32_t gpt_count;
	uint32_t gpt_mode;
	uint32_t gpt_index;
};

struct gpt_test g_gptdev[BCM_NUM_GPT];

/* ISR/Callback to handle GPT interrupts */
static int gpt_isr(void *dev_id)
{
	struct gpt_test *tt = (struct gpt_test *) dev_id;
	++tt->gpt_count;

	/* Stop the timer if we have interrupted the number of times */
	if (tt->gpt_count >= tt->gpt_num_times) {
		gpt_stop(tt->gpt_index);
		return 0;
	}

	/* If the timer is configured as Oneshot start the timer again here.
	 */
	if (tt->gpt_mode == GPT_MODE_ONESHOT) {
		gpt_start(tt->gpt_index, -1);
	}

	return 0;
}

static ssize_t
timer_test_show(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	uint32_t test_time, num, index, tick_time, mode, clk;
	struct gpt_cfg timer_tst;
	
	/* test_time: The ticks for which the timer needs to run
	 *       num: Number of times the timer has to run (On each 
	 *            instance it would interrupt after test_time.
	 *     index: Index of the GPT to test
	 *      mode: Non-zero value for oneshot, 0 for periodic
	 */
	if (sscanf(buf, "%d %d %d %d %d", &test_time, &num,
				&index, &mode, &clk) == 5) {
		if (gpt_request(index) == GPT_ERR_INVALID_INDEX) {
			pr_err("Error configuring CE timer exit\n");
			return -EINVAL;
		}

		/* Disable Pedestral mode */
		timer_tst.gctrl = SW_PEDEMODE;
		/* Set the clock to 32Khz */
		timer_tst.gclk = clk;
		/* Configure the timer mode oneshot/periodic */
		if (mode)
			timer_tst.gmode = GPT_MODE_ONESHOT;
		else
			timer_tst.gmode = GPT_MODE_PERIODIC;

		/* Global variable to keep track */
		g_gptdev[index].gpt_count = 0;
		g_gptdev[index].gpt_mode = timer_tst.gmode;
		g_gptdev[index].gpt_num_times = num;
		g_gptdev[index].gpt_index = index;

		/* Configure the timer for the above configration */
		gpt_config(index, &timer_tst, gpt_isr, &g_gptdev[index]);

		/* Get tick count before start for ref */
		tick_time = timer_get_tick_count();
		/* The timer starts here */
		gpt_start(index, test_time);

		/* Wait here until the timer completes the test */
		while (g_gptdev[index].gpt_count < g_gptdev[index].gpt_num_times)
			msleep(10);

		/* Print start and end time */
		pr_info("Tick time taken for gpt index %d start:%u end:%u\n",
				index, tick_time, timer_get_tick_count());

		/* Free timer here */
		gpt_free(index);
		return n;
	}

	return -EINVAL;
}

static ssize_t
reset_reason_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int index;
#ifdef CONFIG_BCM_POWEROFF_CHARGING
	int reset_reason;

	reset_reason = board_sysconfig(SYSCFG_RESETREASON_SOFT_RESET, SYSCFG_INIT);
	switch (reset_reason) {
	case POWER_ON_RESET:
		index = 0;
		break;
	case SOFT_RESET:
		index = 1;
		break;
	case POWEROFF_CHARGING:
		index = 2;
		board_sysconfig(SYSCFG_RESETREASON_SOFT_RESET, SYSCFG_ENABLE);
		break;
	default:
		index = 3;
		break;
	}
	sprintf(buf, "%s\n", str_reset_reason[index]);
#else /* BCM_POWEROFF_CHARGING */
	index = 0; /* Return POWER_ON_RESET always */
#endif /* BCM_POWEROFF_CHARGING */
	return strlen(str_reset_reason[index]) + 1;
}
static DEVICE_ATTR(mem, 0220, NULL, mem_store);
static DEVICE_ATTR(reset_reason, 0444, reset_reason_show, NULL);
static DEVICE_ATTR(timer_test, 0220, NULL, timer_test_show);

static struct attribute *bcm_attrs[] = {
	&dev_attr_mem.attr,
	&dev_attr_reset_reason.attr,
	&dev_attr_timer_test.attr,
	NULL,
};

static struct attribute_group bcm_attr_group = {
	.attrs = bcm_attrs,
};

static int __init bcm_sysfs_init(void)
{
	bcm_kobj = kobject_create_and_add("bcm", NULL);
	if (!bcm_kobj)
		return -ENOMEM;

	return sysfs_create_group(bcm_kobj, &bcm_attr_group);
}

static void __exit bcm_sysfs_exit(void)
{
	sysfs_remove_group(bcm_kobj, &bcm_attr_group);
}

module_init(bcm_sysfs_init);
module_exit(bcm_sysfs_exit);
