/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/pmu_chip.h
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

/*
*
*****************************************************************************
*
*  pmu_chip.h
*
*  PURPOSE:
*
*  This file defines the common AT ccomamnd interface to PMU chip
*
*  NOTES:
*
*****************************************************************************/

#if !defined(BCM_PMU_CHIP_H)
#define BCM_PMU_CHIP_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/ioctl.h>
#include <linux/fs.h>

/* ---- Constants and Types ---------------------------------------------- */

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

#define BCM_PMU_IOCTL_ENABLE_INTS   \
		_IO(BCM_PMU_MAGIC, BCM_PMU_CMD_ENABLE_INTS)
#define BCM_PMU_IOCTL_DISABLE_INTS  \
		_IO(BCM_PMU_MAGIC, BCM_PMU_CMD_DISABLE_INTS)
#define BCM_PMU_IOCTL_READ_REG      \
		_IOWR(BCM_PMU_MAGIC, BCM_PMU_CMD_READ_REG, pmu_reg)
#define BCM_PMU_IOCTL_WRITE_REG     \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_WRITE_REG, pmu_reg)
#define BCM_PMU_IOCTL_ACTIVATESIM   \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_ACTIVATESIM, pmu_sim_volt)
#define BCM_PMU_IOCTL_DEACTIVATESIM \
		_IO(BCM_PMU_MAGIC, BCM_PMU_CMD_DEACTIVATESIM)
#define BCM_PMU_IOCTL_GET_REGULATOR_STATE \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_REGULATOR_STATE, pmu_regl)
#define BCM_PMU_IOCTL_SET_REGULATOR_STATE \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_REGULATOR_STATE, pmu_regl)
#define BCM_PMU_IOCTL_GET_VOLTAGE   \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_VOLTAGE, pmu_regl_volt)
#define BCM_PMU_IOCTL_SET_VOLTAGE   \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_VOLTAGE, pmu_regl_volt)
#define BCM_PMU_IOCTL_SET_PWM_LED_CTRL \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_LED_CTRL, pmu_pwm_ctrl)
#define BCM_PMU_IOCTL_POWERONOFF    \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_POWERONOFF, pmu_reg)
#define BCM_PMU_IOCTL_SET_PWM_HI_PER \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_HI_PER, int)
#define BCM_PMU_IOCTL_SET_PWM_LO_PER \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_LO_PER, int)
#define BCM_PMU_IOCTL_SET_PWM_PWR_CTRL \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_PWR_CTRL, int)
#define BCM_PMU_IOCTL_GET_CHARGING_CURRENT \
		_IOR(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_CHARGING_CUR, int)
#define BCM_PMU_IOCTL_SET_CHARGING_CURRENT \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_CHARGING_CUR, int)
#define BCM_PMU_IOCTL_START_CHARGING \
		_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_START_CHARGING, int)
#define BCM_PMU_IOCTL_STOP_CHARGING \
		_IO(BCM_PMU_MAGIC, BCM_PMU_CMD_STOP_CHARGING)

typedef struct {
	unsigned long  reg;
	unsigned short  val;
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
	PMU_REGL_ECO, /*0x01 */
	PMU_REGL_OFF, /*0x02 */
	PMU_REGL_TURBO,/*0x03*/
	PMU_REGL_MASK = 0x3
} pmu_regl_state;

typedef struct {
	int regl_id;
	pmu_regl_state state;
} pmu_regl;

typedef struct {
	unsigned long regoffset;
	unsigned long pwmled_ctrl ;
	unsigned long pwmdiv ; /* divider value. fsys/x value. */
} pmu_pwm_ctrl;

typedef struct {
	int regl_id;
	int voltage;
	int min;
	int max;
	int step;
} pmu_regl_volt;

#endif /* BCM_PMU_CHIP_H */
