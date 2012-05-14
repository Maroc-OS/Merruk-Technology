/*
* Optical Finger Mouse with I2C interface

* File Name	:	drivers/input/keyboard/Ofm_driver.c
* Author	:	Chi-Hun Won (chihun.won@samsung.com)
* Date		:	13/05/2011
* This file is subject to the terms and conditions of the GNU General Public
* License.  See the file COPYING in the main directory of this archive for
* more details.
*/


#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/earlysuspend.h>

#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <mach/gpio.h>
#include <linux/io.h>
#include <mach/reg_syscfg.h>

#include <linux/time.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>

#define OFM_DBG 0

#if OFM_DBG
#define OFMDBG(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define OFMDBG(fmt, args...)
#endif


#define I2C_READ_WORD_DATA 1		//for I2C Multi read function

#define OTP_INT 22
#define OTP_POWER_EN 54
#define OTP_SHDN 35
#define OTP_RST 58
#define OTP_SCL 7
#define OTP_SDA 15

#define OFM_ON 1
#define OFM_OFF 0

#define abs_val(a) (((a) < (0)) ? (-(a)) : (a))

#define OFM_SENSITIVITY_X1  90
#define OFM_SENSITIVITY_X2  80
#define OFM_SENSITIVITY_X3  70
#define OFM_SENSITIVITY_X4  60
#define OFM_SENSITIVITY_X5  50
#define OFM_SENSITIVITY_X6  45
#define OFM_SENSITIVITY_X7  40
#define OFM_SENSITIVITY_X8  35
#define OFM_SENSITIVITY_X9  30

#define OFM_SENSITIVITY_X_DEFAULT  4

#define OFM_SENSITIVITY_Y1  90
#define OFM_SENSITIVITY_Y2  80
#define OFM_SENSITIVITY_Y3  70
#define OFM_SENSITIVITY_Y4  60
#define OFM_SENSITIVITY_Y5  50
#define OFM_SENSITIVITY_Y6  45
#define OFM_SENSITIVITY_Y7  40
#define OFM_SENSITIVITY_Y8  35
#define OFM_SENSITIVITY_Y9  30

#define OFM_SENSITIVITY_Y_DEFAULT  4

#define OFM_SENSITIVITY_MAX_LEVEL   9

static int gv_ofm_Xsensitivity[OFM_SENSITIVITY_MAX_LEVEL]=
{
    OFM_SENSITIVITY_X1,
    OFM_SENSITIVITY_X2,
    OFM_SENSITIVITY_X3,
    OFM_SENSITIVITY_X4,
    OFM_SENSITIVITY_X5,
    OFM_SENSITIVITY_X6,
    OFM_SENSITIVITY_X7,
    OFM_SENSITIVITY_X8,
    OFM_SENSITIVITY_X9,
};

static int gv_ofm_Ysensitivity[OFM_SENSITIVITY_MAX_LEVEL]=
{
    OFM_SENSITIVITY_Y1,
    OFM_SENSITIVITY_Y2,
    OFM_SENSITIVITY_Y3,
    OFM_SENSITIVITY_Y4,
    OFM_SENSITIVITY_Y5,
    OFM_SENSITIVITY_Y6,
    OFM_SENSITIVITY_Y7,
    OFM_SENSITIVITY_Y8,
    OFM_SENSITIVITY_Y9,
};

unsigned char gv_ofm_Pressed = 0;
int gv_Sum_DeltaX;
int gv_Sum_DeltaY;
int gv_I2C_Read_Cnt;

int gv_TestSum_DeltaX;
int gv_TestSum_DeltaY;
unsigned char gv_ofm_timer_start = 0 ;

struct ofm_pin
{
    int 	pin;
    int 	pin_setting;
    int 	irq;
    char	*name;
};

struct ofm
{
    struct 	delayed_work work;
    struct	i2c_client *client;
    struct   input_dev *input_dev;
    unsigned int  irq_count;    
    unsigned char intr_disable_count;
    struct mutex ops_lock;    
    unsigned int  x_level;        
    unsigned int  y_level;        

    struct work_struct work_acc;    
    struct hrtimer timer;
    ktime_t acc_poll_delay;    
    struct workqueue_struct *wq;    
    struct early_suspend early_suspend;    
};

struct ofm ofm_global;
//struct workqueue_struct *ofm_workqueue;
//struct class *ofm_class;

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int set_irq_type(unsigned int irq, unsigned int type);

static void ofm_motion_func(struct work_struct *work);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ofm_early_suspend(struct early_suspend *h);
static void ofm_late_resume(struct early_suspend *h);
#endif

/* *******************************************************************************
*	OFM Port configuration settings.													*
*	ofm_power_dn	:	POWER_DOWN PIN (set as output)							*
*	omf_motion		:	OFM MOTION PIN (set as floating / interrupt pin)					*
*	ofm_left			:	OFM DOMME CLICK PIN (set as floating / interrupt pin)			*
******************************************************************************** */
static void ofm_enable_irq(void)
{

//   if (ofm_global.irq_count < 1 )
//   {
//      ofm_global.irq_count++;
      enable_irq(ofm_global.client->irq);
      OFMDBG("[OFM] %s: Enabling irq %d \n",  __FUNCTION__, ofm_global.client->irq);
//   }
//   else
//   {
//      OFMDBG("[OFM] %s: irq %d is already enabled\n", __FUNCTION__, ofm_global.client->irq);
//   }

}

static void ofm_disable_irq(void)
{

//   if (ofm_global.irq_count > 0 )
//   {
//      ofm_global.irq_count--;
      disable_irq(ofm_global.client->irq);      
      OFMDBG("[OFM] %s: Disabling irq %d\n", __FUNCTION__, ofm_global.client->irq);
//   }
//   else
//   {
//      OFMDBG("[OFM] %s : irq %d is already disabled\n",  __FUNCTION__, ofm_global.client->irq);
//   }
   
}


static int ofm_i2c_write(struct i2c_client *client, u_int8_t index, u_int8_t data)
{
	int result;
	u_int8_t buf[2] = {index , data};
    
	result= i2c_master_send(client, buf, 2);
	OFMDBG("[OFM] %s, ret : %d\n", __func__,  result );
	if(result < 0)
	{
        	printk(KERN_ERR "[OFM] ofm_i2c_write ERROR!!index(%x) data(%x) return (%x)\n",index,data,result);
        	return result;
	}
	return 0;
}


static void ofm_timer_enable(void)
{
        OFMDBG("[OFM] ofm_timer_enable, timer delay %lldns\n", ktime_to_ns(ofm_global.acc_poll_delay));
        hrtimer_start(&ofm_global.timer, ofm_global.acc_poll_delay, HRTIMER_MODE_REL);       
}

#if 0 //exposure overwrite
static void ofm_timer_disable(void)
{
	OFMDBG("[OFM] ofm_timer_disable \n");    
        hrtimer_cancel(&ofm_global.timer);
        cancel_work_sync(&ofm_global.work_acc);    
}
#endif


static void ofm_work_func_acc(struct work_struct *work)
{
    OFMDBG("[OFM] ofm_work_func_acc\n");    
    
#if 0 //exposure overwrite

    if(!gpio_get_value(OTP_INT)) //low->high.. So Rising Edge Interrupt is occured.
    {            
	OFMDBG("[OFM] exposure overwrite \n");        
	ofm_i2c_write(ofm_global.client,0x41,200);
    }
    
#else //last movement

    if(!gpio_get_value(OTP_INT)) //low->high.. So Rising Edge Interrupt is occured.
    {            
        printk(KERN_INFO "[OFM] [%3d] [%4d] [%4d] CLEAR\n", ofm_global.x_level, gv_Sum_DeltaX, gv_Sum_DeltaY);           
        gv_Sum_DeltaX = 0;
        gv_Sum_DeltaY = 0;        
        gv_ofm_Pressed = 0;     
    }
    
#endif    
}


static enum hrtimer_restart ofm_timer_func(struct hrtimer *timer)
{
	queue_work(ofm_global.wq, &ofm_global.work_acc);
    
#if 0 //exposure overwrite      

	hrtimer_forward_now(&ofm_global.timer, ofm_global.acc_poll_delay);
	return HRTIMER_RESTART;
    
#else //last movement

	return HRTIMER_NORESTART;

#endif    
}


static void ofm_motion_continue(struct ofm *ofm)
{	
    if(gpio_get_value(OTP_INT)) //low->high.. So Rising Edge Interrupt is occured.
    {            
        schedule_delayed_work(&ofm_global.work, 2);
    }
    else
    {       
        OFMDBG("[OFM] INT_LOW\n");                

        //gv_Sum_DeltaX = 0;
        //gv_Sum_DeltaY = 0;        
        gv_ofm_Pressed = 0;     
        
        if(ofm_global.intr_disable_count)
        {
            ofm_global.intr_disable_count--;
            ofm_enable_irq();
        }                             
        
#if 1 //last movement
            ofm_timer_enable();
#endif
    }

}


static void ofm_motion_func(struct work_struct *work)
{
    struct ofm *ofm = container_of(work, struct ofm, work.work);
    s32 result;
    s8  deltaX, deltaY;
    u8 expMax;
    int relX, relY;

/*
Register : X motion (0x21)
Register : Y motion (0x22)
This register holds the overall X or Y movement data since last polling was done. Value is 8 bit 2¡¯s complement.(1)
-127    -2      -1      0       +1      +2      +126     +127
81         FE      FF     00      01      02       7E         7F
*/
#if I2C_READ_WORD_DATA

    result = i2c_smbus_read_word_data(ofm_global.client,0x21);
    if(result<0)
    {
    	printk(KERN_ERR "[OFM] i2c_smbus_read_word_data (%d)\n",result);
    	goto out;
    }
    deltaX = (result&0xff);
    deltaY = ((result>>8)&0xff);
#else
    result = i2c_smbus_read_byte_data(ofm_global.client,0x22); // Y read
    if(result<0)
    {
    	printk(KERN_ERR "[OFM] i2c_smbus_read_byte_data 1  (%d)\n",result);	
    	goto out;
    }
    deltaY = result;
    result = i2c_smbus_read_byte_data(ofm_global.client,0x21); // X read

    if(result<0)
    {
    	printk(KERN_ERR "[OFM] i2c_smbus_read_byte_data 2 (%d)\n",result);	
    	goto out;
    }
    deltaX = result;	 
#endif
/*
Register : Max Exposure(0x4F)
This registers holds the maximum pixel value for the current frame. It shows if some pixels are saturated or not.
*/
    result = i2c_smbus_read_byte_data(ofm_global.client,0x4F); 
    if(result<0)
    {
    	printk(KERN_ERR "[OFM] i2c_smbus_read_byte_data 3  (%d)\n",result);	
    	goto out;
    }
    expMax = result;
    
    if(expMax <= 25)
    {
        printk(KERN_INFO "[OFM] Exposure = %d\n", expMax);    
        ofm_motion_continue(ofm);        
    	goto out;
    }

    gv_ofm_Pressed =0;   
    gv_Sum_DeltaX += (signed int)(signed char)(deltaX);
    gv_Sum_DeltaY += (signed int)(signed char)(deltaY);

    OFMDBG("[OFM] [%3d] X :[%3d] Y:[%3d] SUMX:[%3d] SUMY:[%3d]\n",++gv_I2C_Read_Cnt,  deltaX, deltaY, gv_Sum_DeltaX, gv_Sum_DeltaY);

    if( (abs_val(gv_Sum_DeltaX) > abs_val(gv_Sum_DeltaY)) 
        &&  ( abs_val(gv_Sum_DeltaX) > gv_ofm_Xsensitivity[ofm_global.x_level]) )
    {
        relX = 1;//(abs_val(gv_Sum_DeltaX) - (OFM_SENSITIVITY_X - 5)) / 10;     
        relY = 0;
        
        gv_TestSum_DeltaX = gv_Sum_DeltaX;
        gv_TestSum_DeltaY = gv_Sum_DeltaY;        
        
        if(gv_Sum_DeltaX > 0 ) //KEY_RIGHT
        {
            input_report_key(ofm_global.input_dev, KEY_RIGHT, 1);
            input_sync(ofm_global.input_dev);
            input_report_key(ofm_global.input_dev, KEY_RIGHT, 0);
            input_sync(ofm_global.input_dev);    
            gv_ofm_Pressed =1;            

            printk(KERN_INFO "[OFM] [%3d] [%4d] [%4d] KEY_RIGHT\n", ofm_global.x_level, gv_Sum_DeltaX, gv_Sum_DeltaY);    
        }
        else  //KEY_LEFT
        {
            input_report_key(ofm_global.input_dev, KEY_LEFT, 1);
            input_sync(ofm_global.input_dev);
            input_report_key(ofm_global.input_dev, KEY_LEFT, 0);
            input_sync(ofm_global.input_dev);
            gv_ofm_Pressed =1;            

            printk(KERN_INFO "[OFM] [%3d] [%4d] [%4d] KEY_LEFT\n", ofm_global.x_level, gv_Sum_DeltaX, gv_Sum_DeltaY);        
        }
    }
    else if( ( abs_val(gv_Sum_DeltaY) > abs_val(gv_Sum_DeltaX) ) 
        && ( abs_val(gv_Sum_DeltaY) > gv_ofm_Ysensitivity[ofm_global.y_level]) )
    {
        relX = 0;
        relY = 1; //(abs_val(gv_Sum_DeltaY) - (OFM_SENSITIVITY_Y - 5)) / 10;           
        
        gv_TestSum_DeltaX = gv_Sum_DeltaX;
        gv_TestSum_DeltaY = gv_Sum_DeltaY;   
        
        if(gv_Sum_DeltaY > 0 )  //KEY_UP
        {
            input_report_key(ofm_global.input_dev, KEY_DOWN, 1);
            input_sync(ofm_global.input_dev);
            input_report_key(ofm_global.input_dev, KEY_DOWN, 0);
            input_sync(ofm_global.input_dev);      
            gv_ofm_Pressed =1;            

            printk(KERN_INFO "[OFM] [%3d] [%4d] [%4d] KEY_DOWN\n",ofm_global.x_level, gv_Sum_DeltaX, gv_Sum_DeltaY );                    
        }
        else //KEY_DOWN
        {
            input_report_key(ofm_global.input_dev, KEY_UP, 1);
            input_sync(ofm_global.input_dev);
            input_report_key(ofm_global.input_dev, KEY_UP, 0);
            input_sync(ofm_global.input_dev);      
            gv_ofm_Pressed =1;            

            printk(KERN_INFO "[OFM] [%3d] [%4d] [%4d] KEY_UP\n", ofm_global.x_level, gv_Sum_DeltaX, gv_Sum_DeltaY);               
        }
    }
    
    if(gv_ofm_Pressed)
    {
        gv_Sum_DeltaX = 0;
        gv_Sum_DeltaY = 0;        
        gv_ofm_Pressed = 0;
    }

    ofm_motion_continue(ofm);

    out: 

    OFMDBG("[OFM] OUT : %d\n", ofm_global.intr_disable_count);            

    if(ofm_global.intr_disable_count)
    {
        ofm_global.intr_disable_count--;
        ofm_enable_irq();              
    }

}


static irqreturn_t ofm_motion_event(int irq, void *dev_id/*, struct pt_regs *regs*/)
{
        gv_I2C_Read_Cnt = 0;    

	disable_irq_nosync(ofm_global.client->irq);
    
	ofm_global.intr_disable_count++;
        OFMDBG("[OFM] INT_HIGH : %d\n", ofm_global.intr_disable_count);    

        schedule_delayed_work(&ofm_global.work, 0);         
        
	return IRQ_HANDLED;
}

	
static void ofm_ctrl_power(int on_off)
{
    //struct ofm *ofm = i2c_get_clientdata(ofm_global.client);

    printk(KERN_INFO "[OFM] ofm_ctrl_power (%d)\n", on_off);

    if(on_off==OFM_ON)
    {
        gpio_request(OTP_POWER_EN, "OTP_POWER_EN");
        gpio_direction_output(OTP_POWER_EN, 1);
        bcm_gpio_pull_up(OTP_POWER_EN, true);
        bcm_gpio_pull_up_down_enable(OTP_POWER_EN, true);                
        //mdelay(10);

        gpio_request(OTP_SHDN, "OTP_SHDN");
        gpio_direction_output(OTP_SHDN, 0);
        mdelay(2);

        /* power management setting (Manual power mode)*/       
        ofm_i2c_write(ofm_global.client,0x05,0x28); //mdelay(1);   

        /* Set Min feature value*/       
        ofm_i2c_write(ofm_global.client,0x29,0x07); //mdelay(1);               

        /* Set X Y Direction*/        
        ofm_i2c_write(ofm_global.client,0x27,0x1A); //mdelay(1);

         /* Set 400 CPI for X data */
        ofm_i2c_write(ofm_global.client,0x2A,0x08); //mdelay(1);
         
        /* Set 400 CPI for Y data*/    
        ofm_i2c_write(ofm_global.client,0x2B,0x08); //mdelay(1);

        /* Return Motion pin to MCU control*/
        ofm_i2c_write(ofm_global.client,0x0E,0x67); //mdelay(1);

        /* Exposure time (Manual Exposure Mode) */
        ofm_i2c_write(ofm_global.client,0x43,0x02); //mdelay(1);
        ofm_i2c_write(ofm_global.client,0x41,0xFF); //mdelay(1);        

    }
    else
    {
        /* Set Motion pin = 0(no motion)*/
        ofm_i2c_write(ofm_global.client,0x0F,0x40); //mdelay(1);   

        /* Set Motion pin to manual control*/
        ofm_i2c_write(ofm_global.client,0x0E,0x65); //mdelay(1);   

        /* Set Internal regulator to low power standby mode*/
        ofm_i2c_write(ofm_global.client,0x05,0x2C); //mdelay(1);      

        mdelay(2);
        gpio_request(OTP_SHDN, "OTP_SHDN");
        gpio_direction_output(OTP_SHDN, 1);

        gpio_request(OTP_POWER_EN, "OTP_POWER_EN");
        gpio_direction_output(OTP_POWER_EN, 0);
        bcm_gpio_pull_up(OTP_POWER_EN, false);
        bcm_gpio_pull_up_down_enable(OTP_POWER_EN, true);    
        //mdelay(2);        
    }
}


static ssize_t ofm_sumXY_fs_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;

	count = sprintf(buf,"%d,%d\n", gv_TestSum_DeltaX, gv_TestSum_DeltaY);

	return count;
}

static ssize_t ofm_level_store(struct device *dev,struct device_attribute *attr, const char *buf, size_t count)
{    
	int rc;
	unsigned long level;

	rc = strict_strtoul(buf, 0, &level);
	if (rc)
	{
                printk(KERN_INFO "[OFM] ofm_level_store : rc = %d\n", rc);
		return rc;
	}

	rc = -ENXIO;
	mutex_lock(&ofm_global.ops_lock);

        printk(KERN_INFO "[OFM] ofm_level_store : Level = %lu\n", level);

        if((level >= 0) && (level < OFM_SENSITIVITY_MAX_LEVEL))
        {
            ofm_global.x_level = level;
            ofm_global.y_level = level;
        }

        rc = count;
    
	mutex_unlock(&ofm_global.ops_lock);

	return rc;
}


static struct device_attribute dev_attr_set_level =
	__ATTR(ojkeylevel, 0664 , NULL, ofm_level_store);

static struct device_attribute dev_attr_get_ojkeysum =
	__ATTR(ojkeysum, 0444, ofm_sumXY_fs_read, NULL);


static struct attribute *ofm_sysfs_attrs[] = {
	&dev_attr_set_level.attr,
	&dev_attr_get_ojkeysum.attr,
	NULL
};

static struct attribute_group ofm_attribute_group = {
	.attrs = ofm_sysfs_attrs,
};

static int ofm_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	//struct ofm	*ofm;
	int result;

	printk(KERN_INFO "[OFM] ofm_i2c_probe start \n");
    
       ofm_global.client = client;    

	i2c_set_clientdata(client, &ofm_global);
        printk(KERN_INFO  "[OFM] slave address 0x%02x\n", client->addr);

	//ofm_global = ofm;
    
	ofm_global.input_dev = input_allocate_device();
	if (ofm_global.input_dev == NULL) 
	{
		dev_err(&client->dev, "[OFM] Failed to allocate input device.\n");
		result = -ENOMEM;
		goto err0;
	}
    
	ofm_global.input_dev->evbit[0] =BIT_MASK(EV_SYN)| BIT_MASK(EV_KEY)|BIT_MASK(EV_REL);
	ofm_global.input_dev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT);
	ofm_global.input_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);

	ofm_global.input_dev->name = "Optical_Joystick";
	ofm_global.input_dev->id.bustype = BUS_I2C;
	ofm_global.input_dev->dev.parent = &client->dev;
	ofm_global.input_dev->phys = "ojkey/input0";
	ofm_global.input_dev->id.vendor = 0x0001;
	ofm_global.input_dev->id.product = 0x0001;
	ofm_global.input_dev->id.version = 0x0100;

	set_bit(KEY_UP, ofm_global.input_dev->keybit);
	set_bit(KEY_RIGHT, ofm_global.input_dev->keybit);
	set_bit(KEY_LEFT, ofm_global.input_dev->keybit);
	set_bit(KEY_DOWN, ofm_global.input_dev->keybit);

	//input_set_drvdata(ofm->input_dev, ofm);

	//ofm->ofm_left = &ofm_left;
	//ofm->ofm_motion = &ofm_motion;
	//ofm->ofm_power_dn = &ofm_power_dn;

	/* pin setting */
	gpio_request(OTP_SHDN, "otp_shut_down");
	gpio_direction_output(OTP_SHDN, 0);
	mdelay(5);

	mutex_init(&ofm_global.ops_lock);

	result = input_register_device(ofm_global.input_dev);
	if (result)
	{
                dev_err(&client->dev, " %s(%s): Unable to register %s input device\n", __FILE__, __FUNCTION__, ofm_global.input_dev->name);
		goto err1;
	}
    
        /*sys_fs*/
	result = sysfs_create_group(&ofm_global.input_dev->dev.kobj,&ofm_attribute_group);
	if (result) {
		printk(KERN_ERR "[OFM] Creating sysfs attribute group failed");
		goto err1;
	}

        /*Default Sensitivity Level*/
        ofm_global.x_level = OFM_SENSITIVITY_X_DEFAULT;
        ofm_global.y_level = OFM_SENSITIVITY_Y_DEFAULT;
        
	/* hrtimer settings.  we poll for light values using a timer. */
#if 0 //exposure overwrite    

	hrtimer_init(&ofm_global.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ofm_global.acc_poll_delay = ns_to_ktime(100 * NSEC_PER_MSEC);
	ofm_global.timer.function =ofm_timer_func;    

#else //last movement

	hrtimer_init(&ofm_global.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ofm_global.acc_poll_delay = ns_to_ktime(200 * NSEC_PER_MSEC);
	ofm_global.timer.function =ofm_timer_func;    
    
#endif

	ofm_global.wq = create_singlethread_workqueue("ofm_wq");
	if (!ofm_global.wq) {
		printk(KERN_ERR "%s: could not create workqueue\n", __func__);
		goto err1;
	}
    
	/* this is the thread function we run on the work queue */
	INIT_WORK(&ofm_global.work_acc, ofm_work_func_acc);        
	INIT_DELAYED_WORK(&ofm_global.work, ofm_motion_func);
    
        ofm_ctrl_power(OFM_ON);

	/* init for motion interrupt */	
	gpio_request(OTP_INT, "otp_irq");
	gpio_direction_input(OTP_INT);
	bcm_gpio_pull_up(OTP_INT, true);
	bcm_gpio_pull_up_down_enable(OTP_INT, true);
       set_irq_type(GPIO_TO_IRQ(OTP_INT),IRQ_TYPE_EDGE_RISING);

	client->irq = GPIO_TO_IRQ(OTP_INT);
        ofm_global.client->irq = GPIO_TO_IRQ(OTP_INT);

       if (client->irq)
       {
            result = request_irq (client->irq, ofm_motion_event, IRQF_TRIGGER_RISING, client->name, &ofm_global);
            if (result) 
            {
                dev_err(&client->dev, "[OFM] %s : Request IRQ  %d  failed\n", __FUNCTION__, client->irq);
                goto err2;
            }
            else
            {
                //result = set_irq_wake(client->irq, 1);
                //ofm_global.irq_count = 0;
                printk(KERN_INFO "[OFM] %s : Request IRQ  %d  Success\n",__FUNCTION__, client->irq);                
            }
       }

#ifdef CONFIG_HAS_EARLYSUSPEND
	ofm_global.early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 1;
	ofm_global.early_suspend.suspend = ofm_early_suspend;
	ofm_global.early_suspend.resume = ofm_late_resume;
	register_early_suspend(&ofm_global.early_suspend);
#endif

#if 0 //exposure overwrite 
        ofm_timer_enable();
#endif

	printk(KERN_INFO "[OFM] %s end\n", __func__ );
    
	return 0;		

	err2:
                free_irq(client->irq, &ofm_global);        
		input_unregister_device(ofm_global.input_dev);
		//ofm->input_dev = NULL; /* so we dont try to free it below */
	err1:
		input_free_device(ofm_global.input_dev);
		//kfree(ofm);
	err0:
                i2c_set_clientdata(client, NULL);
		printk(KERN_ERR "[OFM] I2C device probe error (%d) \n",result);
                
	return result;
}


static __devexit int ofm_i2c_remove(struct i2c_client *client)
{
	struct ofm *ofm = i2c_get_clientdata(client);
    	unregister_early_suspend(&ofm_global.early_suspend);
	input_unregister_device(ofm_global.input_dev);        
	//destroy_workqueue(ofm_workqueue);
	kfree(ofm);

	return 0;
}


static int ofm_suspend(struct i2c_client *client, pm_message_t mesg)
{
    int ret;
    struct ofm *ofm = i2c_get_clientdata(client);

    printk(KERN_INFO "[OFM] %s+\n", __func__ );
    
    ofm_disable_irq();

    gpio_direction_output( OTP_SCL , 0 ); 
    gpio_direction_output( OTP_SDA , 0 ); 

    ret = cancel_delayed_work_sync(&ofm ->work);
    
    if (ret /*&& ofm ->use_irq*/) /* if work was pending disable-count is now 2 */
    {
        printk(KERN_INFO "[OFM] %s : enable irq ret=%d\n",__func__, ret);
        ofm_enable_irq();
    }

    //msleep(10);

#if 0 //exposure overwrite 
    ofm_timer_disable();
#endif

    ofm_ctrl_power(OFM_OFF);

    printk(KERN_INFO "[OFM] %s-\n", __func__ );
    
    return 0;
}


static int ofm_resume(struct i2c_client *client)
{
//    int ret;
//    struct ofm *ofm = i2c_get_clientdata(client);

    printk(KERN_INFO "[OFM] %s+\n", __func__ );

    gpio_direction_output( OTP_SCL , 1 ); 
    gpio_direction_output( OTP_SDA , 1 ); 

    ofm_ctrl_power(OFM_ON);

    //msleep(10);

    ofm_enable_irq();

#if 0 //exposure overwrite 
    ofm_timer_enable();
#endif

    printk(KERN_INFO "[OFM] %s-\n", __func__ );
    
    return 0;
}


#ifdef CONFIG_HAS_EARLYSUSPEND
static void ofm_early_suspend(struct early_suspend *h)
{
	//struct ofm *ofm;
	//ofm = container_of(h, struct ofm, early_suspend);
	ofm_suspend(ofm_global.client, PMSG_SUSPEND);
}


static void ofm_late_resume(struct early_suspend *h)
{
    	//struct ofm *ofm;
	//ofm = container_of(h, struct ofm, early_suspend);    
	ofm_resume(ofm_global.client);
}
#endif


static const struct i2c_device_id ofm_i2c_id[]={
	{"ofm", 0 },
	{}
};


MODULE_DEVICE_TABLE(i2c, ofm_i2c);

static struct i2c_driver ofm_i2c_driver = {
    	.probe	= ofm_i2c_probe,
	.remove	= ofm_i2c_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= ofm_suspend,
	.resume	= ofm_resume,
#endif	
	.id_table	=	ofm_i2c_id,
	.driver	=	{
		.name	="ofm",
		},
};


static int __devinit ofm_init(void)
{
    int ret;
    
    printk(KERN_INFO "[OFM] %s\n", __func__);   

    gpio_request(OTP_POWER_EN, "OTP_POWER_EN");
    gpio_direction_output(OTP_POWER_EN, 1);
    bcm_gpio_pull_up(OTP_POWER_EN, true);
    bcm_gpio_pull_up_down_enable(OTP_POWER_EN, true);         
    mdelay(50);

    gpio_request(OTP_SHDN, "OTP_SHDN");
    gpio_direction_output(OTP_SHDN, 0);
    mdelay(50);

    ret = i2c_add_driver(&ofm_i2c_driver);

    if(ret!=0)
        printk(KERN_ERR "[OFM] I2C device init Faild! return(%d) \n",  ret);
    
    printk(KERN_ERR "[OFM] I2C device init Sucess\n");
    return ret;
}


static void __exit ofm_exit(void)
{
        printk(KERN_INFO "[OFM] %s\n", __func__ );    
        
	i2c_del_driver(&ofm_i2c_driver);
}

module_init(ofm_init);
module_exit(ofm_exit);

MODULE_DESCRIPTION("OFM Device Driver");
MODULE_AUTHOR("Partron Sensor Lab");
MODULE_LICENSE("GPL");

