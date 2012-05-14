/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/input/ponkey-max8986.c
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

#define DEBUG

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/mfd/max8986/max8986.h>
#include <linux/rtc.h>

#define MAX8986_PONKEY_VENDOR_ID       0x0001
#define MAX8986_PONKEY_PRODUCT_ID      0x00010
#define MAX8986_PONKEY_VERSION         0x0100

/*
 *  Private data
 */
static int gv_powerkey_pressed;
struct max8986_ponkey {
	struct max8986 *max8986;
	struct input_dev *dev;
};

 /*********************************************************************
  *                             DEBUG CODE
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

#define DEFAULT_LOG_LVL    (DEBUG_DISABLE)

struct debug {
	struct max8986_ponkey *max8986_ponkey;
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

/* List of commands supported */
enum {
	CMD_SET_LOG_LVL = 'l',
	CMD_SIM_PONKEY_PRESS = '1',
};

static void cmd_show_usage(void)
{
	const char usage[] = "Usage:\n"
	  "echo 'cmd string' > /sys/module/ponkey_max8986/parameters/debug\n"
	  "'cmd string' must be constructed as follows:\n"
	  "Update log level: l 0x01\n"
	  "Simulate ponkey press: 1\n";

	pr_info("%s", usage);
}

/*
 * Command handlers
 */
static void cmd_set_log_lvl(const char *p)
{
	sscanf(p, "%x", &debug.log_lvl);
}

/* This handler sends ponkey press and release events to the input
 * subsystem. This interface is used by the automated monkey-test
 * script to turn ON the LCD once the system has woken up from
 * suspend (by events like IPC interrupt).
 */
static void cmd_sim_ponkey_press(const char *p)
{
	/* Ponkey press event */
	input_report_key(debug.max8986_ponkey->dev, KEY_POWER, 1);
	input_sync(debug.max8986_ponkey->dev);

	msleep(1);

	/* Ponkey release event */
	input_report_key(debug.max8986_ponkey->dev, KEY_POWER, 0);
	input_sync(debug.max8986_ponkey->dev);
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
	case CMD_SIM_PONKEY_PRESS:
		cmd_sim_ponkey_press(p);
		break;
	default:
		cmd_show_usage();
		break;
	}
	return 0;
}

static int param_get_debug(char *buffer, struct kernel_param *kp)
{
	cmd_show_usage();
	return 0;
}

#endif /* CONFIG_BCM215XX_PM_DEBUG */

int max8986_ponkey_check(void)
{
    return gv_powerkey_pressed;
}
EXPORT_SYMBOL(max8986_ponkey_check);

static void max8986_ponkey_isr(int irq, void *data)
{
	/* Android counts the hold time for KEY_POWER so we can just
	 * post the key press and release here.
	 */
	struct max8986_ponkey *max8986_ponkey = data;
	struct device *d = &max8986_ponkey->dev->dev;
       struct timespec ts;
	struct rtc_time tm;
    
	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);

	switch (irq) {
	case MAX8986_IRQID_INT1_PWRONBF:
		dev_info(d, "[%02d:%02d:%02d.%03lu] %s: PWRONBF irq\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec, __func__);
		input_report_key(max8986_ponkey->dev, KEY_POWER, 1);
		input_sync(max8986_ponkey->dev);
                gv_powerkey_pressed = 1;        
		break;

	case MAX8986_IRQID_INT1_PWRONBH:
		dev_info(d, "[%02d:%02d:%02d.%03lu] %s: PWRONBH irq\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec, __func__);
		break;

	case MAX8986_IRQID_INT1_PWRONBR:
		dev_info(d, "[%02d:%02d:%02d.%03lu] %s: PWRONBR irq\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec, __func__);
		input_report_key(max8986_ponkey->dev, KEY_POWER, 0);
		input_sync(max8986_ponkey->dev);
                gv_powerkey_pressed = 0;            
		break;
	}
}

static int __devinit max8986_ponkey_probe(struct platform_device *pdev)
{
	struct max8986 *max8986 = dev_get_drvdata(pdev->dev.parent);
	struct max8986_ponkey *max8986_ponkey = NULL;
	int ret = 0, err;
	u8 rval;

	dev_info(&pdev->dev, "%s\n", __func__);

	max8986_ponkey = kzalloc(sizeof(struct max8986_ponkey), GFP_KERNEL);
	if (max8986_ponkey == NULL) {
		dev_dbg(&pdev->dev, "%s: failed to allocate memory\n",
			__func__);
		ret = -ENOMEM;
		goto err_mem_alloc_fail;
	}

	max8986_ponkey->max8986 = max8986;

	max8986_ponkey->dev = input_allocate_device();
	if (!max8986_ponkey->dev) {
		dev_err(&pdev->dev, "%s: input_allocate_device failed\n",
			__func__);
		ret = -ENOMEM;
		goto err_input_alloc_fail;
	}

	set_bit(EV_KEY, max8986_ponkey->dev->evbit);
	set_bit(KEY_POWER, max8986_ponkey->dev->keybit);

	max8986_ponkey->dev->name = "max8986_ponkey";
	max8986_ponkey->dev->phys = "max8986_ponkey/input0";
	max8986_ponkey->dev->id.bustype = BUS_HOST;
	max8986_ponkey->dev->id.vendor = MAX8986_PONKEY_VENDOR_ID;
	max8986_ponkey->dev->id.product = MAX8986_PONKEY_PRODUCT_ID;
	max8986_ponkey->dev->id.version = MAX8986_PONKEY_VERSION;

	/* Set required debounce times, shutdown delay here if required,
	 * other init code can go here too.
	 */

	/* Lets just set KEYLOCK to prevent PMU from shutting the system
	 * down.
	 */
	err = max8986->read_dev(max8986, MAX8986_PM_REG_PONKEYBDB, &rval);
	if (!err) {
		err = max8986->write_dev(max8986, MAX8986_PM_REG_PONKEYBDB,
					 (rval | MAX8986_PWRONBDB_KEYLOCK));
	}

	if (err)
		dev_err(&pdev->dev, "%s: KEYLOCK bit-set failed\n", __func__);

	ret = input_register_device(max8986_ponkey->dev);
	if (ret) {
		dev_err(&pdev->dev, "%s: input_register_device failed\n",
			__func__);
		goto err_input_reg_fail;
	}

	ret = max8986_request_irq(max8986, MAX8986_IRQID_INT1_PWRONBF,
				  true, max8986_ponkey_isr, max8986_ponkey);
	ret |= max8986_request_irq(max8986, MAX8986_IRQID_INT1_PWRONBH,
				   true, max8986_ponkey_isr, max8986_ponkey);
	ret |= max8986_request_irq(max8986, MAX8986_IRQID_INT1_PWRONBR,
				   true, max8986_ponkey_isr, max8986_ponkey);
	if (ret) {
		dev_err(&pdev->dev, "%s: max8986_request_irq failed\n",
			__func__);
		ret = -EBUSY;
		goto err_request_irq_fail;
	}

	platform_set_drvdata(pdev, max8986_ponkey);
	dev_info(&pdev->dev, "%s: success\n", __func__);

#ifdef CONFIG_BCM215XX_PM_DEBUG
	debug.max8986_ponkey = max8986_ponkey;
#endif
	return 0;

err_request_irq_fail:
	max8986_free_irq(max8986, MAX8986_IRQID_INT1_PWRONBF);
	max8986_free_irq(max8986, MAX8986_IRQID_INT1_PWRONBH);
	max8986_free_irq(max8986, MAX8986_IRQID_INT1_PWRONBR);

err_input_reg_fail:
	input_free_device(max8986_ponkey->dev);

err_input_alloc_fail:
	kfree(max8986_ponkey);

err_mem_alloc_fail:
	return ret;
}

static int __devexit max8986_ponkey_remove(struct platform_device *pdev)
{
	struct max8986_ponkey *max8986_ponkey = platform_get_drvdata(pdev);

	if (max8986_ponkey) {
		max8986_free_irq(max8986_ponkey->max8986,
				 MAX8986_IRQID_INT1_PWRONBF);
		max8986_free_irq(max8986_ponkey->max8986,
				 MAX8986_IRQID_INT1_PWRONBH);
		max8986_free_irq(max8986_ponkey->max8986,
				 MAX8986_IRQID_INT1_PWRONBR);

		if (max8986_ponkey->dev)
			input_free_device(max8986_ponkey->dev);

		kfree(max8986_ponkey);
	}

	return 0;
}

static struct platform_driver max8986_ponkey_driver = {
	.probe = max8986_ponkey_probe,
	.remove = __devexit_p(max8986_ponkey_remove),
	.driver = {
		   .name = "max8986-ponkey",
		   .owner = THIS_MODULE,
		   },
};

static int __init max8986_ponkey_init(void)
{
    
	return platform_driver_register(&max8986_ponkey_driver);
}

module_init(max8986_ponkey_init);

static void __exit max8986_ponkey_exit(void)
{
	platform_driver_unregister(&max8986_ponkey_driver);
}

module_exit(max8986_ponkey_exit);

MODULE_ALIAS("platform:max8986-ponkey");
MODULE_DESCRIPTION("MAX8986 POWER ON pin");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TKG");
