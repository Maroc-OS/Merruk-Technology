/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/audio/max8986-audio.c
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license
* * other than the GPL, without Broadcom's express prior written consent.
* *****************************************************************************/

/**
*
*   @file   max8986-audio.c
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
#include <linux/io.h>
#include <linux/broadcom/max8986/max8986-audio.h>

/* bitwise definition */
/* Input */
#define AUDIO_IN_INADIFF	(0x1 << 7)
#define AUDIO_IN_INBDIFF	(0x1 << 6)
#define AUDIO_IN_PREAMP_GAIN_MASK	0x7
#define AUDIO_IN_B_PREAMP_BIT	0
#define AUDIO_IN_A_PREAMP_BIT	0x3

/* HP and SP MIXER */
#define AUDIO_MIXER_HPL_MIX_BIT	0x4
#define AUDIO_MIXER_HPL_MIX_MASK (0xF << AUDIO_MIXER_HPL_MIX_BIT)
#define AUDIO_MIXER_HPR_MIX_BIT	0
#define AUDIO_MIXER_HPR_MIX_MASK (0xF << AUDIO_MIXER_HPR_MIX_BIT)
#define AUDIO_MIXER_SPK_MIX_BIT	0
#define AUDIO_MIXER_SPK_MIX_MASK (0xF << AUDIO_MIXER_SPK_MIX_BIT)

/* control register */
#define AUDIO_CONT1_ZCD		(0x1 << 7)
#define AUDIO_CONT1_SLEW	(0x1 << 6)
#define AUDIO_CONT1_HPLM	(0x1 << 5)
#define AUDIO_CONT2_LPGAIN	(0x1 << 7)
#define AUDIO_CONT2_HPRM	(0x1 << 5)	/* NEED TO CHECK THE VALUE */
#define AUDIO_HP_VOL_MASK	0x1F

#define AUDIO_CONT3_FFM		(0x1 << 7)
#define AUDIO_CONT3_SPKM	(0x1 << 6)
#define AUDIO_SP_VOL_MASK	0x3F

/* Power Management */
#define AUDIO_PM_SHDN		(0x1 << 7)
#define AUDIO_PM_SPKEN		(0x1 << 4)
#define AUDIO_PM_HPLEN		(0x1 << 2)
#define AUDIO_PM_HPREN		(0x1 << 1)
#define AUDIO_PM_IHF_BYPASS	0x1
#define AUDIO_PM_LPMOD_BIT	0x5

/* HP LEFT AND RIGHT VOLUME */
enum {
	HP_VOLUME_NEG_64DB = 0,
	HP_VOLUME_NEG_60DB,
	HP_VOLUME_NEG_56DB,
	HP_VOLUME_NEG_52DB,
	HP_VOLUME_NEG_48DB,
	HP_VOLUME_NEG_44DB,
	HP_VOLUME_NEG_40DB,
	HP_VOLUME_NEG_37DB,
	HP_VOLUME_NEG_34DB,
	HP_VOLUME_NEG_31DB,
	HP_VOLUME_NEG_28DB,
	HP_VOLUME_NEG_25DB,
	HP_VOLUME_NEG_22DB,
	HP_VOLUME_NEG_19DB,
	HP_VOLUME_NEG_16DB,
	HP_VOLUME_NEG_14DB,
	HP_VOLUME_NEG_12DB,
	HP_VOLUME_NEG_10DB,
	HP_VOLUME_NEG_8DB,
	HP_VOLUME_NEG_6DB,
	HP_VOLUME_NEG_4DB,
	HP_VOLUME_NEG_2DB,
	HP_VOLUME_NEG_1DB,
	HP_VOLUME_0DB,
	HP_VOLUME_1DB,
	HP_VOLUME_2DB,
	HP_VOLUME_3DB,
	HP_VOLUME_4DB,
	HP_VOLUME_4_5DB,
	HP_VOLUME_5DB,
	HP_VOLUME_5_5DB,
	HP_VOLUME_6DB,
	HP_VOLUME_MUTE
};

/* Speaker volume */
enum {
	SP_VOLUME_NEG_30DB = 0x18,
	SP_VOLUME_NEG_26DB,
	SP_VOLUME_NEG_22DB,
	SP_VOLUME_NEG_18DB,
	SP_VOLUME_NEG_14DB,
	SP_VOLUME_NEG_12DB,
	SP_VOLUME_NEG_10DB,
	SP_VOLUME_NEG_8DB,
	SP_VOLUME_NEG_6DB,
	SP_VOLUME_NEG_4DB,
	SP_VOLUME_NEG_2DB,
	SP_VOLUME_0DB,
	SP_VOLUME_1DB,
	SP_VOLUME_2DB,
	SP_VOLUME_3DB,
	SP_VOLUME_4DB,
	SP_VOLUME_5DB,
	SP_VOLUME_6DB,
	SP_VOLUME_7DB,
	SP_VOLUME_8DB,
	SP_VOLUME_9DB,
	SP_VOLUME_10DB,
	SP_VOLUME_11DB,
	SP_VOLUME_12DB,
	SP_VOLUME_12_5DB,
	SP_VOLUME_13DB,
	SP_VOLUME_13_5DB,
	SP_VOLUME_14DB,
	SP_VOLUME_14_5DB,
	SP_VOLUME_15DB,
	SP_VOLUME_15_5DB,
	SP_VOLUME_16DB,
	SP_VOLUME_16_5DB,
	SP_VOLUME_17DB,
	SP_VOLUME_17_5DB,
	SP_VOLUME_18DB,
	SP_VOLUME_18_5DB,
	SP_VOLUME_19DB,
	SP_VOLUME_19_5DB,
	SP_VOLUME_20DB,
	SP_VOLUME_MUTE
};

/* PM REG LPMODE */
enum {
	PM_LPMODE_DIS = 0x0,
	PM_LPMODE_INA_SE = 0x1,
	PM_LPMODE_INB_SE = 0x2,
	PM_LPMODE_INA_DIFF = 0x3
};

struct max8986_audio {
	struct max8986 *max8986;
	bool IHFenabled;
	bool HSenabled;
};

static struct max8986_audio *driv_data;

int max8986_audio_set_mixer_input(int audio_path, max8986_mixer_input input)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 regVal;
	int ret = -EINVAL;
	pr_info("Inside %s\n", __func__);
	if (audio_path == MAX8986_AUDIO_HS_RIGHT ||
	    audio_path == MAX8986_AUDIO_HS_LEFT) {
		ret =
		    max8986->read_dev(max8986, MAX8986_AUDIO_REG_HP_MIXER,
				      &regVal);
		if (audio_path == MAX8986_AUDIO_HS_LEFT) {
			regVal &= ~AUDIO_MIXER_HPL_MIX_MASK;
			regVal |=
			    (input << AUDIO_MIXER_HPL_MIX_BIT) &
			    AUDIO_MIXER_HPL_MIX_MASK;
		} else {
			regVal &= ~AUDIO_MIXER_HPR_MIX_MASK;
			regVal |=
			    (input << AUDIO_MIXER_HPR_MIX_BIT) &
			    AUDIO_MIXER_HPR_MIX_MASK;
		}
		ret |=
		    max8986->write_dev(max8986, MAX8986_AUDIO_REG_HP_MIXER,
				       regVal);
	} else if (audio_path == MAX8986_AUDIO_SPK) {
		regVal =
		    (input << AUDIO_MIXER_SPK_MIX_BIT) &
		    AUDIO_MIXER_SPK_MIX_MASK;
		ret =
		    max8986->write_dev(max8986, MAX8986_AUDIO_REG_SP_MIXER,
				       regVal);
	}
	return ret;
}

EXPORT_SYMBOL(max8986_audio_set_mixer_input);

int max8986_set_input_mode(int input_path, int mode)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 regVal;
	int ret;
	u8 bitPos;
	pr_info("Inside %s\n", __func__);

	ret = max8986->read_dev(max8986, MAX8986_AUDIO_REG_INPUT_GAIN, &regVal);
	if (ret)
		return ret;

	if (input_path == MAX8986_INPUTA)
		bitPos = AUDIO_IN_INADIFF;
	else if (input_path == MAX8986_INPUTB)
		bitPos = AUDIO_IN_INBDIFF;
	else
		return -EINVAL;

	if (mode == MAX8986_INPUT_MODE_DIFFERENTIAL)
		regVal |= bitPos;
	else if (mode == MAX8986_INPUT_MODE_SINGLE_ENDED)
		regVal &= ~bitPos;
	else
		return -EINVAL;
	return max8986->write_dev(max8986, MAX8986_AUDIO_REG_INPUT_GAIN,
				  regVal);
}

EXPORT_SYMBOL(max8986_set_input_mode);

int max8986_set_input_preamp_gain(int input_path, int gain)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 regVal;
	int ret;
	u8 bitPos;
	pr_info("Inside %s\n", __func__);

	ret = max8986->read_dev(max8986, MAX8986_AUDIO_REG_INPUT_GAIN, &regVal);
	if (ret)
		return ret;

	if (input_path == MAX8986_INPUTA)
		bitPos = AUDIO_IN_A_PREAMP_BIT;
	else if (input_path == MAX8986_INPUTB)
		bitPos = AUDIO_IN_B_PREAMP_BIT;
	else
		return -EINVAL;

	regVal &= ~(AUDIO_IN_PREAMP_GAIN_MASK << bitPos);
	regVal |= ((gain & AUDIO_IN_PREAMP_GAIN_MASK) << bitPos);
	return max8986->write_dev(max8986, MAX8986_AUDIO_REG_INPUT_GAIN,
				  regVal);
}
EXPORT_SYMBOL(max8986_set_input_preamp_gain);

int max8986_set_hs_preamp_gain(int gain)
{
	struct max8986 *max8986 = driv_data->max8986;
	struct max8986_audio_pdata *audio_pdata = max8986->pdata->audio_pdata;
	u8 regVal;
	int ret;
	u8 bitPos;

	pr_info("Inside %s\n", __func__);

	ret = max8986->read_dev(max8986, MAX8986_AUDIO_REG_INPUT_GAIN, &regVal);
	if (ret)
		return ret;

	if (audio_pdata->hs_input_path == MAX8986_INPUTA)
		bitPos = AUDIO_IN_A_PREAMP_BIT;
	else if (audio_pdata->hs_input_path == MAX8986_INPUTB)
		bitPos = AUDIO_IN_B_PREAMP_BIT;
	else
		return -EINVAL;

	regVal &= ~(AUDIO_IN_PREAMP_GAIN_MASK << bitPos);
	regVal |= ((gain & AUDIO_IN_PREAMP_GAIN_MASK) << bitPos);
	return max8986->write_dev(max8986, MAX8986_AUDIO_REG_INPUT_GAIN,
				  regVal);
}
EXPORT_SYMBOL(max8986_set_hs_preamp_gain);

int max8986_set_ihf_preamp_gain(int gain)
{
	struct max8986 *max8986 = driv_data->max8986;
	struct max8986_audio_pdata *audio_pdata = max8986->pdata->audio_pdata;
	u8 regVal;
	int ret;
	u8 bitPos;

	pr_info("Inside %s\n", __func__);

	ret = max8986->read_dev(max8986, MAX8986_AUDIO_REG_INPUT_GAIN, &regVal);
	if (ret)
		return ret;

	if (audio_pdata->ihf_input_path == MAX8986_INPUTA)
		bitPos = AUDIO_IN_A_PREAMP_BIT;
	else if (audio_pdata->ihf_input_path == MAX8986_INPUTB)
		bitPos = AUDIO_IN_B_PREAMP_BIT;
	else
		return -EINVAL;

	regVal &= ~(AUDIO_IN_PREAMP_GAIN_MASK << bitPos);
	regVal |= ((gain & AUDIO_IN_PREAMP_GAIN_MASK) << bitPos);
	return max8986->write_dev(max8986, MAX8986_AUDIO_REG_INPUT_GAIN,
				  regVal);
}
EXPORT_SYMBOL(max8986_set_ihf_preamp_gain);

int max8986_audio_hs_poweron(bool power_on)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 regVal;
	int ret;
	pr_info("Inside %s\n", __func__);
	ret = max8986->read_dev(max8986, MAX8986_AUDIO_REG_PM, &regVal);
	if (ret)
		return ret;
	if (power_on) {
                max8986_audio_enable_vol_slew(0);  // 20110706 remove earphone popup
	
		regVal |= (AUDIO_PM_SHDN | AUDIO_PM_HPLEN | AUDIO_PM_HPREN);
		driv_data->HSenabled = true;
	} else {
                max8986_audio_enable_vol_slew(1);  // 20110706 remove earphone popup
	
		if (!driv_data->IHFenabled)
			regVal &= ~AUDIO_PM_SHDN;
		regVal &= ~(AUDIO_PM_HPLEN | AUDIO_PM_HPREN);
		driv_data->HSenabled = false;
	}
        return max8986->write_dev(max8986, MAX8986_AUDIO_REG_PM, regVal);
}
EXPORT_SYMBOL(max8986_audio_hs_poweron);

int max8986_audio_hs_shortcircuit_enable(bool enable)
{
	/*struct max8986 *max8986 = driv_data->max8986; */
	pr_info("Inside %s\n", __func__);
	/* How to do this for Broadcom */
	return 0;
}
EXPORT_SYMBOL(max8986_audio_hs_shortcircuit_enable);

int max8986_audio_hs_set_gain(int hspath, int hsgain)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 regVal;
	int ret = 0;
	pr_info("Inside %s\n", __func__);

	if ((hspath == AUDIO_HS_LEFT) || (hspath == AUDIO_HS_BOTH)) {
		ret = max8986->read_dev(max8986,
					MAX8986_AUDIO_REG_HP_LEFT, &regVal);
		if (hsgain == HP_VOLUME_MUTE)
			regVal |= AUDIO_CONT1_HPLM;	/* Mute */
		else {
			regVal &= ~(AUDIO_HP_VOL_MASK | AUDIO_CONT1_HPLM);
			regVal |= (hsgain & AUDIO_HP_VOL_MASK);
		}
		ret |= max8986->write_dev(max8986,
					  MAX8986_AUDIO_REG_HP_LEFT, regVal);
		if (ret)
			return ret;
	}
	if ((hspath == AUDIO_HS_RIGHT) || (hspath == AUDIO_HS_BOTH)) {
		ret = max8986->read_dev(max8986,
					MAX8986_AUDIO_REG_HP_RIGHT, &regVal);
		if (hsgain == HP_VOLUME_MUTE)
			regVal |= AUDIO_CONT2_HPRM;
		else {
			regVal &= ~(AUDIO_HP_VOL_MASK | AUDIO_CONT2_HPRM);
			regVal |= (hsgain & AUDIO_HP_VOL_MASK);
		}
		ret |= max8986->write_dev(max8986,
					  MAX8986_AUDIO_REG_HP_RIGHT, regVal);
		if (ret)
			return ret;
	}
	return 0;
}

EXPORT_SYMBOL(max8986_audio_hs_set_gain);

int max8986_audio_hs_ihf_poweron(void)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 regVal;
	int ret;
	pr_info("Inside %s\n", __func__);
	ret = max8986->read_dev(max8986, MAX8986_AUDIO_REG_PM, &regVal);
	if (ret)
		return ret;
	regVal |= (AUDIO_PM_SHDN | AUDIO_PM_SPKEN);
	driv_data->IHFenabled = true;
	return max8986->write_dev(max8986, MAX8986_AUDIO_REG_PM, regVal);
}

EXPORT_SYMBOL(max8986_audio_hs_ihf_poweron);

int max8986_audio_hs_ihf_poweroff(void)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 regVal;
	int ret;
	pr_info("Inside %s\n", __func__);
	ret = max8986->read_dev(max8986, MAX8986_AUDIO_REG_PM, &regVal);
	if (ret)
		return ret;
	if (driv_data->HSenabled == false)
		regVal &= ~AUDIO_PM_SHDN;
	regVal &= ~AUDIO_PM_SPKEN;
	driv_data->IHFenabled = false;
	return max8986->write_dev(max8986, MAX8986_AUDIO_REG_PM, regVal);
}

EXPORT_SYMBOL(max8986_audio_hs_ihf_poweroff);

int max8986_audio_hs_ihf_enable_bypass(bool enable)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 regVal;
	int ret;
	pr_info("Inside %s\n", __func__);
	ret = max8986->read_dev(max8986, MAX8986_AUDIO_REG_PM, &regVal);
	if (ret)
		return ret;
	if (enable)
		regVal &= ~AUDIO_PM_IHF_BYPASS;
	else
		regVal |= AUDIO_PM_IHF_BYPASS;
	return max8986->write_dev(max8986, MAX8986_AUDIO_REG_PM, regVal);
}

EXPORT_SYMBOL(max8986_audio_hs_ihf_enable_bypass);

int max8986_audio_hs_ihf_set_gain(int ihfgain)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 regVal;
	int ret;
	pr_info("Inside %s\n", __func__);
	ret = max8986->read_dev(max8986, MAX8986_AUDIO_REG_SPEAKER, &regVal);
	if (ihfgain == SP_VOLUME_MUTE)
		regVal |= AUDIO_CONT3_SPKM;
	else {
		regVal &= ~(AUDIO_CONT3_SPKM | AUDIO_SP_VOL_MASK);
		regVal |= (ihfgain & AUDIO_SP_VOL_MASK);
	}
	return max8986->write_dev(max8986, MAX8986_AUDIO_REG_SPEAKER, regVal);
}

EXPORT_SYMBOL(max8986_audio_hs_ihf_set_gain);

int max8986_audio_enable_vol_slew(int enable)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 reg_val;
	int ret;
	pr_info("Inside %s\n", __func__);
	ret = max8986->read_dev(max8986,
					MAX8986_AUDIO_REG_HP_LEFT, &reg_val);

	if(enable)
		reg_val &= ~AUDIO_CONT1_SLEW;
	else
		reg_val |= AUDIO_CONT1_SLEW; /*set bit disable the feature*/

	ret |= max8986->write_dev(max8986,
					MAX8986_AUDIO_REG_HP_LEFT, reg_val);
	return ret;
}
EXPORT_SYMBOL(max8986_audio_enable_vol_slew);

int max8986_audio_enable_zcd(int enable)
{
	struct max8986 *max8986 = driv_data->max8986;
	u8 reg_val;
	int ret;
	pr_info("Inside %s\n", __func__);
	ret = max8986->read_dev(max8986,
					MAX8986_AUDIO_REG_HP_LEFT, &reg_val);

	if(enable)
		reg_val &= ~AUDIO_CONT1_ZCD;
	else
		reg_val |= AUDIO_CONT1_ZCD; /*set bit disable the feature*/

	ret |= max8986->write_dev(max8986,
					MAX8986_AUDIO_REG_HP_LEFT, reg_val);
	return ret;
}
EXPORT_SYMBOL(max8986_audio_enable_zcd);

static int __devinit max8986_audio_probe(struct platform_device *pdev)
{
	u8 reg_val;
	struct max8986 *max8986 = dev_get_drvdata(pdev->dev.parent);
	struct max8986_audio *audio_data;
	pr_info("Inside %s\n", __func__);

	if (!max8986)
		return -EINVAL;
	audio_data = kzalloc((sizeof(struct max8986_audio)), GFP_KERNEL);
	if (!audio_data) {
		pr_err("%s : Can not allocate memory\n", __func__);
		return -ENOMEM;
	}
	audio_data->HSenabled = false;
	audio_data->IHFenabled = false;
	audio_data->max8986 = max8986;
	driv_data = audio_data;
	platform_set_drvdata(pdev, audio_data);

	/*Disable volume slewing & zero crossing detection by default*/
	max8986->read_dev(max8986,
					MAX8986_AUDIO_REG_HP_LEFT, &reg_val);
	reg_val |= (AUDIO_CONT1_SLEW|AUDIO_CONT1_ZCD); /*set bit disable the feature*/

	max8986->write_dev(max8986,
					MAX8986_AUDIO_REG_HP_LEFT, reg_val);

// 20110706 remove earphone popup 
	max8986_audio_hs_ihf_enable_bypass(1);
	max8986->write_dev(max8986, MAX8986_AUDIO_REG_CHARGE_PUMP, 0x01);

	/*set default miser input, mode and per-amp gain settings */
	if (max8986->pdata && max8986->pdata->audio_pdata) {
		struct max8986_audio_pdata *audio_pdata =
		    max8986->pdata->audio_pdata;
		max8986_audio_set_mixer_input(MAX8986_AUDIO_HS_RIGHT,
					      audio_pdata->rhs_def_mixer_in);
		max8986_audio_set_mixer_input(MAX8986_AUDIO_HS_LEFT,
					      audio_pdata->lhs_def_mixer_in);
		max8986_audio_set_mixer_input(MAX8986_AUDIO_SPK,
					      audio_pdata->ihf_def_mixer_in);

		max8986_set_input_preamp_gain(MAX8986_INPUTA,
					      audio_pdata->ina_def_preampgain);
		max8986_set_input_preamp_gain(MAX8986_INPUTB,
					      audio_pdata->inb_def_preampgain);

		max8986_set_input_mode(MAX8986_INPUTA,
				       audio_pdata->ina_def_mode);
		max8986_set_input_mode(MAX8986_INPUTB,
				       audio_pdata->inb_def_mode);
	}

	return 0;
}

static int __devexit max8986_audio_remove(struct platform_device *pdev)
{
	struct max8986_audio *audio_data = platform_get_drvdata(pdev);
	kfree(audio_data);
	driv_data = NULL;
	return 0;
}

static struct platform_driver max8986_audio_driver = {
	.driver = {
		   .name = "max8986-audio",
		   .owner = THIS_MODULE,
		   },
	.remove = __devexit_p(max8986_audio_remove),
	.probe = max8986_audio_probe,
};

static int __init max8986_audio_init(void)
{
	return platform_driver_register(&max8986_audio_driver);
}

late_initcall(max8986_audio_init);

static void __exit max8986_audio_exit(void)
{
	platform_driver_unregister(&max8986_audio_driver);
}

module_exit(max8986_audio_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Audio Interface Driver for MAX8986 PMU");
MODULE_ALIAS("platform:max8986-audio");
