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

#ifndef __BCM_LCD_R61581__
#define __BCM_LCD_R61581__

#define MIPI_DCS_NOP					    0x00
#define MIPI_DCS_SOFT_RESET				    0x01
#define MIPI_DCS_GET_RED_CHANNEL		    0x06
#define MIPI_DCS_GET_GREEN_CHANNEL		    0x07
#define MIPI_DCS_GET_BLUE_CHANNEL		    0x08
#define MIPI_DCS_GET_POWER_MODE			    0x0A
#define MIPI_DCS_GET_ADDRESS_MODE		    0x0B
#define MIPI_DCS_GET_PIXEL_FORMAT		    0x0C
#define MIPI_DCS_GET_DISPLAY_MODE		    0x0D
#define MIPI_DCS_GET_SIGNAL_MODE		    0x0E
#define MIPI_DCS_GET_DIAGNOSTIC_RESULTS	    0x0F
#define MIPI_DCS_ENTER_SLEEP_MODE		    0x10
#define MIPI_DCS_EXIT_SLEEP_MODE		    0x11
#define MIPI_DCS_ENTER_PARTIAL_MODE	 	    0x12
#define MIPI_DCS_ENTER_NORMAL_MODE	 	    0x13
#define MIPI_DCS_EXIT_INVERT_MODE		    0x20
#define MIPI_DCS_ENTER_INVERT_MODE	 	    0x21
#define MIPI_DCS_SET_GAMMA_CURVE	  	    0x26
#define MIPI_DCS_SET_DISPLAY_OFF	  	    0x28
#define MIPI_DCS_SET_DISPLAY_ON		  	    0x29
#define MIPI_DCS_SET_COLUMN_ADDRESS  	    0x2A
#define MIPI_DCS_SET_PAGE_ADDRESS	  	    0x2B
#define MIPI_DCS_WRITE_MEMORY_START		    0x2C
#define MIPI_DCS_WRITE_LUT				    0x2D
#define MIPI_DCS_READ_MEMORY_START   	    0x2E
#define MIPI_DCS_SET_PARTIAL_AREA	  	    0x30
#define MIPI_DCS_SET_SCROLL_AREA	  	    0x33
#define MIPI_DCS_SET_TEAR_OFF		  	    0x34
#define MIPI_DCS_SET_TEAR_ON		  	    0x35
#define MIPI_DCS_SET_ADDRESS_MODE   	    0x36
#define MIPI_DCS_SET_SCROLL_START	  	    0x37
#define MIPI_DCS_EXIT_IDLE_MODE			    0x38
#define MIPI_DCS_ENTER_IDLE_MODE	 	    0x39
#define MIPI_DCS_SET_PIXEL_FORMAT	  	    0x3A
#define MIPI_DCS_WRITE_MEMORY_CONTINUE	    0x3C
#define MIPI_DCS_READ_MEMORY_CONTINUE 	    0x3E
#define MIPI_DCS_SET_TEAR_SCANLINE	  	    0x44
#define MIPI_DCS_GET_SCANLINE			    0x45
#define MIPI_DCS_READ_DDB_START			    0xA1
#define MIPI_DCS_READ_DDB_CONTINUE		    0xA8
#define MANUFACTURER_COMMAND_ACCESS_PROTECT	0xB0
#define FRAME_MEMORY_ACCESS_INTERFACE_SETTING	0xB3
#define DISPLAY_MODE_FRAME_MEMORY_WRITE_MODE_SETTING	0xB4
#define PANEL_DRIVING_SETTING	0xC0
#define DISPLAY_TIMING_SETTING_FOR_NORMAL_MODE	0xC1
#define SOURCE_VCOM_GATE_DRIVING_TIMING_SETTING	0xC4
#define GAMMA_SET	0xC8
#define POWER_SETTING_COMMON_SETTING	0xD0
#define VCOM_SETTING	0xD1
#define POWER_SETTING_FOR_NORMAL_MODE	0xD2
#define DITHER_SETTING				0xDA

#define RESET_SEQ {WR_CMND, MIPI_DCS_SET_COLUMN_ADDRESS, 0},\
		{WR_DATA, 0x00, (dev->col_start) >> 8},\
		{WR_DATA, 0x00, dev->col_start & 0xFF},\
		{WR_DATA, 0x00, (dev->col_end) >> 8},\
		{WR_DATA, 0x00, dev->col_end & 0xFF},\
		{WR_CMND, MIPI_DCS_SET_PAGE_ADDRESS, 0},\
		{WR_DATA, 0x00, (dev->row_start) >> 8},\
		{WR_DATA, 0x00, dev->row_start & 0xFF},\
		{WR_DATA, 0x00, (dev->row_end) >> 8},\
		{WR_DATA, 0x00, dev->row_end & 0xFF},\
		{WR_CMND, MIPI_DCS_WRITE_MEMORY_START, 0}

#define LCD_CMD(x) (x)
#define LCD_DATA(x) (x)

#ifdef CONFIG_ENABLE_QVGA
#define LCD_HEIGHT              320
#define LCD_WIDTH               240
#else
#define LCD_HEIGHT              480
#define LCD_WIDTH               320
#endif

#ifdef CONFIG_ENABLE_QVGA
#define PANEL_HEIGHT              480
#define PANEL_WIDTH               320
#endif

#ifdef CONFIG_ARGB8888
#define LCD_BITS_PER_PIXEL	32
#else
#define LCD_BITS_PER_PIXEL	16
#endif

#define TEAR_SCANLINE	480

const char *LCD_panel_name = "RENESAS HVGA R61581 Controller";

int LCD_num_panels = 1;
LCD_Intf_t LCD_Intf = LCD_Z80;
#ifdef CONFIG_ARGB8888
LCD_Bus_t LCD_Bus = LCD_16BIT;
#else
LCD_Bus_t LCD_Bus = LCD_18BIT;
#endif

CSL_LCDC_PAR_SPEED_t timingReg = {31, 24, 0, 1, 2, 0};
CSL_LCDC_PAR_SPEED_t timingMem = {31, 24, 0, 1, 2, 0};

LCD_dev_info_t LCD_device[1] = {
	{
	 .panel		= LCD_main_panel,
	 .height	= LCD_HEIGHT,
	 .width		= LCD_WIDTH,
	 .bits_per_pixel = LCD_BITS_PER_PIXEL,
	 .te_supported	= true}
};

Lcd_init_t power_on_seq[] = {
	{SLEEP_MS, 0, 10},
	{WR_CMND_DATA, MANUFACTURER_COMMAND_ACCESS_PROTECT, 0},
	{WR_CMND, FRAME_MEMORY_ACCESS_INTERFACE_SETTING, 0},
	{WR_DATA, 0x00, 0x02},
	{WR_DATA, 0x00, 0x00},
	{WR_DATA, 0x00, 0x00},
	{WR_DATA, 0x00, 0x00},
	{WR_CMND_DATA, DISPLAY_MODE_FRAME_MEMORY_WRITE_MODE_SETTING, 0x00},
	{WR_CMND, PANEL_DRIVING_SETTING, 0},
	{WR_DATA, 0x00, 0x13},
	{WR_DATA, 0x00, 0x3B},
	{WR_DATA, 0x00, 0x00},
	{WR_DATA, 0x00, 0x03},
	{WR_DATA, 0x00, 0x00},
	{WR_DATA, 0x00, 0x01},
	{WR_DATA, 0x00, 0x00},
	{WR_DATA, 0x00, 0x43},
	{WR_CMND, DISPLAY_TIMING_SETTING_FOR_NORMAL_MODE, 0},
	{WR_DATA, 0x00, 0x08},
	{WR_DATA, 0x00, 0x12},
	{WR_DATA, 0x00, 0x08},
	{WR_DATA, 0x00, 0x08},
	{WR_CMND, SOURCE_VCOM_GATE_DRIVING_TIMING_SETTING, 0},
	{WR_DATA, 0x00, 0x11},
	{WR_DATA, 0x00, 0x07},
	{WR_DATA, 0x00, 0x03},
	{WR_DATA, 0x00, 0x03},
	{WR_CMND, GAMMA_SET, 0},
	{WR_DATA, 0x00, 0x04},
	{WR_DATA, 0x00, 0x09},
	{WR_DATA, 0x00, 0x16},
	{WR_DATA, 0x00, 0x5A},
	{WR_DATA, 0x00, 0x02},
	{WR_DATA, 0x00, 0x0A},
	{WR_DATA, 0x00, 0x16},
	{WR_DATA, 0x00, 0x05},
	{WR_DATA, 0x00, 0x00},
	{WR_DATA, 0x00, 0x32},
	{WR_DATA, 0x00, 0x05},
	{WR_DATA, 0x00, 0x16},
	{WR_DATA, 0x00, 0x0A},
	{WR_DATA, 0x00, 0x53},
	{WR_DATA, 0x00, 0x08},
	{WR_DATA, 0x00, 0x16},
	{WR_DATA, 0x00, 0x09},
	{WR_DATA, 0x00, 0x04},
	{WR_DATA, 0x00, 0x32},
	{WR_DATA, 0x00, 0x00},
	{WR_CMND, MIPI_DCS_SET_COLUMN_ADDRESS, 0},
	{WR_DATA, 0x00, 0x00},
	{WR_DATA, 0x00, 0x00},
	{WR_DATA, 0x00, 0x01},
	{WR_DATA, 0x00, 0x3F},
	{WR_CMND, MIPI_DCS_SET_PAGE_ADDRESS, 0},
	{WR_DATA, 0x01, 0x00},
	{WR_DATA, 0x00, 0x00},
	{WR_DATA, 0x00, 0x01},
	{WR_DATA, 0x00, 0xDF},
	{WR_CMND_DATA, MIPI_DCS_SET_TEAR_ON, 0x00},
#ifdef CONFIG_ARGB8888
	{WR_CMND_DATA, MIPI_DCS_SET_PIXEL_FORMAT, 0x07},
#else
	{WR_CMND_DATA, MIPI_DCS_SET_PIXEL_FORMAT, 0x06},
#endif
	{WR_CMND, MIPI_DCS_SET_TEAR_SCANLINE, 0},
	{WR_DATA, 0x00, TEAR_SCANLINE >> 8},
	{WR_DATA, 0x00, TEAR_SCANLINE & 0xFF},
	{WR_CMND, MIPI_DCS_WRITE_MEMORY_START, 0},
	{WR_CMND, MIPI_DCS_EXIT_SLEEP_MODE, 0},
	{SLEEP_MS, 0, 120},
	{WR_CMND, POWER_SETTING_COMMON_SETTING, 0x00},
	{WR_DATA, 0x00, 0x07},
	{WR_DATA, 0x00, 0x07},
	{WR_DATA, 0x00, 0x1E},
	{WR_DATA, 0x00, 0x03},
	{WR_CMND, VCOM_SETTING, 0x00},
	{WR_DATA, 0x00, 0x03},
	{WR_DATA, 0x00, 0x52},
	{WR_DATA, 0x00, 0x10},
	{WR_CMND, POWER_SETTING_FOR_NORMAL_MODE, 0x00},
	{WR_DATA, 0x00, 0x03},
	{WR_DATA, 0x00, 0x24},
	{WR_CMND, MIPI_DCS_SET_DISPLAY_ON, 0},
	{SLEEP_MS, 0, 10},
#ifdef CONFIG_ARGB8888
	{WR_CMND_DATA, DITHER_SETTING, 0x01},
#endif
	{CTRL_END, 0, 0},
};

Lcd_init_t power_off_seq[] = {
	{WR_CMND, MIPI_DCS_SET_DISPLAY_OFF, 0},
	{WR_CMND, MIPI_DCS_ENTER_SLEEP_MODE, 0},
	{SLEEP_MS, 0, 120},
	{CTRL_END, 0, 0},
};

#endif
