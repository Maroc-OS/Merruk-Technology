/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
* 
* 	@file	drivers/sound/brcm/headset/brcm_headset.c
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


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <mach/irqs.h>
#include <linux/irq.h>
#include <linux/version.h>
#include <linux/broadcom/bcm_major.h>
#include <plat/brcm_headset_pd.h>
#include <plat/syscfg.h>
#include <cfg_global.h>
#include "brcm_headset_hw.h"
#include <mach/gpio.h>
#include <linux/string.h>
#ifdef CONFIG_SWITCH
#include <linux/switch.h>
#endif
#if defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif /*CONFIG_HAS_WAKELOCK*/

#define REF_TIME 300000000
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)
#define KEY_BEFORE_PRESS_REF_TIME msecs_to_jiffies(80)
#else
#define KEY_BEFORE_PRESS_REF_TIME msecs_to_jiffies(50)
#endif
#define KEY_PRESS_REF_TIME msecs_to_jiffies(5)
#define TYPE_DETECT_REF_TIME msecs_to_jiffies(100)
#define GET_IMSI_REF_TIME msecs_to_jiffies(8000)
#define MIC_PLUGIN_MASK	0x7f00
#define REG_ANACR12 0x088800b0

#undef REG_DEBUG
//#define REG_DEBUG
#ifdef REG_DEBUG
#define REG_ANACR2      	0x08880088
#define REG_AUXMIC_CMC  	0x0891100c
#define REG_AUXMIC_AUXEN	0x08911014
#endif

#define MAX_DEBOUNCE_NUM 5
#define MAX_KEYS 3
#define KEYCOUNT_THRESHOLD	2

#define HEADSET_4_POLE	1
#define HEADSET_3_POLE 	2
#define PRESS 1
#define RELEASE 0
#define ENABLE 1
#define DISABLE 0
#define WAKE_LOCK_TIME		(HZ * 5)	/* 5 sec */

static const int hs_keycodes[] = {
	KEY_BCM_HEADSET_BUTTON,
	KEY_VOLUMEUP,
	KEY_VOLUMEDOWN,
	//KEY_SEARCH,
};

static int KEY_PRESS_THRESHOLD;
static int KEY_3POLE_THRESHOLD;
static int KEY1_THRESHOLD_L;
static int KEY1_THRESHOLD_U;
static int KEY2_THRESHOLD_L;
static int KEY2_THRESHOLD_U;
static int KEY3_THRESHOLD_L;
static int KEY3_THRESHOLD_U;

static int key_count[3];
static int key_resolved = 0;
static int key_type = 0;
static int FactoryMode = DISABLE;

extern int sync_use_mic;
extern void set_button(int value);
extern int auxadc_access(int);
extern int bcm_gpio_set_db_val(unsigned int gpio, unsigned int db_val);
extern SIMLOCK_SIM_DATA_t* GetSIMData(void);

#ifdef CONFIG_SWITCH
static void switch_work(struct work_struct *);
static void type_work_func(struct work_struct *work);
#endif
static void input_work_func(struct work_struct *work);

struct h2w_switch_data {
#ifdef CONFIG_SWITCH
	struct switch_dev sdev;
#endif
	struct work_struct work;
};

struct mic_t
{
	int hsirq;
	int hsbirq;
	int hsbst;
	struct auxmic *pauxmic; 
	int headset_state;
	ktime_t hstime;
	ktime_t hsbtime;
	int hsmajor;
	int check_count;
	int hsirq_triggerred;
	struct h2w_switch_data switch_data;
	struct input_dev *headset_button_idev;
	struct delayed_work input_work;
#ifdef CONFIG_SWITCH
	struct delayed_work type_work;
#endif
	struct brcm_headset_pd	*headset_pd;
	struct timer_list timer;
	struct delayed_work imsi_work;
	struct wake_lock det_wake_lock;
};

static struct mic_t mic;

static inline int check_delta(ktime_t a, ktime_t b)
{
	int ret = -1;
	ktime_t res = ktime_sub(a,b);
	if(res.tv.sec >= 0){
		/* HSB after HS event -- normal case*/
		if((res.tv.sec >= 1) || (res.tv.nsec > REF_TIME)){
			/* The delay is greater than 800 msec .. so fine*/
			ret = 0;
		} else {
			/* Delay is < 800 msec so HSB must be spurious*/
			ret = -1;
		}
	} else {
		/* HSB event before HS event
		* 1) Could be HSB happened and then HS was removed
		*  2) Could be a spurious event on HS removal or insertion
		* In any case let us reject it*/
		ret = -1;
	}
	return ret;
}

static void input_work_func(struct work_struct *work)
{
	int delta = 0;
	unsigned long val = 0;
#ifdef REG_DEBUG
	unsigned long val_anacr2, val_cmc, val_auxen;
#endif
	static int key_status = RELEASE;
	int adc_value = auxadc_access(2);	
	printk("%s: input_work adc_value=%d\n", __func__, adc_value);

	/* If the key_status is 0, send the event for a key press */
	if(key_status == RELEASE)
	{
		delta = check_delta(mic.hsbtime,mic.hstime);
		if((mic.headset_state == HEADSET_4_POLE) && (delta == 0))
		{
			if (!key_resolved)
			{
				if ( adc_value >= KEY1_THRESHOLD_L && adc_value < KEY1_THRESHOLD_U )
				{
					key_count[0]++;
					if (key_count[0] >= KEYCOUNT_THRESHOLD)
						key_resolved = 1;
					key_type = KEY_BCM_HEADSET_BUTTON;
					printk ("KEY_BCM_HEADSET_BUTTON \n");
				}
				if ( adc_value >= KEY2_THRESHOLD_L && adc_value < KEY2_THRESHOLD_U ) 
				{	
					key_count[1]++;
					if (key_count[1] >= KEYCOUNT_THRESHOLD)
						key_resolved = 1;
					key_type = KEY_VOLUMEUP;
					printk ("KEY_VOLUMEUP \n");
				}
				if ( adc_value >= KEY3_THRESHOLD_L && adc_value < KEY3_THRESHOLD_U ) 
				{
					key_count[2]++;
					if (key_count[2] >= KEYCOUNT_THRESHOLD)
						key_resolved = 1;
					key_type = KEY_VOLUMEDOWN;
					printk ("KEY_VOLUMEDOWN \n");
				}
			}
			else
				input_report_key(mic.headset_button_idev, key_type, PRESS);

			input_sync(mic.headset_button_idev);
			printk("%s: set_button => PRESS\n", __func__);
			set_button(PRESS);
			printk("%s: button pressed : ear_adc=%d\n", __func__, adc_value);
		}
	}

	/* Check if the value read from ANACR12 is greater than the
	* threshold. If so, the key is still pressed and schedule the work
	* queue till the value is less than the threshold */
	val = readl(io_p2v(REG_ANACR12));
	printk("%s: REG_ANACR12=%x\n", __func__,val);

	if (val >= KEY_PRESS_THRESHOLD && (adc_value >= KEY1_THRESHOLD_L && adc_value < KEY3_THRESHOLD_U))
	{
		key_status = PRESS;

		if (!key_resolved)
		{
			if ( adc_value >= KEY1_THRESHOLD_L && adc_value < KEY1_THRESHOLD_U )
			{
				key_count[0]++;
				if (key_count[0] >= KEYCOUNT_THRESHOLD)
					key_resolved = 1;
				key_type = KEY_BCM_HEADSET_BUTTON;
				printk ("KEY_BCM_HEADSET_BUTTON\n");
			}
			if ( adc_value >= KEY2_THRESHOLD_L && adc_value < KEY2_THRESHOLD_U ) 
			{	
				key_count[1]++;
				if (key_count[1] >= KEYCOUNT_THRESHOLD)
					key_resolved = 1;
				key_type = KEY_VOLUMEUP;
				printk ("KEY_VOLUMEUP\n");
			}
			if ( adc_value >= KEY3_THRESHOLD_L && adc_value < KEY3_THRESHOLD_U ) 
			{
				key_count[2]++;
				if (key_count[2] >= KEYCOUNT_THRESHOLD)
					key_resolved = 1;
				key_type = KEY_VOLUMEDOWN;
				printk ("KEY_VOLUMEDOWN\n");
			}
		}
		else
			input_report_key(mic.headset_button_idev, key_type, PRESS);

		schedule_delayed_work(&(mic.input_work), KEY_PRESS_REF_TIME);
		printk("%s: set_button => PRESS\n", __func__);
		set_button(PRESS);
	}
	/* Once the value read from ANACR12 is less than the threshold, send
	* the event for a button release */
	else
	{
		key_status = RELEASE;
		printk ("%s: key_count [%d, %d, %d] \n", __func__, key_count[0], key_count[1], key_count[2]);

		if ( key_count[0] >= KEYCOUNT_THRESHOLD )
		{
			printk("SEND/END RELEASE\n");
			input_report_key(mic.headset_button_idev, KEY_BCM_HEADSET_BUTTON, RELEASE);
			input_sync(mic.headset_button_idev);
		}
		else if ( key_count[1] >= KEYCOUNT_THRESHOLD )
		{	
			printk("VOLUP RELEASE\n");
			input_report_key(mic.headset_button_idev, KEY_VOLUMEUP, RELEASE);
			input_sync(mic.headset_button_idev);
		}
		else if ( key_count[2] >= KEYCOUNT_THRESHOLD )
		{
			printk("VOLDOWN RELEASE\n");
			input_report_key(mic.headset_button_idev, KEY_VOLUMEDOWN, RELEASE);
			input_sync(mic.headset_button_idev);
		}

		printk("%s: set_button => RELEASE\n", __func__);
		set_button(RELEASE);	 
		key_resolved = 0;
		key_count[0] = key_count[1] = key_count[2] = 0;

		if(FactoryMode == DISABLE)
		{
			board_sysconfig(SYSCFG_AUXMIC, SYSCFG_ENABLE | SYSCFG_DISABLE);
		sync_use_mic = DISABLE;
		}

#ifdef REG_DEBUG
		val_anacr2 = readl(io_p2v(REG_ANACR2));
		val_cmc = readl(io_p2v(REG_AUXMIC_CMC));
		val_auxen = readl(io_p2v(REG_AUXMIC_AUXEN));
		printk("%s: REG_ANACR2=%x, REG_AUXMIC_CMC=%x, REG_AUXMIC_AUXEN=%x\n", __func__, val_anacr2, val_cmc, val_auxen);
#endif
	}
}


#ifdef CONFIG_SWITCH
// Switch class work to update state of headset
static void switch_work(struct work_struct *work)
{
#ifdef REG_DEBUG
	unsigned long val_anacr2, val_cmc, val_auxen;
	val_anacr2 = readl(io_p2v(REG_ANACR2));
	val_cmc = readl(io_p2v(REG_AUXMIC_CMC));
	val_auxen = readl(io_p2v(REG_AUXMIC_AUXEN));
	printk("%s: REG_ANACR2=%x, REG_AUXMIC_CMC=%x, REG_AUXMIC_AUXEN=%x\n", __func__, val_anacr2, val_cmc, val_auxen);
#endif

	if(mic.headset_state)
	{
		if(mic.hsbst == DISABLE)
		{
			mic.hsbst = ENABLE;
			board_sysconfig(SYSCFG_AUXMIC, SYSCFG_ENABLE);			
			schedule_delayed_work(&(mic.type_work), TYPE_DETECT_REF_TIME);
		}
	}
	else
	{
		if(mic.hsbst == ENABLE)
		{
			board_sysconfig(SYSCFG_AUXMIC, SYSCFG_DISABLE);

			if (mic.headset_pd->check_hs_state)
				board_sysconfig(SYSCFG_HEADSET, SYSCFG_DISABLE);

			mic.hsbst = DISABLE;
			sync_use_mic = DISABLE;
			switch_set_state(&(mic.switch_data.sdev), mic.headset_state);

			printk("%s: plugged out\n", __func__);
		}
	}
}

static void type_work_func(struct work_struct *work)
{
#ifdef REG_DEBUG
	unsigned long val_anacr2, val_cmc, val_auxen;
#endif

	int adc = auxadc_access(2);

	if(adc>=KEY_3POLE_THRESHOLD)
	{
		if(FactoryMode == DISABLE)
			board_sysconfig(SYSCFG_AUXMIC, SYSCFG_ENABLE | SYSCFG_DISABLE);

		if (mic.headset_pd->check_hs_state)
			board_sysconfig(SYSCFG_HEADSET, SYSCFG_ENABLE);

		mic.headset_state = HEADSET_4_POLE;
		printk("%s: 4-pole inserted : ear_adc=%d\n", __func__, adc);
	}
	else
	{
		board_sysconfig(SYSCFG_AUXMIC, SYSCFG_DISABLE);

		mic.headset_state = HEADSET_3_POLE;
		printk("%s: 3-pole inserted : ear_adc=%d\n", __func__, adc);
	}

	switch_set_state(&(mic.switch_data.sdev), mic.headset_state);

	if(FactoryMode == DISABLE)
	sync_use_mic = DISABLE;

#ifdef REG_DEBUG
	val_anacr2 = readl(io_p2v(REG_ANACR2));
	val_cmc = readl(io_p2v(REG_AUXMIC_CMC));
	val_auxen = readl(io_p2v(REG_AUXMIC_AUXEN));
	printk("%s: REG_ANACR2=%x, REG_AUXMIC_CMC=%x, REG_AUXMIC_AUXEN=%x\n", __func__, val_anacr2, val_cmc, val_auxen);
#endif

}
#endif

static void gpio_jack_timer(unsigned long _data)
{
	struct mic_t *p_mic = (struct mic_t *)_data;
	unsigned long flags = 0;
	int last_state = 0;
	int int_triggerred = 0; 
	int attached = 0;

	local_irq_save(flags);
	int_triggerred = p_mic->hsirq_triggerred;
	
	if(p_mic->hsirq_triggerred)
	       p_mic->hsirq_triggerred = 0; 
	local_irq_restore(flags);

	if(int_triggerred)
	       p_mic->check_count = 0;

	last_state = p_mic->headset_state;

	if(p_mic->headset_pd->check_hs_state)
		p_mic->headset_pd->check_hs_state(&attached); // get gpio state.

	if(attached != last_state)
	{
		if(p_mic->check_count >= MAX_DEBOUNCE_NUM)
		{
			p_mic->headset_state = attached;
			printk("%s: schedule_work: %d, last_state:%d, will change to %d \n", \
				__func__, p_mic->check_count,last_state, p_mic->headset_state); 
			schedule_work(&(p_mic->switch_data.work));
		}
		else
		{
			mod_timer(&p_mic->timer,jiffies + msecs_to_jiffies(p_mic->headset_pd->debounce_ms));
			p_mic->check_count++;
		}
	}
}

static void getIMSI_work_func(struct work_struct *work)
{
	SIMLOCK_SIM_DATA_t* simdata = GetSIMData();
	
	if(simdata == NULL)
	{
		//printk("%s: IMSI NULL\n", __func__);
		FactoryMode = DISABLE;
	}
	else
	{
		//printk("%s: IMSI %s\n", __func__, simdata->imsi_string);
		FactoryMode = strncmp(simdata->imsi_string, "999999999999999", IMSI_DIGITS) == 0 ?  ENABLE : DISABLE;
	}
	
	printk("%s: Factorymode %d\n", __func__, FactoryMode);

	if(FactoryMode == ENABLE)
	{
		if(mic.headset_state)
		{
			if(mic.hsbst == ENABLE && mic.headset_state == HEADSET_4_POLE)
				board_sysconfig(SYSCFG_AUXMIC, SYSCFG_ENABLE);
		}
	}
}

/*------------------------------------------------------------------------------
Function name   : hs_isr
Description     : interrupt handler

Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t hs_isr(int irq, void *dev_id)
{
	struct mic_t *p = (struct mic_t *)dev_id;
	int attached_state = 0; 

	p->hstime = ktime_get();	
        wake_lock_timeout(&p->det_wake_lock, WAKE_LOCK_TIME);

	if (p->headset_pd->check_hs_state)
	{
		sync_use_mic = ENABLE;
		p->hsirq_triggerred = 1 ;
		p->headset_pd->check_hs_state(&attached_state);
		set_irq_type(mic.hsirq, (attached_state) ? IRQF_TRIGGER_RISING : IRQF_TRIGGER_FALLING);
		mod_timer(&p->timer,jiffies + msecs_to_jiffies(p->headset_pd->debounce_ms));
	}
	else
	{
		unsigned long int val = readl(io_p2v(REG_ANACR12));

		val = val & MIC_PLUGIN_MASK;
		/*If the value read from anacr12 is zero and still the ISR is invoked, then the interrupt is
		* deemed illegal*/
		if(!val)
		      return IRQ_NONE;

		p->headset_state = (p->headset_state) ? 0 : 1;
		set_irq_type(mic.hsirq, (p->headset_state) ? IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING);
		sync_use_mic = ENABLE;
		schedule_work(&(p->switch_data.work));
	}

	return IRQ_HANDLED; 
}

/*------------------------------------------------------------------------------
Function name   : hs_buttonisr
Description     : interrupt handler

Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t hs_buttonisr(int irq, void *dev_id)
{
	ktime_t r, temp;
	unsigned int val = 0;
#ifdef REG_DEBUG
	unsigned long val_anacr2, val_cmc, val_auxen;
#endif

	printk ("%s: HS_BUTTONISR (<<<) : status=%d \n", __func__, mic.hsbst);

	if (mic.hsbst == DISABLE || mic.headset_state != HEADSET_4_POLE)
		return IRQ_HANDLED;

	/* Read the ANACR12 register value to check if the interrupt being
	* serviced by the ISR is spurious */
	val = readl(io_p2v(REG_ANACR12));
	temp = ktime_get();
	r = ktime_sub(temp,mic.hsbtime);
	if((r.tv.sec > 0) || (r.tv.nsec > REF_TIME))
	{
		mic.hsbtime = temp;
	}
	else
	{
		printk ("%s: HS_BUTTONISR appeared frequently (r.tv.sec=%d, r.tv.nsec=%d) status=%d \n", __func__, r.tv.sec, r.tv.nsec, mic.hsbst);
		// return IRQ_HANDLED;
	}
	/* If the value read from the ANACR12 register is greater than the
	* threshold, schedule the workqueue */
	printk("%s: REG_ANACR12=%x\n", __func__,val);

#ifdef REG_DEBUG
	val_anacr2 = readl(io_p2v(REG_ANACR2));
	val_cmc = readl(io_p2v(REG_AUXMIC_CMC));
	val_auxen = readl(io_p2v(REG_AUXMIC_AUXEN));
	printk("%s: REG_ANACR2=%x, REG_AUXMIC_CMC=%x, REG_AUXMIC_AUXEN=%x\n", __func__, val_anacr2, val_cmc, val_auxen);
#endif

	if (val >= KEY_PRESS_THRESHOLD)
	{
		board_sysconfig(SYSCFG_AUXMIC, SYSCFG_ENABLE);

		key_resolved = 0;
		key_count[0] = key_count[1] = key_count[2] = 0;

		sync_use_mic = ENABLE;
		schedule_delayed_work(&(mic.input_work), KEY_BEFORE_PRESS_REF_TIME);
		printk("%s: set_button => PRESS\n", __func__);
		set_button(PRESS); 
	}
	else
	{
		pr_info("Headset Button press detected for a illegal interrupt\n");
		printk("%s: set_button => RELEASE\n", __func__);
		set_button(RELEASE);
	}

	printk ("%s: HS_BUTTONISR (>>>) : status=%d \n", __func__, mic.hsbst);

	return IRQ_HANDLED;
}

/*------------------------------------------------------------------------------
Function name   : hs_switchinit
Description     : Register sysfs device for headset
It uses class switch from kernel/common/driver/switch
Return type     : int
------------------------------------------------------------------------------*/
#ifdef CONFIG_SWITCH
int hs_switchinit(struct mic_t *p)
{
	int result = 0;
	p->switch_data.sdev.name = "h2w";

	result = switch_dev_register(&p->switch_data.sdev);
	if (result < 0)
	{
		return result;
	}
	INIT_WORK(&(p->switch_data.work), switch_work);
	INIT_DELAYED_WORK(&(mic.type_work), type_work_func);
	return 0;
}
#endif

/*------------------------------------------------------------------------------
Function name   : hs_inputdev
Description     : Create and Register input device for headset button
Return type     : int
------------------------------------------------------------------------------*/
int  hs_inputdev(struct mic_t *p)
{
	int key = 0;
	int result = 0;

	// Allocate struct for input device
	p->headset_button_idev = input_allocate_device();
	if (p->headset_button_idev == NULL)
	{
		printk("%s: headset button: Not enough memory\n", __func__);
		return -1;
	}

	// specify key event type and value for it
	// we have only one button on headset so only one possible
	// value KEY_SEND used here.
	set_bit(EV_KEY, p->headset_button_idev->evbit);

	for(key = 0; key < MAX_KEYS ; key++)
		input_set_capability(p->headset_button_idev, EV_KEY, hs_keycodes[key]);

	p->headset_button_idev->name = "bcm_headset";
	p->headset_button_idev->phys = "headset/input0";
	p->headset_button_idev->id.bustype = BUS_HOST;
	p->headset_button_idev->id.vendor = 0x0001;
	p->headset_button_idev->id.product = 0x0100;

	// Register input device for headset
	result = input_register_device(p->headset_button_idev);
	
	if (result) {
		printk("%s: Failed to register device\n", __func__);
		return result;
	}

	INIT_DELAYED_WORK(&(mic.input_work), input_work_func);

	return 0;
}

/*------------------------------------------------------------------------------
Function name   : hs_unregsysfs
Description     : Unregister sysfs and input device for headset
Return type     : int
------------------------------------------------------------------------------*/
int hs_unregsysfs(struct mic_t *p)
{
	int result = 0;

#ifdef CONFIG_SWITCH
	result = cancel_work_sync(&(p->switch_data.work));
	if (result)
		return result;

	switch_dev_unregister(&p->switch_data.sdev);
#endif

	return 0;
}

int hs_unreginputdev(struct mic_t *p)
{
	input_unregister_device(p->headset_button_idev);
	return 0;
}

/*------------------------------------------------------------------------------
Function name   : BrcmHeadsetIoctl
Description     : communication method to/from the user space

Return type     : int
------------------------------------------------------------------------------*/
static int hs_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
	case BCM_HEADSET_IOCTL_STATUS:
		return mic.headset_state;
	}

	return 0;
}

/* File operations on HEADSET device */
static struct file_operations hs_fops = {
	ioctl:hs_ioctl,
};

static int hs_remove(struct platform_device *pdev)
{
	return 0;
}

static int __init hs_probe(struct platform_device *pdev)
{
	int result = 0;
	mic.hsmajor = 0;
	mic.headset_state = 0;
	mic.hsbtime.tv.sec = 0;
	mic.hsbtime.tv.nsec = 0;
	mic.headset_pd = NULL;
	mic.check_count = 0;

#ifdef CONFIG_SWITCH
	result = hs_switchinit(&mic);
	if (result < 0)
		return result;
#endif

	result = hs_inputdev(&mic);
	if (result < 0)
		goto err;

	result = register_chrdev(mic.hsmajor, "BrcmHeadset", &hs_fops);
	if(result < 0)
		goto err1;
	else if(result > 0 && (mic.hsmajor == 0))    /* this is for dynamic major */
		mic.hsmajor = result;

	wake_lock_init(&mic.det_wake_lock, WAKE_LOCK_SUSPEND, "sec_jack_det");
	INIT_DELAYED_WORK(&(mic.imsi_work), getIMSI_work_func);

	/* check if platform data is defined for a particular board variant */
	if (pdev->dev.platform_data)
	{
		mic.headset_pd = pdev->dev.platform_data;

		KEY_PRESS_THRESHOLD = mic.headset_pd->key_press_threshold;
		KEY_3POLE_THRESHOLD = mic.headset_pd->key_3pole_threshold;
		KEY1_THRESHOLD_L = mic.headset_pd->key1_threshold_l;
		KEY1_THRESHOLD_U = mic.headset_pd->key1_threshold_u;
		KEY2_THRESHOLD_L = mic.headset_pd->key2_threshold_l;
		KEY2_THRESHOLD_U = mic.headset_pd->key2_threshold_u;
		KEY3_THRESHOLD_L = mic.headset_pd->key3_threshold_l;
		KEY3_THRESHOLD_U = mic.headset_pd->key3_threshold_u;

		if (mic.headset_pd->hsgpio == NULL)
			mic.hsirq = mic.headset_pd->hsirq;
		else
		{
			setup_timer(&mic.timer, gpio_jack_timer, (unsigned long)&mic); // timer register. 

			if (gpio_request(mic.headset_pd->hsgpio, "headset detect") < 0)
			{
				printk("%s: Could not reserve headset signal GPIO!\n", __func__);
				goto err2;
			}
			gpio_direction_input(mic.headset_pd->hsgpio);
			bcm_gpio_set_db_val(mic.headset_pd->hsgpio, 0x7);
			mic.hsirq = gpio_to_irq(mic.headset_pd->hsgpio);
		}
		mic.hsbirq = mic.headset_pd->hsbirq;
	}
	else
	{
		mic.hsirq = platform_get_irq(pdev, 0);
		mic.hsbirq = platform_get_irq(pdev, 1);
	}

	printk("%s: HS irq %d\n", __func__, mic.hsirq);
	printk("%s: HSB irq %d\n", __func__, mic.hsbirq);
	result = request_irq(mic.hsbirq, hs_buttonisr,  IRQF_NO_SUSPEND, "BrcmHeadsetButton",  (void *) NULL);
	mic.hsbst = DISABLE;

	if(result < 0)
		goto err2;

	result = request_irq(mic.hsirq, hs_isr,(IRQF_DISABLED | IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND), "BrcmHeadset",  &mic);
	if(result < 0)
	{
		free_irq(mic.hsbirq, &mic);
		goto err2;
	}

	printk("%s: BrcmHeadset: module inserted >>>> . Major number is = %d\n", __func__, mic.hsmajor);

	/* Set the ANACR2 bit for mic power down */
	board_sysconfig(SYSCFG_AUXMIC, SYSCFG_INIT);
	board_sysconfig(SYSCFG_HEADSET, SYSCFG_INIT);

	/*Fix the audio path is wrong when headset already plugged in the device  then boot device case.*/
	if (mic.headset_pd->hsgpio != NULL)
	{
		mic.headset_pd->check_hs_state(&mic.headset_state);
		printk("%s: headset_state:%d\n", __func__, mic.headset_state); 
		set_irq_type(mic.hsirq, (mic.headset_state) ? IRQF_TRIGGER_RISING : IRQF_TRIGGER_FALLING);
		schedule_work(&(mic.switch_data.work));
		schedule_delayed_work(&(mic.imsi_work), GET_IMSI_REF_TIME);
	}

	return 0;

err2:   unregister_chrdev(mic.hsmajor,"BrcmHeadset");
	if(mic.headset_pd->hsgpio)
		del_timer_sync(&mic.timer);
err1:   hs_unreginputdev(&mic);
err:    hs_unregsysfs(&mic);
	return result;
}

static struct platform_driver headset_driver = {
	.probe = hs_probe,
	.remove = hs_remove,
	.driver = {
		.name = "bcmheadset",
		.owner = THIS_MODULE,
	},
};

/*------------------------------------------------------------------------------
Function name   : BrcmHeadsetModuleInit
Description     : Initialize the driver

Return type     : int
------------------------------------------------------------------------------*/
int __init BrcmHeadsetModuleInit(void)
{
	return platform_driver_register(&headset_driver);
}

/*------------------------------------------------------------------------------
Function name   : BrcmHeadsetModuleExit
Description     : clean up

Return type     : int
------------------------------------------------------------------------------*/
void __exit BrcmHeadsetModuleExit(void)
{
	// Cancel work of event notification to UI and unregister switch dev.
#ifdef CONFIG_SWITCH	
	cancel_work_sync(&mic.switch_data.work);
	switch_dev_unregister(&mic.switch_data.sdev);
#endif
	cancel_work_sync(&mic.imsi_work);
	if(mic.headset_pd->hsgpio)
		del_timer_sync(&mic.timer);
	free_irq(mic.hsirq, &mic.switch_data);
	free_irq(mic.hsbirq, NULL);
	input_unregister_device(mic.headset_button_idev);	
	unregister_chrdev(mic.hsmajor,"BrcmHeadset");
	return platform_driver_unregister(&headset_driver);
}

module_init(BrcmHeadsetModuleInit);
module_exit(BrcmHeadsetModuleExit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Headset plug and button detection");
