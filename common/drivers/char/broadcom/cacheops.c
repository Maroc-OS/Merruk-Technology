/*******************************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this software
 * in any way with any other Broadcom software provided under a license other than
 * the GPL, without Broadcom's express prior written consent.
 * *******************************************************************************************/

/****************************************************************************************************
*
*
*   @file   cacheops.c
*
*   @brief  This driver provides ioctl to flush and invalidate the cache for given range.
*                 
*
*
********************************************************************************************************/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/broadcom/cacheops.h>

static struct class *cache_class;
static spinlock_t cache_lock;
static int major=0;

/*cache file ops*/
static int cache_ioctl(struct inode *, struct file *, unsigned int,unsigned long);
static int cache_open(struct inode *, struct file *);
static int cache_close(struct inode *, struct file *);
static int cacheops(char*,unsigned int*,unsigned int*,unsigned int*);

/*
 * File operations struct, to use operations find the
 * correct file descriptor
 */
static struct file_operations cache_fops = {
	open : cache_open,
    	release: cache_close,
    	ioctl: cache_ioctl,
};

static int cache_open(struct inode *ino, struct file *f) 
{
	return 0;
}

static int cache_close(struct inode *ino, struct file *f) 
{
	return 0;
}


/*
 * cache_ioctl:
 *      a user space program can drive the test functions
 *      through a call to ioctl once the correct file
 *      descriptor has been attained
 */
static int cache_ioctl(struct inode *ino, struct file *f,
                        unsigned int cmd, unsigned long l)
{
	int 			rc;
	cache_interface_t	tif;
	rc = 0;

	/*
	 * the following calls are used to setup the
	 * parameters that might need to be passed
	 * between user and kernel space.
	 */

	if (copy_from_user(&tif, (void *)l, sizeof(tif)) )
	{
		/* Bad address */
		return(-EFAULT);
	}

	/*
	 * Use a switch statement to determine which function
	 * to call, based on the cmd flag that is specified
	 * in user space. Pass in inparms or outparms as
	 * needed
	 *
	 */
 	switch(cmd)
   	{
		
		case		CACHE_OPS: rc= cacheops(&tif.i_f,&tif.pstart,&tif.vstart,&tif.size); 
				break;
		default:
				pr_info("cache-ops: Mismatching ioctl command\n");
				return -EAGAIN;
    	}

	/*
	 * copy in the test return code
	 */

	tif.out_rc = rc;
	rc = 0;
	/*
	 * setup the rest of tif pointer for returning to
	 * to user space, using copy_to_user if needed
	 */
	
    	/* copy tif structure into l so that can be used by user program */
    	if(copy_to_user((void*)l, &tif, sizeof(tif)) )
    	{
        	pr_info("cache-ops: Unsuccessful copy_to_user of tif\n");
        	rc = -EFAULT;
	}
	
	return rc;

}

void flush_caches(void)
{
	/* Clean and Invalidate D cache */
	asm("mcr p15, 0, %0, c7, c14, 0"::"r"(0));
	asm("mcr p15, 0, %0, c7, c10, 4"::"r"(0));
}

static int cacheops(char *invalidate_flush ,unsigned int *pstart,unsigned int *vstart,unsigned int *size)
{
	unsigned long flags;

	if (*invalidate_flush == 'I' || *invalidate_flush == 'i') 
	{
#ifdef CONFIG_OUTER_CACHE
		outer_inv_range(*pstart, *pstart + *size);
#endif
	   	return *size;
	}        
    
	if (*invalidate_flush == 'F' || *invalidate_flush == 'f') 
	{
		spin_lock_irqsave(&cache_lock, flags);
        	flush_caches();
                     
#ifdef CONFIG_OUTER_CACHE
        	outer_flush_range(*pstart, *pstart + *size);
#endif
		spin_unlock_irqrestore(&cache_lock, flags);
		return *size;
	}
	
	return -EINVAL;
}

/*
 * cacheops_init_module
 *      set the owner of cache_fops, register the module
 *      as a char device, and perform any necessary
 *      initialization
 */
static int cache_init_module(void)
{
	int rc;

	cache_fops.owner = THIS_MODULE;

    	pr_info("cache-ops: *** Register device %s **\n", CACHE_OPS_DEVICE);

	rc = register_chrdev(0, CACHE_OPS_DEVICE, &cache_fops);
    	if (rc < 0)
  	{
    		pr_info("cache-test: Failed to register device.\n");
        	return rc;
    	}
	else
	{
		major=rc;
	}	
	/* call any other init functions you might use here */
	pr_info("cache-ops: Registration success.\n");
   	cache_class = class_create(THIS_MODULE, CACHE_OPS_DEVICE);
    	if (cache_class<0) 
	{
		unregister_chrdev(major, CACHE_OPS_DEVICE);
        	return -1;
	}
	device_create(cache_class, NULL, MKDEV(major, 0), NULL, CACHE_OPS_DEVICE);
	spin_lock_init(&cache_lock);
	return 0;
}
/*
 * cache_exit_module
 *      unregister the device and any necessary
 *      operations to close devices
 */ 
static void cache_exit_module(void)
{
	device_destroy(cache_class, MKDEV(major, 0));
    	class_destroy(cache_class);
	unregister_chrdev(major, CACHE_OPS_DEVICE);
}


MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("bcm_cacheops");
MODULE_LICENSE("GPL");
module_init(cache_init_module);
module_exit(cache_exit_module);
