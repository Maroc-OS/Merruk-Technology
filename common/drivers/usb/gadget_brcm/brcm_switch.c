/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use
of this software, this software is licensed to you under the terms of the GNU General Public
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL").
  
Notwithstanding the above, under no circumstances may you combine this software in any way
with any other Broadcom software provided under a license other than the GPL, without
Broadcom's express prior written consent.
*******************************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/suspend.h>
#include <linux/kthread.h>
#include "../../../arch/arm/include/asm/memory.h"
#include "android.h"

enum {
	MSC_ONLY_USB_CONF = 1,
	ADB_MSC_USB_CONF,
	RNDIS_ETHER_USB_CONF,
	RESTORE_ADB_MODE,
	CHECK_USB_MODE,
	ACM_ONLY_MODE,
	ACM_OBEX_MODE,
	LOOPBACK_TEST_MODE	/* looback test = 8 */	
};

struct proc_dir_entry *pentry_brcm_switch;
unsigned char g_check_usb_mode = 0;

static int my_atoi(const char *name)
{
    int val = 0;

    for (;; name++) {
        switch (*name) {
            case '0' ... '9':
                val = 10*val+(*name-'0');
                break;
            default:
                return val;
        }
    }
}

//Test the APIs
static void brcm_switch_msc_only(void* data)
{
#ifdef CONFIG_USB_ANDROID
	Android_switch_usb_conf(MSC_ONLY_USB_CONF - 1);
#endif
}

static void brcm_switch_adb_msc(void* data)
{		
#ifdef CONFIG_USB_ANDROID
	Android_switch_usb_conf(ADB_MSC_USB_CONF - 1);				
#endif
}

static void brcm_switch_eth(void* data)
{
#ifdef CONFIG_USB_ANDROID
	Android_switch_usb_conf(RNDIS_ETHER_USB_CONF - 1);
#endif
}

static void brcm_switch_restore_adb(void* data)
{
#ifdef CONFIG_USB_ANDROID
	Android_switch_usb_conf(RESTORE_ADB_MODE - 1);
#endif
}

static void brcm_switch_acm_only(void* data)
{
#ifdef CONFIG_USB_ANDROID
	Android_switch_usb_conf(ACM_ONLY_MODE);
#endif
}

static void brcm_switch_acm_obex(void* data)
{		
#ifdef CONFIG_USB_ANDROID
	Android_switch_usb_conf(ACM_OBEX_MODE);				
#endif
}

extern void usb_loopback_test(u8 loopbk);
static void brcm_switch_toggle_test(void *data)
{
	static unsigned char loopbk_mode = 1;
#ifdef CONFIG_USB_ANDROID
	usb_loopback_test(loopbk_mode);		
	loopbk_mode = !loopbk_mode;
#endif
}

static ssize_t brcm_switch_read(struct file *file, char *buf, int count, loff_t *ppos)
{
	int	val, len, err;

	if (g_check_usb_mode) {
		g_check_usb_mode = 0;
		pr_info("checking the current USB mod\n");
#ifdef CONFIG_USB_ANDROID
		val = Android_switch_usb_conf(CHECK_USB_MODE - 1) + 1;
#endif
		len = sizeof(val);
		pr_info("val = %d \n",val);
		err = copy_to_user(buf, &val, len);
		if (err != 0)
			return -EFAULT;
		return len;
	} else {
		pr_info("\nTest Modes:\n");
        	pr_info("MSC_ONLY_MODE: echo 1 > /proc/brcm_switch\n");
        	pr_info("ADB_MSC_MODE: echo 2 > /proc/brcm_switch\n");
        	pr_info("RNDIS_ETH_MODE: echo 3 > /proc/brcm_switch\n");
		pr_info("ACM_ONLY_MODE: echo 6 > /proc/brcm_switch\n");
        	pr_info("ACM_MSC_MODE: echo 7 > /proc/brcm_switch\n");
        	pr_info("LOOPBACK_TEST_MODE: echo 8 > /proc/brcm_switch\n");
       	return 0;
	}      
}



static ssize_t brcm_switch_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
        struct task_struct *brcm_switch_task = NULL;
        int thread_mode = my_atoi(buffer);

        switch (thread_mode) {
        case MSC_ONLY_USB_CONF:
			pr_info("\nMSC_ONLY_MODE\n");
			brcm_switch_task = kthread_run(brcm_switch_msc_only, 0, "msc_only");
			break;
        case ADB_MSC_USB_CONF:
			pr_info("\nADB_MSC_MODE\n");
			brcm_switch_task = kthread_run(brcm_switch_adb_msc, 0, "adb_msc");
			break;
        case RNDIS_ETHER_USB_CONF:
			pr_info("\nRNDIS_ETH_MODE\n");
			brcm_switch_task = kthread_run(brcm_switch_eth, 0, "rndis");
			break;
        case RESTORE_ADB_MODE:
			pr_info("\nRESTORE_ADB_MODE\n");
			brcm_switch_task = kthread_run(brcm_switch_restore_adb, 0, "restore_adb");
			break;
	case ACM_ONLY_MODE:
			pr_info("\ACM_ONLY_MODE\n");	
			brcm_switch_task = kthread_run(brcm_switch_acm_only, 0, "acm_only");			
			break;
	case ACM_OBEX_MODE:
			pr_info("\ACM_OBEX_MODE\n");		
			brcm_switch_task = kthread_run(brcm_switch_acm_obex, 0, "acm_msc");				
			break;	
	case LOOPBACK_TEST_MODE:
			pr_info("LOOPBACK_TEST_MODE\n");
			brcm_switch_task = kthread_run(brcm_switch_toggle_test, 0, "toggle_test");
			break;
        case CHECK_USB_MODE:
            g_check_usb_mode = 1;
            return 1;
        default:
        	pr_info("\nThe input mode (%d) is NOT supported!!!!\n", thread_mode);
            return 1;
        }

        if (IS_ERR(brcm_switch_task )) {
            pr_err("Failed to create brcm_switch_task -- ERROR=%x\n", PTR_ERR(brcm_switch_task ));
            brcm_switch_task =  NULL;
        }
       return 1;
}

static struct file_operations brcm_switch_ops = {
  .owner =  THIS_MODULE,
  .read = brcm_switch_read,
  .write = brcm_switch_write,
};

static void brcm_switch_exit(void);

static int brcm_switch_init(void)
{
    pr_alert("printk: brcm_switch_init\n");
    pr_alert("cat /proc/brcm_switch (To list the test modes and usage.....)");
    pentry_brcm_switch = proc_create("brcm_switch", 0666, NULL, &brcm_switch_ops);
      
   if (!pentry_brcm_switch)
              return -EPERM;
          return 0;
}

static void brcm_switch_exit(void)
{
    pr_alert("\nprintk: brcm_switch_exit\n");
    remove_proc_entry("brcm_switch", NULL);
}

module_init(brcm_switch_init)
module_exit(brcm_switch_exit)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Hwang");
MODULE_DESCRIPTION("BRCM SWITCH");

