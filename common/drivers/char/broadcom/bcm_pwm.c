/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/char/broadcom/bcm_pwm.c
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
#include <linux/input.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <asm/sizes.h>
#include <mach/hardware.h>
#include <mach/memory.h>

#include <mach/pwm.h>
#include <plat/pwm/pwm_core.h>
#include <plat/bcm_pwm_block.h>
#include <plat/syscfg.h>

#define BCM_PWM_MAX_CHANNELS                       6

#define PWM_MASTER_CLOCK_FREQ_HZ                   26000000

#define PWM_TOP_CONTROL                            0x00000000
#define PWM_TOP_PRESCALE_CONTROL_OFFSET            0x00000004
#define PWM_TOP_PWM_PERIOD_COUNT_OFFSET(id)        (0x00000008  + (id << 3))
#define PWM_TOP_PWM_DUTY_CYCLE_HIGH_OFFSET(id)     (0x0000000C + (id << 3))

#define PWM_TOP_PRESCALE_MASK(id)                  (0x00000007 << (id << 2))
#define PWM_BLOCK_ENABLE(id)                       (1 << id)

#define PWM_TOP_CONTROL_POLARITY_MASK              0x3F00
#define PWM_TOP_VIBRA_OFFSET                       0x0080
#define VIBRA_PWM5_PWM4                            0x4

#define PWM_CLK_PRESCALE_DIV_8                     0x7

static void __iomem *bcm_pwm_base;
static struct clk *pwm_clk;

struct bcm_pwm_device {
	struct pwm_desc desc;
	int clk_enabled;
	struct platform_device *pdev;
	struct pwm_channel *chan;
	int (*syscfg_inf) (uint32_t module, uint32_t op);
};

#define BCM_PWM_DEV_INIT(id, op)  \
{ \
	.desc = { \
		.pwm_id = id, \
		.ops = op, \
	}, \
}

static struct pwm_channel_ops pwm_chan_ops;

static struct bcm_pwm_device bcm_pwm_devices[] = {
	BCM_PWM_DEV_INIT(BCM_PWM_CHANNEL0, &pwm_chan_ops),
	BCM_PWM_DEV_INIT(BCM_PWM_CHANNEL1, &pwm_chan_ops),
	BCM_PWM_DEV_INIT(BCM_PWM_CHANNEL2, &pwm_chan_ops),
	BCM_PWM_DEV_INIT(BCM_PWM_CHANNEL3, &pwm_chan_ops),
	BCM_PWM_DEV_INIT(BCM_PWM_CHANNEL4, &pwm_chan_ops),
	BCM_PWM_DEV_INIT(BCM_PWM_CHANNEL5, &pwm_chan_ops),
};

/* *************************************************************************** */
/* Function Name:  bcm_pwm_config() */
/* Description:  Configure PWM output. Use id of PWM to configure */
/* Parameters:          pwm              (in)    PWM device instance */
/* Return:              zero on success */
/* *************************************************************************** */
static int bcm_pwm_config(struct pwm_channel *chan, int duty_ns, int period_ns)
{
	unsigned int reg_value, top_ctrlreg_save;
	unsigned long long c;
	unsigned long period_cycles, prescale, pv,dc;
	struct bcm_pwm_device *dev = chan->pwm_data;

	/*Make sure pwm is disabled before calling this function */

	if (period_ns == 0 || duty_ns > period_ns)
		return -EINVAL;

	c = PWM_MASTER_CLOCK_FREQ_HZ;
	c = c * period_ns;
	do_div(c, 1000000000);
	period_cycles = c;

	if (period_cycles < 2)	/*Min. value specified by hardware */
		period_cycles = 2;
	/* (max. period_ns) / 8 , due to a 3-bit precaler*/
	prescale = (period_cycles - 1) / (period_ns / 8);
	pv = period_cycles / (prescale + 1);

	if (prescale > PWM_CLK_PRESCALE_DIV_8) {
		prescale = PWM_CLK_PRESCALE_DIV_8;
	}
	c = (unsigned long long) pv *duty_ns;
	do_div(c, period_ns);
	dc = c;

	reg_value = readl(bcm_pwm_base + PWM_TOP_PRESCALE_CONTROL_OFFSET);
	reg_value &= ~(PWM_TOP_PRESCALE_MASK(dev->desc.pwm_id));
	reg_value |= (prescale << (4 * dev->desc.pwm_id));
	writel(reg_value, bcm_pwm_base + PWM_TOP_PRESCALE_CONTROL_OFFSET);

	writel(pv, bcm_pwm_base +
	       PWM_TOP_PWM_PERIOD_COUNT_OFFSET(dev->desc.pwm_id));
	writel(dc, bcm_pwm_base +
	       PWM_TOP_PWM_DUTY_CYCLE_HIGH_OFFSET(dev->desc.pwm_id));

	/* Save and Disable the pwm before configuring */
	top_ctrlreg_save = reg_value = readl(bcm_pwm_base + PWM_TOP_CONTROL);
	reg_value &= ~(1 << dev->desc.pwm_id);
	writel(reg_value, bcm_pwm_base + PWM_TOP_CONTROL);

	/* Restore pwm configration */
	writel(top_ctrlreg_save, (bcm_pwm_base + PWM_TOP_CONTROL));
	return 0;
}

/* *************************************************************************** */
/* Function Name:       bcm_pwm_request() */
/* Description:         Request PWM resource. */
/* Parameters:          chan                      (in)  PWM channel handlee */
/* Return:              None */
/* *************************************************************************** */
static int bcm_pwm_request(struct pwm_channel *chan)
{
	return 0;
}

/* *************************************************************************** */
/* Function Name:       bcm_pwm_free() */
/* Description:         Free PWM resource. */
/* Parameters:          PWM device Instance */
/* Return:              None */
/* *************************************************************************** */
static void bcm_pwm_free(struct pwm_channel *chan)
{
}

/* *************************************************************************** */
/* Function Name:  bcm_pwm_enable() */
/* Description:    Enable PWM output. Use id of PWM to enable */
/* Parameters:          id                      (in)    PWM output to configure */
/*                      pwm_device              (in)    PWM device instance */
/* *************************************************************************** */
static int bcm_pwm_enable(struct pwm_channel *chan)
{
	struct bcm_pwm_device *dev = chan->pwm_data;

	if (!dev->clk_enabled) {
		clk_enable(pwm_clk);
		dev->clk_enabled = 1;
	}

	writel(readl(bcm_pwm_base + PWM_TOP_CONTROL) |
	       PWM_BLOCK_ENABLE(dev->desc.pwm_id),
	       (bcm_pwm_base + PWM_TOP_CONTROL));

	if (dev->syscfg_inf)
		dev->syscfg_inf(SYSCFG_PWM0 + dev->desc.pwm_id, SYSCFG_ENABLE);

	return 0;
}

/* *************************************************************************** */
/* Function Name:       bcm_pwm_disable() */
/* Description:         Turn off a configured PWM output. */
/* Parameters:                   id      (in)    PWM output to configure */
/*                              pwm     (in)    PWM device instance */
/* Return:                      none */
/* *************************************************************************** */
static void bcm_pwm_disable(struct pwm_channel *chan)
{
	unsigned int reg_value = 0;
	struct bcm_pwm_device *dev = chan->pwm_data;

	if (dev->syscfg_inf)
		dev->syscfg_inf(SYSCFG_PWM0 + dev->desc.pwm_id, SYSCFG_DISABLE);

	reg_value = readl(bcm_pwm_base + PWM_TOP_CONTROL);
	reg_value &= ~(1 << dev->desc.pwm_id);
	writel(reg_value, bcm_pwm_base + PWM_TOP_CONTROL);

	if (dev->clk_enabled) {
		clk_disable(pwm_clk);
		dev->clk_enabled = 0;
	}
}

static struct pwm_channel_ops pwm_chan_ops = {
	.config = bcm_pwm_config,
	.request = bcm_pwm_request,
	.free = bcm_pwm_free,
	.enable = bcm_pwm_enable,
	.disable = bcm_pwm_disable,
};

/* *************************************************************************** */
/* Function Name:       bcm_pwm_probe() */
/* Description:         Probe function for PWM Driver */
/* Parameters:          pdev    (in)    Platform Device instance */
/* Return:              zero on success */
/* *************************************************************************** */
static int __devinit bcm_pwm_probe(struct platform_device *pdev)
{
	unsigned int reg_value, i = 0, ret = 0;
	struct resource *res;
	struct pwm_channel *chan;
	struct bcm_pwm_device *dev;
	struct pwm_platform_data *pdata = pdev->dev.platform_data;
	/* Register all the PWM channels */
	for (i = 0; i < BCM_PWM_MAX_CHANNELS; i++) {
		dev = &bcm_pwm_devices[i];

		chan = pwm_register(&dev->desc, &pdev->dev, dev);
		if (IS_ERR(chan)) {
			ret = PTR_ERR(chan);
			goto pwm_register_fail;
		}

		dev->clk_enabled = 0;
		dev->pdev = pdev;
		dev->chan = chan;
		dev->syscfg_inf = pdata->syscfg_inf;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		ret = -ENODEV;
		goto pwm_resource_fail;
	}

	pwm_clk = clk_get(NULL, "pwm");
	if (IS_ERR(pwm_clk)) {
		ret = PTR_ERR(pwm_clk);
		goto clk_get_fail;
	}

	bcm_pwm_base = (void *__iomem)res->start;

	/* Set all PWMs to "Active High" */
	reg_value = readl(bcm_pwm_base + PWM_TOP_CONTROL);
	reg_value |= PWM_TOP_CONTROL_POLARITY_MASK;
	writel(reg_value, bcm_pwm_base + PWM_TOP_CONTROL);
	writel(readl(bcm_pwm_base + PWM_TOP_VIBRA_OFFSET) |
	       VIBRA_PWM5_PWM4, bcm_pwm_base + PWM_TOP_VIBRA_OFFSET);

	platform_set_drvdata(pdev, bcm_pwm_devices);

	return ret;

clk_get_fail:
	release_mem_region(res->start, res->end - res->start + 1);

pwm_resource_fail:
	i = BCM_PWM_MAX_CHANNELS;

pwm_register_fail:
	while (--i >= 0) {
		dev = &bcm_pwm_devices[i];
		pwm_unregister(dev->chan);
	}

	return ret;
}

/* *************************************************************************** */
/* Function Name:       bcm_pwm_remove() */
/* Description:         Probe function for PWM Driver */
/* Parameters:          pdev        (in)      Platform Device instance */
/* Return:              zero on success */
/* *************************************************************************** */
static int __devexit bcm_pwm_remove(struct platform_device *pdev)
{
	int i = 0;
	struct resource *res;
	struct bcm_pwm_device *dev;

	dev = platform_get_drvdata(pdev);
	if (dev == NULL)
		return -ENODEV;

	for (i = 0; i < BCM_PWM_MAX_CHANNELS; i++) {
		if (pwm_unregister(dev->chan) != 0)
			return -EBUSY;

		dev->chan = NULL;
		dev++;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, res->end - res->start + 1);

	return 0;
}

struct platform_driver bcm_pwm_driver = {
	.probe = bcm_pwm_probe,
	.remove = __devexit_p(bcm_pwm_remove),
	.driver = {
		   .name = "bcm_pwm",
		   }
};

static int __init bcm_pwm_init(void)
{
	pr_info(" init :bcm_pwm_driver\n");
	return platform_driver_register(&bcm_pwm_driver);
}

static void __exit bcm_pwm_exit(void)
{
	platform_driver_unregister(&bcm_pwm_driver);
}

module_init(bcm_pwm_init);
module_exit(bcm_pwm_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("BCM PWM Driver");
MODULE_LICENSE("GPL");
