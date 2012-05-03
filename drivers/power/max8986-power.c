/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/power/max8986-power.c
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
/**
 *
 *   @file   max8986-power.c
 *
 *   @brief  Power Driver for Maxim MAX8986 PMU
 *
 ****************************************************************************/
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#if defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif /*CONFIG_HAS_WAKELOCK*/
#include <linux/mfd/max8986/max8986.h>
#include <linux/mfd/max8986/max8986-private.h>
#include <linux/broadcom/types.h>
#include <linux/broadcom/bcm_kril_Interface.h>
#include <linux/broadcom/bcm_fuse_sysparm.h>

#include <plat/bcm_auxadc.h>
#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
#endif /*CONFIG_HAS_EARLYSUSPEND*/

#if defined(CONFIG_CPU_FREQ_GOV_BCM21553)  
#include <mach/bcm21553_cpufreq_gov.h>
#endif

#ifdef CONFIG_BLX
#include <linux/blx.h>
#endif

DEFINE_MUTEX(spa_charger_mutex);

#define TRUE	1
#define FALSE	0

#define FEAT_EN_TEST_MODE 1

/* # of shifts used to divide the sum to get the average. */
#define ADC_RUNNING_AVG_SHIFT 3
/* # of samples to perform voltage running sum */
#define ADC_RUNNING_AVG_SIZE (1 << ADC_RUNNING_AVG_SHIFT)

/*Macros to control schedule frequency of charging monitor work queue - with
 * and without charger present */
#define BATTERY_LVL_MON_INTERVAL_WHILE_CHARGING		10000 
#define	BATTERY_LVL_MON_INTERVAL			60000 /* 1 min */
//#define BATTERY_LVL_MON_INTERVAL_WHILE_CHARGING		10000 /* 10 sec */
//#define	BATTERY_LVL_MON_INTERVAL			30000 /* 30 sec */
#define BAT_TEMP_EXCEED_LIMIT_COUNT_MAX			3
#define BATTERY_CHARGING_HYSTERESIS			10
#define SUCCESS 0		/* request is successfull */
#define USB_PREENUM_CURR				90
#if defined(CONFIG_BOARD_COOPERVE)
#define USB_PREENUM_CURR_REQ_VAL MAX8986_CHARGING_CURR_550MA
#else
#define USB_PREENUM_CURR_REQ_VAL MAX8986_CHARGING_CURR_450MA
#endif

#if defined(CONFIG_BOARD_COOPERVE)
#define BATT_RECHARGE_VOLT	4120	//4130
#else
#define BATT_RECHARGE_VOLT	4130
#endif

#define BATT_LOW_VOLT		3400

#define BAT_PERCENT_INIT_VALUE 1

#if 0
#define BATT_FULL_VOLT		4200
#define BATT_LEVEL5_VOLT	3970
#define BATT_LEVEL4_VOLT	3870
#define BATT_LEVEL3_VOLT	3790
#define BATT_LEVEL2_VOLT	3740
#define BATT_LEVEL1_VOLT	3700
#define BATT_LEVEL0_VOLT	3600
#define BATT_LEVEL0_1_VOLT	3540
#define BATT_LOW_VOLT		3400
#endif
#define HIGH_SUSPEND_TEMP	650
#define LOW_SUSPEND_TEMP	-50
#define HIGH_RECOVER_TEMP	430
#define LOW_RECOVER_TEMP	0


#define BAT_30SEC_INTERVAL_RECHARGE (10*HZ) /* 10 sec*/
#define BAT_90MIN_INTERVAL_RECHARGE (90*60*HZ) /* 90 min*/
#define MAX8986_LOG_CHARGING_TIME 1
#define AUXADC_BATVOLT_CHANNEL		3
#define AUXADC_BATTEMP_CHANNEL		0
#define AUXADC_BATVF_CHANNEL		1

int prev_scaled_level=0;

static int is_ovp_suspended=false;
static int init_batt_lvl_interval=0;
static int check_func_called_by_cp=false;
static int cnt_func_called=0;
static struct platform_device *power_device;
//static struct max8986_power *g_ptr_data;

extern dwc_otg_cil_get_BC11_Charger_Type(pmu_muic_chgtyp bc11_chargertype);

struct max8986_power {
	struct max8986 *max8986;

	/* power supplies */
	struct power_supply wall;
	struct power_supply usb;
	struct power_supply battery;
	/* current power source */
	enum power_supply_type power_src;

	/*charger type*/
	u8 charger_type;
	u8 dcd_timout;
	u8 batt_percentage;
	u8 batt_health;
	u32 batt_voltage;
	u16 batt_adc_avg;
	int temp_running_sum;
	int tem_read_inx;
	int temp_reading[ADC_RUNNING_AVG_SIZE];
	int batt_temp_adc_avg;
	int batt_temp_celsius;
	int is_charger_inserted;
	int is_jig_inserted;
	int is_timer_expired;
	int is_recharge;
	int level;
	int tmp;
	u8 volt[10];
	int button;
	int ear_adc;
	u32 temp_exceed_limit_count;

	/* battery status */
	int charging_status;
	int suspended;
	int isFullcharged;

  //#if defined(CONFIG_BATT_LVL_FROM_ADC)
	int level_running_sum;
	int level_read_inx;
	int level_reading[ADC_RUNNING_AVG_SIZE];
	int batt_level_adc_avg;
  //#endif
	struct delayed_work batt_lvl_mon_wq;
#if defined(CONFIG_HAS_WAKELOCK)
	struct wake_lock usb_charger_wl;
	struct wake_lock temp_adc_wl;
#endif /*CONFIG_HAS_WAKELOCK*/

#if defined(CONFIG_HAS_WAKELOCK) 
	struct wake_lock jig_on_wl;
#endif /*CONFIG_HAS_WAKELOCK*/

#if defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_suspend_desc;
#endif /*CONFIG_HAS_EARLYSUSPEND*/
#ifdef MAX8986_LOG_CHARGING_TIME
	ktime_t charging_start_time;
	ktime_t charging_end_time;
	ktime_t charging_time;
	unsigned long current_time;	
	unsigned long next_expire_time;
#endif
	struct mutex power_mtx;
};

#if defined(CONFIG_CPU_FREQ_GOV_BCM21553)  
static struct cpufreq_client_desc *jig_client;
#endif

/* these typedefs should ideally be exported by KRIL header files */
typedef enum {
	/* Charger plug in event for both USB and Wall */
	EM_BATTMGR_CHARGER_PLUG_IN_EVENT,
	/* Charger plug out event */
	EM_BATTMGR_CHARGER_PLUG_OUT_EVENT,
	/* End of Charge event. Battery is full - charging is done. */
	EM_BATTMGR_ENDOFCHARGE_EVENT,
	/* BATT temp is outside window (safety) or extreme temperature */
	EM_BATTMGR_BATT_EXTREME_TEMP_EVENT,
	/* BATT low is detected */
	EM_BATTMGR_LOW_BATT_EVENT,
	/* BATT empty is detected */
	EM_BATTMGR_EMPTY_BATT_EVENT,
	/* BATT level change is detected */
	EM_BATTMGR_BATTLEVEL_CHANGE_EVENT
} HAL_EM_BATTMGR_Event_en_t;

typedef struct {
	/* The event type */
	HAL_EM_BATTMGR_Event_en_t eventType;
	/* The battery level, 0~N, depend the sysparm */
	u8 inLevel;
	/* Adc value in mV. Ex, 4000 is 4.0V, 3800 is 3.8V */
	u16 inAdc_avg;
	/* Total levels */
	u8 inTotal_levels;

} HAL_EM_BatteryLevel_t;

/*********************************************************************
 *                             DEBUG CODE                            *
 *********************************************************************/

/* Enable/disable debug logs */
enum {
	/* Disable all logging */
	DEBUG_DISABLE = 0U,
	DEBUG_FLOW    = (1U << 0),
};

#define DEFAULT_LOG_LVL    (DEBUG_DISABLE)

struct debug {
	int log_lvl;
};

#define __param_check_debug(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }

#define param_check_debug(name, p) \
	__param_check_debug(name, p, debug)

static int param_set_debug(const char *val, struct kernel_param *kp);
static int param_get_debug(char *buffer, struct kernel_param *kp);

static struct debug debug = {
	.log_lvl = DEFAULT_LOG_LVL,
};
module_param_named(debug, debug, debug, S_IRUGO | S_IWUSR | S_IWGRP);

/* helpers to test the log_lvl bitmap */
#define IS_FLOW_DBG_ENABLED	(debug.log_lvl & DEBUG_FLOW)

/* List of commands supported */
enum {
	CMD_SET_LOG_LVL = 'l',
	CMD_SHOW_BAT_STAT = 'b',
	CMD_CHARGING_CTRL = 'c',
};

static void cmd_show_usage(void)
{
	const char usage[] = "Usage:\n"
	  "echo 'cmd string' > /sys/module/max8986_power/parameters/debug\n"
	  "'cmd string' must be constructed as follows:\n"
	  "Update log level: l 0x01\n"
	  "Show battery voltage: b volt\n"
	  "Show battery temperature: b temp\n"
	  "Start battery charging: c start current (MBCCTRL4[3:0] value\n"
	  "Stop battery charging: c stop\n"
	  "Test start/stop battery charging repeatedly: c test\n";

	pr_info("%s", usage);
}

/*
 * Command handlers
 */
static void cmd_set_log_lvl(const char *p)
{
	sscanf(p, "%x", &debug.log_lvl);
}

#define AUXADC_BATVOLT_CHANNEL		3
#define AUXADC_BATTEMP_CHANNEL		4

static void cmd_show_bat_stat(const char *p)
{
	int val;

	/* Skip white spaces */
	while (*p == ' ' || *p == '\t')
		p++;

	if (strncmp("volt", p, strlen("volt")) == 0) {
		val = auxadc_access(AUXADC_BATVOLT_CHANNEL);
		pr_info("adc value for battery voltage: 0x%x\n", val);
	} else if (strncmp("temp", p, strlen("temp")) == 0) {
		val = auxadc_access(AUXADC_BATTEMP_CHANNEL);
		pr_info("adc value for battery temperature: 0x%x\n", val);
	} else {
		pr_info("invalid command\n");
	}
}

static void cmd_charging_ctrl(const char *p)
{
	int val;

	/* Skip white spaces */
	while (*p == ' ' || *p == '\t')
		p++;

	if (strncmp("start", p, strlen("start")) == 0) {
		/* Skip 'start' */
		p += strlen("start");

		/* Skip white spaces */
		while (*p == ' ' || *p == '\t')
			p++;

		/* Get current to be configured */
		sscanf(p, "%d", &val);
		pr_info("Charging current: %d\n", val);

		pmu_start_charging();
		pmu_set_charging_current(val);

	} else if (strncmp("stop", p, strlen("stop")) == 0) {
		pmu_stop_charging();
	} else if (strncmp("test", p, strlen("test")) == 0) {
		int i;

		pmu_start_charging();
		for (i = 0; i < 150; i++) {
			pmu_set_charging_current(0);
			pmu_set_charging_current(22);
		}
	} else {
		pr_info("invalid command\n");
	}
}

static int param_set_debug(const char *val, struct kernel_param *kp)
{
	const char *p;

	if (!val)
		return -EINVAL;

	/* Command is only one character followed by a space. Arguments,
	 * if any, starts from offset 2 in val.
	 */
	p = &val[2];

	switch (val[0]) {
	case CMD_SET_LOG_LVL:
		cmd_set_log_lvl(p);
		break;
	case CMD_SHOW_BAT_STAT:
		cmd_show_bat_stat(p);
		break;
	case CMD_CHARGING_CTRL:
		cmd_charging_ctrl(p);
		break;
	default:
		cmd_show_usage();
		break;
	}
	return 0;
}

static int param_get_debug(char *buffer, struct kernel_param *kp)
{
	cmd_show_usage();
	return 0;
}

/*****************************************************************************
 * power supply interface
 *****************************************************************************/
static enum power_supply_property battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MIN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TEMP, /* Temp prop is register only if a valid temp
						adc channel is specified */
};

static enum power_supply_property wall_props[] = {
	POWER_SUPPLY_PROP_ONLINE
};

static enum power_supply_property usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE
};
int pmu_is_charger_inserted();
static int max8986_usb_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
	int ret = 0;
	struct max8986_power *max8986_power =
		dev_get_drvdata(psy->dev->parent);

	if (unlikely(!max8986_power)) {
		pr_err("%s: invalid driver data\n", __func__);
		return -EINVAL;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval =
			(max8986_power->power_src == POWER_SUPPLY_TYPE_USB)
			? 1 : 0;
		break;

	default:
		pr_info("usb: property %d is not implemented\n", psp);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int max8986_wall_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
	int ret = 0;
	struct max8986_power *max8986_power =
		dev_get_drvdata(psy->dev->parent);

	if (unlikely(!max8986_power)) {
		pr_err("%s: invalid driver data\n", __func__);
		return -EINVAL;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval =
			(max8986_power->power_src == POWER_SUPPLY_TYPE_MAINS)
			? 1 : 0;
		break;

	default:
		pr_info("wall: property %d is not implemented\n", psp);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int max8986_battery_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	int ret = 0;
	struct max8986_power *max8986_power =
		dev_get_drvdata(psy->dev->parent);
	struct max8986_power_pdata *pdata;
	if (unlikely(!max8986_power || !max8986_power->max8986)) {
		pr_err("%s: invalid driver data\n", __func__);
		return -EINVAL;
	}
	pdata = max8986_power->max8986->pdata->power;
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = max8986_power->charging_status;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = pdata->batt_technology;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = max8986_power->batt_percentage;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		if (pdata->batt_adc_tbl.num_entries)
			val->intval =
				pdata->batt_adc_tbl.bat_vol[pdata->batt_adc_tbl.num_entries-1]
				* 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN:
		if (pdata->batt_adc_tbl.num_entries)
			val->intval = pdata->batt_adc_tbl.bat_vol[0] * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = max8986_power->batt_voltage;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval =
			(max8986_power->power_src ==
			 POWER_SUPPLY_TYPE_BATTERY) ? 1 : 0;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = max8986_power->batt_health;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		if (max8986_power->max8986->pdata->pmu_event_cb)
			val->intval =
				max8986_power->max8986->pdata->pmu_event_cb(
						PMU_EVENT_BATT_TEMP_TO_DEGREE_C,
					max8986_power->batt_temp_adc_avg);

			max8986_power->batt_temp_celsius = val->intval;
		break;
	default:
		pr_info("bat: property %d is not implemented\n", psp);
		ret = -EINVAL;
		break;
	}
	return ret;
}

/*****************************************************************************
 * pmu query functions
 *****************************************************************************/

static int max8986_is_jig_inserted(struct max8986_power *max8986_power)
{
	struct max8986 *max8986 = max8986_power->max8986;
	int ret;
	unsigned char status1, status2;
	unsigned char intr1, intr2, intr3;
	unsigned char adc, chgtyp;

	status1 = status2 = intr1 = intr2 = intr3 = 0;

	ret = max8986->read_dev(max8986, MAX8986_MUIC_REG_STATUS1, &status1);
	if (ret < 0) {
		pr_err("%s: MAX8986_MUIC_REG_STATUS1 read failed\n", __func__);
		return -EIO;
	}	
	ret = max8986->read_dev(max8986, MAX8986_MUIC_REG_STATUS2, &status2);
	if (ret < 0) {
		pr_err("%s: MAX8986_MUIC_REG_STATUS2 read failed\n", __func__);
		return -EIO;
	}
	pr_info("%s: STATUS1:0x%02x, 2:0x%02x\n", __func__, status1, status2);
	/* interrupt clear. */
	ret = max8986->read_dev(max8986, MAX8986_MUIC_REG_INT1, &intr1);
	if (ret < 0) {
		pr_err("%s: MAX8986_MUIC_REG_STATUS2 read failed\n", __func__);
		return -EIO;
	}	
	ret = max8986->read_dev(max8986, MAX8986_MUIC_REG_INT2, &intr2);
	if (ret < 0) {
		pr_err("%s: MAX8986_MUIC_REG_STATUS2 read failed\n", __func__);
		return -EIO;
	}
	ret = max8986->read_dev(max8986, MAX8986_MUIC_REG_INT3, &intr3);
	if (ret < 0) {
		pr_err("%s: MAX8986_MUIC_REG_STATUS2 read failed\n", __func__);
		return -EIO;
	}	

	adc = status1 & 0x1f; //STATUS1_ADC_MASK;
	chgtyp = status2 & 0x7; //STATUS2_CHGTYP_MASK;

	switch((chgtyp << 8) | adc)
	{
	case 0x18://ADC_JIG_USB_OFF:
	case 0x19://ADC_JIG_USB_ON:
	case 0x1c://ADC_JIG_UART_OFF:
	case 0x1d://ADC_JIG_UART_ON:
		ret = true;
		break;
	default:
		ret = false;
		break;	
	}

	return ret;

}

static int max8986_get_fc_current(struct max8986_power *max8986_power)
{
	u8 reg_val;
	struct max8986 *max8986 = max8986_power->max8986;
	/* Get fc_current as charging current */
	max8986->read_dev(max8986, MAX8986_PM_REG_MBCCTRL4, &reg_val);
	/*MBCICHFC [4]
	1: 200mA to 950mA settings
	0: 90mA*/
	if((reg_val & MAX8986_MBCCTRL4_MBCICHFC4) == 0)
		return MAX8986_CHARGING_CURR_90MA;

	reg_val = (reg_val & MAX8986_MBCCTRL4_MBCICHFC_MASK); /*bits - 0 -4 : no need to shift */
	return reg_val;
}

static void max8986_set_fc_current(struct max8986_power *max8986_power,
					u8 fc_current)
{
	u8 regVal;
	struct max8986 *max8986 = max8986_power->max8986;

	pr_debug("%s: cc = %x\n", __func__, fc_current);

	if(fc_current >= MAX8986_CHARGING_CURR_MAX)
	{
		pr_info("%s: fc charging current param INVALID\n", __func__);
		return;
	}
	mutex_lock(&max8986_power->power_mtx);
	/* Set fc_current as charging current */
	regVal = fc_current & MAX8986_MBCCTRL4_MBCICHFC_MASK;

	max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL4, regVal);
	mutex_unlock(&max8986_power->power_mtx);
}

static u8 max8986_get_charging_current(struct max8986_power *max8986_power,u8 charger_type, u8* supply_type)
{
	struct max8986 *max8986 = max8986_power->max8986;
	struct max8986_power_pdata *pdata = max8986->pdata->power;
	u8 cc;

	switch(charger_type)
	{
	case PMU_MUIC_CHGTYP_USB:
	case PMU_MUIC_CHGTYP_DOWNSTREAM_PORT:
		*supply_type = POWER_SUPPLY_TYPE_USB;
		cc =  USB_PREENUM_CURR_REQ_VAL;
		break;

	case PMU_MUIC_CHGTYP_DEDICATED_CHGR:
		*supply_type = POWER_SUPPLY_TYPE_MAINS;
		cc =  pdata->wac_charging_cc;
		break;

	case PMU_MUIC_CHGTYP_SPL_500MA:
		*supply_type = POWER_SUPPLY_TYPE_MAINS;
		cc =  MAX8986_CHARGING_CURR_500MA;
		break;

	case PMU_MUIC_CHGTYP_SPL_1A:
		*supply_type = POWER_SUPPLY_TYPE_MAINS;
		cc =  MAX8986_CHARGING_CURR_950MA;
		break;

	case PMU_MUIC_CHGTYP_DEAD_BATT_CHG:
		*supply_type = POWER_SUPPLY_TYPE_MAINS; //??
		cc = MAX8986_CHARGING_CURR_90MA;
		break;

	default:
		*supply_type = POWER_SUPPLY_TYPE_BATTERY;
		cc = MAX8986_CHARGING_CURR_90MA;
		break;
	}
	printk(KERN_INFO "%s:cc = %d,supply_type = %d\n",__func__,cc,*supply_type);
	return cc;
}



/****************************************************************************
*
* max8986_get_batt_temperature
*
* returns: temperature measurement in ADC units or -1 on error
*
***************************************************************************/
static int max8986_get_batt_temperature(struct max8986_power *max8986_power)
{
	int temp;
	int i;
	struct max8986_power_pdata *pdata =
      		  max8986_power->max8986->pdata->power;

	//max8986_power = g_ptr_data;

	if (pdata->temp_adc_channel < 0)
		return -EINVAL;

	/* get 10 bit ADC output */
	temp = auxadc_access(pdata->temp_adc_channel);
	//temp >>= 2; /* making it 8 bit */
	if (temp <= 0) {
		pr_err("%s:Error reading ADC\n", __func__);
		return -1;
	}
	/* If it is the very first measurement taken,
	 * initialize the buffer elements to the same value
	 * */
	if (max8986_power->batt_temp_adc_avg == 0) {
		max8986_power->temp_running_sum = 0;
		for (i = 0; i < ADC_RUNNING_AVG_SIZE; i++) {
			temp = auxadc_access(pdata->temp_adc_channel);
			//temp >>= 2; /* making it 8 bit */
			max8986_power->temp_reading[i] = temp;
			max8986_power->temp_running_sum += temp;
		}
		max8986_power->tem_read_inx = 0;
	}
	/* Keep the sum running forwards */
	max8986_power->temp_running_sum -=
		max8986_power->temp_reading[max8986_power->tem_read_inx];
	max8986_power->temp_reading[max8986_power->tem_read_inx] = temp;
	max8986_power->temp_running_sum += temp;
	max8986_power->tem_read_inx =
		(max8986_power->tem_read_inx + 1) % ADC_RUNNING_AVG_SIZE;

	/* Divide the running sum by number
	 * of measurements taken to get the average
	 * */
	max8986_power->batt_temp_adc_avg =
		max8986_power->temp_running_sum >> ADC_RUNNING_AVG_SHIFT;
	return max8986_power->batt_temp_adc_avg;
}
static void max8986_ctrl_charging(struct max8986_power *max8986_power, int ctrl)
{
	u8 regVal;
    //	struct max8986 *max8986 = g_ptr_data->max8986;
	struct max8986 *max8986 = max8986_power->max8986;

	pr_info("%s:%s\n", __func__,(ctrl?"START CHARGING":"STOP CHARGING"));

	if( ctrl==TRUE)
	{
		regVal = MAX8986_MBCCTRL2_VCHGR_FC | MAX8986_MBCCTRL2_MBCHOSTEN; /*other bits are unused*/
		max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL2, regVal);
		pr_info("%s:%s\n", __func__,"Success");
	}
	else
	{
		/* Disable charging and fast charging */
		regVal = 0; //~(MAX8986_MBCCTRL2_VCHGR_FC | MAX8986_MBCCTRL2_MBCHOSTEN) - other bits are unused
		max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL2, regVal);
		pr_info("%s:%s\n", __func__,"Success");		
	}
}
static void max8986_start_charging(struct max8986_power *max8986_power,
		int charger_type)
{
	u8 regVal;
	u8 supply_type;
	struct max8986 *max8986 = max8986_power->max8986;
	u8 charging_cc = max8986_get_charging_current(max8986_power,charger_type,&supply_type);

	pr_debug("%s\n", __func__);

	if(supply_type == POWER_SUPPLY_TYPE_BATTERY)
	{
		pr_info("%s: NO charger connected !!!\n", __func__);
		return;
	}

	max8986_set_fc_current(max8986_power, charging_cc);
	dwc_otg_cil_get_BC11_Charger_Type(charger_type);
	mutex_lock(&max8986_power->power_mtx);

	/* Enable the interrupts */
	max8986_enable_irq(max8986_power->max8986, MAX8986_IRQID_INT2_CHGEOC);
	max8986_enable_irq(max8986_power->max8986,MAX8986_IRQID_INT2_MBCCHGERR);
	max8986_enable_irq(max8986_power->max8986, MAX8986_IRQID_INT2_CHGERR);
	/* Enable Fast charge mode and enable charging +#define
	 * MAX8986_MBCCTRL2_DEFAULT	0x14 */
	regVal = MAX8986_MBCCTRL2_VCHGR_FC | MAX8986_MBCCTRL2_MBCHOSTEN; /*other bits are unused*/
	max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL2, regVal);

	max8986_power->suspended = false;

	if(max8986_power->isFullcharged != TRUE)
		max8986_power->charging_status = POWER_SUPPLY_STATUS_CHARGING;

	if (max8986_power->power_src != supply_type)
	{
		max8986_power->power_src = supply_type;
		power_supply_changed((supply_type == POWER_SUPPLY_TYPE_USB)
				? &max8986_power->usb : &max8986_power->wall);
	}
	power_supply_changed(&max8986_power->battery);
#ifdef MAX8986_LOG_CHARGING_TIME
	max8986_power->charging_start_time = ktime_get();
#endif
	mutex_unlock(&max8986_power->power_mtx);
}

static void max8986_stop_charging(struct max8986_power *max8986_power,
		bool updatePwrSrc)
{
	u8 regVal;
	enum power_supply_type old_pwr_src;
	struct max8986 *max8986 = max8986_power->max8986;

	pr_debug("%s\n", __func__);
	mutex_lock(&max8986_power->power_mtx);
	/* Disable charging and fast charging */
	regVal = 0; //~(MAX8986_MBCCTRL2_VCHGR_FC | MAX8986_MBCCTRL2_MBCHOSTEN) - other bits are unused
	max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL2, regVal);

	/* Disable CHGERR and MBCCHGERR interrupts */
	max8986_disable_irq(max8986, MAX8986_IRQID_INT2_CHGEOC);
	max8986_disable_irq(max8986, MAX8986_IRQID_INT2_MBCCHGERR);
	max8986_disable_irq(max8986, MAX8986_IRQID_INT2_CHGERR);

	if(max8986_power->isFullcharged != TRUE)
		max8986_power->charging_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	
	if (updatePwrSrc) {
		pr_info("%s:updatePwrSrc\n", __func__);
		old_pwr_src = max8986_power->power_src;
		max8986_power->power_src = POWER_SUPPLY_TYPE_BATTERY;
		max8986_power->charging_status =
			POWER_SUPPLY_STATUS_DISCHARGING;
		if (old_pwr_src == POWER_SUPPLY_TYPE_USB)
			power_supply_changed(&max8986_power->usb);
		else if (old_pwr_src == POWER_SUPPLY_TYPE_MAINS) {
			power_supply_changed(&max8986_power->wall);
		}
	}
	power_supply_changed(&max8986_power->battery);
	mutex_unlock(&max8986_power->power_mtx);
}

static void max8986_check_batt_vf(struct max8986_power *max8986_power)
{
	int batt_vf;

	batt_vf = auxadc_access(AUXADC_BATVF_CHANNEL);
	batt_vf >>= 2;
	//power_supply_changed(&max8986_power->battery);

	if(max8986_power->is_charger_inserted){
		if( batt_vf==0xFF )
		{
			pr_info("%s: vf=%d: battery removed\n", __func__,batt_vf);
			max8986_power->batt_percentage=0;
			max8986_power->isFullcharged = FALSE;			
			max8986_power->batt_health = POWER_SUPPLY_HEALTH_DEAD;
			max8986_stop_charging(max8986_power,TRUE);			
			power_supply_changed(&max8986_power->battery);
		}
	}
		
}

#if 1
#if 0
int calculate_batt_level(int batt_volt)
{
	struct max8986_power *max8986_power;
	int scaled_level = 0;

	if(!power_device)
		return;
	max8986_power = platform_get_drvdata(power_device);	
	max8986_power->level = 0;

	if(batt_volt >= BATT_FULL_VOLT) {		//100%
		scaled_level = 100;
		max8986_power->level = 6;
	}
	else if(batt_volt >=  BATT_LEVEL5_VOLT) {	//99% ~ 80%
		scaled_level = ((batt_volt -BATT_LEVEL5_VOLT+1)*20)/(BATT_FULL_VOLT-BATT_LEVEL5_VOLT);
 		scaled_level = scaled_level+80;
		max8986_power->level = 5;
	}
	else if(batt_volt >= BATT_LEVEL4_VOLT) { 	//79% ~ 65%
		scaled_level = ((batt_volt -BATT_LEVEL4_VOLT)*15)/(BATT_LEVEL5_VOLT-BATT_LEVEL4_VOLT);
 		scaled_level = scaled_level+65;
		max8986_power->level = 4;
	}
	else if(batt_volt >= BATT_LEVEL3_VOLT) { 	//64% ~ 50%
		scaled_level = ((batt_volt -BATT_LEVEL3_VOLT)*15)/(BATT_LEVEL4_VOLT-BATT_LEVEL3_VOLT);
 		scaled_level = scaled_level+50;
		max8986_power->level = 3;
	}
	else if(batt_volt >= BATT_LEVEL2_VOLT) {	//49% ~ 35%
		scaled_level = ((batt_volt -BATT_LEVEL2_VOLT)*15)/(BATT_LEVEL3_VOLT-BATT_LEVEL2_VOLT);
		scaled_level = scaled_level+35;
		max8986_power->level = 2;
	}
	else if(batt_volt >= BATT_LEVEL1_VOLT) {	//34% ~ 20%
		scaled_level = ((batt_volt -BATT_LEVEL1_VOLT)*15)/(BATT_LEVEL2_VOLT-BATT_LEVEL1_VOLT);
 		scaled_level = scaled_level+20;
		max8986_power->level = 1;
	}
	else if(batt_volt >= BATT_LEVEL0_VOLT) {	//19% ~ 5%
		scaled_level = ((batt_volt -BATT_LEVEL0_VOLT)*15)/(BATT_LEVEL1_VOLT-BATT_LEVEL0_VOLT);
 		scaled_level = scaled_level+5;
	}
	else if(batt_volt >= BATT_LEVEL0_1_VOLT) {	// 4% ~ 3%
		scaled_level = ((batt_volt -BATT_LEVEL0_1_VOLT)*2)/(BATT_LEVEL0_VOLT-BATT_LEVEL0_1_VOLT);
 		scaled_level = scaled_level+3;
	}
	else if(batt_volt > BATT_LOW_VOLT) {		// 2% ~ 1%
		scaled_level = ((batt_volt -BATT_LOW_VOLT)*2)/(BATT_LEVEL0_VOLT-BATT_LOW_VOLT);
 		scaled_level = scaled_level+1;
	}
	else {
		scaled_level = 1;
	}
	return scaled_level;
}
#endif
static s8 max8986_get_batt_voltage(u16 batt_adc,struct batt_adc_tbl *batt_adc_tbl, u32 *voltage)
{
	int inx;
	u32 per = 100;
	int ratio = 0;
	*voltage = batt_adc_tbl->bat_vol[batt_adc_tbl->num_entries - 1];

	if (batt_adc < batt_adc_tbl->bat_adc[0]) {
		*voltage = batt_adc_tbl->bat_vol[0];
		pr_info("%s: Batt voltage too low\n", __func__);
		return -1; /*batt too low*/
	} else if (batt_adc >
			batt_adc_tbl->bat_adc[batt_adc_tbl->num_entries - 1]) {
		pr_info("%s: Batt voltage too high\n", __func__);
				return 1; /*beyond max limit*/
	}
	for (inx = 0; inx < (batt_adc_tbl->num_entries - 1); inx++) {
		if (batt_adc >= batt_adc_tbl->bat_adc[inx] &&
				batt_adc < batt_adc_tbl->bat_adc[inx+1]) {
			per = inx*100 +
				((batt_adc - batt_adc_tbl->bat_adc[inx])*100)/
				(batt_adc_tbl->bat_adc[inx+1]-batt_adc_tbl->bat_adc[inx]);
			per /= batt_adc_tbl->num_entries-1;
			/* using liner interpolation (lerp) */
			ratio = ((batt_adc - batt_adc_tbl->bat_adc[inx]) * 1000)/(batt_adc_tbl->bat_adc[inx+1]-batt_adc_tbl->bat_adc[inx]);
			*voltage = batt_adc_tbl->bat_vol[inx] + ((batt_adc_tbl->bat_vol[inx + 1] - batt_adc_tbl->bat_vol[inx]) * ratio)/1000;
			break;
		}
	}

	return 0;
}
#else
static s8 max8986_get_batt_capacity(u16 batt_adc,
		struct batt_adc_tbl *batt_adc_tbl, u8 prev_per, u8 *new_per, u32 *voltage)
{
	int inx;
	u32 per = 100;
	int ratio = 0;
	*voltage = batt_adc_tbl->bat_vol[batt_adc_tbl->num_entries - 1];

	if (batt_adc < batt_adc_tbl->bat_adc[0]) {
		*new_per = 0;
		*voltage = batt_adc_tbl->bat_vol[0];
		pr_info("%s: Batt voltage too low\n", __func__);
		return -1; /*batt too low*/
	} else if (batt_adc >
			batt_adc_tbl->bat_adc[batt_adc_tbl->num_entries - 1]) {
		pr_info("%s: Batt voltage too high\n", __func__);
		*new_per = 100;
				return 1; /*beyond max limit*/
	}
	for (inx = 0; inx < (batt_adc_tbl->num_entries - 1); inx++) {
		if (batt_adc >= batt_adc_tbl->bat_adc[inx] &&
				batt_adc < batt_adc_tbl->bat_adc[inx+1]) {
			per = inx*100 +
				((batt_adc - batt_adc_tbl->bat_adc[inx])*100)/
				(batt_adc_tbl->bat_adc[inx+1]-batt_adc_tbl->bat_adc[inx]);
			per /= batt_adc_tbl->num_entries-1;
			/* using liner interpolation (lerp) */
			ratio = ((batt_adc - batt_adc_tbl->bat_adc[inx]) * 1000)/(batt_adc_tbl->bat_adc[inx+1]-batt_adc_tbl->bat_adc[inx]);
			*voltage = batt_adc_tbl->bat_vol[inx] + ((batt_adc_tbl->bat_vol[inx + 1] - batt_adc_tbl->bat_vol[inx]) * ratio)/1000;
			break;
		}
	}
	/*round to nearest multiple of 5. Needed to adjust
	 * minor changes in battery % due to load
	 * */
	if (prev_per != 0xFF) {
		if (per < prev_per)
			per = ((per+5)/5)*5;
		else
			per = (per/5)*5;
	}
	*new_per = (u8)per;
	pr_info("%s: Prev Battery Percentage = %d, New Percentage = %d\n",
			__func__, prev_per, *new_per);
	return 0;
}
#endif
/****************************************************************************
*
* bcm59035_get_batt_level
*
* returns: voltage measurement in ADC units or -1 on error
*
***************************************************************************/
//#if defined(CONFIG_BATT_LVL_FROM_ADC)
static int max8986_get_batt_level(struct max8986_power *max8986_power)
{
	int level;
	int i;
	/* Hard coding ADC channel number for battery level. This is only
	 * temporary. Once CP callbacks are supported this will be removed.
	 * */
	u8 level_adc_channel = AUXADC_BATVOLT_CHANNEL;

	/* get 10 bit ADC output */
	level = auxadc_access(level_adc_channel);
	//level = level >> 2; /* making it a 8-bit value */
	if (level <= 0) {
		pr_err("%s:Error reading ADC\n", __func__);
		return -1;
	}
	/* If it is the very first measurement taken, initialize the buffer
	*  elements to the same value
	*  */
	if (max8986_power->batt_level_adc_avg == 0) {
		max8986_power->level_running_sum = 0;
		for (i = 0; i < ADC_RUNNING_AVG_SIZE; i++) {
			level = auxadc_access(level_adc_channel);
			//level = level >> 2;
			max8986_power->level_reading[i] = level;
			max8986_power->level_running_sum += level;
		}
		max8986_power->level_read_inx = 0;
	}
	/* Keep the sum running forwards */
	max8986_power->level_running_sum -=
	max8986_power->level_reading[max8986_power->level_read_inx];
	max8986_power->level_reading[max8986_power->level_read_inx] = level;
	max8986_power->level_running_sum += level;
	max8986_power->level_read_inx =
		(max8986_power->level_read_inx + 1) % ADC_RUNNING_AVG_SIZE;

	/* Divide the running sum by number of measurements taken to get the
	* average */
	max8986_power->batt_level_adc_avg =
		max8986_power->level_running_sum >> ADC_RUNNING_AVG_SHIFT;

	pr_info("%s: batt_lvl read from ADC: %d\n", __func__,
			max8986_power->batt_level_adc_avg);

	return max8986_power->batt_level_adc_avg;
}

static void max8986_get_batt_level_adc(struct max8986_power *max8986_power)
{
	int batt_level;
	s8 bat_state;
	u8 bat_per = 0,old_bat_per=0;
	struct max8986_power_pdata *pdata =
		max8986_power->max8986->pdata->power;
#if defined(CONFIG_HAS_WAKELOCK)
	wake_lock(&max8986_power->temp_adc_wl);
#endif
	batt_level = max8986_get_batt_level(max8986_power);
#if defined(CONFIG_HAS_WAKELOCK)
	wake_unlock(&max8986_power->temp_adc_wl);
#endif

	max8986_power->batt_adc_avg = batt_level-BATTERY_CHARGING_HYSTERESIS;

	pr_info("%s:ADC average = %d, batt_level = %d\n",__func__, max8986_power->batt_adc_avg, batt_level);
	pr_info("%s:battery percentage = %d\n",__func__, max8986_power->batt_percentage);
#if 1//sehyoung	
	bat_state = max8986_get_batt_voltage(max8986_power->batt_adc_avg,&pdata->batt_adc_tbl, &max8986_power->batt_voltage);
	old_bat_per = max8986_power->batt_percentage;
	//bat_per = calculate_batt_level(max8986_power->batt_voltage);
	if (max8986_power->max8986->pdata->pmu_event_cb)
		bat_per=max8986_power->max8986->pdata->pmu_event_cb(PMU_EVENT_BATT_ADC_TO_VOLTAGE,max8986_power->batt_voltage);	
#else
	bat_state = max8986_get_batt_capacity(max8986_power->batt_adc_avg,
			&pdata->batt_adc_tbl, max8986_power->batt_percentage,
			&bat_per, &max8986_power->batt_voltage);
#endif
	if (bat_state > 0) {
		max8986_power->batt_health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
	} else if (max8986_power->batt_health ==POWER_SUPPLY_HEALTH_OVERVOLTAGE && is_ovp_suspended !=true) {
		max8986_power->batt_health = POWER_SUPPLY_HEALTH_GOOD;
	}
	printk(KERN_INFO "%s:max8986_power->charging_status = %d\n",__func__,max8986_power->charging_status);
	if (max8986_power->charging_status == POWER_SUPPLY_STATUS_FULL){
		bat_per = 100;
		prev_scaled_level=100;
	}

	if (bat_per != old_bat_per) {
		if( bat_per > old_bat_per)
		{
			//if( max8986_power->is_charger_inserted == TRUE )
			//{
				max8986_power->batt_percentage  = bat_per;
				max8986_power->level = max8986_power->batt_percentage/20;
			//}	
		}
		else
		{
			if( max8986_power->is_charger_inserted != TRUE  || max8986_power->batt_percentage==BAT_PERCENT_INIT_VALUE )
			{
				max8986_power->batt_percentage  = bat_per;
				max8986_power->level = max8986_power->batt_percentage/20;
			}
		}		
#if 0
		/*"N mV" = (("8-Bit_ADC_code" - 30) * 4) + 3400*/
		max8986_power->batt_voltage =
			(max8986_power->batt_adc_avg - 30)*4 + 3400 - 12;
#endif
		if (bat_state <= 0 &&
				max8986_power->batt_voltage > pdata->batt_adc_tbl.bat_vol[pdata->batt_adc_tbl.num_entries-1])
			max8986_power->batt_voltage =
				pdata->batt_adc_tbl.bat_vol[pdata->batt_adc_tbl.num_entries-1];
#if 0//sehyoung
		max8986_power->batt_voltage = max8986_power->batt_voltage * 1000;
#endif
		/*max8986_power->batt_voltage *= 1000;*/
		pr_info("%s: Battery percentage : %d, volt = %d\n", __func__,
				max8986_power->batt_percentage,
				max8986_power->batt_voltage);
		power_supply_changed(&max8986_power->battery);
	}
}
//#endif /* CONFIG_BATT_LVL_FROM_ADC */
/*****************************************************************************
 * cp callbacks
 *****************************************************************************/
#if defined(CONFIG_BRCM_FUSE_RIL_CIB) && !defined(CONFIG_BATT_LVL_FROM_ADC)

static void inner_function(struct max8986_power *max8986_power)
{
  //max8986_power = g_ptr_data;

	// 5hr_Timer
	if(max8986_power->is_timer_expired)
	{
		max8986_power->current_time = jiffies;
		pr_info("%s:%ld\n", __func__,max8986_power->current_time);

		if( time_after(max8986_power->current_time,max8986_power->next_expire_time))
		{
			pr_info("%s:recharge=%s\n", __func__,((max8986_power->is_recharge)?"recharge":"not yet"));
			if( max8986_power->is_recharge ) // Charging enable
			{
				max8986_ctrl_charging(max8986_power,TRUE);
				max8986_power->is_recharge = FALSE;
				max8986_power->next_expire_time =max8986_power->current_time+BAT_90MIN_INTERVAL_RECHARGE;
				pr_info("%s:90_min_next=%ld\n", __func__,max8986_power->next_expire_time);
			}
			else // Charging disable
			{
				max8986_ctrl_charging(max8986_power,FALSE);
				max8986_power->is_recharge = TRUE;
				max8986_power->next_expire_time = max8986_power->current_time+BAT_30SEC_INTERVAL_RECHARGE;
				pr_info("%s:30_sec_next=%ld\n", __func__,max8986_power->next_expire_time);
			}
		}		
	}
	
	// Recharge
	if( (max8986_power->isFullcharged==TRUE)&&(max8986_power->batt_voltage <= BATT_RECHARGE_VOLT))
	{
		pr_info("%s:recharge:current_volt=%d\n", __func__,max8986_power->batt_voltage);	
		max8986_start_charging(max8986_power,max8986_power->charger_type);
	}

	// Low Battery
	if(max8986_power->batt_voltage <=BATT_LOW_VOLT)
	{
		if (max8986_power->is_charger_inserted != TRUE)
		{
			max8986_power->batt_percentage=0;
			max8986_power->isFullcharged = FALSE;
			max8986_power->batt_health = POWER_SUPPLY_HEALTH_DEAD;
			max8986_stop_charging(max8986_power,TRUE);
			power_supply_changed(&max8986_power->battery);

		}
		else
		{
			max8986_power->batt_percentage=1;
			max8986_power->batt_health = POWER_SUPPLY_HEALTH_GOOD;
			max8986_start_charging(max8986_power,max8986_power->charger_type);
		}
	}
}
static void max8986_ril_adc_notify_cb(unsigned long msg_type, int result,
		void *dataBuf, unsigned long dataLength)
{
	HAL_EM_BatteryLevel_t *batt_lvl = (HAL_EM_BatteryLevel_t *) dataBuf;
	struct max8986_power *max8986_power;
	struct max8986_power_pdata *pdata;
	u8 bat_per = 0,old_bat_per=0;
	s8 bat_state;
	u32 voltage;
	int celsius,temp;
#if 1
	if (batt_lvl == NULL || power_device == NULL) {
		pr_err("%s:Invalid params ...\n", __func__);
		return;
	}
	max8986_power = platform_get_drvdata(power_device);
	if (max8986_power == NULL) {
		pr_err("%s:Device not init\n", __func__);
		return;
	}
#endif
	//max8986_power = g_ptr_data;

	pdata = max8986_power->max8986->pdata->power;

	if(batt_lvl->inAdc_avg>=0xfff0)
		//batt_lvl->inAdc_avg=pdata->batt_adc_tbl.bat_adc[pdata->batt_adc_tbl.num_entries-1]-1;
		batt_lvl->inAdc_avg=pdata->batt_adc_tbl.bat_adc[0]+1;
	
	switch (batt_lvl->eventType) {
	case EM_BATTMGR_BATTLEVEL_CHANGE_EVENT:

			max8986_power->batt_adc_avg = batt_lvl->inAdc_avg;
		

		bat_state = max8986_get_batt_voltage(max8986_power->batt_adc_avg,&pdata->batt_adc_tbl, &max8986_power->batt_voltage);
		old_bat_per = max8986_power->batt_percentage;
		//bat_per = calculate_batt_level(max8986_power->batt_voltage);
		if (max8986_power->max8986->pdata->pmu_event_cb)
			bat_per=max8986_power->max8986->pdata->pmu_event_cb(PMU_EVENT_BATT_ADC_TO_VOLTAGE,max8986_power->batt_voltage);	
		check_func_called_by_cp=false;
		if (bat_state > 0) {
			max8986_power->batt_health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		} else if (max8986_power->batt_health ==POWER_SUPPLY_HEALTH_OVERVOLTAGE && is_ovp_suspended !=true) {
			max8986_power->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		}
		if (max8986_power->charging_status == POWER_SUPPLY_STATUS_FULL){
			bat_per = 100;
			prev_scaled_level=100;
		}

		if (bat_per != old_bat_per) {
			if( bat_per > old_bat_per)
			{
				//if( max8986_power->is_charger_inserted == TRUE )
				//{
					max8986_power->batt_percentage  = bat_per;
					max8986_power->level = max8986_power->batt_percentage/20;
				//}	
			}
			else
			{
				if( max8986_power->is_charger_inserted != TRUE  || max8986_power->batt_percentage==BAT_PERCENT_INIT_VALUE )
				{
					max8986_power->batt_percentage  = bat_per;
					max8986_power->level = max8986_power->batt_percentage/20;
				}
			}

			if (bat_state <= 0 &&
					max8986_power->batt_voltage > pdata->batt_adc_tbl.bat_vol[pdata->batt_adc_tbl.num_entries-1])
				max8986_power->batt_voltage =
					pdata->batt_adc_tbl.bat_vol[pdata->batt_adc_tbl.num_entries-1];
			power_supply_changed(&max8986_power->battery);
		}
		inner_function(max8986_power);				
		break;
	case EM_BATTMGR_EMPTY_BATT_EVENT:
		pr_info("%s: low batt  event\n", __func__);
		if(!pmu_is_charger_inserted())
			max8986_power->batt_percentage = 0;
		else
			max8986_power->batt_percentage = 1;

		max8986_power->batt_voltage =BATT_LOW_VOLT;

		pr_info("Battery percentage : %d, volt = %d\n",
				max8986_power->batt_percentage,
				max8986_power->batt_voltage);
		power_supply_changed(&max8986_power->battery);
		break;
	
	default:
		break;
	}
}
#endif
/*****************************************************************************
 * charger monitoring
 *****************************************************************************/
static void max8986_batt_lvl_mon_wq(struct work_struct *work)
{
	int temp,charger_type,celsius;
	struct max8986_power *max8986_power =
		container_of(work, struct max8986_power, batt_lvl_mon_wq.work);
	struct max8986_power_pdata *pdata =
		max8986_power->max8986->pdata->power;
	/*init to charging case */
	int mon_interval = BATTERY_LVL_MON_INTERVAL_WHILE_CHARGING;

	if (max8986_power->charging_status == POWER_SUPPLY_STATUS_DISCHARGING
			|| max8986_power->charging_status ==
			POWER_SUPPLY_STATUS_FULL) {
		mon_interval = BATTERY_LVL_MON_INTERVAL;
	}

	if( init_batt_lvl_interval<=3){
		++init_batt_lvl_interval;
		mon_interval = BATTERY_LVL_MON_INTERVAL_WHILE_CHARGING;
	}	
	/*Do temperature monitoring if a valid temp adc channel is specified */
	if (pdata->temp_adc_channel >= 0) {
#if defined(CONFIG_HAS_WAKELOCK)
		wake_lock(&max8986_power->temp_adc_wl);
#endif
		temp = max8986_get_batt_temperature(max8986_power);

#if defined(CONFIG_HAS_WAKELOCK)
		wake_unlock(&max8986_power->temp_adc_wl);
#endif
		if (temp < 0) {
			pr_err("%s: Error reading temp\n", __func__);
		} 
                else 
                {	
			if (max8986_power->max8986->pdata->pmu_event_cb)
					max8986_power->batt_temp_celsius=max8986_power->max8986->pdata->pmu_event_cb(PMU_EVENT_BATT_TEMP_TO_DEGREE_C,max8986_power->batt_temp_adc_avg);

			celsius = max8986_power->batt_temp_celsius;
                	max8986_power->tmp = celsius/10;
			
			if ((celsius >= HIGH_SUSPEND_TEMP) ||(celsius <= LOW_SUSPEND_TEMP)) 
			{
				pr_info("%s: Batt temp(%d) beyond limit\n",__func__, celsius);

				if (max8986_power->suspended == false) 
				{

					if (max8986_power->is_charger_inserted) 
					{
						max8986_power->batt_health =(celsius >= HIGH_SUSPEND_TEMP)
						? POWER_SUPPLY_HEALTH_OVERHEAT : POWER_SUPPLY_HEALTH_COLD;

						pr_info("%s: Temp out of range - Charging stopped\n",__func__);

						max8986_stop_charging(max8986_power,FALSE);
						max8986_power->suspended = true;	
						power_supply_changed(&max8986_power->battery);
					}
					//if (max8986_power->max8986->pdata->pmu_event_cb)
						//max8986_power->max8986->pdata->pmu_event_cb(PMU_EVENT_BATT_TEMP_BEYOND_LIMIT, temp);
				}
			}
			else if ((celsius >= LOW_RECOVER_TEMP) && (celsius <= HIGH_RECOVER_TEMP)) 
			{
				if (max8986_power->suspended==true) 
				{
					max8986_power->suspended = false;
					max8986_power->batt_health =POWER_SUPPLY_HEALTH_GOOD;

					if (max8986_power->is_charger_inserted)
					{
						max8986_start_charging(max8986_power,max8986_power->power_src);
						power_supply_changed(&max8986_power->battery);
					}
				}
			}				
		}
		
	}
	/*Reading voltage level from ADC*/
#if 0	
#if defined(CONFIG_BATT_LVL_FROM_ADC)
	max8986_get_batt_level_adc(max8986_power);
#else
#if defined(CONFIG_BRCM_FUSE_RIL_CIB)
	pr_info("%s: calling KRIL_DevSpecific_Cmd\n", __func__);
	if (KRIL_DevSpecific_Cmd(BCM_POWER_CLIENT,
				RIL_DEVSPECIFICPARAM_BCM_PMU_GET_BATT_ADC,
				NULL, 0) == FALSE)
		pr_err("%s: KRIL_DevSpecific_Cmd failed\n", __func__);
#endif
#endif
#else
	if(pmu_is_charger_inserted())
	{
		cnt_func_called++;

		if( ((check_func_called_by_cp==true) && (cnt_func_called>=10)) ||( cnt_func_called  >= 60) ||( init_batt_lvl_interval<=3)
                      || ((check_func_called_by_cp==true) && (max8986_power->charging_status ==  POWER_SUPPLY_STATUS_FULL)) )
		{
			pr_info("%s: cnt_func_called=%d init_batt_lvl_interval=%d\n",__func__, cnt_func_called,init_batt_lvl_interval);
			cnt_func_called=0;
			max8986_get_batt_level_adc(max8986_power);		
		
			if( mon_interval == BATTERY_LVL_MON_INTERVAL_WHILE_CHARGING)
				check_func_called_by_cp=true;
		}
		inner_function(max8986_power);
	}
	else
	{
		if (KRIL_DevSpecific_Cmd(BCM_POWER_CLIENT,RIL_DEVSPECIFICPARAM_BCM_PMU_GET_BATT_ADC,NULL, 0) == FALSE)
			pr_err("%s: KRIL_DevSpecific_Cmd failed\n", __func__);		
	}
#endif

#if defined(CONFIG_HAS_WAKELOCK)
		wake_lock(&max8986_power->temp_adc_wl);
#endif
	max8986_check_batt_vf(max8986_power);
#if defined(CONFIG_HAS_WAKELOCK)
		wake_unlock(&max8986_power->temp_adc_wl);
#endif

#if 0	
	queue_delayed_work(max8986_power->max8986->pmu_workqueue,
			&max8986_power->batt_lvl_mon_wq,
			msecs_to_jiffies(mon_interval));
#else
	schedule_delayed_work(&max8986_power->batt_lvl_mon_wq, msecs_to_jiffies(mon_interval));

#endif
	power_supply_changed(&max8986_power->battery);

}
/*****************************************************************************
 * MUIC event handler
 *****************************************************************************/

static void max8986_muic_event(int event, u32 param,  void *data)
{
	struct max8986_power *max8986_power = data;
	struct max8986 *max8986 = max8986_power->max8986;
	pr_info("%s:event = %d param = %d\n", __func__, event,param);
	switch(event)
	{
	case MAX8986_MUIC_EVENT_CHARGER_OVP:
		if(param)
		{
			/* stop charging on over-voltage*/
			max8986_power->batt_health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
			max8986_stop_charging(max8986_power, true);
			is_ovp_suspended = true;
		}
		else
		{
			/* re-start charging as charger voltage has come down to valid limit*/
			max8986_power->charger_type = max8986_muic_get_charger_type();
			if (max8986_power->charger_type != PMU_MUIC_CHGTYP_NONE)
			{
				/* Notify through event callback */
				if(max8986->pdata->pmu_event_cb)
				{
					max8986->pdata->pmu_event_cb(PMU_EVENT_CHARGER_INSERT,
						max8986_power->charger_type);
				}
				max8986_power->batt_health = POWER_SUPPLY_HEALTH_GOOD;
				max8986_start_charging(max8986_power, max8986_power->charger_type);
			}
			is_ovp_suspended = false;
		}
		break;

	case MAX8986_MUIC_EVENT_DCDTMR:
		max8986_power->dcd_timout = true;
		break;

	case MAX8986_MUIC_EVENT_CHARGER_TYPE:

		if(param == PMU_MUIC_CHGTYP_NONE)
		{
			max8986_power->dcd_timout = false;
			pr_info("%s: Charger Removed\n", __func__);
			if(max8986_power->power_src == POWER_SUPPLY_TYPE_BATTERY)
				break;
			if (max8986->pdata->pmu_event_cb)
				max8986->pdata->pmu_event_cb(PMU_EVENT_CHARGER_REMOVE,
					max8986_power->charger_type);
			
			max8986_power->is_charger_inserted=FALSE;
			max8986_power->isFullcharged = FALSE;
			max8986_power->is_timer_expired=FALSE;
			is_ovp_suspended = false;
#if defined(CONFIG_HAS_WAKELOCK)
			//if(max8986_power->charger_type == PMU_MUIC_CHGTYP_USB ||max8986_power->charger_type == PMU_MUIC_CHGTYP_DOWNSTREAM_PORT)

			//	wake_unlock(&max8986_power->usb_charger_wl);	/* wake lock is needed only for usb */
#endif
/* Disable charging in pmu */
			max8986_stop_charging(max8986_power, true);
			max8986_power->charger_type = param;
		}
		else
		{
			if( is_ovp_suspended == true){
				pr_info("%s: ovp suspended\n", __func__);
				break;
			}
			if(max8986_power->dcd_timout)
			{
				pr_info("%s: Invalid Charger\n", __func__);
				break;
			}
			max8986_power->charger_type = param;
#if defined(CONFIG_HAS_WAKELOCK)
			//if(max8986_power->charger_type == PMU_MUIC_CHGTYP_USB ||max8986_power->charger_type == PMU_MUIC_CHGTYP_DOWNSTREAM_PORT)
				wake_lock(&max8986_power->usb_charger_wl);	/* wake lock is needed only for usb */
#endif
				/* Notify through event callback */
			if(max8986->pdata->pmu_event_cb)
			{
				max8986->pdata->pmu_event_cb(PMU_EVENT_CHARGER_INSERT,
						max8986_power->charger_type);
			}
			max8986_power->is_charger_inserted= TRUE;

			max8986_start_charging(max8986_power, max8986_power->charger_type);
		}
		break;

	default:
		break;
	}
}
/*****************************************************************************
 * power driver interrupt handling
 *****************************************************************************/
/* Charger insertion Check For TSP */
#if defined(CONFIG_TOUCHSCREEN_F760)
extern void set_tsp_for_ta_detect(int state);
int tsp_charger_type_status=0;
EXPORT_SYMBOL(tsp_charger_type_status);
/* Charger insertion Check For TSP */
#endif

static void max8986_power_isr(int irq, void *data)
{
	u8 hostact;
	struct max8986_power *max8986_power = data;
	struct max8986 *max8986 = max8986_power->max8986;

	pr_info("%s:interrupt id = %u\n", __func__, irq);
	switch (irq) {
	case MAX8986_IRQID_INT2_CHGINS:
		/* Charger insertion is taken care in CHGTYP interrupt */
            #if defined(CONFIG_TOUCHSCREEN_F760)
             tsp_charger_type_status = TRUE;//TSP Charging[JG]
             set_tsp_for_ta_detect(1);//TSP Charging[JG]
            #endif
		pr_info("%s: Charger inserted\n", __func__);
		break;

	case MAX8986_IRQID_INT2_CHGRM:
		pr_info("%s: Charger Removed\n", __func__);
            #if defined(CONFIG_TOUCHSCREEN_F760)
             tsp_charger_type_status = FALSE;//TSP Charging[JG]
             set_tsp_for_ta_detect(0);//TSP Charging[JG]
             #endif
		if( max8986_power->batt_health == POWER_SUPPLY_HEALTH_DEAD){		
			max8986->read_dev(max8986, MAX8986_PM_REG_HOSTACT, &hostact);						
			hostact |= MAX8986_HOSTACT_HOSTDICOFF;
			max8986->write_dev(max8986, MAX8986_PM_REG_HOSTACT, hostact);			
		}			 
		break;

	case MAX8986_IRQID_INT2_CHGEOC:
		pr_info("%s: End of Charging\n", __func__);
		max8986_disable_irq(max8986, MAX8986_IRQID_INT2_CHGEOC);
#ifdef CONFIG_BLX
	if(max8986_power->batt_percentage >= get_charginglimit())
#else
	if(max8986_power->isFullcharged == TRUE)
#endif
		max8986_power->charging_status = POWER_SUPPLY_STATUS_FULL;
		max8986_power->isFullcharged = TRUE;
		max8986_power->is_timer_expired=FALSE;		
		max8986_power->batt_percentage=100;
		prev_scaled_level=100;
		power_supply_changed(&max8986_power->battery);
		max8986_stop_charging(max8986_power,FALSE);
#ifdef MAX8986_LOG_CHARGING_TIME
		max8986_power->charging_end_time = ktime_get();
		max8986_power->charging_time =
			ktime_sub(max8986_power->charging_end_time,
					max8986_power->charging_start_time);
		pr_info("%s:Total Charging Time	%lld us\n", __func__,
				ktime_to_us(max8986_power->charging_time));
#endif
		break;
	case MAX8986_IRQID_INT2_MBCCHGERR:
		pr_info("%s:MAX8986_IRQID_INT2_MBCCHGERR\n", __func__);
		max8986_disable_irq(max8986, MAX8986_IRQID_INT2_MBCCHGERR);
#ifdef CONFIG_BLX
	if(max8986_power->batt_percentage >= get_charginglimit())
#else
	if(max8986_power->isFullcharged == TRUE)
#endif
		max8986_power->charging_status = POWER_SUPPLY_STATUS_FULL;
		max8986_power->isFullcharged = FALSE;
		power_supply_changed(&max8986_power->battery);	
		max8986_power->is_timer_expired=TRUE;
		max8986_power->next_expire_time = jiffies;
		break;
	case MAX8986_IRQID_INT2_CHGERR:
		pr_info("%s:MAX8986_IRQID_INT2_CHGERR\n", __func__);
		break;

	case MAX8986_IRQID_INT3_JIGONBINS:  
		pr_info("%s:MAX8986_IRQID_INT3_JIGONBINS\n", __func__);
#if defined(CONFIG_HAS_WAKELOCK)
        	wake_lock(&max8986_power->jig_on_wl);
#endif

#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
		cpufreq_bcm_dvfs_disable(jig_client);
#endif
		break;

	case MAX8986_IRQID_INT3_JIGONBRM:
		pr_info("%s:MAX8986_IRQID_INT3_JIGONBRM\n", __func__);
#if defined(CONFIG_HAS_WAKELOCK)
       	wake_unlock(&max8986_power->jig_on_wl);
#endif

#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
		cpufreq_bcm_dvfs_enable(jig_client);
#endif
		break;

    default:
		pr_info("%s:Not supporting irq\n", __func__);
		break;
		
	} /* switch (irq) */
}
int pmu_is_charger_inserted()
{
	struct max8986_power *max8986_power;

	if(!power_device)
		return FALSE;

	max8986_power = platform_get_drvdata(power_device);	

	return max8986_power->is_charger_inserted;

}
int current_charger_status(struct max8986_power *max8986_power)
{
#if 0
      //	return g_ptr_data->is_charger_inserted;

	struct max8986_power *max8986_power;
	if(!power_device)
		return -2;

	max8986_power = platform_get_drvdata(power_device);
	if(max8986_power) {
        return max8986_power->is_charger_inserted;
    }
    else{
		pr_err("%s: max8986_power is NULL ERROR\n", __func__);
        return -1;
    }
#else
	struct max8986 *max8986 = max8986_power->max8986;
	int ret,chg_typ;
	unsigned char status2=0;

	ret = max8986->read_dev(max8986, MAX8986_MUIC_REG_STATUS2, &status2);
	if (ret < 0) {
		pr_err("%s: MAX8986_MUIC_REG_STATUS2 read failed\n", __func__);
		return -EIO;
	}
	pr_info("%s: STATUS2:0x%02x\n", __func__,status2);


	chg_typ = status2 & MAX8986_M_STATUS2_CHGTYP_MASK;
	switch(chg_typ){
		case PMU_MUIC_CHGTYP_USB:
		case PMU_MUIC_CHGTYP_DOWNSTREAM_PORT:
		case PMU_MUIC_CHGTYP_DEDICATED_CHGR:
			return TRUE;
		default:
			return FALSE;
	}
#endif	

}
EXPORT_SYMBOL(pmu_is_charger_inserted);
void set_button(int value)
{
	struct max8986_power *max8986_power;

	if(!power_device)
		return;

	max8986_power = platform_get_drvdata(power_device);	

	max8986_power->button = value;
}
EXPORT_SYMBOL(set_button);
int max8986_ear_adc()
{
	int ear_adc = 0;
	struct max8986_power *max8986_power;

	if(!power_device)
		return ear_adc;

	max8986_power = platform_get_drvdata(power_device);	

	ear_adc = auxadc_access(2);
	max8986_power->ear_adc = ear_adc;
	ear_adc >>= 2;
	//power_supply_changed(&max8986_power->battery);
	pr_info("%s: ear=%d---------------\n", __func__,ear_adc);

	return ear_adc;
		
}
EXPORT_SYMBOL(max8986_ear_adc);

/*****************************************************************************
 * usb driver callbacks
 *****************************************************************************/
int pmu_get_usb_enum_current(void)
{
	int cc = MAX8986_CHARGING_CURR_UNKNOWN;
	struct max8986_power *max8986_power;
	pr_info("%s\n", __func__);
	if(!power_device)
	{
		pr_info("%s:Power driver not initialized \n", __func__);
		return cc;
	}

	max8986_power = platform_get_drvdata(power_device);
	if(max8986_power)
	{
		struct max8986_power_pdata *pdata = max8986_power->max8986->pdata->power;
		switch(max8986_power->charger_type)
		{
		case PMU_MUIC_CHGTYP_USB:
		case PMU_MUIC_CHGTYP_DOWNSTREAM_PORT:
			cc = pdata->usb_charging_cc;
		break;

		case PMU_MUIC_CHGTYP_DEDICATED_CHGR:
			cc =  pdata->wac_charging_cc;
			break;

		case PMU_MUIC_CHGTYP_SPL_500MA:
			cc =  MAX8986_CHARGING_CURR_500MA;
			break;

		case PMU_MUIC_CHGTYP_SPL_1A:
			cc =  MAX8986_CHARGING_CURR_950MA;
			break;

		case PMU_MUIC_CHGTYP_DEAD_BATT_CHG:
			cc = MAX8986_CHARGING_CURR_90MA;
			break;

		default:
			break;
		}
	}
	pr_info("%s : cc = %x\n",__func__,cc);
	return cc;
}

EXPORT_SYMBOL(pmu_get_usb_enum_current);

extern void pmu_start_charging(void)
{
	struct max8986_power *max8986_power;
	if(!power_device)
	{
		pr_info("%s:Power driver not initialized \n", __func__);
		return;
	}

	max8986_power = platform_get_drvdata(power_device);
	if(max8986_power)
	{
		max8986_start_charging(max8986_power,max8986_power->charger_type);
	}
}
EXPORT_SYMBOL(pmu_start_charging);

extern void pmu_stop_charging(void)
{
	struct max8986_power *max8986_power;
	if(!power_device)
	{
		pr_info("%s:Power driver not initialized \n", __func__);
		return;
	}

	max8986_power = platform_get_drvdata(power_device);
	if(max8986_power)
	{
		max8986_stop_charging(max8986_power, false);
	}
}
EXPORT_SYMBOL(pmu_stop_charging);

pmu_charging_current pmu_get_charging_current()
{
	pmu_charging_current cc = MAX8986_CHARGING_CURR_UNKNOWN;
	struct max8986_power *max8986_power;
	if(!power_device)
	{
		pr_info("%s:Power driver not initialized \n", __func__);
		return cc;
	}

	max8986_power = platform_get_drvdata(power_device);
	if(max8986_power)
		cc = max8986_get_fc_current(max8986_power);
	pr_info("%s: cc: %d\n", __func__, cc);
	return cc;
}
EXPORT_SYMBOL(pmu_get_charging_current);

void pmu_set_usb_enum_current(bool pre_enum)
{
	struct max8986_power *max8986_power;
	if(!power_device)
	{
		pr_info("%s:Power driver not initialized \n", __func__);
		return;
	}
	max8986_power = platform_get_drvdata(power_device);

	pr_info("%s: pre_enum: %d\n", __func__, pre_enum);
	if(max8986_power)
	{
		struct max8986_power_pdata *pdata = max8986_power->max8986->pdata->power;
		if(pre_enum)
			max8986_set_fc_current(max8986_power,USB_PREENUM_CURR_REQ_VAL);
		else
			max8986_set_fc_current(max8986_power,pdata->usb_charging_cc);
	}
}
EXPORT_SYMBOL(pmu_set_usb_enum_current);

void pmu_set_charging_current(pmu_charging_current charging_cur)
{
	struct max8986_power *max8986_power;
	if(!power_device)
	{
		pr_info("%s:Power driver not initialized \n", __func__);
		return;
	}

	max8986_power = platform_get_drvdata(power_device);
	pr_debug("%s: charging_cur: %x\n", __func__, charging_cur);
	if(max8986_power)
		max8986_set_fc_current(max8986_power,charging_cur);
}

EXPORT_SYMBOL(pmu_set_charging_current);

u8 max8986_read_accessory_type()
{
	struct max8986 *max8986;
	struct max8986_power *max8986_power;
	u8 status1,adc_val;
	int ret;

	if(!power_device)
		return PMU_MUIC_ADC_OUTPUT_INIT;

	max8986_power = platform_get_drvdata(power_device);	
	max8986 = max8986_power->max8986;
	
	ret = max8986->read_dev(max8986, MAX8986_MUIC_REG_STATUS1, &status1);
	if (ret < 0) {
		pr_err("%s: MAX8986_MUIC_REG_STATUS1 read failed\n", __func__);
		return PMU_MUIC_ADC_OUTPUT_INIT;
	}

	adc_val = MAX8986_M_STATUS1_ADC_MASK & status1;

	return adc_val;

}
EXPORT_SYMBOL(max8986_read_accessory_type);


static int max8986_power_ioctl_handler(u32 cmd, u32 arg, void *pri_data)
{
	struct max8986_power *max8986_power = pri_data;
	int ret = -EINVAL;
	pr_info("Inside %s, IOCTL command %d\n", __func__, cmd);
	switch (cmd) {
	case BCM_PMU_IOCTL_START_CHARGING:
	{
		int pwr_spply_type;
		if (copy_from_user(&pwr_spply_type, (int *)arg, sizeof(int))
				!= 0) {
			pr_info("Error copying data from user\n");
			return -EFAULT;
		}
		pr_info("max8986_power_ioctl_handler: max8986_power->power_src \
				%d, pwr_spply_type %d\n",
				max8986_power->power_src, pwr_spply_type);
		if (max8986_power->power_src != pwr_spply_type)
			return -EINVAL;
		if (max8986_power->charging_status !=
				POWER_SUPPLY_STATUS_CHARGING) {
			max8986_start_charging(max8986_power,
					max8986_muic_get_charger_type());
			ret = SUCCESS;
		} else {
			pr_info("max8986_power: already in charging mode or \
					charger is not connected\n");
			ret = -EPERM;
		}
		break;
	}
	case BCM_PMU_IOCTL_STOP_CHARGING:
	{
		if ((max8986_power->charging_status !=
					POWER_SUPPLY_STATUS_DISCHARGING) &&
				(max8986_power->charging_status !=
				 POWER_SUPPLY_STATUS_NOT_CHARGING)) {
			max8986_stop_charging(max8986_power, FALSE);
			ret = SUCCESS;
		} else {
			pr_info("max8986_power: already not in charging mode\
					\n");
			ret = -EPERM;
		}
		break;
	}
	case BCM_PMU_IOCTL_SET_CHARGING_CURRENT:
	{
		/* Not required for now */
		break;
	}
	case BCM_PMU_IOCTL_GET_CHARGING_CURRENT:
	{
		/* Not required for now */
		break;
	}
	} /* End of switch */

	return ret;
}

static int spa_Create_Attrs(struct device * dev, struct device_attribute *power_attr,  int array_size)
{
	int i; 
	int rc = EINVAL;

	for (i = 0; i < array_size; i++)
	{
		rc = device_create_file(dev, power_attr+i);
		if (rc)
		{
			device_remove_file(dev, power_attr+i);
		}
	}
	return rc;
}

static ssize_t spa_Test_Show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t spa_Test_Store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

#ifdef FEAT_EN_TEST_MODE
#define SPA_TEST_ATTR(_name)													\
{																				\
        .attr = { .name = #_name, .mode = S_IRUGO | S_IWUG, .owner = THIS_MODULE },	\
        .show = spa_Test_Show,														\
        .store = spa_Test_Store,														\
}


enum {
	BATT_DEBUG_LEVEL =0,
	BATT_HEALTH,
	BATT_EOC,
	BATT_SET_CHG_TIMER,
	BATT_START_RECHARGE,
	BATT_AVG_VOLT,
	BATT_AVG_TEMP,
	BATT_AVG_TEMP_AVER,	
	BATT_CUR_VOLT,
	BATT_CUR_TEMP,
	BATT_VOLT_ADC,
	BATT_VOLT_ADC_AVER,
	BATT_TEMP_ADC,
	BATT_TEMP_ADC_AVER,	
	BATT_CAPACITY,	
	BATT_STATUS,
	BATT_IS_FULL,
	BATT_IS_CHG,
	BATT_IS_JIG,
	BATT_LEVEL,
	BATT_TEMP,
	BATT_VOLT,
	BUTTON,
	EAR_ADC,
	BATT_RECHG_COUNT,
	BATT_GET_TIMER_STATUS,
	BATT_LEFT_CHG_TIME,
};

static struct device_attribute spa_Test_Attrs[] = {
	SPA_TEST_ATTR(debug_level),
	SPA_TEST_ATTR(health),
	SPA_TEST_ATTR(EOC),
	SPA_TEST_ATTR(timerExpire),
	SPA_TEST_ATTR(reCharge),
	SPA_TEST_ATTR(batt_vol),
	SPA_TEST_ATTR(batt_temp),
	SPA_TEST_ATTR(batt_temp_aver),	
	SPA_TEST_ATTR(batt_vol_aver),
	SPA_TEST_ATTR(batt_tmp_adc_aver),
	SPA_TEST_ATTR(batt_vol_adc_aver),
	SPA_TEST_ATTR(batt_vol_adc),	
	SPA_TEST_ATTR(batt_temp_adc),
	SPA_TEST_ATTR(batt_temp_adc_aver),	
	SPA_TEST_ATTR(capacity),	
	SPA_TEST_ATTR(status),
	SPA_TEST_ATTR(isFullcharged),
	SPA_TEST_ATTR(is_charger_inserted),
	SPA_TEST_ATTR(is_jig_inserted),	
	SPA_TEST_ATTR(level),	
	SPA_TEST_ATTR(tmp),	
	SPA_TEST_ATTR(volt),		
	SPA_TEST_ATTR(button),	
	SPA_TEST_ATTR(ear_adc),		
	SPA_TEST_ATTR(rechargeCount),
	SPA_TEST_ATTR(get_timer_status),
	SPA_TEST_ATTR(left_chg_time),
};

static ssize_t spa_Test_Show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int tmp,tmp_value;
	ssize_t i = 0;
	const ptrdiff_t off = attr - spa_Test_Attrs;


	struct max8986_power *max8986_power =
		dev_get_drvdata(dev->parent);
	struct max8986_power_pdata *pdata;
	if (unlikely(!max8986_power || !max8986_power->max8986)) {
		pr_err("%s: invalid driver data\n", __func__);
		return -EINVAL;
	}
	pdata = max8986_power->max8986->pdata->power;

	switch (off)
	{
		#if 1
		//case BATT_DEBUG_LEVEL:
		//	i += scnprintf(buf + i, PAGE_SIZE - i, "Debug Level = <%d>\n", pc_debug_level);
		//	break;
		case BATT_HEALTH:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_health);
			break;
		//case BATT_EOC:
		//	i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", SPA_GET_STATUS(STATUS_EOC));
		//	break;
		//case BATT_START_RECHARGE:
		//	i += scnprintf(buf + i, PAGE_SIZE -i, "%d\n", SPA_GET_STATUS(STATUS_RECHG));
		//	break;
		case BATT_AVG_VOLT:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_voltage);
			break;
		case BATT_AVG_TEMP:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_temp_celsius);
			break;
		case BATT_AVG_TEMP_AVER:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_temp_celsius);
			break;			
		case BATT_CUR_VOLT:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_voltage);
			break;
		case BATT_CUR_TEMP:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_temp_celsius);
			break;
		case BATT_VOLT_ADC :
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_adc_avg);
			break;
		case BATT_VOLT_ADC_AVER :
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_adc_avg);
			break;			
		case BATT_TEMP_ADC :
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_temp_adc_avg);
			break;
		case BATT_TEMP_ADC_AVER:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_temp_adc_avg);
			break;			
		case BATT_CAPACITY:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->batt_percentage);
			break;
		case BATT_STATUS:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->charging_status);
			break;
		case BATT_IS_FULL:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->isFullcharged);
			break;
		case BATT_IS_CHG:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->is_charger_inserted);
			break;
		case BATT_IS_JIG:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->is_jig_inserted);
			break;
		case BATT_LEVEL:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->level);
			break;
		case BATT_TEMP:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->tmp);			
			break;
		case BATT_VOLT:
			tmp = max8986_power->batt_voltage/10;
			tmp_value = max8986_power->batt_voltage/1000;
			scnprintf(max8986_power->volt,10,"%d.%d", tmp_value,tmp-100*tmp_value);
			i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n", max8986_power->volt);					
			break;
		case BUTTON:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n", ((max8986_power->button)?"ON":"OFF"));
			break;			
			
		case EAR_ADC:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", max8986_power->ear_adc);
			break;			
				
			
			break;
			
		//case BATT_RECHG_COUNT:
		//	i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", SPA_GET_STATUS(STATUS_RECHG_COUNT));
		//	break;
		//case BATT_GET_TIMER_STATUS :
		//	i += scnprintf(buf + i, PAGE_SIZE -i, "%d\n", SPA_GET_STATUS(STATUS_CHG_TIMER));
		//	break;
		//case BATT_LEFT_CHG_TIME :
		//	if(SPA_GET_STATUS(STATUS_CHG_TIMER) == CHARGE_TIMER_STARTED)
		//		i += scnprintf(buf + i, PAGE_SIZE -i, "%d\n", jiffies_to_msecs(g_pdata->Charge_timer.expires - jiffies)/1000);
		//	else
		//		i += scnprintf(buf + i, PAGE_SIZE -i, "%d\n", 0);
		//	break;	
		#endif
		default :
			break;
	}
	
	return i;
}

static ssize_t spa_Test_Store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = 0;
	const ptrdiff_t off = attr - spa_Test_Attrs;

	switch (off)
	{
		#if 0
		case BATT_DEBUG_LEVEL:
		        sscanf(buf, "%u", &pc_debug_level);
		        printk(KERN_ALERT "Debug Level = %d\n", pc_debug_level);

		        ret = count;
			break;
		case BATT_HEALTH:
			sscanf(buf, "%hd", &g_spa_data.Charge_Health);
		       printk(KERN_ALERT "Charge health = %d\n", g_spa_data.Charge_Health);

			schedule_work(&g_pdata->Charge_work);

			ret = count;
			break;
		case BATT_EOC:
			sscanf(buf, "%hd", &g_spa_data.isEndOfCharge);
		       printk(KERN_ALERT "EndOfCharge = <%d>\n", g_spa_data.isEndOfCharge);

			spa_EOC_Interrupt();

			ret=count;
			break;
		case BATT_SET_CHG_TIMER:
		{
			u16 v_Remain_Time;

			sscanf(buf, "%hd", &v_Remain_Time);
		       printk(KERN_ALERT "Timer exipre after <%d>s\n", v_Remain_Time);

			if(timer_pending(&g_pdata->Charge_timer))
			{
				SPA_DBG_MSG(1, "spa_Stop_Charge_Timer : PendingTimer exist, Delete Timer\n"); 
				del_timer(&g_pdata->Charge_timer);
			}
			
			g_pdata->Charge_timer.expires = jiffies + v_Remain_Time*HZ; 

			add_timer(&g_pdata->Charge_timer);
			SPA_SET_STATUS(STATUS_CHG_TIMER, CHARGE_TIMER_STARTED);
		}
			ret = count;
			break;
		case BATT_START_RECHARGE:
			sscanf(buf, "%hd", &g_spa_data.isRecharge);
		       printk(KERN_ALERT "isRecharge = %d\n", g_spa_data.isRecharge);

			SPA_SET_STATUS(STATUS_RECHG, TRUE);	
				
			schedule_work(&g_pdata->Charge_work);

			ret = count;
			break;
		case BATT_AVG_VOLT:
			sscanf(buf, "%ld", &gv_AverageVbat);
		       printk(KERN_ALERT "Battery Voltage stored as = %ld\n", gv_AverageVbat);

			if(gv_AverageVbat == -100)
				isTestMode = FALSE;
			else
			isTestMode = TRUE;

			ret = count;
			break;
		case BATT_AVG_TEMP:
			sscanf(buf, "%ld", &gv_AverageTbat);
		       printk(KERN_ALERT "Battery Temperature stored as = %ld\n", gv_AverageTbat);

			if(gv_AverageTbat == -100)
				isTestMode = FALSE;
			else
				isTestMode = TRUE;

			ret = count;
			break;
		#endif
		default :
			break;
	}

	return ret;
}
#endif


#if defined(CONFIG_HAS_EARLYSUSPEND)
static void max8986_power_early_suspend(struct early_suspend *h)
{
	struct max8986_power *max8986_power =
		container_of(h, struct max8986_power, early_suspend_desc);
	pr_info("[%s]+++\n", __func__);
	if(!max8986_power->is_charger_inserted )
	{
		pr_info("[%s] cancel delayed work \n", __func__);	
		cancel_delayed_work_sync(&max8986_power->batt_lvl_mon_wq);
		wake_unlock(&max8986_power->usb_charger_wl);	/* wake lock is needed only for usb */

	}	
	else
	{
		cancel_delayed_work_sync(&max8986_power->batt_lvl_mon_wq);
		pr_info("[%s] reschedule delayed work \n", __func__);	
		schedule_delayed_work(&max8986_power->batt_lvl_mon_wq,msecs_to_jiffies(500));	
	}
	pr_info("[%s] chg=%d, jig=%d -------------\n", __func__,max8986_power->is_charger_inserted,max8986_power->is_jig_inserted);		
	pr_info("[%s]+++\n", __func__);
}
static void max8986_power_late_resume(struct early_suspend *h)
{
	struct max8986_power *max8986_power =
		container_of(h, struct max8986_power, early_suspend_desc);
	pr_info("[%s]+++\n", __func__);
	schedule_delayed_work(&max8986_power->batt_lvl_mon_wq,
			msecs_to_jiffies(500));
	max8986_power->batt_temp_adc_avg=0;
	max8986_get_batt_temperature(max8986_power);

	pr_info("[%s]+++\n", __func__);
}
#else
#ifdef CONFIG_PM
static int max8986_power_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	struct max8986_power *max8986_power = platform_get_drvdata(pdev);
	if (max8986_power)
		cancel_delayed_work_sync(&max8986_power->batt_lvl_mon_wq);
	return 0;
}

static int max8986_power_resume(struct platform_device *pdev)
{
	struct max8986_power *max8986_power = platform_get_drvdata(pdev);
	if (max8986_power) {
		schedule_delayed_work(&max8986_power->batt_lvl_mon_wq,
				msecs_to_jiffies(2000));
	}
	return 0;
}
#else
#define max8986_power_suspend NULL
#define max8986_power_resume NULL
#endif /* CONFIG_PM */
#endif /*CONFIG_HAS_EARLYSUSPEND*/

/*****************************************************************************
 * init code
 *****************************************************************************/
static void max8986_init_charger(struct max8986_power *max8986_power)
{
	u8 reg_val;
	struct max8986 *max8986 = max8986_power->max8986;
	struct max8986_power_pdata *plat_data = max8986->pdata->power;

//	reg_val = MAX8986_MBCCTRL1_TFCH_DISABLE;
	reg_val = MAX8986_MBCCTRL1_TFCH_5HR;
	max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL1, reg_val);

	/* Set 4.20V as charging voltage bydefault */
	max8986->read_dev(max8986, MAX8986_PM_REG_MBCCTRL3, &reg_val);
	reg_val |= MBCCV_4_20V;
	max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL3, reg_val);

	/* Set 450mA as charging current bydefault */
	reg_val = MAX8986_CHARGING_CURR_450MA ;
	max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL4, reg_val);

	/* Set 60mA as EOC */
	max8986->read_dev(max8986, MAX8986_PM_REG_MBCCTRL7, &reg_val);
	if (plat_data->eoc_current)
		reg_val |= plat_data->eoc_current;
	else
		reg_val |= MAX8986_EOC_60MA;
	max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL7, reg_val);

	/*Disable charging by default*/
	//reg_val = 0;//~(MAX8986_MBCCTRL2_VCHGR_FC | MAX8986_MBCCTRL2_MBCHOSTEN) - other bits are unused
	//max8986->write_dev(max8986, MAX8986_PM_REG_MBCCTRL2, reg_val);

	/*Register for PM interrupts */
	max8986_request_irq(max8986, MAX8986_IRQID_INT2_CHGEOC, FALSE,
		max8986_power_isr, max8986_power);
	max8986_request_irq(max8986, MAX8986_IRQID_INT2_CHGINS, TRUE,
		max8986_power_isr, max8986_power);
	max8986_request_irq(max8986, MAX8986_IRQID_INT2_CHGERR, FALSE,
		max8986_power_isr, max8986_power);
	max8986_request_irq(max8986, MAX8986_IRQID_INT2_CHGRM, TRUE,
		max8986_power_isr, max8986_power);
	max8986_request_irq(max8986, MAX8986_IRQID_INT2_MBCCHGERR, FALSE,
		max8986_power_isr, max8986_power);
	max8986_request_irq(max8986, MAX8986_IRQID_INT3_VERYLOWBAT, TRUE,
		max8986_power_isr, max8986_power);
	max8986_request_irq(max8986, MAX8986_IRQID_INT3_JIGONBINS, TRUE,
		max8986_power_isr, max8986_power);
	max8986_request_irq(max8986, MAX8986_IRQID_INT3_JIGONBRM, TRUE,
		max8986_power_isr, max8986_power);

	max8986_muic_register_event_handler(MAX8986_MUIC_EVENT_CHARGER_OVP, max8986_muic_event,
							max8986_power);
	max8986_muic_register_event_handler(MAX8986_MUIC_EVENT_DCDTMR, max8986_muic_event,
							max8986_power);
	max8986_muic_register_event_handler(MAX8986_MUIC_EVENT_CHARGER_TYPE, max8986_muic_event,
							max8986_power);

}

static int __devinit max8986_power_probe(struct platform_device *pdev)
{
	u8 reg_val;
	struct max8986_power *max8986_power;
	struct max8986 *max8986 = dev_get_drvdata(pdev->dev.parent);
	int ret = 0;
	struct max8986_power_pdata *power_pdata;
#if defined(CONFIG_BRCM_FUSE_RIL_CIB) && !defined(CONFIG_BATT_LVL_FROM_ADC)
	unsigned long notify_id_list[] = { RIL_NOTIFY_DEVSPECIFIC_BATT_LEVEL };
#endif

	pr_info("%s\n", __func__);

	if (unlikely(!max8986->pdata || !max8986->pdata->power)) {
		pr_err("%s: invalid platform data\n", __func__);
		return -EINVAL;
	}

	max8986_power = kzalloc(sizeof(struct max8986_power), GFP_KERNEL);
	if (unlikely(!max8986_power)) {
		pr_err("%s: max8986_power memory alloc failed\n", __func__);
		return -ENOMEM;
	}
	max8986_power->max8986 = max8986;
	platform_set_drvdata(pdev, max8986_power);
	power_device = pdev;
	power_pdata = max8986->pdata->power;

	//g_ptr_data = max8986_power;
	
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
	jig_client = cpufreq_bcm_client_get("jig_client");
	if (!jig_client) {
		pr_err("%s: cpufreq_bcm_client_get failed\n", __func__);
		return -EIO;
	}
#endif
#if defined(CONFIG_HAS_WAKELOCK)
    // Prevent the full system suspend.
	wake_lock_init(&max8986_power->jig_on_wl, WAKE_LOCK_SUSPEND,
						__stringify(jig_on_wl));
#endif	

#if defined(CONFIG_HAS_WAKELOCK)
	wake_lock_init(&max8986_power->usb_charger_wl, WAKE_LOCK_SUSPEND,
						__stringify(usb_charger_wl));
	wake_lock_init(&max8986_power->temp_adc_wl, WAKE_LOCK_IDLE,
		 __stringify(temp_adc_wl));
#endif

	mutex_init(&max8986_power->power_mtx);

	/*set voltage to 20% by default */
	max8986_power->batt_percentage = BAT_PERCENT_INIT_VALUE;
	//max8986_power->batt_voltage = power_pdata->batt_adc_tbl.bat_vol[2]*1000;
	max8986_power->batt_voltage = power_pdata->batt_adc_tbl.bat_vol[power_pdata->batt_adc_tbl.num_entries-1]-1;	
	max8986_power->batt_health = POWER_SUPPLY_HEALTH_GOOD;
	max8986_power->isFullcharged = FALSE;
	max8986_power->is_charger_inserted= FALSE;
	max8986_power->is_jig_inserted= FALSE;
	max8986_power->is_timer_expired=FALSE;
	max8986_power->is_recharge=FALSE;
	max8986_power->suspended = FALSE;
	
	/* init pmu */
	max8986_init_charger(max8986_power);

	INIT_DELAYED_WORK(&max8986_power->batt_lvl_mon_wq,
		max8986_batt_lvl_mon_wq);

	max8986_power->power_src = POWER_SUPPLY_TYPE_BATTERY;
	max8986_power->charging_status = POWER_SUPPLY_STATUS_DISCHARGING;

	/*register power supplies */
	//max8986_power->wall.name = "max8986-wall";
	max8986_power->wall.name = "battery";
	max8986_power->wall.type = POWER_SUPPLY_TYPE_MAINS;
	max8986_power->wall.properties = wall_props;
	max8986_power->wall.num_properties = ARRAY_SIZE(wall_props);
	max8986_power->wall.get_property = max8986_wall_get_property;
	ret = power_supply_register(&pdev->dev, &max8986_power->wall);
	if (ret) {
		pr_err("%s: wall charger registration failed\n", __func__);
		goto wall_err;
	}

	max8986_power->battery.name = "max8986-battery";
	max8986_power->battery.properties = battery_props;
	max8986_power->battery.type = POWER_SUPPLY_TYPE_BATTERY;
	/* Temp property is kept as the last entry in battery_props array.
	 * Temp prop is registered only if a valid
	 * temp adc channel is specified in platform data
	 * */
	max8986_power->battery.num_properties =
		(power_pdata->temp_adc_channel >= 0) ? ARRAY_SIZE(battery_props)
		: ARRAY_SIZE(battery_props) - 1;
	max8986_power->battery.get_property = max8986_battery_get_property;
	ret = power_supply_register(&pdev->dev, &max8986_power->battery);
	if (ret) {
		pr_err("%s: battery registration failed\n", __func__);
		goto batt_err;
	}

	max8986_power->usb.name = "max8986-usb",
	    max8986_power->usb.type = POWER_SUPPLY_TYPE_USB;
	max8986_power->usb.properties = usb_props;
	max8986_power->usb.num_properties = ARRAY_SIZE(usb_props);
	max8986_power->usb.get_property = max8986_usb_get_property;
	ret = power_supply_register(&pdev->dev, &max8986_power->usb);
	if (ret) {
		pr_err("%s: usb power supply registration failed\n", __func__);
		goto usb_err;
	}

#if defined(CONFIG_BRCM_FUSE_RIL_CIB) && !defined(CONFIG_BATT_LVL_FROM_ADC)
	if (KRIL_Register(BCM_POWER_CLIENT, NULL,
		max8986_ril_adc_notify_cb, notify_id_list,
		ARRAY_SIZE(notify_id_list)) == FALSE) {
		pr_err("%s: KRIL_Register failed\n", __func__);
	}
#else
	pr_err("%s:KRIL_Register not defined\n", __func__);
#endif
	/* start the workqueue */
#if 0
	queue_delayed_work(max8986_power->max8986->pmu_workqueue,
		&max8986_power->batt_lvl_mon_wq, msecs_to_jiffies(500));
#else
	schedule_delayed_work(&max8986_power->batt_lvl_mon_wq, msecs_to_jiffies(500));

#endif
	max8986_register_ioctl_handler(max8986, MAX8986_SUBDEV_POWER,
	max8986_power_ioctl_handler, max8986_power);
#if defined(CONFIG_HAS_EARLYSUSPEND)
	max8986_power->early_suspend_desc.level =
		EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	max8986_power->early_suspend_desc.suspend = max8986_power_early_suspend;
	max8986_power->early_suspend_desc.resume = max8986_power_late_resume;
	register_early_suspend(&max8986_power->early_suspend_desc);
#endif /*CONFIG_HAS_EARLYSUSPEND*/

#ifdef FEAT_EN_TEST_MODE
	spa_Create_Attrs(max8986_power->wall.dev, spa_Test_Attrs, ARRAY_SIZE(spa_Test_Attrs));
#endif
	max8986_power->is_timer_expired=FALSE;
	max8986_power->is_recharge=FALSE;

	if( max8986_is_jig_inserted(max8986_power))
	{
		max8986_power->is_jig_inserted = TRUE;
#if defined(CONFIG_HAS_WAKELOCK)
        	wake_lock(&max8986_power->jig_on_wl);
#endif

#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
		cpufreq_bcm_dvfs_disable(jig_client);
#endif
	pr_info("%s jig inserted at init time\n", __func__);
	}
    	else
		max8986_power->is_jig_inserted = FALSE;

	if(current_charger_status(max8986_power)){
		max8986_power->is_charger_inserted = TRUE;
		max8986_ctrl_charging(max8986_power,TRUE);
	}
	else{
		max8986_power->is_charger_inserted = FALSE;		
		max8986_ctrl_charging(max8986_power,FALSE);
	}

	/* if usb/wall charger is already connected, then start the charging
	 * process by simulating charger insertion interrupt.
	 */
	 max8986->read_dev(max8986, MAX8986_PM_REG_ENV1, &reg_val);
	if (reg_val & MAX8986_ENV1_CGPD)
	{
		max8986_muic_force_charger_detection();
	}
	pr_info("%s: success\n", __func__);
	return 0;

usb_err:
	power_supply_unregister(&max8986_power->battery);
batt_err:
	power_supply_unregister(&max8986_power->wall);
wall_err:
	kfree(max8986_power);
	return ret;
}

static int __devexit max8986_power_remove(struct platform_device *pdev)
{
	struct max8986_power *max8986_power = platform_get_drvdata(pdev);

	if (max8986_power) {
		cancel_delayed_work_sync(&max8986_power->batt_lvl_mon_wq);
		power_supply_unregister(&max8986_power->wall);
		power_supply_unregister(&max8986_power->usb);
		power_supply_unregister(&max8986_power->battery);
#if defined(CONFIG_HAS_EARLYSUSPEND)
		unregister_early_suspend(&max8986_power->early_suspend_desc);
#endif
#if defined(CONFIG_HAS_WAKELOCK)
		wake_lock_destroy(&max8986_power->temp_adc_wl);
		wake_lock_destroy(&max8986_power->usb_charger_wl);
#endif
#if defined(CONFIG_HAS_WAKELOCK)
		wake_lock_destroy(&max8986_power->jig_on_wl);
#endif
	}
	kfree(max8986_power);
	return 0;
}

static struct platform_driver max8986_power_driver = {
	.driver = {
		.name = "max8986-power",
		.owner = THIS_MODULE,
	},
	.remove = __devexit_p(max8986_power_remove),
	.probe = max8986_power_probe,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= max8986_power_suspend,
	.resume		= max8986_power_resume,
#endif
};

static int __init max8986_power_init(void)
{
	return platform_driver_register(&max8986_power_driver);
}

late_initcall(max8986_power_init);

static void __exit max8986_power_exit(void)
{
	platform_driver_unregister(&max8986_power_driver);
}

module_exit(max8986_power_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Power Driver for Maxim MAX8986 PMU");
MODULE_ALIAS("platform:max8986-power");
