/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/watchdog/bcm_watchdog.c
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
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <plat/reg_wdt.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <mach/irqs.h>
#include <plat/bcm_watchdog.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/platform_device.h>

#define PFX "watchdog: "

#define OSCR_FREQ		1
#define WATCHDOG_TIMEOUT 10	/* 60 sec default timeout */

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout,
		 "Watchdog cannot be stopped once started (default=CONFIG_WATCHDOG_NOWAYOUT)");

static int boot_status;

struct wdt_info {
	int locked;
	struct semaphore wdt_sem;
	int id;
	unsigned int d_tout;
	unsigned int inten;
	unsigned int *membase;
	int irq;
};

static struct wdt_info *wdt_access;

static void watchdog_start(void);
static void watchdog_stop(void);
static void watchdog_ping(void);

static void watchdog_start()
{
	unsigned int x;
	u16 val;
	x = (wdt_access->inten > 0) ? REG_WDT_CR_INTEN : REG_WDT_CR_SRSTEN;
	val =
	    REG_WDT_CR_EN | REG_WDT_CR_DSLPCEN | REG_WDT_CR_CLKS_1HZ | x |
	    wdt_access->d_tout;
	writel(val, wdt_access->membase);
}

static void watchdog_stop()
{
	u16 val;
	val = readl(wdt_access->membase);
	val &= ~REG_WDT_CR_EN;
	writel(val, wdt_access->membase);
}

/* Per spec, writing to LD field reloads the count
 *  This is all we need to do to PING/KICK the watchdog*/
static void watchdog_ping()
{
	u16 val;
	val = readl(wdt_access->membase);
	val |= (wdt_access->d_tout & 0xFF);
	writel(val, wdt_access->membase);
}

/* KERNEL EXPORT APIs*/

/* The first free watchdog is allocated.
 * Both user and kernel can therefore get any watchdog
 * depending on the order of access
 * */
int wdt_kopen(int inten)
{
	int ret = 0;
	if (down_interruptible(&wdt_access->wdt_sem))
		return -EINTR;
	if (wdt_access->locked) {
		ret = -EBUSY;
	} else {
		wdt_access->locked = 1;
		wdt_access->inten = inten;
	}
	up(&wdt_access->wdt_sem);
	if (nowayout)
		__module_get(THIS_MODULE);
	/* Activate timer */
	return ret;
}

EXPORT_SYMBOL(wdt_kopen);

int wdt_krelease()
{
	if (down_interruptible(&wdt_access->wdt_sem))
		return -EINTR;
	if (!nowayout)
		watchdog_stop();	/* Turn the WDT off */
	wdt_access->locked = 0;
	up(&wdt_access->wdt_sem);
	return 0;
}

EXPORT_SYMBOL(wdt_krelease);

int wdt_kwrite(size_t len)
{
	if (wdt_access->locked == 0)
		return -EINVAL;
	if (len > 0)
		watchdog_ping();
	return len;
}

EXPORT_SYMBOL(wdt_kwrite);

int wdt_kstart()
{
	if (wdt_access->locked == 0)
		return -EINVAL;
	watchdog_start();
	return 0;
}

EXPORT_SYMBOL(wdt_kstart);

int wdt_ksettimeout(int secs)
{
	unsigned int t = 0;
	t = OSCR_FREQ * secs;
	wdt_access->d_tout = (t & 0xFF);
	return 0;
}

EXPORT_SYMBOL(wdt_ksettimeout);

/*
 *	Allow only one person to hold it open
 */
static int watchdog_open(struct inode *inode, struct file *file)
{
	int ret = wdt_kopen(0);
	if (ret < 0)
		return ret;
	else {
		return nonseekable_open(inode, file);
	}
}

static int watchdog_release(struct inode *inode, struct file *file)
{
	/* Shut off the timer.
	 * Lock it in if it's a module and we defined ...NOWAYOUT */
	wdt_krelease();
	return 0;
}

static ssize_t watchdog_write(struct file *file, const char *data, size_t len,
			      loff_t *ppos)
{
	/* Refresh the timer. */
	return wdt_kwrite(len);
}

static struct watchdog_info ident = {
	.options = WDIOF_CARDRESET | WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING |
	    WDIOF_MAGICCLOSE,
	.firmware_version = 0,
	.identity = "Hardware Watchdog",
};
static int watchdog_ioctl(struct file *file,
			  unsigned int cmd, unsigned long arg)
{
	int ret = -ENOIOCTLCMD;
	int time, options;
	void __user *argp = (void __user *)arg;
	int __user *p = argp;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		ret = copy_to_user(argp, &ident, sizeof(ident)) ? -EFAULT : 0;
		break;

	case WDIOC_GETSTATUS:
		ret = put_user(0, p);
		break;

	case WDIOC_GETBOOTSTATUS:
		ret = put_user(boot_status, p);
		break;

	case WDIOC_SETOPTIONS:
		pr_info("WDIOC setopts ****\n");
		ret = get_user(options, p);
		if (ret)
			break;

		if (options & WDIOS_DISABLECARD) {
			watchdog_stop();
			ret = 0;
		}

		if (options & WDIOS_ENABLECARD) {
			pr_info("WDIOC setopts Start WDOG****\n");
			watchdog_start();
			ret = 0;
		}
		break;

	case WDIOC_SETTIMEOUT:
		ret = get_user(time, p);
		if (ret)
			break;

		if (time <= 0 || time > 255) {
			ret = -EINVAL;
			break;
		}
		ret = wdt_ksettimeout(time);
		break;
		/*fall through */
	case WDIOC_GETTIMEOUT:
		ret = put_user(wdt_access->d_tout / OSCR_FREQ, p);
		break;

	case WDIOC_KEEPALIVE:
		watchdog_ping();
		ret = 0;
		break;
	default:
		break;
	}
	return ret;
}

/*
 *	Notifier for system down
 */

static int watchdog_notify_sys(struct notifier_block *this, unsigned long code,
			       void *unused)
{
	if (code == SYS_DOWN || code == SYS_HALT) {
		watchdog_stop();	/* Turn the WDT off */
	}

	return NOTIFY_DONE;
}

static struct file_operations watchdog_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.write = watchdog_write,
	.unlocked_ioctl = watchdog_ioctl,
	.open = watchdog_open,
	.release = watchdog_release,
};

static struct miscdevice watchdog_miscdev = {
	.minor = WATCHDOG_MINOR,
	.name = "watchdog",
	.fops = &watchdog_fops,
};

static struct notifier_block watchdog_notifier = {
	.notifier_call = watchdog_notify_sys,
};

static char banner[] __initdata = KERN_INFO PFX "BROADCOM  WDT driver\n";

static irqreturn_t bcm_wdt_isr(int irq, void *p)
{
	return IRQ_HANDLED;
}
static int __init watchdog_probe(struct platform_device *pdev)
{
	int ret;
	int fail = 0;
	u32 irq;
	struct resource *res;

	/* We do not have a way of knowing if the watchdog reset occured or if it was a normal power-up */
	boot_status = 0;
	wdt_access = NULL;
	ret = register_reboot_notifier(&watchdog_notifier);
	if (ret) {
		pr_err(PFX "cannot register reboot notifier (err=%d)\n", ret);
		return ret;
	}
	/* Register WD 0 */
	ret = misc_register(&watchdog_miscdev);
	if (ret) {
		pr_err(PFX
		       "cannot register miscdev on minor=%d (err=%d)\n",
		       WATCHDOG_MINOR, ret);
		fail++;
	} else {
		wdt_access = kmalloc(sizeof(struct wdt_info), GFP_KERNEL);
		if (wdt_access == NULL) {
			misc_deregister(&watchdog_miscdev);
			fail++;
		} else {
			sema_init(&wdt_access->wdt_sem, 1);
			wdt_access->locked = 0;
			wdt_access->d_tout = OSCR_FREQ * WATCHDOG_TIMEOUT;
			wdt_access->inten = 0;
			irq = platform_get_irq(pdev, 0);
			ret = request_irq(irq, bcm_wdt_isr, IRQF_DISABLED,
					  "watchdog", NULL);
			wdt_access->irq = irq;
			res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
			if (res)
				wdt_access->membase =
				    (void __iomem *)res->start;
			else {
				pr_err("Failed to get memory map for WD IO\n");
				misc_deregister(&watchdog_miscdev);
				fail++;
				kfree(wdt_access);
				free_irq(irq, NULL);
			}
			pr_info("Watchdog memaddr is 0x%x\n",
				(unsigned int)wdt_access->membase);
		}
	}
	if (fail) {
		pr_err("Watchdog failed to register .. exiting\n");
		unregister_reboot_notifier(&watchdog_notifier);
	}
	pr_info("%s", banner);
	return 0;
}
static int watchdog_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver bcm_wdog_driver = {
	.probe = watchdog_probe,
	.remove = watchdog_remove,
	.driver = {
		   .name = "watchdog",
		   .owner = THIS_MODULE,
		   },
};

static int __init watchdog_init(void)
{
	return platform_driver_register(&bcm_wdog_driver);
}

static void __exit watchdog_exit(void)
{
	misc_deregister(&watchdog_miscdev);
	unregister_reboot_notifier(&watchdog_notifier);
	return platform_driver_unregister(&bcm_wdog_driver);
}

module_init(watchdog_init);
module_exit(watchdog_exit);

MODULE_AUTHOR("BROADCOM");
MODULE_DESCRIPTION("BROADCOM BCM1161 Watchdog");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
