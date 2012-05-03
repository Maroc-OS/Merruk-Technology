/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/mfd/bcm59038/bcm59038-private.h
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
*   @file   bcm59038-private.h
*
*   @brief  Contains definitions common to all BCM PMU chips
*
****************************************************************************/

#ifndef __BCM59038_PRIVATE_H__
#define __BCM59038_PRIVATE_H__

#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/regulator/machine.h>

#define BCM59038_LOG_ENABLE 1

#ifdef BCM59038_LOG_ENABLE
extern const u32 bcm59038_debug_mask;
#define PMU_LOG(level, fmt, args...) \
	do {\
		if (level & bcm59038_debug_mask)\
			printk("BCM59038: "fmt, ##args);\
	} while (0)
#else
#define PMU_LOG(level, fmt, args...)
#endif /*BCM59038_LOG_ENABLE */
#include <plat/bcm_i2c.h>

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

#define BCM_PMU_IOCTL_ENABLE_INTS       	_IO(BCM_PMU_MAGIC, BCM_PMU_CMD_ENABLE_INTS)	/* arg is unused */
#define BCM_PMU_IOCTL_DISABLE_INTS      	_IO(BCM_PMU_MAGIC, BCM_PMU_CMD_DISABLE_INTS)	/* arg is unused */
#define BCM_PMU_IOCTL_READ_REG          	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_READ_REG, pmu_reg)
#define BCM_PMU_IOCTL_WRITE_REG         	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_WRITE_REG, pmu_reg)
#define BCM_PMU_IOCTL_ACTIVATESIM       	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_ACTIVATESIM, pmu_sim_volt)
#define BCM_PMU_IOCTL_DEACTIVATESIM     	_IO(BCM_PMU_MAGIC, BCM_PMU_CMD_DEACTIVATESIM)	/* arg is unused */
#define BCM_PMU_IOCTL_GET_REGULATOR_STATE 	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_REGULATOR_STATE, pmu_regl)
#define BCM_PMU_IOCTL_SET_REGULATOR_STATE 	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_REGULATOR_STATE, pmu_regl)
#define BCM_PMU_IOCTL_GET_VOLTAGE        	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_VOLTAGE, pmu_regl_volt)
#define BCM_PMU_IOCTL_SET_VOLTAGE        	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_VOLTAGE, pmu_regl_volt)
#define BCM_PMU_IOCTL_SET_PWM_LED_CTRL    	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_LED_CTRL, pmu_pwm_ctrl)
#define BCM_PMU_IOCTL_POWERONOFF          	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_POWERONOFF, pmu_reg)
#define BCM_PMU_IOCTL_SET_PWM_HI_PER      	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_HI_PER, int)
#define BCM_PMU_IOCTL_SET_PWM_LO_PER      	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_LO_PER, int)
#define BCM_PMU_IOCTL_SET_PWM_PWR_CTRL    	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_PWR_CTRL, int)
#define BCM_PMU_IOCTL_GET_CHARGING_CURRENT    	_IOR(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_CHARGING_CUR, int)
#define BCM_PMU_IOCTL_SET_CHARGING_CURRENT    	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_CHARGING_CUR, int)
#define BCM_PMU_IOCTL_START_CHARGING    	_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_START_CHARGING, int)
#define BCM_PMU_IOCTL_STOP_CHARGING   	 	_IO(BCM_PMU_MAGIC, BCM_PMU_CMD_STOP_CHARGING)

enum {
	PMU_EVENT_INIT_PLATFORM,
	PMU_EVENT_CHARGER_INSERT,
	PMU_EVENT_CHARGER_REMOVE,
	PMU_EVENT_BATT_TEMP_BEYOND_LIMIT,
	/*used in case of dedicated charger support to get the charger type */
	PMU_EVENT_GET_CHARGER_TYPE,
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

enum {
	DEBUG_PMU_INFO = 1U << 0,
	DEBUG_PMU_WARNING = 1U << 1,
	DEBUG_PMU_ERROR = 1U << 2,
};

enum {
	BCM59038_USE_REGULATORS = (1 << 0),
	BCM59038_USE_RTC = (1 << 1),
	BCM59038_USE_PONKEY = (1 << 2),
	BCM59038_USE_DVS = (1 << 3),
	BCM59038_USE_POWER = (1 << 4),
	BCM59038_USE_WATCHDOG = (1 << 5),
	BCM59038_USE_SARADC = (1 << 6),
	BCM59038_REGISTER_POWER_OFF = (1 << 7),
	BCM59038_USE_AUDIO = (1 << 8)
};

/*BCM59038 mfd sub device ids*/
enum {
	BCM59038_SUBDEV_POWER,
	BCM59038_SUBDEV_REGULATOR,
	BCM59038_SUBDEV_MAX
};

/* Platform Data for Power Driver */

struct charger_info {
	u8 fc_cc;
	u8 qc_cc;
	u8 fc1_cv;
	u8 fc2_cv;
};

struct batt_level_table {
	u16 adc_value;
	u16 batt_volt;		/* in mV */
};

struct bcm59038_power_pdata {
	struct charger_info usb;
	struct charger_info wac;
	u8 eoc_current;

	u8 volt_adc_channel;
	u8 temp_adc_channel;
	u8 batt_level_count;
	struct batt_level_table *batt_level_table;

	u16 temp_low_limit;
	u16 temp_high_limit;

	u16 batt_min_volt;
	u16 batt_max_volt;

	u8 batt_technology;

#ifdef BCM59035_POWER_ENABLE_USB_HACK
	void (*usb_start) (void);
	void (*usb_stop) (void);
#endif				/*BCM59038_POWER_ENABLE_USB_HACK */
};

struct bcm_pmu_irq {
	struct list_head node;
	void (*handler) (int, void *);
	void *data;
	int irq;
	bool irq_enabled;
};

struct bcm59038_regl_init_data {
	int regl_id;
	struct regulator_init_data *init_data;

};

struct bcm59038_regl_pdata {
	int num_regulators;
	struct bcm59038_regl_init_data *regl_init;
};

typedef int (*pmu_subdev_ioctl_handler) (u32 cmd, u32 arg, void *pri_data);
struct pmu_ioctl_handler {
	pmu_subdev_ioctl_handler handler;
	void *pri_data;
};

struct bcm59038_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	int flags;
	pmu_platform_callback pmu_event_cb;

	/* Regulator specific data */
	struct bcm59038_regl_pdata *regulators;
	struct bcm59038_power_pdata *power;
};

struct bcm59038 {
	u8 pmu_id;
	int flags;
	u8 page;		/* active page number */

	struct i2c_client *client;

	int (*write_dev) (struct bcm59038 *bcm59038, u32 reg, u8 val);
	int (*read_dev) (struct bcm59038 *bcm59038, u32 reg, u8 *regVal);

	struct mutex list_lock;
	struct mutex lock;
	int irq;
	struct list_head irq_handlers;
	struct work_struct work;
	struct pmu_ioctl_handler ioctl_handler[BCM59038_SUBDEV_MAX];
	struct bcm59038_platform_data *pdata;
};

#endif /* __BCM59038_PRIVATE_H__ */
