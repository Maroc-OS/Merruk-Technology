/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/include/mach/reg_clkpwr.h
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

#ifndef __REG_BCM21553_CLK_PWR_H__
#define __REG_BCM21553_CLK_PWR_H__

#include <mach/io.h>
#include <mach/hardware.h>

#define HW_CLKPWR_BASE                       IO_ADDRESS(BCM21553_CLKPWR_BASE)

/* Clock control module register mapping */
#define ADDR_CLKPWR_CLK_ARMAHB_MODE                     (HW_CLKPWR_BASE+0x00)
#define ADDR_CLKPWR_CLK_CAMCK_ENABLE                    (HW_CLKPWR_BASE+0x04)
#define ADDR_CLKPWR_CLK_CAMCK_MODE                      (HW_CLKPWR_BASE+0x08)
#define ADDR_CLKPWR_CLK_DSP_MODE                        (HW_CLKPWR_BASE+0x0C)
#define ADDR_CLKPWR_CLK_I2S_INT_ENABLE                  (HW_CLKPWR_BASE+0x10)
#define ADDR_CLKPWR_CLK_I2S_INT_MODE                    (HW_CLKPWR_BASE+0x14)
#define ADDR_CLKPWR_CLK_I2S_EXT_ENABLE                  (HW_CLKPWR_BASE+0x18)
#define ADDR_CLKPWR_CLK_DAMCK_ENABLE                    (HW_CLKPWR_BASE+0x1C)
#define ADDR_CLKPWR_CLK_MONITOR_ENABLE                  (HW_CLKPWR_BASE+0x20)
#define ADDR_CLKPWR_CLK_MONITOR_SEL                     (HW_CLKPWR_BASE+0x24)
#define ADDR_CLKPWR_CLK_PDPCK_ENABLE                    (HW_CLKPWR_BASE+0x28)
#define ADDR_CLKPWR_CLK_PDPCK_DIV                       (HW_CLKPWR_BASE+0x2C)
#define ADDR_CLKPWR_CLK_SDIO1_ENABLE                    (HW_CLKPWR_BASE+0x30)
#define ADDR_CLKPWR_CLK_SDIO1_DIV                       (HW_CLKPWR_BASE+0x34)
#define ADDR_CLKPWR_CLK_SDIO2_ENABLE                    (HW_CLKPWR_BASE+0x38)
#define ADDR_CLKPWR_CLK_SDIO2_DIV                       (HW_CLKPWR_BASE+0x3C)
#define ADDR_CLKPWR_CLK_SDRAM_MODE                      (HW_CLKPWR_BASE+0x40)
#define ADDR_CLKPWR_CLK_SRAM_MODE                       (HW_CLKPWR_BASE+0x44)
#define ADDR_CLKPWR_CLK_SPI_ENABLE                      (HW_CLKPWR_BASE+0x48)
#define ADDR_CLKPWR_CLK_SPI_DIV                         (HW_CLKPWR_BASE+0x4C)
#define ADDR_CLKPWR_CLK_UARTA_ENABLE                    (HW_CLKPWR_BASE+0x50)
#define ADDR_CLKPWR_CLK_UARTA_N                         (HW_CLKPWR_BASE+0x54)
#define ADDR_CLKPWR_CLK_UARTA_M                         (HW_CLKPWR_BASE+0x58)
#define ADDR_CLKPWR_CLK_UARTB_ENABLE                    (HW_CLKPWR_BASE+0x5C)
#define ADDR_CLKPWR_CLK_UARTB_N                         (HW_CLKPWR_BASE+0x60)
#define ADDR_CLKPWR_CLK_UARTB_M                         (HW_CLKPWR_BASE+0x64)
#define ADDR_CLKPWR_CLK_UARTC_ENABLE                    (HW_CLKPWR_BASE+0x68)
#define ADDR_CLKPWR_CLK_UARTC_N                         (HW_CLKPWR_BASE+0x6C)
#define ADDR_CLKPWR_CLK_UARTC_M                         (HW_CLKPWR_BASE+0x70)
#define ADDR_CLKPWR_CLK_DSP_SRST_ENABLE                 (HW_CLKPWR_BASE+0x74)
#define ADDR_CLKPWR_CLK_AFCPON_ENABLE                   (HW_CLKPWR_BASE+0x78)
#define ADDR_CLKPWR_CLK_IRPC_ENABLE                     (HW_CLKPWR_BASE+0x7C)
#define ADDR_CLKPWR_CLK_MADDR_STABLE_TIME               (HW_CLKPWR_BASE+0x80)
#define ADDR_CLKPWR_CLK_PC_0_ENABLE                     (HW_CLKPWR_BASE+0x84)
#define ADDR_CLKPWR_CLK_PCHOSTPD_ENABLE                 (HW_CLKPWR_BASE+0x88)
#define ADDR_CLKPWR_CLK_PLL48_STABLE_TIME               (HW_CLKPWR_BASE+0x8C)
#define ADDR_CLKPWR_CLK_PLL48CNTEN                      (HW_CLKPWR_BASE+0x90)
#define ADDR_CLKPWR_CLK_PLL_STABLE_TIME                 (HW_CLKPWR_BASE+0x94)
#define ADDR_CLKPWR_CLK_PLL_SETTLING_COUNTER_ENABLE     (HW_CLKPWR_BASE+0x98)
#define ADDR_CLKPWR_CLK_POWER_MODES                     (HW_CLKPWR_BASE+0x9C)
#define ADDR_CLKPWR_CLK_PWDPLL_ENABLE                   (HW_CLKPWR_BASE+0xA0)
#define ADDR_CLKPWR_CLK_USBPLL_ENABLE_R                 (HW_CLKPWR_BASE+0xA4)
#define ADDR_CLKPWR_CLK_USBPLL_OEN_R                    (HW_CLKPWR_BASE+0xA8)
#define ADDR_CLKPWR_CLK_GPCK_ENABLE                     (HW_CLKPWR_BASE+0xAC)
#define ADDR_CLKPWR_CLK_GPCK_DIV                        (HW_CLKPWR_BASE+0xB0)
#define ADDR_CLKPWR_CLK_MSPRO_ENABLE                    (HW_CLKPWR_BASE+0xB4)
#define ADDR_CLKPWR_CLK_MSPRO_DIV                       (HW_CLKPWR_BASE+0xB8)
#define ADDR_CLKPWR_CLK_TRACE_CLK_MODE                  (HW_CLKPWR_BASE+0xBC)
#define ADDR_CLKPWR_CLK_EVENT_CLK_SEL_0                 (HW_CLKPWR_BASE+0xD0)
#define ADDR_CLKPWR_CLK_EVENT_CLK_SEL_1                 (HW_CLKPWR_BASE+0xD4)
#define ADDR_CLKPWR_CLK_EVENT_CLK_SEL_2                 (HW_CLKPWR_BASE+0xD8)
#define ADDR_CLKPWR_CLK_EVENT_CLK_SEL_3                 (HW_CLKPWR_BASE+0xDC)
#define ADDR_CLKPWR_CLK_CURRENT_EVENT_CLK_SEL_0         (HW_CLKPWR_BASE+0xE0)
#define ADDR_CLKPWR_CLK_CURRENT_EVENT_CLK_SEL_1         (HW_CLKPWR_BASE+0xE4)
#define ADDR_CLKPWR_CLK_CURRENT_EVENT_CLK_SEL_2         (HW_CLKPWR_BASE+0xE8)
#define ADDR_CLKPWR_CLK_CURRENT_EVENT_CLK_SEL_3         (HW_CLKPWR_BASE+0xEC)
#define ADDR_CLKPWR_CLK_APPSPLL_ENABLE                  (HW_CLKPWR_BASE+0xF0)
#define ADDR_CLKPWR_CLK_SEL_MODE                        (HW_CLKPWR_BASE+0xF4)
#define ADDR_CLKPWR_CLK_APPSPLL_DIVIDERS                (HW_CLKPWR_BASE+0xF8)
#define ADDR_CLKPWR_CLK_APPSPLL_FRAC_DIVIDERS           (HW_CLKPWR_BASE+0xFC)
#define ADDR_CLKPWR_CLK_APPSPLL_MODE                    (HW_CLKPWR_BASE+0x100)
#define ADDR_CLKPWR_CLK_APPSPLL_PLL_CTRL_0              (HW_CLKPWR_BASE+0x104)
#define ADDR_CLKPWR_CLK_APPSPLL_PLL_CTRL_1              (HW_CLKPWR_BASE+0x108)
#define ADDR_CLKPWR_CLK_APPSPLL_PLL_CTRL_WE             (HW_CLKPWR_BASE+0x110)
#define ADDR_CLKPWR_CLK_MAINPLL_PLL_CTRL_0              (HW_CLKPWR_BASE+0x114)
#define ADDR_CLKPWR_CLK_MAINPLL_PLL_CTRL_1              (HW_CLKPWR_BASE+0x118)
#define ADDR_CLKPWR_CLK_MAINPLL_PLL_WE                  (HW_CLKPWR_BASE+0x120)
#define ADDR_CLKPWR_CLK_PLL_STATUS                      (HW_CLKPWR_BASE+0x128)
#define ADDR_CLKPWR_CLK_MIPIDSI_CMI_CLK_EN              (HW_CLKPWR_BASE+0x12C)
#define ADDR_CLKPWR_CLK_MIPI_DSI_CTRL                   (HW_CLKPWR_BASE+0x134)
#define ADDR_CLKPWR_CLK_MIPI_DSI_AFE_DIV                (HW_CLKPWR_BASE+0x13C)
#define ADDR_CLKPWR_CLK_ANALOG_PHASES_ENABLE            (HW_CLKPWR_BASE+0x140)
#define ADDR_CLKPWR_CLK_VIDEO_CODEC_PWROFF              (HW_CLKPWR_BASE+0x154)
#define ADDR_CLKPWR_CLK_VDD_VIDEO_CODEC_RAMP_COUNT      (HW_CLKPWR_BASE+0x15C)
#define ADDR_CLKPWR_CLK_POWERSWITCH_CTRL                (HW_CLKPWR_BASE+0x160)
#define ADDR_CLKPWR_CLK_SDRAM_SELFREFRESH_DELAY         (HW_CLKPWR_BASE+0x168)
#define ADDR_CLKPWR_CLK_APPSARM_WARMBOOT_MODE           (HW_CLKPWR_BASE+0x17C)
#define ADDR_CLKPWR_CLK_APPS_DORMANT                    (HW_CLKPWR_BASE+0x180)
#define ADDR_CLKPWR_CLK_APPS_RAMP_COUNT                 (HW_CLKPWR_BASE+0x188)
#define ADDR_CLKPWR_CLK_HSBSC_MODE                      (HW_CLKPWR_BASE+0x1A0)
#define ADDR_CLKPWR_CLK_HSBSC2_MODE                     (HW_CLKPWR_BASE+0x1A4)
#define ADDR_CLKPWR_CLK_I2S_FRAC_MODE                   (HW_CLKPWR_BASE+0x1A8)
#define ADDR_CLKPWR_CLK_CAMRX_MODE                      (HW_CLKPWR_BASE+0x1AC)
#define ADDR_CLKPWR_CLK_SPI2_ENABLE                     (HW_CLKPWR_BASE+0x1B0)
#define ADDR_CLKPWR_CLK_SPI2_DIV                        (HW_CLKPWR_BASE+0x1B4)
#define ADDR_CLKPWR_CLK_HSBSC_ENABLE                    (HW_CLKPWR_BASE+0x1B8)
#define ADDR_CLKPWR_CLK_HSBSC2_ENABLE                   (HW_CLKPWR_BASE+0x1BC)
#define ADDR_CLKPWR_CLK_CAMINTF_ENABLE                  (HW_CLKPWR_BASE+0x1C0)
#define ADDR_CLKPWR_CLK_PWM_ENABLE                      (HW_CLKPWR_BASE+0x1C4)
#define ADDR_CLKPWR_CLK_PWM_SOFTRESET                   (HW_CLKPWR_BASE+0x1C8)
#define ADDR_CLKPWR_CLK_SDIO3_ENABLE                    (HW_CLKPWR_BASE+0x1CC)
#define ADDR_CLKPWR_CLK_SDIO3_DIV                       (HW_CLKPWR_BASE+0x1D0)
#define ADDR_CLKPWR_CLK_CLK_USB_48_ENABLE               (HW_CLKPWR_BASE+0x1D4)
#define ADDR_CLKPWR_CLK_PMSM_FEATURE_CONTROL            (HW_CLKPWR_BASE+0x1F0)
#define ADDR_CLKPWR_CLK_AP_POWER_MODES                  (HW_CLKPWR_BASE+0x1F4)
#define ADDR_CLKPWR_CLK_PM_SLEEP_REQ_MON                (HW_CLKPWR_BASE+0x1F8)
#define ADDR_CLKPWR_CLK_PM_SLEEP_REQ_MASK               (HW_CLKPWR_BASE+0x1FC)
#define ADDR_CLKPWR_CLK_PM_SLEEP_REQ_FORCE              (HW_CLKPWR_BASE+0x200)
#define ADDR_CLKPWR_CLK_DSIPLL_ENABLE                   (HW_CLKPWR_BASE+0x220)
#define ADDR_CLKPWR_CLK_DSIPLL_DIVIDERS                 (HW_CLKPWR_BASE+0x224)
#define ADDR_CLKPWR_CLK_DSIPLL_FRAC_DIVIDERS            (HW_CLKPWR_BASE+0x228)
#define ADDR_CLKPWR_CLK_DSIPLL_MODE                     (HW_CLKPWR_BASE+0x22C)
#define ADDR_CLKPWR_CLK_DSIPLL_PLL_CTRL_0               (HW_CLKPWR_BASE+0x230)
#define ADDR_CLKPWR_CLK_DSIPLL_PLL_CTRL_1               (HW_CLKPWR_BASE+0x234)
#define ADDR_CLKPWR_CLK_DSIPLL_PLL_CTRL_WE              (HW_CLKPWR_BASE+0x238)
#define ADDR_CLKPWR_CLK_DSIPLL_channel_disables         (HW_CLKPWR_BASE+0x23C)
#define ADDR_CLKPWR_CLK_SIMCLK_ENABLE                   (HW_CLKPWR_BASE+0x240)
#define ADDR_CLKPWR_CLK_SIMCLK_MODE                     (HW_CLKPWR_BASE+0x244)
#define ADDR_CLKPWR_CLK_SIM2CLK_ENABLE                  (HW_CLKPWR_BASE+0x248)
#define ADDR_CLKPWR_CLK_SIM2CLK_MODE                    (HW_CLKPWR_BASE+0x24C)
#define ADDR_CLKPWR_CLK_APPSPLL_STABLE_TIME             (HW_CLKPWR_BASE+0x250)
#define ADDR_CLKPWR_CLK_APPSPLL_STABLE_MODE             (HW_CLKPWR_BASE+0x254)
#define ADDR_CLKPWR_CLK_SYSCLK_REQ_POLARITY             (HW_CLKPWR_BASE+0x258)
#define ADDR_CLKPWR_CLK_SYSCLK_REQ_MASK                 (HW_CLKPWR_BASE+0x25C)
#define ADDR_CLKPWR_CLK_SYSCLK_REQ_FORCE                (HW_CLKPWR_BASE+0x260)
#define ADDR_CLKPWR_CLK_SYSCLK_PMU_MODE                 (HW_CLKPWR_BASE+0x264)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_ACTIVE_TIME_LOW  (HW_CLKPWR_BASE+0x268)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_ACTIVE_TIME_HIGH (HW_CLKPWR_BASE+0x26C)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_ACTIVE_TIME_CTRL (HW_CLKPWR_BASE+0x270)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_LOW    \
	(HW_CLKPWR_BASE+0x274)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_HIGH   \
	(HW_CLKPWR_BASE+0x278)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_CTRL   \
	(HW_CLKPWR_BASE+0x27C)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DEEPSLEEP_TIME_LOW   \
	(HW_CLKPWR_BASE+0x280)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DEEPSLEEP_TIME_HIGH  \
	(HW_CLKPWR_BASE+0x284)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DEEPSLEEP_TIME_CTRL  \
	(HW_CLKPWR_BASE+0x288)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DSP_ACTIVE_TIME_LOW  \
	(HW_CLKPWR_BASE+0x298)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DSP_ACTIVE_TIME_HIGH \
	(HW_CLKPWR_BASE+0x29C)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DSP_ACTIVE_TIME_CTRL \
	(HW_CLKPWR_BASE+0x2A0)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_AUDVOC_ACTIVE_TIME_LOW   \
	(HW_CLKPWR_BASE+0x2A4)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_AUDVOC_ACTIVE_TIME_HIGH  \
	(HW_CLKPWR_BASE+0x2A8)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_AUDVOC_ACTIVE_TIME_CTRL  \
	(HW_CLKPWR_BASE+0x2AC)
#define ADDR_CLKPWR_CLK_SYSCLK_EXT_DEBUG_MONITOR_CTRL   \
	(HW_CLKPWR_BASE+0x2B0)
#define ADDR_CLKPWR_CLK_SYSCLK_DEBUG_MON1		\
	(HW_CLKPWR_BASE + 0x2B4)
#define ADDR_CLKPWR_CLK_SYSCLK_DEBUG_MON2		\
	(HW_CLKPWR_BASE + 0x2B8)
#define ADDR_CLKPWR_CLK_SYS_CLK_DEBUG_MON3		\
	(HW_CLKPWR_BASE + 0x2BC)
#define ADDR_CLKPWR_CLK_HSBSC3_ENABLE			\
	(HW_CLKPWR_BASE + 0x2C0)
#define ADDR_CLKPWR_CLK_HSBSC3_MODE			\
	(HW_CLKPWR_BASE + 0x2C4)
/*Registers added for Athena B0 */
#define ADDR_CLKPWR_CLK_V3D_POWEROFF			\
	(HW_CLKPWR_BASE + 0x148)
#define ADDR_CLKPWR_CLK_VDD_V3D_RAMP_COUNT		\
	(HW_CLKPWR_BASE + 0x150)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_AP_ACTIVE_TIME_LOW \
	(HW_CLKPWR_BASE + 0x2A4)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_AP_ACTIVE_TIME_HIGH \
	(HW_CLKPWR_BASE + 0x2A8)
#define ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_AP_ACTIVE_TIME_CTRL \
	(HW_CLKPWR_BASE + 0x2AC)
#define ADDR_CLKPWR_CLK_PLL_ARESET_CTRL			\
	(HW_CLKPWR_BASE + 0x2BC)
/*#define ADDR_CLKPWR_CLK_SYS_CLK_DEBUG_MON3            \
	(HW_CLKPWR_BASE + 0x2C8) */
#define ADDR_CLKPWR_CLK_CLK_52M_RF_CTRL			\
	(HW_CLKPWR_BASE + 0x2CC)
#define ADDR_CLKPWR_CLK_CLK_DSP_MAINPLL_OVERRIDE	\
	(HW_CLKPWR_BASE + 0x2D0)
#define ADDR_CLKPWR_CLK_APPSPLL_ENABLE2			\
	(HW_CLKPWR_BASE + 0x2D4)

#define ADDR_CLKPWR_CLK_PM_SLEEP_REQ_FORCE_PHYS         \
	(BCM21553_CLKPWR_BASE+0x200)
#define ADDR_CLKPWR_CLK_AP_POWER_MODES_PHYS             \
	(BCM21553_CLKPWR_BASE+0x1F4)
#define ADDR_CLKPWR_CLK_POWER_MODES_PHYS                \
	(BCM21553_CLKPWR_BASE+0x9C)
#define ADDR_CLKPWR_CLK_UARTA_ENABLE_PHYS               \
	(BCM21553_CLKPWR_BASE+0x50)

/* Bit masks */

/* SDIO DIV maks */
#define CLK_SDIO_DIV_48_24_SEL                          (1 << 11)
#define CLK_SDIO_DIV_24_SEL                             (1 << 12)
#define CLK_SDIO_DIV_48_24_EN                           (1 << 13)
#define CLK_SDIO_DIV_DIVIDER_MASK                       0x7FF

/* SDRAM Clk */
#define CLK_SDRAM_SYC_MODE                              (1<<2)

/* NVSRAM CLK */
#define CLK_NVSRAM_SYNC_MODE                            (1<<2)

/* SPI DIV */
#define CLK_SPI_DIV_104_EN                              (1<<3)

/* CLK_AP_POWER_MODES register bit defines */
#define CLK_POWER_MODES_PEDESTAL_ENABLE                 (1 << 1)
#define CLK_POWER_MODES_DEEPSLEEP_ENABLE                (1 << 0)
#define CLK_APPS_DORMANT_ENABLE                         (1 << 0)
#define CLK_DEEPSLEEP_AHBOFF_ENABLE                     (1 << 3)

/* CLK_APPSARM_WARMBOOT register bit defines */
#define CLK_APPSARM_WARMBOOT_HIGHBOOT_VEC_LOCK          (1 << 0)

/* CLKPWR_CLK_ANALOG_PHASES_ENABLE */
#define USB_PHY_REF_CLK                                 (1 << 1)
#define AUDIO_TX_DAC_CLK                                (1 << 0)
#define AUDIO_RX_ADC_CLK                                (1 << 5)
#define AUXADC_DAC_REF_CLK                              (1 << 4)

/* ADDR_CLKPWR_CLK_VIDEO_CODEC_PWROFF */
#define VCODEC_POWER_ON	                                0
#define VCODEC_POWER_OFF                                1
#define VCODEC_POWER_OK                                 (1 << 2)

/* ADDR_CLKPWR_CLK_V3D_POWEROFF */
#define V3D_POWER_ON                                    0
#define V3D_POWER_OFF                                   1
#define V3D_POWER_OK                                    (1 << 2)

/* ADDR_SYSCFG_LCD_AHB_CLK_EN */
#define LCD_AHB_EN					1

/* ADDR_CLKPWR_CLK_SYSCLK_DEBUG_MON2 (from Athena DVFS CDD 2011.doc) */
#define APLL_MON_PLL_OFF                                (0)
#define APLL_MON_PLL_WARMUP                             (1)
#define APLL_MON_OFF2ON_DDRSELFREF_ENTER                (2)
#define APLL_MON_PLL_SELECT                             (3)
#define APLL_MON_OFF2ON_DDRSELFREF_EXIT                 (4)
#define APLL_MON_PLL_ON                                 (5)
#define APLL_MON_PLL_ON_FROZEN                          (6)
#define APLL_MON_ON2OFF_DDRSELFREF_ENTER                (7)
#define APLL_MON_PLL_DESELECT                           (8)
#define APLL_MON_ON2OFF_DDRSELFREF_EXIT                 (9)
#define APLL_MON_PLL_OFF_FROZEN                         (10)
#define APLL_MON_OFF_DDRSELFREF_ENTER                   (11)
#define APLL_MON_PLL_OFF_MODE_CHANGE                    (12)
#define APLL_MON_ON_DDRSELFREF_ENTER                    (13)
#define APLL_MON_PLL_ON_MODE_CHANGE                     (14)

#define APPL_MON_SYS_PEDESTAL                           (1 << 15)
#define APPL_MON_SYS_DEEPSLEEP                          (1 << 14)
#define APPL_MON_SYS_EARLY_DEEPSLEEP                    (1 << 13)
#define APPL_MON_APLL_SELFREF_STATE                     (0x1F << 8)
#define APPL_MON_APLL_SELFREF_STATE_SHIFT               (8)
#define APPL_MON_APLL_SWITCH_STATE                      (0x0F << 4)
#define APPL_MON_APLL_SWITCH_STATE_SHIFT                (4)
#define APPL_MON_APPSPLL_FREQ_SEL                       (1 << 3)
#define APPL_MON_APPSPLL_LOCK                           (1 << 2)
#define APPL_MON_APPSPLL_PWRDN                          (1 << 1)
#define APPL_MON_APPSPLL_SEL                            (1 << 0)

/*ADDR_CLKPWR_CLK_MIPI_DSI_AFE_DIV*/
#define DSI_CLK_DIV_SHIFT	8
#define CAM2_CLK_DIV_SHIFT	4
#define CAM1_CLK_DIV_SHIFT	0

#define MIPI_DSI_AFE_DIV_MASK 0xF

#endif /*__REG_BCM21553_CLK_PWR_H__*/
