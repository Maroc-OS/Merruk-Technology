/* drivers/input/touchscreen/melfas_i2c_rmi.c
 *
 * Copyright (C) 2007 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <mach/gpio.h>
#include <linux/device.h>
#include <linux/regulator/consumer.h>
#include <linux/melfas_ts.h>

#include <linux/firmware.h>

static struct workqueue_struct *melfas_wq;

#define HEX_HW_VER	0x01
#define HEX_SW_VER	0x05	//change the version while Firmware Update 
#define FW_VERSION			    0x06

#define MAX_X	240 
#define MAX_Y	320
#define TSP_INT 30

#define MMS128_MAX_TOUCH		5
#define TS_READ_START_ADDR 	    0x10
#define TS_READ_VERSION_ADDR	0x31
#define TS_READ_REGS_LEN 		5
#define MELFAS_MAX_TOUCH		MMS128_MAX_TOUCH
#define MTSI_VERSION		    0x05

//#define __TOUCH_DEBUG__ 1

#define I2C_RETRY_CNT			10

#define PRESS_KEY				1
#define RELEASE_KEY				0

#define TOUCH_ON 1
#define TOUCH_OFF 0

#define SET_DOWNLOAD_BY_GPIO	1

static struct regulator *touch_regulator=NULL;
//add by brcm
static DEFINE_SPINLOCK(melfas_spin_lock);

struct melfas_ts_data *ts_global;

static int firmware_ret_val = -1;
int tsp_irq;
int firm_update( void );


#include "mcs8000_download.h"

enum
{
	TOUCH_SCREEN=0,
	TOUCH_KEY
};

struct muti_touch_info
{
	int strength;
	int width;	
	int posX;
	int posY;
};

struct melfas_ts_data {
	uint16_t addr;
	struct i2c_client *client; 
	struct input_dev *input_dev;
    	int use_irq;
	struct work_struct  work;
	struct early_suspend early_suspend;
};

/* sys fs */
struct class *touch_class;
EXPORT_SYMBOL(touch_class);
struct device *firmware_dev;
EXPORT_SYMBOL(firmware_dev);

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_store( struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_ret_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR(firmware	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_show, firmware_store);
static DEVICE_ATTR(firmware_ret	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_ret_show, firmware_ret_store);
/* sys fs */

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int set_irq_type(unsigned int irq, unsigned int type);

static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];

void touch_ctrl_regulator(int on_off)
{
	if(on_off==TOUCH_ON)
	{
			regulator_set_voltage(touch_regulator,2900000,2900000);
			regulator_enable(touch_regulator);
	}
	else
	{
			regulator_disable(touch_regulator);
	}
}
EXPORT_SYMBOL(touch_ctrl_regulator);

static int melfas_init_panel(struct melfas_ts_data *ts)
{
 	uint8_t buf[TS_READ_REGS_LEN];  
      int ret;
      int i=0;

        printk("[TSP] %s, %d\n", __func__, __LINE__ );   

	if(ts ==NULL)
       printk("[TSP] melfas_ts_init_read : TS NULL\n");

	buf[0] = TS_READ_START_ADDR;

	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		ret = i2c_master_send(ts->client, buf, 1);

		if(ret >=0)
		{
			ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);

			if(ret >=0)
			{
				break; // i2c success
			}
		}
	}

	return true;
}

void TSP_forced_release_forkey(struct melfas_ts_data *ts)
{
	int i;
	int temp_value=0;

	for(i=0; i<MELFAS_MAX_TOUCH; i++)
		{
			if(g_Mtouch_info[i].strength== -1)
				continue;

			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
		      input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
		      input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0 );
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);      				
			input_mt_sync(ts->input_dev);   

                   printk("[TSP] force release\n");
            #ifdef __TOUCH_DEBUG__
             printk("[TSP] i : %d, x: %d, y: %d\n, 3:%d, 4:%d", i, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength, g_Mtouch_info[i].width);
		#endif			
			if(g_Mtouch_info[i].strength == 0)
				g_Mtouch_info[i].strength = -1;

            		temp_value++;
		}

	if(temp_value>0)
		input_sync(ts->input_dev);

	
}
EXPORT_SYMBOL(TSP_forced_release_forkey);

static void melfas_ts_work_func(struct work_struct *work)
{
	int ret = 0, i;
	uint8_t buf[TS_READ_REGS_LEN];
	int touchType=0, touchState =0, touchID=0, posX=0, posY=0, width = 0, strength=0, keyID = 0, reportID = 0;
       unsigned long    flags;

	struct melfas_ts_data *ts = container_of(work, struct melfas_ts_data, work);

      #ifdef __TOUCH_DEBUG__
	printk("[TSP] %s, %d\n", __func__, __LINE__ );
       #endif

	if(ts ==NULL)
       printk("[TSP] melfas_ts_work_func : TS NULL\n");
    
	buf[0] = TS_READ_START_ADDR;

	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		ret = i2c_master_send(ts->client, buf, 1);

		if(ret >=0)
		{
			ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);

			if(ret >=0)
			{
				break; // i2c success
			}
		}
	}
        spin_lock_irqsave(&melfas_spin_lock, flags);
	if (ret < 0)
	{
             printk("[TSP] melfas_ts_work_func: i2c failed : %d\n", ret );
             enable_irq(ts->client->irq);
             spin_unlock_irqrestore(&melfas_spin_lock, flags);
             touch_ctrl_regulator(TOUCH_OFF);
             touch_ctrl_regulator(TOUCH_ON);
             melfas_init_panel(ts);
		return ;	
	}
	else 
	{

#if MTSI_VERSION == 0x07 // MTSI 0.7
		touchType  = (buf[0]>>5)&0x03;
		touchState = (buf[0]>>4)&0x01;
#else // MTSI 0.5

		touchType  = (buf[0]>>6)&0x03;
		touchState = (buf[0]>>4)&0x03;
      
#endif
		reportID = (buf[0]&0x0f);
		posX = (  ( buf[1]& 0x0F) << (8)  ) +  buf[2];
		posY = (( (buf[1]& 0xF0 ) >> 4 ) << (8)) +  buf[3];
#if MTSI_VERSION == 0x07 // MTSI 0.7
		keyID = width = buf[4];
		strength = buf[5]; 			
#else // MTSI 0.5
		keyID = buf[0]&0x07;
		strength = buf[4]; 
#endif
		touchID = reportID-1;

            #ifdef __TOUCH_DEBUG__
             printk("[TSP] 1 : %d, 2: %d, 3: %d\n, 4:%d, 5:%d", buf[0], buf[1], buf[2], buf[3], buf[4]);
             printk("[TSP] touchType : %d, touchstate : %d\n", touchType, touchState);
		#endif	
        
		if((touchID > MELFAS_MAX_TOUCH-1)||(touchID<0))
		{
               printk("[TSP] melfas_ts_work_func: Touch ID: %d\n",  touchID );
                enable_irq(ts->client->irq);       
                spin_unlock_irqrestore(&melfas_spin_lock, flags);
               touch_ctrl_regulator(TOUCH_OFF);
               touch_ctrl_regulator(TOUCH_ON);
               melfas_init_panel(ts);
			return ;
		}

        	if(touchType == TOUCH_SCREEN)
		{
			g_Mtouch_info[touchID].posX= posX;
			g_Mtouch_info[touchID].posY= posY;
			g_Mtouch_info[touchID].width= width;			

			if(touchState)
				g_Mtouch_info[touchID].strength= strength;
			else
				g_Mtouch_info[touchID].strength = 0;

			for(i=0; i<MELFAS_MAX_TOUCH; i++)
			{
				if(g_Mtouch_info[i].strength== -1)
					continue;

				input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
			      input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength );
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);      				
				input_mt_sync(ts->input_dev);   

            #ifdef __TOUCH_DEBUG__
             printk("[TSP] i : %d, x: %d, y: %d\n, 3:%d, 4:%d", i, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength, g_Mtouch_info[i].width);
		#endif			
				if(g_Mtouch_info[i].strength == 0)
					g_Mtouch_info[i].strength = -1;
			}
		}
            	else if(touchType == TOUCH_KEY)
		{
			if (keyID == 0x1)
				input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);		
			if (keyID == 0x2)
				input_report_key(ts->input_dev, KEY_HOME, touchState ? PRESS_KEY : RELEASE_KEY);
			if (keyID == 0x3)
				input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);
			if (keyID == 0x4)
				input_report_key(ts->input_dev, KEY_SEARCH, touchState ? PRESS_KEY : RELEASE_KEY);			

		printk(KERN_ERR "melfas_ts_work_func: keyID : %d, touchState: %d\n", keyID, touchState);
			
		}
        		input_sync(ts->input_dev);
	}

	enable_irq(ts->client->irq);
	spin_unlock_irqrestore(&melfas_spin_lock, flags);
}

static irqreturn_t melfas_ts_irq_handler(int irq, void *dev_id)
{
	struct melfas_ts_data *ts = dev_id;

       #ifdef __TOUCH_DEBUG__
	printk("[TSP] %s, %d\n", __func__, __LINE__ );
       #endif
       
	disable_irq_nosync(ts->client->irq);
	queue_work(melfas_wq, &ts->work);

	return IRQ_HANDLED;
}

static int melfas_ts_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	struct melfas_ts_data *ts;
	int ret = 0; 
 	uint8_t buf[TS_READ_REGS_LEN];   
      	uint8_t buf_firmware;
      int i=0;
      
	touch_ctrl_regulator(TOUCH_ON);

	printk("[TSP] %s, %d\n", __func__, __LINE__ );

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	INIT_WORK(&ts->work, melfas_ts_work_func);
	ts->client = client;
	i2c_set_clientdata(client, ts);

	ts_global = ts;

      tsp_irq=client->irq;

	/* sys fs */
	touch_class = class_create(THIS_MODULE, "touch");
	if (IS_ERR(touch_class))
		pr_err("Failed to create class(touch)!\n");

	firmware_dev = device_create(touch_class, NULL, 0, NULL, "firmware");
	if (IS_ERR(firmware_dev))
		pr_err("Failed to create device(firmware)!\n");

	if (device_create_file(firmware_dev, &dev_attr_firmware) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_firmware.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_firmware_ret) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_firmware_ret.attr.name);

	/* sys fs */

#if SET_DOWNLOAD_BY_GPIO
	buf_firmware = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts->client, &buf_firmware, 1);
	if(ret < 0)
	{
		printk(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);			
	}

	ret = i2c_master_recv(ts->client, &buf_firmware, 1);
	if(ret < 0)
	{
		printk(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);			
	}

	if(buf_firmware < FW_VERSION)
	{
           	printk("[TSP] disable_irq : %d\n", __LINE__ );

	       local_irq_disable();
		ret = mcsdl_download_binary_data();	
        	printk("[TSP] enable_irq : %d\n", __LINE__ );
	      local_irq_enable();

		if(ret > 0)
		{
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}
	}	
#endif // SET_DOWNLOAD_BY_GPIO

      ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		printk(KERN_ERR "melfas_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = "sec_touchscreen ";

	ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
	
	ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
	ts->input_dev->keybit[BIT_WORD(KEY_HOME)] |= BIT_MASK(KEY_HOME);
	ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);		
	ts->input_dev->keybit[BIT_WORD(KEY_SEARCH)] |= BIT_MASK(KEY_SEARCH);	

	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(EV_ABS,  ts->input_dev->evbit);
	ts->input_dev->evbit[0] =  BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);	

   #if 0
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(EV_ABS, ts->input_dev->evbit);
    #endif

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, MAX_X, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, MAX_Y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);

	set_bit(EV_SYN, ts->input_dev->evbit); 
	set_bit(EV_KEY, ts->input_dev->evbit);	
    
	/* ts->input_dev->name = ts->keypad_info->name; */
	ret = input_register_device(ts->input_dev);
	if (ret) {
		printk(KERN_ERR "melfas_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}

    	printk("[TSP] %s, irq=%d\n", __func__, client->irq );

    	gpio_request(TSP_INT, "ts_irq");
	gpio_direction_input(TSP_INT);
	bcm_gpio_pull_up(TSP_INT, true);
	bcm_gpio_pull_up_down_enable(TSP_INT, true);
	set_irq_type(GPIO_TO_IRQ(TSP_INT), IRQF_TRIGGER_FALLING);
    

    if (client->irq) {
		ret = request_irq(client->irq, melfas_ts_irq_handler, IRQF_TRIGGER_FALLING, client->name, ts);

		if (ret == 0)
			ts->use_irq = 1;
		else
			dev_err(&client->dev, "request_irq failed\n");
	}

#if 1
#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = melfas_ts_early_suspend;
	ts->early_suspend.resume = melfas_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif
#endif

	if(ts ==NULL)
       printk("[TSP] melfas_ts_init_read : TS NULL\n");

	buf[0] = TS_READ_START_ADDR;

	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		ret = i2c_master_send(ts->client, buf, 1);

		if(ret >=0)
		{
			ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);

			if(ret >=0)
			{
				break; // i2c success
			}
		}
	}


	return 0;

#if 0
err_request_irq:
	printk(KERN_ERR "melfas-ts: err_request_irq failed\n");
	free_irq(client->irq, ts);
#endif
err_input_register_device_failed:
	printk(KERN_ERR "melfas-ts: err_input_register_device failed\n");
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
	printk(KERN_ERR "melfas-ts: err_input_dev_alloc failed\n");
err_alloc_data_failed:
	printk(KERN_ERR "melfas-ts: err_alloc_data failed_\n");	
#if 0
err_detect_failed:
	printk(KERN_ERR "melfas-ts: err_detect failed\n");
	kfree(ts);
err_check_functionality_failed:
	printk(KERN_ERR "melfas-ts: err_check_functionality failed_\n");
#endif
	return ret;}

static int melfas_ts_remove(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ts->early_suspend);
	if (ts->use_irq)
		free_irq(client->irq, ts);

	//else
	//	hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	touch_ctrl_regulator(TOUCH_OFF);

	printk("[TSP] %s, %d\n", __func__, __LINE__ );

	if (ts->use_irq)
	{
		disable_irq(client->irq);
	}

        TSP_forced_release_forkey(ts);

	return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{

	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	printk("[TSP] %s, %d\n", __func__, __LINE__ );

      touch_ctrl_regulator(TOUCH_ON);
	msleep(40);

      enable_irq(client->irq); // scl wave
      melfas_init_panel(ts);
 
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
	struct melfas_ts_data *ts;
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	melfas_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
	struct melfas_ts_data *ts;
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	melfas_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id melfas_ts_id[] = {
	{ MELFAS_TS_NAME, 0 },
	{ }
};

static struct i2c_driver melfas_ts_driver = {
	.probe		= melfas_ts_probe,
	.remove		= melfas_ts_remove,
#if 1
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= melfas_ts_suspend,
	.resume		= melfas_ts_resume,
#endif
#endif
	.id_table	= melfas_ts_id,
	.driver = {
		.name	= MELFAS_TS_NAME,
	},
};

static int __devinit melfas_ts_init(void)
{

	melfas_wq = create_workqueue("melfas_wq");
	if (!melfas_wq)
		return -ENOMEM;

	touch_regulator = regulator_get(NULL,"touch_vcc");

	return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
	if (touch_regulator) 
	{
       	 regulator_put(touch_regulator);
		 touch_regulator = NULL;
    	}
	
	i2c_del_driver(&melfas_ts_driver);
	if (melfas_wq)
		destroy_workqueue(melfas_wq);
}

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{
      	uint8_t buf_firmware;
       int ret;
       
	printk("[TSP] %s\n",__func__);

	buf_firmware = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts_global->client, &buf_firmware, 1);
	if(ret < 0)
	{
		printk(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);			
	}

	ret = i2c_master_recv(ts_global->client, &buf_firmware, 1);
	if(ret < 0)
	{
		printk(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);			
	}
	printk("[TSP] ver firmware=[%d]\n", buf_firmware);

    	sprintf(buf, "%d\n", buf_firmware);

	return sprintf(buf, "%s", buf );
}

/* firmware - update */
static ssize_t firmware_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);
	firmware_ret_val = -1;
	printk("[TSP] firmware_store  valuie : %d\n",value);
	if ( value == 0 )
	{
		printk("[TSP] Firmware update start!!\n" );

		firm_update( );
		return size;
	}

	return size;
}

static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	printk("[TSP] %s!\n", __func__);

	return sprintf(buf, "%d", firmware_ret_val );
}

static ssize_t firmware_ret_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	printk("[TSP] %s, operate nothing!\n", __func__);

	return size;
}


int firm_update( void )
{
	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);

	disable_irq(tsp_irq);
	local_irq_disable();

	firmware_ret_val = mcsdl_download_binary_data();	

	msleep(1000);
	if( firmware_ret_val )
		printk(KERN_INFO "[TSP] %s success, %d\n", __func__, __LINE__);
	else	
		printk(KERN_INFO "[TSP] %s fail, %d\n", __func__, __LINE__);

	local_irq_enable();

	enable_irq(tsp_irq);

	return 0;
} 

module_init(melfas_ts_init);
module_exit(melfas_ts_exit);

MODULE_DESCRIPTION("Synaptics Touchscreen Driver");
MODULE_LICENSE("GPL");
