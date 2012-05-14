/*******************************************************************************
* Copyright 2010,2011 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/plat-bcmap/bcm_cpufreq.c
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/sysfs.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/slab.h>

#include <mach/bcm21553_cpufreq_gov.h>

#define dprintk(msg...) \
	cpufreq_debug_printk(CPUFREQ_DEBUG_GOVERNOR, "bcm21553", msg)

/* BCM21553 governor private data */
struct bcm_cpufreq_gov_info {
	struct cpufreq_policy *cur_policy;
	struct cpufreq_client_desc *sysfs_client;
	struct bcm21553_cpufreq_gov_plat *plat;
};
static struct bcm_cpufreq_gov_info bcm_cpufreq_gov_info;

static int dvfs_disable;
module_param_named(dvfs_disable, dvfs_disable, int, S_IRUGO);

/*********************************************************************
 *                             DEBUG CODE                            *
 *********************************************************************/

/* Enable debug code */
#define CONFIG_BCM215XX_PM_DEBUG

#ifdef CONFIG_BCM215XX_PM_DEBUG
/* Debug interface is accessed via:
 * /sys/module/cpufreq_bcm21553/parameters/debug
 */

/* Enable/disable debug logs */
enum {
	/* Disable all logging */
	DEBUG_DISABLE = 0U,
	DEBUG_FLOW    = (1U << 0),
};

#define DEFAULT_LOG_LVL    0

struct debug {
	int log_lvl;
};

#define __param_check_debug(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }

#define param_check_debug(name, p) \
	__param_check_debug(name, p, debug)

static int param_set_debug(const char *val, struct kernel_param *kp);
static int param_get_debug(char *buffer, struct kernel_param *kp);

static struct debug debug = {
	.log_lvl = DEFAULT_LOG_LVL,
};
module_param_named(debug, debug, debug, S_IRUGO | S_IWUSR | S_IWGRP);

/* helpers to test the log_lvl bitmap */
#define IS_FLOW_DBG_ENABLED	(debug.log_lvl & DEBUG_FLOW)

/* List of commands supported */
enum {
	CMD_SET_LOG_LVL = 'l',
	CMD_SHOW_STATS = 's',
};

static void cmd_show_usage(void)
{
	const char usage[] = "Usage:\n"
	  "echo 'cmd string' > /sys/module/cpufreq_bcm21553/parameters/debug\n"
	  "'cmd string' must be constructed as follows:\n"
	  "Update log level: l 0x01\n"
	  "Show stats: s\n"
	  "echo 'cmd' > /sys/devices/system/cpu/cpufreq/bcm21553/dvfs\n"
	  " 'cmd' field must be:\n"
	  " a. enable dvfs: y or Y\n"
	  " b. disable dvfs: n or N\n";

	pr_info("%s", usage);
}

/*
 * Command handlers
 */
static void cmd_set_log_lvl(const char *p)
{
	sscanf(p, "%x", &debug.log_lvl);
}

static void cpufreq_bcm_list_states(void);
static void cmd_show_stats(const char *p)
{
	pr_info("log_lvl: 0x%x\n", debug.log_lvl);
	pr_info("dvfs_disable: %d\n", dvfs_disable);
	cpufreq_bcm_list_states();
}

static int param_set_debug(const char *val, struct kernel_param *kp)
{
	const char *p;

	if (!val)
		return -EINVAL;

	/* Command is only one character followed by a space. Arguments,
	 * if any, starts from offset 2 in val.
	 */
	p = &val[2];

	switch (val[0]) {
	case CMD_SET_LOG_LVL:
		cmd_set_log_lvl(p);
		break;
	case CMD_SHOW_STATS:
		cmd_show_stats(p);
		break;
	default:
		cmd_show_usage();
		break;
	}
	return 0;
}

static int param_get_debug(char *buffer, struct kernel_param *kp)
{
	return 0;
}
#else /* CONFIG_BCM215XX_PM_DEBUG */

#define IS_FLOW_DBG_ENABLED	(0)

#endif /* CONFIG_BCM215XX_PM_DEBUG */

/*********************************************************************
 *                 GENERIC DVFS CLIENT INTERFACE                     *
 *********************************************************************/

struct cpufreq_client_desc {
	const char *name;
	int cnt; /* number of active dvfs disable requests */
	struct module *owner;
	struct list_head list;
};

static struct list_head client;
/* Total number of dvfs disable requests pending */
static struct mutex dvfs_lock;

static inline void cpufreq_bcm_turbo_off(void)
{
	struct bcm21553_cpufreq_gov_plat *plat = bcm_cpufreq_gov_info.plat;

	if (bcm_cpufreq_gov_info.cur_policy)
		cpufreq_driver_target(bcm_cpufreq_gov_info.cur_policy,
				      plat->freq_normal, CPUFREQ_RELATION_H);
}

static inline void cpufreq_bcm_turbo_on(void)
{
	struct bcm21553_cpufreq_gov_plat *plat = bcm_cpufreq_gov_info.plat;

	if (bcm_cpufreq_gov_info.cur_policy)
		cpufreq_driver_target(bcm_cpufreq_gov_info.cur_policy,
				      plat->freq_turbo, CPUFREQ_RELATION_H);
}

static void cpufreq_bcm_list_states(void)
{
	struct cpufreq_client_desc *p;

	list_for_each_entry(p, &client, list)
		pr_info("%s: %d\n", p->name, p->cnt);
}

struct cpufreq_client_desc *_cpufreq_bcm_client_get(const char *name,
						    struct module *owner)
{
	struct cpufreq_client_desc *desc;

	desc = kzalloc(sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return desc;

	desc->name = name;
	desc->owner = owner;
	desc->cnt = 0;

#ifdef CPUFREQ_ENABLE_MODULE_REF_CNTS
	if (!try_module_get(owner)) {
		kfree(desc);
		return NULL;
	}
#endif

	mutex_lock(&dvfs_lock);
	list_add(&desc->list, &client);
	mutex_unlock(&dvfs_lock);

	return desc;
}
EXPORT_SYMBOL(_cpufreq_bcm_client_get);

void cpufreq_bcm_client_put(struct cpufreq_client_desc *desc)
{
	if (desc) {
		BUG_ON(desc->cnt > 0);	
#ifdef CPUFREQ_ENABLE_MODULE_REF_CNTS
		module_put(desc->owner);
#endif
		mutex_lock(&dvfs_lock);
		list_del(&desc->list);
		mutex_unlock(&dvfs_lock);
		kfree(desc);
	}
}
EXPORT_SYMBOL(cpufreq_bcm_client_put);

void cpufreq_bcm_dvfs_enable(struct cpufreq_client_desc *desc)
{
	int cnt;

	if (!desc)
		return;
	if (IS_FLOW_DBG_ENABLED)
		pr_info("%s: DVFS enable request from %s\n",
				__func__, desc->name);
	mutex_lock(&dvfs_lock);
	cnt = desc->cnt;
	if (likely(cnt > 0)) {
		cnt--;
		desc->cnt = cnt;
		dvfs_disable--;
		if (dvfs_disable == 0) {
			if (IS_FLOW_DBG_ENABLED)
			pr_info("%s: switching to normal mode: %s\n",
				 __func__, desc->name);
			cpufreq_bcm_turbo_off();
		}
	} else {
		WARN(1, "Unbalanced dvfs enable in client %s\n", desc->name);
	}
	mutex_unlock(&dvfs_lock);
}
EXPORT_SYMBOL(cpufreq_bcm_dvfs_enable);

void cpufreq_bcm_dvfs_disable(struct cpufreq_client_desc *desc)
{
	if (!desc)
		return;
	if (IS_FLOW_DBG_ENABLED)
		pr_info("%s: DVFS disable request from %s\n",
				__func__, desc->name);
	mutex_lock(&dvfs_lock);
	desc->cnt++;
	dvfs_disable++;

	if (dvfs_disable == 1) {
		if (IS_FLOW_DBG_ENABLED)
		pr_info("%s: switching to turbo mode: %s\n",
			__func__, desc->name);
		cpufreq_bcm_turbo_on();
	}
	mutex_unlock(&dvfs_lock);
}
EXPORT_SYMBOL(cpufreq_bcm_dvfs_disable);

/*********************************************************************
 *                    SYSFS CLIENT INTERFACE                         *
 *********************************************************************/

/*
 * sysfs entry point for controlling dvfs from userspace is in:
 * /sys/devices/system/cpu/cpufreq/bcm21553
 */

static void sysfs_client_init(void)
{
	bcm_cpufreq_gov_info.sysfs_client = _cpufreq_bcm_client_get("sysfs",
								THIS_MODULE);
}

static ssize_t show_dvfs(struct kobject *kobj,
			 struct kobj_attribute *attr, char *buf)
{
	cpufreq_bcm_list_states();
	return 0;
}

static ssize_t store_dvfs(struct kobject *kobj,
			  struct kobj_attribute *attr, const char *buf,
			  size_t n)
{
	if (buf[0] == 'y' || buf[0] == 'Y')
		cpufreq_bcm_dvfs_enable(bcm_cpufreq_gov_info.sysfs_client);
	else if (buf[0] == 'n' || buf[0] == 'N')
		cpufreq_bcm_dvfs_disable(bcm_cpufreq_gov_info.sysfs_client);

	return n;
}

/*
 * Helper macros
 */
#define file_rw(_name)               \
static struct kobj_attribute _name = \
__ATTR(_name, 0644, show_##_name, store_##_name)

#define file_ro(_name)               \
static struct kobj_attribute _name = \
__ATTR(_name, 0444, show_##_name, NULL)

#define file_wo(_name)               \
static struct kobj_attribute _name = \
__ATTR(_name, 0222, NULL, store_##_name)

/*
 * Set of files exported to the userspace
 */
file_rw(dvfs);

static struct attribute *bcm_cpufreq_attributes[] = {
	&dvfs.attr,
	NULL
};

static struct attribute_group bcm_cpufreq_attr_group = {
	.attrs = bcm_cpufreq_attributes,
	.name = "bcm21553",
};

/*********************************************************************
 *                          CORE INTERFACE                           *
 *********************************************************************/

static int cpufreq_governor_bcm(struct cpufreq_policy *policy,
	unsigned int event)
{
	struct bcm21553_cpufreq_gov_plat *plat = bcm_cpufreq_gov_info.plat;
	int ret = 0;

	switch (event) {
	case CPUFREQ_GOV_START:
		dprintk("%s: CPUFREQ_GOV_START\n", __func__);
		ret = sysfs_create_group(cpufreq_global_kobject,
			&bcm_cpufreq_attr_group);
		if (ret) {
			pr_err("%s: sysfs_create_group failed\n", __func__);
			break;
		}

		bcm_cpufreq_gov_info.cur_policy = policy;

		/* This governor may be started after bootup at runtime. In
		 * such a scenario, evaluate the status of current client
		 * requests and set the core voltage appropriately.
		 */
		if (dvfs_disable == 0)
			__cpufreq_driver_target(policy, plat->freq_normal,
						CPUFREQ_RELATION_H);
		else
			__cpufreq_driver_target(policy, plat->freq_turbo,
						CPUFREQ_RELATION_H);
		break;

	case CPUFREQ_GOV_LIMITS:
		dprintk("%s: CPUFREQ_GOV_LIMITS\n", __func__);
		break;

	case CPUFREQ_GOV_STOP:
		dprintk("%s: CPUFREQ_GOV_STOP\n", __func__);
		sysfs_remove_group(cpufreq_global_kobject,
			&bcm_cpufreq_attr_group);
		bcm_cpufreq_gov_info.cur_policy = NULL;
		break;

	default:
		break;
	}

	return ret;
}

/*********************************************************************
 *               REGISTER / UNREGISTER CPUFREQ GOVERNOR              *
 *********************************************************************/

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_BCM21553
static
#endif
struct cpufreq_governor cpufreq_gov_bcm = {
	.name		= "bcm21553",
	.governor	= cpufreq_governor_bcm,
	.owner		= THIS_MODULE,
};

static int cpufreq_gov_probe(struct platform_device *pdev)
{
	struct bcm21553_cpufreq_gov_plat *plat = pdev->dev.platform_data;
	pr_info("%s\n", __func__);

	bcm_cpufreq_gov_info.plat = plat;

	dvfs_disable = 0;
	mutex_init(&dvfs_lock);
	INIT_LIST_HEAD(&client);
	sysfs_client_init();

	platform_set_drvdata(pdev, &bcm_cpufreq_gov_info);
	pr_info("%s: succees\n", __func__);

	return cpufreq_register_governor(&cpufreq_gov_bcm);
}

static int __devexit cpufreq_gov_remove(struct platform_device *pdev)
{
	mutex_destroy(&dvfs_lock);
	cpufreq_unregister_governor(&cpufreq_gov_bcm);
	return 0;
}

static struct platform_driver cpufreq_gov = {
	.probe = cpufreq_gov_probe,
	.remove = __devexit_p(cpufreq_gov_remove),
	.driver = {
		.name = "bcm21553-cpufreq-gov",
	},
};

static int __init cpufreq_gov_init(void)
{
	return platform_driver_register(&cpufreq_gov);
}


static void __exit cpufreq_gov_exit(void)
{
	platform_driver_unregister(&cpufreq_gov);
}

MODULE_DESCRIPTION("CPUfreq policy governor 'bcm21553'");
MODULE_LICENSE("GPL");

rootfs_initcall(cpufreq_gov_init);
module_exit(cpufreq_gov_exit);
