/* drivers/input/keyboard/synaptics_i2c_rmi.c
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
#include <linux/synaptics_i2c_rmi.h>
#include <linux/regulator/consumer.h>

#define FIRM_TEST 0
#define DEBUG_INPUT_REPORT

/* firmware - update */
#if FIRM_TEST
#include "./CYPRESS_I2C_ISSP/issp_define.h"
#endif

#include <linux/firmware.h>
#include <linux/uaccess.h> 
/* firmware - update */

#define MAX_X  320 
#define MAX_Y 240 
#define TSP_INT 30
#define TSP_SDA 27
#define TSP_SCL 26

#define TOUCH_ON 1
#define TOUCH_OFF 0

#define TRUE    1
#define FALSE    0

#define I2C_RETRY_CNT	2

#define ADDR_TSP_VENDER_ID	0x07
#define ADDR_TSP_HW_VER		0x08
#define ADDR_TSP_SW_VER		0x09

//#define __TOUCH_DEBUG__ 0
#define USE_THREADED_IRQ	1

static struct regulator *touch_regulator=NULL;

static struct workqueue_struct *synaptics_wq;
static struct workqueue_struct *check_ic_wq;

struct synaptics_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	struct hrtimer timer;
	struct work_struct  work;
	struct work_struct  work_timer;
	struct early_suspend early_suspend;
};

struct synaptics_ts_data *ts_global;

struct touch_trace_data {
	uint32_t time;
	uint16_t x;
	uint16_t y;
	uint8_t press;
};
#define MAX_TOUCH_TRACE_NUMBER	10000
static int touch_trace_index = 0;
static struct touch_trace_data touch_trace_info[MAX_TOUCH_TRACE_NUMBER];

int tsp_i2c_read(u8 reg, unsigned char *rbuf, int buf_size);

/* firmware - update */
static int firmware_ret_val = -1;
static int HW_ver = -1;
unsigned char now_tst200_update_luisa = 0;
unsigned char tsp_special_update = 0;

/* touch information*/
unsigned char touch_vendor_id = 0;
unsigned char touch_hw_ver = 0;
unsigned char touch_sw_ver = 0;

#define TSP_VENDER_ID	0xF0
#define TSP_HW_VER1		0x03
#define TSP_SW_VER1		0x0B

#define TSP_HW_VER2		0x04
#define TSP_SW_VER2		0x09

#define TSP_HW_VER3		0x05
#define TSP_SW_VER3		0x04

int tsp_irq_num = 0;
int tsp_workqueue_num = 0;
int tsp_threadedirq_num = 0;

int g_touch_info_x = 0;
int g_touch_info_y = 0;
int g_touch_info_press = 0;

#if FIRM_TEST
unsigned char g_pTouchFirmware[TSP_TOTAL_LINES*TSP_LINE_LENGTH];
unsigned int g_FirmwareImageSize = 0;

static void issp_request_firmware(char* update_file_name);
#endif

int firm_update( void );
extern int cypress_update( int );

#if FIRM_TEST
unsigned char pSocData[]= {
	"fdjkjklijk"
		//#include "./CYPRESS_I2C_ISSP/Europa_ver02_100218" 	
};
#endif
/* firmware - update */

/* sys fs */
struct class *touch_class;
EXPORT_SYMBOL(touch_class);
struct device *firmware_dev;
EXPORT_SYMBOL(firmware_dev);

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_store( struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_ret_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static ssize_t raw_show_tst200(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t raw_store_tst200(struct device *dev, struct device_attribute *attr, char *buf, size_t size);
static ssize_t raw_enable_tst200(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t raw_disable_tst200(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t read_rawdata(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t read_difference(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t rawdata_pass_fail_tst200(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t touch15test_enable_tst200(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t touch15test_disable_tst200(struct device *dev, struct device_attribute *attr, char *buf);

static DEVICE_ATTR(firmware	, 0444, firmware_show, firmware_store);
static DEVICE_ATTR(firmware_ret	, 0444, firmware_ret_show, firmware_ret_store);
//static DEVICE_ATTR(firmware	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_show, firmware_store);
//static DEVICE_ATTR(firmware_ret	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_ret_show, firmware_ret_store);
static DEVICE_ATTR(raw, 0444, raw_show_tst200, NULL) ;
static DEVICE_ATTR(raw_enable, 0444, raw_enable_tst200, NULL) ;
static DEVICE_ATTR(raw_disable, 0444, raw_disable_tst200, NULL) ;
static DEVICE_ATTR(rawdata_read, 0444, read_rawdata, NULL) ;
static DEVICE_ATTR(difference_read, 0444, read_difference, NULL) ;
static DEVICE_ATTR(raw_value, 0444, rawdata_pass_fail_tst200, NULL) ;
static DEVICE_ATTR(touch15test_enable, 0444, touch15test_enable_tst200, NULL) ;
static DEVICE_ATTR(touch15test_disable, 0444, touch15test_disable_tst200, NULL) ;


static int tsp_testmode = 0;
static int prev_wdog_val = -1;
static int tsp_irq_operation = 0;
static unsigned int touch_present = 0;


/* sys fs */

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h);
static void synaptics_ts_late_resume(struct early_suspend *h);
#endif

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int set_irq_type(unsigned int irq, unsigned int type);

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

int tsp_reset( void )
{
	int ret=1;

      #if defined(__TOUCH_DEBUG__)
	printk("[TSP] %s, %d\n", __func__, __LINE__ );
      #endif 
      
	if (ts_global->use_irq)
	{
		disable_irq(ts_global->client->irq);
	}

	touch_ctrl_regulator(0);

	gpio_direction_output( TSP_SCL , 0 ); 
	gpio_direction_output( TSP_SDA , 0 ); 
	//gpio_direction_output( TSP_INT , 0 ); 

	msleep(200);

	gpio_direction_output( TSP_SCL , 1 ); 
	gpio_direction_output( TSP_SDA , 1 ); 
	//gpio_direction_output( TSP_INT , 1 ); 

	touch_ctrl_regulator(1);

	msleep(10);

	enable_irq(ts_global->client->irq);

	return ret;
}

#if USE_THREADED_IRQ
static irqreturn_t synaptics_ts_work_func(int irq, void *dev_id)
#else
static void synaptics_ts_work_func(struct work_struct *work)
#endif
{
	int i, ret=0;
	uint8_t addr[1];
	uint8_t buf[11];
	//uint8_t i2c_addr = 0x02;
	uint8_t i2c_addr = 0x00;
	uint16_t x=0;
	uint16_t y=0;
	uint16_t z=1;
	int finger = 0;
	uint8_t i2c_reg_info;
	uint8_t InputType =0 ;
	int v_touch_pressed=0;
	int press;
#ifdef DEBUG_INPUT_REPORT
	static int prev_finger = 0;
#endif

#if USE_THREADED_IRQ
    if(tsp_testmode)
		return IRQ_HANDLED;

	struct synaptics_ts_data *ts = dev_id;
#else
	struct synaptics_ts_data *ts = container_of(work, struct synaptics_ts_data, work);
#endif


#if USE_THREADED_IRQ
	//printk("[TSP]tsp_threadedirq_num = %d\n", tsp_threadedirq_num++ );///
#else
	//printk("[TSP]tsp_workqueue_num = %d\n", tsp_workqueue_num++ );///
#endif


      #if defined(__TOUCH_DEBUG__)
	printk("[TSP] %s, %d\n", __func__, __LINE__ );
      #endif
	//ret = tsp_i2c_read( i2c_addr, buf, sizeof(buf));
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		addr[0] = 0x0; //address
		ret = i2c_master_send(ts_global->client, addr, 1);

		if (ret >= 0)
		{
			ret = i2c_master_recv(ts_global->client, buf, sizeof(buf));
			if (ret >= 0)
				break; // i2c success
		}
	}

	if (ret < 0) 
	{
		printk("[TSP] i2c failed : ret=%d, ln=%d\n",ret, __LINE__);
		goto work_func_out;
	}

     #if defined(__TOUCH_DEBUG__)
	printk("[TSP] buf[0]:%d, buf[1]:%d, buf[2]=%d, buf[3]=%d, buf[4]=%d, buf[5]=%d\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	printk("[TSP] buf[6]:%d, buf[7]:%d, buf[8]=%d, buf[9]=%d, buf[10]=%d, buf[11]=%d\n", buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
    #endif
    
        i2c_reg_info = buf[3];
    y = buf[6]; 
    x = buf[4]; 

        if( i2c_reg_info == 1)	/* 6bit mask , 1이면 256이 넘은것을 의미*/
    	x = x + 256;

        press = buf[5];

        if(press)
		press = 1;
	else
		press = 0;

	input_report_abs(ts->input_dev, ABS_X, x);
	input_report_abs(ts->input_dev, ABS_Y, y);		
	input_report_abs(ts->input_dev, ABS_PRESSURE, 1);
	input_report_key(ts->input_dev, BTN_TOUCH, press);
	input_sync(ts->input_dev);    

	/* ++ due to forced release ++ */
	g_touch_info_x = x;
	g_touch_info_y = y;
	g_touch_info_press = press;
	/* -- due to forced release -- */

	/* ++ due to touch trace ++ */
	touch_trace_info[touch_trace_index].time = jiffies;
	touch_trace_info[touch_trace_index].x = x;
	touch_trace_info[touch_trace_index].y = y;
	touch_trace_info[touch_trace_index].press = press;

	if(touch_trace_index >= MAX_TOUCH_TRACE_NUMBER)
	{
		touch_trace_index = 0;
	}
	/* -- due to touch trace -- */
	
     #if defined(__TOUCH_DEBUG__)
	printk("[TSP] x=%d, y=%d, press=%d\n", x, y, press );
    #endif
    
#ifdef DEBUG_INPUT_REPORT
	if ( finger ^ prev_finger )
	{
		printk("[TSP] x=%d, y=%d\n", x, y );
		prev_finger = finger;
	}
#endif

work_func_out:
	if (ts->use_irq)
	{
		#if defined(__TOUCH_DEBUG__)
		printk("[TSP] synaptics_ts_work_func==>enable_irq\n");
		#endif

		#if USE_THREADED_IRQ

		#else
		enable_irq(ts->client->irq);
		#endif
	}

	tsp_irq_operation = 0;
		
	#if USE_THREADED_IRQ
	return IRQ_HANDLED;
	#else

	#endif
}

#if 0
int tsp_i2c_read(u8 reg, unsigned char *rbuf, int buf_size)
{
	int ret=-1;
	struct i2c_msg rmsg;
	uint8_t start_reg;

        	rmsg.addr = ts_global->client->addr;
		rmsg.flags = I2C_M_RD;
		rmsg.len = buf_size;
		rmsg.buf = rbuf;
		ret = i2c_transfer(ts_global->client->adapter, &rmsg, 1 );
	if( ret < 0 )
		{
		printk("[TSP] Error code : %d\n", __LINE__ );
		printk("[TSP] reset ret=%d\n", tsp_reset( ) );
	}

	return ret;
}
#else
int tsp_i2c_read(u8 reg, unsigned char *rbuf, int buf_size)
{
	int i, ret=-1;
	struct i2c_msg rmsg;
	uint8_t start_reg;

	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		rmsg.addr = ts_global->client->addr;
		rmsg.flags = 0;//I2C_M_WR;
		rmsg.len = 1;
		rmsg.buf = &start_reg;
		start_reg = reg;
		
		ret = i2c_transfer(ts_global->client->adapter, &rmsg, 1);

		if(ret >= 0) 
		{
			rmsg.flags = I2C_M_RD;
			rmsg.len = buf_size;
			rmsg.buf = rbuf;
			ret = i2c_transfer(ts_global->client->adapter, &rmsg, 1 );

			if (ret >= 0)
				break; // i2c success
		}

		if( i == (I2C_RETRY_CNT - 1) )
		{
			printk("[TSP] Error code : %d, %d\n", __LINE__, ret );
		}
	}

	return ret;
}
#endif

void TSP_forced_release_forkey(struct synaptics_ts_data *ts)
{
	input_report_abs(ts->input_dev, ABS_X, g_touch_info_x);
	input_report_abs(ts->input_dev, ABS_Y, g_touch_info_y);		
	input_report_abs(ts->input_dev, ABS_PRESSURE, 1);
	input_report_key(ts->input_dev, BTN_TOUCH, 0);
	input_sync(ts->input_dev);    

	printk("[TSP] force release\n");
}

static irqreturn_t synaptics_ts_irq_handler(int irq, void *dev_id)
{
	if(tsp_testmode)
		return IRQ_WAKE_THREAD;

	struct synaptics_ts_data *ts = dev_id;
	
	tsp_irq_operation = 1;
	
	#if defined(__TOUCH_DEBUG__)
	//printk("[TSP] %s, %d\n", __func__, __LINE__ );
	#endif
	
	//printk("[TSP]tsp_irq_num = %d\n", tsp_irq_num++ );///

	#if USE_THREADED_IRQ

	#else
	disable_irq_nosync(ts->client->irq);
	#endif

	#if USE_THREADED_IRQ
	return IRQ_WAKE_THREAD;
	#else
	queue_work(synaptics_wq, &ts->work);
	return IRQ_HANDLED;
	#endif
}

static void check_ic_work_func(struct work_struct *work_timer)
{
	int ret=0;
	uint8_t buf_esd[1];
	uint8_t wdog_val[1];

	struct synaptics_ts_data *ts = container_of(work_timer, struct synaptics_ts_data, work_timer);

	//printk("[TSP] %s, %d\n", __func__, __LINE__ );

	buf_esd[0] = 0x02;
	wdog_val[0] = 1;
	   
	if((tsp_testmode == 0) && (tsp_irq_operation == 0 && (now_tst200_update_luisa == 0)))
	{
		ret = i2c_master_send(ts->client, buf_esd, 1);
			
		if(ret >=0)
		{
			ret = i2c_master_recv(ts->client, wdog_val, 1);
			//printk("[TSP] prev_wdog_val = %d, wdog_val = %d\n", prev_wdog_val, wdog_val[0] );
			
			if(wdog_val[0] == (uint8_t)prev_wdog_val)
			{
				//printk("[TSP] %s tsp_reset counter = %x, prev = %x\n", __func__, wdog_val[0], (uint8_t)prev_wdog_val);
				tsp_reset();
				prev_wdog_val = -1;
			}
			else
			{
				prev_wdog_val = wdog_val[0];
			}			
		}
		else//if(ret < 0)
		{
			tsp_reset();
			printk(KERN_ERR "[TSP] silabs_ts_work_func : i2c_master_send [%d]\n", ret);			
		}
	}
}

static enum hrtimer_restart synaptics_watchdog_timer_func(struct hrtimer *timer)
{
	//printk("[TSP] %s, %d\n", __func__, __LINE__ );

	queue_work(check_ic_wq, &ts_global->work_timer);
	hrtimer_start(&ts_global->timer, ktime_set(2, 0), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

static int synaptics_ts_probe(
		struct i2c_client *client, const struct i2c_device_id *id)
{
	struct synaptics_ts_data *ts;
	int i, ret = 0;
	uint8_t i2c_addr = 0;
	uint8_t addr[1];
	uint8_t buf[11] = {0,};
	int ret_temp=0;
	uint8_t buf_temp[3];
	//uint8_t i2c_addr_temp = 0x02;

	printk("[TSP] %s, %d\n", __func__, __LINE__ );

	touch_ctrl_regulator(TOUCH_ON);
	msleep(100);	
	touch_ctrl_regulator(TOUCH_OFF);
	msleep(200);
	touch_ctrl_regulator(TOUCH_ON);
	msleep(100);

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	#if USE_THREADED_IRQ

	#else
	INIT_WORK(&ts->work, synaptics_ts_work_func);
	#endif
	
	INIT_WORK(&ts->work_timer, check_ic_work_func);
	ts->client = client;
	i2c_set_clientdata(client, ts);

	ts_global = ts;

	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		printk(KERN_ERR "synaptics_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = "sec_touchscreen";

	ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
	ts->input_dev->keybit[BIT_WORD(KEY_POWER)] |= BIT_MASK(KEY_POWER);
	

	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	ts->input_dev->evbit[0] =  BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);	

	printk(KERN_INFO "synaptics_ts_probe: max_x: 240, max_y: 320\n");

	input_set_abs_params(ts->input_dev, ABS_X, 0, MAX_X, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, 0, MAX_Y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_TOOL_WIDTH, 0, 15, 0, 0);

	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);

	hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ts->timer.function = synaptics_watchdog_timer_func;

	/* Check point - i2c check - start */
	//ret = tsp_i2c_read( i2c_addr, buf, sizeof(buf));
	for (i = 0; i < 1; i++)
	{
		printk("[TSP] %s, %d, send\n", __func__, __LINE__ );
		addr[0] = ADDR_TSP_VENDER_ID; //address
		ret = i2c_master_send(ts_global->client, addr, 1);

		if (ret >= 0)
		{
			printk("[TSP] %s, %d, receive\n", __func__, __LINE__ );
			ret = i2c_master_recv(ts_global->client, buf, 3);
			if (ret >= 0)
				break; // i2c success
		}

		printk("[TSP] %s, %d, fail\n", __func__, __LINE__ );
	}

	touch_vendor_id = buf[0];
	touch_hw_ver = buf[1];
	touch_sw_ver = buf[2];
	printk("[TSP] %s:%d, ver tsp=%x, HW=%x, SW=%x\n", __func__,__LINE__, touch_vendor_id, touch_hw_ver, touch_sw_ver);

	HW_ver = touch_hw_ver;
	
	if(ret >= 0)
	{
		touch_present = 1;
		hrtimer_start(&ts->timer, ktime_set(2, 0), HRTIMER_MODE_REL);
	}
	else//if(ret < 0) 
	{
		printk(KERN_ERR "i2c_transfer failed\n");
		printk("[TSP] %s, ln:%d, Failed to register TSP!!!\n\tcheck the i2c line!!!, ret=%d\n", __func__,__LINE__, ret);
 		goto err_check_functionality_failed;
	}
	/* Check point - i2c check - end */

	/* ts->input_dev->name = ts->keypad_info->name; */
	ret = input_register_device(ts->input_dev);
	if (ret) {
		printk(KERN_ERR "synaptics_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}
	
	printk("[TSP] %s, irq=%d\n", __func__, client->irq );
	if (client->irq) {
		#if USE_THREADED_IRQ
		ret = request_threaded_irq(client->irq, synaptics_ts_irq_handler, synaptics_ts_work_func, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, client->name, ts);
		#else
		ret = request_irq(client->irq, synaptics_ts_irq_handler,/* IRQF_TRIGGER_RISING |*/ IRQF_TRIGGER_FALLING , client->name, ts);
		#endif
		
		if (ret == 0) 
			ts->use_irq = 1;
		else
			dev_err(&client->dev, "request_irq failed\n");
	}

	//	hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	//	ts->timer.function = synaptics_ts_timer_func;

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN+1;
	ts->early_suspend.suspend = synaptics_ts_early_suspend;
	ts->early_suspend.resume = synaptics_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	printk(KERN_INFO "synaptics_ts_probe: Start touchscreen %s in %s mode\n", ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");

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
	if (device_create_file(firmware_dev, &dev_attr_raw) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_raw_enable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_enable.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_raw_disable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_disable.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_rawdata_read) < 0)
	   pr_err("Failed to create device file(%s)!\n", dev_attr_rawdata_read.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_difference_read) < 0)
	   pr_err("Failed to create device file(%s)!\n", dev_attr_difference_read.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_raw_value) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_value.attr.name);	
	if (device_create_file(firmware_dev, &dev_attr_touch15test_enable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_touch15test_enable.attr.name);	
	if (device_create_file(firmware_dev, &dev_attr_touch15test_disable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_touch15test_disable.attr.name);	
	/* sys fs */

#if 0//SET_DOWNLOAD_BY_GPIO
	#if 0
	if (/*(touch_vendor_id == TSP_VENDER_ID)&&*/( touch_hw_ver == TSP_HW_VER1)&&(touch_sw_ver < TSP_SW_VER1))
	{ 
		printk("[TSP] Firmware update start!!\n" );
		now_tst200_update_luisa = 1;
		firm_update();
		now_tst200_update_luisa = 0;
	}
	else if (/*(touch_vendor_id == TSP_VENDER_ID)&&*/( touch_hw_ver == TSP_HW_VER2)&&(touch_sw_ver < TSP_SW_VER2))
	{ 
		printk("[TSP] Firmware update start!!\n" );
		now_tst200_update_luisa = 1;
		firm_update();
		now_tst200_update_luisa = 0;	
	}	
	#endif
	{
		HW_ver = 0x03;

		printk("[TSP] Firmware update start!!\n" );
		now_tst200_update_luisa = 1;
		firm_update();
		now_tst200_update_luisa = 0;		
	}
 #endif // SET_DOWNLOAD_BY_GPIO


	return 0;

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
	kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:
	return ret;
}

static int synaptics_ts_remove(struct i2c_client *client)
{
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ts->early_suspend);
	if (ts->use_irq)
		free_irq(client->irq, ts);

	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int synaptics_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
    
	printk("[TSP] %s+\n", __func__ );

	if(now_tst200_update_luisa == 0)
	{
		if (ts->use_irq)
		{
			disable_irq(client->irq);
		}

		if(touch_present == 1)
			hrtimer_cancel(&ts->timer);
		
		gpio_direction_output( TSP_INT , 0 );
		gpio_direction_output( TSP_SCL , 0 ); 
		gpio_direction_output( TSP_SDA , 0 ); 

		bcm_gpio_pull_up(TSP_INT, false);
		bcm_gpio_pull_up_down_enable(TSP_INT, true);

		#if USE_THREADED_IRQ
		
		#else
		ret = cancel_work_sync(&ts->work);
		
		if (ret && ts->use_irq) /* if work was pending disable-count is now 2 */
		{
			enable_irq(client->irq);
		}
		#endif

		msleep(20);

		touch_ctrl_regulator(TOUCH_OFF);

		if(g_touch_info_press == 1)
		{
			TSP_forced_release_forkey(ts);
		}
	}

	printk("[TSP] %s-\n", __func__ );
	return 0;
}

static int synaptics_ts_resume(struct i2c_client *client)
{
	int ret;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
	uint8_t i2c_addr = 0x07;
	uint8_t buf[1];

	printk("[TSP] %s+\n", __func__ );

	if(now_tst200_update_luisa == 0)
	{
		gpio_direction_output( TSP_SCL , 1 ); 
		gpio_direction_output( TSP_SDA , 1 ); 
		//gpio_direction_output( TSP_INT , 1 ); 

		gpio_direction_input(TSP_INT);
		bcm_gpio_pull_up_down_enable(TSP_INT, false);

		touch_ctrl_regulator(TOUCH_ON);

		msleep(100);

		prev_wdog_val = -1;
		
		if(touch_present == 1)
			hrtimer_start(&ts->timer, ktime_set(4, 0), HRTIMER_MODE_REL);

       enable_irq(client->irq);
	}

	printk("[TSP] %s-\n", __func__ );
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	synaptics_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void synaptics_ts_late_resume(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	synaptics_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id synaptics_ts_id[] = {
	{ SYNAPTICS_I2C_RMI_NAME, 1 },
	{ }
};

static struct i2c_driver synaptics_ts_driver = {
	.probe		= synaptics_ts_probe,
	.remove		= synaptics_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= synaptics_ts_suspend,
	.resume		= synaptics_ts_resume,
#endif
	.id_table	= synaptics_ts_id,
	.driver = {
		.name	= SYNAPTICS_I2C_RMI_NAME,
	},
};

static int __devinit synaptics_ts_init(void)
{
	printk("[TSP] %s\n", __func__ );

	gpio_request(TSP_INT, "ts_irq");
	gpio_direction_input(TSP_INT);
	//bcm_gpio_pull_up(TSP_INT, true);
	//bcm_gpio_pull_up_down_enable(TSP_INT, true);
	set_irq_type(GPIO_TO_IRQ(TSP_INT), IRQF_TRIGGER_FALLING);
    
	//disable BB internal pulls for touch int, scl, sda pin
    bcm_gpio_pull_up_down_enable(TSP_INT, 0);
	bcm_gpio_pull_up_down_enable(TSP_SCL, 0);
	bcm_gpio_pull_up_down_enable(TSP_SDA, 0);

	#if USE_THREADED_IRQ

	#else
	synaptics_wq = create_singlethread_workqueue("synaptics_wq");
	if (!synaptics_wq)
		return -ENOMEM;
	#endif

	check_ic_wq = create_singlethread_workqueue("check_ic_wq");	

	if (!check_ic_wq)
		return -ENOMEM;

	touch_regulator = regulator_get(NULL,"touch_vcc");
	return i2c_add_driver(&synaptics_ts_driver);
}

static void __exit synaptics_ts_exit(void)
{
	if (touch_regulator) 
	{
		regulator_put(touch_regulator);
		touch_regulator = NULL;
	}
	
	i2c_del_driver(&synaptics_ts_driver);

	#if USE_THREADED_IRQ

	#else	
	if (synaptics_wq)
		destroy_workqueue(synaptics_wq);
	#endif

	if (check_ic_wq)
		destroy_workqueue(check_ic_wq);	
}

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	//uint8_t i2c_addr = 0x06;
	uint8_t i2c_addr = 0x00;
	uint8_t buf_tmp[11] = {0};
	int phone_ver = 0;

	printk("[TSP] %s\n",__func__);

	tsp_i2c_read( i2c_addr, buf_tmp, sizeof(buf_tmp));

	touch_vendor_id = buf_tmp[7];
	touch_hw_ver = buf_tmp[8];
	touch_sw_ver = buf_tmp[9];
	printk("[TSP] %s:%d, ver tsp=%x, HW=%x, SW=%x\n", __func__,__LINE__, touch_vendor_id, touch_hw_ver, touch_sw_ver);

	HW_ver = touch_hw_ver;

	 		
	if ( HW_ver == 2 )//touch_hw_ver
	{
		phone_ver = 3;  /* change this value if New firmware be released */ 	
	}
	else if ( HW_ver == 3 )
	{
		phone_ver = TSP_SW_VER1;  /* change this value if New firmware be released */ 	
	}
	else if ( HW_ver == 4 )
	{
		phone_ver = TSP_SW_VER2;  /* change this value if New firmware be released */ 	
	}
	else if ( HW_ver == 5 )
	{
		phone_ver = TSP_SW_VER3;  /* change this value if New firmware be released */ 	
	}	
	else
	{
		printk("[TSP] %s:%d,HW_ver is wrong!!\n", __func__,__LINE__ );
	}

	/* below protocol is defined with App. ( juhwan.jeong@samsung.com )
		The TSP Driver report like XY as decimal.
		The X is the Firmware version what phone has.
		The Y is the Firmware version what TSP has. */

	//sprintf(buf, "%d\n", phone_ver + buf_tmp[1]+(buf_tmp[0]*100) );
	sprintf(buf, "%x\n", 0x1000000 + (touch_hw_ver*0x10000) + (touch_sw_ver*0x100) +  phone_ver);
	//sprintf(buf, "%d\n", 1000000 + (buf_tmp[8]*10000) + (buf_tmp[9]*100) +  phone_ver);

	return sprintf(buf, "%s", buf );
}

/* firmware - update */
static ssize_t firmware_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	char *after;

	uint8_t i2c_addr = 0x00;
	uint8_t buf_tmp[11] = {0};

	unsigned long value = simple_strtoul(buf, &after, 10);	
	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);
	firmware_ret_val = -1;


	tsp_i2c_read( i2c_addr, buf_tmp, sizeof(buf_tmp));

	touch_vendor_id = buf_tmp[7];
	touch_hw_ver = buf_tmp[8];
	touch_sw_ver = buf_tmp[9];
	printk("[TSP] %s:%d, ver tsp=%x, HW=%x, SW=%x\n", __func__,__LINE__, touch_vendor_id, touch_hw_ver, touch_sw_ver);

	HW_ver = touch_hw_ver;

	if ( value == 1 )
	{
		printk("[TSP] Firmware update start!!\n" );

		now_tst200_update_luisa = 1;
		firm_update( );
		now_tst200_update_luisa = 0;
#if FIRM_TEST
		printk("[TSP] start update cypress touch firmware !!\n");
		g_FirmwareImageSize = CYPRESS_FIRMWARE_IMAGE_SIZE;

		if(g_pTouchFirmware == NULL)
		{
			printk("[TSP][ERROR] %s() kmalloc fail !! \n", __FUNCTION__);
			return -1;
		}


		/* ready for firmware code */
		size = issp_request_firmware("touch.hex");

		/* firmware update */
		//	issp_upgrade();

		g_FirmwareImageSize = 0;

		// step.1 power off/on

		// step.2 enable irq


#endif
		return size;
	}
	else if( value == 2 )
	{
		printk("[TSP] Special Firmware update start!!\n" );

		tsp_special_update = 1;

		now_tst200_update_luisa = 1;
		firm_update( );
		now_tst200_update_luisa = 0;	
	}

	return size;
}

static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	printk("[TSP] %s!\n", __func__);

	printk("[TSP] %s, firmware_ret_val = %d\n", __func__, firmware_ret_val);
	now_tst200_update_luisa = 1;
	firm_update();
	now_tst200_update_luisa = 0;
	
	return sprintf(buf, "%d", firmware_ret_val );
}

static ssize_t firmware_ret_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	printk("[TSP] %s, operate nothing!\n", __func__);

	return size;
}


#define TST200_RET_SUCCESS 0x00
int sv_tch_firmware_update = 0;
int firm_update( void )
{
	uint8_t update_num;

	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);

	printk("[TSP] disable_irq : %d\n", __LINE__ );
	disable_irq(ts_global->client->irq);
	local_irq_disable();

	for(update_num = 1; update_num <= 10 ; update_num++)
	{
		sv_tch_firmware_update = cypress_update(HW_ver);

		if(sv_tch_firmware_update == TST200_RET_SUCCESS)
		{
			firmware_ret_val = 1; //SUCCESS
			printk( "[TSP] %s, %d : TST200 firmware update SUCCESS !!\n", __func__, __LINE__);
			break;
		}
		else
		{
			printk( "[TSP] %s, %d : TST200 firmware update RETRY !!\n", __func__, __LINE__);
			if(update_num == 10)
			{
				firmware_ret_val = 0; //FAIL
			printk( "[TSP] %s, %d : TST200 firmware update FAIL !!\n", __func__, __LINE__);
			}
				//OemSysHWReset();
///				xassert(0, sv_tch_firmware_update);
		}
	}

#if 0	
	// TEST

	if( HW_ver==1 || HW_ver==2 )
		//firmware_ret_val = cypress_update( HW_ver );
            firmware_ret_val = 0;
	else	
	{
		printk(KERN_INFO "[TSP] %s, %d cypress_update blocked, HW ver=%d\n", __func__, __LINE__, HW_ver);
		firmware_ret_val = 0; // Fail
	}

	if( firmware_ret_val )
		printk(KERN_INFO "[TSP] %s success, %d\n", __func__, __LINE__);
	else	
		printk(KERN_INFO "[TSP] %s fail, %d\n", __func__, __LINE__);
#endif

	local_irq_enable();
	printk("[TSP] enable_irq : %d\n", __LINE__ );
	enable_irq(ts_global->client->irq);

	return 0;
} 

#if FIRM_TEST
static void issp_request_firmware(char* update_file_name)
{
	int idx_src = 0;
	int idx_dst = 0;
	int line_no = 0;
	int dummy_no = 0;
	char buf[2];
	int ret = 0;

	struct device *dev = &ts_global->input_dev->dev;	
	const struct firmware * fw_entry;

	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);
	printk("[TSP] firmware file name : %s\n", update_file_name);

	ret = request_firmware(&fw_entry, update_file_name, dev);
	if ( ret )
	{
		printk("[TSP] request_firmware fail, ln=%d\n", ret );
		return ;
	}
	else
	{
		printk("[TSP] request_firmware success, ln=%d\n", ret );
		printk("[TSP][DEBUG] ret=%d, firmware size=%d\n", ret, fw_entry->size);
		printk("[TSP] %c %c %c %c %c\n", fw_entry->data[0], fw_entry->data[1], fw_entry->data[2], fw_entry->data[3], fw_entry->data[4]);
	}

	do {
		if(fw_entry->data[idx_src] == ':') // remove prefix
		{
			idx_src+=9;
			dummy_no++;

			if(dummy_no != line_no+1)
			{
				printk("[ERROR] Can not skip semicolon !! line_no(%d), dummy_no(%d)\n", line_no, dummy_no);
			}
		}
		else if(fw_entry->data[idx_src] == '\r') // return code
		{
			idx_src+=2; idx_dst--; line_no++;

			if( idx_dst > TSP_LINE_LENGTH*line_no)
			{
				printk("[ERROR] length buffer over error !! line_no(%d), idx_dst(%d)\n", line_no, idx_dst);
			}
		}
		else if(fw_entry->data[idx_src] == 0x0a) // return code
		{
			idx_src+=1; idx_dst--; line_no++;

			if( idx_dst > TSP_LINE_LENGTH*line_no)
			{
				printk("[ERROR] length buffer over error !! line_no(%d), idx_dst(%d)\n", line_no, idx_dst);
			}
		}
		else
		{
			sprintf(buf, "%c%c", fw_entry->data[idx_src], fw_entry->data[idx_src+1]);
			if(idx_dst > TSP_TOTAL_LINES*TSP_LINE_LENGTH)
			{
				printk("[ERROR] buffer over error !!  line_no(%d), idx_dst(%d)\n", line_no, idx_dst);
			}
			g_pTouchFirmware[idx_dst] = simple_strtol(buf, NULL, 16);
			idx_src+=2; idx_dst++;
		}
	} while ( line_no < TSP_TOTAL_LINES );

	release_firmware(fw_entry);
}
#endif

/* firmware - update */


static ssize_t raw_store_tst200(struct device *dev, struct device_attribute *attr, char *buf, size_t size)
{
	int i, ret;
	uint8_t buf1[2] = {0,};
	
	printk("[TSP] %s start. line : %d, \n", __func__,__LINE__);

	mdelay(300); 
	
	if(strncasecmp(buf, "start", 5) == 0)
	{
		for (i = 0; i < I2C_RETRY_CNT; i++)
		{
			buf1[0] = 0x00;//address
			buf1[1] = 0x70;//value
			ret = i2c_master_send(ts_global->client, buf1, 2);	//enter Inspection Mode

			if (ret >= 0)
				break; // i2c success
		}
		
		tsp_testmode = 1;
		printk("[TSP] %s start. line : %d, \n", __func__,__LINE__);
	}
	else if(strncasecmp(buf, "stop", 4) == 0)
	{
		for (i = 0; i < I2C_RETRY_CNT; i++)
		{	
			buf1[0] = 0x00;//address
			buf1[1] = 0x00;//value
			ret = i2c_master_send(ts_global->client, buf1, 2);	//exit Inspection Mode

			if (ret >= 0)
				break; // i2c success
		}

		tsp_testmode = 0;
		printk("[TSP] %s stop. line : %d, \n", __func__,__LINE__);
	}
	else
	{
		printk("[TSP] %s error-unknown command. line : %d, \n", __func__,__LINE__);
	}

	return size ;
}

static ssize_t raw_show_tst200(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);

	if(!tsp_testmode)
		return 0;
	
	int tst200_col_num = 9;	//0 ~ 8
	int tst200_row_num = 7;	//0 ~ 6

	int  written_bytes = 0 ;	/* & error check */

	uint8_t buf1[1]={0,};
	uint8_t buf2[32]={0,};

	uint16_t ref1[16]={0,};
	uint16_t ref2[16]={0,};

	int i,j,k;
	int ret;
	

	/////* Diff Value */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x2C;//address

		ret = i2c_master_send(ts_global->client, buf1, 1);
		if (ret >= 0)
		{
			ret = i2c_master_recv(ts_global->client, buf2, (tst200_col_num + tst200_row_num));

			if (ret >= 0)
				break; // i2c success
		}
	}


	printk("[TSP]");
	for(i = 0 ; i < (tst200_col_num + tst200_row_num) ; i++)
	{
		printk(" 0x%2x", buf2[i]);
	}
	printk("\n");
	
	for(i = 0 ; i < (tst200_col_num + tst200_row_num) ; i++)
	{
		ref2[i] = buf2[i];
		printk(" %d", ref2[i]);
	}
	printk("\n");


	/////* Raw Value */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x0C; //address

		ret = i2c_master_send(ts_global->client, buf1, 1);

		if (ret >= 0)
		{
			ret = i2c_master_recv(ts_global->client, buf2, (tst200_col_num + tst200_row_num)*2);

			if (ret >= 0)
				break; // i2c success
		}
	}

	printk("[TSP]");
	for(i = 0 ; i < (tst200_col_num + tst200_row_num)*2 ; i++)
	{
		printk(" 0x%2x", buf2[i]);
	}
	printk("\n");
	
	for(i = 0 ; i < (tst200_col_num + tst200_row_num); i++)
	{
		ref1[i] = (buf2[i*2] <<8) + buf2[i*2+1];
		printk(" %d", ref1[i]);
	}
	printk("\n");


	/////* Send Value */////
	for (i = 0; i < (tst200_col_num + tst200_row_num) ; i++)
	{
		written_bytes += sprintf(buf+written_bytes, "%d %d\n", ref1[i], ref2[i]);
	}

	if (written_bytes > 0)
		return written_bytes ;

	return sprintf(buf, "-1") ;
}


static ssize_t raw_enable_tst200(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, ret;
	uint8_t buf1[2] = {0,};

	/////* Enter Inspection Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0x70;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//enter Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}
	
	tsp_testmode = 1;
	printk("[TSP] %s start. line : %d, \n", __func__,__LINE__);

	mdelay(300); 

	return 1;
}


static ssize_t raw_disable_tst200(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, ret;
	uint8_t buf1[2] = {0,};

	/////* Exit Inspection Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{	
		buf1[0] = 0x00;//address
		buf1[1] = 0x00;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//exit Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}
	
	tsp_testmode = 0;
	printk("[TSP] %s stop. line : %d, \n", __func__,__LINE__);

	return 1;
}


static ssize_t read_rawdata(struct device *dev, struct device_attribute *attr, char *buf)//AT+TSPPTEST=1,1,1
{
	printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);
	
	int tst200_col_num = 9; //0 ~ 8
	int tst200_row_num = 7; //0 ~ 6

	int  written_bytes = 0 ;	/* & error check */

	uint8_t buf1[2]={0,};
	uint8_t buf2[32]={0,};

	uint16_t ref1[16]={0,};
	uint16_t ref2[16]={0,};

	int i,j,k;
	int ret;

	tsp_testmode = 1;

	/////* Enter Inspection Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0x70;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//enter Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}

	mdelay(300); 


	/////* Raw Value */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x0C; //address

		ret = i2c_master_send(ts_global->client, buf1, 1);

		if (ret >= 0)
		{
			ret = i2c_master_recv(ts_global->client, buf2, (tst200_col_num + tst200_row_num)*2);

			if (ret >= 0)
				break; // i2c success
		}
	}

	printk("[TSP]");
	for(i = 0 ; i < (tst200_col_num + tst200_row_num); i++)
	{
		ref1[i] = (buf2[i*2] <<8) + buf2[i*2+1];
		printk(",%d", ref1[i]);
	}
	printk("\n");


	/////* Exit Inspection Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{	
		buf1[0] = 0x00;//address
		buf1[1] = 0x00;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//exit Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}


	/////* Send Value */////
	for (i = 0; i < (tst200_col_num + tst200_row_num) ; i++)
	{
		written_bytes += sprintf(buf+written_bytes, ",%d", ref1[i]);
	}

	printk("[TSP] %s\n", buf);
	
	mdelay(300);

	tsp_testmode = 0;

	if (written_bytes > 0)
		return written_bytes ;

	return sprintf(buf, "-1") ;
}


static ssize_t read_difference(struct device *dev, struct device_attribute *attr, char *buf)//AT+TSPPTEST=1,2,1
{
	printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);

	mdelay(300); 
	
	int tst200_col_num = 9; //0 ~ 8
	int tst200_row_num = 7; //0 ~ 6

	int  written_bytes = 0 ;	/* & error check */

	uint8_t buf1[2]={0,};
	uint8_t buf2[32]={0,};

	uint16_t ref1[16]={0,};
	uint16_t ref2[16]={0,};

	int i,j,k;
	int ret;

	tsp_testmode = 1;

	/////* Enter Inspection Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0x70;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//enter Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}

	mdelay(300); 	


	/////* Diff Value */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x2C;//address

		ret = i2c_master_send(ts_global->client, buf1, 1);
		if (ret >= 0)
		{
			ret = i2c_master_recv(ts_global->client, buf2, (tst200_col_num + tst200_row_num));

			if (ret >= 0)
				break; // i2c success
		}
	}

	printk("[TSP]");
	for(i = 0 ; i < (tst200_col_num + tst200_row_num) ; i++)
	{
		ref2[i] = buf2[i];
		printk(",%d", ref2[i]);
	}
	printk("\n");


	/////* Exit Inspection Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{	
		buf1[0] = 0x00;//address
		buf1[1] = 0x00;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//exit Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}
	

	/////* Send Value */////
	for (i = 0; i < (tst200_col_num + tst200_row_num) ; i++)
	{
		written_bytes += sprintf(buf+written_bytes, ",%d", ref2[i]);
	}

	printk("[TSP] %s\n", buf);

	mdelay(300);

	tsp_testmode = 0;

	if (written_bytes > 0)
		return written_bytes ;

	return sprintf(buf, "-1") ;
}


static ssize_t rawdata_pass_fail_tst200(struct device *dev, struct device_attribute *attr, char *buf)
{
	int tst200_col_num = 9;	//0 ~ 8
	int tst200_row_num = 7;	//0 ~ 6

	int  written_bytes = 0 ;	/* & error check */

	uint8_t buf1[2]={0,};
	uint8_t buf2[32]={0,};

	uint16_t ref1[16]={0,};
	uint16_t ref2[16]={0,};

	int i,j,k;
	int ret;

	uint16_t RAWDATA_MAX[16] = {16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383};
	uint16_t RAWDATA_MIN[16] = {0,};

	tsp_testmode = 1;

	/////* Raw Value */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x0C; //address

		ret = i2c_master_send(ts_global->client, buf1, 1);

		if (ret >= 0)
		{
			ret = i2c_master_recv(ts_global->client, buf2, (tst200_col_num + tst200_row_num)*2);

			if (ret >= 0)
				break; // i2c success
		}
	}

	for(i = 0 ; i < (tst200_col_num + tst200_row_num); i++)
	{
		ref1[i] = (buf2[i*2] <<8) + buf2[i*2+1];
		printk(" %d", ref1[i]);
	}
	printk("\n");


	/////* Check MAX/MIN Raw Value */////
	for (i = 0; i < (tst200_col_num + tst200_row_num); i++)
    {
		if( RAWDATA_MAX[i] < ref1[i]) return sprintf(buf, "0"); // fail
		if( RAWDATA_MIN[i] > ref1[i]) return sprintf(buf, "0"); // fail
    }

	tsp_testmode = 0;

    return sprintf(buf, "1"); // success
 }


static ssize_t touch15test_enable_tst200(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, ret;
	uint8_t buf1[2] = {0,};

	tsp_testmode = 1;

	/////* Enter touch 15 test */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0x50;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//enter Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}
	
	printk("[TSP] %s start. line : %d, \n", __func__,__LINE__);
	printk("[TSP] Enter touch 15 test\n");

	mdelay(300); 

	tsp_testmode = 0;

	return 1;
}


static ssize_t touch15test_disable_tst200(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, ret;
	uint8_t buf1[2] = {0,};

	tsp_testmode = 1;

	/////* Exit touch 15 test */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{	
		buf1[0] = 0x00;//address
		buf1[1] = 0x00;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//exit touch 15 test
	
		if (ret >= 0)
			break; // i2c success
	}
	
	printk("[TSP] %s stop. line : %d, \n", __func__,__LINE__);
	printk("[TSP] Exit touch 15 test\n");

	mdelay(100); 

	tsp_testmode = 0;

	return 1;

}


module_init(synaptics_ts_init);
module_exit(synaptics_ts_exit);

MODULE_DESCRIPTION("Synaptics Touchscreen Driver");
MODULE_LICENSE("GPL");

