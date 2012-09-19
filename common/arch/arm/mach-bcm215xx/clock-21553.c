/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/clock-21553.c
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
#include <asm/clkdev.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <mach/clkmgr.h>

#include <mach/reg_clkpwr.h>
#include <mach/reg_syscfg.h>
#include <mach/reg_emi.h>


#define BCM_CLK_NO_DISABLE_ON_INIT (1 << 15)

#define MAX_UART_N_VAL 0x1FF
#define MAX_UART_M_VAL 0x1FF
#define MULTIPLICATION_FACTOR 10

#define BRCM_CLK_PTR(_name) (&BRCM_CLK_NAME(_name))
#define BRCM_CLK_NAME(_name) clk_##_name
#define DEFINE_BRCM_CLK(_name, _id, _parent, _flgs, _enable_reg, _enable_bit_mask) \
	static struct clk BRCM_CLK_NAME(_name) = \
	{\
		.id = BCM21553_CLK_##_id,\
		.flags = _flgs,\
		.enable = bcm21553_##_name##_enable,\
		.disable = bcm21553_##_name##_disable,\
		.set_rate = bcm21553_##_name##_set_rate,\
		.get_rate = bcm21553_##_name##_get_rate,\
		.round_rate = bcm21553_##_name##_round_rate,\
		.parent = _parent,\
		.enable_bit_mask = _enable_bit_mask,\
		.enable_reg = (void __iomem *)_enable_reg,\
	}

#define DEFINE_BRCM_CLK_SIMPLE(_name, _id, _parent, _flgs, _enable_reg, _enable_bit_mask) \
	static struct clk BRCM_CLK_NAME(_name) = \
	{\
		.id = BCM21553_CLK_##_id,\
		.flags = _flgs,\
		.parent = _parent,\
		.enable_bit_mask = _enable_bit_mask,\
		.enable_reg = (void __iomem *)_enable_reg,\
	}

#define BRCM_REGISTER_CLK(con, dev, clock)	\
	{\
		.con_id = con,\
		.dev_id = dev,\
		.clk = &BRCM_CLK_NAME(clock),\
	}

#define FREQ_MHZ(mhz)		((mhz)*1000UL*1000UL)
#define FREQ_KHZ(khz)		((khz)*1000UL)

extern void bcm215xx_set_armahb_mode(u32 mode);
extern void bcm215xx_set_appll_enable(u32 enable);


static struct proc_dir_entry *brcm_proc_file;

/*ARM11 clock */
#define bcm21553_arm11_enable 		NULL
#define bcm21553_arm11_disable 		NULL
#define bcm21553_arm11_set_parent 	NULL
static unsigned long bcm21553_arm11_get_rate(struct clk *clk);
static int bcm21553_arm11_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_arm11_round_rate(struct clk *clk,
				      unsigned long desired_val);

/*AHB*/
#define bcm21553_ahb_enable 		NULL
#define bcm21553_ahb_disable  	NULL
#define bcm21553_ahb_set_rate     NULL
#define bcm21553_ahb_round_rate   NULL
static unsigned long bcm21553_ahb_get_rate(struct clk *clk);

/*AHB fast*/
#define bcm21553_ahb_fast_enable 		NULL
#define bcm21553_ahb_fast_disable  	NULL
#define bcm21553_ahb_fast_set_rate     NULL
#define bcm21553_ahb_fast_round_rate   NULL
static unsigned long bcm21553_ahb_fast_get_rate(struct clk *clk);


/* CAM clock */
static void bcm21553_cam_disable(struct clk *clk);
static int bcm21553_cam_enable(struct clk *clk);
static unsigned long bcm21553_cam_get_rate(struct clk *clk);
static int bcm21553_cam_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_cam_round_rate(struct clk *clk, unsigned long desired_val);

/*I2S Int clk */
#define bcm21553_i2s_int_enable 		NULL
#define bcm21553_i2s_int_disable 		NULL
static unsigned long bcm21553_i2s_int_get_rate(struct clk *clk);
static int bcm21553_i2s_int_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_i2s_int_round_rate(struct clk *clk,
					unsigned long desired_val);

/*DAM clk */
#define bcm21553_damck_get_rate 	NULL
#define bcm21553_damck_set_rate		NULL
#define bcm21553_damck_round_rate	NULL
static int bcm21553_damck_enable(struct clk *clk);
static void bcm21553_damck_disable(struct clk *clk);

/*PDP clk */
#define bcm21553_pdpck_enable 		NULL
#define bcm21553_pdpck_disable 		NULL
static unsigned long bcm21553_pdpck_get_rate(struct clk *clk);
static int bcm21553_pdpck_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_pdpck_round_rate(struct clk *clk,
				      unsigned long desired_val);

/*SDIO1 clk*/
#define bcm21553_sdio1_enable 	bcm21553_sdio_enable
#define bcm21553_sdio1_disable  bcm21553_sdio_disable
#define bcm21553_sdio1_get_rate	bcm21553_sdio_get_rate
#define bcm21553_sdio1_set_rate bcm21553_sdio_set_rate
#define bcm21553_sdio1_round_rate bcm21553_sdio_round_rate

/*SDIO2 clk*/
#define bcm21553_sdio2_enable 	bcm21553_sdio_enable
#define bcm21553_sdio2_disable  bcm21553_sdio_disable
#define bcm21553_sdio2_get_rate	bcm21553_sdio_get_rate
#define bcm21553_sdio2_set_rate bcm21553_sdio_set_rate
#define bcm21553_sdio2_round_rate bcm21553_sdio_round_rate

/*SDIO3 clk*/
#define bcm21553_sdio3_enable 	bcm21553_sdio_enable
#define bcm21553_sdio3_disable  bcm21553_sdio_disable
#define bcm21553_sdio3_get_rate	bcm21553_sdio_get_rate
#define bcm21553_sdio3_set_rate bcm21553_sdio_set_rate
#define bcm21553_sdio3_round_rate bcm21553_sdio_round_rate

static int bcm21553_sdio_enable(struct clk *clk);
static void bcm21553_sdio_disable(struct clk *clk);
static unsigned long bcm21553_sdio_get_rate(struct clk *clk);
static int bcm21553_sdio_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_sdio_round_rate(struct clk *clk,
				     unsigned long desired_val);

/*SDRAM clk*/
#define bcm21553_sdram_enable 		NULL
#define bcm21553_sdram_disable  	NULL
#define bcm21553_sdram_set_rate     NULL
#define bcm21553_sdram_round_rate   NULL
static unsigned long bcm21553_sdram_get_rate(struct clk *clk);

/*NVSRAM clk*/
#define bcm21553_nvsram_enable 		NULL
#define bcm21553_nvsram_disable  	NULL
#define bcm21553_nvsram_set_rate     NULL
#define bcm21553_nvsram_round_rate   NULL
static unsigned long bcm21553_nvsram_get_rate(struct clk *clk);

/*SPI1 clk */
#define bcm21553_spi1_enable 		NULL
#define bcm21553_spi1_disable  		NULL
#define bcm21553_spi1_get_rate		bcm21553_spi_get_rate
#define bcm21553_spi1_set_rate		bcm21553_spi_set_rate
#define bcm21553_spi1_round_rate	bcm21553_spi_round_rate

/*SPI2 clk */
#define bcm21553_spi2_enable 		NULL
#define bcm21553_spi2_disable  		NULL
#define bcm21553_spi2_get_rate		bcm21553_spi_get_rate
#define bcm21553_spi2_set_rate		bcm21553_spi_set_rate
#define bcm21553_spi2_round_rate	bcm21553_spi_round_rate

static unsigned long bcm21553_spi_get_rate(struct clk *clk);
static int bcm21553_spi_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_spi_round_rate(struct clk *clk, unsigned long desired_val);

/* UARTA clk */
#define bcm21553_uarta_enable 	bcm21553_uart_enable
#define bcm21553_uarta_disable  bcm21553_uart_disable
#define bcm21553_uarta_get_rate	bcm21553_uart_get_rate
#define bcm21553_uarta_set_rate bcm21553_uart_set_rate
#define bcm21553_uarta_round_rate bcm21553_uart_round_rate

/* UARTB clk */
#define bcm21553_uartb_enable 		bcm21553_uart_enable
#define bcm21553_uartb_disable  	bcm21553_uart_disable
#define bcm21553_uartb_get_rate	bcm21553_uart_get_rate
#define bcm21553_uartb_set_rate bcm21553_uart_set_rate
#define bcm21553_uartb_round_rate bcm21553_uart_round_rate

/* UARTC clk */
#define bcm21553_uartc_enable 	bcm21553_uart_enable
#define bcm21553_uartc_disable  bcm21553_uart_disable
#define bcm21553_uartc_get_rate	bcm21553_uart_get_rate
#define bcm21553_uartc_set_rate bcm21553_uart_set_rate
#define bcm21553_uartc_round_rate bcm21553_uart_round_rate

static int bcm21553_uart_enable(struct clk *clk);
static void bcm21553_uart_disable(struct clk *clk);
static unsigned long bcm21553_uart_get_rate(struct clk *clk);
static int bcm21553_uart_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_uart_round_rate(struct clk *clk,
				     unsigned long desired_val);

/*GP clk*/
#define bcm21553_gp_enable 	NULL
#define bcm21553_gp_disable  NULL
static unsigned long bcm21553_gp_get_rate(struct clk *clk);
static int bcm21553_gp_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_gp_round_rate(struct clk *clk, unsigned long desired_val);

/*MSPRO clk*/
#define bcm21553_mspro_enable 	NULL
#define bcm21553_mspro_disable  NULL
static unsigned long bcm21553_mspro_get_rate(struct clk *clk);
static int bcm21553_mspro_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_mspro_round_rate(struct clk *clk,
				      unsigned long desired_val);

/*USB Clk*/
#define bcm21553_usb_get_rate		NULL
#define bcm21553_usb_set_rate		NULL
#define bcm21553_usb_round_rate		NULL
static void bcm21553_usb_disable(struct clk *clk);
static int bcm21553_usb_enable(struct clk *clk);

 /*MIPIDSI*/
static int bcm21553_mipidsi_enable(struct clk *clk);
static void bcm21553_mipidsi_disable(struct clk *clk);
static unsigned long bcm21553_mipidsi_get_rate(struct clk *clk);
static int bcm21553_mipidsi_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_mipidsi_round_rate(struct clk *clk,
					unsigned long desired_val);

/* I2C1 clk */
#define bcm21553_i2c1_enable 	NULL
#define bcm21553_i2c1_disable  	NULL
#define bcm21553_i2c1_get_rate	bcm21553_i2c_get_rate
#define bcm21553_i2c1_set_rate bcm21553_i2c_set_rate
#define bcm21553_i2c1_round_rate bcm21553_i2c_round_rate

/* I2C2 clk */
#define bcm21553_i2c2_enable 	NULL
#define bcm21553_i2c2_disable  	NULL
#define bcm21553_i2c2_get_rate	bcm21553_i2c_get_rate
#define bcm21553_i2c2_set_rate bcm21553_i2c_set_rate
#define bcm21553_i2c2_round_rate bcm21553_i2c_round_rate

/* I2C3 clk */
#define bcm21553_i2c3_enable	NULL
#define bcm21553_i2c3_disable	NULL
#define bcm21553_i2c3_get_rate	bcm21553_i2c_get_rate
#define bcm21553_i2c3_set_rate	bcm21553_i2c_set_rate
#define bcm21553_i2c3_round_rate	bcm21553_i2c_round_rate

static unsigned long bcm21553_i2c_get_rate(struct clk *clk);
static int bcm21553_i2c_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_i2c_round_rate(struct clk *clk, unsigned long desired_val);

/*CAMRX clk */
#define bcm21553_camrx_enable 	NULL
#define bcm21553_camrx_disable  NULL
static unsigned long bcm21553_camrx_get_rate(struct clk *clk);
static int bcm21553_camrx_set_rate(struct clk *clk, unsigned long val);
static long bcm21553_camrx_round_rate(struct clk *clk,
				      unsigned long desired_val);

/*v3d_power  Clk*/
#define bcm21553_v3d_power_get_rate		NULL
#define bcm21553_v3d_power_set_rate		NULL
#define bcm21553_v3d_power_round_rate		NULL
static void bcm21553_v3d_power_disable(struct clk *clk);
static int bcm21553_v3d_power_enable(struct clk *clk);

/*vcodec_power clk*/
#define bcm21553_vcodec_power_get_rate		NULL
#define bcm21553_vcodec_power_set_rate		NULL
#define bcm21553_vcodec_power_round_rate		NULL
static void bcm21553_vcodec_power_disable(struct clk *clk);
static int bcm21553_vcodec_power_enable(struct clk *clk);

/*Apps PLL enable clk*/
#define bcm21553_appspll_en_get_rate		NULL
#define bcm21553_appspll_en_set_rate		NULL
#define bcm21553_appspll_en_round_rate		NULL
static void bcm21553_appspll_en_disable(struct clk *clk);
static int bcm21553_appspll_en_enable(struct clk *clk);

/* LCD clk */
#define bcm21553_lcd_get_rate			NULL
#define bcm21553_lcd_set_rate			NULL
#define bcm21553_lcd_round_rate			NULL
static void bcm21553_lcd_disable(struct clk *clk);
static int bcm21553_lcd_enable(struct clk *clk);

static u32 bcm21553_generic_round_rate(u32 desired_val,
	const u32 *supportedFreqList, u8 count);
static long bcm21553_uart_find_m_n(unsigned long freq_val, long *m_set,
				   long *n_set);
/*Pedestal mode control flag */
static u32 pedestal_ctrl_flag;
/* Define clocks */
static struct clk clk_mainpll = {
	.id = BCM21553_CLK_MAIN_PLL,
	.flags = BCM_CLK_ALWAYS_ENABLED,	/* can't be disabled thro' s/w */
};

static struct clk clk_appspll = {
	.id = BCM21553_CLK_APPS_PLL,
	.flags = BCM_CLK_ALWAYS_ENABLED,	/* can't be disabled thro' s/w */
};

DEFINE_BRCM_CLK_SIMPLE(pedestal_ctrl, PEDESTAL_CTRL, BRCM_CLK_PTR(mainpll), 0,
		       &pedestal_ctrl_flag, 0x01);
DEFINE_BRCM_CLK(arm11, ARM11, BRCM_CLK_PTR(appspll), BCM_CLK_ALWAYS_ENABLED, 0, 0);
DEFINE_BRCM_CLK(ahb, AHB, BRCM_CLK_PTR(appspll), BCM_CLK_ALWAYS_ENABLED, 0, 0);
DEFINE_BRCM_CLK(ahb_fast, AHB_FAST, BRCM_CLK_PTR(appspll), BCM_CLK_ALWAYS_ENABLED, 0, 0);

DEFINE_BRCM_CLK(cam, CAMERA, BRCM_CLK_PTR(pedestal_ctrl), 0, ADDR_CLKPWR_CLK_CAMCK_ENABLE,
		0x01);
DEFINE_BRCM_CLK(i2s_int, I2S_INT, BRCM_CLK_PTR(pedestal_ctrl), 0,
		ADDR_CLKPWR_CLK_I2S_INT_ENABLE, 0x01);
DEFINE_BRCM_CLK_SIMPLE(i2s_ext, I2S_EXT, BRCM_CLK_PTR(mainpll), 0,
		       ADDR_CLKPWR_CLK_I2S_EXT_ENABLE, 0x01);
DEFINE_BRCM_CLK(damck, DAMCK, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_DAMCK_ENABLE,
		0x01);
DEFINE_BRCM_CLK(pdpck, PDPCK, BRCM_CLK_PTR(pedestal_ctrl), 0, ADDR_CLKPWR_CLK_PDPCK_ENABLE,
		0x01);
DEFINE_BRCM_CLK(sdio1, SDIO1, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_SDIO1_ENABLE,
		0x01);
DEFINE_BRCM_CLK(sdio2, SDIO2, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_SDIO2_ENABLE,
		0x01);
DEFINE_BRCM_CLK(sdio3, SDIO3, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_SDIO3_ENABLE,
		0x01);
DEFINE_BRCM_CLK(sdram, SDRAM, BRCM_CLK_PTR(mainpll), BCM_CLK_ALWAYS_ENABLED, 0, 0);
DEFINE_BRCM_CLK(nvsram, NVSRAM, BRCM_CLK_PTR(mainpll), BCM_CLK_ALWAYS_ENABLED, 0, 0);
DEFINE_BRCM_CLK(spi1, SPI1, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_SPI_ENABLE, 0x01);
DEFINE_BRCM_CLK(spi2, SPI2, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_SPI2_ENABLE,
		0x01);

#if defined(CONFIG_DEBUG_LL)
	DEFINE_BRCM_CLK(uarta, UARTA, BRCM_CLK_PTR(mainpll), BCM_CLK_NO_DISABLE_ON_INIT,
               ADDR_CLKPWR_CLK_UARTA_ENABLE, 0x01);
#else
	DEFINE_BRCM_CLK(uarta, UARTA, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_UARTA_ENABLE, 0x01);
#endif

DEFINE_BRCM_CLK(uartb, UARTB, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_UARTB_ENABLE, 0x01);
DEFINE_BRCM_CLK(uartc, UARTC, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_UARTC_ENABLE, 0x01);
DEFINE_BRCM_CLK(gp, GP, BRCM_CLK_PTR(pedestal_ctrl), 0, ADDR_CLKPWR_CLK_GPCK_ENABLE, 0x01);
DEFINE_BRCM_CLK(mspro, MSPRO, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_MSPRO_ENABLE,
		0x01);
DEFINE_BRCM_CLK(usb, USB, BRCM_CLK_PTR(mainpll), BCM_CLK_NO_DISABLE_ON_INIT, 0, 0);
DEFINE_BRCM_CLK(mipidsi, MIPIDSI, BRCM_CLK_PTR(pedestal_ctrl), 0,
		ADDR_CLKPWR_CLK_MIPIDSI_CMI_CLK_EN, 0x01);
DEFINE_BRCM_CLK(i2c1, I2C1, BRCM_CLK_PTR(pedestal_ctrl), 0, ADDR_CLKPWR_CLK_HSBSC_ENABLE,
		0x01);
DEFINE_BRCM_CLK(i2c2, I2C2, BRCM_CLK_PTR(pedestal_ctrl), 0, ADDR_CLKPWR_CLK_HSBSC2_ENABLE,
		0x01);
DEFINE_BRCM_CLK(i2c3, I2C3, BRCM_CLK_PTR(pedestal_ctrl), 0, ADDR_CLKPWR_CLK_HSBSC3_ENABLE,
		0x01);
DEFINE_BRCM_CLK_SIMPLE(pwm, PWM, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_PWM_ENABLE, 0x01);
DEFINE_BRCM_CLK_SIMPLE(camsys, CAMSYS, BRCM_CLK_PTR(mainpll), 0,
		       ADDR_CLKPWR_CLK_CAMINTF_ENABLE, 0x01);
DEFINE_BRCM_CLK(camrx, CAMRX, BRCM_CLK_PTR(mainpll), 0, ADDR_CLKPWR_CLK_CAMINTF_ENABLE,
		0x02);
DEFINE_BRCM_CLK_SIMPLE(vcodec, VCODEC, BRCM_CLK_PTR(pedestal_ctrl),
		       0, ADDR_SYSCFG_VIDEO_CODEC_AHB_CLK_EN, 0x01);
DEFINE_BRCM_CLK_SIMPLE(dmac, DMAC, BRCM_CLK_PTR(mainpll), 0, ADDR_SYSCFG_DMAC_AHB_CLK_EN,
		       0x01);
DEFINE_BRCM_CLK(lcd, LCD, BRCM_CLK_PTR(mainpll), BCM_CLK_NO_DISABLE_ON_INIT,
		       ADDR_SYSCFG_LCD_AHB_CLK_EN, 0x01);
/*RNG AHB clk should be released by s/w*/
DEFINE_BRCM_CLK_SIMPLE(rng, RNG, BRCM_CLK_PTR(mainpll), 0, ADDR_SYSCFG_RNG_AHB_CLK_EN,
		       0x01);
DEFINE_BRCM_CLK_SIMPLE(mphi, MPHI, BRCM_CLK_PTR(mainpll), 0,
		       ADDR_SYSCFG_MPHI_AHB_CLK_EN, 0x01);
DEFINE_BRCM_CLK(vcodec_power, VCODEC_POWER, NULL,
		       BCM_CLK_INVERT_ENABLE /*| BCM_CLK_NO_DISABLE_ON_INIT*/,
		       ADDR_CLKPWR_CLK_VIDEO_CODEC_PWROFF, 0x01);
DEFINE_BRCM_CLK_SIMPLE(v3d, V3D, BRCM_CLK_PTR(pedestal_ctrl), 0, ADDR_SYSCFG_V3D_AHB_CLK_EN,
	0x01);
DEFINE_BRCM_CLK(v3d_power, V3D_POWER, NULL,
	BCM_CLK_INVERT_ENABLE, ADDR_CLKPWR_CLK_V3D_POWEROFF, 0x01);
DEFINE_BRCM_CLK_SIMPLE(usb_phy_ref, USB_PHY_REF, BRCM_CLK_PTR(mainpll),
		       BCM_CLK_INVERT_ENABLE,
		       ADDR_CLKPWR_CLK_ANALOG_PHASES_ENABLE, USB_PHY_REF_CLK);
DEFINE_BRCM_CLK_SIMPLE(audio_rx_adc, AUDIO_RX_ADC, BRCM_CLK_PTR(mainpll),
		       BCM_CLK_INVERT_ENABLE,
		       ADDR_CLKPWR_CLK_ANALOG_PHASES_ENABLE, AUDIO_RX_ADC_CLK);
DEFINE_BRCM_CLK_SIMPLE(audio_tx_dac, AUDIO_TX_DAC, BRCM_CLK_PTR(mainpll),
		       BCM_CLK_INVERT_ENABLE,
		       ADDR_CLKPWR_CLK_ANALOG_PHASES_ENABLE, AUDIO_TX_DAC_CLK);
DEFINE_BRCM_CLK_SIMPLE(auxadc_dac_ref, AUXADC_DAC_REF, BRCM_CLK_PTR(mainpll),
		       BCM_CLK_INVERT_ENABLE,
		       ADDR_CLKPWR_CLK_ANALOG_PHASES_ENABLE,
		       AUXADC_DAC_REF_CLK);
DEFINE_BRCM_CLK(appspll_en, APPSPLL_EN, NULL,
			0,ADDR_CLKPWR_CLK_APPSPLL_ENABLE, 0x01);

DEFINE_BRCM_CLK_SIMPLE(fsusb,FSUSB, BRCM_CLK_PTR(mainpll),
		       0,ADDR_SYSCFG_FSUSBHOST_AHB_CLK_EN,
		       0x01);
DEFINE_BRCM_CLK_SIMPLE(crypto, CRYPTO, BRCM_CLK_PTR(mainpll), 0,
		       ADDR_SYSCFG_CRYPTO_AHB_CLK_EN, 0x01);

static u32 clk_armahb_reg_to_arm11_freq_mapping[] = {
	FREQ_MHZ(52),
	FREQ_MHZ(156),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(156),
	FREQ_MHZ(13),
	FREQ_MHZ(26),
	FREQ_MHZ(312),
	FREQ_MHZ(312),
	FREQ_MHZ(156),
	FREQ_MHZ(156),
	FREQ_MHZ(312),
	FREQ_MHZ(312),
	FREQ_MHZ(468),
	FREQ_MHZ(468),
	FREQ_MHZ(624)
};

static u32 clk_armahb_reg_to_cp_freq_mapping[] = {

	FREQ_MHZ(52),
	FREQ_MHZ(78),
	FREQ_MHZ(52),
	FREQ_MHZ(104),
	FREQ_MHZ(156),
	FREQ_MHZ(13),
	FREQ_MHZ(26),
	FREQ_MHZ(104),
	FREQ_MHZ(312),
	FREQ_MHZ(156),
	FREQ_MHZ(312),
	FREQ_MHZ(156),
	FREQ_MHZ(312),
	FREQ_MHZ(156),
	FREQ_MHZ(312),
	FREQ_MHZ(312)
};

static u32 clk_armahb_reg_to_ahb_fast_freq_mapping[] = {
	FREQ_MHZ(52),
	FREQ_MHZ(78),
	FREQ_MHZ(52),
	FREQ_MHZ(104),
	FREQ_MHZ(52),
	FREQ_MHZ(13),
	FREQ_MHZ(26),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(156),
	FREQ_MHZ(156),
	FREQ_MHZ(156),
	FREQ_MHZ(156),
	FREQ_MHZ(156),
	FREQ_MHZ(156),
	FREQ_MHZ(156)
};

static u32 clk_armahb_reg_to_ahb_freq_mapping[] = {
	FREQ_MHZ(52),
	FREQ_MHZ(78),
	FREQ_MHZ(52),
	FREQ_MHZ(104),
	FREQ_MHZ(52),
	FREQ_MHZ(13),
	FREQ_MHZ(26),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(104)
};

static u32 clk_armahb_reg_to_v3d_clk_mapping[] = {

	FREQ_MHZ(52),
	FREQ_MHZ(78),
	FREQ_MHZ(52),
	FREQ_MHZ(104),
	FREQ_MHZ(52),
	FREQ_MHZ(13),
	FREQ_MHZ(26),
	FREQ_MHZ(104),
	FREQ_MHZ(104),
	FREQ_MHZ(156),
	FREQ_MHZ(156),
	FREQ_MHZ(156),
	FREQ_MHZ(156),
	FREQ_MHZ(234),
	FREQ_MHZ(234),
	FREQ_MHZ(234)
};

static u32 clk_sdram_mode_reg_to_appspll_divider[] = {
	8, 8, 7, 6,
};

static u32 cam_freq_list[] = {
	FREQ_MHZ(12), FREQ_MHZ(13), FREQ_MHZ(24), FREQ_MHZ(26), FREQ_MHZ(48)
};

static u32 i2s_int_freq_list[] = {
	FREQ_MHZ(12), FREQ_MHZ(2.4), FREQ_MHZ(2.048), FREQ_MHZ(1.536),
	FREQ_KHZ(512), FREQ_KHZ(256), FREQ_KHZ(128)
};

static u32 pdpck_freq_list[] = {
	FREQ_MHZ(78), FREQ_MHZ(39), FREQ_MHZ(26), FREQ_MHZ(19.5),
	FREQ_MHZ(15.6), FREQ_MHZ(13), FREQ_MHZ(11.142857), FREQ_MHZ(9.75)
};

static u32 spi_freq_list[] = {
	FREQ_MHZ(104), FREQ_MHZ(78), FREQ_MHZ(39), FREQ_MHZ(26),
	FREQ_MHZ(19.5), FREQ_MHZ(15.6), FREQ_MHZ(13), FREQ_MHZ(11.142857),
	FREQ_MHZ(9.75)
};

static u32 gp_freq_list[] = {
	FREQ_MHZ(78), FREQ_MHZ(39), FREQ_MHZ(26), FREQ_MHZ(19.5),
	FREQ_MHZ(15.6), FREQ_MHZ(13), FREQ_MHZ(11.142857), FREQ_MHZ(9.75)
};

static u32 mspro_freq_list[] = {
	FREQ_MHZ(39), FREQ_MHZ(26), FREQ_MHZ(19.5),
	FREQ_MHZ(15.6), FREQ_MHZ(13), FREQ_MHZ(11.142857), FREQ_MHZ(9.75)
};

static u32 mipidsi_freq_list[] = {
	FREQ_MHZ(78), FREQ_MHZ(39), FREQ_MHZ(26), FREQ_MHZ(19.5),
	FREQ_MHZ(15.6), FREQ_MHZ(13), FREQ_MHZ(11.142857), FREQ_MHZ(9.75)
};

bool brcm_clk_is_pedestal_allowed(void)
{
	return (pedestal_ctrl_flag == 0);
}

EXPORT_SYMBOL(brcm_clk_is_pedestal_allowed);

unsigned long bcm21553_arm11_get_rate(struct clk *clk)
{
	unsigned int mode = 0;
	unsigned long apps_pll_freq = 0;
	/*div are multiplied by 2 to cater for 1.5*/
	const u8 apps_pll_div[] = {4,4,3};

	mode = readl(ADDR_CLKPWR_CLK_ARMAHB_MODE) & 0x0F;

	/* We assume that APPS PLL is active
	& mode value less than 7 is not supported*/
	if(mode <= 0xC)
		return clk_armahb_reg_to_arm11_freq_mapping[mode];

	apps_pll_freq = bcm21553_apps_pll_get_rate();
	apps_pll_freq = (apps_pll_freq*2)/apps_pll_div[mode-0xD];
	return apps_pll_freq;
}

int bcm21553_arm11_set_rate(struct clk *clk, unsigned long val)
{
	u32 mode;
	u32 arm11_freq[10];
	u32 apps_pll_freq = bcm21553_apps_pll_get_rate();
	
	arm11_freq[0] = (apps_pll_freq*2)/8;
	arm11_freq[1] = (apps_pll_freq*2)/6;
	arm11_freq[2] = (apps_pll_freq*3)/8;
	arm11_freq[3] = (apps_pll_freq*3)/6;
	arm11_freq[4] = (apps_pll_freq*2)/3;
	arm11_freq[5] = (apps_pll_freq*3)/4;
	arm11_freq[6] = apps_pll_freq;
	arm11_freq[7] = (apps_pll_freq*2)/16;
	arm11_freq[8] = (apps_pll_freq*2)/12;
	arm11_freq[9] = (apps_pll_freq*3)/13;
    /* 1248, 936, 832, 624, 468, 416, 312, 288, 208, 156 */
	/*we support only two modes - 0x0C/0x0F - thats what he said*/
	if (val == arm11_freq[0])
	{
		mode = 0x09;
	}
	else if (val == arm11_freq[1])
	{
		mode = 0x0A;
	}
	else if (val == arm11_freq[2])
	{
		mode = 0x0B;
	}
	else if (val == arm11_freq[3])
	{
		mode = 0x0C;
	}
	else if (val == arm11_freq[4])
	{
		mode = 0x0D;
	}
	else if (val == arm11_freq[5])
	{
		mode = 0x0E;
	}
	else if (val == arm11_freq[6])
	{
		mode = 0x0F;
	}
	else if (val == arm11_freq[7])
	{
		mode = 0x10;
	}
	else if (val == arm11_freq[8])
	{
		mode = 0x11;
	}
	else if (val == arm11_freq[9])
	{
		mode = 0x12;
	} else
	{
		return -EINVAL;
	}
	//writel(mode, ADDR_CLKPWR_CLK_ARMAHB_MODE);
	bcm215xx_set_armahb_mode(mode);
	return 0;
}

long bcm21553_arm11_round_rate(struct clk *clk, unsigned long desired_val)
{
	u32 arm11_freq[10];
	u32 apps_pll_freq = bcm21553_apps_pll_get_rate();

	/*we support only two freq  - 312Mhz & appPll/1.5*/
	/*Are you crazy? we can support more :)*/
	arm11_freq[0] = (apps_pll_freq*2)/8;
	arm11_freq[1] = (apps_pll_freq*2)/6;
	arm11_freq[2] = (apps_pll_freq*3)/8;
	arm11_freq[3] = (apps_pll_freq*3)/6;
	arm11_freq[4] = (apps_pll_freq*2)/3;
	arm11_freq[5] = (apps_pll_freq*3)/4;
	arm11_freq[6] = apps_pll_freq;
	arm11_freq[7] = (apps_pll_freq*2)/16;
	arm11_freq[8] = (apps_pll_freq*2)/12;
	arm11_freq[9] = (apps_pll_freq*3)/13;

	return (long)bcm21553_generic_round_rate(desired_val,
						 arm11_freq,
						 10);
}

/*AHB clock*/
unsigned long bcm21553_ahb_get_rate(struct clk *clk)
{
	u32 mode = 0, ahb;
	u32 apps_pll_freq = 0;
	mode = readl(ADDR_CLKPWR_CLK_ARMAHB_MODE) & 0x0F;
	if(mode < 0x0C)
		ahb = clk_armahb_reg_to_ahb_freq_mapping[mode];
	else
	{
		apps_pll_freq = bcm21553_apps_pll_get_rate();
		/*We have really need this stupid manupilation !???
		 * [ ((apps_pll_freq/9)/FREQ_MHZ(1))*FREQ_MHZ(1) ]
		 * Runing @ 138,666666667Mhz
		 */
		ahb = apps_pll_freq/9;
	}

	return ahb;
}

/*AHB fast clock*/
unsigned long bcm21553_ahb_fast_get_rate(struct clk *clk)
{
	u32 mode = 0, ahb_fast;
	u32 apps_pll_freq = 0;
	mode = readl(ADDR_CLKPWR_CLK_ARMAHB_MODE) & 0x0F;

	if(mode < 0x0C)
		ahb_fast = clk_armahb_reg_to_ahb_fast_freq_mapping[mode];
	else
	{
		apps_pll_freq = bcm21553_apps_pll_get_rate();
		/* If Really we need to go fast for what this stupid
		 * [ (mode <= 8) ? apps_pll_freq/9 :  ] again !!!!!
		 * Runing @ 208Mhz
		 */ 
		ahb_fast = apps_pll_freq/6;
	}

		/* What the hell is Going on with Samsung?
		 * Agaiiin????? :@ (ahb_fast*FREQ_MHZ(1))/FREQ_MHZ(1);
		 * ahb_fast = ahb_fast;
		 */
	return ahb_fast;
}

/* Camera clk - interface functions */
void bcm21553_cam_disable(struct clk *clk)
{
	u32 regVal;
	/* Enable CAM clk */
	regVal = readl(clk->enable_reg);
	regVal &= ~(clk->enable_bit_mask);
	writel(regVal, clk->enable_reg);

	writel(0x0, ADDR_SYSCFG_CAMARA_INTERFACE_AHB_CLK_EN);	/* disable AHB clock */
}

int bcm21553_cam_enable(struct clk *clk)
{
	u32 regVal;

	writel(0x01, ADDR_SYSCFG_CAMARA_INTERFACE_AHB_CLK_EN);	/* Enable AHB clock */

	/* Enable CAM clk */
	regVal = readl(clk->enable_reg);
	regVal |= clk->enable_bit_mask;
	writel(regVal, clk->enable_reg);
	return 0;
}

unsigned long bcm21553_cam_get_rate(struct clk *clk)
{
	return cam_freq_list[(readl(ADDR_CLKPWR_CLK_CAMCK_MODE) & 0x07)];
}

int bcm21553_cam_set_rate(struct clk *clk, unsigned long val)
{
	int inx;
	int count = ARRAY_SIZE(cam_freq_list);

	for (inx = 0; inx < count; inx++) {
		if (cam_freq_list[inx] == val) {
			writel(inx & 0x07, ADDR_CLKPWR_CLK_CAMCK_MODE);
			return 0;
		}
	}

	return -EINVAL;
}

long bcm21553_cam_round_rate(struct clk *clk, unsigned long desired_val)
{
	int count = ARRAY_SIZE(cam_freq_list);

	return (long)bcm21553_generic_round_rate(desired_val, cam_freq_list,
						 count);
}

/*I2S_INT - interface functions*/
unsigned long bcm21553_i2s_int_get_rate(struct clk *clk)
{
	u32 regVal;
	int i;
	const u32 i2s_int_mode[] = {
		0x00C, 0x040, 0x04B, 0x064, 0x12F, 0x260, 0x4C1
	};
	int count = ARRAY_SIZE(i2s_int_mode);

	regVal = readl(ADDR_CLKPWR_CLK_I2S_INT_MODE) & 0xFFF;

	for (i = 0; i < count; i++) {
		if (i2s_int_mode[i] == regVal) {
			return i2s_int_freq_list[i];
		}
	}
	return 0;
}

int bcm21553_i2s_int_set_rate(struct clk *clk, unsigned long val)
{
	u32 regVal;
	int i;
	const u32 i2s_int_mode[] = {
		0x00C, 0x040, 0x04B, 0x064, 0x12F, 0x260, 0x4C1
	};
	const u32 i2s_int_frac[] = { 0x0, 0x00, 0x0B, 0x24, 0x2C, 0x18, 0x30 };
	int count = ARRAY_SIZE(i2s_int_mode);

	for (i = 0; i < count; i++) {
		if (i2s_int_freq_list[i] == val) {
			regVal = i2s_int_mode[i] & 0xFFF;
			writel(regVal, ADDR_CLKPWR_CLK_I2S_INT_MODE);

			regVal = i2s_int_frac[i] & 0x3F;
			writel(regVal, ADDR_CLKPWR_CLK_I2S_FRAC_MODE);

			return 0;
		}
	}

	return -EINVAL;

}

long bcm21553_i2s_int_round_rate(struct clk *clk, unsigned long desired_val)
{
	u32 count = ARRAY_SIZE(i2s_int_freq_list);

	return (long)bcm21553_generic_round_rate(desired_val, i2s_int_freq_list,
						 count);
}

/* PDP clk - interface functions */
unsigned long bcm21553_pdpck_get_rate(struct clk *clk)
{
	return pdpck_freq_list[(readl(ADDR_CLKPWR_CLK_PDPCK_DIV) & 0x7)];

}

int bcm21553_pdpck_set_rate(struct clk *clk, unsigned long val)
{
	u32 count = ARRAY_SIZE(pdpck_freq_list);
	u32 inx;

	for (inx = 0; inx < count; inx++) {
		if (pdpck_freq_list[inx] == val) {
			writel(inx & 0x07, ADDR_CLKPWR_CLK_PDPCK_DIV);
			return 0;
		}
	}

	return -EINVAL;

}

long bcm21553_pdpck_round_rate(struct clk *clk, unsigned long desired_val)
{
	u32 count = ARRAY_SIZE(pdpck_freq_list);

	return (long)bcm21553_generic_round_rate(desired_val, pdpck_freq_list,
						 count);
}

/* SDIO clk - interface functions */
int bcm21553_sdio_enable(struct clk *clk)
{
	u32 regVal;
	u32 ahbReg;

	/* Enable AHB clock */
	if (clk->id == BCM21553_CLK_SDIO1) {
		ahbReg = ADDR_SYSCFG_SDIO1_AHB_CLK_EN;
	} else if (clk->id == BCM21553_CLK_SDIO2) {
		ahbReg = ADDR_SYSCFG_SDIO2_AHB_CLK_EN;
	} else if (clk->id == BCM21553_CLK_SDIO3) {
		ahbReg = ADDR_SYSCFG_SDIO3_AHB_CLK_EN;
	} else {
		pr_info("Invalid clk !!!!\n");
		return -EINVAL;
	}
	writel(0x01, ahbReg);	/* Enable AHB clock */

	/* Enable SDIO clk */
	regVal = readl(clk->enable_reg);
	regVal |= clk->enable_bit_mask;
	writel(regVal, clk->enable_reg);

	return 0;
}

void bcm21553_sdio_disable(struct clk *clk)
{
	u32 regVal;
	u32 ahbReg;

	/*  AHB clock enable reg address */
	if (clk->id == BCM21553_CLK_SDIO1) {
		ahbReg = ADDR_SYSCFG_SDIO1_AHB_CLK_EN;
	} else if (clk->id == BCM21553_CLK_SDIO2) {
		ahbReg = ADDR_SYSCFG_SDIO2_AHB_CLK_EN;
	} else if (clk->id == BCM21553_CLK_SDIO3) {
		ahbReg = ADDR_SYSCFG_SDIO3_AHB_CLK_EN;
	} else {
		pr_info("Invalid clk !!!!\n");
		return;
	}
	/* disable SDIO clk */
	regVal = readl(clk->enable_reg);
	regVal &= ~(clk->enable_bit_mask);
	writel(regVal, clk->enable_reg);

	writel(0x00, ahbReg);	/* Disable AHB clock */

}

unsigned long bcm21553_sdio_get_rate(struct clk *clk)
{
	unsigned int div = 0;
	volatile u32 regVal;
	u32 regAddr;
	u32 rate = 0;

	if (clk->id == BCM21553_CLK_SDIO1) {
		regAddr = ADDR_CLKPWR_CLK_SDIO1_DIV;
	} else if (clk->id == BCM21553_CLK_SDIO2) {
		regAddr = ADDR_CLKPWR_CLK_SDIO3_DIV;
	} else if (clk->id == BCM21553_CLK_SDIO3) {
		regAddr = ADDR_CLKPWR_CLK_SDIO3_DIV;
	} else {
		pr_info("Invalid clk !!!!\n");
		return 0;
	}

	regVal = readl(regAddr);

	/* See if 24/48 Mhz select bit is set */
	if (regVal & CLK_SDIO_DIV_48_24_SEL) {
		if ((regVal & (CLK_SDIO_DIV_24_SEL | CLK_SDIO_DIV_48_24_EN)) ==
		    (CLK_SDIO_DIV_24_SEL | CLK_SDIO_DIV_48_24_EN)) {
			rate = FREQ_MHZ(24);
		} else if (regVal & CLK_SDIO_DIV_48_24_EN) {
			rate = FREQ_MHZ(48);
		}
	} else {
		div = regVal & CLK_SDIO_DIV_DIVIDER_MASK;

		/* The clk_sdio1 fequency = 104 / n */
		/* where n is equal to (clk_sdio_div_divider + 1)*2 */
		/* For example, when you program clk_sdio1_div = 1, */
		/* then clk_sdio1 = 104 / ((1 + 1)*2)) = 26 Mhz. */

		rate = ((FREQ_MHZ(104) / ((div + 1) * 2)));
	}
	return rate;
}

int bcm21553_sdio_set_rate(struct clk *clk, unsigned long val)
{
	long div = 0;
	volatile u32 regVal;
	u32 regAddr;

	/* Freq = 104MHz / ((div + 1) *2) */
	if (val > FREQ_MHZ(52)) {
		return -EINVAL;
	}

	if (clk->id == BCM21553_CLK_SDIO1) {
		regAddr = ADDR_CLKPWR_CLK_SDIO1_DIV;
	} else if (clk->id == BCM21553_CLK_SDIO2) {
		regAddr = ADDR_CLKPWR_CLK_SDIO2_DIV;
	} else if (clk->id == BCM21553_CLK_SDIO3) {
		regAddr = ADDR_CLKPWR_CLK_SDIO3_DIV;
	} else {
		pr_info("Invalid clk !!!!\n");
		return -EINVAL;
	}

	if (val == FREQ_MHZ(24)) {
		writel(CLK_SDIO_DIV_48_24_SEL, regAddr);
		writel((CLK_SDIO_DIV_48_24_SEL | CLK_SDIO_DIV_48_24_EN),
		       regAddr);
		writel(CLK_SDIO_DIV_48_24_SEL | CLK_SDIO_DIV_48_24_EN |
		       CLK_SDIO_DIV_24_SEL, regAddr);

	} else if (val == FREQ_MHZ(48)) {

		writel(CLK_SDIO_DIV_48_24_SEL, regAddr);
		writel((CLK_SDIO_DIV_48_24_SEL | CLK_SDIO_DIV_48_24_EN),
		       regAddr);
	} else {

		div = FREQ_MHZ(52) / val;	/* (div + 1) = 52MHZ / val. */
		div = div - 1;
		regVal = div & CLK_SDIO_DIV_DIVIDER_MASK;
		writel(regVal, regAddr);
	}

	return 0;
}

long bcm21553_sdio_round_rate(struct clk *clk, unsigned long desired_val)
{
	long temp = 0;

	/* Freq = 104MHz / ((div + 1) *2) */
	if (desired_val > FREQ_MHZ(52)) {
		return -EINVAL;
	}
	if (desired_val == FREQ_MHZ(24) || desired_val == FREQ_MHZ(48))
		return desired_val;

	temp = FREQ_MHZ(52) / desired_val;	/* (div + 1) = 52MHZ / val. */

	temp = temp - 1;

	/* Now return the actual possible value. */
	return FREQ_MHZ(104) / ((temp + 1) * 2);
}

/* SDRAM CLK - interface functions */
unsigned long bcm21553_sdram_get_rate(struct clk *clk)
{
	u32 sdram_mode, mode;
	unsigned long apps_pll_freq, sdram_clk_speed;
	sdram_mode = readl(ADDR_CLKPWR_CLK_SDRAM_MODE);
	apps_pll_freq = bcm21553_apps_pll_get_rate();

	if (sdram_mode & CLK_SDRAM_SYC_MODE)
	{
		mode = readl(ADDR_CLKPWR_CLK_ARMAHB_MODE) & 0x0F;
		if(mode < 0xC)
			sdram_clk_speed = clk_armahb_reg_to_ahb_fast_freq_mapping[mode];
		else
			/*STUPID THINGS AGAIN [ (mode <= 8) ? apps_pll_freq/9 :  ]*/
			sdram_clk_speed = apps_pll_freq/6;
	}
	else
	{
		sdram_clk_speed = apps_pll_freq /
			    clk_sdram_mode_reg_to_appspll_divider[sdram_mode & 0x3];
	}
	return sdram_clk_speed;
}

/*NVSRAM Clk - interface functions*/
unsigned long bcm21553_nvsram_get_rate(struct clk *clk)
{
	u32 regVal;
	u32 ahbVal;
	regVal = readl(ADDR_CLKPWR_CLK_SRAM_MODE);
	if (regVal & CLK_NVSRAM_SYNC_MODE) {
		ahbVal = readl(ADDR_CLKPWR_CLK_ARMAHB_MODE) & 0x0F;
		return clk_armahb_reg_to_ahb_freq_mapping[ahbVal];

	} else {
		switch (regVal & 0x3) {
		case 0x00:
			return FREQ_MHZ(78);
		case 0x01:
			return FREQ_MHZ(104);
		case 0x02:
			return FREQ_MHZ(156);
		}
	}

	return 0;
}

/*SPI clk - interface functions */
unsigned long bcm21553_spi_get_rate(struct clk *clk)
{
	u32 regVal;
	u32 regAddr;

	if (clk->id == BCM21553_CLK_SPI1)
		regAddr = ADDR_CLKPWR_CLK_SPI_DIV;
	else if (clk->id == BCM21553_CLK_SPI2)
		regAddr = ADDR_CLKPWR_CLK_SPI2_DIV;
	else {
		pr_info("SPI : Invalid clk\n");
		return 0;
	}

	regVal = readl(regAddr);

	if (regVal & CLK_SPI_DIV_104_EN)
		return FREQ_MHZ(104);
	else {
		regVal &= 0x7;
		return spi_freq_list[regVal + 1];
	}

}

int bcm21553_spi_set_rate(struct clk *clk, unsigned long val)
{
	u32 regAddr;
	int i;
	u32 count = ARRAY_SIZE(spi_freq_list);

	if (clk->id == BCM21553_CLK_SPI1)
		regAddr = ADDR_CLKPWR_CLK_SPI_DIV;
	else if (clk->id == BCM21553_CLK_SPI2)
		regAddr = ADDR_CLKPWR_CLK_SPI2_DIV;
	else {
		pr_info("SPI : Invalid clk\n");
		return 0;
	}

	if (val == FREQ_MHZ(104)) {
		writel(CLK_SPI_DIV_104_EN, regAddr);
		return 0;
	} else {
		for (i = 1; i < count; i++) {
			if (val == spi_freq_list[i]) {
				writel(i - 1, regAddr);
				return 0;
			}
		}
	}
	return -EINVAL;
}

long bcm21553_spi_round_rate(struct clk *clk, unsigned long desired_val)
{
	u32 count = ARRAY_SIZE(spi_freq_list);

	return (long)bcm21553_generic_round_rate(desired_val, spi_freq_list,
						 count);
}

/*UART clk - interface clocks*/
int bcm21553_uart_enable(struct clk *clk)
{
	u32 regVal;
	u32 ahbReg;

	/* Enable AHB clock */
	if (clk->id == BCM21553_CLK_UARTA) {
		ahbReg = ADDR_SYSCFG_UARTA_AHB_CLK_EN;
	} else if (clk->id == BCM21553_CLK_UARTB) {
		ahbReg = ADDR_SYSCFG_UARTB_AHB_CLK_EN;
	} else if (clk->id == BCM21553_CLK_UARTC) {
		ahbReg = ADDR_SYSCFG_UARTC_AHB_CLK_EN;
	} else {
		pr_info("Invalid clk !!!!\n");
		return -EINVAL;
	}
	writel(0x01, ahbReg);	/* Enable AHB clock */

	/* Enable UART clk */
	regVal = readl(clk->enable_reg);
	regVal |= clk->enable_bit_mask;
	writel(regVal, clk->enable_reg);

	return 0;
}

void bcm21553_uart_disable(struct clk *clk)
{
	u32 regVal;
	u32 ahbReg;
	/*  AHB clock enable reg address */
	if (clk->id == BCM21553_CLK_UARTA) {
		ahbReg = ADDR_SYSCFG_UARTA_AHB_CLK_EN;
	} else if (clk->id == BCM21553_CLK_UARTB) {
		ahbReg = ADDR_SYSCFG_UARTB_AHB_CLK_EN;
	} else if (clk->id == BCM21553_CLK_UARTC) {
		ahbReg = ADDR_SYSCFG_UARTC_AHB_CLK_EN;
	} else {
		pr_info("Invalid clk !!!!\n");
		return;
	}
	/* disable SDIO clk */
	regVal = readl(clk->enable_reg);
	regVal &= ~(clk->enable_bit_mask);
	writel(regVal, clk->enable_reg);

	writel(0x00, ahbReg);	/* Disable AHB clock */

}

unsigned long bcm21553_uart_get_rate(struct clk *clk)
{
	volatile unsigned int rr_n = 0;
	volatile unsigned int rr_m = 0;

	if (clk->id == BCM21553_CLK_UARTA) {
		rr_n = ((readl(ADDR_CLKPWR_CLK_UARTA_N)) & 0x1FF);
		rr_m = ((readl(ADDR_CLKPWR_CLK_UARTA_M)) & 0x1FF);

	} else if (clk->id == BCM21553_CLK_UARTB) {
		rr_n = ((readl(ADDR_CLKPWR_CLK_UARTB_N)) & 0x1FF);
		rr_m = ((readl(ADDR_CLKPWR_CLK_UARTB_M)) & 0x1FF);
	} else if (clk->id == BCM21553_CLK_UARTC) {
		rr_n = ((readl(ADDR_CLKPWR_CLK_UARTC_N)) & 0x1FF);
		rr_m = ((readl(ADDR_CLKPWR_CLK_UARTC_M)) & 0x1FF);
	}
	/* The clk_uart fequency = 156 * n / m */
	return (FREQ_MHZ(156) / rr_m) * rr_n;
}

int bcm21553_uart_set_rate(struct clk *clk, unsigned long val)
{
	long ret_val = 0;
	long m_set = 0;
	long n_set = 0;
	u32 regVal;

	ret_val = bcm21553_uart_find_m_n(val, &m_set, &n_set);
	if (ret_val > 0) {

		if (clk->id == BCM21553_CLK_UARTA) {
			regVal = (u32) n_set & 0x1FF;
			writel(regVal, ADDR_CLKPWR_CLK_UARTA_N);

			regVal = (u32) m_set & 0x1FF;
			writel(regVal, ADDR_CLKPWR_CLK_UARTA_M);
		}

		else if (clk->id == BCM21553_CLK_UARTB) {
			regVal = (u32) n_set & 0x1FF;
			writel(regVal, ADDR_CLKPWR_CLK_UARTB_N);

			regVal = (u32) m_set & 0x1FF;
			writel(regVal, ADDR_CLKPWR_CLK_UARTB_M);

		} else if (clk->id == BCM21553_CLK_UARTC) {
			regVal = (u32) n_set & 0x1FF;
			writel(regVal, ADDR_CLKPWR_CLK_UARTC_N);

			regVal = (u32) m_set & 0x1FF;
			writel(regVal, ADDR_CLKPWR_CLK_UARTC_M);

		}
		return 0;
	}

	return ret_val;

}

long bcm21553_uart_round_rate(struct clk *clk, unsigned long desired_val)
{
	long m_set = 0;
	long n_set = 0;
	return bcm21553_uart_find_m_n(desired_val, &m_set, &n_set);
}

/*GP - interface function */
unsigned long bcm21553_gp_get_rate(struct clk *clk)
{
	return gp_freq_list[(readl(ADDR_CLKPWR_CLK_GPCK_DIV) & 0x7)];
}

int bcm21553_gp_set_rate(struct clk *clk, unsigned long val)
{
	u32 count = ARRAY_SIZE(gp_freq_list);
	u32 inx;
	u32 regVal;

	for (inx = 0; inx < count; inx++) {
		if (gp_freq_list[inx] == val) {
			regVal = inx & 0x07;
			writel(regVal, ADDR_CLKPWR_CLK_GPCK_DIV);
			return 0;
		}
	}

	return -EINVAL;
}

long bcm21553_gp_round_rate(struct clk *clk, unsigned long desired_val)
{
	u32 count = ARRAY_SIZE(gp_freq_list);

	return (long)bcm21553_generic_round_rate(desired_val, gp_freq_list,
						 count);
}

/* MSPRO clk - interface functions*/
unsigned long bcm21553_mspro_get_rate(struct clk *clk)
{
	u32 regVal = readl(ADDR_CLKPWR_CLK_MSPRO_DIV) & 0x7;

	return mspro_freq_list[regVal - 1];
}

int bcm21553_mspro_set_rate(struct clk *clk, unsigned long val)
{
	u32 count = ARRAY_SIZE(mspro_freq_list);
	u32 inx;
	u32 regVal;

	for (inx = 0; inx < count; inx++) {
		if (mspro_freq_list[inx] == val) {
			regVal = (inx + 1) & 0x07;
			writel(regVal, ADDR_CLKPWR_CLK_MSPRO_DIV);
			return 0;
		}
	}

	return -EINVAL;
}

long bcm21553_mspro_round_rate(struct clk *clk, unsigned long desired_val)
{
	u32 count = ARRAY_SIZE(mspro_freq_list);

	return (long)bcm21553_generic_round_rate(desired_val, mspro_freq_list,
						 count);
}

/*I2C clk - interface functions */
unsigned long bcm21553_i2c_get_rate(struct clk *clk)
{
	u32 regAddr;
	u32 regVal;

	if (clk->id == BCM21553_CLK_I2C1)
		regAddr = ADDR_CLKPWR_CLK_HSBSC_MODE;
	else if (clk->id == BCM21553_CLK_I2C2)
		regAddr = ADDR_CLKPWR_CLK_HSBSC2_MODE;
	else if (clk->id == BCM21553_CLK_I2C3)
		regAddr = ADDR_CLKPWR_CLK_HSBSC3_MODE;
	else {
		pr_info("I2C: Invalid clk\n");
		return 0;
	}

	regVal = readl(regAddr) & 0x01;

	if (regVal)
		return FREQ_MHZ(104);
	else
		return FREQ_MHZ(13);
}

int bcm21553_i2c_set_rate(struct clk *clk, unsigned long val)
{
	u32 regAddr;
	u32 regVal;

	if (val != FREQ_MHZ(13) && val != FREQ_MHZ(104))
		return -EINVAL;

	if (clk->id == BCM21553_CLK_I2C1)
		regAddr = ADDR_CLKPWR_CLK_HSBSC_MODE;
	else if (clk->id == BCM21553_CLK_I2C2)
		regAddr = ADDR_CLKPWR_CLK_HSBSC2_MODE;
	else if (clk->id == BCM21553_CLK_I2C3)
		regAddr = ADDR_CLKPWR_CLK_HSBSC3_MODE;
	else {
		pr_info("I2C: Invalid clk\n");
		return -EINVAL;
	}

	regVal = (val == FREQ_MHZ(104)) ? 1 : 0;
	writel(regVal, regAddr);
	return 0;
}

long bcm21553_i2c_round_rate(struct clk *clk, unsigned long desired_val)
{
	u32 i2c_freq_list[] = { FREQ_MHZ(13), FREQ_MHZ(104) };

	return (long)bcm21553_generic_round_rate(desired_val, i2c_freq_list, 2);
}

int bcm21553_damck_enable(struct clk *clk)
{
	u32 regVal;

	writel(0x01, ADDR_SYSCFG_DA_AHB_CLK_EN);	/* Enable AHB clock */

	/* Enable DAM clk */
	regVal = readl(clk->enable_reg);
	regVal |= clk->enable_bit_mask;
	writel(regVal, clk->enable_reg);
	return 0;
}

void bcm21553_damck_disable(struct clk *clk)
{
	u32 regVal;
	/* disable DAM clk */
	regVal = readl(clk->enable_reg);
	regVal &= ~(clk->enable_bit_mask);
	writel(regVal, clk->enable_reg);

	writel(0x0, ADDR_SYSCFG_DA_AHB_CLK_EN);	/* disable AHB clock */

}

/*MIPIDSI clk - interface functions*/
int bcm21553_mipidsi_enable(struct clk *clk)
{
	u32 regVal;

	writel(0x01, ADDR_SYSCFG_DSICR);	/* Enable AHB clock */

	/* Enable MIPI DSI clk */
	regVal = readl(clk->enable_reg);
	regVal |= clk->enable_bit_mask;
	writel(regVal, clk->enable_reg);
	return 0;

}

void bcm21553_mipidsi_disable(struct clk *clk)
{
	u32 regVal;
	/* disable mipidsi clk */
	regVal = readl(clk->enable_reg);
	regVal &= ~(clk->enable_bit_mask);
	writel(regVal, clk->enable_reg);

	writel(0x0, ADDR_SYSCFG_DSICR);	/* disable AHB clock */

}

unsigned long bcm21553_mipidsi_get_rate(struct clk *clk)
{
	return mipidsi_freq_list[(readl(ADDR_CLKPWR_CLK_MIPI_DSI_CTRL) & 0x7)];
}

int bcm21553_mipidsi_set_rate(struct clk *clk, unsigned long val)
{
	u32 count = ARRAY_SIZE(mipidsi_freq_list);
	u32 inx;
	u32 regVal;

	for (inx = 0; inx < count; inx++) {
		if (mipidsi_freq_list[inx] == val) {
			regVal = inx & 0x07;
			writel(regVal, ADDR_CLKPWR_CLK_MIPI_DSI_CTRL);
			return 0;
		}
	}

	return -EINVAL;
}

long bcm21553_mipidsi_round_rate(struct clk *clk, unsigned long desired_val)
{
	u32 count = ARRAY_SIZE(mipidsi_freq_list);

	return (long)bcm21553_generic_round_rate(desired_val, mipidsi_freq_list,
						 count);
}

/*camrx clk - interface functions*/
unsigned long bcm21553_camrx_get_rate(struct clk *clk)
{
	u32 regVal = readl(ADDR_CLKPWR_CLK_CAMRX_MODE) & 0x7;

	switch (regVal) {
	case 0x01:
		return FREQ_MHZ(208);
	case 0x02:
		return FREQ_MHZ(156);
	case 0x03:
		return FREQ_MHZ(104);
	}
	return 0;
}

int bcm21553_camrx_set_rate(struct clk *clk, unsigned long val)
{
	const u32 camrx_freq_list[] = {
		FREQ_MHZ(208), FREQ_MHZ(156), FREQ_MHZ(104)
	};
	u32 inx;
	u32 regVal;

	for (inx = 0; inx < 3; inx++) {
		if (camrx_freq_list[inx] == val) {
			regVal = (inx + 1) & 0x07;
			writel(regVal, ADDR_CLKPWR_CLK_CAMRX_MODE);
			return 0;
		}
	}

	return -EINVAL;
}

long bcm21553_camrx_round_rate(struct clk *clk, unsigned long desired_val)
{
	const u32 camrx_freq_list[] = {
		FREQ_MHZ(208), FREQ_MHZ(156), FREQ_MHZ(104)
	};
	u32 count = ARRAY_SIZE(camrx_freq_list);

	return (long)bcm21553_generic_round_rate(desired_val, camrx_freq_list,
						 count);
}

/*USB clk - interface functions */
void bcm21553_usb_disable(struct clk *clk)
{
	writel(0, ADDR_CLKPWR_CLK_USBPLL_ENABLE_R);
	writel(0, ADDR_CLKPWR_CLK_USBPLL_OEN_R);
	writel(0, ADDR_CLKPWR_CLK_CLK_USB_48_ENABLE);

	writel(0, ADDR_SYSCFG_USB_AHB_CLK_EN);

}

int bcm21553_usb_enable(struct clk *clk)
{
	writel(1, ADDR_SYSCFG_USB_AHB_CLK_EN);

	writel(1, ADDR_CLKPWR_CLK_USBPLL_ENABLE_R);
	writel(1, ADDR_CLKPWR_CLK_USBPLL_OEN_R);
	writel(1, ADDR_CLKPWR_CLK_CLK_USB_48_ENABLE);
	return 0;
}


int bcm21553_v3d_power_enable(struct clk *clk)
{
	unsigned long flags;
	u32 ahb_mode;
	int i;
	/* We need to switch to 312Mhz (mode = 0x0C) before enabling
	 * V3D - WORKAROUND for V3D bug
	 */
	raw_local_irq_save(flags);
	/*Assert if not in turbo mode */
	//BUG_ON(readl(ADDR_CLKPWR_CLK_APPSPLL_ENABLE)== 0);

	/*Enable AHB request*/
	//writel(0x01, ADDR_SYSCFG_V3D_AHB_CLK_EN);

	/*Save ahb mode and set ahb mode to 0x0C*/
	ahb_mode = readl(ADDR_CLKPWR_CLK_ARMAHB_MODE) & 0x0F;
	//writel(0x0C, ADDR_CLKPWR_CLK_ARMAHB_MODE);
	/*I wanna Switch to 0x0D :P*/
	bcm215xx_set_armahb_mode(0x0D);
	udelay(80);

	/* Write 0 bit 0 to POWER ON V3D island */
	writel(V3D_POWER_ON, clk->enable_reg);
	do {
		udelay(1);
	} while ((readl(clk->enable_reg) & V3D_POWER_OK) == 0);

	/* Soft reset V3D HW */
	for (i = 0; i < 4; i++) {
		writel(1, ADDR_SYSCFG_V3DRSTR);
		udelay(1);
		writel(0, ADDR_SYSCFG_V3DRSTR);
		udelay(1);
	}

	//writel(ahb_mode, ADDR_CLKPWR_CLK_ARMAHB_MODE);
	bcm215xx_set_armahb_mode(ahb_mode);
	udelay(1);
	raw_local_irq_restore(flags);

	return 0;
}

void bcm21553_v3d_power_disable(struct clk *clk)
{
	unsigned long flags;
	raw_local_irq_save(flags);
	/*assert if not in turbo mode */
	//BUG_ON(readl(ADDR_CLKPWR_CLK_APPSPLL_ENABLE)== 0);
	udelay(5); /*2.	5uSec blanket delay*/
	/* Write 1 bit 0 to POWER OFF V3D island */
	writel(V3D_POWER_OFF, clk->enable_reg);
	do {
		udelay(1);
	} while ((readl(clk->enable_reg) & V3D_POWER_OK) == V3D_POWER_OK);
	udelay(12);
	/*disable AHB request*/
	writel(0x0, ADDR_SYSCFG_V3D_AHB_CLK_EN);
	raw_local_irq_restore(flags);
}

void bcm21553_vcodec_power_disable(struct clk *clk)
{
	/*write 1 bit 0 to POWER OFF vcodec island*/
	writel(VCODEC_POWER_OFF, clk->enable_reg);
	do
	{
		udelay(1);
	}
	while((readl(clk->enable_reg) & VCODEC_POWER_OK) == VCODEC_POWER_OK);
}

int bcm21553_vcodec_power_enable(struct clk *clk)
{
	/*write 0 bit 0 to POWER ON vcodec island*/
	writel(VCODEC_POWER_ON, clk->enable_reg);
	do
	{
		udelay(1);
	}
	while((readl(clk->enable_reg) & VCODEC_POWER_OK) == 0);
	return 0;
}

/*Apps PL Enalbe clk*/
void bcm21553_appspll_en_disable(struct clk *clk)
{
	/*u32 regVal;
	regVal = readl(clk->enable_reg);
	regVal &= ~(clk->enable_bit_mask);
	writel(regVal, clk->enable_reg);
	udelay(500);*/
	bcm215xx_set_appll_enable(0);
}

int bcm21553_appspll_en_enable(struct clk *clk)
{
	/*u32 regVal;
	regVal = readl(clk->enable_reg);
	regVal |= clk->enable_bit_mask;
	writel(regVal, clk->enable_reg);
	udelay(500);*/
	bcm215xx_set_appll_enable(1);
	return 0;
}

static int bcm21553_lcd_enable(struct clk *clk)
{
	writel(LCD_AHB_EN, clk->enable_reg);
	return 0;
}

static void bcm21553_lcd_disable(struct clk *clk)
{
	writel(0, clk->enable_reg);
}

static struct clk_lookup lookups[] = {
	BRCM_REGISTER_CLK(BCM_CLK_MAIN_PLL_STR_ID, NULL, mainpll),
	BRCM_REGISTER_CLK(BCM_CLK_APPS_PLL_STR_ID, NULL, appspll),
	BRCM_REGISTER_CLK(BCM_CLK_ARM11_STR_ID, NULL, arm11),
	BRCM_REGISTER_CLK(BCM_CLK_AHB_STR_ID, NULL, ahb),
	BRCM_REGISTER_CLK(BCM_CLK_AHB_FAST_STR_ID, NULL, ahb_fast),
	BRCM_REGISTER_CLK(BCM_CLK_I2S_INT_STR_ID, NULL, i2s_int),
	BRCM_REGISTER_CLK(BCM_CLK_I2S_EXT_STR_ID, NULL, i2s_ext),
	BRCM_REGISTER_CLK(BCM_CLK_DAM_STR_ID, NULL, damck),
	BRCM_REGISTER_CLK(BCM_CLK_PDP_STR_ID, NULL, pdpck),
	BRCM_REGISTER_CLK(BCM_CLK_SDIO1_STR_ID, NULL, sdio1),
	BRCM_REGISTER_CLK(BCM_CLK_SDIO2_STR_ID, NULL, sdio2),
	BRCM_REGISTER_CLK(BCM_CLK_SDIO3_STR_ID, NULL, sdio3),
	BRCM_REGISTER_CLK(BCM_CLK_SDRAM_STR_ID, NULL, sdram),
	BRCM_REGISTER_CLK(BCM_CLK_NVSRAM_STR_ID, NULL, nvsram),
	BRCM_REGISTER_CLK(BCM_CLK_SPI0_STR_ID, NULL, spi1),
	BRCM_REGISTER_CLK(BCM_CLK_SPI1_STR_ID, NULL, spi2),
	BRCM_REGISTER_CLK(BCM_CLK_UARTA_STR_ID, NULL, uarta),
	BRCM_REGISTER_CLK(BCM_CLK_UARTB_STR_ID, NULL, uartb),
	BRCM_REGISTER_CLK(BCM_CLK_UARTC_STR_ID, NULL, uartc),
	BRCM_REGISTER_CLK(BCM_CLK_GP_STR_ID, NULL, gp),
	BRCM_REGISTER_CLK(BCM_CLK_MSPRO_STR_ID, NULL, mspro),
	BRCM_REGISTER_CLK(BCM_CLK_I2C1_STR_ID, NULL, i2c1),
	BRCM_REGISTER_CLK(BCM_CLK_I2C2_STR_ID, NULL, i2c2),
	BRCM_REGISTER_CLK(BCM_CLK_I2C3_STR_ID, NULL, i2c3),
	BRCM_REGISTER_CLK(BCM_CLK_MIPIDSI_STR_ID, NULL, mipidsi),
	BRCM_REGISTER_CLK(BCM_CLK_PWM_STR_ID, NULL, pwm),
	BRCM_REGISTER_CLK(BCM_CLK_CAMERA_STR_ID, NULL, cam),
	BRCM_REGISTER_CLK(BCM_CLK_CAMERARX_STR_ID, NULL, camrx),
	BRCM_REGISTER_CLK(BCM_CLK_CAMERASYS_STR_ID, NULL, camsys),
	BRCM_REGISTER_CLK(BCM_CLK_USB_STR_ID, NULL, usb),
	BRCM_REGISTER_CLK(BCM_CLK_VCODEC_STR_ID, NULL, vcodec),
	BRCM_REGISTER_CLK(BCM_CLK_VCODEC_POWER_STR_ID, NULL,
			  vcodec_power),
	BRCM_REGISTER_CLK(BCM_CLK_V3D_STR_ID, NULL, v3d),
	BRCM_REGISTER_CLK(BCM_CLK_V3D_POWER_STR_ID, NULL, v3d_power),
	BRCM_REGISTER_CLK(BCM_CLK_DMAC_STR_ID, NULL, dmac),
	BRCM_REGISTER_CLK(BCM_CLK_RNG_STR_ID, NULL, rng),
	BRCM_REGISTER_CLK(BCM_CLK_LCD_STR_ID, NULL, lcd),
	BRCM_REGISTER_CLK(BCM_CLK_MPHI_STR_ID, NULL, mphi),
	BRCM_REGISTER_CLK(BCM_CLK_USB_PHY_REF_STR_ID, NULL, usb_phy_ref),
	BRCM_REGISTER_CLK(BCM_CLK_AUDIO_RX_ADC_STR_ID, NULL, audio_rx_adc),
	BRCM_REGISTER_CLK(BCM_CLK_AUDIO_TX_DAC_STR_ID, NULL, audio_tx_dac),
	BRCM_REGISTER_CLK(BCM_CLK_AUXADC_DAC_REF_STR_ID, NULL, auxadc_dac_ref),
	BRCM_REGISTER_CLK(BCM_CLK_APPSPLL_EN_STR_ID, NULL, appspll_en),
	BRCM_REGISTER_CLK(BCM_CLK_FSUSB_STR_ID, NULL, fsusb),
	BRCM_REGISTER_CLK(BCM_CLK_CRYPTO_STR_ID, NULL, crypto),
};

u32 bcm21553_generic_round_rate(u32 desired_val, const u32 *supportedFreqList,
				u8 count)
{
	u32 i = 0;
	const u32 *ppossible_freqs = supportedFreqList;
	u32 closest_freq = 0xFFFFFFFF;	/* Set it to some highest value. */
	u32 greatest_freq_in_array = 0;

	while (i < count) {
		if (desired_val == *ppossible_freqs) {
			return desired_val;
		} else {
			if ((*ppossible_freqs > desired_val)
			    && (*ppossible_freqs <= closest_freq)) {
				closest_freq = *ppossible_freqs;
			}
			if (*ppossible_freqs > greatest_freq_in_array) {
				greatest_freq_in_array = *ppossible_freqs;
			}
		}
		i++;
		ppossible_freqs++;
	}

	/* This means that desired_val is greater than the maximum possible values */
	if (closest_freq == 0xFFFFFFFF) {
		/* This means the desired_val is greater than the greatest element */
		/* So lets return with the greatest freq in array. */
		return greatest_freq_in_array;
	} else {
		return closest_freq;
	}
}

long bcm21553_uart_find_m_n(unsigned long freq_val, long *m_set, long *n_set)
{
	long factor = 0;	/* = m/n */
	long eq_2_val = MAX_UART_M_VAL + MAX_UART_N_VAL;
	long n_val = 0;
	long long n_iter = 0;
	long long m_iter = 0;
	long long left = 0;
	long long right = 0;
	long long diff = 0;
	long long least_diff = 0;
	long long least_diff_m_iter = 1;
	long long least_diff_n_iter = 1;
	long long val = 0;
	long ret_val;

	/* Formula : Freq = (156MHz * n) / m. */

	if (freq_val > FREQ_MHZ(156)) {	/* Can't have freq greater than 156MHz. */
		return -EINVAL;
	} else if (freq_val == FREQ_MHZ(156)) {
		*m_set = *n_set = 1;
		return FREQ_MHZ(156);
	}

	*m_set = 0;
	*n_set = 0;

	factor = FREQ_MHZ(156) / freq_val;

	/* range of m :  0 <= m <= 0x1ff */
	/* range of n :  0 <= n <= 0x1ff */
	/* Equation 1 : factor = m /n. */
	/*           so m = (factor * n). */
	/* Equation 2 : m + n <= 0x1ff + 0x1ff. */
	/*              m + n <= (2 * 0x1ff) ; Let's call (2 * 0x1ff) as eq_2_val. */
	/*          so  m + n <= eq_2_val. */
	/* Substiture eq1 in eq2. */
	/* (factor * n) + n <= eq_2_val. */
	/* taking n common : */
	/* (factor + 1) * n <= eq_2_val. */
	/* so n <= eq_2_val / (factor + 1). <==== This is a very important conclusion. */
	/* Now the logic is to iterate(iter) till n, and
	 * find a value of m, where m = 156MHz * iter / val. */
	n_val = eq_2_val / (factor + 1);

	pr_info("n_val = %ld, eq_2_val = %ld, factor = %ld\n", n_val, eq_2_val,
		factor);

	n_iter = 0;
	diff = 1;
	least_diff = 0xffffffff;
	val = freq_val;

	while ((n_iter <= n_val) && (diff != 0)) {
		n_iter++;
		left = FREQ_MHZ(156) * n_iter;

		m_iter = 0;
		while ((m_iter <= MAX_UART_M_VAL) && (diff != 0)) {
			m_iter++;
			right = val * m_iter;

			/* Always left side must be greater than right. So diff has to be positive. */
			diff = left - right;

			if ((diff > 0) && (diff < least_diff)) {
				least_diff = diff;
				least_diff_m_iter = m_iter;
				least_diff_n_iter = n_iter;
			}
			/* pr_info("n_iter = %lld, m_iter = %lld, left = %lld, \
			   right = %lld, diff = %lld, least_diff = %lld, \
			   least_diff_m_iter = %lld, \
			   least_diff_n_iter = %lld\n",
			   n_iter, m_iter, left, right, diff, least_diff,
			   least_diff_m_iter, least_diff_n_iter); */
		}
	}

	/* if diff == 0, it means we found a perfect, m, and n value. */
	if (diff == 0) {
		/* pr_info("We found the correct, m, n values. \
		   m = %lld, n= %lld\n", m_iter, n_iter); */
		*m_set = m_iter;
		*n_set = n_iter;
		return val;
	} else {
		/* This means we didn't find a good m,n value. */
		*m_set = least_diff_m_iter;
		*n_set = least_diff_n_iter;
		ret_val = (((FREQ_MHZ(156) / *m_set)) * *n_set);
		/* pr_info("Didn't find a perfect m, n value, but will given \
		   the nearest possible value. m = %lld, n = %lld, \
		   freq = %ld\n", least_diff_m_iter,
		   least_diff_n_iter, ret_val) ; */
		return ret_val;
	}
}

u32 bcm21553_apps_pll_get_rate()
{
	u32 regval;
	regval = (readl(ADDR_CLKPWR_CLK_APPSPLL_DIVIDERS) & 0xFF00) >> 8;
	return FREQ_MHZ(26 * regval);
}


int clk_get_mipidsi_afe_div(int id)
{
	u32 shift = 0;
	u32 reg_val;
	switch(id)
	{
	case CAM1_CLK_DIV:
		shift = CAM1_CLK_DIV_SHIFT;
		break;

	case CAM2_CLK_DIV:
		shift = CAM1_CLK_DIV_SHIFT;
		break;

	case DSI_CLK_DIV:
		shift = DSI_CLK_DIV_SHIFT;
		break;

	default:
		return -EINVAL;
	}
	reg_val = readl(ADDR_CLKPWR_CLK_MIPI_DSI_AFE_DIV);
	reg_val &= MIPI_DSI_AFE_DIV_MASK << shift;
	reg_val >>= shift;
	return reg_val;
}
EXPORT_SYMBOL(clk_get_mipidsi_afe_div);

int clk_set_mipidsi_afe_div(int id, int div)
{
	u32 shift = 0;
	u32 reg_val;
	switch(id)
	{
	case CAM1_CLK_DIV:
		shift = CAM1_CLK_DIV_SHIFT;
		break;

	case CAM2_CLK_DIV:
		shift = CAM1_CLK_DIV_SHIFT;
		break;

	case DSI_CLK_DIV:
		shift = DSI_CLK_DIV_SHIFT;
		break;

	default:
		return -EINVAL;
	}
	reg_val = readl(ADDR_CLKPWR_CLK_MIPI_DSI_AFE_DIV);
	reg_val |= (div & MIPI_DSI_AFE_DIV_MASK) << shift;
	writel(reg_val, ADDR_CLKPWR_CLK_MIPI_DSI_AFE_DIV);
	return 0;
}
EXPORT_SYMBOL(clk_set_mipidsi_afe_div);

static int
brcm_clk_parse_string(const char *inputStr, char *clkName, u32 *opCode,
		      u32 *arg)
{
	int numArg;
	char tempStr[50];
	int ret = 2;

	numArg = sscanf(inputStr, "%s%s%u", clkName, tempStr, arg);

	if (numArg < 2) {
		return -1;
	}

	if (strcmp(tempStr, "enable") == 0) {
		*opCode = 1;
	}

	else if (strcmp(tempStr, "disable") == 0) {
		*opCode = 2;
	} else if (strcmp(tempStr, "getrate") == 0) {
		*opCode = 3;
	} else if (strcmp(tempStr, "setrate") == 0) {
		*opCode = 4;
		if (numArg < 3)
			return -1;
		ret = 3;
	}

	return ret;

}

static int
brcm_clk_proc_read(char *page, char **start,
		   off_t off, int count, int *eof, void *data)
{
	int i;
	struct clk *clk;
	int len = 0;
	char *pg = page;
	const u8 cp_clk_div[]  = {6,3,3};
	const u8 dsp_clk_div[] = {12,9,6,6,6,5};
	const u8 arm11_div[] = {4,4,3};
	u32 apps_pll_freq;
	u32 dsp_clk;
	u32 ahb;
	u32 ahb_fast;
	u32 sdram;
	u32 cp_clk;
	u32 v3d_clk;
	u32 arm11;
	u32 mode,dsp_mode,sdram_mode;

	apps_pll_freq = bcm21553_apps_pll_get_rate();
	mode = readl(ADDR_CLKPWR_CLK_ARMAHB_MODE) & 0x0F;
	/*I heat you Samsung shauld i fix that? */
	/*apps_pll_freq /= FREQ_MHZ(1);*/

	/* We assume that APPS PLL is active
	& mode value less than 0xC is not supported*/
	if(mode <= 0xC)
	{
		arm11	 = clk_armahb_reg_to_arm11_freq_mapping[mode];
		cp_clk	 = clk_armahb_reg_to_cp_freq_mapping[mode];
		ahb		 = clk_armahb_reg_to_ahb_freq_mapping[mode];
		ahb_fast = clk_armahb_reg_to_ahb_fast_freq_mapping[mode];
		v3d_clk	 = clk_armahb_reg_to_v3d_clk_mapping[mode];
	}
	else
	{
		arm11 = (apps_pll_freq*2)/arm11_div[mode -0xD];
		cp_clk = apps_pll_freq/cp_clk_div[mode -0xD];
		ahb = apps_pll_freq/9;
		ahb_fast = apps_pll_freq/6;
		v3d_clk = apps_pll_freq/4;
	}

	dsp_mode = readl(ADDR_CLKPWR_CLK_DSP_MODE) & 0x7;
	if(dsp_mode == 0)
		dsp_clk = 52; /*52 Mhz - only on main pll*/
	/* special case 932 ??!! nope :) it's 1372 :P */
	else if(FREQ_MHZ(apps_pll_freq) > FREQ_MHZ(1274) && dsp_mode == 6)
		dsp_clk = (apps_pll_freq*2)/9;
	else
		dsp_clk = apps_pll_freq/dsp_clk_div[dsp_mode -1];

	sdram_mode = readl(ADDR_CLKPWR_CLK_SDRAM_MODE);
	if(sdram_mode & CLK_SDRAM_SYC_MODE)
		sdram = ahb_fast;
	else
	{
		sdram = apps_pll_freq /
		    clk_sdram_mode_reg_to_appspll_divider[sdram_mode & 0x3];
	}

	pg += sprintf(pg,"Apps PLL = %uMhz\n",apps_pll_freq);
	pg += sprintf(pg,"ARM11 = %uMhz\n",arm11);
	pg += sprintf(pg,"AHB = %uMhz\n",ahb);
	pg += sprintf(pg,"AHB Fast = %uMhz\n",ahb_fast);
	pg += sprintf(pg,"V3D clk= %uMhz\n",v3d_clk);
	pg += sprintf(pg,"SDRAM clk= %uMhz  (%s)\n",sdram, (sdram_mode & 0x4) ? "Sync Mode" : "Async Mode");
	pg += sprintf(pg,"DSP clk= %uMhz\n",dsp_clk);
	pg += sprintf(pg,"CP-ARM= %uMhz\n\n",cp_clk);

	pg += sprintf(pg, "clk-name\tUsageCount\tCurrentRate\n");

	/* traverse through the clk list */
	for (i = 0; i < ARRAY_SIZE(lookups); i++) {
		clk = clk_get(NULL, lookups[i].con_id);

		if (!clk) {
			pr_info("clk_get() error !!! for %s\n",
				lookups[i].con_id);
			break;
		}
		pg +=
		    sprintf(pg, "%s\t\t%u\t\t%uKHz\n", lookups[i].con_id,
			    clk->cnt, (unsigned int)clk_get_rate(clk) / 1000);
	}

	*start = page;

	len = pg - page;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len : count;
}

static int
brcm_clk_proc_write(struct file *file,
		    const char *buffer, unsigned long count, void *data)
{
	int len;
	char inputStr[100];
	char clkName[10];
	u32 opCode = 0;
	u32 arg;
	int ret;
	int numArg;
	struct clk *clk;

	if (count > 100)
		len = 100;
	else
		len = count;

	if (copy_from_user(inputStr, buffer, len))
		return -EFAULT;

	numArg = brcm_clk_parse_string(inputStr, clkName, &opCode, &arg);

	clk = clk_get(NULL, clkName);
	if (numArg <= 0 || clk == NULL || IS_ERR(clk)) {
		pr_info("Invalid arguments !!!\n");
		return len;
	}
	switch (opCode) {
	case 1:
		ret = clk_enable(clk);
		pr_info("clk_enable() : retun value %i\n", ret);
		if (!ret) {
			if (clk->enable_reg)
				pr_info(" %s clk enable reg (%x) = %x\n",
					clkName,
					(u32) io_v2p((u32) clk->enable_reg),
					readl(clk->enable_reg));
		}
		break;

	case 2:
		clk_disable(clk);
		pr_info("clk_disable()\n");
		if (clk->enable_reg)
			pr_info(" %s clk enable reg (%x) = %x\n", clkName,
				(u32) io_v2p((u32) clk->enable_reg),
				readl(clk->enable_reg));
		break;

	case 3:
		pr_info("clk_get_rate() : %s rate = %u\n", clkName,
			(unsigned int)clk_get_rate(clk));
		break;

	case 4:
		ret = clk_set_rate(clk, arg);
		pr_info("clk_set_rate() : return value %i\n", ret);

		if (!ret) {
			pr_info("%s new rate = %u\n", clkName,
				(unsigned int)clk_get_rate(clk));
		}
		break;
	}
	return len;
}

static int __init clk_init(void)
{
	struct clk *clk;
	u32 regVal;
	int i;

	/* MobC00163909 */
	writel(0x00, ADDR_CLKPWR_CLK_POWERSWITCH_CTRL);

	/*To avoid panic in V3D disable call...*/
	if(readl(ADDR_CLKPWR_CLK_ARMAHB_MODE) == 0x0F)
		bcm215xx_set_appll_enable(1);
		//writel(0x01,ADDR_CLKPWR_CLK_APPSPLL_ENABLE);

	/* register the clock lookups */
	for (i = 0; i < ARRAY_SIZE(lookups); i++) {

		/*Make sure that all clocks are disabled by default */
		clk = lookups[i].clk;
		pr_info("%s:%s\n", __func__, lookups[i].con_id);
		if ((clk->flags & (BCM_CLK_ALWAYS_ENABLED |
				   BCM_CLK_NO_DISABLE_ON_INIT)) == 0) {
			if (clk->disable)
				clk->disable(clk);
			else if (clk->enable_reg) {
				regVal = readl(clk->enable_reg);
				if (clk->flags & BCM_CLK_INVERT_ENABLE) {
					regVal |= clk->enable_bit_mask;
					writel(regVal, clk->enable_reg);
				} else {
					regVal &= ~(clk->enable_bit_mask);
					writel(regVal, clk->enable_reg);
				}
			}
		}

		clkdev_add(&lookups[i]);
	}

	/* In turbo mode CLK_APPSPLL_ENABLE should be high (as per ASIC team) */
	if(readl(ADDR_CLKPWR_CLK_ARMAHB_MODE) == 0x0F)
	{
		clk = clk_get(NULL,BCM_CLK_APPSPLL_EN_STR_ID);
		if(IS_ERR(clk))
			return -ENODEV;
		clk_enable(clk);
		clk_put(clk);
	}
	brcm_proc_file = create_proc_entry("bcm21553_clks", 0644, NULL);

	if (brcm_proc_file) {
		brcm_proc_file->data = NULL;
		brcm_proc_file->read_proc = brcm_clk_proc_read;
		brcm_proc_file->write_proc = brcm_clk_proc_write;
/*              brcm_proc_file->owner = THIS_MODULE; */
		return 0;
	}
	return -ENODEV;
}

arch_initcall(clk_init);
