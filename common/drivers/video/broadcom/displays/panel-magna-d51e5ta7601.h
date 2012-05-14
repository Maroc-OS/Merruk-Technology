/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/video/broadcom/displays/panel-magna-d51e5ta7601.c
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

/****************************************************************************
*
*  lcd.c
*
*  PURPOSE:
*    This is the LCD-specific code for a BOE Hyundai BTG222432 module.
*
*****************************************************************************/

#ifndef __BCM_LCD_D51E5TA7601__
#define __BCM_LCD_D51E5TA7601__

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

/*  LCD command definitions */
#define SRAMWRCNTL      0x0022
#define WIN_COL_E       0x0044
#define WIN_COL_S       0x0045
#define WIN_ROW_E       0x0046
#define WIN_ROW_S       0x0047
#define GRAM_ADR_ROW_S  0x0020	/* init to 0   , UPDATE FIRST */
#define GRAM_ADR_COL_S  0x0021	/* init to 319 , UPDATE LAST */

#define RESET_SEQ {WR_CMND_DATA, WIN_COL_E, dev->width - 1 - dev->col_start},\
	{WR_CMND_DATA, WIN_COL_S, dev->width - 1 - dev->col_end},\
	{WR_CMND_DATA, WIN_ROW_E, dev->row_end},\
	{WR_CMND_DATA, WIN_ROW_S, dev->row_start},\
	{WR_CMND_DATA, GRAM_ADR_ROW_S, dev->row_start},\
	{WR_CMND_DATA, GRAM_ADR_COL_S, dev->width - 1 - dev->col_start},\
	{WR_CMND, REN_READ_WRITE_DATA_GRAM, 0}

#define LCD_HEIGHT              480
#define LCD_WIDTH               320
#define LCD_BITS_PER_PIXEL      16

const char *LCD_panel_name = "Truly 320x480 with Magnachip D51E5TA7601";

int LCD_num_panels = 1;
LCD_Intf_t LCD_Intf = LCD_Z80;
LCD_Bus_t LCD_Bus = LCD_18BIT;
CSL_LCDC_PAR_SPEED_t timingReg = {24, 25, 0, 3, 3, 0};
CSL_LCDC_PAR_SPEED_t timingMem = {24, 25, 0, 1, 1, 0};

LCD_dev_info_t LCD_device[1] = {
	{
	 .panel		= LCD_main_panel,
	 .height	= LCD_HEIGHT,
	 .width		= LCD_WIDTH,
	 .bits_per_pixel = LCD_BITS_PER_PIXEL,
	 .te_supported	= true}
};

Lcd_init_t power_on_seq[] = {
	{WR_CMND_DATA, 0x0001, 0x003C},
	{WR_CMND_DATA, 0x0002, 0x0100},
	{WR_CMND_DATA, 0x0003, 0x1020},
	{WR_CMND_DATA, 0x0008, 0x0808},
	{WR_CMND_DATA, 0x000A, 0x0500},
	{WR_CMND_DATA, 0x000B, 0x0000},
	{WR_CMND_DATA, 0x000C, 0x0770},
	{WR_CMND_DATA, 0x000D, 0x0000},
	{WR_CMND_DATA, 0x000E, 0x0001},
	{WR_CMND_DATA, 0x0011, 0x0406},
	{WR_CMND_DATA, 0x0012, 0x000E},
	{WR_CMND_DATA, 0x0013, 0x0222},
	{WR_CMND_DATA, 0x0014, 0x0015},
	{WR_CMND_DATA, 0x0015, 0x4277},
	{WR_CMND_DATA, 0x0016, 0x0000},
	{WR_CMND_DATA, 0x0030, 0x6A50},
	{WR_CMND_DATA, 0x0031, 0x00C9},
	{WR_CMND_DATA, 0x0032, 0xC7BE},
	{WR_CMND_DATA, 0x0033, 0x0003},
	{WR_CMND_DATA, 0x0036, 0x3443},
	{WR_CMND_DATA, 0x003B, 0x0000},
	{WR_CMND_DATA, 0x003C, 0x0000},
	{WR_CMND_DATA, 0x002C, 0x6A50},
	{WR_CMND_DATA, 0x002D, 0x00C9},
	{WR_CMND_DATA, 0x002E, 0xC7BE},
	{WR_CMND_DATA, 0x002F, 0x0003},
	{WR_CMND_DATA, 0x0035, 0x3443},
	{WR_CMND_DATA, 0x0039, 0x0000},
	{WR_CMND_DATA, 0x003A, 0x0000},
	{WR_CMND_DATA, 0x0028, 0x6A50},
	{WR_CMND_DATA, 0x0029, 0x00C9},
	{WR_CMND_DATA, 0x002A, 0xC7BE},
	{WR_CMND_DATA, 0x002B, 0x0003},
	{WR_CMND_DATA, 0x0034, 0x3443},
	{WR_CMND_DATA, 0x0037, 0x0000},
	{WR_CMND_DATA, 0x0038, 0x0000},
	{SLEEP_MS, 0, 20},
	{WR_CMND_DATA, 0x0012, 0x200E},
	{SLEEP_MS, 0, 160},
	{WR_CMND_DATA, 0x0012, 0x2003},
	{SLEEP_MS, 0, 40},
	{WR_CMND_DATA, WIN_COL_E, 0x013F},
	{WR_CMND_DATA, WIN_COL_S, 0x0000},
	{WR_CMND_DATA, WIN_ROW_E, 0x01DF},
	{WR_CMND_DATA, WIN_ROW_S, 0x0000},
	{WR_CMND_DATA, GRAM_ADR_ROW_S, 0x0000},
	{WR_CMND_DATA, GRAM_ADR_COL_S, 0x013F},
	{WR_CMND_DATA, 0x0007, 0x0012},
	{SLEEP_MS, 0, 40},
	{WR_CMND_DATA, 0x0007, 0x0017},
	{CTRL_END, 0, 0}
};

Lcd_init_t power_off_seq[] = {
	{WR_CMND_DATA, 0x07, 0x0072},
	{SLEEP_MS, 0, 100},
	{WR_CMND_DATA, 0x07, 0x0001},
	{SLEEP_MS, 0, 15},
	{WR_CMND_DATA, 0x07, 0x0000},
	{SLEEP_MS, 0, 15},
	{WR_CMND_DATA, 0x12, 0x0000},
	{WR_CMND_DATA, 0x17, 0x0000},
	{WR_CMND_DATA, 0x10, 0x0000},
	{WR_CMND_DATA, 0x11, 0x0000},
	{WR_CMND_DATA, 0x13, 0x0000},
	{CTRL_END, 0, 0},
};

#endif
