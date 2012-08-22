 /*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/osdal_21553_clock.c
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <plat/clock.h>
#include <plat/types.h>
#include <plat/osdal_os.h>
#include <plat/osdal_os_service.h>
#include <asm/clkdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <mach/clkmgr.h>

#include <mach/reg_clkpwr.h>
#include <mach/reg_syscfg.h>

#define CLK_UNSUPPORTED  NULL

#define		FREQ_MHZ(mhz)		((mhz)*1000UL*1000UL)
#define		FREQ_KHZ(khz)		((khz)*1000UL)

/**
SYSCONFIG clock register map
*/
static const UInt32 sysconfig_clk_map[] =
{
	ADDR_SYSCFG_VIDEO_CODEC_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_VIDEO_CODEC
	ADDR_SYSCFG_CAMARA_INTERFACE_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_CAMARA_IF
	ADDR_SYSCFG_USB_AHB_CLK_EN,  //OSDAL_SYSCLK_ENABLE_USB
	ADDR_SYSCFG_CRYPTO_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_CRYPTO
	ADDR_SYSCFG_PKA_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_PKA
	ADDR_SYSCFG_UARTA_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_UARTA
	ADDR_SYSCFG_UARTB_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_UARTB
	ADDR_SYSCFG_DA_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_DA
	ADDR_SYSCFG_LCD_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_LCD
	/* OSDAL_SYSCLK_ENABLE_DPE needs to be changed to
	 * OSDAL_SYSCLK_ENABLE_V3D once all RTOS drivers are migrated
	 * to Linux
	 */
	ADDR_SYSCFG_V3D_AHB_CLK_EN, /* OSDAL_SYSCLK_ENABLE_DPE */
    ADDR_SYSCFG_DMAC_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_DMAC
    ADDR_SYSCFG_SDIO1_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_SDIO1
    ADDR_SYSCFG_SDIO2_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_SDIO2
    ADDR_SYSCFG_DES_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_DES
    ADDR_SYSCFG_UARTC_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_UARTC
    ADDR_SYSCFG_RNG_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_RNG
    ADDR_SYSCFG_SDIO3_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_SDIO3
    ADDR_SYSCFG_FSUSBHOST_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_FSUSBHOST
    ADDR_SYSCFG_MPHI_AHB_CLK_EN, //OSDAL_SYSCLK_ENABLE_MPHI
    ADDR_SYSCFG_DMAC_AHB_CLK_MODE //OSDAL_SYSCLK_ENABLE_DMAC_MODE
};

/*OSDAL clock id to Linux clk str id map*/
static const char* clk_map[] =
{
	BCM_CLK_CAMERA_STR_ID, //OSDAL_CLK_CAM
    CLK_UNSUPPORTED, //OSDAL_CLK_DSP
    BCM_CLK_I2S_INT_STR_ID, //OSDAL_CLK_I2S
    BCM_CLK_I2C1_STR_ID, //OSDAL_CLK_I2C1
    BCM_CLK_I2C2_STR_ID, //OSDAL_CLK_I2C2
    BCM_CLK_DAM_STR_ID, //OSDAL_CLK_DAM
    BCM_CLK_SDIO1_STR_ID, //OSDAL_CLK_SDIO1
    BCM_CLK_SDIO2_STR_ID, //OSDAL_CLK_SDIO2
    BCM_CLK_SPI0_STR_ID, //OSDAL_CLK_SPI
    BCM_CLK_UARTA_STR_ID, //OSDAL_CLK_UARTA
    BCM_CLK_UARTB_STR_ID, //OSDAL_CLK_UARTB
    BCM_CLK_UARTC_STR_ID, //OSDAL_CLK_UARTC
    BCM_CLK_USB_STR_ID, //OSDAL_CLK_USBPLL_ENABLE
    BCM_CLK_USB_STR_ID, //OSDAL_CLK_USBPLL_OEN
    BCM_CLK_MIPIDSI_STR_ID, //OSDAL_CLK_MIPIDSI_CMI
    CLK_UNSUPPORTED, //OSDAL_CLK_MIPIDSI_AFE
    CLK_UNSUPPORTED, //OSDAL_CLK_ISP
    BCM_CLK_SDIO3_STR_ID, //OSDAL_CLK_SDIO3
    BCM_CLK_USB_STR_ID, //OSDAL_CLK_USB48
    CLK_UNSUPPORTED, //OSDAL_CLK_TVOUT_PLL
    CLK_UNSUPPORTED, //OSDAL_CLK_TVOUT_PLL_CHANS
    BCM_CLK_CAMERARX_STR_ID, //OSDAL_CLK_CAMINTF_CAMRX
    BCM_CLK_CAMERASYS_STR_ID, //OSDAL_CLK_CAMINTF_CAMSYS
    BCM_CLK_AUDIO_RX_ADC_STR_ID, //OSDAL_CLK_AUDIO_RX_ADC_CLOCK
    BCM_CLK_AUXADC_DAC_REF_STR_ID, //OSDAL_CLK_AUXADC_DAC_REF_CLOCK
    BCM_CLK_USB_PHY_REF_STR_ID, //OSDAL_CLK_USB_PHY_REF_CLOCK,
    BCM_CLK_AUDIO_TX_DAC_STR_ID, //OSDAL_CLK_AUDIO_TX_DAC_CLOCK
};

/**
*
*  This function to enable/gate clock for a peripheral(Obsolete in new chips)
*
*  @param		clk (in) peripheral clock to enable/gate
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_SYSCLK_Enable(OSDAL_SYSCLK_ENABLE_SELECT clk)
{
	UInt32 sys_clk_reg;
	/*Make sure that clk id is valid*/
	if( clk >= OSDAL_SYSCLK_ENABLE_VIDEO_CODEC && clk <= OSDAL_SYSCLK_ENABLE_DMAC_MODE)
	{
		sys_clk_reg = sysconfig_clk_map[clk-OSDAL_SYSCLK_ENABLE_VIDEO_CODEC];
		writel(0x01,sys_clk_reg);
		return OSDAL_ERR_OK;
	}
	return OSDAL_ERR_UNSUPPORTED;
}
EXPORT_SYMBOL(OSDAL_SYSCLK_Enable);

/**
*
*  This function to disable/ungate clock for a peripheral(Obsolete in new chips)
*
*  @param		clk (in) peripheral clock to disable/ungate
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_SYSCLK_Disable(OSDAL_SYSCLK_ENABLE_SELECT clk)
{
	UInt32 sys_clk_reg;
	/*Make sure that clk id is valid*/
	if( clk >= OSDAL_SYSCLK_ENABLE_VIDEO_CODEC && clk <= OSDAL_SYSCLK_ENABLE_DMAC_MODE)
	{
		sys_clk_reg = sysconfig_clk_map[clk-OSDAL_SYSCLK_ENABLE_VIDEO_CODEC];
		writel(0x0,sys_clk_reg);
		return OSDAL_ERR_OK;
	}
	return OSDAL_ERR_UNSUPPORTED;
}
EXPORT_SYMBOL(OSDAL_SYSCLK_Disable);

/**
*
*  This function to open a clock
*
*  @param		clk (in) clock to open
*
*  @return	    handle to be used later
*
*****************************************************************************/
OSDAL_CLK_HANDLE OSDAL_CLK_Open(OSDAL_CLK_SELECT clk)
{
	const char* clk_id;
	struct clk* lnx_clk;
	if(clk >= OSDAL_CLK_CAM && clk <= OSDAL_CLK_AUDIO_TX_DAC_CLOCK)
	{
		clk_id = clk_map[clk-OSDAL_CLK_CAM];
		if(clk_id)
		{
			lnx_clk =clk_get(NULL,clk_id);
			return  IS_ERR(lnx_clk) ? NULL : lnx_clk;
		}
	}
	return NULL;
}
EXPORT_SYMBOL(OSDAL_CLK_Open);

/**
*
*  This function to close a clock
*
*  @param		handle (in) clock to close
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status  OSDAL_CLK_Close(OSDAL_CLK_HANDLE handle)
{
	if(handle)
	{
		struct clk* clk = (struct clk*)handle;
		clk_put(clk);
		return OSDAL_ERR_OK;
	}
	return OSDAL_ERR_INVAL;
}
EXPORT_SYMBOL(OSDAL_CLK_Close);

/**
*
*  This function to configue a clock generation
*
*  @param		handle (in) clock to configure
*  		        speed (in) clock speed
*  		        cfg_data (in) clock specific config parameters
*
*  @return	    OSDAL return status
*
*****************************************************************************/

OSDAL_Status OSDAL_CLK_Set_Config(OSDAL_CLK_HANDLE handle, OSDAL_CLK_SPEED speed, void *cfg_data)
{
	int ret = -1;
	u32 bcm_21553_clk_speed = 0;
	struct clk* clk = (struct clk*)handle;

	if(!handle)
	    return OSDAL_ERR_INVAL;

	switch (clk->id) {
	case BCM21553_CLK_CAMERA:
		switch (speed) {
		case OSDAL_CLK_CAM_12MHZ:
		bcm_21553_clk_speed = FREQ_MHZ(12);
		break;
		case OSDAL_CLK_CAM_13MHZ:
		bcm_21553_clk_speed = FREQ_MHZ(13);
		break;
		case OSDAL_CLK_CAM_24MHZ:
		bcm_21553_clk_speed = FREQ_MHZ(24);
		break;
		case OSDAL_CLK_CAM_26MHZ:
		bcm_21553_clk_speed = FREQ_MHZ(26);
		break;
		case OSDAL_CLK_CAM_48MHZ:
		bcm_21553_clk_speed = FREQ_MHZ(48);
		break;

		default:
			break;
		}
	break;
	case BCM21553_CLK_CAMRX:
		switch (speed) {
		case OSDAL_CLK_CAMINTF_CAMRX_104MHZ:
		bcm_21553_clk_speed = FREQ_MHZ(104);
		break;
		case OSDAL_CLK_CAMINTF_CAMRX_156MHZ:
		bcm_21553_clk_speed = FREQ_MHZ(156);
		break;
		case OSDAL_CLK_CAMINTF_CAMRX_208MHZ:
		bcm_21553_clk_speed = FREQ_MHZ(208);
		break;

		default:
			break;
		}
	break;
	case BCM21553_CLK_I2S_INT:
		switch (speed) {
		case OSDAL_CLK_I2S_INT_12_0M:
			bcm_21553_clk_speed = FREQ_MHZ(12);
			break;
		case OSDAL_CLK_I2S_INT_2_4M:
			bcm_21553_clk_speed = FREQ_MHZ(2.4);
			break;
		case OSDAL_CLK_I2S_INT_2_048M:
			bcm_21553_clk_speed = FREQ_MHZ(2.048);
			break;
		case OSDAL_CLK_I2S_INT_1_536M:
			bcm_21553_clk_speed = FREQ_MHZ(1.536);
			break;
		case OSDAL_CLK_I2S_INT_512K:
			bcm_21553_clk_speed = FREQ_KHZ(512);
			break;
		case OSDAL_CLK_I2S_INT_256K:
			bcm_21553_clk_speed = FREQ_KHZ(256);
			break;
		case OSDAL_CLK_I2S_INT_128K:
			bcm_21553_clk_speed = FREQ_KHZ(128);
			break;
		default:
			break;

		}
	break;
	case BCM21553_CLK_I2C1:
	case BCM21553_CLK_I2C2:
	case BCM21553_CLK_I2C3:
		switch (speed) {
		case OSDAL_CLK_I2C_13MHZ:
			bcm_21553_clk_speed = FREQ_MHZ(13);
			break;
		case OSDAL_CLK_I2C_104MHZ:
			bcm_21553_clk_speed = FREQ_MHZ(104);
			break;
		default:
			break;
		}
	break;

	default:
		pr_info("Unsupported clock ID\n");
		return OSDAL_ERR_UNSUPPORTED;
	}
	if (bcm_21553_clk_speed != 0)
	    ret = clk_set_rate(clk, bcm_21553_clk_speed);
	else
	    return OSDAL_ERR_INVAL;

	if(!ret)
	    return OSDAL_ERR_OK;
	else
	    return OSDAL_ERR_OTHER;
}
EXPORT_SYMBOL(OSDAL_CLK_Set_Config);

/**
*
*  This function to start a clock generation
*
*  @param		handle (in) clock to start
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_CLK_Start(OSDAL_CLK_HANDLE handle)
{
	if(handle)
	{
		struct clk* clk = (struct clk*)handle;
		return (clk_enable(clk) == 0) ? OSDAL_ERR_OK: OSDAL_ERR_INVAL;
	}
	return OSDAL_ERR_INVAL;
}
EXPORT_SYMBOL(OSDAL_CLK_Start);

/**
*
*  This function to stop a clock generation
*
*  @param		handle (in) clock to stop
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_CLK_Stop(OSDAL_CLK_HANDLE handle)
{
	if(handle)
	{
		struct clk* clk = (struct clk*)handle;
		clk_disable(clk);
		return OSDAL_ERR_OK;
	}
	return OSDAL_ERR_INVAL;
}
EXPORT_SYMBOL(OSDAL_CLK_Stop);
