/*  $Date: 2010/11/04 15:40:00 $
 *  $Revision: 1.2 $ 
 */

/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html
 
 * (C) Copyright 2010 Bosch Sensortec GmbH
 * All Rights Reserved
 */


/*! \file BMA222_driver.c
    \brief This file contains all function implementations for the BMA222 in linux
    
    Details.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <mach/gpio.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/i2c-algo-bit.h>
#include <linux/wakelock.h>
#include <linux/input.h>

#include <linux/bma222.h>
#include <linux/bma222_driver.h>

#define BMA222_DEBUG 0

#if BMA222_DEBUG
#define ACCDBG(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define ACCDBG(fmt, args...)
#endif

#define CAL 1

#define ACC_DEV_MAJOR 241
#define BMA222_RETRY_COUNT	3

extern int board_hw_revision;
bma222_t * g_bma222;
bma222acc_t g_acc;

#define	ACC_ENABLED 1

/* globe variant */
static struct i2c_client *bma222_client = NULL;
struct class *acc_class;
struct bma222_data {
	struct work_struct work_acc;
	struct hrtimer timer;
	ktime_t acc_poll_delay;
	u8 state;
	struct mutex power_lock;
	struct workqueue_struct *wq;
	bma222_t bma222;
};

static int bma222_fast_calibration(signed char *data);
// this proc file system's path is "/proc/driver/bma020"
// usage :	(at the path) type "cat bma020" , it will show short information for current accelation
// 			use it for simple working test only

//#define BMA222_PROC_FS

#ifdef BMA222_PROC_FS

#include <linux/proc_fs.h>

#define DRIVER_PROC_ENTRY		"driver/bma222"
static void bma222_acc_enable(void);

static int bma222_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;
	int len = 0;
	
	mutex_lock(&g_bma222->power_lock);

	ACCDBG("[BMA222]bma222_proc_read\n");

	g_bma222->state |= ACC_ENABLED;
	bma222_acc_enable();

	mutex_unlock(&g_bma222->power_lock);

	len = (p - page) - off;
	if (len < 0) {
		len = 0;
	}

	ACCDBG("[BMA222]bma222_proc_read: success full\n");

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;
	return len;
}
#endif	//BMA222_PROC_FS

/*************************************************************************/
/*						BMA220 I2C_API						   */
/*************************************************************************/
/*	i2c delay routine for eeprom	*/
static inline void bma222_i2c_delay(unsigned int msec)
{
	mdelay(msec);
}

/*	i2c write routine for bma222	*/
static inline char bma222_i2c_write(unsigned char reg_addr, unsigned char *data, unsigned char len)
{
	uint8_t i;
	unsigned char tmp[2];
	tmp[0] = reg_addr;
	tmp[1] = *data;
	
	struct i2c_msg msg[] = {
		{
			.addr	= bma222_client->addr,
			.flags	= 0,
			.len	= 2,
			.buf	= tmp,
		}
	};
	
	for (i = 0; i < BMA222_RETRY_COUNT; i++) {
		if (i2c_transfer(bma222_client->adapter, msg, 1) >= 0) {
			break;
		}
		mdelay(10);
	}

	if (i >= BMA222_RETRY_COUNT) {
		pr_err("%s: retry over %d\n", __FUNCTION__, BMA222_RETRY_COUNT);
		return -EIO;
	}
	return 0;
}


/*	i2c read routine for bma222	*/
static inline char bma222_i2c_read(unsigned char reg_addr, unsigned char *data, unsigned char len) 
{
	uint8_t i;

	struct i2c_msg msgs[] = {
		{
			.addr	= bma222_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= &reg_addr,
		},
		{
			.addr	= bma222_client->addr,
			.flags	= I2C_M_RD,
			.len	= len,
			.buf	= data,
		}
	};
	
	for (i = 0; i < BMA222_RETRY_COUNT; i++) {
		if (i2c_transfer(bma222_client->adapter, msgs, 2) >= 0) {
			break;
		}
		mdelay(10);
	}

	if (i >= BMA222_RETRY_COUNT) {
		pr_err("%s: retry over %d\n", __FUNCTION__, BMA222_RETRY_COUNT);
		return -EIO;
	}
	return 0;	
}


/*************************************************************************/
/*						BMA222 Sysfs						   */
/*************************************************************************/
static ssize_t bma222_fs_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;
	bma222acc_t acc;
	//bma222_read_accel_xyz(&acc);

        acc.x = g_acc.x;
        acc.y = g_acc.y;
        acc.z = g_acc.z;

       // printk("x: %d,y: %d,z: %d\n", acc.x, acc.y, acc.z);
	count = sprintf(buf,"%d,%d,%d\n", acc.x, acc.y, acc.z );

	return count;
}

static ssize_t bma222_fs_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	//buf[size]=0;
	ACCDBG("[BMA222]bma222_fs_write data --> %s\n", buf);
	return size;
}

static DEVICE_ATTR(acc_file, S_IRUGO | S_IRUSR | S_IROTH /*| S_IXOTH*/, bma222_fs_read, NULL /*bma222_fs_write*/);

static ssize_t bma222_fs_cal(struct device *dev, struct device_attribute *attr, char *buf)
{
	int err, count =0;
	bma222acc_t acc;
	char data[3];
	
	data[0] = 0; //x
	data[1] = 0; //y
	data[2] = 2; //z ("2" means "-1"-> 0 0 -64) ("1" means "1" -> 0 0 64)

	bma222_read_accel_xyz(&acc);
    
	printk(KERN_INFO "[BMA222] +bma222_fs_cal  X: %d Y: %d Z: %d \n",acc.x, acc.y, acc.z);
	
	printk(KERN_INFO "[BMA222] +bma222_fs_cal data0: %d data1: %d data2: %d \n",data[0], data[1], data[2]);
#if CAL
	err = bma222_fast_calibration(data);
#endif					
	bma222_read_accel_xyz(&acc);
			
        g_acc.x = acc.x;
        g_acc.y = acc.y;
        g_acc.z = acc.z;
			
	printk(KERN_INFO "[BMA222] -bma222_fs_cal X: %d Y: %d Z: %d \n",acc.x, acc.y, acc.z);

	count = sprintf(buf,"%d\n", err);

	return count;
}
static DEVICE_ATTR(calibrate, S_IRUGO | S_IRUSR | S_IROTH, bma222_fs_cal, NULL);

/*	read command for BMA222 device file	*/
static ssize_t bma222_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{	
	bma222acc_t acc;	
	int ret;
	if( bma222_client == NULL )
	{
		printk(KERN_ERR "I2C driver not install\n");
		return -1;
	}

	bma222_read_accel_xyz(&acc);

	ACCDBG("[BMA222] bma222_read: X/Y/Z axis: %-8d %-8d %-8d\n" ,(int)acc.x, (int)acc.y, (int)acc.z);  

	if( count != sizeof(acc) )
	{
		return -1;
	}
	ret = copy_to_user(buf,&acc, sizeof(acc));
	if( ret != 0 )
	{
            ACCDBG("[BMA222] bma222_read: copy_to_user result: %d\n", ret);
	}
	return sizeof(acc);
}

/*	write command for BMA222 device file	*/
static ssize_t bma222_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	if( bma222_client == NULL )
		return -1;

	ACCDBG("[BMA222] should be accessed with ioctl command\n");
	return 0;
}

/*	open command for BMA222 device file	*/
static int bma222_open(struct inode *inode, struct file *file)
{
	if( bma222_client == NULL)
	{
		printk(KERN_ERR "I2C driver not install\n"); 
		return -1;
	}

	printk(KERN_INFO "[BMA222] has been opened\n");
	return 0;
}

/*	release command for BMA222 device file	*/
static int bma222_close(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "[BMA222]%s\n",__FUNCTION__);	
	return 0;
}


#if CAL
//static int bma222_fast_calibration(signed char *data);	
/******************************************************************************
*
 * Description: *//**\brief  This API start the fast inline calibration and 
store
 *              offset value into EEPROM
 *
 *
 *  \param       signed char data[0]  --> x axis target offset  
 *                           data[1]  --> y axis target offset
 *                           data[2]  --> z axis target offset
 *                  00b ------ 0g
 *                  01b ------ 1g
 *                  10b ------ -1g
 *                  11b ------ 0g
 * 
******************************************************************************/
static int bma222_fast_calibration(signed char *data)
{
    signed char tmp;

    printk(KERN_INFO "[BMA222] %s\n",__FUNCTION__);
    printk(KERN_INFO "[BMA222] data are %d,%d,%d\n",data[0],data[1],data[2]);
    printk(KERN_INFO "[BMA222] start x axis fast calibration\n");

    bma222_set_offset_target_x(data[0]);
    //bma222_get_offset_target_x(&tmp);
    //printk(KERN_INFO "x offset is %d\n",tmp);
    //bma222_get_offset_filt_x(&tmp);
    //printk(KERN_INFO "x offset filt is %d\n",tmp);
    tmp=1;//selet x axis in cal_trigger
    bma222_set_cal_trigger(tmp);
    do
    {
        mdelay(2);
        bma222_get_cal_ready(&tmp);

        printk(KERN_INFO "[BMA222]wait 2ms and got cal ready flag is %d\n",tmp);

    }while(tmp==0);
	
    bma222_get_offset_filt_x(&tmp);
    printk(KERN_INFO "[BMA222] offset filt is %d\n",tmp);
    printk(KERN_INFO "[BMA222] x axis fast calibration finished\n");
    printk(KERN_INFO "[BMA222] start y axis fast calibration\n");

    bma222_set_offset_target_y(data[1]);
    //bma222_get_offset_target_y(&tmp);
    //printk(KERN_INFO "y offset is %d\n",tmp);
    //bma222_get_offset_filt_y(&tmp);
    //printk(KERN_INFO "y offset filt is %d\n",tmp);
    tmp=2;//selet y axis in cal_trigger
    bma222_set_cal_trigger(tmp);
    do
    {
        mdelay(2); 
        bma222_get_cal_ready(&tmp);

        printk(KERN_INFO "[BMA222]wait 2ms and got cal ready flag is %d\n",tmp);

    }while(tmp==0);
	
    bma222_get_offset_filt_y(&tmp);
    printk(KERN_INFO "[BMA222] y offset filt is %d\n",tmp);
    printk(KERN_INFO "[BMA222] y axis fast calibration finished\n");
    printk(KERN_INFO "[BMA222] start z axis fast calibration\n");

    bma222_set_offset_target_z(data[2]);

    //bma222_get_offset_target_z(&tmp);
    //printk(KERN_INFO "z offset is %d\n",tmp);
    //bma222_get_offset_filt_z(&tmp);
    //printk(KERN_INFO "z offset filt is %d\n",tmp);
    tmp=3;//selet z axis in cal_trigger
    bma222_set_cal_trigger(tmp);
    do
    {
        mdelay(2); 
        bma222_get_cal_ready(&tmp);
        printk(KERN_INFO "[BMA222]wait 2ms and got cal ready flag is %d\n",tmp);
    }while(tmp==0);
	
    bma222_get_offset_filt_z(&tmp);
    printk(KERN_INFO "[BMA222] z offset filt is %d\n",tmp);
    printk(KERN_INFO "[BMA222] z axis fast calibration finished\n");
    printk(KERN_INFO "[BMA222] store xyz offset to eeprom\n");

    tmp=1;//unlock eeprom
    bma222_set_ee_w(tmp);
    bma222_set_ee_prog_trig();//update eeprom
    do
    {
        mdelay(2); 
        bma222_get_eeprom_writing_status(&tmp);

        printk(KERN_INFO "[BMA222] wait 2ms and got eeprom writing status is %d\n",tmp);

    }while(tmp==0);
	
    tmp=0;//lock eemprom
    bma222_set_ee_w(tmp);
    printk(KERN_INFO "[BMA222] eeprom writing is finished\n");
    return 0;
}
#endif


/*	ioctl command for BMA222 device file	*/
static int bma222_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	unsigned char data[6];
	unsigned char data_cal[3];
	bma222acc_t acc;

	/* check cmd */
	if(_IOC_TYPE(cmd) != BMA222_IOC_MAGIC)	
	{
		printk(KERN_ERR "cmd magic type error\n");
		return -ENOTTY;
	}
	if(_IOC_NR(cmd) > BMA222_IOC_MAXNR)
	{
		printk(KERN_ERR "cmd number error\n");
		return -ENOTTY;
	}

	if(_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,(void __user*)arg, _IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd));
	
	if(err)
	{
		printk(KERN_ERR "cmd access_ok error\n");
		return -EFAULT;
	}
	/* check bam220_client */
	if( bma222_client == NULL)
	{
		printk(KERN_ERR "I2C driver not install\n"); 
		return -EFAULT;
	}
	
	/* cmd mapping */

	switch(cmd)
	{
	case BMA222_SOFT_RESET:
		err = bma222_soft_reset();
		printk(KERN_INFO "[BMA222] + IOCTL bma222_soft_reset \n");                                                            
		return err;


	case BMA222_SET_RANGE:
		if(copy_from_user(data,(unsigned char*)arg,1)!=0)
		{
			printk(KERN_ERR "copy_from_user error\n");
			return -EFAULT;
		}
		err = bma222_set_range(*data);
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_SET_RANGE \n");                                                    
		return err;

	case BMA222_GET_RANGE:
		err = bma222_get_range(data);
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_GET_RANGE \n");                                            
		if(copy_to_user((unsigned char*)arg,data,1)!=0)
		{
			printk(KERN_ERR "copy_to_user error\n");
			return -EFAULT;
		}
		return err;

	case BMA222_SET_MODE:
		if(copy_from_user(data,(unsigned char*)arg,1)!=0)
		{
			printk(KERN_ERR "copy_from_user error\n");
			return -EFAULT;
		}
		err = bma222_set_mode(*data);
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_SET_MODE : %d\n", *data);                                    
		return err;

	case BMA222_GET_MODE:
		err = bma222_get_mode(data);
		if(copy_to_user((unsigned char*)arg,data,1)!=0)
		{
			printk(KERN_ERR "copy_to_user error\n");
			return -EFAULT;
		}
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_GET_MODE : %d\n", *data);           
		return err;

	case BMA222_SET_BANDWIDTH:
		if(copy_from_user(data,(unsigned char*)arg,1)!=0)
		{
			printk(KERN_ERR "copy_from_user error\n");
			return -EFAULT;
		}
		err = bma222_set_bandwidth(*data);
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_SET_BANDWIDTH \n");                    
		return err;

	case BMA222_GET_BANDWIDTH:
		err = bma222_get_bandwidth(data);
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_GET_BANDWIDTH \n");            
		if(copy_to_user((unsigned char*)arg,data,1)!=0)
		{
			printk(KERN_ERR "copy_to_user error\n");
			return -EFAULT;
		}
		return err;

	case BMA222_RESET_INTERRUPT:
		err = bma222_reset_interrupt();
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_RESET_INTERRUPT \n");                            
		return err;

	case BMA222_READ_ACCEL_X:
		err = bma222_read_accel_x((short*)data);
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_READ_ACCEL_X \n");                    
		if(copy_to_user((short*)arg,(short*)data,2)!=0)
		{
			printk(KERN_ERR "copy_to_user error\n");
			return -EFAULT;
		}
		return err;

	case BMA222_READ_ACCEL_Y:
		err = bma222_read_accel_y((short*)data);
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_READ_ACCEL_Y \n");            
		if(copy_to_user((short*)arg,(short*)data,2)!=0)
		{
			printk(KERN_ERR "copy_to_user error\n");
			return -EFAULT;
		}
		return err;

	case BMA222_READ_ACCEL_Z:
		err = bma222_read_accel_z((short*)data);
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_READ_ACCEL_Z \n");        
		if(copy_to_user((short*)arg,(short*)data,2)!=0)
		{
			printk(KERN_ERR "copy_to_user error\n");
			return -EFAULT;
		}
		return err;
		
	case BMA222_READ_ACCEL_XYZ:
		err = bma222_read_accel_xyz((bma222acc_t*)data);
		acc.x = data[1] << 8 | data[0];
		acc.y = data[3] << 8 | data[2];
		acc.z = data[5] << 8 | data[4];        
		ACCDBG("[BMA222] + IOCTL BMA222_READ_ACCEL_XYZ : %d, %d, %d \n",acc.x, acc.y, acc.z);
		if(copy_to_user((bma222acc_t*)arg,(bma222acc_t*)data,6)!=0)
		{
			printk(KERN_ERR "copy_to error\n");
			return -EFAULT;
		}		
		return err;
		
	case BMA222_CALIBRATION:		
		bma222_read_accel_xyz(&acc);
		printk(KERN_INFO "[BMA222] + IOCTL BMA222_CALIBRATION X: %d Y: %d Z: %d \n",acc.x, acc.y, acc.z);

		if(copy_from_user(data_cal,(unsigned char*)arg,3)!=0)
		{
			printk(KERN_INFO "copy_from_user error\n");
			return -EFAULT;		
		}	
        
		printk(KERN_INFO "[BMA222] IOCTL BMA222_CALIBRATION data0: %d data1: %d data2: %d \n",data_cal[0], data_cal[1], data_cal[2]);
#if CAL
		err = bma222_fast_calibration(data_cal);
#endif					
		bma222_read_accel_xyz(&acc);
			
		printk(KERN_INFO "[BMA222] - IOCTL BMA222_CALIBRATION X: %d Y: %d Z: %d \n",acc.x, acc.y, acc.z);
		return err;
		
	default:
		return 0;
	}
}

static const struct file_operations bma222_fops = {
	.owner = THIS_MODULE,
	.read = bma222_read,
	.write = bma222_write,
	.open = bma222_open,
	.release = bma222_close,
	.ioctl = bma222_ioctl,
};

static struct miscdevice bma222_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bma222",
	.fops = &bma222_fops,
};

static int bma222_detect(struct i2c_client *client, int kind,
			  struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	ACCDBG("[BMA222] %s\n", __FUNCTION__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;

	strlcpy(info->type, "bma222", I2C_NAME_SIZE);

	return 0;
}



static void I2c_udelaySet(void)
{
	struct i2c_algo_bit_data *adap = bma222_client->adapter->algo_data;
	adap->udelay = 1;
}

static void bma222_acc_enable(void)
{
//	printk(KERN_INFO "[BMA222] starting poll timer, delay %lldns\n", ktime_to_ns(g_bma222->acc_poll_delay));
        printk(KERN_INFO "[BMA222] bma222_acc_enable, timer delay %lldns\n", ktime_to_ns(g_bma222->acc_poll_delay));
	hrtimer_start(&g_bma222->timer, g_bma222->acc_poll_delay, HRTIMER_MODE_REL);
}

static void bma222_acc_disable(void)
{

//	printk(KERN_INFO "[BMA222] cancelling poll timer\n");
	printk(KERN_INFO "[BMA222] bma222_acc_disable\n");

	hrtimer_cancel(&g_bma222->timer);
	cancel_work_sync(&g_bma222->work_acc);
}

/////////////////////////////////////////////////////////////////////////////////////

static ssize_t poll_delay_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%lld\n", ktime_to_ns(g_bma222->acc_poll_delay));
}


static ssize_t poll_delay_store(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	int64_t new_delay;
	int err;

	err = strict_strtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	ACCDBG("[BMA222] new delay = %lldns, old delay = %lldns\n",   new_delay, ktime_to_ns(g_bma222->acc_poll_delay));

	mutex_lock(&g_bma222->power_lock);
	if (new_delay != ktime_to_ns(g_bma222->acc_poll_delay)) {
		bma222_acc_disable();
		g_bma222->acc_poll_delay = ns_to_ktime(new_delay);
		if (g_bma222->state & ACC_ENABLED) {
			bma222_acc_enable();
		}
	}
	mutex_unlock(&g_bma222->power_lock);

	return size;
}

static ssize_t acc_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", (g_bma222->state & ACC_ENABLED) ? 1 : 0);
}


static ssize_t acc_enable_store(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	bool new_value;

	if (sysfs_streq(buf, "1"))
		new_value = true;
	else if (sysfs_streq(buf, "0"))
		new_value = false;
	else {
		pr_err("%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	mutex_lock(&g_bma222->power_lock);

	ACCDBG("[BMA222] new_value = %d, old state = %d\n", new_value, (g_bma222->state & ACC_ENABLED) ? 1 : 0);

	if (new_value && !(g_bma222->state & ACC_ENABLED)) {
		g_bma222->state |= ACC_ENABLED;
		bma222_acc_enable();
	} else if (!new_value && (g_bma222->state & ACC_ENABLED)) {
		bma222_acc_disable();
		g_bma222->state = 0;
	}
	mutex_unlock(&g_bma222->power_lock);
	return size;
}

static DEVICE_ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
		   poll_delay_show, poll_delay_store);

static struct device_attribute dev_attr_acc_enable =
	__ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP,
	       acc_enable_show, acc_enable_store);

static struct attribute *acc_sysfs_attrs[] = {
	&dev_attr_acc_enable.attr,
	&dev_attr_poll_delay.attr,
	NULL
};

static struct attribute_group acc_attribute_group = {
	.attrs = acc_sysfs_attrs,
};
///////////////////////////////////////////////////////////////////////////////////

static void bma222_work_func_acc(struct work_struct *work)
{
	bma222acc_t acc;
	int err;
		
	err = bma222_read_accel_xyz(&acc);
	
	//printk("[BMA222] ##### %d,  %d,  %d\n", acc.x, acc.y, acc.z );

        g_acc.x = acc.x;
        g_acc.y = acc.y;
        g_acc.z = acc.z;

	input_report_rel(g_bma222->acc_input_dev, REL_X, acc.x);
	input_report_rel(g_bma222->acc_input_dev, REL_Y, acc.y);
	input_report_rel(g_bma222->acc_input_dev, REL_Z, acc.z);
	input_sync(g_bma222->acc_input_dev);
}

/* This function is for light sensor.  It operates every a few seconds.
 * It asks for work to be done on a thread because i2c needs a thread
 * context (slow and blocking) and then reschedules the timer to run again.
 */
static enum hrtimer_restart bma222_timer_func(struct hrtimer *timer)
{
	queue_work(g_bma222->wq, &g_bma222->work_acc);
	hrtimer_forward_now(&g_bma222->timer, g_bma222->acc_poll_delay);
	return HRTIMER_RESTART;
}

static int bma222_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int err = 0;
	int tempvalue;
	struct input_dev *input_dev;
	struct bma222_data *data;

	printk(KERN_INFO "[BMA222] %s\n",__FUNCTION__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		printk(KERN_ERR "i2c_check_functionality error\n");
		goto exit;
	}
	data = kzalloc(sizeof(struct bma222_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}		

	printk(KERN_INFO "[BMA222] [%s] slave addr = %x\n", __func__, client->addr);
	
	/* read chip id */
	tempvalue = 0;
#ifdef BMA222_SMBUS
	tempvalue = i2c_smbus_read_word_data(client, 0x00);
#else
	err = i2c_master_send(client, (char*)&tempvalue, 1);
	if(err < 0)
	{
		printk(KERN_ERR "bma222_probe : i2c_master_send [%d]\n", err);			
	}
    
	err = i2c_master_recv(client, (char*)&tempvalue, 1);
	if(err < 0)
	{
		printk(KERN_ERR "bma222_probe : i2c_master_recv [%d]\n", err);			
	}       
#endif

	if((tempvalue&0x00FF) == 0x0003)
	{
		printk(KERN_INFO "[BMA222] Bosch Sensortec Device detected!\n [BMA222] registered I2C driver!\n");

		bma222_client = client;
//		I2c_udelaySet();
	}
	else
	{
		printk(KERN_INFO "[BMA222] Bosch Sensortec Device not found, i2c error %d \n", tempvalue);
		bma222_client = client;
#if 0
		bma222_client = NULL;
		err = -1;
		goto kfree_exit;
#endif
	}
	
	i2c_set_clientdata(bma222_client, data);
	err = misc_register(&bma222_device);
	if (err) {
		printk(KERN_ERR "bma222 device register failed\n");
		goto kfree_exit;
	}

	printk(KERN_INFO "[BMA222] bma222 device create ok\n");

	g_bma222 = &data->bma222;
//////////////////////////////////////////////////////////////////////////////
	mutex_init(&g_bma222->power_lock);

	/* hrtimer settings.  we poll for light values using a timer. */
	hrtimer_init(&g_bma222->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	g_bma222->acc_poll_delay = ns_to_ktime(50 * NSEC_PER_MSEC);
	g_bma222->timer.function = bma222_timer_func;

	/* the timer just fires off a work queue request.  we need a thread
	   to read the i2c (can be slow and blocking). */
	g_bma222->wq = create_singlethread_workqueue("bma222_wq");
	if (!g_bma222->wq) {
		err = -ENOMEM;
		printk(KERN_ERR "%s: could not create workqueue\n", __func__);
		goto err_create_workqueue;
	}
	/* this is the thread function we run on the work queue */
	INIT_WORK(&g_bma222->work_acc, bma222_work_func_acc);

///////////////////////////////////////////////////////////////////////////////////
	/* allocate lightsensor-level input_device */
	input_dev = input_allocate_device();
	if (!input_dev) {
		printk(KERN_ERR "%s: could not allocate input device\n", __func__);
		err = -ENOMEM;
		goto err_input_allocate_device_light;
	}
	input_set_drvdata(input_dev, g_bma222);
	input_dev->name = "accelerometer_sensor";


	set_bit(EV_REL, input_dev->evbit);	
	/* 32768 == 1g, range -4g ~ +4g */
	/* acceleration x-axis */
	input_set_capability(input_dev, EV_REL, REL_X);
	input_set_abs_params(input_dev, REL_X, -256, 256, 0, 0);
	/* acceleration y-axis */
	input_set_capability(input_dev, EV_REL, REL_Y);
	input_set_abs_params(input_dev, REL_Y, -256, 256, 0, 0);
	/* acceleration z-axis */
	input_set_capability(input_dev, EV_REL, REL_Z);
	input_set_abs_params(input_dev, REL_Z, -256, 256, 0, 0);

	printk(KERN_INFO "[BMA222] registering lightsensor-level input device\n");

	err = input_register_device(input_dev);
	if (err < 0) {
		printk(KERN_ERR "%s: could not register input device\n", __func__);
		input_free_device(input_dev);
		goto err_input_register_device_light;
	}
	g_bma222->acc_input_dev = input_dev;


	err = sysfs_create_group(&input_dev->dev.kobj,&acc_attribute_group);
	if (err) {
		printk(KERN_ERR "Creating bh1721 attribute group failed");
		goto error_device;
	}
//////////////////////////////////////////////////////////////////////////////

	/* bma220 sensor initial */
	data->bma222.bus_write = bma222_i2c_write;
	data->bma222.bus_read = bma222_i2c_read;
	data->bma222.delay_msec = bma222_i2c_delay;	

	bma222_init(&data->bma222);
	bma222_set_bandwidth(1); //bandwidth 125Hz(4) => 62.50Hz(3) =>31.25Hz(2) => 15.63Hz(1) for filtering noise
	bma222_set_range(0);	//range +/- 2G

#ifdef BMA222_PROC_FS
	create_proc_read_entry(DRIVER_PROC_ENTRY, 0, 0, bma222_proc_read, NULL);
#endif	//BMA222_PROC_FS

	return 0;

error_device:
	sysfs_remove_group(&client->dev.kobj, &acc_attribute_group);
err_input_register_device_light:
	input_unregister_device(g_bma222->acc_input_dev);
err_input_allocate_device_light:	
    	destroy_workqueue(g_bma222->wq);
err_create_workqueue:
	mutex_destroy(&data->power_lock);
kfree_exit:
	kfree(data);
exit:
	return err;
}


static int bma222_remove(struct i2c_client *client)
{
	struct bma222_data *data = i2c_get_clientdata(client);

	if (g_bma222->state & ACC_ENABLED)
	{
		g_bma222->state = 0;
		bma222_acc_disable();
	}
	sysfs_remove_group(&g_bma222->acc_input_dev->dev.kobj, &acc_attribute_group);
	input_unregister_device(g_bma222->acc_input_dev);

	destroy_workqueue(g_bma222->wq);
	mutex_destroy(&g_bma222->power_lock);
	
	printk(KERN_INFO "[BMA222] %s\n",__FUNCTION__);

	misc_deregister(&bma222_device);

	//i2c_detach_client(client);
	kfree(data);
	g_bma222 = NULL;
	return 0;
}

static int bma222_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret = 0;;

	if((ret = bma222_set_mode(bma222_MODE_SUSPEND)) != 0)	// 2: suspend mode
		printk(KERN_ERR "[%s] Change to Suspend Mode is failed\n",__FUNCTION__);
		
	if (g_bma222->state & ACC_ENABLED) 
		bma222_acc_disable();

	printk(KERN_INFO "[BMA222] [%s] bma220 !!suspend mode!!\n",__FUNCTION__);
	return 0;
}

static int bma222_resume(struct i2c_client *client)
{
	int ret = 0;

	if((ret = bma222_set_mode(bma222_MODE_NORMAL)) != 0)	// Normal mode
		printk(KERN_ERR "[%s] Change to Normal Mode is failed\n",__FUNCTION__);

	if((ret = bma222_set_bandwidth(1)) != 0)	//bandwidth 125Hz(4) => 62.50Hz(3) =>31.25Hz(2) => 15.63Hz(1)  for filtering noise
		printk(KERN_ERR "[%s] Set Bandwidth is failed\n",__FUNCTION__);

	if((ret = bma222_set_range(0)) != 0)	// range +/- 2G
		printk(KERN_ERR "[%s] Set range is failed\n",__FUNCTION__);

	if (g_bma222->state & ACC_ENABLED)
		bma222_acc_enable();

	printk(KERN_INFO "[BMA222] [%s] bma220 !!resume mode!!\n",__FUNCTION__);
	return 0;
}

static unsigned short normal_i2c[] = { I2C_CLIENT_END};
I2C_CLIENT_INSMOD_1(bma222);

static const struct i2c_device_id bma222_id[] = {
	{ "bma222", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, bma222_id);

static struct i2c_driver bma222_driver = {	
	.driver = {
		.owner	= THIS_MODULE,	
		.name	= "bma222",
	},
	//.class		= I2C_CLASS_HWMON,
	.id_table		= bma222_id,
//	.address_data	= &addr_data,		// 3.2 Kernel
	.probe		= bma222_probe,
	.remove		= bma222_remove,
	.detect		= bma222_detect,
	.suspend	= bma222_suspend,
	.resume		= bma222_resume,
};

static int __init BMA222_init(void)
{
	struct device *dev_t;

	printk(KERN_INFO "[BMA222] %s\n",__FUNCTION__);

	acc_class = class_create(THIS_MODULE, "accelerometer");

	if (IS_ERR(acc_class)) 
		return PTR_ERR( acc_class );

	dev_t = device_create( acc_class, NULL, MKDEV(ACC_DEV_MAJOR, 0), "%s", "accelerometer");

	if (device_create_file(dev_t, &dev_attr_acc_file) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_acc_file.attr.name);
	if (device_create_file(dev_t, &dev_attr_calibrate) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_calibrate.attr.name);

	if (IS_ERR(dev_t)) 
	{
		return PTR_ERR(dev_t);
	}

	return i2c_add_driver(&bma222_driver);
}

static void __exit BMA222_exit(void)
{
	printk(KERN_INFO "[BMA222] BMA222_ACCEL exit\n");

	i2c_del_driver(&bma222_driver);
	class_destroy( acc_class );
}

MODULE_DESCRIPTION("BMA222 driver");

module_init(BMA222_init);
module_exit(BMA222_exit);




