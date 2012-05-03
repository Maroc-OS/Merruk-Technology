/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/input/misc/bma150.c
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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/bma150.h>
#include <linux/delay.h>
#include <linux/slab.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include "smb380.h"
#include "smb380calib.h"

/* uncomment to enable interrupt based event generation */
/* #define BMA150_ACCL_IRQ_MODE */
#define ACCL_NAME			"bma150_accl"
#define ACCL_VENDORID			0x0001

#define LOW_G_THRES			5
#define LOW_G_DUR			50
#define HIGH_G_THRES			10
#define HIGH_G_DUR			1
#define ANY_MOTION_THRES		1
#define ANY_MOTION_CT			1

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("bma150");

struct drv_data {
	struct input_dev *ip_dev;
	struct i2c_client *i2c;
	smb380_t smb380;
	int irq;
	int bma150_accl_mode;
	char bits_per_transfer;
	struct delayed_work work_data;
	bool config;
	struct list_head next_dd;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend suspend_desc;
#endif
};

static struct mutex bma150_accl_dd_lock;
static struct mutex bma150_accl_wrk_lock;
static struct list_head dd_list;
#ifndef BMA150_ACCL_IRQ_MODE
static struct timer_list bma_wakeup_timer;
#endif
static atomic_t bma_on;
static atomic_t a_flag;
static smb380acc_t newacc;

/* cannot avoid global pointer as the smb380 read/ write callback does not
 * absract interface level information
 */
static struct i2c_client *bma150_accl_client;

enum bma150_accl_modes {
	SENSOR_DELAY_FASTEST = 0,
	SENSOR_DELAY_GAME = 20,
	SENSOR_DELAY_UI = 60,
	SENSOR_DELAY_NORMAL = 200
};

static inline void bma150_accl_i2c_delay(unsigned int msec)
{
	mdelay(msec);
}

/*      i2c write routine for bma150    */
static inline char bma150_accl_i2c_write(unsigned char reg_addr,
					 unsigned char *data, unsigned char len)
{
	s32 dummy;
	if (bma150_accl_client == NULL)	/*      No global client pointer?       */
		return -1;

	while (len--) {
		dummy =
		    i2c_smbus_write_byte_data(bma150_accl_client, reg_addr,
					      *data);
		reg_addr++;
		data++;
		if (dummy < 0)
			return -1;
	}
	return 0;
}

/*      i2c read routine for bma150     */
static inline char bma150_accl_i2c_read(unsigned char reg_addr,
					unsigned char *data, unsigned char len)
{
	s32 dummy;
	if (bma150_accl_client == NULL)	/*      No global client pointer?       */
		return -1;

	while (len--) {
		dummy = i2c_smbus_read_byte_data(bma150_accl_client, reg_addr);
		if (dummy < 0)
			return -1;
		*data = dummy & 0x000000ff;
		reg_addr++;
		data++;
	}
	return 0;
}

static int bma150_accl_power_down(struct drv_data *dd)
{
	u8 data;
	int rc;

	/* smb380 does not provide functionality to power down the device */
	/* For sleep mode set bit 0 of control register (0x0A) to 1 */
	data = 0x01;
	rc = i2c_smbus_write_byte_data(dd->i2c, 0x0A, data);
	if (rc < 0)
		pr_err("G-Sensor power down failed\n");
	else
		atomic_set(&bma_on, 0);

	return rc;
}

static int bma150_accl_power_up(struct drv_data *dd)
{
	u8 data;
	int rc;

	/* smb380 does not provide functionality to power up the device */
	/* To exit sleep mode set bit 0 of control register (0x0A) to 0 */
	data = 0x00;
	rc = i2c_smbus_write_byte_data(dd->i2c, 0x0A, data);
	if (rc < 0)
		pr_err("G-Sensor power up failed\n");
	else
		atomic_set(&bma_on, 1);

	return rc;
}

#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
static int bma150_accl_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct drv_data *dd;
	dd = i2c_get_clientdata(client);

#ifdef BMA150_ACCL_IRQ_MODE
	disable_irq(dd->irq);
#else
	del_timer(&bma_wakeup_timer);
	cancel_delayed_work_sync(&dd->work_data);
#endif
	if (atomic_read(&bma_on))
		bma150_accl_power_down(dd);

	return 0;
}

static int bma150_accl_resume(struct i2c_client *client)
{
	struct drv_data *dd;
	dd = i2c_get_clientdata(client);

#ifdef BMA150_ACCL_IRQ_MODE
	enable_irq(dd->irq);
	if (atomic_read(&a_flag))
		bma150_accl_power_up(dd);
#else
	if (atomic_read(&a_flag))
		mod_timer(&bma_wakeup_timer, jiffies + HZ / 1000);
#endif

	return 0;
}

#else
#define bma150_accl_suspend NULL
#define bma150_accl_resume NULL
#endif /* CONFIG_PM */

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bma150_accl_early_suspend(struct early_suspend *desc)
{
	struct drv_data *dd = container_of(desc, struct drv_data, suspend_desc);
	pm_message_t mesg = {
		.event = PM_EVENT_SUSPEND,
		};
	bma150_accl_suspend(dd->i2c, mesg);
}

static void bma150_accl_late_resume(struct early_suspend *desc)
{
	struct drv_data *dd = container_of(desc, struct drv_data, suspend_desc);
	bma150_accl_resume(dd->i2c);
}
#endif /* CONFIG_HAS_EARLYSUSPEND */

#ifdef BMA150_ACCL_IRQ_MODE
static irqreturn_t bma150_accl_irq(int irq, void *dev_id)
{
	struct device *dev = dev_id;
	struct drv_data *dd;
	long delay;

	dd = dev_get_drvdata(dev);
	delay = dd->bma150_accl_mode * HZ / 1000;
	if (!delay)		/* check for FAST MODE */
		delay = 1;
	schedule_delayed_work(&dd->work_data, delay);

	return IRQ_HANDLED;
}
#else
void bma_wakeup_timer_func(unsigned long data)
{
	struct drv_data *dd;
	long delay = 0;

	dd = (struct drv_data *)data;

	delay = dd->bma150_accl_mode * HZ / 1000;
	/* Set delay >= 2 jiffies to avoid cpu hogging */
	if (delay < 2)
		delay = 2;
	schedule_delayed_work(&dd->work_data, HZ / 1000);
	if (atomic_read(&a_flag))
		mod_timer(&bma_wakeup_timer, jiffies + delay);
}
#endif

static int bma150_accl_open(struct input_dev *dev)
{
	int rc = 0;
#ifdef BMA150_ACCL_IRQ_MODE
	struct drv_data *dd = input_get_drvdata(dev);

	if (!dd->irq)
		return -1;
	/* Timer based implementation */
	/* does not require irq t be enabled */
	rc = request_irq(dd->irq,
			 &bma150_accl_irq, 0, ACCL_NAME, &dd->i2c->dev);
#endif

	return rc;
}

static void bma150_accl_release(struct input_dev *dev)
{
#ifdef BMA150_ACCL_IRQ_MODE
	struct drv_data *dd = input_get_drvdata(dev);

	/* Timer based implementation */
	/* does not require irq t be enabled */
	free_irq(dd->irq, &dd->i2c->dev);
#endif

	return;
}

static void bma150_accl_getdata(struct drv_data *dd)
{
	smb380acc_t acc;
	int X, Y, Z;
	struct bma150_accl_platform_data *pdata = pdata =
	    bma150_accl_client->dev.platform_data;

	mutex_lock(&bma150_accl_wrk_lock);
#ifndef BMA150_ACCL_IRQ_MODE
	if (!atomic_read(&bma_on)) {
		bma150_accl_power_up(dd);
		/* BMA150 need 2 to 3 ms delay */
		/* to give valid data after wakeup */
		msleep(2);
	}
#endif
	smb380_read_accel_xyz(&acc);

	switch (pdata->orientation) {
	case BMA_ROT_90:
		X = -acc.y;
		Y = acc.x;
		Z = acc.z;
		break;
	case BMA_ROT_180:
		X = -acc.x;
		Y = -acc.y;
		Z = acc.z;
		break;
	case BMA_ROT_270:
		X = acc.y;
		Y = -acc.x;
		Z = acc.z;
		break;
	default:
		pr_err("bma150_accl: invalid orientation specified\n");
	case BMA_NO_ROT:
		X = acc.x;
		Y = acc.y;
		Z = acc.z;
		break;
	}
	if (pdata->invert) {
		X = -X;
		Z = -Z;
	}
	input_report_rel(dd->ip_dev, REL_X, X);
	input_report_rel(dd->ip_dev, REL_Y, Y);
	input_report_rel(dd->ip_dev, REL_Z, Z);
	input_sync(dd->ip_dev);

	newacc.x = X;
	newacc.y = Y;
	newacc.z = Z;

#ifndef BMA150_ACCL_IRQ_MODE
	if (dd->bma150_accl_mode >= SENSOR_DELAY_UI)
		bma150_accl_power_down(dd);
#endif
	mutex_unlock(&bma150_accl_wrk_lock);

	return;

}

static void bma150_accl_work_f(struct work_struct *work)
{
	struct delayed_work *dwork =
	    container_of(work, struct delayed_work, work);
	struct drv_data *dd = container_of(dwork, struct drv_data, work_data);

	bma150_accl_getdata(dd);

}

#ifdef BMA150_ACCL_IRQ_MODE
static int __devinit bma150_accl_configuration(void)
{
	int ret;

/* low-g interrupt config: 0.156g, 150ms */
	ret = smb380_set_low_g_threshold((unsigned char)LOW_G_THRES);
	if (ret < 0)
		return ret;
	ret = smb380_set_low_g_duration((unsigned char)LOW_G_DUR);
	if (ret < 0)
		return ret;

/* high-g interrupt config: 1.50g, 150ms */
	ret = smb380_set_high_g_threshold((unsigned char)HIGH_G_THRES);
	if (ret < 0)
		return ret;
	ret = smb380_set_high_g_duration((unsigned char)HIGH_G_DUR);
	if (ret < 0)
		return ret;

/* any motion interrupt config: 0.75g, 3 */
	ret = smb380_set_any_motion_threshold((unsigned char)ANY_MOTION_THRES);
	if (ret < 0)
		return ret;
	ret = smb380_set_any_motion_count((unsigned char)ANY_MOTION_CT);
	if (ret < 0)
		return ret;

	return 0;

}

static int bma150_accl_set_mode(void)
{
	int ret = 0;
	struct drv_data *dd;
	dd = i2c_get_clientdata(bma150_accl_client);

	switch (dd->bma150_accl_mode) {
	case SENSOR_DELAY_FASTEST:
		/* BMA150 mode set to FASTEST (max sensitivity) */
	case SENSOR_DELAY_GAME:
		/* BMA150 mode set to GAME */
		ret = smb380_set_any_motion_count(1);
		if (ret < 0)
			return ret;
		ret = smb380_set_bandwidth(5);
		if (ret < 0)
			return ret;
		break;

	case SENSOR_DELAY_NORMAL:
		/* BMA150 mode set to NORMAL */
	case SENSOR_DELAY_UI:
		/* BMA150 mode set to UI */
	default:
		ret = smb380_set_any_motion_count(2);
		if (ret < 0)
			return ret;
		ret = smb380_set_bandwidth(3);
		if (ret < 0)
			return ret;
		break;
	}

	return 0;
}
#endif

static int bma150_accl_misc_open(struct inode *inode, struct file *file)
{
	int err;
	err = nonseekable_open(inode, file);
	if (err < 0)
		return err;

	file->private_data = inode->i_private;

	return 0;
}

static int bma150_accl_misc_ioctl(struct inode *inode, struct file *file,
				  unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int delay;
	struct drv_data *dd;
	dd = i2c_get_clientdata(bma150_accl_client);

	switch (cmd) {
	case BMA150_ACCL_IOCTL_GET_DELAY:
		delay = dd->bma150_accl_mode;

		if (copy_to_user(argp, &delay, sizeof(delay)))
			return -EFAULT;
		break;

	case BMA150_ACCL_IOCTL_SET_DELAY:
		if (copy_from_user(&delay, argp, sizeof(delay)))
			return -EFAULT;
		if (delay < 0 || delay > 200)
			return -EINVAL;
		dd->bma150_accl_mode = delay;
#ifdef BMA150_ACCL_IRQ_MODE
		bma150_accl_set_mode();
#endif
		break;
	case BMA150_ACCL_IOCTL_SET_FLAG:
		if (copy_from_user(&delay, argp, sizeof(delay)))
			return -EFAULT;
#ifdef BMA150_ACCL_IRQ_MODE
		if (delay == 0)
			bma150_accl_power_down(dd);
		else if (delay == 1)
			bma150_accl_power_up(dd);
#else
		if (delay == 1)
			mod_timer(&bma_wakeup_timer, jiffies + HZ / 1000);
		else if (delay == 0)
			del_timer(&bma_wakeup_timer);
#endif
		else
			return -EINVAL;
		atomic_set(&a_flag, delay);
		break;
	case BMA150_ACCL_IOCTL_GET_DATA:
		if (!atomic_read(&a_flag))
			bma150_accl_getdata(dd);
		if (copy_to_user(argp, &newacc, sizeof(newacc)))
			return -EFAULT;
		break;
	}

	return 0;
}

static const struct file_operations bma150_accl_misc_fops = {
	.owner = THIS_MODULE,
	.open = bma150_accl_misc_open,
	.ioctl = bma150_accl_misc_ioctl,
};

static struct miscdevice bma150_accl_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bma150_accl",
	.fops = &bma150_accl_misc_fops,
};

static int __devinit bma150_accl_probe(struct i2c_client *client,
				       const struct i2c_device_id *id)
{
	struct drv_data *dd;
	int rc = 0;
	struct bma150_accl_platform_data *pdata = pdata =
	    client->dev.platform_data;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("i2c_check_functionality error\n");
		goto probe_exit;
	}
	dd = kzalloc(sizeof(struct drv_data), GFP_KERNEL);
	if (!dd) {
		rc = -ENOMEM;
		goto probe_exit;
	}
	bma150_accl_client = client;

	mutex_lock(&bma150_accl_dd_lock);
	list_add_tail(&dd->next_dd, &dd_list);
	mutex_unlock(&bma150_accl_dd_lock);
	INIT_DELAYED_WORK(&dd->work_data, bma150_accl_work_f);
	dd->i2c = client;

	if (pdata && pdata->init) {
		rc = pdata->init(&client->dev);
		if (rc)
			goto probe_err_cfg;
	}

	dd->ip_dev = input_allocate_device();
	if (!dd->ip_dev) {
		rc = -ENOMEM;
		goto probe_err_reg;
	}
	input_set_drvdata(dd->ip_dev, dd);
	dd->irq = client->irq;
	dd->ip_dev->open = bma150_accl_open;
	dd->ip_dev->close = bma150_accl_release;
	dd->ip_dev->name = ACCL_NAME;
	dd->ip_dev->phys = ACCL_NAME;
	dd->ip_dev->id.vendor = ACCL_VENDORID;
	dd->ip_dev->id.product = 1;
	dd->ip_dev->id.version = 1;
	set_bit(EV_REL, dd->ip_dev->evbit);
	set_bit(REL_X, dd->ip_dev->relbit);
	set_bit(REL_Y, dd->ip_dev->relbit);
	set_bit(REL_Z, dd->ip_dev->relbit);
	rc = input_register_device(dd->ip_dev);
	if (rc) {
		dev_err(&dd->ip_dev->dev,
			"bma150_accl_probe: input_register_device rc=%d\n", rc);
		goto probe_err_reg_dev;
	}
	rc = misc_register(&bma150_accl_misc_device);
	if (rc < 0) {
		dev_err(&client->dev, "bma150 misc_device register failed\n");
		goto probe_err_reg_misc;
	}
	/* bma150 sensor initial */
	dd->smb380.bus_write = bma150_accl_i2c_write;
	dd->smb380.bus_read = bma150_accl_i2c_read;
	dd->smb380.delay_msec = bma150_accl_i2c_delay;
	rc = smb380_init(&dd->smb380);
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Error configuring device rc=%d\n", rc);
		goto probe_err_smbcfg;
	}
	/* verify chip id */
	if ((dd->smb380).chip_id != 2) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Device not found \n");
		goto probe_err_smbcfg;
	}
	rc = smb380_set_range(0);	/* range + /-2G */
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Error configuring device rc=%d\n", rc);
		goto probe_err_smbcfg;
	}
	/* close all interrupt */
	rc = smb380_set_low_g_int(0);
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Error configuring device rc=%d\n", rc);
		goto probe_err_smbcfg;
	}
	rc = smb380_set_high_g_int(0);
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Error configuring device rc=%d\n", rc);
		goto probe_err_smbcfg;
	}
#ifdef BMA150_ACCL_IRQ_MODE
	rc = smb380_set_any_motion_int(1);
#else
	rc = smb380_set_any_motion_int(0);
#endif
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Error configuring device rc=%d\n", rc);
		goto probe_err_smbcfg;
	}
	rc = smb380_set_alert_int(0);
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Error configuring device rc=%d\n", rc);
		goto probe_err_smbcfg;
	}
#ifdef BMA150_ACCL_IRQ_MODE
	rc = smb380_set_advanced_int(1);
#else
	rc = smb380_set_advanced_int(0);
#endif
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Error configuring device rc=%d\n", rc);
		goto probe_err_smbcfg;
	}
	rc = smb380_reset_interrupt();
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Error configuring device rc=%d\n", rc);
		goto probe_err_smbcfg;
	}
#ifndef BMA150_ACCL_IRQ_MODE
	rc = smb380_set_new_data_int(1);
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe : \
				Error configuring device rc\n");
		goto probe_err_smbcfg;
	}
#endif

	dd->bma150_accl_mode = 200;	/* NORMAL Mode */
	i2c_set_clientdata(client, dd);
#ifdef BMA150_ACCL_IRQ_MODE
	rc = bma150_accl_configuration();
	rc = bma150_accl_set_mode();
	if (rc < 0) {
		dev_err(&dd->ip_dev->dev, "bma150_accl_probe: \
				Error configuring device rc=%d\n", rc);
		goto probe_err_setmode;
	}
#else
	setup_timer(&bma_wakeup_timer, bma_wakeup_timer_func, (long)dd);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	dd->suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
	dd->suspend_desc.suspend = bma150_accl_early_suspend,
	dd->suspend_desc.resume = bma150_accl_late_resume,
	register_early_suspend(&dd->suspend_desc);
#endif

	return rc;
#ifdef BMA150_ACCL_IRQ_MODE
probe_err_setmode:
	i2c_set_clientdata(client, NULL);
#endif
probe_err_smbcfg:
	misc_deregister(&bma150_accl_misc_device);
probe_err_reg_misc:
	input_unregister_device(dd->ip_dev);
probe_err_reg_dev:
	input_free_device(dd->ip_dev);
	dd->ip_dev = NULL;
probe_err_reg:
	if (pdata && pdata->exit)
		pdata->exit(&client->dev);
probe_err_cfg:
	mutex_lock(&bma150_accl_dd_lock);
	list_del(&dd->next_dd);
	mutex_unlock(&bma150_accl_dd_lock);
	kfree(dd);
probe_exit:
	return rc;
}

static int __devexit bma150_accl_remove(struct i2c_client *client)
{
	struct drv_data *dd;
	struct bma150_accl_platform_data *pdata = pdata =
	    client->dev.platform_data;
	int rc;
	const char *devname;

	dd = i2c_get_clientdata(client);
	devname = dd->ip_dev->phys;

	rc = bma150_accl_power_down(dd);
	if (rc)
		dev_err(&dd->ip_dev->dev,
			"%s: power down failed with error %d\n", __func__, rc);
#ifdef BMA150_ACCL_IRQ_MODE
	free_irq(dd->irq, &dd->i2c->dev);
#else
	del_timer(&bma_wakeup_timer);
#endif
	misc_deregister(&bma150_accl_misc_device);
	input_unregister_device(dd->ip_dev);
	i2c_set_clientdata(client, NULL);
	if (pdata && pdata->exit)
		pdata->exit(&client->dev);
	mutex_lock(&bma150_accl_dd_lock);
	list_del(&dd->next_dd);
	mutex_unlock(&bma150_accl_dd_lock);
	kfree(devname);
	kfree(dd);

	return 0;
}

static struct i2c_device_id bma150_accl_idtable[] = {
	{"bma150_accl", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bma150_accl_idtable);

static struct i2c_driver bma150_accl_driver = {
	.driver = {
		   .name = ACCL_NAME,
		   .owner = THIS_MODULE,
		   },
	.id_table = bma150_accl_idtable,
	.probe = bma150_accl_probe,
	.remove = __devexit_p(bma150_accl_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = bma150_accl_suspend,
	.resume = bma150_accl_resume,
#endif
};

static int __init bma150_accl_init(void)
{
	INIT_LIST_HEAD(&dd_list);
	mutex_init(&bma150_accl_dd_lock);
	mutex_init(&bma150_accl_wrk_lock);

	return i2c_add_driver(&bma150_accl_driver);
}

module_init(bma150_accl_init);

static void __exit bma150_accl_exit(void)
{
	i2c_del_driver(&bma150_accl_driver);
}

module_exit(bma150_accl_exit);
