/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/broadcom/displays/lcd_s6d05a1x01.h
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
*  lcd_tiama_s6d04h0.h
*
*  PURPOSE:
*    This is the LCD-specific code for a S6d05a1x01 module.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#ifndef __BCM_LCD_ILI9341_BOE_H
#define __BCM_LCD_ILI9341_BOE_H

//  LCD command definitions
#define PIXEL_FORMAT_RGB565  0x55   // for MPU & RGB interface
#define PIXEL_FORMAT_RGB666  0x66   // for MPU & RGB interface
#define PIXEL_FORMAT_RGB888  0x77   // for MPU & RGB interface

#define LCD_HEIGHT		320
#define LCD_WIDTH		240

#define LCD_BITS_PER_PIXEL      32
#define TEAR_LINE 500

#define LCD_CMD(x) (x)
#define LCD_DATA(x) (x)

#define PANEL_BOE           0x61bcd1
#define LCD_DET 32

#define RESET_SEQ 	{WR_CMND, 0x2A,0},\
	{WR_DATA, 0, (dev->col_start) >> 8},\
	{WR_DATA, 0, dev->col_start & 0xFF},\
	{WR_DATA, 0, (dev->col_end) >> 8},\
	{WR_DATA, 0, dev->col_end & 0xFF},\
	{WR_CMND, 0x2B,0},\
	{WR_DATA, 0, (dev->row_start) >> 8},\
	{WR_DATA, 0, dev->row_start & 0xFF},\
	{WR_DATA, 0, (dev->row_end) >> 8},\
	{WR_DATA, 0, dev->row_end & 0xFF},\
	{WR_CMND, 0x2C,0}
	
const char *LCD_panel_name = "S6D04H0X20 LCD";

int LCD_num_panels = 1;
LCD_Intf_t LCD_Intf = LCD_Z80;
LCD_Bus_t LCD_Bus = LCD_16BIT;
CSL_LCDC_PAR_SPEED_t timingReg = {24, 25, 0, 3, 3, 0};
CSL_LCDC_PAR_SPEED_t timingMem = {24, 25, 0, 3, 3, 0};

LCD_dev_info_t LCD_device[1] = {
	{
	 .panel		= LCD_main_panel,
	 .height	= LCD_HEIGHT,
	 .width		= LCD_WIDTH,
	 .bits_per_pixel= LCD_BITS_PER_PIXEL,
	 .te_supported	= true,
	 }
};

Lcd_init_t power_on_seq_s6d04h0_boe[] =
{
	// Initial Sequence
	{WR_CMND, 0xCF,0}, // (PASSWD1)
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0xFA},	
	{WR_DATA, 0, 0xFF},

	{WR_CMND, 0xED,0}, // (PASSWD1)
	{WR_DATA, 0, 0x67},
	{WR_DATA, 0, 0x03},	
	{WR_DATA, 0, 0x12},
	{WR_DATA, 0, 0x81},

	{WR_CMND, 0xE8,0}, // (PASSWD1)
	{WR_DATA, 0, 0x85},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x78},

    	{WR_CMND, 0xE9,0}, // (PASSWD1)
	{WR_DATA, 0, 0x04},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x78},

	{WR_CMND, 0xCB,0}, // (PASSWD1)
	{WR_DATA, 0, 0x39},
	{WR_DATA, 0, 0x2c},	
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x34},
	{WR_DATA, 0, 0x02},

	{WR_CMND, 0xF7,0}, // (PASSWD1)
	{WR_DATA, 0, 0x20},

	{WR_CMND, 0xEA,0}, // (PASSWD1)
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},

	{WR_CMND, 0xC0,0}, // (PASSWD1)
	{WR_DATA, 0, 0x15},

	{WR_CMND, 0xC1,0}, // (PASSWD1)
	{WR_DATA, 0, 0x10},

	{WR_CMND, 0xC5,0}, // (PASSWD1)
	{WR_DATA, 0, 0x14},
	{WR_DATA, 0, 0x44},

	{WR_CMND, 0xC7,0}, // (PASSWD1)
	{WR_DATA, 0, 0x00},

    	{WR_CMND, 0xEF,0}, // (PASSWD1)
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x80},
      	{WR_DATA, 0, 0x02},

	{WR_CMND, 0x35,0}, // (PASSWD1)
	{WR_DATA, 0, 0x00},

	{WR_CMND, 0x36,0}, // (PASSWD1)
	{WR_DATA, 0, 0x08},

	{WR_CMND, 0x3A,0}, // (PASSWD1)
	{WR_DATA, 0, 0x06},//66

	{WR_CMND, 0x26,0}, // (PASSWD1)
	{WR_DATA, 0, 0x01},

	{WR_CMND, 0xB1,0}, // (PASSWD1)
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x17},

	{WR_CMND, 0xB4,0}, // (PASSWD1)
	{WR_DATA, 0, 0x02},

    	{WR_CMND, 0xB5,0}, // (PASSWD1)
	{WR_DATA, 0, 0x07},
	{WR_DATA, 0, 0x07},
	{WR_DATA, 0, 0x0A},
      	{WR_DATA, 0, 0x26},

	{WR_CMND, 0xB6,0}, // (PASSWD1)
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0xA2},

	{WR_CMND, 0xF6,0}, // (PASSWD1)
	{WR_DATA, 0, 0x01},
	{WR_DATA, 0, 0x30},//33
	{WR_DATA, 0, 0x00},//00

	{WR_CMND, 0xF2,0}, // (PASSWD1)
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xE0,0}, // (PWRCTL)
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x19},	
	{WR_DATA, 0, 0x18},
	{WR_DATA, 0, 0x0B},	
	{WR_DATA, 0, 0x0E},
	{WR_DATA, 0, 0x07},	
	{WR_DATA, 0, 0x45},
	{WR_DATA, 0, 0xA7},	
	{WR_DATA, 0, 0x35},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x0E},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x15},
	{WR_DATA, 0, 0x13},
	{WR_DATA, 0, 0x0F},

	{WR_CMND, 0xE1,0}, // (PWRCTL)
	{WR_DATA, 0, 0x05},
	{WR_DATA, 0, 0x24},	
	{WR_DATA, 0, 0x27},
	{WR_DATA, 0, 0x05},	
	{WR_DATA, 0, 0x11},
	{WR_DATA, 0, 0x08},	
	{WR_DATA, 0, 0x3A},
	{WR_DATA, 0, 0x58},	
	{WR_DATA, 0, 0x4A},
	{WR_DATA, 0, 0x07},
	{WR_DATA, 0, 0x11},
	{WR_DATA, 0, 0x0F},
	{WR_DATA, 0, 0x2A},
	{WR_DATA, 0, 0x2B},
	{WR_DATA, 0, 0x00},


	{WR_CMND, 0x11,0}, // (SLPOUT)

	{SLEEP_MS, 0, 130}, // 120ms

	{WR_CMND, 0x29,0}, // (DISPON)

	{SLEEP_MS, 0, 40}, // 40ms
	
	{CTRL_END, 0, 0}
};

Lcd_init_t power_on_seq_s6d04k1_sdi[] =
{
    	{CTRL_END, 0, 0}
};

Lcd_init_t power_off_seq[] =
{
	{WR_CMND, 0x10,0}, // (SLPIN)
	{SLEEP_MS, 0, 120},
	{CTRL_END, 0, 0}
};
#endif /* __BCM_LCD_TIAMA_S6D04H0_H */

