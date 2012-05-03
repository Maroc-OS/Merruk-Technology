/*
 * linux/drivers/video/backlight/s2c_bl.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/backlight/s2c_bl.c
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
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/s2c_bl.h>
#include <plat/gpio.h>
#include <linux/delay.h>
#include <linux/broadcom/lcd.h>

struct s2c_bl_data {
	struct platform_device *pdev;
	unsigned int ctrl_pin;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_desc;
#endif
};

static void s2c_disable_backlight(unsigned int ctrl_pin)
{
	gpio_set_value(ctrl_pin, 0);

	/* Delay of TLR to be provided, as specified in datasheet */
	udelay(500);
}

void s2c_set_brightness(unsigned int ctrl_pin, int brightness, int max_brightness)
{
#if 0
	/* The below level is based on following table (taking
	 * values that are same for both main and sub-group)
	 * Data         D1 - D3         D4
	 *  1             20            20
	 *  2             14            14
	 *  3             10            10
	 *  4              7             7
	 *  5             20             0
	 *  6             14             0
	 *  7             10             0
	 *  8              7             0
	 *  9              0            20
	 *  10             0            14
	 *  11             0            10
	 *  12             0             7
	 *  13          0.05          0.05
	 *  14           0.5           0.5
	 *  15             1             1
	 *  16             2             2
	 */
	u8 level[8] = { 14, 16, 4, 4, 3, 3, 2, 1 };
	int i;

	brightness = (brightness / 4);
	if (brightness > (sizeof(level) - 1))
		brightness = sizeof(level) - 1;

	for (i = 0; i < level[brightness]; ++i) {
		gpio_set_value(ctrl_pin, 0);
		/* Max frequency at which gpio can toggle is 1 Mhz
		 * for the S2C to take notice of the toggle */
		udelay(1);
		gpio_set_value(ctrl_pin, 1);
	}

#endif
	int s2c_bright;
	int i;
	
	if (brightness > max_brightness) brightness = max_brightness;

	if (brightness == 0)
		s2c_bright = 32;
	else 
		s2c_bright = 33 - 32 * brightness/max_brightness;
	
	for (i = 0; i < s2c_bright; ++i) {
		gpio_set_value(ctrl_pin, 0);
		/* Max frequency at which gpio can toggle is 1 Mhz
		 * for the S2C to take notice of the toggle */
		udelay(1);
		gpio_set_value(ctrl_pin, 1);
	}
	
	
	/* Delay of TLR to be provided, as specified in datasheet */
	udelay(500);
}

static int s2c_backlight_update_status(struct backlight_device *bl)
{
	struct s2c_bl_data *s2c = dev_get_drvdata(&bl->dev);
	int brightness = bl->props.brightness;
	if (brightness == 0)
		s2c_disable_backlight(s2c->ctrl_pin);
	else {
		s2c_set_brightness(s2c->ctrl_pin, brightness, bl->props.max_brightness);
	}
	return 0;
}

static int s2c_backlight_get_brightness(struct backlight_device *bl)
{
	return bl->props.brightness;
}

static struct backlight_ops s2c_backlight_ops = {
	.update_status = s2c_backlight_update_status,
	.get_brightness = s2c_backlight_get_brightness,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void s2c_backlight_earlysuspend(struct early_suspend *desc)
{
	struct s2c_bl_data *s2c = container_of(desc, struct s2c_bl_data,
					       early_suspend_desc);

	s2c_disable_backlight(s2c->ctrl_pin);
}

static void s2c_backlight_earlyresume(struct early_suspend *desc)
{
	struct s2c_bl_data *s2c = container_of(desc, struct s2c_bl_data,
					       early_suspend_desc);
	struct backlight_device *bl = platform_get_drvdata(s2c->pdev);

	backlight_update_status(bl);
}
#else
#ifdef CONFIG_PM
static int s2c_backlight_suspend(struct platform_device *pdev,
				 pm_message_t state)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct s2c_bl_data *s2c = dev_get_drvdata(&bl->dev);

	s2c_disable_backlight(s2c->ctrl_pin);
	return 0;
}

static int s2c_backlight_resume(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);

	backlight_update_status(bl);
	return 0;
}
#else
#define s2c_backlight_suspend  NULL
#define s2c_backlight_resume   NULL
#endif
#endif

static int s2c_backlight_probe(struct platform_device *pdev)
{
	struct backlight_properties props;
	struct platform_s2c_backlight_data *data = pdev->dev.platform_data;
	struct backlight_device *bl;
	struct s2c_bl_data *s2c;
	int ret;

	if (!data) {
		dev_err(&pdev->dev, "failed to find platform data\n");
		return -EINVAL;
	}

	s2c = kzalloc(sizeof(*s2c), GFP_KERNEL);
	if (!s2c) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	s2c->ctrl_pin = data->ctrl_pin;

	memset(&props, 0, sizeof(struct backlight_properties));
	props.max_brightness = data->max_brightness;
	bl = backlight_device_register(pdev->name, &pdev->dev,
				       s2c, &s2c_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		ret = PTR_ERR(bl);
		goto err_bl;
	}

	gpio_request(data->ctrl_pin, "s2c-backlight");
	gpio_direction_output(data->ctrl_pin, 1);
	s2c_set_brightness(data->ctrl_pin, data->max_brightness, data->max_brightness);
#ifdef CONFIG_HAS_EARLYSUSPEND
	s2c->pdev = pdev;
	s2c->early_suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	s2c->early_suspend_desc.suspend = s2c_backlight_earlysuspend;
	s2c->early_suspend_desc.resume = s2c_backlight_earlyresume;
	register_early_suspend(&s2c->early_suspend_desc);
#endif

	bl->props.max_brightness = data->max_brightness;
	bl->props.brightness = data->dft_brightness;
	backlight_update_status(bl);

	platform_set_drvdata(pdev, bl);
	return 0;

      err_bl:
	kfree(s2c);
      err_alloc:
	return ret;
}

static int s2c_backlight_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct s2c_bl_data *s2c = dev_get_drvdata(&bl->dev);

	backlight_device_unregister(bl);
	s2c_disable_backlight(s2c->ctrl_pin);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&s2c->early_suspend_desc);
#endif

	kfree(s2c);
	return 0;
}

static struct platform_driver s2c_backlight_driver = {
	.driver = {
		   .name = "s2c-backlight",
		   .owner = THIS_MODULE,
		   },
	.probe = s2c_backlight_probe,
	.remove = s2c_backlight_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = s2c_backlight_suspend,
	.resume = s2c_backlight_resume,
#endif
};

static int __init s2c_backlight_init(void)
{
	return platform_driver_register(&s2c_backlight_driver);
}

module_init(s2c_backlight_init);

static void __exit s2c_backlight_exit(void)
{
	platform_driver_unregister(&s2c_backlight_driver);
}

module_exit(s2c_backlight_exit);

MODULE_DESCRIPTION("S2C based Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:s2c-backlight");
