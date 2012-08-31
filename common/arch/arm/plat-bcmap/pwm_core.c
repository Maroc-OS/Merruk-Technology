/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/plat-bcmap/pwm_core.c
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/spinlock_types.h>
#include <mach/pwm.h>
#include <plat/pwm/pwm_core.h>

/*
 * MAX_PWM_DEVICE_ID has to be set to appropriate value by the platform code
 */
#ifndef MAX_PWM_DEVICE_ID
#define MAX_PWM_DEVICE_ID            256
#endif

/* List of pwm devices */
static struct list_head pwm_device_list;
DEFINE_SPINLOCK(pwm_device_list_lock);
/*
 * The caller is required to hold the pwm_device_list_lock.
 */
static struct pwm_channel *pwm_find_device(int pwm_id)
{
	struct pwm_channel *chan = NULL;
	bool found = false;

	/* Search the list to see if the pwm device with the
	 * given id is present.
	 */
	list_for_each_entry(chan, &pwm_device_list, node) {
		if (chan->desc->pwm_id == pwm_id) {
			found = true;
			break;
		}
	}

	return (found == true) ? chan : NULL;
}

/****************************************************************************
 * PWM consumer api
 ****************************************************************************/

struct pwm_device *pwm_request(int pwm_id, const char *label)
{
	struct pwm_device *dev = NULL;
	struct pwm_channel *chan;
	int ret;
	unsigned long flags;

	pr_debug("%s\n", __func__);

	if (pwm_id >= MAX_PWM_DEVICE_ID) {
		pr_debug("%s: invalid pwm id requested\n", __func__);
		goto req_exit;
	}

	spin_lock_irqsave(&pwm_device_list_lock, flags);

	/* Check if the device exists */
	chan = pwm_find_device(pwm_id);
	if (!chan) {
		pr_debug("%s: pwm device with id %d not found\n",
			 __func__, pwm_id);
		goto req_unlock_list;
	}

	/* Check if the device is already in use */
	if (chan->in_use != 0) {
		pr_debug("%s: pwm device already in use\n", __func__);
		goto req_unlock_list;
	}

	/* Create handle for the user */
	dev = kzalloc(sizeof(struct pwm_device), GFP_KERNEL);
	if (!dev) {
		pr_debug("%s: could not allocate pwm_device\n", __func__);
		goto req_unlock_list;
	}

	/* Call the device driver's request call-back */
	if (chan->desc->ops->request) {
		ret = chan->desc->ops->request(chan);
		if (ret != 0) {
			pr_debug("%s: error from driver request callback :"
				 " %d\n", __func__, ret);
			goto req_callback_fail;
		}
	}

	/* Mark the device as in-use */
	chan->in_use++;
	chan->desc->label = (char *)label;
	dev->handle = chan;

	spin_unlock_irqrestore(&pwm_device_list_lock, flags);

	return dev;

req_callback_fail:
	kfree(dev);
req_unlock_list:
	spin_unlock_irqrestore(&pwm_device_list_lock, flags);
req_exit:
	return NULL;
}

EXPORT_SYMBOL(pwm_request);

void pwm_free(struct pwm_device *dev)
{
	struct pwm_channel *chan = dev->handle;
	unsigned long flags;

	pr_debug("%s\n", __func__);

	spin_lock_irqsave(&pwm_device_list_lock, flags);
	if (chan->in_use > 0) {
		chan->in_use--;
		chan->desc->label = NULL;
		if (chan->desc->ops->free)
			chan->desc->ops->free(chan);
		kfree(dev);
	} else {
		pr_debug("%s: trying to free already freed device\n", __func__);
	}
	spin_unlock_irqrestore(&pwm_device_list_lock, flags);
}

EXPORT_SYMBOL(pwm_free);

int pwm_config(struct pwm_device *dev, int duty_ns, int period_ns)
{
	int ret = 0;
	struct pwm_channel *chan = dev->handle;
	unsigned long flags;

	pr_debug("%s\n", __func__);

	spin_lock_irqsave(&pwm_device_list_lock, flags);
	if (chan->desc->ops->config)
		ret = chan->desc->ops->config(chan, duty_ns, period_ns);
	spin_unlock_irqrestore(&pwm_device_list_lock, flags);

	return ret;
}

EXPORT_SYMBOL(pwm_config);

int pwm_setmode(struct pwm_device *dev, int mode)
{
	int ret = 0;
	struct pwm_channel *chan = dev->handle;
	unsigned long flags;

	pr_debug("%s\n", __func__);

	spin_lock_irqsave(&pwm_device_list_lock, flags);
	if (chan->desc->ops->setmode)
		ret = chan->desc->ops->setmode(chan, mode);
	spin_unlock_irqrestore(&pwm_device_list_lock, flags);

	return ret;
}

EXPORT_SYMBOL(pwm_setmode);

int pwm_enable(struct pwm_device *dev)
{
	int ret = 0;
	struct pwm_channel *chan = dev->handle;
	unsigned long flags;

	pr_debug("%s\n", __func__);

	spin_lock_irqsave(&pwm_device_list_lock, flags);
	if (chan->desc->ops->enable)
		ret = chan->desc->ops->enable(chan);
	spin_unlock_irqrestore(&pwm_device_list_lock, flags);

	return ret;
}

EXPORT_SYMBOL(pwm_enable);

void pwm_disable(struct pwm_device *dev)
{
	struct pwm_channel *chan = dev->handle;
	unsigned long flags;

	pr_debug("%s\n", __func__);

	spin_lock_irqsave(&pwm_device_list_lock, flags);
	if (chan->desc->ops->disable)
		chan->desc->ops->disable(chan);
	spin_unlock_irqrestore(&pwm_device_list_lock, flags);
}

EXPORT_SYMBOL(pwm_disable);

/****************************************************************************
 * PWM driver api
 ****************************************************************************/
struct pwm_channel *pwm_register(struct pwm_desc *desc,
				 struct device *dev, void *driver_data)
{
	struct pwm_channel *chan;
	unsigned long flags;

	pr_debug("%s\n", __func__);

	if (!desc)
		return ERR_PTR(-EINVAL);

	if (!desc->ops || desc->pwm_id > MAX_PWM_DEVICE_ID)
		return ERR_PTR(-EINVAL);

	chan = kzalloc(sizeof(struct pwm_channel), GFP_KERNEL);
	if (!chan) {
		pr_debug("%s: could not allocate pwm_channel\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	chan->desc = desc;
	chan->pwm_data = driver_data;
	chan->dev = dev;
	chan->in_use = 0;

	spin_lock_irqsave(&pwm_device_list_lock, flags);
	list_add_tail(&chan->node, &pwm_device_list);
	spin_unlock_irqrestore(&pwm_device_list_lock, flags);

	/*
	 * Add code to register with sysfs and integrate into the device
	 * framework here
	 */

	return chan;
}

EXPORT_SYMBOL(pwm_register);

int pwm_unregister(struct pwm_channel *chan)
{
	unsigned long flags;
	pr_debug("%s\n", __func__);

	spin_lock_irqsave(&pwm_device_list_lock, flags);
	if (chan && (chan->in_use == 0)) {
		list_del(&chan->node);
		kfree(chan);
	} else {
		pr_debug("%s: Trying to remove a device in use!\n", __func__);
		return -EBUSY;
	}
	spin_unlock_irqrestore(&pwm_device_list_lock, flags);

	return 0;
}

EXPORT_SYMBOL(pwm_unregister);

/****************************************************************************
 * SYSFS interface routines
 ****************************************************************************/

/****************************************************************************
 * Device core integration
 ****************************************************************************/

/****************************************************************************
 * Init routines
 ****************************************************************************/
static int __init bcm_pwm_init(void)
{
	pr_debug("%s\n", __func__);

	INIT_LIST_HEAD(&pwm_device_list);

	return 0;
}

subsys_initcall(bcm_pwm_init);

static void __exit bcm_pwm_exit(void)
{
	INIT_LIST_HEAD(&pwm_device_list);
}

module_exit(bcm_pwm_exit);

MODULE_ALIAS("core:bcm-pwm");
MODULE_DESCRIPTION("BCM PWM Core Framework");
MODULE_LICENSE("GPL");
