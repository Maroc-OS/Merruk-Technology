/* drivers/misc/timed_gpio.c
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
//#include <linux/hrtimer.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/timed_gpio.h>

#ifdef CONFIG_REGULATOR
#include <linux/workqueue.h>
#include <linux/regulator/consumer.h>
#endif

#include "timed_output.h"

typedef struct {
	struct delayed_work haptic_delayed_work;
	struct timeval time;
	unsigned 	gpio;
	int 		max_timeout;
	u8 		active_low;
	int enable;
	int value;
	struct regulator * vcc;
} haptic_work_t;

haptic_work_t *haptic_work;

struct timed_gpio_data {
	struct timed_output_dev dev;
	/*struct hrtimer timer;*/
	haptic_work_t *work;
	spinlock_t lock;
	unsigned 	gpio;
	int 		max_timeout;
	u8 		active_low;
	struct regulator	*vcc;
};

/* lock to protect work execution in multi processor systems */
static DEFINE_MUTEX(wq_lock);

static void haptic_regulator_control(struct delayed_work *haptic_delayed_work)
{
	haptic_work_t *reg_work =
		container_of(haptic_delayed_work, haptic_work_t, haptic_delayed_work);

	mutex_lock(&wq_lock);

	if (reg_work->enable) {
		if (reg_work->vcc && !regulator_is_enabled(reg_work->vcc)) {
			/* Enable regulator if not already enabled */
			regulator_enable(reg_work->vcc);
		}

		/* Set GPIO direction */
		gpio_direction_output(reg_work->gpio,
			(reg_work->active_low ?
			!reg_work->value : !!reg_work->value));

		if (reg_work->value > 0) {
			if (reg_work->value > reg_work->max_timeout)
				reg_work->value = reg_work->max_timeout;

			/* compute expiry time */
			do_gettimeofday(&reg_work->time);
			reg_work->time.tv_sec += (reg_work->value / 1000);
			reg_work->time.tv_usec += ((reg_work->value % 1000) * 1000);
			if (reg_work->time.tv_usec > 1000000) {
				reg_work->time.tv_usec -= 1000000;
				reg_work->time.tv_sec++;
			}

			/*
			 * Schedule another work after specified delay to
			 * disable the regulator
			 */
			reg_work->enable = 0;
			schedule_delayed_work((struct delayed_work *)reg_work,
				msecs_to_jiffies(reg_work->value));
		}
	} else {
		if (reg_work->vcc && regulator_is_enabled(reg_work->vcc)) {
			/* Disable regulator here */
			regulator_disable(reg_work->vcc);
		}

		/* Clear time */
		reg_work->time.tv_sec = 0;
		reg_work->time.tv_usec = 0;

		/* toggle GPIO direction */
		gpio_direction_output(reg_work->gpio,
			(reg_work->active_low ? 1 : 0));
	}

	mutex_unlock(&wq_lock);
	return;
}

static int gpio_get_time(struct timed_output_dev *dev)
{
	struct timeval curr_time, exp_time;
	struct timed_gpio_data	*data =
		container_of(dev, struct timed_gpio_data, dev);
	unsigned long	flags;

	spin_lock_irqsave(&data->lock, flags);

	if (data->work->time.tv_sec || data->work->time.tv_usec) {
		do_gettimeofday(&curr_time);

		exp_time.tv_sec = data->work->time.tv_sec - curr_time.tv_sec;
		exp_time.tv_usec = data->work->time.tv_usec - curr_time.tv_usec;

		spin_unlock_irqrestore(&data->lock, flags);
		return exp_time.tv_sec * 1000 + exp_time.tv_usec / 1000;
	}

	spin_unlock_irqrestore(&data->lock, flags);
	return 0;
}

static void gpio_enable(struct timed_output_dev *dev, int value)
{
	struct timed_gpio_data	*data =
		container_of(dev, struct timed_gpio_data, dev);
	unsigned long	flags;

	spin_lock_irqsave(&data->lock, flags);

	if (haptic_work) {
		haptic_work->vcc = data->vcc;
		haptic_work->gpio = data->gpio;
		haptic_work->active_low = data->active_low;
		haptic_work->max_timeout = data->max_timeout;
		haptic_work->value = value;
		haptic_work->enable = 1;

		/* Schedule work with a delay of zero to enable regulator */
		schedule_delayed_work((struct delayed_work *)haptic_work, 0);
	}

	spin_unlock_irqrestore(&data->lock, flags);
}

static int timed_gpio_probe(struct platform_device *pdev)
{
	struct timed_gpio_platform_data *pdata = pdev->dev.platform_data;
	struct timed_gpio *cur_gpio;
	struct timed_gpio_data *gpio_data, *gpio_dat;
	int i, j, ret = 0;

	if (!pdata)
		return -EBUSY;

	gpio_data = kzalloc(sizeof(struct timed_gpio_data) * pdata->num_gpios,
			GFP_KERNEL);
	if (!gpio_data)
		return -ENOMEM;

	for (i = 0; i < pdata->num_gpios; i++) {
		cur_gpio = &pdata->gpios[i];
		gpio_dat = &gpio_data[i];

		spin_lock_init(&gpio_dat->lock);

		gpio_dat->dev.name = cur_gpio->name;
		gpio_dat->dev.get_time = gpio_get_time;
		gpio_dat->dev.enable = gpio_enable;

		/* INIT_WORK to system work queue */
		haptic_work = (haptic_work_t *)kzalloc(sizeof(haptic_work_t), GFP_KERNEL);
		if (!haptic_work)
			return -ENOMEM;

		gpio_dat->work = haptic_work;
		INIT_DELAYED_WORK(&haptic_work->haptic_delayed_work, haptic_regulator_control);

		if (cur_gpio->regl_id) {
			/* Fetch a regulator */
			gpio_dat->vcc = regulator_get(NULL, cur_gpio->regl_id);
			if (IS_ERR(gpio_dat->vcc))
				gpio_dat->vcc = NULL;
		}

		ret = gpio_request(cur_gpio->gpio, cur_gpio->name);
		if (ret >= 0) {
			ret = timed_output_dev_register(&gpio_dat->dev);
			if (ret < 0)
				gpio_free(cur_gpio->gpio);
		}
		if (ret < 0) {
			for (j = 0; j < i; j++) {
				timed_output_dev_unregister(&gpio_data[i].dev);
				gpio_free(gpio_data[i].gpio);
			}
			kfree(gpio_data);
			return ret;
		}

		gpio_dat->gpio = cur_gpio->gpio;
		gpio_dat->max_timeout = cur_gpio->max_timeout;
		gpio_dat->active_low = cur_gpio->active_low;
		gpio_direction_output(gpio_dat->gpio, gpio_dat->active_low);
	}

	platform_set_drvdata(pdev, gpio_data);

	return 0;
}

static int timed_gpio_remove(struct platform_device *pdev)
{
	struct timed_gpio_platform_data *pdata = pdev->dev.platform_data;
	struct timed_gpio_data *gpio_data = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < pdata->num_gpios; i++) {
		timed_output_dev_unregister(&gpio_data[i].dev);
		gpio_free(gpio_data[i].gpio);
	}

	if (haptic_work)
		kfree((void *)haptic_work);

	kfree(gpio_data);

	return 0;
}

static struct platform_driver timed_gpio_driver = {
	.probe		= timed_gpio_probe,
	.remove		= timed_gpio_remove,
	.driver		= {
		.name		= TIMED_GPIO_NAME,
		.owner		= THIS_MODULE,
	},
};

static int __init timed_gpio_init(void)
{
	return platform_driver_register(&timed_gpio_driver);
}

static void __exit timed_gpio_exit(void)
{
	platform_driver_unregister(&timed_gpio_driver);
}

module_init(timed_gpio_init);
module_exit(timed_gpio_exit);

MODULE_AUTHOR("Mike Lockwood <lockwood@android.com>");
MODULE_DESCRIPTION("timed gpio driver");
MODULE_LICENSE("GPL");
