/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/mach-bcm215xx/board-athenaray.c
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
 * AthenaRay board specific driver definitions
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/cpufreq.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <mach/reg_irq.h>

#include <linux/serial_8250.h>
#include <mach/hardware.h>
#include <mach/reg_syscfg.h>
#include <mach/reg_auxmic.h>
#include <asm/mach/map.h>
#include <mach/setup.h>
#include <mach/gpt.h>
#include <linux/broadcom/gpt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/leds.h>

#if defined (CONFIG_ANDROID_PMEM)
#include <linux/android_pmem.h>
#include <linux/dma-mapping.h>
#define PMEM_ADSP_SIZE (1024 * 1024 * 8)
#endif

#ifdef CONFIG_MMC_BCM
#include <mach/sdio.h>
#include <mach/reg_clkpwr.h>
#endif

#if defined(CONFIG_MFD_MAX8986)
#include <linux/power_supply.h>
#include <linux/mfd/max8986/max8986.h>
#include <linux/broadcom/max8986/max8986-audio.h>
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <mach/usbctl.h>
#endif
#ifdef CONFIG_KEYBOARD_BCM
#include <mach/bcm_keymap.h>
#include <plat/bcm_keypad.h>
#endif
#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
#endif

#if defined (CONFIG_TOUCHSCREEN_TSC2007)
#include <linux/i2c/tsc2007.h>
#endif

#if defined (CONFIG_BCM_AUXADC)
#include <plat/bcm_auxadc.h>
#endif
#include "device.h"

#if defined(CONFIG_BRCM_HEADSET)  || defined(CONFIG_BRCM_HEADSET_MODULE)
#include <plat/brcm_headset_pd.h>
#endif

#include <linux/spinlock.h>
#include <linux/spi/spi.h>

#ifdef CONFIG_SPI
#include <plat/bcm_spi.h>
#endif

#include <plat/syscfg.h>
#include <plat/timer.h>

#if defined (CONFIG_BCM_PWM)
#include <plat/bcm_pwm_block.h>
#endif

#include <plat/bcm_i2c.h>

#ifdef CONFIG_BCM215XX_DSS
#include <plat/bcm_lcdc.h>
#endif

#define BCM2091_CLK_REQ_PASS_THOUGH 36
#include <linux/broadcom/types.h>
#include <linux/broadcom/bcmtypes.h>

#if defined(CONFIG_BRCM_FUSE_SYSPARM)
#include <linux/broadcom/bcm_fuse_sysparm.h>
#endif

#if defined(CONFIG_USB_ANDROID)
#include <linux/usb/android.h>
#endif

#define FREQ_MHZ(mhz)		((mhz)*1000UL*1000UL)
#define FREQ_KHZ(khz)		((khz)*1000UL)

/*
 * BITMAP to indicate the available GPTs on AP
 * Reset the bitmap to indicate the GPT not to be used on AP
 */
#define GPT_AVAIL_BITMAP         0x3F

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
void __init gpt_init(struct gpt_base_config *base_config);

//#ifdef CONFIG_BCM_4319_DRIVER

extern void bcmsdhc_sdio_host_force_scan(struct platform_device *pdev, bool on);
void bcm_wlan_power_on(int val);
void bcm_wlan_power_off(int val);
int bcm_set_core_power(unsigned int bcm_core, unsigned int pow_on,
		       unsigned int reset);
//#endif /* 4319 */
#ifdef CONFIG_MMC_BCM
extern int bcmsdhc_cfg_card_detect(void __iomem *ioaddr, u8 ctrl_slot);
extern int bcmsdhc_external_reset(void __iomem *ioaddr, u8 ctrl_slot);
extern int bcmsdhc_enable_int(void __iomem *ioaddr, u8 ctrl_slot);
extern void *cam_mempool_base;
int bcmsdhc_set_sys_interface(u8 ctrl_slot);
int bcmsdhc_request_cfg_pullup(void __iomem *ioaddr, u8 ctrl_slot);
int bcmsdhc_finish_cfg_pullup(void __iomem *ioaddr, u8 ctrl_slot);

static struct bcmsdhc_platform_data bcm21553_sdhc_data1 = {
	.base_clk = FREQ_MHZ(50),
	.flags = SDHC_DEVTYPE_SDIO | SDHC_MANUAL_SUSPEND_RESUME,
	.cd_pullup_cfg = SDCD_PULLUP | SDCD_UPDOWN_ENABLE,
	.irq_cd = 53,
	.regl_id = "vddo_sdio1",
	.reg_minuv = 1800000,
	.reg_maxuv = 1800000,
	.syscfg_interface = board_sysconfig,
	.cfg_card_detect = bcmsdhc_cfg_card_detect,
	.external_reset = bcmsdhc_external_reset,
	.enable_int = bcmsdhc_enable_int,
};

#if !defined(CONFIG_MTD_ONENAND) && !defined(CONFIG_MTD_NAND)
/*
 * SDHC2 is used for eMMC and SDHC2 shares pin mux with FLASH(OneNAND)
 * So both OneNAND and SDHC2 cannot co-exist
 */

static struct bcmsdhc_platform_data bcm21553_sdhc_data2 = {
	.base_clk = FREQ_MHZ(50),
	.flags = SDHC_DEVTYPE_EMMC | SDIO_CARD_ALWAYS_PRESENT,
	.cd_pullup_cfg = SDCD_PULLUP | SDCD_UPDOWN_ENABLE,
	.irq_cd = -1,
	.syscfg_interface = board_sysconfig,
	.cfg_card_detect = bcmsdhc_cfg_card_detect,
	.external_reset = bcmsdhc_external_reset,
	.enable_int = bcmsdhc_enable_int,
};
#endif /*CONFIG_MTD_ONENAND */

static struct bcmsdhc_platform_data bcm21553_sdhc_data3 = {
	.base_clk = FREQ_MHZ(50),
	.flags = SDHC_DEVTYPE_SD,
	.cd_pullup_cfg = SDCD_PULLUP | SDCD_UPDOWN_ENABLE,
	.irq_cd = 32,
	.regl_id = "sd_b_vdd",
	.syscfg_interface = board_sysconfig,
	.cfg_card_detect = bcmsdhc_cfg_card_detect,
	.external_reset = bcmsdhc_external_reset,
	.enable_int = bcmsdhc_enable_int,
};
#endif /*CONFIG_MMC_BCM */

//#ifdef CONFIG_BCM_4319_DRIVER

#define BCM4325_BT 1
#define BCM4325_WLAN 2
#define BCM4325_BT_RESET 31
#define BCM4325_WLAN_RESET 31
#define GPIO_WLAN_BT_REG_ON 31
#define SDIO1_WLAN_MUX_TTL 35
#define BT_VREG_CTRL	21
#define BT_RST_CHECK	20
int bcm_set_core_power(unsigned int bcm_core, unsigned int pow_on,
		       unsigned int reset)
{
	unsigned gpio_rst, gpio_rst_another;

	if ((reset != 2) && (reset != 1)) {
		pr_info("%s: Error!! BAD Argument. ", __FUNCTION__);
		return -EINVAL;
	}

	switch (bcm_core) {
	case BCM4325_BT:
		gpio_rst = BCM4325_BT_RESET;
		gpio_rst_another = BCM4325_WLAN_RESET;
		break;

	case BCM4325_WLAN:
		gpio_rst = BCM4325_WLAN_RESET;
		gpio_rst_another = BCM4325_BT_RESET;
		break;

	default:
		pr_err("bcm_power: Unknown bcm core!\n");
		return -1;
	}

/*    mutex_lock(&bcm4325_pwr_lock); */

	/* Since reg on is coupled, check whether the other core is ON before
	   touching it */
	if ((gpio_get_value(gpio_rst_another) == 0) && ((reset == 1)|| (reset == 2))) {

		gpio_direction_output(SDIO1_WLAN_MUX_TTL, 0);
		msleep(500);


		/* Make GPIO31 to out direction, and set value 0 */
			writel((readl(ADDR_SYSCFG_IOCR5) & ~SYSCFG_IOCR5_GPIO31_MUX(0x3)),
					ADDR_SYSCFG_IOCR5);

			gpio_request(GPIO_WLAN_BT_REG_ON, "sdio1_wlan_reset");
			gpio_direction_output(GPIO_WLAN_BT_REG_ON, pow_on);



		/* enable WLAN_BT_REG_ON */
///		gpio_direction_output(GPIO_WLAN_BT_REG_ON, pow_on);
		pr_info
		    ("bcm_power: Set WLAN_BT_REG_ON %s because %s is OFF now.\n",
		     gpio_get_value(GPIO_WLAN_BT_REG_ON) ? "High" : "Low",
		     bcm_core ? "BT" : "WLAN");
		msleep(150);
	}
	/* enable specified core */
	gpio_direction_output(gpio_rst, pow_on);
	pr_info("bcm_power: Set %s %s\n",
		bcm_core ? "WLAN_RESET" : "BT_RESET",
		gpio_get_value(gpio_rst) ? "High [chip out of reset]" :
		"Low [put into reset]");

			gpio_free(GPIO_WLAN_BT_REG_ON);


/*    mutex_unlock(&bcm4325_pwr_lock); */

	return 0;
}

#define BCM_RESET 2
#define BCM_POWER 1
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/** @addtogroup BoardBravaAPIGroup
	@{
*/
/**
* @brief	Power ON the WLAN
*
* @param val	Start/Power ON
*/
void bcm_wlan_power_on(int val)
{

	int err = 0;
	pr_info("%s: Enter.\n ", __FUNCTION__);



		if( (gpio_get_value(BT_RST_CHECK) == 0) )
		{
			gpio_direction_output(BT_VREG_CTRL, 1);
			msleep(500);
		}


	switch (val) {
	case BCM_POWER:
		pr_info("SHRI %s: WIFI POWER UP!!  \n ", __FUNCTION__);
		err = bcm_set_core_power(BCM4325_WLAN, TRUE, val);
		if (err < 0)
			pr_info("%s: FAILED!!\n ", __FUNCTION__);
		bcmsdhc_sdio_host_force_scan(&bcm21553_sdhc_slot1, true);
		break;
	case BCM_RESET:
		pr_info("%s: WIFI DRIVER START!!\n ", __FUNCTION__);
		err = bcm_set_core_power(BCM4325_WLAN, TRUE, val);
		if (err < 0)
			pr_info("%s: FAILED!!\n ", __FUNCTION__);
		break;
	default:
		pr_info("%s: INVALID ARG!!\n ", __FUNCTION__);





		}
	/* Note: that the platform device struct has to be exported from where it is defined */
	/* The below function would induce a forced mmc_rescan to detect the newly */
	/* powered up card. */

}


/**
* @brief	Power OFF the WLAN
*
* @param val	Stop/Power OFF
*/
void bcm_wlan_power_off(int val)
{

	int err = 0;
	pr_info("%s: Enter.\n ", __FUNCTION__);



	switch (val) {
	case BCM_POWER:
		pr_info("%s: WIFI POWER DOWN!\n ", __FUNCTION__);
		err = bcm_set_core_power(BCM4325_WLAN, FALSE, val);
		if (err < 0)
			pr_info("%s: FAILED!!\n ", __FUNCTION__);
		bcmsdhc_sdio_host_force_scan(&bcm21553_sdhc_slot1, false);
		break;
	case BCM_RESET:
		pr_info("%s: WIFI DRIVER STOP!!\n ", __FUNCTION__);
		err = bcm_set_core_power(BCM4325_WLAN, FALSE, val);
		if (err < 0)
			pr_info("%s: FAILED!!\n ", __FUNCTION__);
		break;
	default:
		pr_info("%s: INVALID ARG!!\n ", __FUNCTION__);

	}



		if( (gpio_get_value(BT_RST_CHECK) == 0) )
		{
			gpio_direction_output(BT_VREG_CTRL, 0);
			msleep(500);
		}


	/* Note: that the platform device struct has to be exported from where it is defined */
	/* The below function would induce a forced mmc_rescan to detect the newly */
	/* powered up card. */

}
/** @} */
EXPORT_SYMBOL(bcm_wlan_power_on);
EXPORT_SYMBOL(bcm_wlan_power_off);

//#endif /* 4319 */
#ifdef CONFIG_KEYBOARD_BCM
static void bcm_keypad_config_iocr(int row, int col)
{
	row = (1 << row) - 1;
	col = (1 << col) - 1;
	/* Set lower "row" & "col" number of bits to 1 to indicate configuration of keypad */
	writel((SYSCFG_IOCR1_KEY_ROW(row) | SYSCFG_IOCR1_KEY_COL(col)),
				ADDR_SYSCFG_IOCR1);
}

static struct bcm_keymap newKeymap[] = {
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "Home Key", KEY_HOME},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "key left", KEY_LEFT},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "key menus", KEY_MENUS},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "key e", KEY_E},
	{BCM_KEY_ROW_0, BCM_KEY_COL_4, "key w", KEY_W},
	{BCM_KEY_ROW_0, BCM_KEY_COL_5, "key q", KEY_Q},
	{BCM_KEY_ROW_0, BCM_KEY_COL_6, "key h", KEY_H},
	{BCM_KEY_ROW_0, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_0, "key up", KEY_UP},
	{BCM_KEY_ROW_1, BCM_KEY_COL_1, "key enter", KEY_ENTER},
	{BCM_KEY_ROW_1, BCM_KEY_COL_2, "key down", KEY_DOWN},
	{BCM_KEY_ROW_1, BCM_KEY_COL_3, "key d", KEY_D},
	{BCM_KEY_ROW_1, BCM_KEY_COL_4, "key s", KEY_S},
	{BCM_KEY_ROW_1, BCM_KEY_COL_5, "key a", KEY_A},
	{BCM_KEY_ROW_1, BCM_KEY_COL_6, "key f", KEY_F},
	{BCM_KEY_ROW_1, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_0, "Back Key", KEY_BACK},
	{BCM_KEY_ROW_2, BCM_KEY_COL_1, "key right", KEY_RIGHT},
	{BCM_KEY_ROW_2, BCM_KEY_COL_2, "key c", KEY_C},
	{BCM_KEY_ROW_2, BCM_KEY_COL_3, "key x", KEY_X},
	{BCM_KEY_ROW_2, BCM_KEY_COL_4, "key z", KEY_Z},
	{BCM_KEY_ROW_2, BCM_KEY_COL_5, "key lshift", KEY_LEFTSHIFT},
	{BCM_KEY_ROW_2, BCM_KEY_COL_6, "key t", KEY_T},
	{BCM_KEY_ROW_2, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_1, "key space", KEY_SPACE},
	{BCM_KEY_ROW_3, BCM_KEY_COL_2, "key tab", KEY_TAB},
	{BCM_KEY_ROW_3, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_6, "key v", KEY_V},
	{BCM_KEY_ROW_3, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_0, "key r", KEY_R},
	{BCM_KEY_ROW_4, BCM_KEY_COL_1, "key dot", KEY_DOT},
	{BCM_KEY_ROW_4, BCM_KEY_COL_2, "key p", KEY_P},
	{BCM_KEY_ROW_4, BCM_KEY_COL_3, "key o", KEY_O},
	{BCM_KEY_ROW_4, BCM_KEY_COL_4, "key i", KEY_I},
	{BCM_KEY_ROW_4, BCM_KEY_COL_5, "key u", KEY_U},
	{BCM_KEY_ROW_4, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_0, "key send", KEY_SEND},
	{BCM_KEY_ROW_5, BCM_KEY_COL_1, "key semicol", KEY_SEMICOLON},
	{BCM_KEY_ROW_5, BCM_KEY_COL_2, "key bkspace", KEY_BACKSPACE},
	{BCM_KEY_ROW_5, BCM_KEY_COL_3, "key l", KEY_L},
	{BCM_KEY_ROW_5, BCM_KEY_COL_4, "key k", KEY_K},
	{BCM_KEY_ROW_5, BCM_KEY_COL_5, "key j", KEY_J},
	{BCM_KEY_ROW_5, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_0, "key y", KEY_Y},
	{BCM_KEY_ROW_6, BCM_KEY_COL_1, "key b", KEY_B},
	{BCM_KEY_ROW_6, BCM_KEY_COL_2, "key g", KEY_G},
	{BCM_KEY_ROW_6, BCM_KEY_COL_3, "key comma", KEY_COMMA},
	{BCM_KEY_ROW_6, BCM_KEY_COL_4, "key m", KEY_M},
	{BCM_KEY_ROW_6, BCM_KEY_COL_5, "key n", KEY_N},
	{BCM_KEY_ROW_6, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_7, "unused", 0},
};

static struct bcm_keypad_platform_info bcm215xx_keypad_data = {
	.row_num = 7,
	.col_num = 7,
	.keymap = newKeymap,
	.iocr_cfg = bcm_keypad_config_iocr,
	.bcm_keypad_base = (void *)__iomem IO_ADDRESS(BCM21553_KEYPAD_BASE),
};

#endif

#ifdef CONFIG_BACKLIGHT_PWM
struct platform_device bcm_backlight_devices = {
	.name = "pwm-backlight",
	.id = 0,
};

static struct platform_pwm_backlight_data bcm_backlight_data = {
	/* backlight */
	.pwm_id = 1,
	.max_brightness = 32,	/* Android calibrates to 32 levels*/
	.dft_brightness = 32,
	/*set pwm clock rate to 200Hz. min value of the operating range of the max1561
         *step-up converter(200Hz - 200KHz) which takes this PWM as an input signal*/
	.pwm_period_ns =  5000000,
};
#endif /*CONFIG_BACKLIGHT_PWM */

#ifdef CONFIG_BCM_PWM
static struct pwm_platform_data pwm_dev = {
	.max_pwm_id = 6,
	.syscfg_inf = board_sysconfig,
};
#endif

#if defined (CONFIG_MTD_ONENAND_BCM_XBOARD)
static struct resource onenand_resources[] = {
	{
	 .start = IO_ADDRESS(BCM21553_ONENAND_BASE),
	 .end = IO_ADDRESS(BCM21553_ONENAND_BASE) + SZ_128K - 1,
	 .flags = IORESOURCE_MEM,
	 },
};

struct flash_platform_data onenand_data = {
	.parts = NULL,
	.nr_parts = 0,
};

struct platform_device athenaray_device_onenand = {
	.name = "onenand",
	.resource = onenand_resources,
	.num_resources = ARRAY_SIZE(onenand_resources),
	.dev = {
		.platform_data = &onenand_data,
		},
};
#endif

#if defined (CONFIG_MTD_NAND_BRCM_NVSRAM)
static struct resource nand_resources[] = {
	{
	 .start = BCM21553_NAND_BASE,
	 .end = BCM21553_NAND_BASE + SZ_32M - 1,
	 .flags = IORESOURCE_MEM,
	 },
};

static struct flash_platform_data nand_data = {
	.parts = NULL,
	.nr_parts = 0,
};

static struct platform_device bcm21553_device_nand = {
	.name = "bcm-nand",
	.resource = nand_resources,
	.num_resources = ARRAY_SIZE(nand_resources),
	.dev = {
		.platform_data = &nand_data,
		},
};
#endif

#if defined (CONFIG_ANDROID_PMEM)
static struct android_pmem_platform_data android_pmem_pdata = {
        .name = "pmem_adsp",
        .no_allocator = 0,
        .cached = 0,
};

static struct platform_device android_pmem_device = {
        .name = "android_pmem",
        .id = 0,
        .dev = {.platform_data = &android_pmem_pdata},
};
#endif

#if defined (CONFIG_BRCM_HAL_CAM)
struct i2c_slave_platform_data stv_cam_pdata = {
        .i2c_spd = I2C_SPD_400K,
};

static struct i2c_board_info __initdata bcm21553_cam_i2c_board_info[] = {
        {I2C_BOARD_INFO("cami2c", (0x20 >> 1)),
         .platform_data = (void *)&stv_cam_pdata,
#if defined (CONFIG_CAM_CSI2)
         .irq = IRQ_CAM_CSI2,
#endif
#if defined (CONFIG_CAM_CPI)
         .irq = IRQ_CAM_CCP2,
#endif
         },
};
#endif

#if defined(CONFIG_MFD_MAX8986)

#define SYSPARM_PMU_REG(index, dst)					\
	do {								\
		unsigned int parm;					\
		int ret = SYSPARM_GetPMURegSettings(index, &parm);	\
		if (ret == 0) {						\
			pr_info("sysparm: %s: %x\n", #index, parm);	\
			dst = parm;					\
		}							\
	} while (0)

/* Indexes of PMU registers in syparm */
enum {
	PMU_REG_0X07_PM_A1OPMODCTRL = 0x07,
	PMU_REG_0X08_PM_D1OPMODCTRL = 0x08,
	PMU_REG_0X09_PM_A8OPMODCTRL = 0x09,
	PMU_REG_0X0A_PM_A2OPMODCTRL = 0x0A,
	PMU_REG_0X0B_PM_H1OPMODCTRL = 0x0B,
	PMU_REG_0X0C_PM_H2OPMODCTRL = 0x0C,
	PMU_REG_0X0D_PM_D2OPMODCTRL = 0x0D,
	PMU_REG_0X0E_PM_A5OPMODCTRL = 0x0E,
	PMU_REG_0X0F_PM_A4OPMODCTRL = 0x0F,
	PMU_REG_0X10_PM_LVOPMODCTRL = 0x10,
	PMU_REG_0X11_PM_SIMOPMODCTRL = 0x11,
	PMU_REG_0X15_PM_CSROPMODCTRL = 0x15,
	PMU_REG_0X17_PM_IOSROPMODCTRL = 0x17,
	PMU_REG_0x1B_PM_MBCCTRL4 = 0x1B,
	PMU_REG_0X2B_PM_A6OPMODCTRL = 0x2B,
	PMU_REG_0X2C_PM_A3OPMODCTRL = 0x2C,
	PMU_REG_0X2D_PM_AX1OPMODCTRL = 0x2D,
	PMU_REG_0X2E_PM_A7OPMODCTRL = 0x2E,
	PMU_REG_0X41_PM_D3OPMODCTRL = 0x41,
	PMU_REG_0x43_PM_D4OPMODCTRL = 0x43,
	PMU_REG_0X45_PM_A9OPMODCTRL = 0x45,
	PMU_REG_0X47_PM_TSROPMODCTRL = 0x47,
};

/*
DLDO1 supplies - compass, accelarometer, SD-A VDD
Max4996 mux, vddo_sdio1
*/
static struct regulator_consumer_supply dldo1_consumers[] = {
	{
		.dev = NULL,
		.supply = "sd_a_vdd",
	},
	{
		.dev = NULL,
		.supply = "max4996_vcc",
	},
	{
		.dev = NULL,
		.supply = "vddo_sdio3",
	},
	{
		.dev = NULL,
		.supply = "compass_vdd",
	},
	{
		.dev = NULL,
		.supply = "accel_vdd",
	},

};

static struct regulator_init_data dldo1_init_data = {
	.constraints = {
		.min_uV = 2500000,
		.max_uV = 3000000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on = 0,
		.boot_on = 0,
	},
	.num_consumer_supplies = ARRAY_SIZE(dldo1_consumers),
	.consumer_supplies = dldo1_consumers,
};

static struct regulator_consumer_supply dldo3_consumers[] = {
	{
		.dev = NULL,
		.supply = "cam_vdd",
	},
	{
		.dev = NULL,
		.supply = "haptic_pwm",
	},
};

static struct regulator_init_data dldo3_init_data = {
	.constraints = {
		.min_uV = 1800000,
		.max_uV = 3000000,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.always_on = 0,
		.boot_on = 0,
	},
	.num_consumer_supplies = ARRAY_SIZE(dldo3_consumers),
	.consumer_supplies = dldo3_consumers,
};

static struct regulator_consumer_supply aldo8_consumers[] = {
	{
		.dev = NULL,
		.supply = "cam_2v8",
	},
};

static struct regulator_init_data aldo8_init_data = {
	.constraints = {
		.min_uV = 2500000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.always_on = 0,
		.boot_on = 0,
	},
	.num_consumer_supplies = ARRAY_SIZE(aldo8_consumers),
	.consumer_supplies = aldo8_consumers,
};

static struct regulator_consumer_supply sim_consumers[] = {
	{
		.dev	= NULL,
		.supply	= "sim_vcc",
	},
};

static struct regulator_init_data sim_init_data = {
	.constraints = {
		.min_uV = 1800000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.always_on = 0,
		.boot_on = 0,
	},
	.num_consumer_supplies = ARRAY_SIZE(sim_consumers),
	.consumer_supplies = sim_consumers,
};

static struct regulator_consumer_supply csr_nm1_consumers[] = {
	{
		.dev	= NULL,
		.supply	= "csr_nm1",
	},
};

static struct regulator_init_data csr_nm1_init_data = {
	.constraints = {
		.min_uV = 900000,
		.max_uV = 1380000,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.boot_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(csr_nm1_consumers),
	.consumer_supplies = csr_nm1_consumers,
};

static struct regulator_consumer_supply csr_nm2_consumers[] = {
	{
		.dev	= NULL,
		.supply	= "csr_nm2",
	},
};

static struct regulator_init_data csr_nm2_init_data = {
	.constraints = {
		.min_uV = 900000,
		.max_uV = 1380000,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.boot_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(csr_nm2_consumers),
	.consumer_supplies = csr_nm2_consumers,
};

static struct max8986_regl_init_data bcm21553_regulators[] = {
	{
		.regl_id = MAX8986_REGL_SIMLDO,
		.dsm_opmode = MAX8986_REGL_LPM_IN_DSM,
		.init_data = &sim_init_data,
	},
	{
		.regl_id = MAX8986_REGL_CSR_NM1,
		.dsm_opmode = MAX8986_REGL_LPM_IN_DSM,
		.init_data = &csr_nm1_init_data,
	},
	{
		.regl_id = MAX8986_REGL_CSR_NM2,
		.dsm_opmode = MAX8986_REGL_LPM_IN_DSM,
		.init_data = &csr_nm2_init_data,
	},
};

static struct max8986_regl_pdata regl_pdata = {
	.num_regulators = ARRAY_SIZE(bcm21553_regulators),
	.regl_init =  bcm21553_regulators,
	.regl_default_pmmode = {
		[MAX8986_REGL_ALDO1]	= 0x00,
		[MAX8986_REGL_ALDO2]	= 0x00,
		[MAX8986_REGL_ALDO3]	= 0x00,
		[MAX8986_REGL_ALDO4]	= 0x00,
		[MAX8986_REGL_ALDO5]	= 0x00,
		[MAX8986_REGL_ALDO6]	= 0x00,
		[MAX8986_REGL_ALDO7]	= 0x00,
		[MAX8986_REGL_ALDO8]	= 0x00,
		[MAX8986_REGL_ALDO9]	= 0x00,
		[MAX8986_REGL_DLDO1]	= 0x00,
		[MAX8986_REGL_DLDO1]	= 0x00,
		[MAX8986_REGL_DLDO2]	= 0x00,
		[MAX8986_REGL_DLDO3]	= 0x00,
		[MAX8986_REGL_DLDO4]	= 0x00,
		[MAX8986_REGL_HCLDO1]	= 0x00,
		[MAX8986_REGL_HCLDO2]	= 0x00,
		[MAX8986_REGL_LVLDO]	= 0x00,
		[MAX8986_REGL_SIMLDO]	= 0xAA,
		[MAX8986_REGL_AUXLDO1]	= 0x00,
		[MAX8986_REGL_TSRLDO]	= 0x00,
		[MAX8986_REGL_CSR_LPM]	= 0xC1, /*USE MAX8986_REGL_CSR_LPM set CSR op mode*/
		[MAX8986_REGL_IOSR]	= 0x11,
	},
};

static void __init update_regl_sysparm(void)
{
#if defined(CONFIG_BRCM_FUSE_SYSPARM)

#endif /* CONFIG_BRCM_FUSE_SYSPARM */
}

/*Default settings for audio amp */
static struct max8986_audio_pdata audio_pdata = {
	.ina_def_mode = MAX8986_INPUT_MODE_SINGLE_ENDED,
	.inb_def_mode = MAX8986_INPUT_MODE_DIFFERENTIAL,
	.ina_def_preampgain = PREAMP_GAIN_NEG_3DB,
	.inb_def_preampgain = PREAMP_GAIN_NEG_3DB,

	.lhs_def_mixer_in = MAX8986_MIXER_INA1,
	.rhs_def_mixer_in = MAX8986_MIXER_INA2,
	.ihf_def_mixer_in = MAX8986_MIXER_INB2,

	.hs_input_path = MAX8986_INPUTA,
	.ihf_input_path = MAX8986_INPUTB,
};
static struct max8986_power_pdata power_pdata = {
	.usb_charging_cc = MAX8986_CHARGING_CURR_500MA,
	.wac_charging_cc = MAX8986_CHARGING_CURR_950MA,
	.eoc_current = MAX8986_EOC_60MA,

	.batt_technology = POWER_SUPPLY_TECHNOLOGY_LION,
};

struct max8986_muic_pdata muic_data = {
	.mic_insert_adc_val = PMU_MUIC_ADC_OUTPUT_1M,
	.mic_btn_press_adc_val = PMU_MUIC_ADC_OUTPUT_2K,
};


static void __init update_power_sysparm(void)
{
#if defined(CONFIG_BRCM_FUSE_SYSPARM)
	SYSPARM_PMU_REG(PMU_REG_0x1B_PM_MBCCTRL4, power_pdata.usb_charging_cc);
	SYSPARM_PMU_REG(PMU_REG_0x1B_PM_MBCCTRL4, power_pdata.wac_charging_cc);
#endif /* CONFIG_BRCM_FUSE_SYSPARM */
}

/*****************************************************************************
 * platform specific usb interface routines for the power driver
 *****************************************************************************/
/* indicates whether the usb stack is ready or not */
static bool usb_driver_initialized;
/* indicates whether usb charger is detected or not */
static bool usb_charger_detected;
extern void Android_usb_cable_connection(bool is_connected);
extern void dwc_otg_pcd_StartUsb(int is_on);
extern void Android_PMU_USB_Start(void);
extern void Android_PMU_USB_Stop(void);


/* if both the power driver and the usb driver are ready, then start the
 * enumeration seqence.
 */
static void pmu_usb_start(void)
{
	pr_info("%s\n", __func__);

#ifdef CONFIG_USB_ANDROID
	if (usb_driver_initialized == true && usb_charger_detected == true)
	{
		Android_usb_cable_connection(1);
		Android_PMU_USB_Start();
		pr_info("%s: usb stack is UP\n", __func__);
	} else
	{
		Android_usb_cable_connection(0);
		pr_info("%s: usb stack is not up\n", __func__);
	}
#endif
}

static void pmu_usb_stop(void)
{
	pr_info("%s\n", __func__);
#ifdef CONFIG_USB_DWC_OTG
	Android_usb_cable_connection(0);
	Android_PMU_USB_Stop();
#endif
}

/*****************************************************************************
 * USB driver call backs
 *****************************************************************************/

/* this function is called from the usb driver once the USB reset event is
 * detected.
 */
void pmu_usb_enum_started(void)
{
	pr_info("%s\n", __func__);
}
EXPORT_SYMBOL(pmu_usb_enum_started);

/* this function is called by the usb driver once the usb stack init is
 * completed.
 */
void pmu_usb_driver_initialized(void)
{
	pr_info("%s\n", __func__);

	/* start usb enumeration if this function is called for the first
	 * time
	 */
	if (usb_driver_initialized == false) {
		usb_driver_initialized = true;
		pmu_usb_start();
	}
}
EXPORT_SYMBOL(pmu_usb_driver_initialized);

#define PMU_IRQ_GPIO    (34)
static u32 pmu_event_callback(int event, int param)
{
	u32 ret = 0;
	/*
	bat_adc_volt_mul calculated as follows:
	V(mV) = ADC *((R1+R2)/ R2)*( ADC input range /10-bit ADC)
	V(mV) = ADC * ((750K+243K)/ 243K)*(1200/1023)
	V(mV) = ADC * (4.08642)*(1.173021)
	V(mV) = ADC * (4.793454)
	*/
	const u32 bat_adc_volt_mul = 4793;
	static u16 bat_vol[] = {3300, 3350, 3400, 3450, 3500, 3550, 3600, 3650, 3700, 3750, 3800, 3850, 3900, 3950, 4000, 4050, 4100, 4150, 4200};
	static u8 bat_per[] = {0, 6, 11, 16, 20, 26, 32, 37, 42, 50, 56, 62, 68, 76, 83, 88, 92, 96, 100};

	switch (event) {

	case PMU_EVENT_INIT_PLATFORM:
	pr_info("%s: PMU: init hardware\n", __func__);

	if (gpio_request(PMU_IRQ_GPIO, "pmu_irq") == 0) {
		gpio_direction_input(PMU_IRQ_GPIO);
		bcm_gpio_pull_up_down_enable(PMU_IRQ_GPIO, true);
		bcm_gpio_pull_up(PMU_IRQ_GPIO, true);

		set_irq_type(GPIO_TO_IRQ(PMU_IRQ_GPIO), IRQF_TRIGGER_FALLING);
	} else {
		pr_err("%s: failed to allocate GPIO for PMU IRQ\n",
			__func__);
	}
	/* init batt params */
	power_pdata.batt_lvl_tbl.num_entries = ARRAY_SIZE(bat_vol);
	power_pdata.batt_lvl_tbl.bat_percentage = bat_per;
	power_pdata.batt_lvl_tbl.bat_vol = bat_vol;
	break;

	case PMU_EVENT_BATT_ADC_TO_VOLTAGE:
		ret = param*bat_adc_volt_mul;
		break;

	case PMU_EVENT_CHARGER_INSERT:
	pr_info("%s: PMU_EVENT_CHARGER_INSERT\n", __func__);

	/* try to start the usb enumeration process. this may not succeed if
	 * the usb stack is still not up.
	 */
	if (param == PMU_MUIC_CHGTYP_USB ||
			param == PMU_MUIC_CHGTYP_DOWNSTREAM_PORT)
	{
		usb_charger_detected = true;
		pmu_usb_start();
	}
	break;

	case PMU_EVENT_CHARGER_REMOVE:
	pr_info("%s: PMU_EVENT_CHARGER_REMOVE\n", __func__);
	if (param == PMU_MUIC_CHGTYP_USB ||
			param == PMU_MUIC_CHGTYP_DOWNSTREAM_PORT)
	{
		usb_charger_detected = false;
		pmu_usb_stop();
	}
	break;

	default:
	pr_err("%s: unrecognized event: %d\n", __func__, event);
	break;

	}
	return ret;
}

struct max8986_platform_data max8986_pdata = {
	.i2c_pdata = {.i2c_spd = I2C_SPD_400K,},
	.flags = MAX8986_USE_REGULATORS | MAX8986_REGISTER_POWER_OFF |
		MAX8986_ENABLE_AUDIO | MAX8986_USE_PONKEY | MAX8986_ENABLE_MUIC |
		MAX8986_USE_RTC | MAX8986_USE_POWER | MAX8986_ENABLE_DVS,
	.pmu_event_cb = pmu_event_callback,
	.regulators = &regl_pdata,
	.audio_pdata = &audio_pdata,
	.power = &power_pdata,
	.muic_pdata = &muic_data,
	/* CSR Voltage data */
	.csr_nm_volt = CSR_VOUT_1_36V,
	.csr_nm2_volt = -1, /*AVS driver will set this*/
	.csr_lpm_volt = CSR_VOUT_0_90V,
};
#endif /*CONFIG_MFD_MAX8986*/

#ifdef CONFIG_BCM_AUXADC
static struct bcm_plat_auxadc adcplat = {
	.readmask = 0x3FF,
	.croff = 0,
	.rdoff = 8,
	.ready = (0x1 << 15),
	.start = (0x1 << 3),
	.auxpm = 0,
	.regoff = (0x1 << 23),
	.bgoff = (0x1 << 22),
};
#endif

#if defined (CONFIG_TOUCHSCREEN_TSC2007)
struct tsc2007_platform_data tsc2007_pdata;
#endif

#if defined (CONFIG_TOUCHSCREEN_TSC2007)
#define PEN_IRQ_GPIO (30)
/*
 * This Function returns the pen down state of the penirq pin
 * parameters:
 * input:
 * pin:irq number
 * output:
 * irq line status
 */

static int tsc2007_pen_down_state(void)
{
	return (gpio_get_value(PEN_IRQ_GPIO)) ? 0 : 1;
}

/*  This Funtion detects the proper PENIRQ line.
 *  It return the GPIO line ID to which the PENIRQ is connected
 *  & sets the GPIO Line as input.
 *  parameters:
 *  output:
 *  irq number of the correspoding penirq.
 */

static void tsc2007_clear_penirq(void)
{
	return;
}

static int tsc2007_init_platform_hw(void)
{
	pr_info("tsc2007_init_platform_hw\n");
	gpio_request(PEN_IRQ_GPIO, "tsc2007");
	gpio_direction_input(PEN_IRQ_GPIO);
	set_irq_type(GPIO_TO_IRQ(PEN_IRQ_GPIO), IRQF_TRIGGER_FALLING);
	return 0;
}

static void tsc2007_exit_platform_hw(void)
{
}

/* Computes the x value based on the screen orientation */
int tsc2007_get_x_value(struct tsc2007_platform_data *pdata, int x, u32 max_val)
{

	return ((pdata->validx_right - pdata->validx_left) -
		(x - pdata->validx_left)) * max_val /
	    (pdata->validx_right - pdata->validx_left);
}

/* Computes the y value based on the screen orientation */
int tsc2007_get_y_value(struct tsc2007_platform_data *pdata, int y, u32 max_val)
{
	return ((pdata->validy_down - pdata->validy_up) -
		(y - pdata->validy_up)) * max_val /
	    (pdata->validy_down - pdata->validy_up);
}
#endif

#if defined(CONFIG_BRCM_HEADSET)  || defined(CONFIG_BRCM_HEADSET_MODULE)

#define HEADSET_DET_GPIO        33    /* HEADSET DET */
void check_hs_state (int *headset_state)
{
	*headset_state = gpio_get_value(HEADSET_DET_GPIO);
}

static struct brcm_headset_pd headset_pd = {
	.hsirq		= 0,
	.hsbirq		= IRQ_MICON,
	.check_hs_state = check_hs_state,
	.hsgpio		= HEADSET_DET_GPIO,
	.debounce_ms    = 400,
    .keypress_threshold_lp  = 0x5200,
    .keypress_threshold_fp  = 0x6000,
};
#endif

#if defined(CONFIG_I2C_BCM1160)
static struct i2c_board_info __initdata athenaray_i2c1_board_info[] = {
#if defined (CONFIG_TOUCHSCREEN_TSC2007)
	{I2C_BOARD_INFO("tsc2007", 0x48),
	 .platform_data = (void *)&tsc2007_pdata,
	 .irq = GPIO_TO_IRQ(30),
	 },
#endif
};

#endif

static struct i2c_board_info __initdata athenaray_i2c3_board_info[] = {
#if defined(CONFIG_MFD_MAX8986)
	{
		I2C_BOARD_INFO("max8986", 0x08),
		.platform_data = (void *)&max8986_pdata,
		.irq = GPIO_TO_IRQ(34),
	},
#endif /*CONFIG_MFD_MAX8986*/
};


static void athenaray_add_i2c_slaves(void)
{
#if defined (CONFIG_TOUCHSCREEN_TSC2007)
	tsc2007_pdata.get_pendown_state = tsc2007_pen_down_state;
	tsc2007_pdata.clear_penirq = tsc2007_clear_penirq;
	tsc2007_pdata.init_platform_hw = tsc2007_init_platform_hw;
	tsc2007_pdata.exit_platform_hw = tsc2007_exit_platform_hw;
	tsc2007_pdata.get_x_value = tsc2007_get_x_value;
	tsc2007_pdata.get_y_value = tsc2007_get_y_value;
	tsc2007_pdata.x_plate_ohms = 50;
	tsc2007_pdata.validx_left = 320;
	tsc2007_pdata.validy_up = 410;
	tsc2007_pdata.validx_right = 3700;
	tsc2007_pdata.validy_down = 3840;
	tsc2007_pdata.i2c_pdata = (struct i2c_slave_platform_data) {
	.i2c_spd = I2C_SPD_100K,};
#endif

	i2c_register_board_info(I2C_BSC_ID0, athenaray_i2c1_board_info,
				ARRAY_SIZE(athenaray_i2c1_board_info));
	i2c_register_board_info(I2C_BSC_ID2, athenaray_i2c3_board_info,
				ARRAY_SIZE(athenaray_i2c3_board_info));
#if defined (CONFIG_BRCM_HAL_CAM)
        i2c_register_board_info(I2C_BSC_ID1, bcm21553_cam_i2c_board_info,
                                ARRAY_SIZE(bcm21553_cam_i2c_board_info));
#endif
}

#ifdef CONFIG_SPI
static DEFINE_SPINLOCK(bcm_spi_lock);
static int bcm21xx_cs_control(struct driver_data *drv_data, u8 cs)
{
	unsigned long flags;

	if (!drv_data)
		return -EINVAL;

	spin_lock_irqsave(&bcm_spi_lock, flags);

	writeb(SPI_SPIFSSCR_FSSNEW(cs) |
	       SPI_SPIFSSCR_FSSSELNEW, drv_data->ioaddr + SPI_SPIFSSCR);

	spin_unlock_irqrestore(&bcm_spi_lock, flags);

	return 0;
}

static struct bcm21xx_spi_platform_data bcm21xx_spi_info = {
	.slot_id = 0,
	.enable_dma = 1,
	.cs_line = 1,
	.mode = 0,		/* Configure for master mode */
	.syscfg_inf = board_sysconfig,
	.cs_control = bcm21xx_cs_control,
};

/*
 * SPI board info for the slaves
 */
static struct spi_board_info spi_slave_board_info[] __initdata = {
	{
	 .modalias = "spidev",	/* use spidev generic driver */
	 .max_speed_hz = 60000000,	/* use max speed */
	 .bus_num = 0,		/* framework bus number */
	 .chip_select = 0,	/* for each slave */
	 .platform_data = NULL,	/* no spi_driver specific */
	 .controller_data = &bcm21xx_spi_info,
	 .irq = 0,		/* IRQ for this device */
	 .mode = SPI_LOOP,	/* SPI mode 0 */
	 },
	/* TODO: adding more slaves here */
};

#endif

#if defined(CONFIG_FB_BCM)
/*!
 *  * The LCD definition structure.
 *   */

struct platform_device bcm21553_device_fb = {
	.name   = "LCDfb",
	.id     = -1,
	.dev    = {
		.dma_mask               = (u64 *)~(u32)0,
		.coherent_dma_mask      = ~(u32)0,
	},
	.num_resources = 0,
};
#endif

#ifdef CONFIG_BCM215XX_DSS
static struct lcdc_platform_data_t lcdc_pdata = {
	.gpio = 24,
	.te_supported = true,
};
#endif

#ifdef CONFIG_USB_ANDROID
static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id = 0x18D1,
	.product_id = 0x0005,
	.adb_product_id = 0x0002,
	.version = 0x0100,
	.product_name = "BCM21553-Thunderbird",
	.manufacturer_name = "Broadcom",
	.serial_number="0123456789ABCDEF",
	.nluns = 1,
};

static struct platform_device android_usb_device = {
      .name = "android_usb",
      .id         = -1,
      .dev        = {
            .platform_data = &android_usb_pdata,
      },
};
#endif


/* These are active devices on this board */
static struct platform_device *devices[] __initdata = {
#if defined(CONFIG_SERIAL_8250) || defined(CONFIG_SERIAL_8250_MODULE)
	&bcm21553_serial_device0,
	&bcm21553_serial_device1,
	&bcm21553_serial_device2,
#endif
#if defined(CONFIG_DMADEVICES) && defined(CONFIG_OSDAL_SUPPORT)
	&bcm21xx_dma_device,
#endif
#ifdef CONFIG_MMC_BCM
	&bcm21553_sdhc_slot1,
#if !defined(CONFIG_MTD_ONENAND) && !defined(CONFIG_MTD_NAND)
	&bcm21553_sdhc_slot2,
#endif
	&bcm21553_sdhc_slot3,
#endif

#if defined(CONFIG_BCM_WATCHDOG)
	&bcm_watchdog_device,
#endif

#ifdef CONFIG_KEYBOARD_BCM
	&bcm215xx_kp_device,
#endif
#ifdef CONFIG_BCM_AUXADC
	&auxadc_device,
#endif
#if defined(CONFIG_BRCM_HEADSET) || defined(CONFIG_BRCM_HEADSET_MODULE)
	&bcm_headset_device,
#endif
#ifdef CONFIG_BCM_PWM
	&bcm_pwm_device,
#ifdef CONFIG_BACKLIGHT_PWM
	&bcm_backlight_devices,
#endif
#endif
#ifdef CONFIG_MTD_ONENAND_BCM_XBOARD
	&athenaray_device_onenand,
#endif
#ifdef CONFIG_MTD_NAND_BRCM_NVSRAM
	&bcm21553_device_nand,
#endif
#ifdef CONFIG_I2C
	&bcm21553_device_i2c1,
	&bcm21553_device_i2c2,
	&bcm21553_device_i2c3,
#endif
#ifdef CONFIG_SPI
	&bcm21xx_device_spi,
#endif
#ifdef CONFIG_FB_BCM
	&bcm21553_device_fb,
#endif
#ifdef CONFIG_BCM215XX_DSS
	&bcm215xx_lcdc_device,
#endif
#if defined (CONFIG_BCM_OTP)
	&bcm_otp_device,
#endif
#ifdef CONFIG_BCM_I2SDAI
    &i2sdai_device,
#endif
#ifdef CONFIG_BCM_CPU_FREQ
	&bcm21553_cpufreq_drv,
#endif
#ifdef CONFIG_BCM_AVS
	&bcm215xx_avs_device,
#endif
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
	&bcm21553_cpufreq_gov,
#endif
#ifdef CONFIG_USB_ANDROID
        &android_usb_device,
#endif
#if defined (CONFIG_BRCM_V3D)
    &v3d_device,
#endif
};

#ifdef CONFIG_MMC_BCM
int bcmsdhc_ctrl_slot_is_invalid(u8 ctrl_slot)
{
	if (ctrl_slot > 3)
		return -EINVAL;

	return 0;
}
#endif /*CONFIG_MMC_BCM */

static void __init bcm21553_timer_init(void)
{
	struct timer_config bcm21553_timer_config = {
		.ce_module = TIMER_MODULE_GPT,
		.cs_module = TIMER_MODULE_GPT,
		.cp_cs_module = TIMER_MODULE_GPT,
		.ce_base = (void __iomem *)io_p2v(BCM21553_GPTIMER_BASE),
		.cp_cs_base = (void __iomem *)io_p2v(BCM21553_SLPTMR1_BASE),
		.cs_index = 0,
		.ce_index = 1,
		.cp_cs_index = 0,
		.irq = IRQ_GPT1,
	};

	struct gpt_base_config base_config = {
		.avail_bitmap = GPT_AVAIL_BITMAP,
		.base_addr = (void __iomem *)io_p2v(BCM21553_GPTIMER_BASE),
		.irq = IRQ_GPT1,
	};

	gpt_init(&base_config);
	bcm_timer_init(&bcm21553_timer_config);
}

static struct sys_timer bcm21553_timer = {
	.init = bcm21553_timer_init,
};

int board_sysconfig(uint32_t module, uint32_t op)
{
	static DEFINE_SPINLOCK(bcm_syscfg_lock);
	unsigned long flags, auxmic_base;
	int ret = 0;
	spin_lock_irqsave(&bcm_syscfg_lock, flags);
	auxmic_base = (unsigned long) io_p2v(BCM21553_AUXMIC_BASE);
	switch (module) {
	case SYSCFG_LCD:
		if(op == SYSCFG_ENABLE) {
			writel(readl(ADDR_SYSCFG_IOCR6) &
				~(SYSCFG_IOCR6_GPIO25_24_MUX),
				ADDR_SYSCFG_IOCR6);
			writel(readl(ADDR_SYSCFG_IOCR2) |
				(SYSCFG_IOCR2_OSC2_ENABLE),
				ADDR_SYSCFG_IOCR2);
		} else {
			/* Configure IOCR0[29].IOCR0[25] = 10 (GPIO[49:44])*/
			/* Configure IOCR0[29].IOCR0[25] = 00 (LCDCTRL,LCDD[0])*/
			writel((readl(ADDR_SYSCFG_IOCR0) &
				~(SYSCFG_IOCR0_LCD_CTRL_MUX | SYSCFG_IOCR0_MPHI_MUX)),
				ADDR_SYSCFG_IOCR0);
			if (op == SYSCFG_INIT) {
				writel((readl(ADDR_SYSCFG_IOCR0) | SYSCFG_IOCR0_LCD_CTRL_MUX),
				ADDR_SYSCFG_IOCR0);
			}
		}
		break;
	case SYSCFG_PWM0:
#define GPIO16_AS_PWM0 ((1 << 10))
#define GPIO16_AS_GPIO (~(GPIO16_AS_PWM0))
			if (op == SYSCFG_ENABLE) {
				writel(readl(io_p2v(BCM21553_SYSCFG_BASE) + 0x00) |
					GPIO16_AS_PWM0,
					io_p2v(BCM21553_SYSCFG_BASE) + 0x00);
			}
			if (op == SYSCFG_DISABLE) {
				writel(readl(io_p2v(BCM21553_SYSCFG_BASE) + 0x00) &
					GPIO16_AS_GPIO,
					io_p2v(BCM21553_SYSCFG_BASE) + 0x00);
			}
		break;
	case (SYSCFG_PWM0 + 1): /* PWM1 => LCD Backlight */
		if ((op == SYSCFG_INIT) || (op == SYSCFG_DISABLE))
			writel(readl(ADDR_SYSCFG_IOCR0) &
				~SYSCFG_IOCR0_GPIO17_PWM0_MUX, ADDR_SYSCFG_IOCR0);
		else if (op == SYSCFG_ENABLE)
			writel(readl(ADDR_SYSCFG_IOCR0) |
				SYSCFG_IOCR0_GPIO17_PWM0_MUX, ADDR_SYSCFG_IOCR0);

		break;
	case SYSCFG_RESETREASON_SOFT_RESET:
		if (op == SYSCFG_INIT) {
		 	ret =  readl(ADDR_SYSCFG_PUMR);

		} else if (op == SYSCFG_DISABLE) {

			writel(PUMR_VAL_SOFT_RESET, ADDR_SYSCFG_PUMR);
		} else if (op == SYSCFG_ENABLE) {
		}
		break;

	 case SYSCFG_CAMERA:
                 if(op == SYSCFG_INIT){
                        u32 val;
                        /* IOCR 0 */
                        val = readl(ADDR_SYSCFG_IOCR0) & ~(SYSCFG_IOCR0_CAMCK_GPIO_MUX);
                        writel(val, ADDR_SYSCFG_IOCR0);
                        /* IOCR 3 */
                        val = readl(ADDR_SYSCFG_IOCR3);
                         /* Clear bits 6,5 and 4 */
                        val &= ~(SYSCFG_IOCR3_CAMCK_DIS |
				SYSCFG_IOCR3_CAMDCK_PU | SYSCFG_IOCR3_CAMDCK_PD);
                        val |= SYSCFG_IOCR3_CAMDCK_PD;
                        writel(val, ADDR_SYSCFG_IOCR3);
                        /* IOCR 4*/
                        val = readl(ADDR_SYSCFG_IOCR4);
                        /* Bits 14:12 */
                        val |= SYSCFG_IOCR4_CAM_DRV_STGTH(0x7);
                        writel(val, ADDR_SYSCFG_IOCR4);
                  } else if(op == SYSCFG_ENABLE){
                        u32 val;
                         /* IOCR 0 */
                        val = readl(ADDR_SYSCFG_IOCR0) & ~(SYSCFG_IOCR0_CAMCK_GPIO_MUX);
                        writel(val, ADDR_SYSCFG_IOCR0);
                        /* IOCR 3 */
                        val = readl(ADDR_SYSCFG_IOCR3);
                        val &= ~(SYSCFG_IOCR3_CAMCK_DIS | SYSCFG_IOCR3_CAMDCK_PU |
				 SYSCFG_IOCR3_CAMDCK_PD | SYSCFG_IOCR3_CAMHVS_PU |
				 SYSCFG_IOCR3_CAMHVS_PD | SYSCFG_IOCR3_CAMD_PU |
				 SYSCFG_IOCR3_CAMD_PD);
                        val |= SYSCFG_IOCR3_CAMDCK_PD;
#if defined (CONFIG_CAM_CPI)
                        val |=  (SYSCFG_IOCR3_CAMD_PU | SYSCFG_IOCR3_CAMHVS_PD);
#endif

                        writel(val, ADDR_SYSCFG_IOCR3);
                        /* IOCR 5 for trace muxing */
                        val = readl(ADDR_SYSCFG_IOCR5);
                        val &= ~(SYSCFG_IOCR5_CAM_TRACE_EN);
#if defined (CONFIG_CAM_CPI)
			val |= SYSCFG_IOCR5_CAM_TRACE_EN;
#endif

                        writel(val, ADDR_SYSCFG_IOCR5);
                        /* IOCR 4*/
                        val = readl(ADDR_SYSCFG_IOCR4);
                        /* Bits 14:12 Cam drive strength */
                        val |= SYSCFG_IOCR4_CAM_DRV_STGTH(0x7);
                        writel(val, ADDR_SYSCFG_IOCR4);
                        /* IOCR 6 for cam afe and mode */
                        val = readl(ADDR_SYSCFG_IOCR6);
                        val &= ~(SYSCFG_IOCR6_CAM2_CAM1_B);
                        val |= (SYSCFG_IOCR6_CAM2_CAM1_B); /* CAM 1 interface for now */
                        val &= ~(SYSCFG_IOCR6_CAM_MODE(0x3)); /* Clear */
#if defined (CONFIG_CAM_CPI)
                        val |= (SYSCFG_IOCR6_CAM_MODE(0x3));
#else
			val |= (SYSCFG_IOCR6_CAM_MODE(0x2));
#endif
                        writel(val, ADDR_SYSCFG_IOCR6);
                        printk(KERN_INFO"Board sys Done enable 0x%x\n", readl(ADDR_SYSCFG_IOCR6));
                  } else if(op == SYSCFG_DISABLE) {
			u32 val;
                          /* IOCR 3 */
                        val = readl(ADDR_SYSCFG_IOCR3);
                        val &= ~(SYSCFG_IOCR3_CAMCK_DIS);
                        val |= SYSCFG_IOCR3_CAMCK_DIS; /* Disable CAM outputs */
                        writel(val, ADDR_SYSCFG_IOCR3);
                        /* IOCR 4*/
                        val = readl(ADDR_SYSCFG_IOCR4);
                        /* Bits 14:12 Cam drive strength */
                        val &= ~(SYSCFG_IOCR4_CAM_DRV_STGTH(0x7));
                        /* Drive strength minimum */
                        val |= (SYSCFG_IOCR4_CAM_DRV_STGTH(1));
                        writel(val, ADDR_SYSCFG_IOCR4);
                        /* IOCR 6 for cam afe and mode */
                        val = readl(ADDR_SYSCFG_IOCR6);
                        val &= ~(SYSCFG_IOCR6_CAM_MODE(0x3)); /* Clear */
                        writel(val, ADDR_SYSCFG_IOCR6);
                   }
                break;
	case SYSCFG_SDHC1:
		if (op == SYSCFG_INIT) {

			/* Offset for IOCR5 = 0x24 */
			/* GPIO35_MUX set to GPIO35 */
			writel((readl(ADDR_SYSCFG_IOCR5) & ~SYSCFG_IOCR5_GPIO35_UARTA_OUT2N_MUX(0x03)),
                		ADDR_SYSCFG_IOCR5);

			/* Make GPIO35 to out direction, and set value 0 */
			gpio_request(SDIO1_WLAN_MUX_TTL, "sdio1_wlan_mux_ttl");
			gpio_direction_output(SDIO1_WLAN_MUX_TTL, 1);
			gpio_set_value(SDIO1_WLAN_MUX_TTL, 0);
			bcm_gpio_pull_up(SDIO1_WLAN_MUX_TTL, false);
			bcm_gpio_pull_up_down_enable(SDIO1_WLAN_MUX_TTL, false);
			gpio_free(SDIO1_WLAN_MUX_TTL);

			/* Offset for IOCR0 = 0x00 */
			/* SPI_GPIO_MASK = 0x18 */
			writel((readl(ADDR_SYSCFG_IOCR0) & ~SYSCFG_IOCR0_SD1_MUX(0x3)),
			       ADDR_SYSCFG_IOCR0);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2) &
			       ~(SYSCFG_IOCR2_SD1CMD_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN) |
				 SYSCFG_IOCR2_SD1DAT_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);




	/* Make GPIO36 to pull down */
		bcm_gpio_pull_up_down_enable(BCM2091_CLK_REQ_PASS_THOUGH, true);
		bcm_gpio_pull_up(BCM2091_CLK_REQ_PASS_THOUGH, false);


		/* Make GPIO31 to out direction, and set value 0 */
			writel((readl(ADDR_SYSCFG_IOCR5) & ~SYSCFG_IOCR5_GPIO31_MUX(0x3)),
					ADDR_SYSCFG_IOCR5);

			gpio_request(BCM4325_WLAN_RESET, "sdio1_wlan_reset");
			gpio_direction_output(BCM4325_WLAN_RESET, 0);
			gpio_set_value(BCM4325_WLAN_RESET, 0);
			gpio_free(BCM4325_WLAN_RESET);

	//Set SDIO1 Driving Strength
			writel((readl(ADDR_SYSCFG_IOCR4) | SYSCFG_IOCR4_SD1_DAT_DRV_STGTH(0x7)),
					ADDR_SYSCFG_IOCR4);

			writel((readl(ADDR_SYSCFG_IOCR6) | SYSCFG_IOCR6_SD1_CLK_DRV_STGTH(0x7)),
					ADDR_SYSCFG_IOCR6);

			/* Pull down MSD_CD_B GPIO to save power */
#define GPIO_MSD_CD_B	(32)
			/* GPIO32_MUX set to GPIO32 */
			writel((readl(ADDR_SYSCFG_IOCR5) & ~SYSCFG_IOCR5_GPIO32_UARTA_DSR_MUX(0x3)),
					ADDR_SYSCFG_IOCR5);

			gpio_request(GPIO_MSD_CD_B, "gpio_msd_cd_b");
			bcm_gpio_pull_up_down_enable(GPIO_MSD_CD_B, true);
			bcm_gpio_pull_up(GPIO_MSD_CD_B, false);
			gpio_free(GPIO_MSD_CD_B);

				// This is to Enable the SYS_CLK_REQ From 2091 for P102 WCG WLAN CARD
			writel((readl(ADDR_CLKPWR_CLK_SYSCLK_REQ_MASK) | 0x00000001),
                		ADDR_CLKPWR_CLK_SYSCLK_REQ_MASK);

		} else if (op == SYSCFG_ENABLE) {
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       & ~(SYSCFG_IOCR2_SD1CMD_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       | SYSCFG_IOCR2_SD1CMD_PULL_CTRL(SD_PULL_UP),
			       ADDR_SYSCFG_IOCR2);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       & ~(SYSCFG_IOCR2_SD1DAT_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       | SYSCFG_IOCR2_SD1DAT_PULL_CTRL(SD_PULL_UP),
			       ADDR_SYSCFG_IOCR2);

				//This is to Enable the SYS_CLK_REQ From 2091 for P102 WCG WLAN CARD

			writel((readl(ADDR_CLKPWR_CLK_SYSCLK_REQ_MASK) | 0x00000001),
                		ADDR_CLKPWR_CLK_SYSCLK_REQ_MASK);

	//Set SDIO1 Driving Strength
			writel((readl(ADDR_SYSCFG_IOCR4) | SYSCFG_IOCR4_SD1_DAT_DRV_STGTH(0x7)),
					ADDR_SYSCFG_IOCR4);

			writel((readl(ADDR_SYSCFG_IOCR6) | SYSCFG_IOCR6_SD1_CLK_DRV_STGTH(0x7)),
					ADDR_SYSCFG_IOCR6);

		} else if (op == SYSCFG_DISABLE) {
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2) &
				~(SYSCFG_IOCR2_SD1CMD_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN) |
			         SYSCFG_IOCR2_SD1DAT_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);

		}
		break;
	case SYSCFG_SDHC2:
		if (op == SYSCFG_INIT) {
			/* Offset for IOCR0 = 0x00 */
			writel((readl(ADDR_SYSCFG_IOCR0) |
				SYSCFG_IOCR0_FLASH_SD2_MUX),
			       ADDR_SYSCFG_IOCR0);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2) &
			       ~(SYSCFG_IOCR2_SD2CMD_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN) |
				 SYSCFG_IOCR2_SD2DATL_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
		} else if (op == SYSCFG_ENABLE) {
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       & ~(SYSCFG_IOCR2_SD2CMD_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       | SYSCFG_IOCR2_SD2CMD_PULL_CTRL(SD_PULL_UP),
			       ADDR_SYSCFG_IOCR2);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       & ~(SYSCFG_IOCR2_SD2DATL_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       | SYSCFG_IOCR2_SD2DATL_PULL_CTRL(SD_PULL_UP),
			       ADDR_SYSCFG_IOCR2);
		} else if (op == SYSCFG_DISABLE) {
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2) &
				~(SYSCFG_IOCR2_SD2CMD_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN) |
			         SYSCFG_IOCR2_SD2DATL_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
		}
		break;
	case SYSCFG_SDHC3:
		if (op == SYSCFG_INIT) {
			/* Offset for IOCR0 = 0x00 */
			/* MSPRO_SD3 = 0x3 */
			writel((readl(ADDR_SYSCFG_IOCR0) &
				~SYSCFG_IOCR0_SD3_MUX(3)),
			       ADDR_SYSCFG_IOCR0);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2) &
			       ~(SYSCFG_IOCR2_SD3CMD_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN) |
				 SYSCFG_IOCR2_SD3DAT_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
		} else if (op == SYSCFG_ENABLE) {
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       & ~(SYSCFG_IOCR2_SD3CMD_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       | SYSCFG_IOCR2_SD3CMD_PULL_CTRL(SD_PULL_UP),
			       ADDR_SYSCFG_IOCR2);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       & ~(SYSCFG_IOCR2_SD3DAT_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2)
			       | SYSCFG_IOCR2_SD3DAT_PULL_CTRL(SD_PULL_UP),
			       ADDR_SYSCFG_IOCR2);
		} else if (op == SYSCFG_DISABLE) {
			/* Offset for IOCR2 = 0x0c */
			writel(readl(ADDR_SYSCFG_IOCR2) &
				~(SYSCFG_IOCR2_SD3CMD_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN) |
			         SYSCFG_IOCR2_SD3DAT_PULL_CTRL(SD_PULL_UP | SD_PULL_DOWN)),
			       ADDR_SYSCFG_IOCR2);
		}
		break;
#ifdef CONFIG_USB_DWC_OTG
	case SYSCFG_USB:
		if (op == SYSCFG_DISABLE)
			{
				REG_SYS_ANACR9 = (ANACR9_USB_IDLE_ENABLE |
				ANACR9_USB_UTMI_STOP_DIGITAL_CLOCKS |
				ANACR9_USB_UTMI_SOFT_RESET_DISABLE);
				REG_SYS_ANACR9 &= ~(ANACR9_PLL_SUSPEND_ENABLE |
					ANACR9_USB_PLL_POWER_ON);

				REG_CLKPWR_CLK_USB_48_ENABLE &= ~1;
				REG_CLKPWR_CLK_ANALOG_PHASE_ENABLE |= 2; //bit 1
				REG_CLKPWR_CLK_USB_48_ENABLE &= ~1;
				REG_CLKPWR_USBPLL_ENABLE &= ~1;
				REG_SYS_ANACR9 |= ANACR9_USB_AFE_NON_DRVING;

			}
			else if (op == SYSCFG_ENABLE)
			{
				REG_CLKPWR_CLK_USB_48_ENABLE |= 1;
				REG_CLKPWR_CLK_ANALOG_PHASE_ENABLE &= ~2; //bit 1
				REG_CLKPWR_CLK_USB_48_ENABLE |= 1;
				REG_CLKPWR_USBPLL_ENABLE |= 1;

				REG_SYS_ANACR9 = (
				ANACR9_USB_UTMI_STOP_DIGITAL_CLOCKS |
				ANACR9_USB_UTMI_SOFT_RESET_DISABLE |
				ANACR9_USB_PLL_POWER_ON |
				ANACR9_USB_PLL_CAL_ENABLE |
				ANACR9_USB_SELECT_OTG_MODE |
				ANACR9_USB_SELECT_DEVICE_MODE |
				ANACR9_PLL_SUSPEND_ENABLE |
				ANACR9_USB_PPC_PWR_OFF_ANALOG_DRIVERS);

				/* Reset the PHY since this could
				 * be a mode change too */
				REG_SYS_ANACR9 = (
				ANACR9_USB_UTMI_STOP_DIGITAL_CLOCKS |
				ANACR9_USB_PLL_POWER_ON |
				ANACR9_USB_PLL_CAL_ENABLE |
				ANACR9_USB_SELECT_OTG_MODE |
				ANACR9_USB_SELECT_DEVICE_MODE |
				ANACR9_PLL_SUSPEND_ENABLE |
				ANACR9_USB_PPC_PWR_OFF_ANALOG_DRIVERS);

				/* De-activate PHY reset */
				REG_SYS_ANACR9 = (
				ANACR9_USB_UTMI_STOP_DIGITAL_CLOCKS |
				ANACR9_USB_UTMI_SOFT_RESET_DISABLE |
				ANACR9_USB_PLL_POWER_ON |
				ANACR9_USB_PLL_CAL_ENABLE |
				ANACR9_USB_SELECT_OTG_MODE |
				ANACR9_USB_SELECT_DEVICE_MODE |
				ANACR9_USB_PPC_PWR_OFF_ANALOG_DRIVERS |
				ANACR9_PLL_SUSPEND_ENABLE);
			}
		break;
#endif
	case SYSCFG_HEADSET:
		/* power up analog block for headset */
		if (op == SYSCFG_INIT)
			writel(SYSCFG_MON5_MIC_PWR_DOWN, ADDR_SYSCFG_ANACR2);
		break;
	case SYSCFG_AUXMIC:
		if (op == SYSCFG_INIT) {
			writel(AUXMIC_PRB_CYC_MS(128), auxmic_base + AUXMIC_PRB_CYC);
			writel(AUXMIC_MSR_DLY_MS(2), auxmic_base + AUXMIC_MSR_DLY);
			writel(AUXMIC_MSR_INTVL_MS(64), auxmic_base + AUXMIC_MSR_INTVL);
			writel(readl(auxmic_base + AUXMIC_CMC) & ~AUXMIC_CMC_PROB_CYC_INF,
				auxmic_base + AUXMIC_CMC);
			writel(readl(auxmic_base + AUXMIC_MIC) | AUXMIC_MIC_SPLT_MSR_INTR,
					auxmic_base + AUXMIC_MIC);
			writel(readl(auxmic_base + AUXMIC_AUXEN) & ~AUXMIC_ENABLE_BIAS,
					auxmic_base + AUXMIC_AUXEN);
			writel(AUXMIC_MICINTH_ADJ_VAL(13),
					auxmic_base + AUXMIC_MICINTH_ADJ);
			writel(AUXMIC_MICINENTH_ADJ_VAL(9),
					auxmic_base + AUXMIC_MICINENTH_ADJ);
			writel(AUXMIC_MICONTH_ADJ_VAL(0x00),
					auxmic_base + AUXMIC_MICONTH_ADJ);
			writel(AUXMIC_MICONENTH_ADJ_VAL(0x00),
					auxmic_base + AUXMIC_MICONENTH_ADJ);
			writel(readl(auxmic_base + AUXMIC_F_PWRDWN) & ~AUXMIC_F_PWRDWN_ENABLE,
					auxmic_base + AUXMIC_F_PWRDWN);
			/* Enable the debounce counter for headset insertion and headset button */
			writel((readl(ADDR_IRQ_ICCR) |
					(INTC_ICCR_MICINDBEN | INTC_ICCR_MICINDBC(3)) |
					(INTC_ICCR_MICONDBEN | INTC_ICCR_MICONDBC(2))),
					ADDR_IRQ_ICCR);
		}
		if(op == SYSCFG_DISABLE){
			writel(readl(auxmic_base + AUXMIC_AUXEN) & ~AUXMIC_ENABLE_BIAS,
					auxmic_base + AUXMIC_AUXEN);
			writel(readl(auxmic_base + AUXMIC_CMC) | AUXMIC_CMC_PROB_CYC_INF,
					auxmic_base + AUXMIC_CMC);
		}
		break;
	case SYSCFG_SPI1:
		/* During init 21:11 is set to 01 to select UARTC
		 * To enable SPI1 set [21:11] =  00
		 * To disable SPI1 set [21:11] = 01 to revert UARTC setting
		 * We toggle Bit 11 to enable/disable SPI, and bit 21 need
		 * not be touched
		 */
		if (op == SYSCFG_ENABLE)	{
			 writel((readl(ADDR_SYSCFG_IOCR0) &
				~(SYSCFG_IOCR0_SPIL_GPIO_MUX
				| SYSCFG_IOCR0_SPIH_GPIO_MUX)),
		                ADDR_SYSCFG_IOCR0);
		}
		/* Set Bits [21:11] = 01 to disable SPI1 and enable UARTC */
		if (op == SYSCFG_DISABLE)	{
			 writel((readl(ADDR_SYSCFG_IOCR0) |
				(SYSCFG_IOCR0_SPIL_GPIO_MUX
				 | SYSCFG_IOCR0_SPIH_GPIO_MUX)),
                		ADDR_SYSCFG_IOCR0);
		}
		break;
	case SYSCFG_SYSTEMINIT:
		if (op == SYSCFG_INIT) {
			/* BIT8 | BIT12 | BIT13 | BIT14 | BIT15:16 | BIT23 | BIT24 | BIT27 */
			writel(readl(ADDR_SYSCFG_IOCR0) |
				SYSCFG_IOCR0_SPI_UARTC_MUX |
				SYSCFG_IOCR0_GPIO52_GPEN7_MUX |
				SYSCFG_IOCR0_GPIO53_GPEN8_MUX |
				SYSCFG_IOCR0_GPEN9_SPI_GPIO54_L_MUX |
				SYSCFG_IOCR0_GPIO55_GPEN10_MUX(0x3) |
				SYSCFG_IOCR0_PCM_SPI2_GPIO4043_MUX(0x02) |
				SYSCFG_IOCR0_DIGMIC_GPIO6362_MUX |
				SYSCFG_IOCR0_GPEN9_SPI_GPIO54_H_MUX
				, ADDR_SYSCFG_IOCR0);
			/* SD1/SD2/SD3 DAT[0:3]/CMD lines pulled down
			 * UARTA enable internal pullup */
			/* BIT10 | BIT12 | BIT20 | BIT23 | BIT25 | BIT27 | BIT29 | BIT31 */
			writel(readl(ADDR_SYSCFG_IOCR2) |
				SYSCFG_IOCR2_LCDDATA_PULL_CTRL(0x1) |
				SYSCFG_IOCR2_UARTA_PULL_CTRL(0x1) |
				SYSCFG_IOCR2_SD3CMD_PULL_CTRL(SD_PULL_DOWN) |
				SYSCFG_IOCR2_SD3DAT_PULL_CTRL(SD_PULL_DOWN) |
				SYSCFG_IOCR2_SD1CMD_PULL_CTRL(SD_PULL_DOWN) |
				SYSCFG_IOCR2_SD1DAT_PULL_CTRL(SD_PULL_DOWN) |
				SYSCFG_IOCR2_SD2CMD_PULL_CTRL(SD_PULL_DOWN) |
				SYSCFG_IOCR2_SD2DATL_PULL_CTRL(SD_PULL_DOWN)
				, ADDR_SYSCFG_IOCR2);
			/* BIT21 | BIT27 | BIT31 */
			writel(readl(ADDR_SYSCFG_IOCR3) |
				SYSCFG_IOCR3_TWIF_ENB |
				SYSCFG_IOCR3_SIMDAT_PU | SYSCFG_IOCR3_I2C3_EN
				, ADDR_SYSCFG_IOCR3);
			/* UARTB enable internal pullup */
			/* BIT9 | BIT11 | BIT14 */
			writel(readl(ADDR_SYSCFG_IOCR5) |
			    SYSCFG_IOCR5_GPIO15L_DRV_STGTH(0x1), ADDR_SYSCFG_IOCR5);
			/* SD2 DAT[7:4] pulled down */
			/* BIT21 */
			writel(readl(ADDR_SYSCFG_IOCR6) |
			    SYSCFG_IOCR6_SD2DATH_PULL_CTRL(SD_PULL_DOWN)
			    , ADDR_SYSCFG_IOCR6);
#ifdef CONFIG_ARCH_BCM21553_B1
			/*Disable Hantro performance ECO*/
			writel(readl(ADDR_SYSCFG_IOCR10) |
				SYSCFG_IOCR10_DEC_SYNC_BRG_INCR_TO_INCR4 |
				SYSCFG_IOCR10_ENC_SYNC_BRG_INCR_TO_INCR4,
				ADDR_SYSCFG_IOCR10);
#endif

			/* Enable BSC3 on GPIO7 and GPIO15 */
			/* Configure IOCR3[31].IOCR10[29] = 1 0 */
			writel(readl(ADDR_SYSCFG_IOCR3) |
				SYSCFG_IOCR3_I2C3_EN,
				ADDR_SYSCFG_IOCR3);
			writel(readl(ADDR_SYSCFG_IOCR10) &
				~SYSCFG_IOCR10_BSC3_GPIOH_ENABLE,
				ADDR_SYSCFG_IOCR10);

			bcm21553_cam_lmatrix_prio();
#define EXT_CTRL_REQ13 26
#define EXT_CTRL_REQ2	4
			writel(EXT_BY_32AHB_L_CYC << EXT_CTRL_REQ13,
					ADDR_SYSCFG_SYSCONF_AHB_CLK_EXTEND0);
			writel(EXT_BY_32AHB_L_CYC << EXT_CTRL_REQ2,
					ADDR_SYSCFG_SYSCONF_AHB_CLK_EXTEND1);
		}
		break;
#ifdef CONFIG_BRCM_V3D
	case SYSCFG_V3D:
		if (op == SYSCFG_INIT) {
			writel(SYSCFG_V3DRSTR_RST, ADDR_SYSCFG_V3DRSTR);
			writel(~SYSCFG_V3DRSTR_RST, ADDR_SYSCFG_V3DRSTR);
		}
		break;
#endif
	case SYSCFG_CSL_DMA:
		if(op == SYSCFG_ENABLE)
			writel(SYSCFG_AHB_PER_CLK_EN, ADDR_SYSCFG_DMAC_AHB_CLK_MODE);
		else if(op == SYSCFG_DISABLE)
			writel(~SYSCFG_AHB_PER_CLK_EN, ADDR_SYSCFG_DMAC_AHB_CLK_MODE);
		break;
	case SYSCFG_CP_START:
		if (op == SYSCFG_INIT) {
			writel(readl(ADDR_SYSCFG_IOCR3) |
				SYSCFG_IOCR3_TWIF_ENB,
				ADDR_SYSCFG_IOCR3);
		}
		break;
	default:
		pr_info("%s: inval arguments\n", __func__);
		spin_unlock_irqrestore(&bcm_syscfg_lock, flags);
		return -EINVAL;
	}
	spin_unlock_irqrestore(&bcm_syscfg_lock, flags);
	return ret;
}

EXPORT_SYMBOL(board_sysconfig);




static void __init bcm21553_anthenaray_init(void)
{
	/* Configure the SYSCFG Registers */
	board_sysconfig(SYSCFG_SYSTEMINIT, SYSCFG_INIT);
}

static void athenaray_add_platform_data(void)
{
#if defined(CONFIG_SERIAL_8250) || defined(CONFIG_SERIAL_8250_MODULE)
	bcm21553_serial_device0.dev.platform_data = &serial_platform_data0;
	bcm21553_serial_device1.dev.platform_data = &serial_platform_data1;
	bcm21553_serial_device2.dev.platform_data = &serial_platform_data2;
#endif
#ifdef CONFIG_MMC
	bcm21553_sdhc_slot1.dev.platform_data = &bcm21553_sdhc_data1;
#if !defined(CONFIG_MTD_ONENAND) && !defined(CONFIG_MTD_NAND)
	bcm21553_sdhc_slot2.dev.platform_data = &bcm21553_sdhc_data2;
#endif
	bcm21553_sdhc_slot3.dev.platform_data = &bcm21553_sdhc_data3;
#endif
#ifdef CONFIG_BCM_PWM
	bcm_pwm_device.dev.platform_data = &pwm_dev;
#ifdef CONFIG_BACKLIGHT_PWM
	bcm_backlight_devices.dev.platform_data = &bcm_backlight_data;
#endif
#endif
#ifdef CONFIG_KEYBOARD_BCM
	bcm215xx_kp_device.dev.platform_data = &bcm215xx_keypad_data;
#endif
#ifdef CONFIG_BCM_AUXADC
	auxadc_device.dev.platform_data = &adcplat;
#endif
#ifdef CONFIG_SPI
	bcm21xx_device_spi.dev.platform_data = &bcm21xx_spi_info;
#endif
#if defined(CONFIG_BRCM_HEADSET)  || defined(CONFIG_BRCM_HEADSET_MODULE)
	bcm_headset_device.dev.platform_data = &headset_pd;
#endif
#ifdef CONFIG_BCM215XX_DSS
	bcm215xx_lcdc_device.dev.platform_data = &lcdc_pdata;
#endif
}

static int __init arch_late_init(void)
{
	/* Placeholder for any platform specific late init code */
	static void *alloc_mem;
	static dma_addr_t dma_address;
#if defined (CONFIG_ANDROID_PMEM)
	alloc_mem = cam_mempool_base;
	dma_address = (dma_addr_t) virt_to_phys(alloc_mem);
	if (alloc_mem != NULL) {
		android_pmem_pdata.start = dma_address;
		android_pmem_pdata.size = PMEM_ADSP_SIZE;
		platform_device_register(&android_pmem_device);
		printk(" ****** %s:Success PMEM alloc 0x%x ****** \n", __func__, dma_address);
	}
	else {
		printk("******* %s:Fail to alloc memory ****** \n", __func__);
	}
#endif
	return 0;
}
late_initcall(arch_late_init);

static void __init bcm21553_init_machine(void)
{
	bcm21553_platform_init();
	bcm21553_anthenaray_init();
	athenaray_add_i2c_slaves();
	athenaray_add_platform_data();
	platform_add_devices(devices, ARRAY_SIZE(devices));
#ifdef CONFIG_SPI
	/*Function to register SPI board info : required when spi device is
	   present */
	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));
#endif
#if 0 
        {
                static dma_addr_t dma_address;
                static void *alloc_mem;

                alloc_mem =
                    dma_alloc_coherent(NULL, PMEM_ADSP_SIZE, &dma_address,
                                       GFP_ATOMIC | GFP_DMA);
                if (alloc_mem != NULL) {
                        android_pmem_pdata.start = dma_address;
                        android_pmem_pdata.size = PMEM_ADSP_SIZE;
                        platform_device_register(&android_pmem_device);
                        pr_info("%s:Success PMEM alloc 0x%x\n", __func__,
                                dma_address);
                } else {
                        pr_info("%s:Fail to alloc memory\n", __func__);
                }
        }
#endif
}

/* TODO: Replace BCM1160 with BCM21553/AthenaRay once registered */
MACHINE_START(BCM1160, "BCM21553 ThunderbirdEDN31 platform")
	/* Maintainer: Broadcom Corporation */
	.phys_io = BCM21553_UART_A_BASE,
	.io_pg_offst = (IO_ADDRESS(BCM21553_UART_A_BASE) >> 18) & 0xfffc,
	.boot_params = (PHYS_OFFSET + 0x100),
	.map_io = bcm21553_map_io,
	.init_irq = bcm21553_init_irq,
	.timer = &bcm21553_timer,
	.init_machine = bcm21553_init_machine,
MACHINE_END
