/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

//***************************************************************************
/**
*
*   @file   atc_kernel.c
*
*   @brief  This driver is typically used for handling AT command through capi2 api.
*
*
****************************************************************************/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/mfd/max8986/max8986.h>
#include "mobcom_types.h"



#include <linux/broadcom/bcm_major.h>
#if defined(CONFIG_BOARD_ACAR)
#include <plat/syscfg.h>
#endif


#define GPS_KERNEL_MODULE_NAME  "bcm_gps"

#define GPS_KERNEL_TRACE_ON
#ifdef GPS_KERNEL_TRACE_ON
#define GPS_KERNEL_TRACE(str) printk str
#else
#define GPS_KERNEL_TRACE(str) {}
#endif

/**
 *  file ops 
 */
static int GPS_KERNEL_Open(struct inode *inode, struct file *filp);
static int GPS_KERNEL_Read(struct file *filep, char __user *buf, size_t size, loff_t *off);
static int GPS_KERNEL_Ioctl(struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg) ;
static int GPS_KERNEL_Release(struct inode *inode, struct file *filp) ;

static struct class *gps_class;


//Platform device data
typedef struct _PlatformDevData_t
{
	int init;	
} PlatformDevData_t;


static struct file_operations sFileOperations = 
{
    .owner      = THIS_MODULE,
    .open       = GPS_KERNEL_Open,
    .read       = GPS_KERNEL_Read,
    .write      = NULL,
    .ioctl      = GPS_KERNEL_Ioctl,
    .poll       = NULL,
    .mmap       = NULL,
    .release    = GPS_KERNEL_Release
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverRemove
//
//  Description: 'remove' call back function
//
//------------------------------------------------------------
static int DriverRemove(struct platform_device *pdev)
{
	return 0;
}

//======================================File operations==================================================
//***************************************************************************
/**
 *  Called by Linux I/O system to handle open() call.   
 *  @param  (in)    not used
 *  @param  (io)    file pointer    
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by struct file_operations 'open' member.
 */

static int GPS_KERNEL_Open(struct inode *inode, struct file *filp)
{
    UInt32 ret = 0;
    GPS_KERNEL_TRACE(( "GPS_KERNEL_Open\n") ) ;
    return(ret);
}

static int GPS_KERNEL_Read(struct file *filep, char __user *buf, size_t size, loff_t *off)
{
	GPS_KERNEL_TRACE(( "GPS_KERNEL_Read\n") ) ;
	return 0;
}

static int GPS_KERNEL_Ioctl(struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg) 
{
	struct regulator *gpsRegulator; 
	
	//GPS_KERNEL_TRACE(( "GPS_KERNEL_Ioctl %d %d \n",cmd,*((int *)arg))) ;
	GPS_KERNEL_TRACE(("GPS_KERNEL_Ioctl %d \n",cmd)) ;

	switch(cmd)
	{
		case 0:	  // GPS is ON
#if defined(CONFIG_BOARD_ACAR)
            board_sysconfig(SYSCFG_SERIAL, SYSCFG_ENABLE);
#endif
			gpsRegulator=regulator_get(NULL, "aldo1_3v0");
			regulator_set_mode(gpsRegulator,REGULATOR_MODE_NORMAL);
			regulator_put(gpsRegulator);
			break;
		case 1:	 //GPS is off
			gpsRegulator=regulator_get(NULL, "aldo1_3v0");
			regulator_set_mode(gpsRegulator,REGULATOR_MODE_STANDBY);
			regulator_put(gpsRegulator);
			break;
		default:
			break;
	}
	return 0;
}

static int GPS_KERNEL_Release(struct inode *inode, struct file *filp)
{
	GPS_KERNEL_TRACE(( "GPS_KERNEL_Release\n") ) ;
	return 0;
}


//***************************************************************************
/**
 *  Called by Linux I/O system to initialize module.   
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_init macro
 */
static int __init GPS_KERNEL_ModuleInit(void)
{
    int err = 1;
	struct device *myDevice;
	dev_t myDev;

    GPS_KERNEL_TRACE(("enter GPS_KERNEL_ModuleInit()\n"));
    
    //drive driver process:
    if (register_chrdev(BCM_GPS_MAJOR, GPS_KERNEL_MODULE_NAME, &sFileOperations) < 0 )
    {
        GPS_KERNEL_TRACE(("register_chrdev failed\n" ) );
        return -1 ;
    }

    gps_class = class_create(THIS_MODULE, "bcm_gps");
    if (IS_ERR(gps_class))
    {
        return PTR_ERR(gps_class);
    }
    myDev=MKDEV(BCM_GPS_MAJOR, 0);

	GPS_KERNEL_TRACE(("mydev = %d\n",myDev ) );

    myDevice=device_create(gps_class, NULL, myDev,NULL, "bcm_gps");

    err = PTR_ERR(myDevice);
    if (IS_ERR(myDevice))
    {
        GPS_KERNEL_TRACE(("device create failed\n" ) );
        return -1 ;
    }

    GPS_KERNEL_TRACE(("exit sucessfuly GPS_KERNEL_ModuleInit()\n"));
    return 0;
}

//***************************************************************************
/**
 *  Called by Linux I/O system to exit module.   
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_exit macro
 **/
static void __exit GPS_KERNEL_ModuleExit(void)
{
    GPS_KERNEL_TRACE(("GPS_KERNEL_ModuleExit()\n"));
    unregister_chrdev( BCM_GPS_MAJOR, GPS_KERNEL_MODULE_NAME ) ;
}



/**
 *  export module init and export functions
 **/
module_init(GPS_KERNEL_ModuleInit);
module_exit(GPS_KERNEL_ModuleExit);
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPS Driver");




