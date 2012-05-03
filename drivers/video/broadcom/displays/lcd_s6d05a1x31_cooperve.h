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
#ifndef __BCM_LCD_S6D05A1X31_COOPERVE_H
#define __BCM_LCD_S6D05A1X31_COOPERVE_H

//  LCD command definitions
#define PIXEL_FORMAT_RGB565  0x55   // for MPU & RGB interface
#define PIXEL_FORMAT_RGB666  0x66   // for MPU & RGB interface
#define PIXEL_FORMAT_RGB888  0x77   // for MPU & RGB interface

#define LCD_HEIGHT		480
#define LCD_WIDTH		320

//#define LCD_BITS_PER_PIXEL      16
#define LCD_BITS_PER_PIXEL      32
#define TEAR_LINE 500

#define LCD_CMD(x) (x)
#define LCD_DATA(x) (x)

//#define PANEL_BOE           0x61a511
#define PANEL_BOE           0x61bc11
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

#define PANEL_DTC	0x6bc010
#define PANEL_AUO	0x6b4c10
#define PANEL_SHARP	0x6b1c10 

//const char *LCD_panel_name = "S6D04H0X20 LCD";
const char *LCD_panel_name = "S6D05A1X31 LCD";

int LCD_num_panels = 1;
LCD_Intf_t LCD_Intf = LCD_Z80;
//LCD_Bus_t LCD_Bus = LCD_18BIT;
LCD_Bus_t LCD_Bus = LCD_16BIT;
CSL_LCDC_PAR_SPEED_t timingReg = {24, 25, 0, 1, 2, 0};
CSL_LCDC_PAR_SPEED_t timingMem = {24, 25, 0, 1, 2, 0};

LCD_dev_info_t LCD_device[1] = {
	{
	 .panel		= LCD_main_panel,
	 .height	= LCD_HEIGHT,
	 .width		= LCD_WIDTH,
	 .bits_per_pixel= LCD_BITS_PER_PIXEL,
	 .te_supported	= true,
	 }
};


Lcd_init_t power_on_seq_s5d05a1x31_cooperve_AUO[] =
{
	// Initial Sequence
	{WR_CMND, 0xF0,0}, // (PASSWARD1)
	{WR_DATA, 0, 0x5A},
	{WR_DATA, 0, 0x5A},	

	{WR_CMND, 0xF1,0}, // (PASSWARD2)
	{WR_DATA, 0, 0x5A},
	{WR_DATA, 0, 0x5A},	

	{WR_CMND, 0xF2,0}, // (DISCTL)
	{WR_DATA, 0, 0x3B},
	{WR_DATA, 0, 0x38}, 
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x08},
	
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x08}, 
	
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x54}, 

	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x04},
	{WR_DATA, 0, 0x04}, 

	//Power Setting Sequence
	{WR_CMND, 0xF4,0}, // (PWRCTL)
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x00},
	
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x15},
	{WR_DATA, 0, 0x6B},
	
	{WR_DATA, 0, 0x03}, 

	{WR_CMND, 0xF5,0}, // (VCMCTL)
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x47},	
	{WR_DATA, 0, 0x75},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x00},
	
	{WR_DATA, 0, 0x04},	
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},	 
	{WR_DATA, 0, 0x04},	 

	{WR_CMND, 0xF6,0}, // (SRCCTL)
	{WR_DATA, 0, 0x04},
	{WR_DATA, 0, 0x00},	
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x03},	
	{WR_DATA, 0, 0x01},
	
	{WR_DATA, 0, 0x00},	 

	//Initializing Sequence
	{WR_CMND, 0xF7,0}, // (IFCTL)
	{WR_DATA, 0, 0x48},
	{WR_DATA, 0, 0x80}, 
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x02}, 
	{WR_DATA, 0, 0x00},

	{WR_CMND, 0xF8,0}, // (PANELCTL)
	{WR_DATA, 0, 0x11},
	{WR_DATA, 0, 0x00}, 


	//Gamma Setting
	{WR_CMND, 0xF9,0}, // (GAMMASEL)
	{WR_DATA, 0, 0x24},
	
	{WR_CMND, 0xFA,0}, // (PGAMMACTL)
	{WR_DATA, 0, 0x23},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x18},
	{WR_DATA, 0, 0x1E},
	
	{WR_DATA, 0, 0x22},
	{WR_DATA, 0, 0x29},
	{WR_DATA, 0, 0x1D},
	{WR_DATA, 0, 0x2A},
	{WR_DATA, 0, 0x2F},
	
	{WR_DATA, 0, 0x3A},
	{WR_DATA, 0, 0x3C},
	{WR_DATA, 0, 0x30},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x2A},

	{WR_DATA, 0, 0x00},
 
	{WR_CMND, 0xF9,0}, // (GAMMASEL)
	{WR_DATA, 0, 0x22},
	
	{WR_CMND, 0xFA,0}, // (PGAMMACTL)
	{WR_DATA, 0, 0x30},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x1B},
	{WR_DATA, 0, 0x1B},
	
	{WR_DATA, 0, 0x1F},
	{WR_DATA, 0, 0x25},
	{WR_DATA, 0, 0x1A},
	{WR_DATA, 0, 0x26},
	{WR_DATA, 0, 0x24},
	
	{WR_DATA, 0, 0x25},
	{WR_DATA, 0, 0x22},
	{WR_DATA, 0, 0x2C},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x2A},

	{WR_DATA, 0, 0x00}, 

	{WR_CMND, 0xF9,0}, // (GAMMASEL)
	{WR_DATA, 0, 0x21},
	
	{WR_CMND, 0xFA,0}, // (PGAMMACTL)
	{WR_DATA, 0, 0x30},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x21},
	{WR_DATA, 0, 0x31},
	
	{WR_DATA, 0, 0x33},
	{WR_DATA, 0, 0x32},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x1D},
	{WR_DATA, 0, 0x20},
	
	{WR_DATA, 0, 0x21},
	{WR_DATA, 0, 0x21},
	{WR_DATA, 0, 0x20},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x2A},

	{WR_DATA, 0, 0x00},
         

	//Initializing Sequence
	{WR_CMND, 0x3A,0}, // (COLMOD)
	{WR_DATA, 0, 0x77}, //66

	{WR_CMND, 0x35,0}, // (TEON)
	{WR_DATA, 0, 0x00},

	{WR_CMND, 0x36,0}, // (MADCTL)
	{WR_DATA, 0, 0xD0},

	{WR_CMND, 0x11,0}, // (SLPOUT)

	{SLEEP_MS, 0, 120}, // 120ms

	{WR_CMND, 0x29,0}, // (DISPON) 
	
	{CTRL_END, 0, 0}
};



Lcd_init_t power_on_seq_s5d05a1x31_cooperve_DTC[] =
{
    // Initial Sequence
    {WR_CMND, 0xF0,0}, // (PASSWARD1)
    {WR_DATA, 0, 0x5A},
    {WR_DATA, 0, 0x5A},	

    {WR_CMND, 0xF1,0}, // (PASSWARD2)
    {WR_DATA, 0, 0x5A},
    {WR_DATA, 0, 0x5A},

    {SLEEP_MS, 0, 50}, // 50ms

    {WR_CMND, 0xF2,0}, // (DISCTL)
    {WR_DATA, 0, 0x3B},
    {WR_DATA, 0, 0x35}, 
    {WR_DATA, 0, 0x03},
    {WR_DATA, 0, 0x04},
    {WR_DATA, 0, 0x02},

    {WR_DATA, 0, 0x08},
    {WR_DATA, 0, 0x08},
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x08},
    {WR_DATA, 0, 0x08}, 

    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x54}, 

    {WR_DATA, 0, 0x08},
    {WR_DATA, 0, 0x08},
    {WR_DATA, 0, 0x08},
    {WR_DATA, 0, 0x08}, 

    //Power Setting Sequence
    {WR_CMND, 0xF4,0}, // (PWRCTL)
    {WR_DATA, 0, 0x0A},
    {WR_DATA, 0, 0x00},	
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},	
    {WR_DATA, 0, 0x00},

    {WR_DATA, 0, 0x00},	
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},	
    {WR_DATA, 0, 0x15},
    {WR_DATA, 0, 0x6B},

    {WR_DATA, 0, 0x03},
    {WR_DATA, 0, 0x04},
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x02},

    {WR_CMND, 0xF3,0}, // (MANPWRSEQ)
    {WR_DATA, 0, 0x03},
    {WR_DATA, 0, 0x00},	
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},	
    {WR_DATA, 0, 0x00},	

    {WR_CMND, 0xF5,0}, // (VCMCTL)
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x4E},	
    {WR_DATA, 0, 0x61},
    {WR_DATA, 0, 0x00},	
    {WR_DATA, 0, 0x00},

    {WR_DATA, 0, 0x02},	
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},	 
    {WR_DATA, 0, 0x04},
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},	
    {WR_DATA, 0, 0x00},

    {WR_CMND, 0xF6,0}, // (SRCCTL)
    {WR_DATA, 0, 0x04},
    {WR_DATA, 0, 0x00},	
    {WR_DATA, 0, 0x06},
    {WR_DATA, 0, 0x03},	
    {WR_DATA, 0, 0x01},

    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x01},	
    {WR_DATA, 0, 0x00},

    //Initializing Sequence
    {WR_CMND, 0xF7,0}, // (IFCTL)
    {WR_DATA, 0, 0x48},
    {WR_DATA, 0, 0x80},
    {WR_DATA, 0, 0x10},
    {WR_DATA, 0, 0x02},
    {WR_DATA, 0, 0x00},

    {WR_CMND, 0xF8,0}, // (PANELCTL)
    {WR_DATA, 0, 0x11},
    {WR_DATA, 0, 0x00}, 


    //Gamma Setting
    {WR_CMND, 0xF9,0}, // (GAMMASEL)
    {WR_DATA, 0, 0x24},

    {WR_CMND, 0xFA,0}, // (PGAMMACTL)
    {WR_DATA, 0, 0x01},
    {WR_DATA, 0, 0x02},
    {WR_DATA, 0, 0x03},
    {WR_DATA, 0, 0x22},
    {WR_DATA, 0, 0x20},

    {WR_DATA, 0, 0x21},
    {WR_DATA, 0, 0x21},
    {WR_DATA, 0, 0x27},
    {WR_DATA, 0, 0x37},
    {WR_DATA, 0, 0x3D},

    {WR_DATA, 0, 0x3E},
    {WR_DATA, 0, 0x3A},
    {WR_DATA, 0, 0x37},
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},

    {WR_DATA, 0, 0x00},

    {WR_CMND, 0xF9,0}, // (GAMMASEL)
    {WR_DATA, 0, 0x22},

    {WR_CMND, 0xFA,0}, // (PGAMMACTL)
    {WR_DATA, 0, 0x2F},
    {WR_DATA, 0, 0x01},
    {WR_DATA, 0, 0x01},
    {WR_DATA, 0, 0x20},
    {WR_DATA, 0, 0x1E},
    {WR_DATA, 0, 0x1F},
    {WR_DATA, 0, 0x20},
    {WR_DATA, 0, 0x24},
    {WR_DATA, 0, 0x33},
    {WR_DATA, 0, 0x34},

    {WR_DATA, 0, 0x2E},
    {WR_DATA, 0, 0x34},
    {WR_DATA, 0, 0x2A},
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},

    {WR_DATA, 0, 0x00}, 

    {WR_CMND, 0xF9,0}, // (GAMMASEL)
    {WR_DATA, 0, 0x21},

    {WR_CMND, 0xFA,0}, // (PGAMMACTL)
    {WR_DATA, 0, 0x10},
    {WR_DATA, 0, 0x0F},
    {WR_DATA, 0, 0x01},
    {WR_DATA, 0, 0x20},
    {WR_DATA, 0, 0x2B},

    {WR_DATA, 0, 0x2A},
    {WR_DATA, 0, 0x24},
    {WR_DATA, 0, 0x22},
    {WR_DATA, 0, 0x31},
    {WR_DATA, 0, 0x38},

    {WR_DATA, 0, 0x34},
    {WR_DATA, 0, 0x3D},
    {WR_DATA, 0, 0x35},
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x00},

    {WR_DATA, 0, 0x00},


    //Initializing Sequence
    {WR_CMND, 0x3A,0}, // (COLMOD)
    {WR_DATA, 0, 0x77}, //66	/* 0x77 = 24bits, 0x66 = 18bits, 0x55 = 16bits */

    {WR_CMND, 0x35,0}, // (TEON)
    {WR_DATA, 0, 0x00},

 // this routine must be excuted.
    {WR_CMND, 0x36,0}, // (MADCTL)
    {WR_DATA, 0, 0xD0}, 

    {WR_CMND, 0x2A,0}, // (COLUMNADDRESSSET)
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x01},
    {WR_DATA, 0, 0x3F},

    {WR_CMND, 0x2B,0}, // (PAGEADDRESSSET)
    {WR_DATA, 0, 0x00},
    {WR_DATA, 0, 0x01},
    {WR_DATA, 0, 0xDF},

    {WR_CMND, 0x2C,0}, // ( RAM WRITE )
    {WR_CMND, 0x11,0}, // (SLPOUT)

    {SLEEP_MS, 0, 120}, // 120ms

    {WR_CMND, 0x29,0}, // (DISPON) 

    {SLEEP_MS, 0, 50}, // 50ms	// just spec 	

    {CTRL_END, 0, 0}
};


// VER A8
Lcd_init_t power_on_seq_s5d05a1x31_cooperve_SHARP[] =
{
	{WR_CMND, 0xF0, 0},
	{WR_DATA, 0, 0x5A},
	{WR_DATA, 0, 0x5A},
	
	{WR_CMND, 0xF2, 0},
	{WR_DATA, 0, 0x3B},
	{WR_DATA, 0, 0x33},
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x0C},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x33},
	{WR_DATA, 0, 0x0C},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x0C},
	{WR_DATA, 0, 0x08},
	
	{WR_CMND, 0xF3, 0},
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xF4, 0},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x04},
	{WR_DATA, 0, 0x70},
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x04},
	{WR_DATA, 0, 0x70},
	{WR_DATA, 0, 0x03},
	
	{WR_CMND, 0xF5, 0},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x68},
	{WR_DATA, 0, 0x70},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x02},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x68},
	{WR_DATA, 0, 0x70},
	
	{WR_CMND, 0xF6, 0},
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x08},
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x03},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xF7, 0},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x80},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x02},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xF8, 0},
	{WR_DATA, 0, 0x11},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xF9, 0},
	{WR_DATA, 0, 0x14},
	
	{WR_CMND, 0xFA, 0},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x25},
	{WR_DATA, 0, 0x29},
	{WR_DATA, 0, 0x33},
	{WR_DATA, 0, 0x37},
	{WR_DATA, 0, 0x0E},
	{WR_DATA, 0, 0x1C},
	{WR_DATA, 0, 0x22},
	{WR_DATA, 0, 0x28},
	{WR_DATA, 0, 0x1A},
	{WR_DATA, 0, 0x02},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xFB, 0},
	{WR_DATA, 0, 0x0E},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x02},
	{WR_DATA, 0, 0x1A},
	{WR_DATA, 0, 0x28},
	{WR_DATA, 0, 0x22},
	{WR_DATA, 0, 0x1C},
	{WR_DATA, 0, 0x0E},
	{WR_DATA, 0, 0x37},
	{WR_DATA, 0, 0x33},
	{WR_DATA, 0, 0x29},
	{WR_DATA, 0, 0x25},
	{WR_DATA, 0, 0x0A},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xF9, 0},
	{WR_DATA, 0, 0x12},
	
	{WR_CMND, 0xFA, 0},
	{WR_DATA, 0, 0x11},
	{WR_DATA, 0, 0x19},
	{WR_DATA, 0, 0x09},
	{WR_DATA, 0, 0x2A},
	{WR_DATA, 0, 0x2F},
	{WR_DATA, 0, 0x36},
	{WR_DATA, 0, 0x39},
	{WR_DATA, 0, 0x0C},
	{WR_DATA, 0, 0x1A},
	{WR_DATA, 0, 0x1E},
	{WR_DATA, 0, 0x25},
	{WR_DATA, 0, 0x1E},
	{WR_DATA, 0, 0x02},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xFB, 0},
	{WR_DATA, 0, 0x17},
	{WR_DATA, 0, 0x11},
	{WR_DATA, 0, 0x02},
	{WR_DATA, 0, 0x1E},
	{WR_DATA, 0, 0x25},
	{WR_DATA, 0, 0x1E},
	{WR_DATA, 0, 0x1A},
	{WR_DATA, 0, 0x0C},
	{WR_DATA, 0, 0x39},
	{WR_DATA, 0, 0x36},
	{WR_DATA, 0, 0x2F},
	{WR_DATA, 0, 0x2A},
	{WR_DATA, 0, 0x09},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xF9, 0},
	{WR_DATA, 0, 0x11},
	
	{WR_CMND, 0xFA, 0},
	{WR_DATA, 0, 0x36},
	{WR_DATA, 0, 0x16},
	{WR_DATA, 0, 0x05},
	{WR_DATA, 0, 0x0E},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x13},
	{WR_DATA, 0, 0x32},
	{WR_DATA, 0, 0x3C},
	{WR_DATA, 0, 0x3B},
	{WR_DATA, 0, 0x35},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x02},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0xFB, 0},
	{WR_DATA, 0, 0x14},
	{WR_DATA, 0, 0x36},
	{WR_DATA, 0, 0x02},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x35},
	{WR_DATA, 0, 0x3B},
	{WR_DATA, 0, 0x3C},
	{WR_DATA, 0, 0x32},
	{WR_DATA, 0, 0x13},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x10},
	{WR_DATA, 0, 0x0E},
	{WR_DATA, 0, 0x05},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	
	{WR_CMND, 0x44, 0},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x01},
	
	{WR_CMND, 0x35, 0},
	{WR_DATA, 0, 0x01},
	
	{WR_CMND, 0x36, 0},
	{WR_DATA, 0, 0x88}, //0x48 -> 0xD8 -> 0x88
	
	{WR_CMND, 0x2A, 0},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x01},
	{WR_DATA, 0, 0x3F},
	
	{WR_CMND, 0x2B, 0},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x00},
	{WR_DATA, 0, 0x01},
	{WR_DATA, 0, 0xDF},
	
	{WR_CMND, 0x3A, 0},
	{WR_DATA, 0, 0x77}, // 0x55(16 bit/pixel) -> 0x77 (24 bit/pixel)
	
	{WR_CMND, 0x11, 0},
	
	{SLEEP_MS, 0, 120}, // 120ms
	
	{WR_CMND, 0x29, 0},
	
	{CTRL_END, 0, 0}
};


Lcd_init_t power_on_seq_s6d04k1_sdi[] =
{
	{CTRL_END, 0, 0}
};

Lcd_init_t power_off_seq_AUO[] =
{
	{WR_CMND, 0x10,0}, // (SLPIN)
	{SLEEP_MS, 0, 120},
	{CTRL_END, 0, 0}
};

Lcd_init_t power_off_seq_DTC[] =
{
	{WR_CMND, 0x28,0}, // (DISPOFF)
	{WR_CMND, 0x10,0}, // (SLPIN)
	{SLEEP_MS, 0, 120},
	{CTRL_END, 0, 0}
};

Lcd_init_t power_off_seq_SHARP[] =
{
	{WR_CMND, 0x28,0}, // (DISPOFF)
	{WR_CMND, 0x10,0}, // (SLPIN)
	{SLEEP_MS, 0, 120},
	{CTRL_END, 0, 0}
};

#endif /* __BCM_LCD_S6D05A1X31_COOPERVE_H */

