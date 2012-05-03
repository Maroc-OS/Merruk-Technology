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
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/suspend.h>
#include <linux/brcm_console.h>
#include "../../../arch/arm/include/asm/memory.h"

struct proc_dir_entry *pentry_brcm_usb_test;

struct delayed_work g_delay_workq, g_delay_workq_usbcalled;

#define TST_SZ 1023
char s1[256] = "aaaaaaaaaabbbbbbbbbbbbbbbccccccccccccccddddddddddddddddddeeeeeeeeeeeeeeeeeeeee";
char s2[256] = "1111111111222222222222333333333333333333444444444444444444455555555555555555666666666666666666666777777777777778888888888888888888899999999";
char test1[TST_SZ];
unsigned char print_it;

extern int brcm_klogging(char *data, int length);
extern char brcm_netconsole_register_callbacks(struct brcm_netconsole_callbacks *_cb);

static int netconsole_start_cb (void)
{
	pr_info("%s\n",__func__);
	return 0;
}

static int netconsole_stop_cb(void)
{
	pr_info("%s\n",__func__);
	return 0;
}

static struct brcm_netconsole_callbacks brcm_netcon_callbacks = {
        .start = netconsole_start_cb,
        .stop = netconsole_stop_cb,        
};

static void brcm_usb_test_logging(void)
{
      static int i = 0;
      
       for (i=0;i<TST_SZ;i++)
                test1[i] = i;

      /*  brcm_klogging(test1, TST_SZ);*/
	brcm_klogging(s1, strlen(s1));
	brcm_klogging(s2, strlen(s2));
}

static void brcm_usb_test_klogging_task(void* data)
{
   while (print_it) {
	brcm_usb_test_logging();
	msleep(10);
   }
}

static int brcm_usb_test_read(struct inode *inode, struct file *file)
{

      pr_info("start brcm_printk test... \n");
      print_it = 1;
       /*create thread to test the klogging*/
	kernel_thread(brcm_usb_test_klogging_task, 0, 0);
      return 0;
}

static int brcm_usb_test_write(struct inode *inode, struct file *file)
{
	print_it = 0;

	pr_info("brcm_usb_test_write....\n");
	brcm_netconsole_register_callbacks(&brcm_netcon_callbacks);

	return 0;
}

static int brcm_usb_test_open(struct inode *inode, struct file *file)
{	
	return 0;
}

static struct file_operations brcm_usb_test_ops = {
  .owner =  THIS_MODULE,
  .read = brcm_usb_test_read,
  .write = brcm_usb_test_write,
  .open = brcm_usb_test_open,
};

static void brcm_usb_test_exit(void);

static int brcm_usb_test_init(void)
{
	pr_alert("printk: brcm usb test init\n");
	pentry_brcm_usb_test = proc_create("brcm_usb_test", 0666, NULL, &brcm_usb_test_ops);	
	
	if (!pentry_brcm_usb_test)
		return -EPERM;
	return 0;
}

static void brcm_usb_test_exit(void)
{
    pr_alert("printk: brcm usb test exit\n");
    remove_proc_entry("brcm_usb_test", NULL);
}

module_init(brcm_usb_test_init)
module_exit(brcm_usb_test_exit)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Hwang");
MODULE_DESCRIPTION("BRCM USB TEST");
