 /*
  *  arch/arm/plat-bcmap/bcm_avs.c
  *
  *  AVS (Advaptive Voltage Scaling) driver for BCM Chips
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 2 as
  * published by the Free Software Foundation.
  */

/*******************************************************************************
* Copyright 2010,2011 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/plat-bcmap/bcm_avs.c
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
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <plat/bcm_otp.h>
#include <plat/bcm_avs.h>
#include <linux/platform_device.h>

#define OTP_BITS_PER_ROW 32

static const char* silicon_type_str[] = { "slow", "typical", "fast","unknown"};


static int silicon_type = SILICON_TYPE_UNKNOWN;
module_param_named(silicon_type, silicon_type, int, S_IRUGO | S_IWUSR | S_IWGRP);
static int silicon_otp_val;
module_param_named(silicon_otp_val, silicon_otp_val, int, S_IRUGO | S_IWUSR | S_IWGRP);

static int _bcm_avs_get_silicon_type(const struct bcm_avs_platform_data_t *pdata)
{
	int type = SILICON_TYPE_UNKNOWN;
	int row;
	u32 num_bits;
	u32 otp_val;
	num_bits = pdata->otp_bit_msb - pdata->otp_bit_lsb + 1;

	if(num_bits < OTP_BITS_PER_ROW)
	{
		row = pdata->otp_bit_lsb >> 5;	/*Divide  by 32 to get row number */
		if(otp_read_word(row, 0, &otp_val) == 0)
		{
			otp_val >>= pdata->otp_bit_lsb & 0x1F; /*otp_bit_lsb % 32*/
			otp_val &= 0xFFFFFFFF >> (OTP_BITS_PER_ROW - num_bits);
			pr_info("%s : otp_val = %d\n", __func__, otp_val);
			silicon_otp_val = otp_val;
			if(otp_val > pdata->fast_silicon_thold)
			{
				type = SILICON_TYPE_FAST;
			}
			else if(otp_val < pdata->slow_silicon_thold)
			{
				type = SILICON_TYPE_SLOW;
			}
			else
				type = SILICON_TYPE_TYPICAL;
		}
	}
	return type;
}

static int bcm_avs_drv_probe(struct platform_device *pdev)
{
	struct bcm_avs_platform_data_t *pdata = pdev->dev.platform_data;
	struct silicon_type_info *type_info = NULL;
	int ret = 0;
	struct regulator *regl;

	pr_info("%s\n", __func__);

	if(!pdata)
	{
		pr_info("%s : invalid paltform data !!\n", __func__);
		ret = -EPERM;
		goto error;
	}
	if((pdata->otp_bit_msb - pdata->otp_bit_lsb + 1) > OTP_BITS_PER_ROW)
	{
		pr_info("%s : invalid param: num otp bits > 32 !!\n",
				 __func__);
		ret = -EPERM;
		goto error;
	}

	silicon_type = _bcm_avs_get_silicon_type(pdata);
	pr_info("%s:AVS --  Silicon Type = %s !!\n", __func__, silicon_type_str[silicon_type]);
	switch(silicon_type)
	{
	case SILICON_TYPE_FAST:
		type_info = pdata->fast_silicon;
		break;
	case SILICON_TYPE_SLOW:
		type_info = pdata->slow_silicon;
		break;

	case SILICON_TYPE_TYPICAL:
		type_info = pdata->typical_silicon;
		break;

	default:
		pr_info("%s:AVS --  Silicon Type UNKNOWN !!\n", __func__);
		goto error;
	}

	if(!type_info)
	{
		pr_info("%s : invalid param: silicon_type_info is NULL !!\n",
			__func__);
		ret = -EPERM;
		goto error;
	}

	if(pdata->core_lpm_regl && type_info->lpm_voltage != -1)
	{
		regl = regulator_get(NULL, pdata->core_lpm_regl);
		if(IS_ERR(regl))
		{
			pr_info("%s: Core LPM regulator_get failed\n",
				__func__);
			ret = PTR_ERR(regl);
			goto error;
		}
		regulator_set_voltage(regl, type_info->lpm_voltage,
				      type_info->lpm_voltage);
		regulator_put(regl);
	}
	if(pdata->core_nml_regl && type_info->nm_voltage != -1)
	{
		regl = regulator_get(NULL, pdata->core_nml_regl);
		if(IS_ERR(regl))
		{
			pr_info("%s: Core Normal mode regulator_get failed\n",
				__func__);
			ret = PTR_ERR(regl);
			goto error;
		}
		regulator_set_voltage(regl, type_info->nm_voltage,
				      type_info->nm_voltage);
		regulator_put(regl);
	}

	/* It is assumed that, at boot up system is in turbo mode */
	if (pdata->core_turbo_regl && type_info->nm2_turbo_voltage != -1)
	{
		regl = regulator_get(NULL, pdata->core_turbo_regl);
		if(IS_ERR(regl))
		{
			pr_info("%s: Core Turbo mode regulator_get failed\n",
				__func__);
			ret = PTR_ERR(regl);
			goto error;
		}
		regulator_set_voltage(regl, type_info->nm2_turbo_voltage,
				      type_info->nm2_turbo_voltage);
		regulator_put(regl);
	}

	if (pdata->notify_silicon_type)
		pdata->notify_silicon_type(silicon_type);
error:
	return ret;
}

static int __devexit bcm_avs_drv_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver bcm_avs_drv =
{
	.probe  =  bcm_avs_drv_probe,
	.remove = __devexit_p(bcm_avs_drv_remove),
	.driver = { .name =	 "bcm-avs-drv",},
};

static int __init bcm_avs_drv_init(void)
{
	return platform_driver_register(&bcm_avs_drv);
}

fs_initcall(bcm_avs_drv_init);
static void __exit bcm_avs_drv_exit(void)
{
	platform_driver_unregister(&bcm_avs_drv);
}

module_exit(bcm_avs_drv_exit);

MODULE_ALIAS("platform:bcm_avs_drv");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Adaptive Voltage Scaling driver for Broadcom Chipsets");

