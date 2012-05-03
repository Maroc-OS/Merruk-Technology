/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/broadcom/displays/lcd_s6d04k1.h
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
*  lcd_s6d04k1.h
*
*  PURPOSE:
*    This is the LCD-specific code for a S6d05a1x01 module.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#ifndef __BCM_LCD_S6D04K1_H
#define __BCM_LCD_S6D04K1_H

//  LCD command definitions
#define PIXEL_FORMAT_RGB565  0x55   // for MPU & RGB interface
#define PIXEL_FORMAT_RGB666  0x66   // for MPU & RGB interface
#define PIXEL_FORMAT_RGB888  0x77   // for MPU & RGB interface

#define LCD_HEIGHT		240
#define LCD_WIDTH		320

#define LCD_BITS_PER_PIXEL      16
#define TEAR_LINE 500

#define PANEL_BOE           0x61a511

#define LCD_CMD(x) (x)
#define LCD_DATA(x) (x)

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
	
const char *LCD_panel_name = "S6D04K1 LCD";

int LCD_num_panels = 1;
LCD_Intf_t LCD_Intf = LCD_Z80;
LCD_Bus_t LCD_Bus = LCD_18BIT;
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


Lcd_init_t power_on_seq_s6d04k1_sdi[] =
{
	// Initial Sequence
	{WR_CMND, 0xF0,0}, // (PASSWD1)
	{WR_DATA, 0, 0x5A},
	{WR_DATA, 0, 0x5A},	

    	{WR_CMND, 0xF0,0}, // (PASSWD2)
	{WR_DATA, 0, 0x5A},
	{WR_DATA, 0, 0x5A},	
	
	{WR_CMND, 0xF4,0}, // (PWRCTL)
	{WR_DATA, 0, 0x09},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x21},
	{WR_DATA, 0, 0x47},	
	{WR_DATA, 0, 0x01},
	{WR_DATA, 0, 0x02},	
	{WR_DATA, 0, 0x2A},
	{WR_DATA, 0, 0x64},
	{WR_DATA, 0, 0x05},
	{WR_DATA, 0, 0x2A},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x05},

	{WR_CMND, 0xF5,0}, // (VCMCTL)
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x4D},	
	{WR_DATA, 0, 0x5C},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x09},	
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x01},
	{WR_DATA, 0, 0x01},

	{WR_CMND, 0xF6,0}, // (SRCCTL)
	{WR_DATA, 0, 0x01},
	{WR_DATA, 0, 0x01},	
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x04},
	{WR_DATA, 0, 0x0C},	
	{WR_DATA, 0, 0x02},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x07},

	{WR_CMND, 0x35,0}, // (TEON)
	{WR_DATA, 0, 0x00},

	{WR_CMND, 0x36,0}, // (MDACTL)
	{WR_DATA, 0, 0xC8},//48

	{WR_CMND, 0x3A,0}, // (COLMOD)
	{WR_DATA, 0, 0x66},//55

	{WR_CMND, 0xF7,0}, // (IFCTL)
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00}, 
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},

	{WR_CMND, 0xF2,0}, // (DISCTL)
	{WR_DATA, 0, 0x1E},
	{WR_DATA, 0, 0x95}, 
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x08}, 
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x19},
	{WR_DATA, 0, 0x48},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x07},
	{WR_DATA, 0, 0x01},

	{WR_CMND, 0xF9,0}, // (GAMMASEL)
	{WR_DATA, 0, 0x04},
	
	{WR_CMND, 0xFA,0}, // (PGAMMACTL)
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x0B},
	{WR_DATA, 0, 0x1B},
	{WR_DATA, 0, 0x14},
	{WR_DATA, 0, 0x28},
	{WR_DATA, 0, 0x1F},
	{WR_DATA, 0, 0x33},
	{WR_DATA, 0, 0x2A},
	{WR_DATA, 0, 0x25},
	{WR_DATA, 0, 0x23},
	{WR_DATA, 0, 0x00},
        
	{WR_CMND, 0xFB,0}, // (NGAMMACTL)
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x23},
	{WR_DATA, 0, 0x25},
	{WR_DATA, 0, 0x27},
	{WR_DATA, 0, 0x2C},
	{WR_DATA, 0, 0x1F},
	{WR_DATA, 0, 0x2E},
	{WR_DATA, 0, 0x20},
	{WR_DATA, 0, 0x1B},
	{WR_DATA, 0, 0x0B},
	{WR_DATA, 0, 0x00},

	{WR_CMND, 0xF9,0}, // (GAMMASEL)
	{WR_DATA, 0, 0x03},
	
	{WR_CMND, 0xFA,0}, // (PGAMMACTL)
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x24},
	{WR_DATA, 0, 0x0B},
	{WR_DATA, 0, 0x14},
	{WR_DATA, 0, 0x1A},
	{WR_DATA, 0, 0x33},
	{WR_DATA, 0, 0x2D},
	{WR_DATA, 0, 0x22},
	{WR_DATA, 0, 0x17},
	{WR_DATA, 0, 0x11},
	{WR_DATA, 0, 0x0B},
	{WR_DATA, 0, 0x00},
        
	{WR_CMND, 0xFB,0}, // (NGAMMACTL)
	{WR_DATA, 0, 0x24},
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x0B},
	{WR_DATA, 0, 0x11},
	{WR_DATA, 0, 0x14},
	{WR_DATA, 0, 0x1B},
	{WR_DATA, 0, 0x11},
	{WR_DATA, 0, 0x39},
	{WR_DATA, 0, 0x26},
	{WR_DATA, 0, 0x14},
	{WR_DATA, 0, 0x0B},
	{WR_DATA, 0, 0x00},

	{WR_CMND, 0x11,0}, // (SLPOUT)

	{SLEEP_MS, 0, 130}, // 120ms
        
	{WR_CMND, 0xF0,0}, // (PASSWD1)
	{WR_DATA, 0, 0xA5},
	{WR_DATA, 0, 0xA5},

	{WR_CMND, 0x2A,0}, 
	{WR_DATA, 0, 0x01},        
	{WR_DATA, 0, 0x3F},	
	
	{WR_CMND, 0x2B,0}, 
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0xEF},
	
	{WR_CMND, 0x29,0}, // (DISPON) 	
	
	{SLEEP_MS, 0, 40}, // 40ms
	
	{CTRL_END, 0, 0}
};

Lcd_init_t power_on_seq_s6d04h0_boe[] =
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


