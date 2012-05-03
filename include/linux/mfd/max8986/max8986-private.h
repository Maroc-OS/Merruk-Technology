/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/mfd/max8986/max8986-private.h
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
*   @file   max8986-private.h
*
*   @brief  Contains definitions common to all Maxim PMU chips
*
****************************************************************************/

#ifndef __MAX8986_PRIVATE_H__
#define __MAX8986_PRIVATE_H__


#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/regulator/machine.h>
#include <plat/bcm_i2c.h>
#include <linux/broadcom/pmu_chip.h>

/* MAX8986 Regulator IDs */
enum {
	MAX8986_REGL_ALDO1 = 0,
	MAX8986_REGL_ALDO2,
	MAX8986_REGL_ALDO3,
	MAX8986_REGL_ALDO4,
	MAX8986_REGL_ALDO5,
	MAX8986_REGL_ALDO6,
	MAX8986_REGL_ALDO7,
	MAX8986_REGL_ALDO8,
	MAX8986_REGL_ALDO9,

	MAX8986_REGL_DLDO1,
	MAX8986_REGL_DLDO2,
	MAX8986_REGL_DLDO3,
	MAX8986_REGL_DLDO4,

	MAX8986_REGL_HCLDO1,
	MAX8986_REGL_HCLDO2,

	MAX8986_REGL_LVLDO,

	MAX8986_REGL_SIMLDO,

	MAX8986_REGL_AUXLDO1,

	MAX8986_REGL_TSRLDO,

	/*CSR is registered as 3 regulators to control LPM, NM1 & NM2 voltages
	USE MAX8986_REGL_CSR_LPM TO SET CSR OP MODE
	*/
	MAX8986_REGL_CSR_NM1,
	MAX8986_REGL_CSR_NM2,
	MAX8986_REGL_CSR_LPM,
	MAX8986_REGL_IOSR,

	MAX8986_REGL_NUM_REGULATOR
};

enum muic_adc_output {
	PMU_MUIC_ADC_OUTPUT_GND,
	PMU_MUIC_ADC_OUTPUT_2K,
	PMU_MUIC_ADC_OUTPUT_2P6K,
	PMU_MUIC_ADC_OUTPUT_3K,
	PMU_MUIC_ADC_OUTPUT_4K,
	PMU_MUIC_ADC_OUTPUT_4P8K,
	PMU_MUIC_ADC_OUTPUT_6K,
	PMU_MUIC_ADC_OUTPUT_8K,
	PMU_MUIC_ADC_OUTPUT_10K,
	PMU_MUIC_ADC_OUTPUT_12K,
	PMU_MUIC_ADC_OUTPUT_14K,
	PMU_MUIC_ADC_OUTPUT_17K,
	PMU_MUIC_ADC_OUTPUT_20K,
	PMU_MUIC_ADC_OUTPUT_24K,
	PMU_MUIC_ADC_OUTPUT_28K,
	PMU_MUIC_ADC_OUTPUT_34K,
	PMU_MUIC_ADC_OUTPUT_40K,
	PMU_MUIC_ADC_OUTPUT_49K,
	PMU_MUIC_ADC_OUTPUT_64K,
	PMU_MUIC_ADC_OUTPUT_80K,
	PMU_MUIC_ADC_OUTPUT_102K,
	PMU_MUIC_ADC_OUTPUT_121K,
	PMU_MUIC_ADC_OUTPUT_150K,
	PMU_MUIC_ADC_OUTPUT_200K,
	PMU_MUIC_ADC_OUTPUT_255K,
	PMU_MUIC_ADC_OUTPUT_301K,
	PMU_MUIC_ADC_OUTPUT_365K,
	PMU_MUIC_ADC_OUTPUT_442K,
	PMU_MUIC_ADC_OUTPUT_523K,
	PMU_MUIC_ADC_OUTPUT_619K,
	PMU_MUIC_ADC_OUTPUT_1M,
	PMU_MUIC_ADC_OUTPUT_OPEN,

	PMU_MUIC_ADC_OUTPUT_INIT
};

/* used to communicate events between the mfd core, power driver and the
 * platform code
 */
enum {
	PMU_EVENT_INIT_PLATFORM,
	PMU_EVENT_CHARGER_INSERT,
	PMU_EVENT_CHARGER_REMOVE,
	PMU_EVENT_BATT_TEMP_BEYOND_LIMIT,
	/*used in case of dedicated charger support to get the charger type*/
	PMU_EVENT_GET_CHARGER_TYPE,
	/*used to convert temp adc to degree celsius */
	PMU_EVENT_BATT_TEMP_TO_DEGREE_C,
	/*used to convert batt voltage ADC to voltage */
	PMU_EVENT_BATT_ADC_TO_VOLTAGE,

};

typedef u32 (*pmu_platform_callback)(int event, int param);


typedef int(*pmu_subdev_ioctl_handler)(u32 cmd, u32 arg, void *pri_data);

enum {
	DEBUG_PMU_INFO = 1U << 0,
	DEBUG_PMU_WARNING = 1U << 1,
	DEBUG_PMU_ERROR = 1U << 2,
};

enum {
	MAX8986_USE_REGULATORS   			=  (1 << 0),
	MAX8986_USE_RTC          			=  (1 << 1),
	MAX8986_USE_POWER        			=  (1 << 2),
	MAX8986_USE_PONKEY 				=  (1 << 3),
	MAX8986_ENABLE_DVS       			=  (1 << 4),
	MAX8986_REGISTER_POWER_OFF			=  (1 << 5),
	MAX8986_ENABLE_AUDIO				=  (1 << 6),
	MAX8986_ENABLE_MUIC					=  (1 << 8),
};

/*regualtor DSM settings */
enum {
	MAX8986_REGL_LPM_IN_DSM,   /*if enabled, LPM in DSM (PC1 = 0)*/
	MAX8986_REGL_OFF_IN_DSM, /*if enabled, off in DSM (PC1 = 0)*/
	MAX8986_REGL_ON_IN_DSM ,   /*if enabled, ON in DSM (PC1 = 0)*/
};


/*MAX8986 mfd sub device ids*/
enum {
	MAX8986_SUBDEV_POWER,
	MAX8986_SUBDEV_REGULATOR,
	MAX8986_SUBDEV_MAX
};

/* MAX8986 slave IDs */
enum {
	MAX8986_SLAVE_CORE = 0,
	MAX8986_SLAVE_RTC,
	MAX8986_SLAVE_MUIC,
	MAX8986_SLAVE_AUDIO,
	MAX8986_NUM_SLAVES
};

struct max_pmu_irq {
	struct list_head node;
	void (*handler)(int, void *);
	void *data;
	int irq;
	bool irq_enabled;
};

struct max8986_regl_init_data {
	int regl_id;
	u8 dsm_opmode;
	struct regulator_init_data *init_data;

};

struct max8986_muic_pdata
{
	u16 mic_insert_adc_val;
	u16 mic_btn_press_adc_val;
};


struct max8986_regl_pdata {
	int num_regulators;
	struct max8986_regl_init_data *regl_init;
	/*Regulator PM mode*/
	u8 regl_default_pmmode[MAX8986_REGL_NUM_REGULATOR];
};

struct charger_info {
	u8 charging_cc;
	u8 charging_cv;
};

struct pmu_ioctl_handler {
	pmu_subdev_ioctl_handler handler;
	void *pri_data;
};


struct batt_adc_tbl {
	u16 *bat_adc;
	u16 *bat_vol;
	u32 num_entries;
};

struct max8986_power_pdata
{
	u8 usb_charging_cc;
	u8 wac_charging_cc;
	u8 eoc_current;

	int temp_adc_channel;
	u16 temp_low_limit;
	u16 temp_high_limit;

	struct batt_adc_tbl batt_adc_tbl;
	u8 batt_technology;
};

struct max8986_audio_pdata
{
	u8 ina_def_mode;
	u8 inb_def_mode;
	u8 ina_def_preampgain;
	u8 inb_def_preampgain;

	u8 lhs_def_mixer_in;
	u8 rhs_def_mixer_in;
	u8 ihf_def_mixer_in;

	int hs_input_path;
	int ihf_input_path;
};

struct max8986_client {
	struct i2c_client *client;
	u16 addr;
};

struct max8986;
struct max8986_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	int flags;
	pmu_platform_callback pmu_event_cb;
	/* Regulator specific data */
	struct max8986_regl_pdata *regulators;

	/*audio default settings*/
	struct max8986_audio_pdata* audio_pdata;

	/*power data*/
	struct max8986_power_pdata *power;

	/*MUIC data*/
	struct max8986_muic_pdata *muic_pdata;

	/* CSR Voltage data */
	s8 csr_nm_volt;
	s8 csr_nm2_volt;
	s8 csr_lpm_volt;

	/* Initialize maxim pmu registers from sysparms */
	void (*max8986_sysparms)(struct max8986 *max8986);
};

struct max8986 {
	int revision;
	int flags;
	struct max8986_client max8986_cl[MAX8986_NUM_SLAVES];
	/* Single byte write */
	int (*write_dev) (struct max8986 *max8986, u32 command, u8 val);
	/* Single byte read */
	int (*read_dev) (struct max8986 *max8986, u32 command, u8 *regVal);

	int (*write_mul_dev) (struct max8986 *max8986, u32 command,
						u32 length, u8 *val);
	int (*read_mul_dev) (struct max8986 *max8986, u32 command,
						u32 length, u8 *val);

	struct mutex list_lock;
	struct mutex i2c_rw_lock;
	struct mutex muic_int_lock;
	int muic_int_enable;

	int irq;
	struct list_head irq_handlers;
	struct work_struct work;
	struct workqueue_struct *pmu_workqueue;
	struct pmu_ioctl_handler ioctl_handler[MAX8986_SUBDEV_MAX];

	struct max8986_platform_data *pdata;
};

#endif /* __MAX8986_PRIVATE_H__ */
