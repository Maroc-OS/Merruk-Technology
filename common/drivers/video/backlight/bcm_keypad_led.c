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
#include <linux/bcm_keypad_led.h>
#include <plat/gpio.h>
#include <linux/delay.h>
#include <linux/broadcom/lcd.h>
#include <mach/reg_syscfg.h>

#define KEYLED_DEBUG 0

#if KEYLED_DEBUG
#define KLDBG(fmt, args...) printk(fmt, ## args)
#else
#define KLDBG(fmt, args...)
#endif

struct keypad_led_data {
	struct platform_device *pdev;
	unsigned int ctrl_pin;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_desc;
#endif
};

int keyled_status = 0;
static int keyled_pin = 62;

#define KEYLED_ON   1
#define KEYLED_OFF 0

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);

void Keypad_Led_control_On(void)
{
    writel(readl(ADDR_SYSCFG_IOCR0) |SYSCFG_IOCR0_DIGMIC_GPIO6362_MUX , ADDR_SYSCFG_IOCR0);

    gpio_request(keyled_pin, "KEYLED_EN");
    gpio_direction_output(keyled_pin, 1);       

    keyled_status = KEYLED_ON;                
    KLDBG("[KEYLED] ON: SET[%d], GET[%d]\n", 1, gpio_get_value(keyled_pin));    

}
EXPORT_SYMBOL(Keypad_Led_control_On);

/* input: intensity in percentage 0% - 100% */
static int bcm_keypad_led_update_status(struct backlight_device *bd)
{
    struct keypad_led_data *keyled= dev_get_drvdata(&bd->dev);
    int user_intensity = bd->props.brightness;

    KLDBG("[KEYLED] bcm_keypad_led_update_status : %d\n", user_intensity);
    
    writel(readl(ADDR_SYSCFG_IOCR0) |SYSCFG_IOCR0_DIGMIC_GPIO6362_MUX , ADDR_SYSCFG_IOCR0);

    gpio_request(keyled->ctrl_pin, "KEYLED_EN");
    
    if(user_intensity==0)
    {
        gpio_direction_output(keyled->ctrl_pin, KEYLED_OFF);        
        keyled_status = KEYLED_ON;           

        KLDBG("[KEYLED] OFF: SET[%d], GET[%d]\n", 0, gpio_get_value(keyled->ctrl_pin));        
    }
    else
    {
        gpio_direction_output(keyled->ctrl_pin, KEYLED_ON);
        keyled_status = KEYLED_OFF;            
        
        KLDBG("[KEYLED] ON: SET[%d], GET[%d]\n", 1, gpio_get_value(keyled->ctrl_pin));              

    }
    
    return 0;
}

static int bcm_keypad_led_get_brightness(struct backlight_device *bl)
{
        KLDBG("[KEYLED] bcm_keypad_led_get_brightness\n");
    
	return 0;
}

static struct backlight_ops bcm_keypad_led_ops = {
	.update_status	= bcm_keypad_led_update_status,
	.get_brightness	= bcm_keypad_led_get_brightness,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bcm_keypad_led_earlysuspend(struct early_suspend *desc)
{
    struct  keypad_led_data *keyled = container_of(desc, struct keypad_led_data, early_suspend_desc);    
    struct backlight_device *bl = platform_get_drvdata(keyled->pdev);
    
    KLDBG("[KEYLED] bcm_keypad_led_earlysuspend\n");

    bcm_gpio_pull_up(keyled_pin, false);
    bcm_gpio_pull_up_down_enable(keyled_pin, true);        
}

static void bcm_keypad_led_earlyresume(struct early_suspend *desc)
{
    struct keypad_led_data *keyled = container_of(desc, struct keypad_led_data, early_suspend_desc);
    struct backlight_device *bl = platform_get_drvdata(keyled->pdev);

    KLDBG("[KEYLED] bcm_keypad_led_earlyresume\n");
    
    bcm_keypad_led_update_status(bl);

    bcm_gpio_pull_up(keyled_pin, false);
    bcm_gpio_pull_up_down_enable(keyled_pin, true);         
}
#else
#ifdef CONFIG_PM
static int bcm_keypad_led_suspend(struct platform_device *pdev,
					pm_message_t state)
{
    struct backlight_device *bl = platform_get_drvdata(pdev);
    struct keypad_led_data *keyled = dev_get_drvdata(&bl->dev);

    KLDBG("[KEYLED] bcm_keypad_led_suspend\n");

    bcm_keypad_led_update_status(bl);
    gpio_direction_output(keyled->ctrl_pin, KEYLED_OFF);

    return 0;
}

static int bcm_keypad_led_resume(struct platform_device *pdev)
{
    struct backlight_device *bl = platform_get_drvdata(pdev);
    struct keypad_led_data *keyled = dev_get_drvdata(&bl->dev);

    KLDBG("[KEYLED] bcm_keypad_led_resume\n");

    gpio_direction_output(keyled->ctrl_pin, KEYLED_ON);      
    bcm_keypad_led_update_status(bl);

    return 0;
}
#else
#define bcm_keypad_led_suspend  NULL
#define bcm_keypad_led_resume   NULL
#endif
#endif

static int bcm_keypad_led_probe(struct platform_device *pdev)
{
	struct platform_keypad_led_data *data = pdev->dev.platform_data;
	struct backlight_device *bl;
	struct keypad_led_data *keyled;
    	struct backlight_properties props;
	int ret;

        KLDBG("[KEYLED] bcm_keypad_led_probe, 0x%x\n", readl(HW_SYSCFG_BASE));
        
	if (!data) {
		dev_err(&pdev->dev, "failed to find platform data\n");
		return -EINVAL;
	}

	keyled = kzalloc(sizeof(*keyled), GFP_KERNEL);
	if (!keyled) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	keyled->ctrl_pin = data->ctrl_pin;
    
	memset(&props, 0, sizeof(struct backlight_properties));
	props.max_brightness = data->max_brightness;

	bl = backlight_device_register(pdev->name, &pdev->dev,
			keyled, &bcm_keypad_led_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		ret = PTR_ERR(bl);
		goto err_bl;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	keyled->pdev = pdev;
	keyled->early_suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN-1;
	keyled->early_suspend_desc.suspend = bcm_keypad_led_earlysuspend;
	keyled->early_suspend_desc.resume = bcm_keypad_led_earlyresume;
	register_early_suspend(&keyled->early_suspend_desc);
#endif

	bl->props.max_brightness = data->max_brightness;
	bl->props.brightness = data->default_brightness;

        platform_set_drvdata(pdev, bl);

        bcm_keypad_led_update_status(bl);
        
        bcm_gpio_pull_up(keyled_pin, false);
        bcm_gpio_pull_up_down_enable(keyled_pin, true);  
        
	return 0;

err_bl:
	kfree(keyled);
err_alloc:
	return ret;
}

static int bcm_keypad_led_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct keypad_led_data *keyled = dev_get_drvdata(&bl->dev);

	backlight_device_unregister(bl);


#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&keyled->early_suspend_desc);
#endif

	kfree(keyled);
	return 0;
}


static struct platform_driver keypad_led_driver = {
	.driver		= {
		.name	= "keypad-led",
		.owner	= THIS_MODULE,
	},
	.probe		= bcm_keypad_led_probe,
	.remove		= bcm_keypad_led_remove,

#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend        = bcm_keypad_led_suspend,
	.resume         = bcm_keypad_led_resume,
#endif

};

static int __init bcm_keypad_led_init(void)
{
    KLDBG("[KEYLED] bcm_keypad_led_init\n");

    return platform_driver_register(&keypad_led_driver);
}
module_init(bcm_keypad_led_init);

static void __exit bcm_keypad_led_exit(void)
{
	platform_driver_unregister(&keypad_led_driver);
}
module_exit(bcm_keypad_led_exit);

MODULE_DESCRIPTION("Keyapd LED Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:keypad-led");


