/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/regulator/max8986-regulator.c
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
*   @file   max8986-regulator.c
*
*   @brief  Regulator Driver for Broadcom MAX8986 PMU
*
****************************************************************************/

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/mfd/max8986/max8986.h>
#include <asm/io.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/driver.h>


#define MICRO_VOLT(x)	((x)*1000L*1000L)
#define _DEFINE_REGL_VOLT(vol, regVal)	{ MICRO_VOLT(vol), regVal }


#define MAX8986_DEFINE_REGL(_name, _pm_reg, _ctrl_reg, _vol_list)\
	[MAX8986_REGL_##_name] =\
			{\
				.desc = {\
					.name = "MAX8986_REGL_"#_name,\
					.ops = &max8986_regulator_ops,\
					.type = REGULATOR_VOLTAGE,\
					.id = MAX8986_REGL_##_name,\
					.owner = THIS_MODULE,\
				},\
				.ctrl_reg = _ctrl_reg,\
				.pm_reg = _pm_reg,\
				.num_vol = ARRAY_SIZE(_vol_list),\
				.vol_list = _vol_list,\
			}

enum {
	PC2PC1_00 = 0,
	PC2PC1_01 = 2,
	PC2PC1_10 = 4,
	PC2PC1_11 = 6
};

struct max8986_regl {
	struct regulator_desc desc;
	u32 ctrl_reg;
	u32 pm_reg;
	u8 num_vol;
	u8 dsm_opmode;
	int const (*vol_list)[2];
};


struct max8986_regl_priv {
	struct max8986 *max8986;
	int num_regl;
	struct regulator_dev *regl[];
};

static const int aldo1_vol[][2] = {
	_DEFINE_REGL_VOLT(3.0, 7), 	/*111: 3.0V */
};

static const int aldo2_vol[][2] = {
	_DEFINE_REGL_VOLT(2.5, 0), 	/*000: 2.5V */
	_DEFINE_REGL_VOLT(2.6, 1), 	/*001: 2.6V */
	_DEFINE_REGL_VOLT(2.7, 2), 	/*010: 2.7V */
	_DEFINE_REGL_VOLT(2.8, 3), 	/*011: 2.8V */
};

static const int aldo3_vol[][2] = {
	_DEFINE_REGL_VOLT(3.3, 7), 		/*111: 3.3V */
};

static const int aldo4_vol[][2] = {
	_DEFINE_REGL_VOLT(2.8, 0x10), 	/*10 000: 2.8V */
};

static const int aldo5_vol[][2] = {
	_DEFINE_REGL_VOLT(2.8, 0x1F), 	/*11 111: 2.8V */
};

static const int aldo6_vol[][2] = {
	_DEFINE_REGL_VOLT(1.2, 0),		/*0000: 1.2V */
	_DEFINE_REGL_VOLT(1.4, 7),		/*0111: 1.4V */
	_DEFINE_REGL_VOLT(1.5, 8),		/*1001: 1.5V */
	_DEFINE_REGL_VOLT(1.8, 9),		/*1001: 1.8V */
	_DEFINE_REGL_VOLT(2.7, 0xA),		/*1010: 2.7V */
	_DEFINE_REGL_VOLT(2.8, 0xB),		/*1011: 2.8V */
	_DEFINE_REGL_VOLT(2.9, 0xC),		/*1100: 2.9V */
	_DEFINE_REGL_VOLT(3.0, 0xD),		/*1101: 3.0V */
	_DEFINE_REGL_VOLT(3.3, 0xE),		/*1110: 3.3V */
	_DEFINE_REGL_VOLT(1.3, 0xF),		/*1111: 1.3V */
};


/* aldo9, dldo3, dldo4 and auxldo1 are having same values like aldo7_vol
 * Becareful while changing aldo7_vol values
*/
static const int aldo7_vol[][2] = {
	_DEFINE_REGL_VOLT(1.2, 0),		/*00000: 1.2V */
	_DEFINE_REGL_VOLT(1.3, 1),		/*00001: 1.3V */
	_DEFINE_REGL_VOLT(1.4, 2),		/*00010: 1.4V */
	_DEFINE_REGL_VOLT(1.5, 3),		/*00011: 1.5V */
	_DEFINE_REGL_VOLT(1.6, 4),		/*00100: 1.6V */
	_DEFINE_REGL_VOLT(1.7, 5),		/*00101: 1.7V */
	_DEFINE_REGL_VOLT(1.8, 6),		/*00110: 1.8V */
	_DEFINE_REGL_VOLT(1.9, 7),		/*00111: 1.9V */
	_DEFINE_REGL_VOLT(2.0, 8),		/*01000: 2.0V */
	_DEFINE_REGL_VOLT(2.1, 9),		/*01001: 2.1V */
	_DEFINE_REGL_VOLT(2.2, 0xA),		/*01010: 2.2V */
	_DEFINE_REGL_VOLT(2.3, 0xB),		/*01011: 2.3V */
	_DEFINE_REGL_VOLT(2.4, 0xC),		/*01100: 2.4V */
	_DEFINE_REGL_VOLT(2.5, 0xD),		/*01101: 2.5V */
	_DEFINE_REGL_VOLT(2.6, 0xE),		/*01110: 2.6V */
	_DEFINE_REGL_VOLT(2.7, 0xF),		/*01111: 2.7V */
	_DEFINE_REGL_VOLT(2.8, 0x10),		/*10000: 2.8V */
	_DEFINE_REGL_VOLT(2.9, 0x11),		/*10001: 2.9V */
	_DEFINE_REGL_VOLT(3.0, 0x12),		/*10010: 3.0V */
	_DEFINE_REGL_VOLT(3.1, 0x13),		/*10011: 3.1V */
	_DEFINE_REGL_VOLT(3.2, 0x14),		/*10100: 3.2V */
	_DEFINE_REGL_VOLT(3.3, 0x15),		/*10101: 3.3V */
};

static const int aldo8_vol[][2] = {
	_DEFINE_REGL_VOLT(2.5, 0),		/*0000: 2.5V */
	_DEFINE_REGL_VOLT(2.6, 1),		/*0001: 2.6V */
	_DEFINE_REGL_VOLT(2.8, 2),		/*0010: 2.8V */
	_DEFINE_REGL_VOLT(2.9, 3),		/*0011: 2.9V */
	_DEFINE_REGL_VOLT(3.0, 4),		/*0100: 3.0V */
	_DEFINE_REGL_VOLT(1.8, 5),		/*0101: 1.8V */
	_DEFINE_REGL_VOLT(3.3, 6),		/*0110: 3.3V */
	_DEFINE_REGL_VOLT(2.7, 7),		/*0111: 2.7V */
};

/* aldo9 & aldo7 have same voltage values */
#define aldo9_vol aldo7_vol


/* hcldo1_vol and hcldo2_vol are having same values like dldo1_vol
 * Becareful while changing dldo1_vol values
*/
static const int dldo1_vol[][2] = {
	_DEFINE_REGL_VOLT(2.5, 0),		/*00000: 2.5V */
	_DEFINE_REGL_VOLT(2.6, 1),		/*00001: 2.6V */
	_DEFINE_REGL_VOLT(2.7, 2),		/*00010: 2.7V */
	_DEFINE_REGL_VOLT(2.8, 3),		/*00011: 2.8V */
	_DEFINE_REGL_VOLT(2.9, 4),		/*00100: 2.9V */
	_DEFINE_REGL_VOLT(3.3, 5),		/*00101: 3.3V */
	_DEFINE_REGL_VOLT(3.1, 6),		/*00110: 3.1V */
	_DEFINE_REGL_VOLT(3.0, 7),		/*00111: 3.0V */
};

static const int dldo2_vol[][2] = {
	_DEFINE_REGL_VOLT(1.8, 6),		/*00110: 1.8V */
};

/* dldo3 & aldo7 have same voltage values */
#define dldo3_vol aldo7_vol

/* dldo4 & aldo7 have same voltage values */
#define dldo4_vol aldo7_vol

/* hcldo1 & dldo1 have same voltage values */
#define hcldo1_vol dldo1_vol

/* hcldo2 & dldo1 have same voltage values */
#define hcldo2_vol dldo1_vol

static const int lvldo_vol[][2] = {
	_DEFINE_REGL_VOLT(1.2, 0),		/*00000: 1.2V */
};

static const int simldo_vol[][2] = {
	_DEFINE_REGL_VOLT(3.0, 0),		/*00: 3.0V */
	_DEFINE_REGL_VOLT(2.5, 1),		/*01: 2.5V */
	_DEFINE_REGL_VOLT(3.1, 2),		/*10: 3.1V */
	_DEFINE_REGL_VOLT(1.8, 3),		/*11: 1.8V */
};

/* auxldo1 & aldo7 have same voltage values */
#define auxldo1_vol aldo7_vol

static const int tsrldo_vol[][2] = {
	_DEFINE_REGL_VOLT(0.7, 0),		/*00000: 0.7V */
	_DEFINE_REGL_VOLT(0.8, 1),		/*00001: 0.8V */
	_DEFINE_REGL_VOLT(0.9, 2),		/*00010: 0.9V */
	_DEFINE_REGL_VOLT(1.0, 3),		/*00011: 1.0V */
	_DEFINE_REGL_VOLT(1.1, 4),		/*00100: 1.1V */
	_DEFINE_REGL_VOLT(1.2, 5),		/*00101: 1.2V */
	_DEFINE_REGL_VOLT(1.3, 6),		/*00110: 1.3V */
	_DEFINE_REGL_VOLT(1.4, 7),		/*00111: 1.4V */
	_DEFINE_REGL_VOLT(1.5, 8),		/*01000: 1.5V */
	_DEFINE_REGL_VOLT(1.6, 9),		/*01001: 1.6V */
	_DEFINE_REGL_VOLT(1.7, 0xA),		/*01010: 1.7V */
	_DEFINE_REGL_VOLT(1.8, 0xB),		/*01011: 1.8V */
	_DEFINE_REGL_VOLT(1.9, 0xC),		/*01100: 1.9V */
	_DEFINE_REGL_VOLT(2.0, 0xD),		/*01101: 2.0V */
	_DEFINE_REGL_VOLT(2.1, 0xE),		/*01110: 2.1V */
	_DEFINE_REGL_VOLT(2.2, 0xF),		/*01111: 2.2V */
	_DEFINE_REGL_VOLT(2.3, 0x10),		/*10000: 2.3V */
	_DEFINE_REGL_VOLT(2.4, 0x11),		/*10001: 2.4V */
	_DEFINE_REGL_VOLT(2.5, 0x12),		/*10010: 2.5V */
	_DEFINE_REGL_VOLT(2.6, 0x13),		/*10011: 2.6V */
	_DEFINE_REGL_VOLT(2.7, 0x14),		/*10100: 2.7V */
	_DEFINE_REGL_VOLT(2.8, 0x15),		/*10101: 2.8V */
	_DEFINE_REGL_VOLT(2.9, 0x16),		/*10110: 2.9V */
};

static const int csr_dvs_vol[][2] = {
	_DEFINE_REGL_VOLT(0.76, 0),
	_DEFINE_REGL_VOLT(0.78, 1),
	_DEFINE_REGL_VOLT(0.80, 2),
	_DEFINE_REGL_VOLT(0.82, 3),
	_DEFINE_REGL_VOLT(0.84, 4),
	_DEFINE_REGL_VOLT(0.86, 5),
	_DEFINE_REGL_VOLT(1.38, 6),
	_DEFINE_REGL_VOLT(1.36, 7),
	_DEFINE_REGL_VOLT(1.34, 8),
	_DEFINE_REGL_VOLT(1.32, 9),
	_DEFINE_REGL_VOLT(1.30, 0xA),
	_DEFINE_REGL_VOLT(1.28, 0xB),
	_DEFINE_REGL_VOLT(1.26, 0xC),
	_DEFINE_REGL_VOLT(1.24, 0xD),
	_DEFINE_REGL_VOLT(1.22, 0xE),
	_DEFINE_REGL_VOLT(1.20, 0xF),
	_DEFINE_REGL_VOLT(1.18, 0x10),
	_DEFINE_REGL_VOLT(1.16, 0x11),
	_DEFINE_REGL_VOLT(1.14, 0x12),
	_DEFINE_REGL_VOLT(1.12, 0x13),
	_DEFINE_REGL_VOLT(1.10, 0x14),
	_DEFINE_REGL_VOLT(1.08, 0x15),
	_DEFINE_REGL_VOLT(1.06, 0x16),
	_DEFINE_REGL_VOLT(1.04, 0x17),
	_DEFINE_REGL_VOLT(1.02, 0x18),
	_DEFINE_REGL_VOLT(1.00, 0x19),
	_DEFINE_REGL_VOLT(0.98, 0x1A),
	_DEFINE_REGL_VOLT(0.96, 0x1B),
	_DEFINE_REGL_VOLT(0.94, 0x1C),
	_DEFINE_REGL_VOLT(0.92, 0x1D),
	_DEFINE_REGL_VOLT(0.90, 0x1E),
	_DEFINE_REGL_VOLT(0.88, 0x1F)
};


static const int iosr_vol[][2] = {
	_DEFINE_REGL_VOLT(1.8, 7),		/*00111: 1.8V */
};


static struct regulator_ops max8986_regulator_ops;

static struct max8986_regl max8986_regls[] = {

	MAX8986_DEFINE_REGL(ALDO1, MAX8986_PM_REG_A1OPMODCTRL,
				MAX8986_PM_REG_A1_D1_LDOCTRL, aldo1_vol),
	MAX8986_DEFINE_REGL(ALDO2, MAX8986_PM_REG_A2OPMODCTRL,
				MAX8986_PM_REG_A8_A2_LDOCTRL, aldo2_vol),
	MAX8986_DEFINE_REGL(ALDO3, MAX8986_PM_REG_A3OPMODCTRL,
				MAX8986_PM_REG_A5_A3_LDOCTRL, aldo3_vol),
	MAX8986_DEFINE_REGL(ALDO4, MAX8986_PM_REG_A4OPMODCTRL,
				MAX8986_PM_REG_A4_SIM_LDOCTRL, aldo4_vol),
	MAX8986_DEFINE_REGL(ALDO5, MAX8986_PM_REG_A5OPMODCTRL,
				MAX8986_PM_REG_A5_A3_LDOCTRL, aldo5_vol),
	MAX8986_DEFINE_REGL(ALDO6, MAX8986_PM_REG_A6OPMODCTRL,
				MAX8986_PM_REG_LV_A6_LDOCTRL, aldo6_vol),
	MAX8986_DEFINE_REGL(ALDO7, MAX8986_PM_REG_A7OPMODCTRL,
				MAX8986_PM_REG_A7LDOCTRL, aldo7_vol),
	MAX8986_DEFINE_REGL(ALDO8, MAX8986_PM_REG_A8OPMODCTRL,
				MAX8986_PM_REG_A8_A2_LDOCTRL, aldo8_vol),
	MAX8986_DEFINE_REGL(ALDO9, MAX8986_PM_REG_A9OPMODCTRL,
				MAX8986_PM_REG_A9LDOCTRL, aldo9_vol),

	MAX8986_DEFINE_REGL(DLDO1, MAX8986_PM_REG_D1OPMODCTRL,
				MAX8986_PM_REG_A1_D1_LDOCTRL, dldo1_vol),
	MAX8986_DEFINE_REGL(DLDO2, MAX8986_PM_REG_D2OPMODCTRL,
				MAX8986_PM_REG_D2LDOCTRL, dldo2_vol),
	MAX8986_DEFINE_REGL(DLDO3, MAX8986_PM_REG_D3OPMODCTRL,
				MAX8986_PM_REG_D3LDOCTRL, dldo3_vol),
	MAX8986_DEFINE_REGL(DLDO4, MAX8986_PM_REG_D4OPMODCTRL,
				MAX8986_PM_REG_D4LDOCTRL, dldo4_vol),

	MAX8986_DEFINE_REGL(HCLDO1, MAX8986_PM_REG_H1OPMODCTRL,
				MAX8986_PM_REG_HCLDOCTRL, hcldo1_vol),
	MAX8986_DEFINE_REGL(HCLDO2, MAX8986_PM_REG_H2OPMODCTRL,
				MAX8986_PM_REG_HCLDOCTRL, hcldo2_vol),

	MAX8986_DEFINE_REGL(LVLDO, MAX8986_PM_REG_LVOPMODCTRL,
			MAX8986_PM_REG_LV_A6_LDOCTRL, lvldo_vol),

	MAX8986_DEFINE_REGL(SIMLDO, MAX8986_PM_REG_SIMOPMODCTRL,
			MAX8986_PM_REG_A4_SIM_LDOCTRL, simldo_vol),

	MAX8986_DEFINE_REGL(AUXLDO1, MAX8986_PM_REG_AX1OPMODCTRL,
			MAX8986_PM_REG_AX1LDOCTRL, auxldo1_vol),

	MAX8986_DEFINE_REGL(TSRLDO, MAX8986_PM_REG_TSROPMODCTRL,
			MAX8986_PM_REG_TSRCTRL, tsrldo_vol),

	/*CSR is registered as 3 regulators to control LPM, NM1 & NM2 voltages */
	MAX8986_DEFINE_REGL(CSR_NM1, MAX8986_PM_REG_CSROPMODCTRL,
			MAX8986_PM_REG_CSRCTRL10, csr_dvs_vol),
	MAX8986_DEFINE_REGL(CSR_NM2, MAX8986_PM_REG_CSROPMODCTRL,
			MAX8986_PM_REG_CSRCTRL3, csr_dvs_vol),
	MAX8986_DEFINE_REGL(CSR_LPM, MAX8986_PM_REG_CSROPMODCTRL,
			MAX8986_PM_REG_CSRCTRL1, csr_dvs_vol),

	MAX8986_DEFINE_REGL(IOSR, MAX8986_PM_REG_IOSROPMODCTRL,
			MAX8986_PM_REG_IOSRCTRL2, iosr_vol),
};

int max8986_csr_reg2volt(int reg)
{
	int volt = -EINVAL;
	int i;

	for (i = 0; i < ARRAY_SIZE(csr_dvs_vol); i++) {
		if (csr_dvs_vol[i][1] == reg) {
			volt = csr_dvs_vol[i][0];
			break;
		}
	}

	return volt;
}
EXPORT_SYMBOL(max8986_csr_reg2volt);

static u8 max8986_regulator_ctrl_reg_mask(int regl_id, int *bitPos)
{
	u8 mask = 0;
	switch (regl_id) {
	case MAX8986_REGL_ALDO1:
	case MAX8986_REGL_ALDO8:
	case MAX8986_REGL_HCLDO1:
		*bitPos = 0;
		mask = 0x7;
		break;

	case MAX8986_REGL_DLDO1:
	case MAX8986_REGL_ALDO2:
	case MAX8986_REGL_HCLDO2:
	*bitPos = 4;
		mask = 0x7;
		break;

	case MAX8986_REGL_AUXLDO1:
	case MAX8986_REGL_ALDO4:
	case MAX8986_REGL_DLDO2:
	case MAX8986_REGL_ALDO5:
	case MAX8986_REGL_ALDO7:
	case MAX8986_REGL_DLDO3:
	case MAX8986_REGL_DLDO4:
	case MAX8986_REGL_ALDO9:
	case MAX8986_REGL_TSRLDO:
		*bitPos = 0;
		mask = 0x1F;
		break;

	case MAX8986_REGL_ALDO3:
		*bitPos = 5;
		mask = 0x7;
		break;

	case MAX8986_REGL_SIMLDO:
		*bitPos = 6;
		mask = 0x3;
		break;

	case MAX8986_REGL_LVLDO:
		*bitPos = 0;
		mask = 0xF;
		break;

	case MAX8986_REGL_ALDO6:
		*bitPos = 4;
		mask = 0xF;
		break;

	case MAX8986_REGL_CSR_LPM:
	case MAX8986_REGL_CSR_NM2:
	case MAX8986_REGL_CSR_NM1:
		mask = 0x1F;
		*bitPos = 0;
		break;

	case MAX8986_REGL_IOSR:
		*bitPos = 2;
		mask =  0x1F;
		break;

	default:
		return 0;

	}
	mask <<= *bitPos;
	return mask;
}

static int _max8986_regulator_enable(struct max8986_regl_priv *regl_priv,
									int id)
{
	struct max8986 *max8986 = regl_priv->max8986;
	int ret;
	u8 regVal;
	if ((id < 0) || (id >= MAX8986_REGL_NUM_REGULATOR))
		return -EINVAL;
	ret = max8986->read_dev(max8986, max8986_regls[id].pm_reg, &regVal);
	/*00: ON (normal) 01: Low power mode 10: OFF */
	/*Normal mode : PC1 = 1*/
	regVal &= ~((PMU_REGL_MASK << PC2PC1_01) |
					(PMU_REGL_MASK << PC2PC1_11));
	switch (max8986_regls[id].dsm_opmode) {
	case MAX8986_REGL_OFF_IN_DSM:
		/* clear LPM bits */
		regVal &= ~((PMU_REGL_MASK << PC2PC1_00) |
				(PMU_REGL_MASK << PC2PC1_10));
		/* set to off in DSM */
		regVal |= ((PMU_REGL_OFF << PC2PC1_00) |
				(PMU_REGL_OFF << PC2PC1_10));
		break;
	case MAX8986_REGL_LPM_IN_DSM:
		regVal &= ~((PMU_REGL_MASK << PC2PC1_00) |
				(PMU_REGL_MASK << PC2PC1_10));
		regVal |= ((PMU_REGL_ECO << PC2PC1_00) |
				(PMU_REGL_ECO << PC2PC1_10));
		break;
	case MAX8986_REGL_ON_IN_DSM:
		/* clear LPM bits - on in DSM */
		regVal &= ~((PMU_REGL_MASK << PC2PC1_00) |
				(PMU_REGL_MASK << PC2PC1_10));
		break;
	}
	ret |= max8986->write_dev(max8986, max8986_regls[id].pm_reg, regVal);
	return ret;
}

static int max8986_regulator_enable(struct regulator_dev *rdev)
{
	struct max8986_regl_priv *regl_priv = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	return _max8986_regulator_enable(regl_priv, id);
}

static int _max8986_regulator_disable(struct max8986_regl_priv *regl_priv,
									int id)
{
	struct max8986 *max8986 = regl_priv->max8986;
	int ret;
	u8 regVal = 0;

	if ((id < 0) || (id >= MAX8986_REGL_NUM_REGULATOR))
		return -EINVAL;
	/*00: ON (normal) 01: Low power mode 10: OFF*/
	/*Normal mode : PC1 = 1*/
	regVal = ((PMU_REGL_OFF << PC2PC1_00) |
			(PMU_REGL_OFF << PC2PC1_10) |
			(PMU_REGL_OFF << PC2PC1_01) |
			(PMU_REGL_OFF << PC2PC1_11));
	/* set to off in all modes */
	ret = max8986->write_dev(max8986, max8986_regls[id].pm_reg, regVal);
	return ret;
}

static int max8986_regulator_disable(struct regulator_dev *rdev)
{
	struct max8986_regl_priv *regl_priv = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);

	return _max8986_regulator_disable(regl_priv, id);
}

static int _max8986_regulator_is_enabled(struct max8986_regl_priv *regl_priv,
									int id)
{
	struct max8986 *max8986 = regl_priv->max8986;
	int ret;
	u8 regVal;

	if ((id < 0) || (id >= MAX8986_REGL_NUM_REGULATOR))
		return -EINVAL;
	ret = max8986->read_dev(max8986, max8986_regls[id].pm_reg, &regVal);

	if (ret)
		return ret;
	/*00: ON (normal) 01: Low power mode 10: OFF*/
	return ((regVal & (PMU_REGL_MASK << PC2PC1_01)) == 0);
}

static int max8986_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct max8986_regl_priv *regl_priv = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	return _max8986_regulator_is_enabled(regl_priv, id);
}

static int max8986_get_best_voltage_inx(int reg_id, int min_uV, int max_uV)
{
	/*int reg_id = rdev_get_id(rdev); */
	int i;
	int bestmatch;
	int bestindex;

	/*
	* Locate the minimum voltage fitting the criteria on
	* this regulator. The switchable voltages are not
	* in strict falling order so we need to check them
	* all for the best match.
	*/
	if ((reg_id < 0) || (reg_id >= MAX8986_REGL_NUM_REGULATOR))
		return -EINVAL;

	bestmatch = INT_MAX;
	bestindex = -1;
	for (i = 0; i < max8986_regls[reg_id].num_vol; i++) {
		if (max8986_regls[reg_id].vol_list[i][0] >= min_uV &&
			max8986_regls[reg_id].vol_list[i][0] < bestmatch) {

			bestmatch = max8986_regls[reg_id].vol_list[i][0];
			bestindex = i;
		}
	}

	if (bestindex < 0 || bestmatch > max_uV) {
		pr_info("%s: no possible values for min_uV = %d & max_uV = %d\n"
						, __func__, min_uV, max_uV);
		return -EINVAL;
	}
	return bestindex;
}

static int _max8986_regulator_set_voltage(struct max8986_regl_priv *pri_dev,
							int id, int volt)
{
	struct max8986_regl_priv *regl_priv = pri_dev;
	struct max8986 *max8986 = regl_priv->max8986;
	int bitPos, ret;
	u8 mask, regVal;

	if ((id < 0) || (id >= MAX8986_REGL_NUM_REGULATOR))
		return -EINVAL;
	mask = max8986_regulator_ctrl_reg_mask(id, &bitPos);
	if (!mask)
		return -EINVAL;
	ret = max8986->read_dev(max8986, max8986_regls[id].ctrl_reg, &regVal);
	regVal &= ~(mask);
	volt <<= bitPos;
	regVal |= (volt & mask);
	ret |= max8986->write_dev(max8986, max8986_regls[id].ctrl_reg, regVal);
	return ret;
}

static int max8986_regulator_set_voltage(struct regulator_dev *rdev,
						int min_uV, int max_uV)
{
	struct max8986_regl_priv *regl_priv = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	int inx;
	u8 value;

	/* Find the best index */
	inx = max8986_get_best_voltage_inx(id, min_uV, max_uV);

	if (inx < 0 || inx >= max8986_regls[id].num_vol)
		return inx;
	value = max8986_regls[id].vol_list[inx][1];

	return _max8986_regulator_set_voltage(regl_priv, id, value);
}

static int _max8986_regulator_get_voltage(struct max8986_regl_priv *pri_dev,
									int id)
{
	int ret, val;
	u8 mask, regVal;
	int bitPos, i;
	struct max8986_regl_priv *regl_priv = pri_dev;
	struct max8986 *max8986 = regl_priv->max8986;

	if ((id < 0) || (id >= MAX8986_REGL_NUM_REGULATOR))
		return -EINVAL;

	ret = max8986->read_dev(max8986, max8986_regls[id].ctrl_reg, &regVal);
	if (ret)
		return ret;
	mask = max8986_regulator_ctrl_reg_mask(id, &bitPos);
	val = (regVal & mask) >> bitPos;

	for (i = 0; i < max8986_regls[id].num_vol; i++) {
		if (val == max8986_regls[id].vol_list[i][1]) {
			pr_info("%s id: %d val: %d\n", __func__, id, val);
			return max8986_regls[id].vol_list[i][0];
		}
	}
	return -EINVAL;
}

static int max8986_regulator_get_voltage(struct regulator_dev *rdev)
{
	struct max8986_regl_priv *regl_priv = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);

	return _max8986_regulator_get_voltage(regl_priv, id);
}

static int _max8986_regulator_set_mode(struct max8986_regl_priv *pri_dev,
						int id, pmu_regl_state mode)
{
	struct max8986_regl_priv *regl_priv = pri_dev;
	struct max8986 *max8986 = regl_priv->max8986;
	u8 regVal;
	int ret;

	if ((id < 0) || (id >= MAX8986_REGL_NUM_REGULATOR))
		return -EINVAL;
	if (mode < 0 || mode >= PMU_REGL_MASK)
		return -EINVAL;
	ret = max8986->read_dev(max8986, max8986_regls[id].pm_reg, &regVal);
	if (ret)
		return ret;
	/*Clear it off and then set the passed mode. */
	regVal &= ~((PMU_REGL_MASK << PC2PC1_00) |
					(PMU_REGL_MASK << PC2PC1_10));
	regVal |= ((mode << PC2PC1_00) | (mode << PC2PC1_10));
	printk(KERN_INFO "%s: rgVal = %x\n",__func__,regVal);
	ret = max8986->write_dev(max8986, max8986_regls[id].pm_reg, regVal);
	return ret;
}

static int max8986_regulator_set_mode(struct regulator_dev *rdev,
							unsigned int mode)
{
	u8 opmode;
	struct max8986_regl_priv *regl_priv = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	printk(KERN_INFO "%s:mode = %d\n",__func__,mode);

	switch(mode)
	{
	case REGULATOR_MODE_FAST:
		opmode = PMU_REGL_TURBO;
		break;

	case REGULATOR_MODE_NORMAL:
		opmode = PMU_REGL_ON;
		break;
	case REGULATOR_MODE_IDLE:
		opmode = PMU_REGL_ECO;
		break;
	case REGULATOR_MODE_STANDBY:
		opmode = PMU_REGL_OFF;
		break;

	default:
		return -EINVAL;
	}

	return _max8986_regulator_set_mode(regl_priv, id, opmode);

}

static int _max8986_regulator_get_mode(struct max8986_regl_priv *pri_dev,
									int id)
{
	struct max8986_regl_priv *regl_priv = pri_dev;
	struct max8986 *max8986 = regl_priv->max8986;

	u8 regVal = 0;
	int ret;

	if ((id < 0) || (id >= MAX8986_REGL_NUM_REGULATOR))
		return -EINVAL;

	ret = max8986->read_dev(max8986, max8986_regls[id].pm_reg, &regVal);
	pr_info("%s : regVal = 0x%x\n", __func__, regVal);
	if (ret)
		return ret;
	regVal &= (PMU_REGL_MASK << PC2PC1_00);
	regVal >>= PC2PC1_00;
	return regVal;
}

static unsigned int max8986_regulator_get_mode(struct regulator_dev *rdev)
{
	u8 mode;
	struct max8986_regl_priv *regl_priv = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);

	mode = _max8986_regulator_get_mode(regl_priv, id);
	printk(KERN_INFO "%s:opmode = %d\n",__func__,mode);
	switch(mode)
	{
	case PMU_REGL_ON:
		return REGULATOR_MODE_NORMAL;
	case PMU_REGL_ECO:
		return REGULATOR_MODE_IDLE;
	case PMU_REGL_OFF:
		return REGULATOR_MODE_STANDBY;
	case PMU_REGL_TURBO:
		return REGULATOR_MODE_FAST;
	}
	return 0;
}


static struct regulator_ops max8986_regulator_ops = {
	.enable = max8986_regulator_enable,
	.disable = max8986_regulator_disable,
	.is_enabled = max8986_regulator_is_enabled,
	.set_voltage = max8986_regulator_set_voltage,
	.get_voltage = max8986_regulator_get_voltage,
	.set_mode = max8986_regulator_set_mode,
	.get_mode = max8986_regulator_get_mode,
};

static int max8986_regulator_ioctl_handler(u32 cmd, u32 arg, void *pri_data)
{
	struct max8986_regl_priv *regl_priv =
		(struct max8986_regl_priv *) pri_data;
	int ret = -EINVAL;
	pr_debug("Inside %s, IOCTL command is %d\n", __func__, cmd);
	switch (cmd) {
	case BCM_PMU_IOCTL_SET_VOLTAGE:
	{
		pmu_regl_volt regulator;
		int inx;
		u8  value;
		if (copy_from_user(&regulator,
					(pmu_regl_volt *)arg,
					sizeof(pmu_regl_volt)) != 0) {
			return -EFAULT;
		}

		if ((regulator.regl_id < 0) ||
			(regulator.regl_id >= MAX8986_REGL_NUM_REGULATOR))
			return -EINVAL;

		/* Validate the input voltage */
		inx = max8986_get_best_voltage_inx(
				regulator.regl_id,
				regulator.voltage,
				regulator.voltage);
		if (inx < 0 || inx >= max8986_regls[regulator.regl_id].num_vol)
			return -EINVAL;
		value =
			(u8)max8986_regls[regulator.regl_id].vol_list[inx][1];
		ret = _max8986_regulator_set_voltage(regl_priv,
				regulator.regl_id,
				value);
		break;
	}
	case BCM_PMU_IOCTL_GET_VOLTAGE:
	{
		pmu_regl_volt regulator;
		int volt;
		if (copy_from_user(&regulator,
					(pmu_regl_volt *)arg,
					sizeof(pmu_regl_volt)) != 0) {

			return -EFAULT;
		}
		volt  = _max8986_regulator_get_voltage(
				regl_priv,
				regulator.regl_id);
		if (volt > 0) {
			regulator.voltage = volt;
			ret = copy_to_user((pmu_regl_volt *)arg,
					&regulator,
					sizeof(regulator));
		} else
			ret = volt;
		break;
	}
	case BCM_PMU_IOCTL_GET_REGULATOR_STATE:
	{
		pmu_regl rmode;
		if (copy_from_user(&rmode, (pmu_regl *)arg,
					sizeof(pmu_regl)) != 0) {

			return -EFAULT;
		}
		rmode.state = _max8986_regulator_get_mode(
				regl_priv,
				rmode.regl_id);
		if (rmode.state > 0)
			ret = copy_to_user((pmu_regl *)arg,
					&rmode,
					sizeof(rmode));
		else
			ret = rmode.state;
		break;
	}
	case BCM_PMU_IOCTL_SET_REGULATOR_STATE:
	{
		pmu_regl rmode;
		if (copy_from_user(&rmode, (pmu_regl *)arg,
					sizeof(pmu_regl)) != 0)
			return -EFAULT;
		ret = _max8986_regulator_set_mode(regl_priv,
				rmode.regl_id,
				rmode.state);
		break;
	}
	case BCM_PMU_IOCTL_ACTIVATESIM:
	{
		int id = MAX8986_REGL_SIMLDO;
		pmu_sim_volt sim_volt;
		u8 value;
		if (copy_from_user(&sim_volt, (int *)arg,
					sizeof(int)) != 0)
			return -EFAULT;

		/*Make sure that voltage index inx is valid */
		if(max8986_regls[id].num_vol >= sim_volt)
			return -EINVAL;
		/*check the status of SIMLDO*/
		ret = _max8986_regulator_is_enabled(regl_priv, id);
		if (ret) {
			pr_info("SIMLDO is activated already\n");
			return -EPERM;
		}
		/* Put SIMLDO in ON State */
		ret = _max8986_regulator_enable(regl_priv, id);
		if (ret)
			return ret;
		/* Set SIMLDO voltage */
		value = max8986_regls[id].vol_list[sim_volt][1];
		ret = _max8986_regulator_set_voltage(regl_priv, id,
				value);
		break;
	}
	case BCM_PMU_IOCTL_DEACTIVATESIM:
	{
		int id = MAX8986_REGL_SIMLDO;
		/*check the status of SIMLDO*/
		ret = _max8986_regulator_is_enabled(regl_priv, id);
		if (!ret) {
			pr_info("SIMLDFO is already disabled\n");
			return -EPERM;
		}
		ret = _max8986_regulator_disable(regl_priv, id);
		if (ret)
			return ret;
		break;
	}
	}	/*end of switch*/
	return ret;
}

static int __devinit max8986_regulator_probe(struct platform_device *pdev)
{
	struct max8986_regl_priv *regl_priv;
	struct max8986 *max8986 = dev_get_drvdata(pdev->dev.parent);
	struct max8986_regl_pdata *regulators;
	int i, ret;

	pr_info("%s\n", __func__);

	if (unlikely(!max8986->pdata || !max8986->pdata->regulators)) {
		pr_err("%s: invalid platform data !!!\n", __func__);
		return -EINVAL;
	}
	regulators = max8986->pdata->regulators;

	regl_priv = kzalloc((sizeof(struct max8986_regl_priv) +
				regulators->num_regulators*sizeof(
					struct regulator_dev *)),
					GFP_KERNEL);
	if (unlikely(!regl_priv))
		return -ENOMEM;

	regl_priv->max8986 = max8986;

	for (i = 0; i < regulators->num_regulators; i++) {
		/* copy flag */
		max8986_regls[regulators->regl_init[i].regl_id].dsm_opmode =
			regulators->regl_init[i].dsm_opmode;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30))
		pdev->dev.platform_data = regulators->regl_init[i].init_data;

		regl_priv->regl[i] = regulator_register(
			&max8986_regls[regulators->regl_init[i].regl_id].desc,
			&pdev->dev, regl_priv);
#else
		regl_priv->regl[i] = regulator_register(
			&max8986_regls[regulators->regl_init[i].regl_id].desc,
			&pdev->dev, regulators->regl_init[i].init_data,
			regl_priv);

#endif

		if (IS_ERR(regl_priv->regl[i])) {
			pr_err("%s: regulator_register Error !!!\n", __func__);
			ret = PTR_ERR(regl_priv->regl[i]);
			goto err;
		}
	}
	regl_priv->num_regl = regulators->num_regulators;

	/* Set default Regualtor PM mode values */
	max8986->write_dev(max8986, MAX8986_PM_REG_A1OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_ALDO1]);
	max8986->write_dev(max8986, MAX8986_PM_REG_A2OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_ALDO2]);
	max8986->write_dev(max8986, MAX8986_PM_REG_A3OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_ALDO3]);
	max8986->write_dev(max8986, MAX8986_PM_REG_A4OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_ALDO4]);
	max8986->write_dev(max8986, MAX8986_PM_REG_A5OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_ALDO5]);
	max8986->write_dev(max8986, MAX8986_PM_REG_A6OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_ALDO6]);
	max8986->write_dev(max8986, MAX8986_PM_REG_A7OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_ALDO7]);
	max8986->write_dev(max8986, MAX8986_PM_REG_A8OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_ALDO8]);
	max8986->write_dev(max8986, MAX8986_PM_REG_A9OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_ALDO9]);
	max8986->write_dev(max8986, MAX8986_PM_REG_D1OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_DLDO1]);
	max8986->write_dev(max8986, MAX8986_PM_REG_D2OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_DLDO2]);
	max8986->write_dev(max8986, MAX8986_PM_REG_D3OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_DLDO3]);
	max8986->write_dev(max8986, MAX8986_PM_REG_D4OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_DLDO4]);
	max8986->write_dev(max8986, MAX8986_PM_REG_H1OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_HCLDO1]);
	max8986->write_dev(max8986, MAX8986_PM_REG_H2OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_HCLDO2]);
	max8986->write_dev(max8986, MAX8986_PM_REG_LVOPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_LVLDO]);
	max8986->write_dev(max8986, MAX8986_PM_REG_AX1OPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_AUXLDO1]);
	max8986->write_dev(max8986, MAX8986_PM_REG_TSROPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_TSRLDO]);

	max8986->write_dev(max8986, MAX8986_PM_REG_CSROPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_CSR_LPM]);
	max8986->write_dev(max8986, MAX8986_PM_REG_IOSROPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_IOSR]);
	max8986->write_dev(max8986, MAX8986_PM_REG_SIMOPMODCTRL,
			regulators->regl_default_pmmode[MAX8986_REGL_SIMLDO]);

	platform_set_drvdata(pdev, regl_priv);

	max8986_register_ioctl_handler(max8986, MAX8986_SUBDEV_REGULATOR,
					max8986_regulator_ioctl_handler,
					regl_priv);

	pr_info("%s: success\n", __func__);

	return 0;

err:
	while (--i >= 0)
		regulator_unregister(regl_priv->regl[i]);
	kfree(regl_priv);
	return ret;
}

static int __devexit max8986_regulator_remove(struct platform_device *pdev)
{
	struct max8986_regl_priv *regl_priv = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < regl_priv->num_regl; i++)
		regulator_unregister(regl_priv->regl[i]);
	return 0;
}

static struct platform_driver max8986_regulator_driver = {
	.driver	= {
		.name	= "max8986-regulator",
		.owner	= THIS_MODULE,
	},
	.remove		= __devexit_p(max8986_regulator_remove),
	.probe		= max8986_regulator_probe,
};

static int __init max8986_regulator_init(void)
{
	return platform_driver_register(&max8986_regulator_driver);
}
subsys_initcall(max8986_regulator_init);

static void __exit max8986_regulator_exit(void)
{
	platform_driver_unregister(&max8986_regulator_driver);
}
module_exit(max8986_regulator_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Regulator Driver for MAX8986 PMU");
MODULE_ALIAS("platform:max8986-regulator");

