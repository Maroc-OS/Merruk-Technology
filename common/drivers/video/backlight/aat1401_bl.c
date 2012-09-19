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
#include <linux/aat1401_bl.h>
#include <plat/gpio.h>
#include <linux/delay.h>
#include <linux/broadcom/lcd.h>
#include <linux/spinlock.h>
#include <linux/broadcom/PowerManager.h>
#include <mach/reg_syscfg.h>
#include <linux/rtc.h>

int current_intensity;
static int backlight_pin = 17;

static DEFINE_SPINLOCK(bl_ctrl_lock);
static int lcd_brightness = 0;
int real_level = 13;

#ifdef CONFIG_HAS_EARLYSUSPEND
/* early suspend support */
extern int gLcdfbEarlySuspendStopDraw;
#endif
static int backlight_mode=1;

#define MAX_BRIGHTNESS_IN_BLU	33
#if defined(CONFIG_BACKLIGHT_TOTORO)
#define DIMMING_VALUE		2
#elif defined(CONFIG_BACKLIGHT_LUISA)
#define DIMMING_VALUE		1
#elif defined(CONFIG_BACKLIGHT_TASSVE)
#define DIMMING_VALUE		2
#elif defined(CONFIG_BACKLIGHT_COOPERVE)
#define DIMMING_VALUE		2
#else
#define DIMMING_VALUE		1
#endif
#define MAX_BRIGHTNESS_VALUE	255
#define MIN_BRIGHTNESS_VALUE	30
#define BACKLIGHT_DEBUG 0
#define BACKLIGHT_SUSPEND 0
#define BACKLIGHT_RESUME 1

#if BACKLIGHT_DEBUG
#define BLDBG(fmt, args...) printk(fmt, ## args)
#else
#define BLDBG(fmt, args...)
#endif

struct aat1401_bl_data {
	struct platform_device *pdev;
	unsigned int ctrl_pin;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_desc;
#endif
};

struct brt_value{
	int level;				// Platform setting values
	int tune_level;			// Chip Setting values
};

#if defined(CONFIG_BACKLIGHT_TOTORO)
struct brt_value brt_table_ktd[] = {
	{ MIN_BRIGHTNESS_VALUE,  2 }, // Min pulse 27(33-6) by HW 
	{ 39,  3 }, 
	{ 48,  4 }, 
	{ 57,  5 }, 
	{ 66,  6 }, 
	{ 75,  7 }, 
	{ 84,  8 },  
	{ 93,  9 }, 
	{ 102,	10 }, 
	{ 111,	11 },	
	{ 120,	12 }, 
	{ 129,	13 }, 
	{ 138,  14 },//default value 21 (33-12) 
	{ 147,	15 }, 
	{ 155,  16 },
	{ 163,	17 },  
	{ 170,	18 },  
	{ 178,	19 }, 
	{ 186,  20 },
	{ 194,  21 }, 
	{ 202,  22 },
	{ 210,	23 },  
	{ 219,	24 }, 
	{ 228,	25 }, 
	{ 237,  26 },  
	{ 246,	27 }, 
	{ MAX_BRIGHTNESS_VALUE,  28 }, // Max pulse 7(33-26) by HW

};
#elif defined(CONFIG_BACKLIGHT_LUISA)
struct brt_value brt_table_ktd[] = {
	  { MIN_BRIGHTNESS_VALUE,  1 }, // Min pulse 32(33-1) by HW 
	  { 40,  2 }, 
	  { 51,  3 }, 
	  { 61,  4 },
	  { 72,  5 }, 
	  { 83,  6 }, 
	  { 93,  7 }, 
	  { 104,  8 }, 
	  { 115,  9 }, 
	  { 125,  10 }, 
	  { 136,  11 }, 
	  { 147,  12 },//default value 21 (33-12)
	  { 157,  13 }, 
	  { 168,  14 }, 
	  { 179,  15 },
	  { 190,  16 },
	  { 201,  17 },
	  { 222,  18 },
	  { 233,  19 },
	  { 244,  20 },
	  { 250,  21 },
	  { MAX_BRIGHTNESS_VALUE,  22 }, // Max pulse 11(33-22) by HW
};
#elif defined(CONFIG_BACKLIGHT_TASSVE)
struct brt_value brt_table_ktd[] = {
	  { MIN_BRIGHTNESS_VALUE,  2 }, // Min pulse 31(33-2) by HW 
	  { 37,  3 }, 
	  { 45,  4 }, 
	  { 53,  5 }, 
	  { 61,  6 }, 
	  { 69,  7 }, 
	  { 76,  8 }, 
	  { 84,  9 },
	  { 92,  10 }, 
	  { 100,  11 },  
	  { 108,  12 }, 
	  { 115,  13 },  
	  { 123,  14 }, 
	  { 131,  15 }, 	  
	  { 139,  16 },
	  { 147,  17 },//default value 16(33-17) 
	  { 156,  18 },
	  { 165,  19 },
	  { 174,  20 },
	  { 183,  21 }, 
	  { 192,  22 }, 
	  { 201,  23 },
	  { 210,  24 },
	  { 219,  25 },	  
	  { 228,  26 }, 
	  { 237,  27 },
	  { 246,  28 }, 	  
	  { MAX_BRIGHTNESS_VALUE,  29 }, // Max pulse 4(33-29) by HW
};
#elif defined(CONFIG_BACKLIGHT_COOPERVE)
struct brt_value brt_table_ktd[] = {
   { MIN_BRIGHTNESS_VALUE,  2 }, // Min pulse 31(33-2) by HW 
   { 37,  3 },  
   { 44,  4 }, 
   { 51,  5 }, 
   { 58,  6 }, 
   { 65,  7 }, 
   { 72,  8 }, 
   { 79,  9 }, 
   { 86,  10 }, 
   { 93,  11 }, 
   { 100,  12 }, 
   { 108,  13 }, 
   { 115,  14 },  
   { 123,  15 },  
   { 131,  16 }, 
   { 139,  17 }, 
   { 147,  18 },//default value   
   { 156,  19 },
   { 165,  20 }, 
   { 174,  21 }, 
   { 183,  22 }, 
   { 192,  23 }, 
   { 201,  24 },
   { 210,  25 },
   { 219,  26 },
   { 228,  27 },
   { 237,  28 },
   { 246,  29 },
   { MAX_BRIGHTNESS_VALUE,  30 }, // Max pulse 3(33-30) by HW
};
#else
struct brt_value brt_table_ktd[] = {
	  { MIN_BRIGHTNESS_VALUE,  2 }, // Min pulse 27(33-6) by HW 
	  { 39,  3 }, 
	  { 48,  4 }, 
	  { 57,  5 }, 
	  { 67,  6 }, 
	  { 77,  7 }, 
	  { 87,  8 },  
	  { 97,  9 }, 
	  { 107,  10 }, 
	  { 117,  11 },   
	  { 127,  12 }, 
	  { 137,  13 }, 
	  { 147,  14 }, 
      { 156,  15 },
	  { 165,  16 },  
	  { 174,  17 }, 
	  { 183,  18 }, 
	  { 192,  19 }, 
      { 201,  20 },
      { 210,  21 }, 
      { 219,  22 },
	  { 228,  23 },  
	  { 237,  24 }, 
	  { 246,  25 }, 
	  { MAX_BRIGHTNESS_VALUE,  26 }, // Max pulse 7(33-26) by HW
};
#endif

#define MAX_BRT_STAGE_KTD (int)(sizeof(brt_table_ktd)/sizeof(struct brt_value))

extern int lcd_pm_update(PM_CompPowerLevel compPowerLevel, PM_PowerLevel sysPowerLevel);

static void lcd_backlight_control(int num)
{
    int limit;
    
    limit = num;

   spin_lock(&bl_ctrl_lock);
    for(;limit>0;limit--)
    {
       udelay(2);
       gpio_set_value(backlight_pin,0);
       udelay(2); 
       gpio_set_value(backlight_pin,1);
    }
   spin_unlock(&bl_ctrl_lock);

}

/* input: intensity in percentage 0% - 100% */
static int aat1401_backlight_update_status(struct backlight_device *bd)
{
    	struct aat1401_bl_data *aat1401= dev_get_drvdata(&bd->dev);
	int user_intensity = bd->props.brightness;
    	int tune_level = 0;
	int pulse;
      int i;

        BLDBG("[BACKLIGHT] aat1401_backlight_update_status ==> user_intensity  : %d\n", user_intensity);
if(backlight_mode==BACKLIGHT_RESUME)
{
    if(gLcdfbEarlySuspendStopDraw==0)
    {
    		if(user_intensity > 0) {
			if(user_intensity < MIN_BRIGHTNESS_VALUE) {
				tune_level = DIMMING_VALUE; //DIMMING
			} else if (user_intensity == MAX_BRIGHTNESS_VALUE) {
				tune_level = brt_table_ktd[MAX_BRT_STAGE_KTD-1].tune_level;
			} else {
				for(i = 0; i < MAX_BRT_STAGE_KTD; i++) {
					if(user_intensity <= brt_table_ktd[i].level ) {
						tune_level = brt_table_ktd[i].tune_level;
						break;
					}
				}
			}
		}

        BLDBG("[BACKLIGHT] aat1401_backlight_update_status ==> tune_level : %d\n", tune_level);

    if (real_level==tune_level)
    {
        return 0;
    }
    else
    {
    if(real_level == 0)
    {
	mdelay(200);
    }
    
    if(tune_level<=0)
    {
                gpio_set_value(backlight_pin,0);
                mdelay(3); 

    }
    else
    {

            pulse = MAX_BRIGHTNESS_IN_BLU -tune_level;
            if (pulse==0)
            {
                return 0;
            }
            gpio_set_value(backlight_pin,0);
            udelay(80);      
            lcd_backlight_control(pulse); 
            udelay(500);   
    }

    real_level = tune_level;
	return 0;
    }
    }
}
}

static int aat1401_backlight_get_brightness(struct backlight_device *bl)
{
        BLDBG("[BACKLIGHT] aat1401_backlight_get_brightness\n");
    
	return current_intensity;
}

static struct backlight_ops aat1401_backlight_ops = {
	.update_status	= aat1401_backlight_update_status,
	.get_brightness	= aat1401_backlight_get_brightness,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void aat1401_backlight_earlysuspend(struct early_suspend *desc)
{
    struct aat1401_bl_data *aat1401 = container_of(desc, struct aat1401_bl_data,
			early_suspend_desc);
    struct backlight_device *bl = platform_get_drvdata(aat1401->pdev);
    struct timespec ts;
    struct rtc_time tm;
 
    backlight_mode=BACKLIGHT_SUSPEND; 
	
	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);

    printk("[%02d:%02d:%02d.%03lu][BACKLIGHT] earlysuspend\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
}

static void aat1401_backlight_earlyresume(struct early_suspend *desc)
{
	struct aat1401_bl_data *aat1401 = container_of(desc, struct aat1401_bl_data,
				early_suspend_desc);
	struct backlight_device *bl = platform_get_drvdata(aat1401->pdev);
      struct timespec ts;
      struct rtc_time tm;
    
	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);
       backlight_mode=BACKLIGHT_RESUME;
       printk("[%02d:%02d:%02d.%03lu][BACKLIGHT] earlyresume\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
       backlight_update_status(bl);
}
#else
#ifdef CONFIG_PM
static int aat1401_backlight_suspend(struct platform_device *pdev,
					pm_message_t state)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct aat1401_bl_data *aat1401 = dev_get_drvdata(&bl->dev);
    
        BLDBG("[BACKLIGHT] aat1401_backlight_suspend\n");
        
	return 0;
}

static int aat1401_backlight_resume(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);

        BLDBG("[BACKLIGHT] aat1401_backlight_resume\n");
        
	  backlight_update_status(bl);
        
	return 0;
}
#else
#define aat1401_backlight_suspend  NULL
#define aat1401_backlight_resume   NULL
#endif
#endif

static int aat1401_backlight_probe(struct platform_device *pdev)
{
	struct platform_aat1401_backlight_data *data = pdev->dev.platform_data;
	struct backlight_device *bl;
	struct aat1401_bl_data *aat1401;
    	struct backlight_properties props;
	int ret;

        BLDBG("[BACKLIGHT] aat1401_backlight_probe\n");
    
	if (!data) {
		dev_err(&pdev->dev, "failed to find platform data\n");
		return -EINVAL;
	}

	aat1401 = kzalloc(sizeof(*aat1401), GFP_KERNEL);
	if (!aat1401) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	aat1401->ctrl_pin = data->ctrl_pin;
    
	memset(&props, 0, sizeof(struct backlight_properties));
	props.max_brightness = data->max_brightness;

	bl = backlight_device_register(pdev->name, &pdev->dev,
			aat1401, &aat1401_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		ret = PTR_ERR(bl);
		goto err_bl;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	aat1401->pdev = pdev;
	aat1401->early_suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	aat1401->early_suspend_desc.suspend = aat1401_backlight_earlysuspend;
	aat1401->early_suspend_desc.resume = aat1401_backlight_earlyresume;
	register_early_suspend(&aat1401->early_suspend_desc);
#endif

	bl->props.max_brightness = data->max_brightness;
	bl->props.brightness = data->dft_brightness;

	platform_set_drvdata(pdev, bl);

      	aat1401_backlight_update_status(bl);
    
	return 0;

err_bl:
	kfree(aat1401);
err_alloc:
	return ret;
}

static int aat1401_backlight_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct aat1401_bl_data *aat1401 = dev_get_drvdata(&bl->dev);

	backlight_device_unregister(bl);


#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&aat1401->early_suspend_desc);
#endif

	kfree(aat1401);
	return 0;
}


static struct platform_driver aat1401_backlight_driver = {
	.driver		= {
		.name	= "aat1401-backlight",
		.owner	= THIS_MODULE,
	},
	.probe		= aat1401_backlight_probe,
	.remove		= aat1401_backlight_remove,

#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend        = aat1401_backlight_suspend,
	.resume         = aat1401_backlight_resume,
#endif

};

static int __init aat1401_backlight_init(void)
{
	return platform_driver_register(&aat1401_backlight_driver);
}
module_init(aat1401_backlight_init);

static void __exit aat1401_backlight_exit(void)
{
	platform_driver_unregister(&aat1401_backlight_driver);
}
module_exit(aat1401_backlight_exit);

MODULE_DESCRIPTION("AAT1401 based Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:aat1401-backlight");

