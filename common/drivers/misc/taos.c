 /*****************************************************************************
 *
 * Title: Linux Device Driver for Proximity Sensor TAOSP002S00F
 * COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2015 ALL RIGHTS RESERVED
 *
 *****************************************************************************/
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <mach/hardware.h>
#include <linux/wakelock.h>
#include <asm/gpio.h>  

 #include <linux/taos.h>
 #include <linux/regulator/consumer.h>

#define GPIO_PROXI_EN	0
#define GPIO_PROXI_INT	29
#define PROX_IRQ	gpio_to_irq(GPIO_PROXI_INT)

#define TAOS_DEBUG 0

/*********** for debug **********************************************************/
#if TAOS_DEBUG 
#define PROXDBG(fmt, args...) printk(fmt, ## args)
#else
#define PROXDBG(fmt, args...)
#endif
/*******************************************************************************/

/* Triton register offsets */
#define CNTRL				0x00
#define ALS_TIME			0X01
#define PRX_TIME			0x02
#define WAIT_TIME			0x03
#define ALS_MINTHRESHLO			0X04
#define ALS_MINTHRESHHI			0X05
#define ALS_MAXTHRESHLO			0X06
#define ALS_MAXTHRESHHI			0X07
#define PRX_MINTHRESHLO			0X08
#define PRX_MINTHRESHHI			0X09
#define PRX_MAXTHRESHLO			0X0A
#define PRX_MAXTHRESHHI			0X0B
#define INTERRUPT			0x0C
#define PRX_CFG				0x0D
#define PRX_COUNT			0x0E
#define GAIN				0x0F
#define REVID				0x11
#define CHIPID				0x12
#define STATUS				0x13
#define ALS_CHAN0LO			0x14
#define ALS_CHAN0HI			0x15
#define ALS_CHAN1LO			0x16
#define ALS_CHAN1HI			0x17
#define PRX_LO				0x18
#define PRX_HI				0x19
#define TEST_STATUS			0x1F

/* Triton cmd reg masks */
#define CMD_REG				0X80
#define CMD_BYTE_RW			0x00
#define CMD_WORD_BLK_RW			0x20
#define CMD_SPL_FN			0x60
#define CMD_PROX_INTCLR			0X05
#define CMD_ALS_INTCLR			0X06
#define CMD_PROXALS_INTCLR		0X07
#define CMD_TST_REG			0X08
#define CMD_USER_REG			0X09

/* Triton cntrl reg masks */
#define CNTL_REG_CLEAR			0x00
#define CNTL_PROX_INT_ENBL		0X20
#define CNTL_ALS_INT_ENBL		0X10
#define CNTL_WAIT_TMR_ENBL		0X08
#define CNTL_PROX_DET_ENBL		0X04
#define CNTL_ADC_ENBL			0x02
#define CNTL_PWRON			0x01
#define CNTL_ALSPON_ENBL		0x03
#define CNTL_INTALSPON_ENBL		0x13
#define CNTL_PROXPON_ENBL		0x0F
#define CNTL_INTPROXPON_ENBL		0x2F

/* Triton status reg masks */
#define STA_ADCVALID			0x01
#define STA_PRXVALID			0x02
#define STA_ADC_PRX_VALID		0x03
#define STA_ADCINTR			0x10
#define STA_PRXINTR			0x20

/* Lux constants */
#define	MAX_LUX				65535000
#define SCALE_MILLILUX			3
#define FILTER_DEPTH			3
#define	GAINFACTOR			9

/* Thresholds */
#define ALS_THRESHOLD_LO_LIMIT		0x0000
#define ALS_THRESHOLD_HI_LIMIT		0xFFFF
#define PROX_THRESHOLD_LO_LIMIT		0x0000
#define PROX_THRESHOLD_HI_LIMIT		0xFFFF

/* Device default configuration */
#define CALIB_TGT_PARAM			300000
#define ALS_TIME_PARAM			100
#define SCALE_FACTOR_PARAM		1
#define GAIN_TRIM_PARAM			512
#define GAIN_PARAM			1
#define ALS_THRSH_HI_PARAM		0xFFFF
#define ALS_THRSH_LO_PARAM		0

/* [HSS] This value depends on each model's H/W. It needs tunning */
#if (defined(CONFIG_MACH_COOPER) || defined(CONFIG_MACH_BENI))
	#define PRX_THRSH_HI_PARAM		550
	#define PRX_THRSH_LO_PARAM		400
#elif defined(CONFIG_MACH_TASS)
	#define PRX_THRSH_HI_PARAM		600
	#define PRX_THRSH_LO_PARAM		450
#elif defined(CONFIG_MACH_TASSVE)
	#define PRX_THRSH_HI_PARAM		550
	#define PRX_THRSH_LO_PARAM		400
#elif defined(CONFIG_MACH_COOPERVE)
	#define PRX_THRSH_HI_PARAM		550
	#define PRX_THRSH_LO_PARAM		400
#else			
	#define PRX_THRSH_HI_PARAM		0x226 //550 //0x2BC: 700
	#define PRX_THRSH_LO_PARAM		0x190 //400 //0x226 : 550
#endif
#define PRX_INT_TIME_PARAM		0xFC
#define PRX_ADC_TIME_PARAM		0xFF // [HSS] Original value : 0XEE
#define PRX_WAIT_TIME_PARAM		0xF2
#define INTR_FILTER_PARAM		0x33
#define PRX_CONFIG_PARAM		0x00
#if defined(CONFIG_MACH_BENI)
#define PRX_PULSE_CNT_PARAM		0x0A //0x0F
#else
#define PRX_PULSE_CNT_PARAM		0x08
#endif
#define PRX_GAIN_PARAM			0x20



/* global var */
static struct i2c_client *opt_i2c_client = NULL;
static bool proximity_enable = OFF;
static short proximity_value = 0;
static struct wake_lock prx_wake_lock;

static ktime_t timeA,timeB;
#if USE_INTERRUPT
static ktime_t timeSub;
#endif

#if !defined (CONFIG_BOARD_COOPERVE)
static struct regulator *prox_regulator=NULL;
static bool prox_power_mode = false;
#endif
static short prox_value_cnt = 0;

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int set_irq_type(unsigned int irq, unsigned int type);

static void prox_ctrl_regulator(int on_off)
{
#if !defined (CONFIG_BOARD_COOPERVE)
	if(on_off)
	{
                if(prox_power_mode) return;
        
		if(!regulator_is_enabled(prox_regulator))
		{
			regulator_set_voltage(prox_regulator,2900000,2900000);
			regulator_enable(prox_regulator);

                        printk(KERN_INFO "[TAOS] : prox_ctrl_regulator ON \n");        
                        
                        prox_power_mode = true;        

                        /*After Power Supply is supplied, about 1ms delay is required before issuing read/write commands */
                        mdelay(2);            
		}
	}
	else
	{
                if(!prox_power_mode) return;    
        
		if(regulator_is_enabled(prox_regulator))
		{
			regulator_disable(prox_regulator);
            
                        printk(KERN_INFO "[TAOS] : prox_ctrl_regulator OFF  \n");                            
		}        

                prox_power_mode = false;        
        
	}
#endif
}

/*************************************************************************/
/*		TAOS I2C_API	  				         */
/*************************************************************************/
/*  i2c read routine for taos  */
#if  0 //USE_INTERRUPT
static int opt_i2c_read(u8 reg, u8 *val, unsigned int len )
{
	int err;
	u8 buf[3];

	struct i2c_msg msg[2];


	buf[0] = reg; 

	msg[0].addr = opt_i2c_client->addr;
	msg[0].flags = 1;
	
	msg[0].len = 2;
	msg[0].buf = buf;
	err = i2c_transfer(opt_i2c_client->adapter, msg, 1);
	
	*val = buf[0] << 8 | buf[1];
	
    if (err >= 0) return 0;

    printk(KERN_ERR "[TAOS] %s %d i2c transfer error\n", __func__, __LINE__);
    return err;
}
#endif

/*  i2c write routine for taos */
static int opt_i2c_write( u8 reg, u8 *val )
{
    int err;
    struct i2c_msg msg[1];
    unsigned char data[2];

    if( opt_i2c_client == NULL )
        return -ENODEV;

    data[0] = reg;
    data[1] = *val;

    msg->addr = opt_i2c_client->addr;
    msg->flags = I2C_M_WR;
    msg->len = 2;
    msg->buf = data;

    err = i2c_transfer(opt_i2c_client->adapter, msg, 1);

    if (err >= 0) return 0;

    printk(KERN_ERR "[TAOS] %s %d i2c transfer error : reg = [%X]\n", __func__, __LINE__, reg);
    return err;
}


/*************************************************************************/
/*		TAOS sysfs	  				         */
/*************************************************************************/
int proximity_get_int_value(void)
{
        int int_value;

        PROXDBG("[GP2A] proximity_get_int_value GPIO_PS_OUT : %d\n", gpio_get_value(GPIO_PROXI_INT));     

        if(gpio_get_value(GPIO_PROXI_INT))
                int_value =1;
        else
                int_value =0;                

        return int_value;
        }
EXPORT_SYMBOL(proximity_get_int_value);
        
short taos_get_proximity_value()
{
	PROXDBG("[TAOS] taos_get_proximity_value called : %d\n",proximity_value); 

	return ((proximity_value == 1)? 0:1);
}

EXPORT_SYMBOL(taos_get_proximity_value);

#if 0
static ssize_t proxsensor_file_state_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned int detect = 0;

	PROXDBG("[TAOS] called %s \n",__func__);

	detect = gpio_get_value(GPIO_PROXI_INT);	

//	return sprintf(buf,"%u\n",detect);
	return detect;
}

static DEVICE_ATTR(proxsensor_file_state,0666, proxsensor_file_state_show, NULL);
#endif


/*****************************************************************************************
 *  
 *  function    : taos_work_func_prox 
 *  description : This function is for proximity sensor (using B-1 Mode ). 
 *                when INT signal is occured , it gets value from VO register.   
 *
 *                 
 */
#if USE_INTERRUPT
static void taos_work_func_prox(struct work_struct *work) 
{
	//struct taos_data *taos = container_of(work, struct taos_data, work_prox);
	unsigned char vout=0;
	u16 adc_data;
	u16 threshold_high;
	u16 threshold_low;
	u8 prox_int_thresh[4];
	int i;
	/* Read VO & INT Clear */
	
	PROXDBG("[TAOS] %s : \n",__func__);

	/* change Threshold */ 
	adc_data = i2c_smbus_read_word_data(opt_i2c_client, CMD_REG | PRX_LO);
	threshold_high= i2c_smbus_read_word_data(opt_i2c_client, (CMD_REG | PRX_MAXTHRESHLO) );
	threshold_low= i2c_smbus_read_word_data(opt_i2c_client, (CMD_REG | PRX_MINTHRESHLO) );
	if ( (threshold_high ==  (PRX_THRSH_HI_PARAM)) && (adc_data >=  (PRX_THRSH_HI_PARAM) ) )
	{
		PROXDBG("[TAOS] +++ adc_data=[%d], threshold_high=[%d],  threshold_min=[%d]\n", adc_data, threshold_high, threshold_low);

		proximity_value = 1;
		prox_int_thresh[0] = (PRX_THRSH_LO_PARAM) & 0xFF;
		prox_int_thresh[1] = (PRX_THRSH_LO_PARAM >> 8) & 0xFF;
		prox_int_thresh[2] = (0xFFFF) & 0xFF;
		prox_int_thresh[3] = (0xFFFF >> 8) & 0xFF; 
		for (i = 0; i < 4; i++)
		{
			opt_i2c_write((CMD_REG|(PRX_MINTHRESHLO + i)),&prox_int_thresh[i]);
		}
	}
	else if ( (threshold_high ==  (0xFFFF)) && (adc_data <=  (PRX_THRSH_LO_PARAM) ) )
	{

		PROXDBG("[TAOS] --- adc_data=[%d], threshold_high=[%d],  threshold_min=[%d]\n", adc_data, threshold_high, threshold_low);

		proximity_value = 0;
		prox_int_thresh[0] = (0x0000) & 0xFF;
		prox_int_thresh[1] = (0x0000 >> 8) & 0xFF;
		prox_int_thresh[2] = (PRX_THRSH_HI_PARAM) & 0xFF;
		prox_int_thresh[3] = (PRX_THRSH_HI_PARAM >> 8) & 0xFF; 
		for (i = 0; i < 4; i++)
		{
			opt_i2c_write((CMD_REG|(PRX_MINTHRESHLO + i)),&prox_int_thresh[i]);
		}
	}
        else
        {
            printk(KERN_INFO "[TAOS] Error! Not Common Case!adc_data=[%d], threshold_high=[%d],  threshold_min=[%d]\n", adc_data, threshold_high, threshold_low);
        }
      
	printk(KERN_INFO "[TAOS] proximity_value = %d \n",proximity_value);
      
	if(proximity_value ==0)
	{
		timeB = ktime_get();
		
		timeSub = ktime_sub(timeB,timeA);

		PROXDBG("[TAOS] timeSub sec = %d, timeSub nsec = %d \n",timeSub.tv.sec,timeSub.tv.nsec);
		
		if (timeSub.tv.sec>=3 )
		{
		    wake_lock_timeout(&prx_wake_lock,HZ/2);

		    PROXDBG("[TAOS] wake_lock_timeout : HZ/2 \n");
		}
		else
		{
		    PROXDBG("[TAOS] wake_lock is already set \n");
		}

	}

	if(USE_INPUT_DEVICE)
	{
		input_report_abs(taos_global->input_dev,ABS_DISTANCE,(int)vout);
		input_sync(taos_global->input_dev);
		mdelay(1);
	}

	/* reset Interrupt pin */
	/* to active Interrupt, TMD2771x Interuupt pin shoud be reset. */
	i2c_smbus_write_byte(opt_i2c_client,(CMD_REG|CMD_SPL_FN|CMD_PROXALS_INTCLR));

	/* enable INT */

	enable_irq(taos_global->irq);
	printk(KERN_INFO "[TAOS] enable_irq IRQ_NO:%d\n",taos_global->irq);
	

}


static irqreturn_t taos_irq_handler(int irq, void *dev_id)
{
  	printk(KERN_INFO "[TAOS] taos_irq_handler called\n");
	if(taos_global->irq !=-1)
	{
		disable_irq_nosync(taos_global->irq);
		printk(KERN_INFO "[TAOS] disable_irq : IRQ_NO:%d\n", taos_global->irq);
		queue_work(taos_wq, &taos_global->work_prox);
	}
        else
        {
                printk(KERN_ERR "[TAOS] PROX_IRQ not handled");
        }

	return IRQ_HANDLED;
}
#endif



void taos_chip_init(void)
{
	
	PROXDBG("[TAOS] %s called\n",__func__); 
#if !defined (CONFIG_BOARD_COOPERVE)
	prox_regulator = regulator_get(NULL,"prox_vcc");
#endif	
}



/*****************************************************************************************
 *  
 *  function    : taos_on 
 *  description : This function is power-on function for optical sensor.
 *
 *  int type    : Sensor type. Two values is available (PROXIMITY,LIGHT).
 *                it support power-on function separately.
 *                
 *                 
 */

void taos_on(struct taos_data *taos, int type)
{
	u8 value;
	u8 prox_int_thresh[4];
	int err = 0;
	int i;
	
	printk(KERN_INFO "[TAOS] taos_on(%d)\n",type);

	prox_ctrl_regulator(1);

#if 0	
	//register irq to wakeup source
	printk("[TAOS] register irq = %d\n",taos ->irq);
	err = set_irq_wake(taos ->irq, 1);  // enable : 1, disable : 0
	printk("[TAOS] register wakeup source = %d\n",err);
	if (err) 
		printk("[TAOS] register wakeup source failed\n");
#endif
	if(type == PROXIMITY)
	{
		enable_irq(taos_global ->irq);
		PROXDBG("[TAOS] enable irq for proximity\n");        
	}

        mdelay(12); // [HSS] Fix Proximity I2C Fail : Add delay 12ms after VDD ON

	value = CNTL_REG_CLEAR;
	if ((err = (opt_i2c_write((CMD_REG|CNTRL),&value))) < 0){
			printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to clr ctrl reg failed in ioctl prox_on\n");
		}
	value = PRX_INT_TIME_PARAM;
	if ((err = (opt_i2c_write((CMD_REG|ALS_TIME), &value))) < 0) {
			printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to als time reg failed in ioctl prox_on\n");
		}
	value = PRX_ADC_TIME_PARAM;
	if ((err = (opt_i2c_write((CMD_REG|PRX_TIME), &value))) < 0) {
			printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to prox time reg failed in ioctl prox_on\n");
		}
	value = PRX_WAIT_TIME_PARAM;
	if ((err = (opt_i2c_write((CMD_REG|WAIT_TIME), &value))) < 0){
			printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to wait time reg failed in ioctl prox_on\n");
		}
	value = INTR_FILTER_PARAM;
	if ((err = (opt_i2c_write((CMD_REG|INTERRUPT), &value))) < 0) {
			printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to interrupt reg failed in ioctl prox_on\n");
		}
	value = PRX_CONFIG_PARAM;
	if ((err = (opt_i2c_write((CMD_REG|PRX_CFG), &value))) < 0) {
			printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to prox cfg reg failed in ioctl prox_on\n");
		}
	value = PRX_PULSE_CNT_PARAM;
	if ((err = (opt_i2c_write((CMD_REG|PRX_COUNT), &value))) < 0){
			printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to prox cnt reg failed in ioctl prox_on\n");
		}
	value = PRX_GAIN_PARAM;
	if ((err = (opt_i2c_write((CMD_REG|GAIN), &value))) < 0) {
			printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to prox gain reg failed in ioctl prox_on\n");
		}
		prox_int_thresh[0] = (0x0000) & 0xFF;
		prox_int_thresh[1] = (0x0000 >> 8) & 0xFF;
		prox_int_thresh[2] = (PRX_THRSH_HI_PARAM) & 0xFF;
		prox_int_thresh[3] = (PRX_THRSH_HI_PARAM >> 8) & 0xFF; 
	for (i = 0; i < 4; i++) {
		if ((err = (opt_i2c_write((CMD_REG|(PRX_MINTHRESHLO + i)),&prox_int_thresh[i]))) < 0) {
				printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to prox int thrsh regs failed in ioctl prox_on\n");
			}
	}
	value = CNTL_INTPROXPON_ENBL;
	if ((err = (opt_i2c_write((CMD_REG|CNTRL), &value))) < 0) {
			printk(KERN_ERR "[TAOS] i2c_smbus_write_byte_data to ctrl reg failed in ioctl prox_on\n");
		}
}

/*****************************************************************************************
 *  
 *  function    : taos_off 
 *  description : This function is power-off function for optical sensor.
 *
 *  int type    : Sensor type. Three values is available (PROXIMITY,LIGHT,ALL).
 *                it support power-on function separately.
 *                
 *                 
 */

void taos_off(struct taos_data *taos, int type)
{
	u8 value;
//	int err = 0;
	
	printk(KERN_INFO "[TAOS] taos_off(%d)\n",type);
	
	value = CNTL_REG_CLEAR;
	opt_i2c_write((u8)(CMD_REG|CNTRL) , &value);

#if 0
        prox_ctrl_regulator(0);
        //delete irq from wakeup source
	PROXDBG("[TAOS] unregister irq = %d\n",taos ->irq);
	err = set_irq_wake(taos ->irq, 0); // enable : 1, disable : 0
	PROXDBG("[TAOS] register wakeup source = %d\n",err);
	if (err) 
		printk("[TAOS] register wakeup source failed\n");
#endif
	
	if(type == PROXIMITY || type==ALL)
	{
		disable_irq_nosync(taos_global ->irq);
		PROXDBG("[TAOS] disable irq for proximity \n");        
	}
}


/*************************************************************************/
/*		TAOS file operations  				         */
/*************************************************************************/
static int proximity_open(struct inode *ip, struct file *fp)
{
	return nonseekable_open(ip, fp);
}

static int proximity_release(struct inode *ip, struct file *fp)
{
	return 0;

}

static long proximity_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret=0;
   
    switch(cmd) {
        case TAOS_PROX_OPEN:
        {
            printk(KERN_INFO "[TAOS] TAOS_PROX_OPEN called\n");
            taos_on(taos_global,PROXIMITY);
            proximity_enable =1;			
        }
        break;

        case TAOS_PROX_CLOSE:
        {
            printk(KERN_INFO "[TAOS] TAOS_PROX_CLOSE called\n");
            taos_off(taos_global,PROXIMITY);
            proximity_enable=0;
            prox_value_cnt = 0;                
            proximity_value = 0;                                
        }
        break;

        default:
            printk(KERN_INFO "[TAOS] unknown ioctl %d\n", cmd);
            ret = -ENOTTY;
        break;
    }
    
    return ret;
}



static struct file_operations proximity_fops = {
	.owner  = THIS_MODULE,
	.open   = proximity_open,
	.release = proximity_release,
	.unlocked_ioctl = proximity_ioctl,
};
                 
static struct miscdevice proximity_device = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "proximity",
    .fops   = &proximity_fops,
};


//------------------------------------------------------------------------------------

static int taos_opt_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int err = 0;
	unsigned char value;

	printk(KERN_INFO "[TAOS] %s\n",__func__);

	prox_ctrl_regulator(1);
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		printk(KERN_INFO "[TAOS] i2c_check_functionality error\n");
		err = -ENODEV;
		goto exit;
	}
	if ( !i2c_check_functionality(client->adapter,I2C_FUNC_SMBUS_BYTE_DATA) ) {
		printk(KERN_INFO "[TAOS] byte op is not permited.\n");
		goto exit;
	}
    
	/* OK. For now, we presume we have a valid client. We now create the
	client structure, even though we cannot fill it completely yet. */
	if (!(taos_global = kzalloc(sizeof(struct taos_data), GFP_KERNEL)))
	{
		printk(KERN_ERR "[TAOS] kzalloc:allocating driver_data error\n");
		err = -ENOMEM;
		goto exit;
	}
	memset(taos_global, 0, sizeof(struct taos_data));
	taos_global->client = client;
	i2c_set_clientdata(client, taos_global);
	opt_i2c_client = client;
	printk(KERN_INFO "[TAOS] [%s] slave addr = %x\n", __func__, client->addr);	

	/* misc device Settings */
	err = misc_register(&proximity_device);
	if(err) 
        {
		printk(KERN_ERR "[TAOS] taos driver misc_register failed\n");
		goto FREE_TAOS_DATA;        
	}	

	mdelay(12); // [HSS] Fix Proximity I2C Fail : Add delay 12ms after VDD ON
	
	/* Basic Software Operation for TMD2771X Module */
	value = CNTL_REG_CLEAR;
	opt_i2c_write((u8)(CMD_REG|CNTRL) , &value);
	value = ATIME;
	opt_i2c_write((u8)(CMD_REG|ALS_TIME) , &value);
	value = PTIME;
	opt_i2c_write((u8)(CMD_REG|PRX_TIME) , &value);
	value = WTIME;
	opt_i2c_write((u8)(CMD_REG|WAIT_TIME) , &value);
	value = PPCOUNT;
	opt_i2c_write((u8)(CMD_REG|PRX_COUNT) , &value);
	value = PDRIVE | PDIODE | PGAIN | AGAIN;
	opt_i2c_write((u8)(CMD_REG|GAIN) , &value);
	value = (WEN | PEN | PON); 
	opt_i2c_write((u8)(CMD_REG|CNTRL) , &value);
    
	mdelay(12);
    
	if (i2c_smbus_read_byte(client) < 0)
	{
		printk(KERN_ERR "[TAOS] i2c_smbus_read_byte error!!\n");
	}
	else
	{
		printk(KERN_INFO "[TAOS] Device detected!\n");
	}


	/* gpio config */
	if (gpio_request(GPIO_PROXI_INT, "TAOS_INT")) {
		printk(KERN_ERR "[TAOS] Proximity Request GPIO_%d failed!\n", GPIO_PROXI_INT);
	}
        gpio_direction_input(GPIO_PROXI_INT);
	bcm_gpio_pull_up(GPIO_PROXI_INT, true);
	bcm_gpio_pull_up_down_enable(GPIO_PROXI_INT, true);
	//gpio_free(GPIO_PROXI_INT);

	/* Input device Settings */
        taos_global->input_dev = input_allocate_device();
        if (taos_global->input_dev == NULL) 
        {
            printk(KERN_ERR "[TAOS] Failed to allocate input device\n");
            goto MISC_DREG;
        }
        taos_global->input_dev->name = "proximity_sensor";
        set_bit(EV_SYN,taos_global->input_dev->evbit);
        set_bit(EV_ABS,taos_global->input_dev->evbit);
        input_set_abs_params(taos_global->input_dev, ABS_DISTANCE, 0, 1, 0, 0);
        err = input_register_device(taos_global->input_dev);
        if (err) 
        {
            printk(KERN_ERR "[TAOS] Unable to register %s input device\n", taos_global->input_dev->name);
            input_free_device(taos_global->input_dev);
            kfree(taos_global);
            goto MISC_DREG;
        }
	printk(KERN_INFO "[TAOS] Input device settings complete");


#if USE_INTERRUPT
	/* WORK QUEUE Settings */
	taos_wq = create_singlethread_workqueue("taos_wq");
	if (!taos_wq)
	{
		printk(KERN_ERR "[TAOS] Not enough memory for taos_wq\n");        
		err = -ENOMEM;
		goto INPUT_DEV_DREG;
	} 
	INIT_WORK(&taos_global->work_prox, taos_work_func_prox);
	printk(KERN_INFO "[TAOS] Workqueue Settings complete\n");

	/* INT Settings */	
	taos_global->irq = -1;    
	set_irq_type(PROX_IRQ, IRQ_TYPE_EDGE_FALLING);
	if( (err = request_irq(PROX_IRQ, taos_irq_handler,IRQF_DISABLED | IRQF_NO_SUSPEND , "proximity_int", NULL )) )
	{
		printk(KERN_ERR "[TAOS] request_irq failed IRQ_NO:%d", PROX_IRQ);        
		goto DESTROY_WORK_QUEUE;
	}
        else
        {
		printk(KERN_ERR "[TAOS] request_irq failed IRQ_NO:%d", PROX_IRQ);        
        }
	taos_global->irq = PROX_IRQ;    	
#endif

	/* wake lock init */
	wake_lock_init(&prx_wake_lock, WAKE_LOCK_SUSPEND, "prx_wake_lock");

	/* ktime init */
	timeA = ktime_set(0,0);
	timeB = ktime_set(0,0);
      
	
	// maintain power-down mode before using sensor
	taos_off(taos_global, ALL);

	printk(KERN_INFO "[TAOS] %s end\n",__func__);
	return 0;

DESTROY_WORK_QUEUE:
	destroy_workqueue(taos_wq);	
INPUT_DEV_DREG:
	input_unregister_device(taos_global->input_dev);	
MISC_DREG:
	misc_deregister(&proximity_device);    
FREE_TAOS_DATA:
	kfree(taos_global);
exit:
	return err;
}


static int taos_opt_remove(struct i2c_client *client)
{
	printk(KERN_INFO "%s\n",__func__);

#if USE_INTERRUPT
	free_irq(gpio_to_irq(GPIO_PROXI_INT), NULL);
#endif

        destroy_workqueue(taos_wq);
        input_unregister_device(taos_global->input_dev);
        misc_deregister(&proximity_device);    
        kfree(taos_global);

	return 0;
}

#ifdef CONFIG_PM
static int taos_opt_suspend(struct i2c_client *client, pm_message_t mesg)
{
	PROXDBG("[TAOS] [%s] suspend mode!!\n",__func__);

	return 0;
}

static int taos_opt_resume(struct i2c_client *client)
{
	PROXDBG("[TAOS] [%s] resume mode!!\n",__func__);

	return 0;
}
#else
#define taos_opt_suspend NULL
#define taos_opt_resume NULL
#endif


//static unsigned short normal_i2c[] = { I2C_CLIENT_END};
//I2C_CLIENT_INSMOD_1(taos);

static const struct i2c_device_id taos_id[] = {
	{ "taos", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, taos_id);

static struct i2c_driver taos_opt_driver = {	
	.driver = {
		.owner	= THIS_MODULE,	
		.name	= "taos",
	},
//	.class		= I2C_CLASS_HWMON,
	.id_table	= taos_id,
//	.address_data	= &addr_data,
	.probe		= taos_opt_probe,
	.remove		= taos_opt_remove,
	.suspend	= taos_opt_suspend,
	.resume		= taos_opt_resume,
};

static int __init taos_opt_init(void)
{
	printk(KERN_INFO "%s\n",__func__);

	taos_chip_init();

	return i2c_add_driver(&taos_opt_driver);
}

static void __exit taos_opt_exit(void)
{
	i2c_del_driver(&taos_opt_driver);

	printk(KERN_INFO "%s\n",__func__);

}

module_init( taos_opt_init );
module_exit( taos_opt_exit );

MODULE_AUTHOR("SAMSUNG");
MODULE_DESCRIPTION("Optical Sensor driver for taosp002s00f");
MODULE_LICENSE("GPL");
