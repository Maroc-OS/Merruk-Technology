/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/cpuidle_bcm21553.c
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

#include <linux/cpuidle.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include <linux/time.h>
#include <linux/io.h>
#include <mach/bcm21553_cpuidle_drv.h>

#include "bcm215xx_pm.h"

#define BCM21553_SAFE_STATE	BCM21553_STATE_C0

/* CPU states description */
struct bcm21553_cpuidle_state {
	char *name;
	char *desc;
	unsigned int flags;
	unsigned int exit_latency; /* in us */
	unsigned int target_residency; /* in us */
	int state;
};

static struct bcm21553_cpuidle_state bcm_cpu_states[] = {
	{
		.name = "C0",
		.desc = "wfi",
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.exit_latency = 0,
		.target_residency = 0,
		.state = BCM21553_STATE_C0,
	}, {
		.name = "C1",
		.desc = "pedestal",
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.exit_latency = 5,
		.target_residency = 5,
		.state = BCM21553_STATE_C1,
	}, {
		.name = "C2",
		.desc = "pedestal-dormant",
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.exit_latency = 200,
		.target_residency = 50,
		.state = BCM21553_STATE_C2,
	},
};

DEFINE_PER_CPU(struct cpuidle_device, bcm21553_idle_dev);

/*********************************************************************
 *                             DEBUG CODE                            *
 *********************************************************************/

/* Enable debug code */
#define CONFIG_BCM215XX_PM_DEBUG

#ifdef CONFIG_BCM215XX_PM_DEBUG
/* Enable/disable debug logs */
enum {
	/* Disable all logging */
	DEBUG_DISABLE = 0U,
	DEBUG_FLOW    = (1U << 0),
};

#define __param_check_cpu_dbg(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }

#define param_check_cpu_dbg(name, p) \
	__param_check_cpu_dbg(name, p, cpu_dbg)

static int param_set_cpu_dbg(const char *val, struct kernel_param *kp);
static int param_get_cpu_dbg(char *buffer, struct kernel_param *kp);

struct cpu_dbg {
	unsigned long state_cntr[BCM21553_MAX_CPU_STATES];
	int log_lvl;
	int idle_time;
};

static struct cpu_dbg cpu_dbg = {
	.log_lvl = DEBUG_FLOW,
};
module_param_named(debug, cpu_dbg, cpu_dbg, S_IRUGO | S_IWUSR | S_IWGRP);

static inline void instrument_cpuidle(struct bcm21553_cpuidle_state *cpu,
				      int idle_time)
{
	cpu_dbg.idle_time = idle_time;

	if (cpu->state < BCM21553_MAX_CPU_STATES)
		cpu_dbg.state_cntr[cpu->state]++;
}

/* helpers to test the log_lvl bitmap */
#define DEBUG_FLOW_ENABLED	(cpu_dbg.log_lvl & DEBUG_FLOW)

/* List of commands supported */
enum {
	CMD_CHANGE_STATE = 'c',
	CMD_SHOW_CNTRS = 's',
	CMD_SET_LOG_LVL = 'l',
	CMD_ENABLE_CPUIDLE = 'e',
	CMD_DISABLE_CPUIDLE = 'd',
};

static void cmd_show_usage(void)
{
	const char usage[] = "Usage:\n"
	  "echo 'cmd string' > /sys/module/cpuidle_bcm21553/parameters/debug\n"
	  "'cmd string' must be constructed as follows:\n"
	  "Update log level: l 0x01\n"
	  "Show counters: s\n"
	  "Change state timings as follows:c cpu state field value\n"
	  "Change exit_latency of cpu 1 state 0: c 1 0 exit 20\n"
	  "Change target_residency of cpu 0 state 1: c 0 1 target 20\n"
	  "Enable cpuidle: e\n"
	  "Disable cpuidle: d\n";

	pr_info("%s", usage);
}

/*
 * Command handlers
 */
static void cmd_change_state(const char *p)
{
	struct cpuidle_device *dev;
#define FIELD_SIZE	10
#define MAX_CPUS	1
	char field[FIELD_SIZE + 1];
	unsigned int value;
	int state, cpu;
	int i;

	/* Get cpu number */
	sscanf(p, "%d", &cpu);
	if (cpu < 0 || cpu >= MAX_CPUS) {
		pr_info("%s: invalid cpu number\n", __func__);
		return;
	}
	/* Skip past the cpu number */
	while (*p != ' ' && *p != '\t')
		p++;
	/* Skip white spaces */
	while (*p == ' ' || *p == '\t')
		p++;

	/* Get state */
	sscanf(p, "%d", &state);
	if (state < 0 || state > BCM21553_MAX_CPU_STATES) {
		pr_info("%s: invalid state number\n", __func__);
		return;
	}
	/* Skip past the state number */
	while (*p != ' ' && *p != '\t')
		p++;
	/* Skip white spaces */
	while (*p == ' ' || *p == '\t')
		p++;

	/* Get field name */
	for (i = 0; i < FIELD_SIZE; i++) {
		if (*p != ' ' && *p != '\t')
			field[i] = *p++;
		else
			break;
	}
	field[i] = '\0';
	/* Skip any extra chars (should not be there ideally) */
	while (*p != ' ' && *p != '\t')
		p++;
	/* Skip white spaces */
	while (*p == ' ' || *p == '\t')
		p++;

	/* Get field value */
	sscanf(p, "%d", &value);

	pr_info("Setting field '%s' of state%d to %u\n", field, state, value);

	dev = &per_cpu(bcm21553_idle_dev, cpu);
	if (strncmp("exit", field, 4) == 0)
		dev->states[state].exit_latency = value;
	else if (strncmp("target", field, 4) == 0)
		dev->states[state].target_residency = value;
}

static void cmd_show_cntrs(const char *p)
{
	struct cpuidle_device *dev;
	int i;

	pr_info("log_lvl: 0x%x, idle_time: %d\n", cpu_dbg.log_lvl,
		cpu_dbg.idle_time);

	/* Get handle of current cpu */
	dev = &per_cpu(bcm21553_idle_dev, smp_processor_id());
	for (i = 0; i < BCM21553_MAX_CPU_STATES; i++)
		pr_info("state%d:%lu, exit_latency:%u, target_residency: %u\n",
			i, cpu_dbg.state_cntr[i],
			dev->states[i].exit_latency,
			dev->states[i].target_residency);
}

static void cmd_set_log_lvl(const char *p)
{
	sscanf(p, "%x", &cpu_dbg.log_lvl);
}

static void cmd_enable_cpuidle(const char *p)
{
	cpuidle_resume_and_unlock();
}

static void cmd_disable_cpuidle(const char *p)
{
	cpuidle_pause_and_lock();
}

static int param_set_cpu_dbg(const char *val, struct kernel_param *kp)
{
	const char *p;

	if (!val)
		return -EINVAL;

	/* Command is only one character followed by a space. Arguments,
	 * if any, starts from offset 2 in val.
	 */
	p = &val[2];

	switch (val[0]) {
	case CMD_CHANGE_STATE:
		cmd_change_state(p);
		break;
	case CMD_SHOW_CNTRS:
		cmd_show_cntrs(p);
		break;
	case CMD_SET_LOG_LVL:
		cmd_set_log_lvl(p);
		break;
	case CMD_ENABLE_CPUIDLE:
		cmd_enable_cpuidle(p);
		break;
	case CMD_DISABLE_CPUIDLE:
		cmd_disable_cpuidle(p);
		break;
	default:
		cmd_show_usage();
		break;
	}
	return 0;
}

static int param_get_cpu_dbg(char *buffer, struct kernel_param *kp)
{
	return 0;
}
#else /* CONFIG_BCM215XX_PM_DEBUG */

#define DEBUG_FLOW_ENABLED	(0)
#define instrument_cpuidle(cpu, idle_time)

#endif /* CONFIG_BCM215XX_PM_DEBUG */

/*********************************************************************
 *                           CPU IDLE HANDLER                        *
 *********************************************************************/

static int bcm21553_cpuidle_handler(struct cpuidle_device *dev,
				    struct cpuidle_state *state)
{
	struct timeval before, after;
	struct bcm21553_cpuidle_state *cpu = cpuidle_get_statedata(state);
	int idle_time;

	local_irq_disable();
	local_fiq_disable();

	do_gettimeofday(&before);
	bcm215xx_enter_idle(cpu->state);
	do_gettimeofday(&after);

	local_fiq_enable();
	local_irq_enable();

	idle_time = (after.tv_sec - before.tv_sec) * USEC_PER_SEC +
		(after.tv_usec - before.tv_usec);

	if (DEBUG_FLOW_ENABLED)
		instrument_cpuidle(cpu, idle_time);

	return idle_time;
}

/*********************************************************************
 *                              INIT CODE                            *
 *********************************************************************/

static struct cpuidle_driver bcm21553_idle_driver = {
	.name = "bcm21553_idle",
	.owner = THIS_MODULE,
};

static int __init cpuidle_drv_init(void)
{
	struct cpuidle_device *dev;
	struct cpuidle_state *state;
	int i, ret;

	pr_info("%s\n", __func__);

	/* Register CPUIDLE driver */
	ret = cpuidle_register_driver(&bcm21553_idle_driver);
	if (ret < 0) {
		pr_err("%s: cpuidle_register_driver failed\n", __func__);
		goto err_cpuidle_drv_reg;
	}

	dev = &per_cpu(bcm21553_idle_dev, smp_processor_id());
	for (i = 0; i < BCM21553_MAX_CPU_STATES; i++) {
		state = &dev->states[i];

		strncpy(state->name, bcm_cpu_states[i].name,
			CPUIDLE_NAME_LEN - 1);
		strncpy(state->desc, bcm_cpu_states[i].desc,
			CPUIDLE_DESC_LEN - 1);
		cpuidle_set_statedata(state, &bcm_cpu_states[i]);
		state->flags = bcm_cpu_states[i].flags;
		state->exit_latency = bcm_cpu_states[i].exit_latency;
		state->target_residency = bcm_cpu_states[i].target_residency;
		state->enter = bcm21553_cpuidle_handler;
	}
	dev->state_count = BCM21553_MAX_CPU_STATES;
	dev->safe_state = BCM21553_SAFE_STATE;

	/* Register CPUIDLE device */
	ret = cpuidle_register_device(dev);
	if (ret < 0) {
		pr_err("%s: cpuidle_register_driver failed\n", __func__);
		goto err_cpuidle_dev_reg;
	}
	return 0;

err_cpuidle_dev_reg:
	cpuidle_unregister_driver(&bcm21553_idle_driver);
err_cpuidle_drv_reg:
	return ret;
}
module_init(cpuidle_drv_init);

static void __exit cpuidle_drv_exit(void)
{
	struct cpuidle_device *dev;

	dev = &per_cpu(bcm21553_idle_dev, smp_processor_id());
	cpuidle_unregister_device(dev);
	cpuidle_unregister_driver(&bcm21553_idle_driver);
}
module_exit(cpuidle_drv_exit);

MODULE_ALIAS("bcm_cpuidle_drv");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CPU Idle Driver for Broadcom Chipsets");
