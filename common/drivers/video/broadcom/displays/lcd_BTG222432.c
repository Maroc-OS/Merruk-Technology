/*****************************************************************************
*  Copyright 2001 - 2007 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/****************************************************************************
*
*  lcd.c
*
*  PURPOSE:
*    This is the LCD-specific code for a BOE Hyundai BTG222432 module.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/string.h>
#include <linux/module.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/interrupt.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/kernel_stat.h>
#include <linux/broadcom/bcm_sysctl.h>
#include <linux/dma-mapping.h>
#include <plat/dma.h>

#include <asm/byteorder.h>
#include <mach/reg_lcd.h>
#include <mach/reg_sys.h>
#include <linux/broadcom/regaccess.h>
#if defined(CONFIG_BCM_IDLE_PROFILER_SUPPORT)
#include <linux/broadcom/idle_prof.h>
#endif
#include <linux/broadcom/cpu_sleep.h>
#include <asm/mach/irq.h>
#include <asm/io.h>

#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/hw.h>
#include <linux/broadcom/lcd.h>
#include <linux/broadcom/PowerManager.h>
#include <linux/broadcom/lcd_backlight.h>

#include <cfg_global.h>

/* #define DEBUG */
#include <linux/broadcom/lcd_common.h>
#include <linux/broadcom/timer.h>

/* ---- Public Variables ------------------------------------------------- */

/*
 * Attributes of the LCD
 */
#define LCD_HEIGHT              320
#define LCD_WIDTH               240
#define LCD_BITS_PER_PIXEL      16

/* globals for lcd_common.c to use */
const char *LCD_panel_name = "BOE Hyundai BTG222432";

LCD_Intf_t LCD_Intf = LCD_Z80;
LCD_Volt_t LCD_Volt = LCD_30V;
LCD_Bus_t LCD_Bus = LCD_18BIT;

int LCD_num_panels = 1;

LCD_dev_info_t LCD_device[1] = {
	{
	 LCD_main_panel,
	 LCD_HEIGHT,
	 LCD_WIDTH,
	 LCD_BITS_PER_PIXEL}
};

/* ---- Private Constants and Types -------------------------------------- */

/* commands must be in bits 8:1, and cmd-data split into 17:10,8:1 */
/* (however plain RGB data does not need these hacks) */
#define LCD_CMD(cmd) (((u32)(cmd) & 0xFF) << 1)
#define LCD_DATA(data) ((((u32)(data) & 0xFF00) << 2) | \
    (((u32)(data) & 0xFF) << 1))

/* Renesas R61505U instruction set: */
#define REN_DEVICE_CODE_READ   0x00
#define REN_DEVICE_ID    0x1505
#define REN_DRIVER_OUTPUT_CONTROL  0x01
#define REN_LCD_DRIVE_WAVEFORM_CONTROL  0x02
#define REN_LCD_ENTRY_MODE   0x03
#define REN_RESIZE_CONTROL    0x04
/* 0x05..0x06 setting inhibited */
#define REN_DISPLAY_CONTROL_1   0x07
#define REN_DISPLAY_CONTROL_2   0x08
#define REN_DISPLAY_CONTROL_3   0x09
#define REN_DISPLAY_CONTROL_4   0x0a
/* 0x0b       setting inhibited */
#define REN_EXTERNAL_DISPLAY_CONTROL_1  0x0c
#define REN_FRAME_MARKER_CONTROL  0x0d
/* 0x0e       setting inhibited */
#define REN_EXTERNAL_DISPLAY_CONTROL_2  0x0f

#define REN_POWER_CONTROL_1   0x10
#define REN_POWER_CONTROL_2   0x11
#define REN_POWER_CONTROL_3   0x12
#define REN_POWER_CONTROL_4   0x13
/* 0x15..0x16 setting inhibited */
#define REN_POWER_CONTROL_5   0x17
/* 0x18..0x1f setting inhibited */

#define REN_RAM_ADDRESS_SET_HORIZONTAL  0x20
#define REN_RAM_ADDRESS_SET_VERTICAL  0x21
#define REN_READ_WRITE_DATA_GRAM  0x22
/* 0x23..0x27 setting inhibited */
#define REN_NVM_DATA_READ   0x28
#define REN_VCOM_HIGH_VOLTAGE_1   0x29
#define REN_VCOM_HIGH_VOLTAGE_2   0x2a
/* 0x2b..0x2f setting inhibited */

/* gamma control 1..14 */
#define REN_GAMMA_CONTROL(n)   (0x30 + (n) - 1)
/* 0x3e..0x3f setting inhibited */

/* 0x40..0x4f setting inhibited */

#define REN_WINDOW_HORIZONTAL_RAM_ADDRESS_START 0x50
#define REN_WINDOW_HORIZONTAL_RAM_ADDRESS_END 0x51
#define REN_WINDOW_VERTICAL_RAM_ADDRESS_START 0x52
#define REN_WINDOW_VERTICAL_RAM_ADDRESS_END 0x53
/* 0x54..0x5f setting inhibited */

#define REN_DRIVER_OUTPUT_CONTROL_B  0x60
#define REN_BASE_IMAGE_DISPLAY_CONTROL  0x61
/* 0x62..0x69 setting inhibited */
#define REN_VERTICAL_SCROLL_CONTROL  0x6a
/* 0x6b..0x6f setting inhibited */

/* 0x70..0x7f setting inhibited */

#define REN_PARTIAL_IMAGE_1_DISPLAY_POSITION 0x80
#define REN_PARTIAL_IMAGE_1_RAM_ADDRESS_START 0x81
#define REN_PARTIAL_IMAGE_1_RAM_ADDRESS_END 0x82
#define REN_PARTIAL_IMAGE_2_DISPLAY_POSITION 0x83
#define REN_PARTIAL_IMAGE_2_RAM_ADDRESS_START 0x84
#define REN_PARTIAL_IMAGE_2_RAM_ADDRESS_END 0x85
/* 0x86..0x8f setting inhibited */

#define REN_PANEL_INTERFACE_CONTROL_1  0x90
/* 0x91       setting inhibited */
#define REN_PANEL_INTERFACE_CONTROL_2  0x92
#define REN_PANEL_INTERFACE_CONTROL_3  0x93
/* 0x94       setting inhibited */
#define REN_PANEL_INTERFACE_CONTROL_4  0x95
/* 0x96       setting inhibited */
#define REN_PANEL_INTERFACE_CONTROL_5  0x97
#define REN_PANEL_INTERFACE_CONTROL_6  0x98
/* 0x99..0x9f setting inhibited */

#define REN_NVM_ACCESS_CONTROL_1  0xa0
#define REN_NVM_ACCESS_CONTROL_2  0xa1
/* 0xa2..0xa3 setting inhibited */
#define REN_CALIBRATION_CONTROL   0xa4
/* 0xa5..0xaf setting inhibited */

/* 0xb0..0xff setting inhibited */

/* end of Renesas command set */

/* ---- Private Variables ------------------------------------------------ */

typedef struct lcd_init_t {
	uint8_t reg;
	uint16_t val;

} Lcd_init_t;

static Lcd_init_t g_init[] = {
	/* initialization data from MSP/Nucleus driver [BOE Hyundai document] */
	{0x07, 0x0000},
	{0x12, 0x0000},
	{0xFF, 15},		/* delay ms */
	{0xA4, 0x0001},
	{0xFF, 15},		/* delay ms */
	{0x60, 0x2700},
	{0x08, 0x0404},
	{0x30, 0x0307},		/* gamma setting */
	{0x31, 0x0303},		/* gamma setting */
	{0x32, 0x0303},		/* gamma setting */
	{0x33, 0x0202},		/* gamma setting */
	{0x34, 0x0202},		/* gamma setting */
	{0x35, 0x0202},		/* gamma setting */
	{0x36, 0x1F1F},		/* gamma setting */
	{0x37, 0x0303},		/* gamma setting */
	{0x38, 0x0303},		/* gamma setting */
	{0x39, 0x0303},		/* gamma setting */
	{0x3A, 0x0202},		/* gamma setting */
	{0x3B, 0x0102},		/* gamma setting */
	{0x3C, 0x0204},		/* gamma setting */
	{0x3D, 0x0000},
	{0x07, 0x0001},

	{0x17, 0x0001},
	{0x10, 0x17B0},
	{0x11, 0x0007},
	{0xFF, 60},		/* delay ms */
	{0x12, 0x0119},
	{0x13, 0x0700},		/* was 0x1F00 */
	{0x29, 0x0000},		/* was 0x001C */
	{0x12, 0x0139},
	{0xFF, 40},		/* delay ms */
	{0x01, 0x0100},
	{0x02, 0x0300},
	{0x03, 0x52B0},		/* was 0x50B0 */
	{0x09, 0x0000},
	{0x0A, 0x0008},
	{0x0C, 0x0000},
	{0x0D, 0x0000},
	/* { 0x15, 0x2203 }, missing from MSP/Nucleus driver? */
	{0x50, 0x0000},
	{0x51, 0x00EF},
	{0x52, 0x0000},
	{0x53, 0x013F},
	{0x61, 0x0001},		/* was 0x0000 */
	{0x90, 0x0011},
	{0x92, 0x0100},
	{0x93, 0x0001},
	{0xFF, 15},		/* delay ms */
	{0x07, 0x0021},
	{0x07, 0x0061},
	{0xFF, 15},		/* delay ms */
	{0x07, 0x0173},
	{0xFF, 15},		/* delay ms */

	/* 0x22, image Data */

	{0, 0}
};

static Lcd_init_t g_poweroff[] = {
	/* poweroff data from MSP/Nucleus driver [BOE Hyundai document] */
	{0x07, 0x0072},
	{0xFF, 100},		/* delay ms */
	{0x07, 0x0001},
	{0xFF, 15},		/* delay ms */
	{0x07, 0x0000},
	{0xFF, 15},		/* delay ms */
	{0x12, 0x0000},
	{0x17, 0x0000},
	{0x10, 0x0000},
	{0x11, 0x0000},
	{0x13, 0x0000},

};

void lcd_ResetStartAddr(LCD_dev_info_t *dev)
{
	WRITE_LCD_CMD(LCD_CMD(REN_WINDOW_HORIZONTAL_RAM_ADDRESS_START));
	WRITE_LCD_PARAM(LCD_DATA(dev->col_start));

	WRITE_LCD_CMD(LCD_CMD(REN_WINDOW_HORIZONTAL_RAM_ADDRESS_END));
	WRITE_LCD_PARAM(LCD_DATA(dev->col_end));

	WRITE_LCD_CMD(LCD_CMD(REN_WINDOW_VERTICAL_RAM_ADDRESS_START));
	WRITE_LCD_PARAM(LCD_DATA(dev->row_start));

	WRITE_LCD_CMD(LCD_CMD(REN_WINDOW_VERTICAL_RAM_ADDRESS_END));
	WRITE_LCD_PARAM(LCD_DATA(dev->row_end));

	/* reset horizontal start address */
	WRITE_LCD_CMD(LCD_CMD(REN_RAM_ADDRESS_SET_HORIZONTAL));
	WRITE_LCD_PARAM(LCD_DATA(dev->col_start));

	/* reset vertical start address */
	WRITE_LCD_CMD(LCD_CMD(REN_RAM_ADDRESS_SET_VERTICAL));
	WRITE_LCD_PARAM(LCD_DATA(dev->row_start));
}

void lcd_setup_for_data(LCD_dev_info_t *dev)
{
	lcd_ResetStartAddr(dev);
	WRITE_LCD_CMD(LCD_CMD(REN_READ_WRITE_DATA_GRAM));
}

static int lcd_send_cmd_sequence(Lcd_init_t *init)
{
	int i;

	LCD_PUTS("enter");

	for (i = 0; init[i].reg != 0x0; i++) {
		if (init[i].reg == 0xFF) {
			msleep(init[i].val);
		} else {
			WRITE_LCD_CMD(LCD_CMD(init[i].reg));
			WRITE_LCD_PARAM(LCD_DATA(init[i].val));
		}
	}

	/* lcd_ResetStartAddr(&LCD_device[0]); */

	/* set screen to a single color */
	/* WRITE_LCD_CMD(LCD_CMD(REN_READ_WRITE_DATA_GRAM)); */

	/* for (i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) */
	/*  WRITE_LCD_DATA(LCD_COLOR_BLACK); */

	LCD_PUTS("done");
	return 0;

}				/* lcd_send_cmd_sequence */

void lcd_init_panels(void)
{
	LCD_PUTS("enter");

	/* lcd_ResetStartAddr(&LCD_device[0]); */
	/* lcd_backlight_enable(15); */

	lcd_send_cmd_sequence(g_init);

}				/* lcd_init_controller */

void lcd_poweroff_panels(void)
{
	LCD_PUTS("enter");
	lcd_send_cmd_sequence(g_poweroff);

}
