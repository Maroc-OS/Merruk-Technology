/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/mfd/bcm59035/bcm59035-private.h
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
*   @file   bcm59035-private.h
*
*   @brief  Contains definitions common to all BCM PMU chips
*
****************************************************************************/

#ifndef __BCM59035_PRIVATE_H__
#define __BCM59035_PRIVATE_H__

#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/regulator/machine.h>
#include <mach/pwm.h>
#include <plat/pwm/pwm_core.h>

#define BCM59035_LOG_ENABLE 1

#ifdef BCM59035_LOG_ENABLE
extern const u32 bcm59035_debug_mask;
#define PMU_LOG(level, fmt, args...) \
	do {\
		if (level & bcm59035_debug_mask)\
			printk("BCM59035: "fmt, ##args);\
	} while (0)
#else
#define PMU_LOG(level, fmt, args...)
#endif /*BCM59035_LOG_ENABLE */
#include <plat/bcm_i2c.h>

/* BCM59035 Regulator IDs */
enum {
	BCM59035_REGL_ALDO1 = 0,
	BCM59035_REGL_ALDO2,

	BCM59035_REGL_RFLDO1,
	BCM59035_REGL_RFLDO2,

	BCM59035_REGL_LCLDO,

	BCM59035_REGL_LVLDO1,
	BCM59035_REGL_LVLDO2,

	BCM59035_REGL_HCLDO1,
	BCM59035_REGL_HCLDO2,

	BCM59035_REGL_IOLDO,

	BCM59035_REGL_MSLDO1,
	BCM59035_REGL_MSLDO2,

	BCM59035_REGL_AUXLDO1,
	BCM59035_REGL_AUXLDO2,

	BCM59035_REGL_SIMLDO,

	BCM59035_REGL_CSR,
	BCM59035_REGL_IOSR,

	BCM59035_REGL_NUM_REGULATOR
};

/*ioctl cmds*/
#define BCM_PMU_MAGIC   'P'

#define BCM_PMU_CMD_FIRST               0x80

#define BCM_PMU_CMD_ENABLE_INTS         0x80
#define BCM_PMU_CMD_DISABLE_INTS        0x81
#define BCM_PMU_CMD_READ_REG            0x83
#define BCM_PMU_CMD_WRITE_REG           0x84
#define BCM_PMU_CMD_ACTIVATESIM         0x85
#define BCM_PMU_CMD_DEACTIVATESIM       0x86
#define BCM_PMU_CMD_GET_REGULATOR_STATE 0x87
#define BCM_PMU_CMD_SET_REGULATOR_STATE 0x88
#define BCM_PMU_CMD_SET_PWM_LED_CTRL    0x89
#define BCM_PMU_CMD_POWERONOFF          0x00
#define BCM_PMU_CMD_SET_PWM_HI_PER      0x8a
#define BCM_PMU_CMD_SET_PWM_LO_PER      0x8b
#define BCM_PMU_CMD_SET_PWM_PWR_CTRL    0x8c
#define BCM_PMU_CMD_GET_VOLTAGE         0x8d
#define BCM_PMU_CMD_SET_VOLTAGE         0x8e
#define BCM_PMU_CMD_START_CHARGING	0x8f
#define BCM_PMU_CMD_STOP_CHARGING	0x90
#define BCM_PMU_CMD_SET_CHARGING_CUR	0x91
#define BCM_PMU_CMD_GET_CHARGING_CUR	0x92

#define BCM_PMU_CMD_LAST                0x92

#define BCM_PMU_IOCTL_ENABLE_INTS       _IO(BCM_PMU_MAGIC, BCM_PMU_CMD_ENABLE_INTS)	/* arg is unused */
#define BCM_PMU_IOCTL_DISABLE_INTS      _IO(BCM_PMU_MAGIC, BCM_PMU_CMD_DISABLE_INTS)	/* arg is unused */
#define BCM_PMU_IOCTL_READ_REG          _IOWR(BCM_PMU_MAGIC, BCM_PMU_CMD_READ_REG, pmu_reg)
#define BCM_PMU_IOCTL_WRITE_REG         _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_WRITE_REG, pmu_reg)
#define BCM_PMU_IOCTL_ACTIVATESIM       _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_ACTIVATESIM, pmu_sim_volt)
#define BCM_PMU_IOCTL_DEACTIVATESIM     _IO(BCM_PMU_MAGIC, BCM_PMU_CMD_DEACTIVATESIM)	/* arg is unused */
#define BCM_PMU_IOCTL_GET_REGULATOR_STATE _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_REGULATOR_STATE, pmu_regl)
#define BCM_PMU_IOCTL_SET_REGULATOR_STATE _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_REGULATOR_STATE, pmu_regl)
#define BCM_PMU_IOCTL_GET_VOLTAGE        _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_VOLTAGE, pmu_regl_volt)
#define BCM_PMU_IOCTL_SET_VOLTAGE        _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_VOLTAGE, pmu_regl_volt)
#define BCM_PMU_IOCTL_SET_PWM_LED_CTRL    _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_LED_CTRL, pmu_pwm_ctrl)
#define BCM_PMU_IOCTL_POWERONOFF          _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_POWERONOFF, pmu_reg)
#define BCM_PMU_IOCTL_SET_PWM_HI_PER      _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_HI_PER, int)
#define BCM_PMU_IOCTL_SET_PWM_LO_PER      _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_LO_PER, int)
#define BCM_PMU_IOCTL_SET_PWM_PWR_CTRL    _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_PWR_CTRL, int)
#define BCM_PMU_IOCTL_GET_CHARGING_CURRENT    _IOR(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_CHARGING_CUR, int)
#define BCM_PMU_IOCTL_SET_CHARGING_CURRENT    _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_CHARGING_CUR, int)
#define BCM_PMU_IOCTL_START_CHARGING    _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_START_CHARGING, int)
#define BCM_PMU_IOCTL_STOP_CHARGING    _IO(BCM_PMU_MAGIC, BCM_PMU_CMD_STOP_CHARGING)

enum {
	PMU_EVENT_INIT_PLATFORM,
	PMU_EVENT_CHARGER_INSERT,
	PMU_EVENT_CHARGER_REMOVE,
	PMU_EVENT_BATT_TEMP_BEYOND_LIMIT,
	/*used in case of dedicated charger support to get the charger type */
	PMU_EVENT_GET_CHARGER_TYPE,
	/*used to convert temp adc to degree celsius */
	PMU_EVENT_BATT_TEMP_TO_DEGREE_C,

};

typedef int (*pmu_platform_callback) (int event, int param);

typedef struct {
	u8 reg;
	u8 val;
} pmu_reg;

typedef enum {
	SIM_3POINT0VOLT = 0,
	SIM_2POINT5VOLT,
	SIM_3POINT1VOLT,
	SIM_1POINT8VOLT,
	SIM_MAX_VOLTAGE
} pmu_sim_volt;

typedef enum {
	PMU_REGL_ON = 0x0,
	PMU_REGL_ECO,		/* 0x01 */
	PMU_REGL_OFF,		/* 0x02 */
	PMU_REGL_MASK
} pmu_regl_state;

typedef struct {
	int regl_id;
	pmu_regl_state state;
} pmu_regl;

typedef struct {
	u32 regoffset;
	u32 pwmled_ctrl;
	u32 pwmdiv;		/* divider value. fsys/x value. */
} pmu_pwm_ctrl;

typedef struct {
	int regl_id;
	int voltage;
	int min;
	int max;
	int step;
} pmu_regl_volt;

typedef int (*pmu_subdev_ioctl_handler) (u32 cmd, u32 arg, void *pri_data);

enum {
	DEBUG_PMU_INFO = 1U << 0,
	DEBUG_PMU_WARNING = 1U << 1,
	DEBUG_PMU_ERROR = 1U << 2,
};

enum {
	BCM59035_USE_REGULATORS = (1 << 0),
	BCM59035_USE_GPIO = (1 << 1),
	BCM59035_USE_RTC = (1 << 2),
	BCM59035_USE_POWER = (1 << 3),
	BCM59035_USE_PONKEY = (1 << 4),
	BCM59035_ENABLE_DVS = (1 << 5),
	BCM59035_REGISTER_POWER_OFF = (1 << 6),
	BCM59035_USE_DEDICATED_USB_CHARGER = (1 << 7),
	BCM59035_USE_PWM = (1 << 8),
	BCM59035_USE_LED = (1 << 9),
	BCM59035_ENABLE_NTC = (1 << 10),
};

/*regualtor DSM settings */
enum {
	BCM59035_REGL_LPM_IN_DSM,	/*if enabled, LPM in DSM (PC1 = 0) */
	BCM59035_REGL_OFF_IN_DSM,	/*if enabled, off in DSM (PC1 = 0) */
	BCM59035_REGL_ON_IN_DSM,	/*if enabled, ON in DSM (PC1 = 0) */
};

/*BCM59035 mfd sub device ids*/
enum {
	BCM59035_SUBDEV_POWER,
	BCM59035_SUBDEV_REGULATOR,
	BCM59035_SUBDEV_MAX
};

typedef enum {
	NO_CHARGER = 0,
	HOST_CHARGER = 1,
	DEDICATED_CHARGER = 2,
	UNKNOWN_CHARGER = 3
} USB_Charger_Type_t;
/*
 * enums for BCM59035 PWM module
 */

/* pwm channel mode configuration */
enum bcm59035_pwm_mode {
	MODE_PWM_OFF_LED_OFF,
	MODE_PWM_OFF_LED_ON,
	MODE_PWM_ON_LED_OFF,
	MODE_PWM_ON_LED_ON,
};

/*
 * division factor for 'fsystem' frequency
 */
enum bcm59035_pwm_div {
	FSYSTEM_DIV_4,
	FSYSTEM_DIV_16,
	FSYSTEM_DIV_64,
	FSYSTEM_DIV_512,
};

/* pwm channel pad configuration in the PMU */
enum bcm59035_pwm_led_pad_sel {
	PAD_SEL_PWM1_PWM2,
	PAD_SEL_PWM1_LED2,
	PAD_SEL_LED1_PWM2,
	PAD_SEL_LED1_LED2,
};

struct bcm_pmu_irq {
	struct list_head node; /**< list for PMU irq delivery to PMU sub devices */
	void (*handler) (int, void *); /**< irq handler callback */
	void *data; /**< data to be passed to the irq handler */
	int irq; /**< IRQ number */
	bool irq_enabled; /**< irq enable/disable state */
};

struct bcm59035_regl_init_data {
	int regl_id; /**< Regulator ID */
	u8 dsm_opmode; /**< Mode of the regulator in deep sleep*/
	struct regulator_init_data *init_data; /**< intialization details of
	the regulator*/
};

struct bcm59035_regl_pdata {
	int num_regulators; /**< Number of regulators */
	struct bcm59035_regl_init_data *regl_init; /**< intialization details
	of the regulator*/
	u8 regl_default_pmmode[BCM59035_REGL_NUM_REGULATOR]; /**<Regulator PM
	mode. Based on this mode, regulator will be off/on/power saving mode*/
};

struct charger_info {
	u8 tc_current; /**< trickle charge current */
	u8 rc_current; /**< rapid charge current*/
	u8 tc_thold; /**< trickle charge current threshold*/
	u8 rc_thold; /**< rapid charge current threshold*/
};

struct pmu_ioctl_handler {
	pmu_subdev_ioctl_handler handler; /**< ioctl handler of the PMU sub device*/
	void *pri_data;	/**< private data to be passed to the ioctl handler*/
};

struct batt_adc_tbl {
	u16 *bat_adc;
	u16 *bat_vol;
	u32 num_entries;
};
struct bcm59035_power_pdata {
	struct charger_info usb; /**< structure holding trickle and rapid
	charging surrent details for USB*/
	struct charger_info wac; /**< structure holding trickle and rapid
	charging surrent details for WAC*/
	int temp_adc_channel; /**< ADC channel on which battery temperature
	can be measured*/
	u8 eoc_current;	/**< End oc charge current. Once the charging current
	reaches this value PMU generates EOC interrupt*/

	u16 temp_low_limit; /**< Lower limit of battery temperature below
	which charging is stopped.*/
	u16 temp_high_limit; /**< Higher limit of battery temperature above
	which charging is stopped.*/
	struct batt_adc_tbl batt_adc_tbl;
	u8 batt_technology; /**< Technology of the battery used.*/

};

struct bcm59035_led {
	const char *name;
	const char *default_trigger;
	int max_brightness;
	unsigned pwm_id;
	/*
	 * ID of the GPIO used to enable/disable LED
	 */
	int gpio;
	/*
	 * Value of the above GPIO in its active state (0 or 1)
	 */
	int gpio_active_on;
	/*
	 * flags for the led core (see include/linux/leds.h)
	 */
	int flags;
	/*
	 * channel initial state: enum bcm59035_pwm_mode
	 */
	int init_state;
};

struct bcm59035_led_pdata {
	/* start id of the bcm59035 pwm channels */
	int start_id;

	int num_leds;
	struct bcm59035_led *leds;
};

struct bcm59035_pwm_channel_data {
	int id;	/**< PWM channel ID*/

	int init_state;	/**< channel initial state: bcm59035_pwm_state*/

	int mode; /**< Mode of the PWM and LED*/

	int duty_ns; /**< Duty cycle of the PWM channel in ns*/
	int period_ns; /**< Period of the PWM channel in ns*/

	int pad_config;	/**< pwm channel pad configuration in the PMU*/
};

struct bcm59035_pwm_pdata {
	int start_id; /**< ID of the PWM channel */

	struct bcm59035_pwm_channel_data *data;	/**< structure holding mode,
	duty cycle and period of the pwm channel */
	int num_channels; /**< Number of PWM channels */
};

struct bcm59035_platform_data {
	struct i2c_slave_platform_data i2c_pdata; /**< Platform data for the
	PMU I2C slave */
	int flags; /**< Flags controlling the registration of PMU subdevice
	drivers. This also indicate the use of dedicated charger. This data is
	provided by the platform*/
	pmu_platform_callback pmu_event_cb; /**< Callback into the platform to
	do board specific initializations or detections.*/
	struct bcm59035_regl_pdata *regulators;	/**< Regulator specific data*/

	int gpio_base; /**< GPIO base for the PMU gpio*/

	struct bcm59035_power_pdata *power; /**< Platfrom data for power
	driver.This include the USB/WAC charger information. Maximum and
	minimum values of battery temperature and level. */

	u8 csr_nm_volt;	/**< CSR Voltage data : nm*/
	u8 csr_lpm_volt; /**< CSR Voltage data : lpm*/

	struct bcm59035_pwm_pdata *pwm;	/**< Platform data for the PWM driver. */

	struct bcm59035_led_pdata *led;	/**< Platform data for the LED driver. */
};

struct bcm59035 {
	int revision; /**< revision of PMU driver */
	int flags; /**< Flags controlling the registration of PMU subdevice
	drivers. This also indicate the use of dedicated charger.*/
	struct i2c_client *client; /**< i2c_client structure for the PMU i2c
	driver*/

	int (*write_dev) (struct bcm59035 * bcm59035, u32 reg, u8 val);/**<
	Write callback which allows writes to PMU registers */
	int (*read_dev) (struct bcm59035 * bcm59035, u32 reg, u8 * regVal);
									  /**<
	Read callback which allows reads to PMU registers */

	struct mutex list_lock;	/**< Mutex for pretecting PMU managed irq
	handler list */
	struct mutex lock; /**< lock used to avoid concurrent access which
	reading/writing PMU registers*/
	int irq; /**< irq number for PMU interrupts */
	struct list_head irq_handlers; /**< Head of the irq handlers list */
	struct work_struct work; /**< Work used to perform perodic
	monitoring of battery level and temperature and handling irqs*/
	struct workqueue_struct *pmu_workqueue;	/**< PMU private Work queue used to perform
	perodic monitoring of battery level and temperature and handling irqs*/
	struct pmu_ioctl_handler ioctl_handler[BCM59035_SUBDEV_MAX]; /**<
	Array of PMU subdevice irq handlers*/

	struct bcm59035_platform_data *pdata; /**< Platform data for the PMU*/
};

#endif /* __BCM59035_PRIVATE_H__ */
