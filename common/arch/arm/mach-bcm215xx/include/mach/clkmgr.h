/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/mach-bcm215xx/include/mach/clkmgr.h
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
 *   @file   clkmgr.h
 *
 *   @brief  BCM21553 side interface functions for clock frame work.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_MACH_BCM21553_CLOCK_H
#define __ARCH_ARM_MACH_BCM21553_CLOCK_H

#define BCM_CLK_MAIN_PLL_STR_ID		"mainpll"
#define BCM_CLK_APPS_PLL_STR_ID		"appspll"
#define BCM_CLK_ARM11_STR_ID		"arm11"
#define BCM_CLK_AHB_STR_ID			"ahb"
#define BCM_CLK_AHB_FAST_STR_ID		"ahb_fast"
#define BCM_CLK_I2S_INT_STR_ID		"i2s_int"
#define BCM_CLK_I2S_EXT_STR_ID		"i2s_ext"
#define BCM_CLK_DAM_STR_ID		"damck"
#define BCM_CLK_PDP_STR_ID		"pdpck"
#define BCM_CLK_SDIO1_STR_ID		"bcm_sdhc.1"
#define BCM_CLK_SDIO2_STR_ID		"bcm_sdhc.2"
#define BCM_CLK_SDIO3_STR_ID		"bcm_sdhc.3"
#define BCM_CLK_SDRAM_STR_ID		"sdram"
#define BCM_CLK_NVSRAM_STR_ID		"nvsram"
#define BCM_CLK_SPI0_STR_ID		"bcm_spi.0"
#define BCM_CLK_SPI1_STR_ID		"bcm_spi.1"
#define BCM_CLK_UARTA_STR_ID		"serial8250.0"
#define BCM_CLK_UARTB_STR_ID		"serial8250.1"
#define BCM_CLK_UARTC_STR_ID		"serial8250.2"
#define BCM_CLK_GP_STR_ID		"gp"
#define BCM_CLK_MSPRO_STR_ID		"mspro"
#define BCM_CLK_I2C1_STR_ID		"i2cbcm.0"
#define BCM_CLK_I2C2_STR_ID		"i2cbcm.1"
#define BCM_CLK_I2C3_STR_ID		"i2cbcm.2"
#define BCM_CLK_MIPIDSI_STR_ID		"mipidsi"
#define BCM_CLK_PWM_STR_ID		"pwm"
#define BCM_CLK_CAMERA_STR_ID		"cam"
#define BCM_CLK_CAMERARX_STR_ID		"camrx"
#define BCM_CLK_CAMERASYS_STR_ID	"camsys"
#define BCM_CLK_USB_STR_ID		"usb"
#define BCM_CLK_VCODEC_STR_ID		"vcodec"
#define BCM_CLK_VCODEC_POWER_STR_ID	"vcodec_pwr"
#define BCM_CLK_V3D_STR_ID		"v3d"
#define BCM_CLK_V3D_POWER_STR_ID	"v3d_pwr"
#define BCM_CLK_DMAC_STR_ID		"dmac"
#define BCM_CLK_RNG_STR_ID		"rng"
#define BCM_CLK_LCD_STR_ID		"lcd"
#define BCM_CLK_MPHI_STR_ID		"mphi"
#define BCM_CLK_USB_PHY_REF_STR_ID	"usb_phy_ref"
#define BCM_CLK_AUDIO_RX_ADC_STR_ID	"audio_rx_adc"
#define BCM_CLK_AUDIO_TX_DAC_STR_ID	"audio_tx_dac"
#define BCM_CLK_AUXADC_DAC_REF_STR_ID  "auxadc_dac_ref"
#define BCM_CLK_APPSPLL_EN_STR_ID	"appspll_en"
#define BCM_CLK_FSUSB_STR_ID		"fsusb"
#define BCM_CLK_CRYPTO_STR_ID		"crypto"

/* Athena Clock Ids */
enum {
	BCM21553_CLK_MAIN_PLL = 1,
	BCM21553_CLK_APPS_PLL,
	BCM21553_CLK_ARM11,
	BCM21553_CLK_AHB,
	BCM21553_CLK_AHB_FAST,
	BCM21553_CLK_I2S_INT,
	BCM21553_CLK_I2S_EXT,
	BCM21553_CLK_DAMCK,
	BCM21553_CLK_PDPCK,
	BCM21553_CLK_SDIO1,
	BCM21553_CLK_SDIO2,
	BCM21553_CLK_SDIO3,
	BCM21553_CLK_SDRAM,
	BCM21553_CLK_NVSRAM,
	BCM21553_CLK_SPI1,
	BCM21553_CLK_SPI2,
	BCM21553_CLK_UARTA,
	BCM21553_CLK_UARTB,
	BCM21553_CLK_UARTC,
	BCM21553_CLK_GP,
	BCM21553_CLK_MSPRO,
	BCM21553_CLK_I2C1,
	BCM21553_CLK_I2C2,
	BCM21553_CLK_I2C3,
	BCM21553_CLK_MIPIDSI,
	BCM21553_CLK_PWM,
	BCM21553_CLK_CAMERA,
	BCM21553_CLK_CAMRX,
	BCM21553_CLK_CAMSYS,
	BCM21553_CLK_USB,
	BCM21553_CLK_VCODEC,
	BCM21553_CLK_VCODEC_POWER,
	BCM21553_CLK_V3D,
	BCM21553_CLK_V3D_POWER,
	BCM21553_CLK_DMAC,
	BCM21553_CLK_RNG,
	BCM21553_CLK_LCD,
	BCM21553_CLK_MPHI,
	BCM21553_CLK_USB_PHY_REF,
	BCM21553_CLK_AUDIO_RX_ADC,
	BCM21553_CLK_AUDIO_TX_DAC,
	BCM21553_CLK_AUXADC_DAC_REF,
	BCM21553_CLK_APPSPLL_EN,
	BCM21553_CLK_FSUSB,
	BCM21553_CLK_CRYPTO,
	BCM21553_CLK_PEDESTAL_CTRL
};

/*MIPI DSI AFE DIV ids*/
enum
{
	CAM1_CLK_DIV,
	CAM2_CLK_DIV,
	DSI_CLK_DIV
};

extern int clk_set_mipidsi_afe_div(int id, int div);
extern int clk_get_mipidsi_afe_div(int id);


#endif /* __ARCH_ARM_MACH_BCM21553_CLOCK_H */
