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

#include "mcs8000_download.h"

/** defines **/
#define __TOUCH_DEBUG__  //TODO : 서버에 올리는 경우에는 막고 올리기.
#define USE_THREADED_IRQ 1 //TODO : QUEUE 방식이 아닌 THREAD 방식으로 변경. 이렇게 하니, IRQ 가 정상적으로 잘됨.
#define DELAY_BEFORE_VDD
#ifdef CONFIG_TOUCHSCREEN_TMA340_COOPERVE //CooperVE
#define SET_DOWNLOAD_BY_GPIO 0 //TODO : TSP 초기화 루틴에서 강제로 최신 FW 로 업데이트 하는 루틴으로 사용하면 안됨.
#define LATEST_FW_VER   0x03 //TODO : 이부분을 0x0 으로 하면, SET_DOWNLOAD_BY_GPIO 1 이어도 동작하지 안함.
#define FORCED_DOWNLOAD_OF_BLANKMEMORY	// TSP blank memory( No firmware ) 상태시 자동 펌웨어 다운로드
#endif

#define TOUCH_ON  1
#define TOUCH_OFF 0

#ifdef CONFIG_TOUCHSCREEN_TMA340_COOPERVE //CooperVE
#define __TOUCH_KEYLED__
#endif

#if defined (__TOUCH_KEYLED__)
static struct regulator *touchkeyled_regulator=NULL;
#endif

/** structures **/
struct muti_touch_info
{
    int state;
    int strength;
    int width;
    int posX;
    int posY;
};

struct melfas_ts_data
{
    uint16_t addr;
    struct i2c_client *client;
    struct input_dev *input_dev;
    struct work_struct  work;
    uint32_t flags;
    //int (*power)(int on);
    struct early_suspend early_suspend;
    int hw_rev;
    int fw_ver;
};

/** variables **/
static struct muti_touch_info g_Mtouch_info[TS_MAX_TOUCH];
static struct melfas_ts_data *ts;
struct melfas_ts_data *ts_global_melfas;
#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
static int8_t MMS128_Connected = 0;
#endif
static struct regulator *touch_regulator = NULL;
static int firmware_ret_val = -1;
static DEFINE_SPINLOCK(melfas_spin_lock);
#if USE_THREADED_IRQ

#else
static struct workqueue_struct *melfas_wq;
#endif

#ifdef FORCED_DOWNLOAD_OF_BLANKMEMORY
static bool bBlankMemory = false;
#endif

/** functions **/
extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int set_irq_type(unsigned int irq, unsigned int type);

struct class *touch_class_melfas;
EXPORT_SYMBOL(touch_class_melfas);
struct device *firmware_dev_melfas;
EXPORT_SYMBOL(firmware_dev_melfas);

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_store( struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_ret_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR(firmware	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_show, firmware_store);
static DEVICE_ATTR(firmware_ret	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_ret_show, firmware_ret_store);

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg);
static int melfas_ts_resume(struct i2c_client *client);
#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
extern int Is_Synaptics_Connected(void);
#endif

#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
int Is_MMS128_Connected(void)
{
    return (int) MMS128_Connected;
}
#endif

 //TODO : touch_ctrl_regulator() 함수는 다른 파일에서 선언된뒤, export 되어 잇음. //synaptics_i2c_rmi_tma340_cooperve.c synaptics_i2c_rmi_tma340_tassveve.c
void touch_ctrl_regulator_mms128(int on_off)
{
    if (on_off == TOUCH_ON)
    {
        regulator_set_voltage(touch_regulator, 2900000, 2900000);
        regulator_enable(touch_regulator);

#if defined (__TOUCH_KEYLED__)
        regulator_set_voltage(touchkeyled_regulator,3300000,3300000);
        regulator_enable(touchkeyled_regulator);
#endif
    }
    else
    {
        regulator_disable(touch_regulator);

#if defined (__TOUCH_KEYLED__) 
        regulator_disable(touchkeyled_regulator);
#endif
    }
}

#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
int tsp_i2c_read_melfas(u8 reg, unsigned char *rbuf, int buf_size) //same with tsp_i2c_read()
{
	int i, ret=-1;
	struct i2c_msg rmsg;
	uint8_t start_reg;
  int retry = 3;

	for (i = 0; i < retry; i++)
	{
		rmsg.addr = ts->client->addr;
		rmsg.flags = 0;//I2C_M_WR;
		rmsg.len = 1;
		rmsg.buf = &start_reg;
		start_reg = reg;
		
		ret = i2c_transfer(ts->client->adapter, &rmsg, 1);

		if(ret >= 0) 
		{
			rmsg.flags = I2C_M_RD;
			rmsg.len = buf_size;
			rmsg.buf = rbuf;
			ret = i2c_transfer(ts->client->adapter, &rmsg, 1 );

			if (ret >= 0)
				break; // i2c success
		}

		if( i == (retry - 1) )
		{
			printk("[TSP] Error code : %d, %d\n", __LINE__, ret );
		}
	}

	return ret;
}
#endif


static int melfas_i2c_read(struct i2c_client* p_client, u8 reg, u8* data, int len)
{
    struct i2c_msg msg;

    /* set start register for burst read */
    /* send separate i2c msg to give STOP signal after writing. */
    /* Continous start is not allowed for cypress touch sensor. */

    msg.addr = p_client->addr;
    msg.flags = 0;
    msg.len = 1;
    msg.buf = &reg;

    if (1 != i2c_transfer(p_client->adapter, &msg, 1))
    {
        printk("[TSP][MMS128][%s] set data pointer fail! reg(%x)\n", __func__, reg);
        return -EIO;
    }

    /* begin to read from the starting address */

    msg.addr = p_client->addr;
    msg.flags = I2C_M_RD;
    msg.len = len;
    msg.buf = data;

    if (1 != i2c_transfer(p_client->adapter, &msg, 1))
    {
        printk("[TSP][MMS128][%s] fail! reg(%x)\n", __func__, reg);
        return -EIO;
    }

    return 0;
}

static int melfas_i2c_write(struct i2c_client* p_client, u8* data, int len)
{
    struct i2c_msg msg;

    msg.addr = p_client->addr;
    msg.flags = 0; /* I2C_M_WR */
    msg.len = len;
    msg.buf = data ;

    if (1 != i2c_transfer(p_client->adapter, &msg, 1))
    {
        printk("[TSP][MMS128][%s] set data pointer fail!\n", __func__);
        return -EIO;
    }

    return 0;
}

static int melfas_init_panel(struct melfas_ts_data *ts)
{
    int ret, buf = 0x10;
    ret = melfas_i2c_write(ts->client, &buf, 1);
    ret = melfas_i2c_write(ts->client, &buf, 1);

    if (ret < 0)
    {
        printk(KERN_DEBUG "[TSP][MMS128][%s] melfas_i2c_write() failed\n [%d]", __func__, ret);
        return 0;
    }

    return 1;
}

#if USE_THREADED_IRQ
static irqreturn_t melfas_ts_work_func(int irq, void *dev_id)
#else
static void melfas_ts_work_func(struct work_struct *work)
#endif
{
#if USE_THREADED_IRQ
	struct melfas_ts_data *ts = dev_id;
#else
	struct melfas_ts_data *ts = container_of(work, struct melfas_ts_data, work);
#endif
	int ret = 0, i, j;
	uint8_t buf[66];
	int read_num = 0, touchType = 0, touchState = 0, fingerID = 0, keyID = 0;
	unsigned long flags;

#ifdef __TOUCH_DEBUG__
	printk(KERN_DEBUG "[TSP][MMS128][%s] \n",__func__);
#endif
	if (ts == NULL)
		printk("[TSP][MMS128][%s] : TS NULL\n",__func__);

	/**
	Simple send transaction:
	S Addr Wr [A]  Data [A] Data [A] ... [A] Data [A] P
	Simple recv transaction:
	S Addr Rd [A]  [Data] A [Data] A ... A [Data] NA P
	*/
	for (i = 0; i < 10; i++)
	{
		ret = melfas_i2c_read(ts->client, 0x0F, buf, 1);
		if (ret >= 0)
			break; // i2c success
	}
	spin_lock_irqsave(&melfas_spin_lock, flags);
	if (ret < 0)
	{
		printk("[TSP][MMS128][%s] i2c failed : %d\n", __func__, ret);
		enable_irq(ts->client->irq);
		spin_unlock_irqrestore(&melfas_spin_lock, flags);
		touch_ctrl_regulator_mms128(TOUCH_OFF);
		touch_ctrl_regulator_mms128(TOUCH_ON);
		melfas_init_panel(ts);
		return ;
	}
	else
	{
		read_num = buf[0];
	}

	if (read_num > 0)
	{
		for (i = 0; i < 10; i++)
		{
			ret = melfas_i2c_read(ts->client, 0x10, buf, read_num);
			if (ret >= 0)
				break; // i2c success
		}

		if (ret < 0)
		{
			printk("[TSP][MMS128][%s] i2c failed : %d\n", __func__, ret);
			enable_irq(ts->client->irq);
			spin_unlock_irqrestore(&melfas_spin_lock, flags);
			touch_ctrl_regulator_mms128(TOUCH_OFF);
			touch_ctrl_regulator_mms128(TOUCH_ON);
			melfas_init_panel(ts);
			return ;
		}
		else
		{
			bool touched_src = false;
			for (i = 0; i < read_num; i = i + 6)
			{
				touchType = (buf[i] >> 5) & 0x03;
				touchState = (buf[i] & 0x80);

				if (touchType == 1)	//Screen
				{
					touched_src = true;
					fingerID = (buf[i] & 0x0F) - 1;

					if ((fingerID > TS_MAX_TOUCH - 1) || (fingerID < 0))
					{
						printk("[TSP][MMS128][%s] fingerID : %d\n", __func__, fingerID);
						enable_irq(ts->client->irq);
						spin_unlock_irqrestore(&melfas_spin_lock, flags);
						touch_ctrl_regulator_mms128(TOUCH_OFF);
						touch_ctrl_regulator_mms128(TOUCH_ON);
						melfas_init_panel(ts);
						return ;
					}

					g_Mtouch_info[fingerID].posX = (uint16_t)(buf[i + 1] & 0x0F) << 8 | buf[i + 2];
					g_Mtouch_info[fingerID].posY = (uint16_t)(buf[i + 1] & 0xF0) << 4 | buf[i + 3];
					g_Mtouch_info[fingerID].width = buf[i + 4];

					if (touchState)
						g_Mtouch_info[fingerID].strength = buf[i + 5];
					else
						g_Mtouch_info[fingerID].strength = 0;
				}
 #ifdef CONFIG_TOUCHSCREEN_TMA340_COOPERVE //CooperVE
				else if (touchType == 2)	//Key
				{
					keyID = (buf[i] & 0x0F);

					if (keyID == 0x1)
						input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
					if (keyID == 0x2)
						input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);

#ifdef __TOUCH_DEBUG__
					printk(KERN_DEBUG "[TSP][MMS128][%s] keyID: %d, State: %d\n", __func__, keyID, touchState);
#endif
				}
 #endif
			}
			if (touched_src)
			{
				for (j = 0; j < TS_MAX_TOUCH; j ++)
				{
					if (g_Mtouch_info[j].strength == -1)
						continue;

					input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, j);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[j].posX);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[j].posY);
					input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[j].strength);
					input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[j].width);
					input_mt_sync(ts->input_dev);
#ifdef __TOUCH_DEBUG__
					printk(KERN_DEBUG "[TSP][MMS128][%s] fingerID: %d, State: %d, x: %d, y: %d, z: %d, w: %d\n",
						__func__, j, (g_Mtouch_info[j].strength > 0), g_Mtouch_info[j].posX, g_Mtouch_info[j].posY, g_Mtouch_info[j].strength, g_Mtouch_info[j].width);
#endif
					if (g_Mtouch_info[j].strength == 0)
						g_Mtouch_info[j].strength = -1;
				}
			}				
			input_sync(ts->input_dev);
		}
	}

#if USE_THREADED_IRQ

#else
	enable_irq(ts->client->irq);
#endif

	spin_unlock_irqrestore(&melfas_spin_lock, flags);

#if USE_THREADED_IRQ
	return IRQ_HANDLED;
#else

#endif
}


static irqreturn_t melfas_ts_irq_handler(int irq, void *dev_id)
{
    struct melfas_ts_data *ts = dev_id;

#if USE_THREADED_IRQ

#else
    disable_irq_nosync(ts->client->irq);
#endif

#if USE_THREADED_IRQ
    return IRQ_WAKE_THREAD;
#else
    queue_work(melfas_wq, &ts->work);
    return IRQ_HANDLED;
#endif
}

void melfas_upgrade(INT32 hw_ver)
{
    int ret;
    unsigned char buf[2];

    printk("[TSP][MMS128][F/W D/L] Entry mcsdl_download_binary_data\n");
    printk("[TOUCH] Melfas  H/W version: 0x%02x.\n", ts->hw_rev);
    printk("[TOUCH] Current F/W version: 0x%02x.\n", ts->fw_ver);	

    disable_irq(ts->client->irq);
    local_irq_disable();
	
    ret = mcsdl_download_binary_data(ts->hw_rev);
  
    local_irq_enable();
    enable_irq(ts->client->irq);

    printk("[TSP] melfas_upgrade()--  ret=%d\n", ret);

    if (0 == melfas_i2c_read(ts->client, MCSTS_MODULE_VER_REG, buf, 2))
    {
        ts->hw_rev = buf[0];
        ts->fw_ver = buf[1];
        printk("[TSP][MMS128][%s] HW Ver : 0x%02x, FW Ver : 0x%02x\n", __func__, buf[0], buf[1]);
    }
    else
    {
        ts->hw_rev = 0;
        ts->fw_ver = 0;
        printk("[TSP][MMS128][%s] Can't find HW Ver, FW ver!\n", __func__);
    }

    if (ret > 0)
    {
        if ((ts->hw_rev < 0) || (ts->fw_ver < 0))
            printk(KERN_DEBUG "[TSP][MMS128][%s] i2c_transfer failed\n",__func__);
        else
            printk("[TSP][MMS128][%s] Firmware update success! [Melfas H/W version: 0x%02x., Current F/W version: 0x%02x.]\n", __func__, ts->hw_rev, ts->fw_ver);
    }
    else
    {
        printk("[TSP][MMS128][%s] Firmware update failed.. RESET!\n",__func__);
        mcsdl_vdd_off();
        mdelay(500);
        mcsdl_vdd_on();
        mdelay(200);
    }

	if(ret == MCSDL_RET_SUCCESS)
		firmware_ret_val = 1;
	else
		firmware_ret_val = 0;	
	
}

#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
int melfas_ts_check(struct melfas_ts_data *ts)
{
    int ret, i;
    uint8_t buf_tmp[3]={0,0,0};
    int retry = 3;
    uint8_t VenderID;

    ret = tsp_i2c_read_melfas(0x1B, buf_tmp, sizeof(buf_tmp));		

// i2c read retry
    if(ret <= 0)
    {
    	for(i=0; i<retry;i++)
    	{
    		ret=tsp_i2c_read_melfas( 0x1B, buf_tmp, sizeof(buf_tmp));

		if(ret > 0)
			break;
    	}
    }

    if (ret <= 0) {
        printk("[TSP][MMS128][%s] %s\n", __func__,"Failed melfas i2c");
        MMS128_Connected = 0;
    } else {
        printk("[TSP][MMS128][%s] %s\n", __func__,"Passed melfas i2c");
        MMS128_Connected = 1;
    }

    VenderID = buf_tmp[0];
    ts->hw_rev = buf_tmp[1];
    ts->fw_ver = buf_tmp[2];	

    printk("[TSP][MMS128][%s][SlaveAddress : 0x%x][ret : %d] [ID : 0x%x] [HW : 0x%x] [SW : 0x%x]\n", __func__,ts->client->addr, ret, buf_tmp[0],buf_tmp[1],buf_tmp[2]);

    if( (ret > 0) && (VenderID == 0xa0 ) )
    {
        ret = 1;
        printk("[TSP][MMS128][%s] %s\n", __func__,"Passed melfas_ts_check");
    }
#ifdef FORCED_DOWNLOAD_OF_BLANKMEMORY
    else if ( (ret > 0) && (VenderID == 0x0)&& (ts->hw_rev == 0x0) && (ts->fw_ver == 0x0) )
    {
        ret = 1;
        bBlankMemory = true;
        printk("[TSP][MMS128][%s] %s\n", __func__,"Blank memory !!");
        printk("[TSP][MMS128][%s] %s\n", __func__,"Passed melfas_ts_check");
    }
#endif
    else
    {
        ret = 0;
        printk("[TSP][MMS128][%s] %s\n", __func__,"Failed melfas_ts_check");
    }

    return ret;
}
#endif

static int melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0, i;
    uint8_t buf[2];

#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
    printk("[TSP][MMS128][%s] %s\n", __func__, "Called");
    if (Is_Synaptics_Connected() == 1)
    {
        printk("[TSP][MMS128][%s] %s\n", __func__, "Synaptics already detected !!");
        return -ENXIO;
    }
#endif

    printk(KERN_DEBUG"+-----------------------------------------+\n");
    printk(KERN_DEBUG "|  Melfas Touch Driver Probe!            |\n");
    printk(KERN_DEBUG"+-----------------------------------------+\n");

#ifdef DELAY_BEFORE_VDD
    gpio_direction_output( GPIO_TSP_SCL , 1 ); 
    gpio_direction_output( GPIO_TSP_SDA , 1 ); 
    msleep(10);
#endif
 
    touch_ctrl_regulator_mms128(TOUCH_ON);
    msleep(70);

    ts = kzalloc(sizeof(struct melfas_ts_data), GFP_KERNEL);
    if (ts == NULL)
    {
        printk(KERN_DEBUG "[TSP][MMS128][%s] failed to create a state of melfas-ts\n",__func__);
        ret = -ENOMEM;
        goto err_alloc_data_failed;
    }

#if USE_THREADED_IRQ

#else
    INIT_WORK(&ts->work, melfas_ts_work_func);
#endif

    ts->client = client;
    i2c_set_clientdata(client, ts);

#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
    ret = melfas_ts_check(ts);
    if (ret <= 0)
    {
	i2c_release_client(client);	
       //touch_ctrl_regulator_mms128(TOUCH_OFF); //Melfas 에서 TOUCH_OFF 하면, Cypress 로 OFF 된다.
	kfree(ts);

	return -ENXIO;
    }
#else
    ret = melfas_i2c_write(ts->client, &buf, 1);
    printk(KERN_DEBUG "[TSP][MMS128][%s] melfas_i2c_write() [%d], Add[%d]\n", __func__, ret, ts->client->addr);
#endif

    /* sys fs */
    touch_class_melfas = class_create(THIS_MODULE, "touch");
    if (IS_ERR(touch_class_melfas))
        pr_err("[TSP][MMS128] Failed to create class(touch)!\n");
    firmware_dev_melfas = device_create(touch_class_melfas, NULL, 0, NULL, "firmware");
    if (IS_ERR(firmware_dev_melfas))
        pr_err("[TSP][MMS128] Failed to create device(firmware)!\n");
    if (device_create_file(firmware_dev_melfas, &dev_attr_firmware) < 0)
        pr_err("[TSP][MMS128] Failed to create device file(%s)!\n", dev_attr_firmware.attr.name);
    if (device_create_file(firmware_dev_melfas, &dev_attr_firmware_ret) < 0)
        pr_err("[TSP][MMS128] Failed to create device file(%s)!\n", dev_attr_firmware_ret.attr.name);

    /* sys fs */
//#if SET_DOWNLOAD_BY_GPIO
#if 0
    if (0 == melfas_i2c_read(ts->client, MCSTS_MODULE_VER_REG, buf, 2))
    {
        ts->hw_rev = buf[0];
        ts->fw_ver = buf[1];
        printk("[TSP][MMS128][%s] HW Ver : 0x%02x, FW Ver : 0x%02x\n", __func__, buf[0], buf[1]);
    }
    else
    {
        ts->hw_rev = 0;
        ts->fw_ver = 0;
        printk("[TSP][MMS128][%s] Can't find HW Ver, FW ver!\n", __func__);
    }

    printk("[TSP][MMS128] Melfas	H/W version: 0x%02x.\n", ts->hw_rev);
    printk("[TSP][MMS128] Current F/W version: 0x%02x.\n", ts->fw_ver);

    if (ts->fw_ver < LATEST_FW_VER)
    {
        melfas_upgrade(ts->hw_rev);
        msleep(1);
        touch_ctrl_regulator_mms128(TOUCH_OFF);
        msleep(1000);
        touch_ctrl_regulator_mms128(TOUCH_ON);
        msleep(70);
        //local_irq_disable();
        //mcsdl_download_binary_data();
        //printk("[TSP] enable_irq : %d\n", __LINE__);
        //local_irq_enable();
    }
#endif // SET_DOWNLOAD_BY_GPIO

#ifdef FORCED_DOWNLOAD_OF_BLANKMEMORY
	if (bBlankMemory)
	{
		melfas_upgrade(ts->hw_rev);
		msleep(1);
		touch_ctrl_regulator_mms128(TOUCH_OFF);
		msleep(1000);
		touch_ctrl_regulator_mms128(TOUCH_ON);
		msleep(70);
		//local_irq_disable();
		//mcsdl_download_binary_data();
		//printk("[TSP] enable_irq : %d\n", __LINE__);
		//local_irq_enable();	
	}
#endif

    ts->input_dev = input_allocate_device();
    if (!ts->input_dev)
    {
        printk(KERN_DEBUG "[TSP][MMS128][%s] Failed to allocate input device\n",__func__);
        ret = -ENOMEM;
        goto err_input_dev_alloc_failed;
    }
    ts->input_dev->name = "sec_touchscreen" ;

#ifdef CONFIG_TOUCHSCREEN_TMA340_COOPERVE //CooperVE
    ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);

    ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
    ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);

    //set_bit(BTN_TOUCH, ts->input_dev->keybit);
    //set_bit(EV_ABS,  ts->input_dev->evbit);
    //ts->input_dev->evbit[0] =  BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);

    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, TS_MAX_TOUCH - 1, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0);

    //set_bit(EV_SYN, ts->input_dev->evbit);
    //set_bit(EV_KEY, ts->input_dev->evbit);
#else //CONFIG_TOUCHSCREEN_TMA340 //TassVE
    ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
    ts->input_dev->keybit[BIT_WORD(KEY_POWER)] |= BIT_MASK(KEY_POWER);

    set_bit(BTN_TOUCH, ts->input_dev->keybit);
    set_bit(EV_ABS,  ts->input_dev->evbit);
    ts->input_dev->evbit[0] =  BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);

    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, 240 /*TS_MAX_X_COORD*/, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, 320 /*TS_MAX_Y_COORD*/, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, TS_MAX_TOUCH - 1, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0);

    set_bit(EV_SYN, ts->input_dev->evbit);
    set_bit(EV_KEY, ts->input_dev->evbit);
#endif

    ret = input_register_device(ts->input_dev);
    if (ret)
    {
        printk(KERN_DEBUG "[TSP][MMS128][%s] Unable to register %s input device\n", __func__, ts->input_dev->name);
        goto err_input_register_device_failed;
    }

    printk("[TSP][MMS128][%s] irq=%d\n", __func__, client->irq);

    gpio_request(GPIO_TOUCH_INT, "ts_irq");
    gpio_direction_input(GPIO_TOUCH_INT);
    bcm_gpio_pull_up(GPIO_TOUCH_INT, true);
    bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, true);
    set_irq_type(GPIO_TO_IRQ(GPIO_TOUCH_INT), IRQF_TRIGGER_FALLING);

    if (ts->client->irq)
    {
        printk(KERN_DEBUG "[TSP][MMS128][%s] trying to request irq: %s-%d\n", __func__, ts->client->name, ts->client->irq);
#if USE_THREADED_IRQ
        ret = request_threaded_irq(ts->client->irq, melfas_ts_irq_handler, melfas_ts_work_func, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, ts->client->name, ts);
#else
        ret = request_irq(ts->client->irq, melfas_ts_irq_handler, IRQF_TRIGGER_FALLING, ts->client->name, ts);
#endif
        if (ret > 0)
        {
            printk(KERN_DEBUG "[TSP][MMS128][%s] Can't allocate irq %d, ret %d\n", __func__, ts->client->irq, ret);
            ret = -EBUSY;
            goto err_request_irq;
        }
    }

    //schedule_work(&ts->work);
    //queue_work(melfas_wq, &ts->work);

    for (i = 0; i < TS_MAX_TOUCH ; i++)
        g_Mtouch_info[i].strength = -1;

#ifdef CONFIG_HAS_EARLYSUSPEND
    ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    ts->early_suspend.suspend = melfas_ts_early_suspend;
    ts->early_suspend.resume = melfas_ts_late_resume;
    register_early_suspend(&ts->early_suspend);
#endif

    printk(KERN_INFO "[TSP][MMS128][%s] Start touchscreen. name: %s, irq: %d\n", __func__, ts->client->name, ts->client->irq);

    return 0;

#if 1
err_request_irq:
    printk(KERN_DEBUG "[TSP][MMS128][%s] err_request_irq failed\n",__func__);
    free_irq(client->irq, ts);
#endif
err_input_register_device_failed:
    printk(KERN_DEBUG "[TSP][MMS128][%s] err_input_register_device failed\n",__func__);
    input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
    printk(KERN_DEBUG "[TSP][MMS128][%s] err_input_dev_alloc failed\n",__func__);
err_alloc_data_failed:
    printk(KERN_DEBUG "[TSP][MMS128][%s] err_alloc_data failed_\n",__func__);
#if 0
err_detect_failed:
    printk(KERN_DEBUG "melfas-ts: err_detect failed\n");
    kfree(ts);
err_check_functionality_failed:
    printk(KERN_DEBUG "melfas-ts: err_check_functionality failed_\n");
#endif
    return ret;
}

static int melfas_ts_remove(struct i2c_client *client)
{
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    unregister_early_suspend(&ts->early_suspend);
    free_irq(client->irq, ts);
    input_unregister_device(ts->input_dev);
    kfree(ts);
    return 0;
}

static void release_all_fingers(struct melfas_ts_data *ts)
{
    int i;

    for (i = 0; i < TS_MAX_TOUCH; i++)
    {
        if (g_Mtouch_info[i].strength == -1)
            continue;

        g_Mtouch_info[i].strength = 0;
        g_Mtouch_info[i].width = 0;
        g_Mtouch_info[i].posX = 0;
        g_Mtouch_info[i].posY = 0;

        input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
        input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
        input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
        input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength);
        input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
        input_mt_sync(ts->input_dev);

        if (g_Mtouch_info[i].strength == 0)
            g_Mtouch_info[i].strength = -1;
    }
    input_sync(ts->input_dev);
}

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    int ret = 0;
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    release_all_fingers(ts);

    disable_irq(client->irq);

    printk("[TSP][MMS128][%s] irq=%d\n", __func__, client->irq);

    ret = cancel_work_sync(&ts->work);

    return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    msleep(70);
 
    melfas_init_panel(ts);
    enable_irq(client->irq); // scl wave

    printk("[TSP][MMS128][%s] irq=%d\n", __func__, client->irq);

    return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
    printk("[TSP][MMS128][%s] \n", __FUNCTION__);
    release_all_fingers(ts);
    disable_irq(ts->client->irq);

    gpio_direction_output( GPIO_TOUCH_INT , 0 );
    gpio_direction_output( GPIO_TSP_SCL , 0 ); 
    gpio_direction_output( GPIO_TSP_SDA , 0 ); 

    bcm_gpio_pull_up(GPIO_TOUCH_INT, false);
    bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, true);
	
    touch_ctrl_regulator_mms128(TOUCH_OFF);
    //ts = container_of(h, struct melfas_ts_data, early_suspend);
    //melfas_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
    printk("[TSP][MMS128][%s] \n", __FUNCTION__);

    gpio_direction_output( GPIO_TSP_SCL , 1 ); 
    gpio_direction_output( GPIO_TSP_SDA , 1 ); 
    //gpio_direction_output( TSP_INT , 1 ); 

    gpio_direction_input(GPIO_TOUCH_INT);
    bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, false);

#ifdef DELAY_BEFORE_VDD
    msleep(10);
#endif

    touch_ctrl_regulator_mms128(TOUCH_ON);
    msleep(70);
    enable_irq(ts->client->irq);
    //ts = container_of(h, struct melfas_ts_data, early_suspend);
    //melfas_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id melfas_ts_id[] =
{
    { MELFAS_TS_NAME, 0 },
    { }
};

static struct i2c_driver melfas_ts_driver =
{
    .probe	 = melfas_ts_probe,
    .remove = __devexit_p(melfas_ts_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
    .suspend	= melfas_ts_suspend,
    .resume	= melfas_ts_resume,
#endif
    .id_table	= melfas_ts_id,
    .driver = {
        .name	= MELFAS_TS_NAME,
    },
};

static int __devinit melfas_ts_init(void)
{
#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
    if (Is_Synaptics_Connected() == 1)
    {
        printk("[TSP][MMS128][%s] %s\n", __func__, "Synaptics already detected !!");
        return -ENXIO;
    }
#endif
 
#if USE_THREADED_IRQ

#else	
    melfas_wq = create_workqueue("melfas_wq");
    if (!melfas_wq)
        return -ENOMEM;
#endif

    touch_regulator = regulator_get(NULL, "touch_vcc");

#if defined (__TOUCH_KEYLED__)
    touchkeyled_regulator = regulator_get(NULL,"touch_keyled");
#endif

    return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
    if (touch_regulator)
    {
        regulator_put(touch_regulator);
        touch_regulator = NULL;
    }


#if defined (__TOUCH_KEYLED__)
    if (touchkeyled_regulator) 
    {
        regulator_put(touchkeyled_regulator);
        touchkeyled_regulator = NULL;
    }
#endif

    i2c_del_driver(&melfas_ts_driver);

#if USE_THREADED_IRQ

#else
    if (melfas_wq)
        destroy_workqueue(melfas_wq);
#endif
}

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
    u8 buf1[2] = {0,};
    int hw_rev, fw_ver, phone_ver;

    if (0 == melfas_i2c_read(ts->client, MCSTS_MODULE_VER_REG, buf1, 2))
    {
        hw_rev = buf1[0];
        fw_ver = buf1[1];	 
 	  phone_ver = 1;
	 
/*
	TSP phone Firmware version : hw_rev  (xx)
	TSP IC	  Firmware version   : fw_ver (xx)
	HW 				  version	  : phone_ver (xxx)
*/
	
	 sprintf(buf, "%03X%02X%02X\n", phone_ver,fw_ver,hw_rev); // 10003xx
	 
       printk("[TSP][MMS128][%s]  phone_ver=%d, fw_ver=%d, hw_rev=%d\n",buf, phone_ver,fw_ver, hw_rev );
	   
    }
    else
    {	 
        printk("[TSP][MMS128][%s] Can't find HW Ver, FW ver!\n", __func__);
    }

    return sprintf(buf, "%s", buf); 
}

static ssize_t firmware_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
    char *after;
    int ret;

    unsigned long value = simple_strtoul(buf, &after, 10);	
    printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);
    firmware_ret_val = -1;

    printk("[TSP] firmware_store  valuie : %d\n",value);
    if ( value == 1 )
    {
        printk("[TSP] Firmware update start!!\n" );

        //firm_update( );
	//	  melfas_upgrade(ts->hw_rev);

		disable_irq(ts->client->irq);
		local_irq_disable();
	
		ret = mms100_ISC_download_binary_data();

		local_irq_enable();
    	enable_irq(ts->client->irq);

		if(ret == MCSDL_RET_SUCCESS)
			firmware_ret_val = 1;
		else
			firmware_ret_val = 0;	

        printk("[TSP] Firmware update end!!\n" );		

        return size;
    }

    return size;
}

static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
    printk("[TSP][MMS128][%s] !\n", __func__);

    return sprintf(buf, "%d", firmware_ret_val );
}

static ssize_t firmware_ret_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
    printk("[TSP][MMS128][%s] operate nothing!\n", __func__);

    return size;
}

#if 0 //TSP_TEST_MODE
static ssize_t tsp_test_inspection_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    printk(KERN_DEBUG "Reference START %s\n", __func__) ;

    return sprintf(buf, "%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d \n",
                   tsp_test_inspection[inspec_y_channel_num][0], tsp_test_inspection[inspec_y_channel_num][1], tsp_test_inspection[inspec_y_channel_num][2], tsp_test_inspection[inspec_y_channel_num][3],
                   tsp_test_inspection[inspec_y_channel_num][4], tsp_test_inspection[inspec_y_channel_num][5], tsp_test_inspection[inspec_y_channel_num][6], tsp_test_inspection[inspec_y_channel_num][7],
                   tsp_test_inspection[inspec_y_channel_num][8], tsp_test_inspection[inspec_y_channel_num][9]);
}

static ssize_t tsp_test_inspection_store(struct device *dev, struct device_attribute *attr, char *buf, size_t size)
{
    unsigned int position;
    int j, i, ret;
    uint16_t ref_value;
    uint8_t buf1[2], buff[20];

    sscanf(buf, "%d\n", &position);

    if (position == 100)
    {
        inspec_test_cnt = 0;
        printk("reset_reference_value\n");
    }

    if (!inspec_test_cnt)
    {

        /* disable TSP_IRQ */
        disable_irq(ts->client->irq);
        for (i = 0;i < 14;i++)
        {
            for (j = 0;j < 10;j++)
            {
                buf1[0] = 0xA0 ;		/* register address */
                buf1[1] = 0x42 ;
                buf1[2] = i;
                buf1[3] = j;

                if (melfas_i2c_write(ts->client, buf1, 4) != 0)
                {
                    printk(KERN_DEBUG "Failed to enter testmode\n") ;
                }

                while (1)
                {
                    if (MCSDL_GPIO_RESETB_IS_HIGH() == 0) //TSP INT Low
                        break;
                }

                if (melfas_i2c_read(ts->client, 0xAE, buff, 1) != 0)
                {
                    printk(KERN_DEBUG "Failed to read(referece data)\n") ;
                }

                if (melfas_i2c_read(ts->client, 0xAF, buff, 2) != 0)
                {
                    printk(KERN_DEBUG "Failed to read(referece data)\n") ;
                }

                printk("ref value0=%x\n", buff[0]);
                printk("ref value1=%x\n", buff[1]);

                ref_value = (uint16_t)(buff[1] << 8) | buff[0] ;
                tsp_test_inspection[i][j] = ref_value;
                printk("ins value[%d]=%d\n", i, ref_value);
                inspec_test_cnt = 1;
            }
        }
        mcsdl_vdd_off();
        mdelay(50);
        mcsdl_vdd_on();
        mdelay(250);
        printk("[TOUCH] reset.\n");
        /* enable TSP_IRQ */
        enable_irq(ts->client->irq);
    }

    if (position < 0 || position > 14)
    {
        printk(KERN_DEBUG "Invalid values\n");
        return -EINVAL;
    }

    inspec_y_channel_num = (uint8_t)position;

    return size;
}

static DEVICE_ATTR(tsp_inspection, 0664, tsp_test_inspection_show, tsp_test_inspection_store);
#endif

MODULE_DESCRIPTION("Driver for Melfas MTSI Touchscreen Controller");
MODULE_AUTHOR("MinSang, Kim <kimms@melfas.com>");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

module_init(melfas_ts_init);
module_exit(melfas_ts_exit);
