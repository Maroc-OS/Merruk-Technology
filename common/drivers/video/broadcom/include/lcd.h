/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/broadcom/include/lcd.h
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

#ifndef __BCM_LCD_H
#define __BCM_LCD_H

#define DEBUG

#ifdef DEBUG
#define LCD_DEBUG(fmt, args...) printk("[%s]: " fmt, __FUNCTION__ , ##args)
#else
#define LCD_DEBUG(fmt, args...)
#endif

#define LCD_PUTS(str)	LCD_DEBUG("%s\n", str)

#define USE_DMA

typedef enum {
	LCD_main_panel = 0,	/* must match device minor number */
	LCD_sub_panel		/* must match device minor number */
} LCD_panel_t;

/* from lcd_drv.h */

/** LCD Color
*/
typedef enum {
	LCD_COLOR_BLACK = 0x0000,	/* /< lcd black pixel */
	LCD_COLOR_GREEN = 0x07E0,	/* /< lcd green pixel */
	LCD_COLOR_RED = 0xF800,	/* /< lcd red pixel */
	LCD_COLOR_BLUE = 0x001F,	/* /< lcd blue pixel */
	LCD_COLOR_YELLOW = 0xFFE0,	/* /< lcd yellow pixel */
	LCD_COLOR_CYAN = 0xF81F,	/* /< lcd cyan pixel */
	LCD_COLOR_DDD = 0x07FF,	/* /< lcd ddd pixel */
	LCD_COLOR_WHITE = 0xFFFF	/* /< lcd white pixel */
} LcdColor_t;

/* LCD interface type, IOCR0[29:28] */
typedef enum {
	LCD_Z80,
	LCD_M68,
	LCD_SPI,
	LCD_ETM,
	LCD_MSPRO,
	LCD_SD2,
	LCD_DSI,
} LCD_Intf_t;

/* LCD bus width */
typedef enum {
	LCD_16BIT = 16,
	LCD_18BIT = 18,
	LCD_24BIT = 24,
	LCD_32BIT = 32,
} LCD_Bus_t;

struct timing_config_t {
	uint32_t rd_hold;
	uint32_t rd_pulse;
	uint32_t rd_setup;
	uint32_t wr_hold;
	uint32_t wr_pulse;
	uint32_t wr_setup;
};

typedef struct {
	size_t sizeInBytes;
	void *virtPtr;
	dma_addr_t physPtr;
} LCD_FrameBuffer_t;


/* info for each LCD panel - some global vars moved into here */
typedef struct lcd_dev_info_t {
	LCD_panel_t panel;	/* hint  - when panels require different cmds */

	int height;
	int width;
	int bits_per_pixel;
	bool te_supported;
	uint32_t row_start;
	uint32_t row_end;
	uint32_t col_start;
	uint32_t col_end;

	LCD_FrameBuffer_t frame_buffer;
	LCD_DirtyRect_t dirty_rect;

	int physical_height;	/* millimeter */
	int physical_width;	/* millimeter */
} LCD_dev_info_t;

extern LCD_Bus_t LCD_Bus;

typedef enum
{
	WR_CMND,
	WR_DATA,
	WR_CMND_DATA,
	WR_CMND_MULTIPLE_DATA,
	SLEEP_MS,
	CTRL_END,
}ctrl_t;

typedef struct lcd_init_t
{
    ctrl_t type;
    uint16_t cmd;
    uint16_t data;
    uint32_t datasize;   //< data size for multiple paramerter
    uint8_t* dataptr;
} Lcd_init_t;


typedef struct lcd_interface_drv_t
{
	void (*lcd_get_interface_info)(uint32_t* phy_addr);
	void (*lcd_init_panels) (void);
	void (*lcd_poweroff_panels)(void);
	void (*lcd_display_black_background)(void);
	void (*lcd_update_column)(LCD_dev_info_t * dev, unsigned int column);

	int (*lcd_update_rect_dma)(LCD_dev_info_t * dev, void*  req);
	int (*lcd_probe)(struct platform_device *pdev);
	void (*lcd_send_data) (uint16_t * p, int img_width, int img_height, bool rle);
	void (*lcd_lock_csl_handle)(void);
	void (*lcd_unlock_csl_handle)(void);
}LCD_Interface_Drv_t;

#endif /* __BCM_LCD_H */
