/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/video/broadcom/displays/lcd_HX8347_TFT1P5158.c
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
*lcd_HX8347_TFT1P5158.c
*
*PURPOSE:
*This is the LCD-specific code for a Truely TFT1P5158 with HX8347-D
*controller.
*
*****************************************************************************/

/*---- Include Files ----------------------------------------------------*/
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
/*#include <linux/broadcom/lcd_backlight.h>*/

#include <cfg_global.h>

/*#define DEBUG*/
#include <linux/broadcom/lcd_common.h>
#include <plat/timer.h>

/*Attributes of the LCD*/

#define LCD_HEIGHT 320
#define LCD_WIDTH 240
#define LCD_BITS_PER_PIXEL 16

#define ROW_OFFSET  0
#define COL_OFFSET  0

#define WRITE_LCD_INDEX(x)    WRITE_LCD_CMD(x)
#define WRITE_LCD_CMD_DATA(x) WRITE_LCD_PARAM(x)
/*#define WRITE_LCD_PARAM(x)     WRITE_LCD_PARAM(x). This is defined in lcd_common.h*/

/* globals for lcd_common.c to use*/
const char *LCD_panel_name = "Truly QVGA HX8347_TFT1P5158-E";

LCD_Intf_t LCD_Intf = LCD_Z80;
LCD_Volt_t LCD_Volt = LCD_30V;
LCD_Bus_t LCD_Bus = LCD_18BIT;

int LCD_num_panels = 1;

LCD_dev_info_t LCD_device[1] = {
	{
	 LCD_main_panel,
	 LCD_HEIGHT,
	 LCD_WIDTH,
	 LCD_BITS_PER_PIXEL,
	 ROW_OFFSET,
	 COL_OFFSET}
};

/*---- Private Constants and Types --------------------------------------*/

#define LCD_OUT(data) (((u32)(data)&0x000000ff))

/* LCD init sequence*/
typedef enum {
	WRITE_INDEX,
	WRITE_DATA,
	WRITE_PAUSE,
	WRITE_FULL_SCREEN,
	READ_INDEX,
	READ_DATA,
	STOP
} Init_Cmd_t;

typedef struct {
	Init_Cmd_t type;
	u32 data;
} Init_Seq_t;

/* ---- Private Variables ------------------------------------------------ */
static Init_Seq_t Init_Seq[] = {
/*{WRITE_INDEX,  LCD_OUT(0x00FF)},   {WRITE_DATA,    LCD_OUT(0x0000)}, Page 0 select.  ???*/

	{WRITE_INDEX, LCD_OUT(0x00EA)}, {WRITE_DATA, LCD_OUT(0x0000)},	/*PTBA[15:8]  ==>??? PTBA is up-side down to STBA */
	{WRITE_INDEX, LCD_OUT(0x00EB)}, {WRITE_DATA, LCD_OUT(0x0020)},	/*PTBA[7:0] */
	{WRITE_INDEX, LCD_OUT(0x00EC)}, {WRITE_DATA, LCD_OUT(0x000C)},	/*STBA[15:8] */
	{WRITE_INDEX, LCD_OUT(0x00ED)}, {WRITE_DATA, LCD_OUT(0x00C4)},	/*STBA[7:0] */
	{WRITE_INDEX, LCD_OUT(0x00E8)}, {WRITE_DATA, LCD_OUT(0x0040)},	/*OPON[7:0] */
	{WRITE_INDEX, LCD_OUT(0x00E9)}, {WRITE_DATA, LCD_OUT(0x0038)},	/*OPON1[7:0] */
	{WRITE_INDEX, LCD_OUT(0x00F1)}, {WRITE_DATA, LCD_OUT(0x0001)},	/*OTPS1B   ==>??? Don't see it in datasheet */
	{WRITE_INDEX, LCD_OUT(0x00F2)}, {WRITE_DATA, LCD_OUT(0x0010)},	/*GEN==>??? Don't see it in datasheet */
	{WRITE_INDEX, LCD_OUT(0x0027)}, {WRITE_DATA, LCD_OUT(0x00A3)},

/*Gamma 2.2 Setting*/

	{WRITE_INDEX, LCD_OUT(0x0040)}, {WRITE_DATA, LCD_OUT(0x0000)},	/*gamma setting  */
	{WRITE_INDEX, LCD_OUT(0x0041)}, {WRITE_DATA, LCD_OUT(0x0000)},
	{WRITE_INDEX, LCD_OUT(0x0042)}, {WRITE_DATA, LCD_OUT(0x0001)},
	{WRITE_INDEX, LCD_OUT(0x0043)}, {WRITE_DATA, LCD_OUT(0x0013)},
	{WRITE_INDEX, LCD_OUT(0x0044)}, {WRITE_DATA, LCD_OUT(0x0010)},
	{WRITE_INDEX, LCD_OUT(0x0045)}, {WRITE_DATA, LCD_OUT(0x0026)},
	{WRITE_INDEX, LCD_OUT(0x0046)}, {WRITE_DATA, LCD_OUT(0x0008)},
	{WRITE_INDEX, LCD_OUT(0x0047)}, {WRITE_DATA, LCD_OUT(0x0051)},
	{WRITE_INDEX, LCD_OUT(0x0048)}, {WRITE_DATA, LCD_OUT(0x0002)},
	{WRITE_INDEX, LCD_OUT(0x0049)}, {WRITE_DATA, LCD_OUT(0x0012)},
	{WRITE_INDEX, LCD_OUT(0x004A)}, {WRITE_DATA, LCD_OUT(0x0018)},
	{WRITE_INDEX, LCD_OUT(0x004B)}, {WRITE_DATA, LCD_OUT(0x0019)},
	{WRITE_INDEX, LCD_OUT(0x004C)}, {WRITE_DATA, LCD_OUT(0x0014)},
	{WRITE_INDEX, LCD_OUT(0x0050)}, {WRITE_DATA, LCD_OUT(0x0019)},
	{WRITE_INDEX, LCD_OUT(0x0051)}, {WRITE_DATA, LCD_OUT(0x002F)},
	{WRITE_INDEX, LCD_OUT(0x0052)}, {WRITE_DATA, LCD_OUT(0x002C)},
	{WRITE_INDEX, LCD_OUT(0x0053)}, {WRITE_DATA, LCD_OUT(0x003E)},
	{WRITE_INDEX, LCD_OUT(0x0054)}, {WRITE_DATA, LCD_OUT(0x003F)},
	{WRITE_INDEX, LCD_OUT(0x0055)}, {WRITE_DATA, LCD_OUT(0x003F)},
	{WRITE_INDEX, LCD_OUT(0x0056)}, {WRITE_DATA, LCD_OUT(0x002E)},
	{WRITE_INDEX, LCD_OUT(0x0057)}, {WRITE_DATA, LCD_OUT(0x0077)},
	{WRITE_INDEX, LCD_OUT(0x0058)}, {WRITE_DATA, LCD_OUT(0x000B)},
	{WRITE_INDEX, LCD_OUT(0x0059)}, {WRITE_DATA, LCD_OUT(0x0006)},
	{WRITE_INDEX, LCD_OUT(0x005A)}, {WRITE_DATA, LCD_OUT(0x0007)},
	{WRITE_INDEX, LCD_OUT(0x005B)}, {WRITE_DATA, LCD_OUT(0x000D)},
	{WRITE_INDEX, LCD_OUT(0x005C)}, {WRITE_DATA, LCD_OUT(0x001D)},
	{WRITE_INDEX, LCD_OUT(0x005D)}, {WRITE_DATA, LCD_OUT(0x00CC)},

/*Power Voltage Setting*/
	{WRITE_INDEX, LCD_OUT(0x001B)}, {WRITE_DATA, LCD_OUT(0x001B)},	/*VRH=4.65V */
	{WRITE_INDEX, LCD_OUT(0x001A)}, {WRITE_DATA, LCD_OUT(0x0001)},	/*BT (VGH~15V,VGL~-10V,DDVDH~5V) */
	{WRITE_INDEX, LCD_OUT(0x001C)}, {WRITE_DATA, LCD_OUT(0x0004)},	/*AP2-0 */
	{WRITE_INDEX, LCD_OUT(0x0024)}, {WRITE_DATA, LCD_OUT(0x002F)},	/*VMH(VCOM High voltage ~3.2V) */
	{WRITE_INDEX, LCD_OUT(0x0025)}, {WRITE_DATA, LCD_OUT(0x0059)},	/*VML(VCOM Low voltage -1.2V) */

	/****VCOM offset**/
	{WRITE_INDEX, LCD_OUT(0x0023)}, {WRITE_DATA, LCD_OUT(0x008E)},	/*for Flicker adjust can reload from OTP */

	/*Power on Setting */

	{WRITE_INDEX, LCD_OUT(0x0018)}, {WRITE_DATA, LCD_OUT(0x0037)},	/*I/P_RADJ,N/P_RADJ, Normal mode 80Hz */
	{WRITE_INDEX, LCD_OUT(0x0060)}, {WRITE_DATA, LCD_OUT(0x0008)},	/*turn on framesignal   ==>??? Don't see detail register in datasheet */

	{WRITE_INDEX, LCD_OUT(0x0019)}, {WRITE_DATA, LCD_OUT(0x0001)},	/*OSC_EN='1', start Osc */
	{WRITE_INDEX, LCD_OUT(0x0001)}, {WRITE_DATA, LCD_OUT(0x0000)},	/*DP_STB='0', out deep sleep */
	{WRITE_INDEX, LCD_OUT(0x001F)}, {WRITE_DATA, LCD_OUT(0x0088)},	/* GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0 */
	{WRITE_PAUSE, 10},	/*delay 10ms */

	{WRITE_INDEX, LCD_OUT(0x001F)}, {WRITE_DATA, LCD_OUT(0x0080)},	/* GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0 */
	{WRITE_PAUSE, 10},	/*delay 10ms */
	{WRITE_INDEX, LCD_OUT(0x001F)}, {WRITE_DATA, LCD_OUT(0x0090)},	/* GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0 */
	{WRITE_PAUSE, 10},	/*delay 10ms */
	{WRITE_INDEX, LCD_OUT(0x001F)}, {WRITE_DATA, LCD_OUT(0x00D0)},	/*GAS=1, VOMG=10, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0 */
	{WRITE_PAUSE, 10},	/*delay 10ms */
	{WRITE_INDEX, LCD_OUT(0x002F)}, {WRITE_DATA, LCD_OUT(0x0001)},	/*1-line Inversion */

	{WRITE_INDEX, LCD_OUT(0x0016)}, {WRITE_DATA, LCD_OUT(0x0008)},	/*MY,MX,MV,ML,BGR SETING */
	{WRITE_INDEX, LCD_OUT(0x0017)}, {WRITE_DATA, LCD_OUT(0x0006)},	/*default 0x0006 262k color   0x0005 65k color */

	/*SET PANEL */
	{WRITE_INDEX, LCD_OUT(0x0036)}, {WRITE_DATA, LCD_OUT(0x0000)},	/*SS_P, GS_P,REV_P,BGR_P */

	/*Display ON Setting */
	{WRITE_INDEX, LCD_OUT(0x0028)}, {WRITE_DATA, LCD_OUT(0x0038)},	/*GON=1, DTE=1, D=1000 */
	{WRITE_PAUSE, 50},	/*delay 50ms */
	{WRITE_INDEX, LCD_OUT(0x0028)}, {WRITE_DATA, LCD_OUT(0x003C)},	/*GON=1, DTE=1, D=1100 */

	/*Set GRAM Area */
	{WRITE_INDEX, LCD_OUT(0x0002)}, {WRITE_DATA, LCD_OUT(0x0000)},
	{WRITE_INDEX, LCD_OUT(0x0003)}, {WRITE_DATA, LCD_OUT(0x0000)},	/*Column Start */
	{WRITE_INDEX, LCD_OUT(0x0004)}, {WRITE_DATA, LCD_OUT(0x0000)},
	{WRITE_INDEX, LCD_OUT(0x0005)}, {WRITE_DATA, LCD_OUT(0x00EF)},	/*Column End */
	{WRITE_INDEX, LCD_OUT(0x0006)}, {WRITE_DATA, LCD_OUT(0x0000)},
	{WRITE_INDEX, LCD_OUT(0x0007)}, {WRITE_DATA, LCD_OUT(0x0000)},	/*Row Start */
	{WRITE_INDEX, LCD_OUT(0x0008)}, {WRITE_DATA, LCD_OUT(0x0001)},
	{WRITE_INDEX, LCD_OUT(0x0009)}, {WRITE_DATA, LCD_OUT(0x003F)},	/*Row End */
	{WRITE_INDEX, LCD_OUT(0x0022)},	/*Start GRAM write  ==>Do we need it??? */

	{STOP, LCD_OUT(0x00)}	/* End of sequence */
};

static Init_Seq_t PowerOff_Seq[] = {

	{WRITE_INDEX, LCD_OUT(0x0028)}, {WRITE_DATA, LCD_OUT(0x0038)},	/* GON=1, DTE=1, D1-0=10 */
	{WRITE_PAUSE, 40},
	{WRITE_INDEX, LCD_OUT(0x0028)}, {WRITE_DATA, LCD_OUT(0x0028)},	/* GON=1, DTE=0, D1-0=10 */
	{WRITE_PAUSE, 40},
	{WRITE_INDEX, LCD_OUT(0x0028)}, {WRITE_DATA, LCD_OUT(0x0020)},	/* GON=1, DTE=0, D1-0=00 */
	{WRITE_PAUSE, 20},

	/* Power supply halt    */
	{WRITE_INDEX, LCD_OUT(0x001F)}, {WRITE_DATA, LCD_OUT(0x0092)},	/* VCOMG=0 */
	{WRITE_PAUSE, 10},
	{WRITE_INDEX, LCD_OUT(0x001F)}, {WRITE_DATA, LCD_OUT(0x008A)},	/* PON=0, DK=1 */
	{WRITE_INDEX, LCD_OUT(0x001C)}, {WRITE_DATA, LCD_OUT(0x0000)},	/* AP=000 */
	{WRITE_PAUSE, 20},

	/* Go to Sleep */
	{WRITE_INDEX, LCD_OUT(0x001F)}, {WRITE_DATA, LCD_OUT(0x008B)},	/* STB=1, Enter standby */
	{WRITE_INDEX, LCD_OUT(0x0019)}, {WRITE_DATA, LCD_OUT(0x0000)},	/* OSC_EN=0, Stop oscillation */
	{WRITE_PAUSE, 5},
	{STOP, LCD_OUT(0x00)}	/* End of sequence */
};

#if 0

static Init_Seq_t SleepIn_Seq[] = {
	{WRITE_INDEX, LCD_OUT(0x0028)}, {WRITE_DATA, LCD_OUT(0x00b8)},
	{WRITE_PAUSE, 40},
	{WRITE_INDEX, LCD_OUT(0x001f)}, {WRITE_DATA, LCD_OUT(0x0089)},
	{WRITE_PAUSE, 40},
	{WRITE_INDEX, LCD_OUT(0x0028)}, {WRITE_DATA, LCD_OUT(0x0004)},
	{WRITE_PAUSE, 40},
	{WRITE_INDEX, LCD_OUT(0x0021)}, {WRITE_DATA, LCD_OUT(0x0000)},
	{WRITE_PAUSE, 5},
	{STOP, LCD_OUT(0x00)}	/* End of sequence */
};

static Init_Seq_t SleepOut_Seq[] = {

	{WRITE_INDEX, LCD_OUT(0x0018)}, {WRITE_DATA, LCD_OUT(0x0036)},
	{WRITE_INDEX, LCD_OUT(0x0019)}, {WRITE_DATA, LCD_OUT(0x0001)},
	{WRITE_INDEX, LCD_OUT(0x001f)}, {WRITE_DATA, LCD_OUT(0x0088)},

	{WRITE_PAUSE, 5},
	{WRITE_INDEX, LCD_OUT(0x001f)}, {WRITE_DATA, LCD_OUT(0x0080)},
	{WRITE_PAUSE, 5},
	{WRITE_INDEX, LCD_OUT(0x001f)}, {WRITE_DATA, LCD_OUT(0x0090)},
	{WRITE_PAUSE, 5},
	{WRITE_INDEX, LCD_OUT(0x001f)}, {WRITE_DATA, LCD_OUT(0x00d0)},
	{WRITE_PAUSE, 5},
	{WRITE_INDEX, LCD_OUT(0x0028)}, {WRITE_DATA, LCD_OUT(0x0038)},
	{WRITE_PAUSE, 40},
	{WRITE_INDEX, LCD_OUT(0x0028)}, {WRITE_DATA, LCD_OUT(0x003f)},

	{STOP, LCD_OUT(0x00)}	/* End of sequence */
};

#endif

void lcd_ResetStartAddr(LCD_dev_info_t * dev)
{
	/*set Column Start/End address */
	WRITE_LCD_INDEX(LCD_OUT(0x02));
	WRITE_LCD_PARAM(LCD_OUT((dev->col_start >> 8) & 0x00FF));	/*Display start column (ms addr) */
	WRITE_LCD_INDEX(LCD_OUT(0x03));
	WRITE_LCD_PARAM(LCD_OUT(dev->col_start & 0x00FF));	/*(ls addr) */
	WRITE_LCD_INDEX(LCD_OUT(0x04));
	WRITE_LCD_PARAM(LCD_OUT((dev->col_end >> 8) & 0x00FF));	/* Display end column  (ms addr) */
	WRITE_LCD_INDEX(LCD_OUT(0x05));
	WRITE_LCD_PARAM(LCD_OUT(dev->col_end & 0x00FF));	/*(ls addr) */

	/*set Row Start/End address */
	WRITE_LCD_INDEX(LCD_OUT(0x06));
	WRITE_LCD_PARAM(LCD_OUT((dev->row_start >> 8) & 0x00FF));	/*Display start row   (ms addr) */
	WRITE_LCD_INDEX(LCD_OUT(0x07));
	WRITE_LCD_PARAM(LCD_OUT(dev->row_start & 0x00FF));	/*(ls addr) */
	WRITE_LCD_INDEX(LCD_OUT(0x08));
	WRITE_LCD_PARAM(LCD_OUT((dev->row_end >> 8) & 0x00FF));	/*Display end row    (ms addr) */
	WRITE_LCD_INDEX(LCD_OUT(0x09));
	WRITE_LCD_PARAM(LCD_OUT(dev->row_end & 0x00FF));	/*(ls addr) */

	/* Should we write the address column and row address counters as well? */
	/* 0x80, 0x81 and 0x82, 0x83 */
	/* Reset column counter */
	WRITE_LCD_INDEX(LCD_OUT(0x80));
	WRITE_LCD_PARAM(LCD_OUT((dev->col_start >> 8) & 0x00FF));	/* Display end row    (ms addr) */
	WRITE_LCD_INDEX(LCD_OUT(0x81));
	WRITE_LCD_PARAM(LCD_OUT(dev->col_start & 0x00FF));	/*(ls addr) */

	WRITE_LCD_INDEX(LCD_OUT(0x82));
	WRITE_LCD_PARAM(LCD_OUT((dev->row_start >> 8) & 0x00FF));	/* Display end row    (ms addr) */
	WRITE_LCD_INDEX(LCD_OUT(0x83));
	WRITE_LCD_PARAM(LCD_OUT(dev->row_start & 0x00FF));	/*(ls addr) */
}

void lcd_setup_for_data(LCD_dev_info_t * dev)
{
	/*lcd_enable_ce(false); */

	lcd_ResetStartAddr(dev);

	/*start writing again */
	WRITE_LCD_INDEX(LCD_OUT(0x0022));

}

static int lcd_send_cmd_sequence(Init_Seq_t * init)
{
	int i;

	LCD_PUTS("enter");
	/*lcd_enable_ce(false); */

	for (i = 0; init[i].type != STOP; i++) {
		if (init[i].type == WRITE_INDEX) {
			LCD_DEBUG("Index:0x%04x(0x%04x), Data:0x%04x(0x%04x)\n",
				  init[i].data >> 1, init[i].data,
				  init[i + 1].data >> 1, init[i + 1].data);
			WRITE_LCD_INDEX(init[i].data);
		} else if (init[i].type == WRITE_DATA) {
			WRITE_LCD_PARAM(init[i].data);
		} else if (init[i].type == WRITE_FULL_SCREEN) {
			LCD_PUTS("Just dummy write full screen now");

		} else if (init[i].type == WRITE_PAUSE) {
			LCD_DEBUG("Delay:%dms\n", init[i].data);
			msleep(init[i].data);
		}
	}

	LCD_PUTS("done");
	return 0;

}				/* lcd_send_cmd_sequence */

void lcd_init_panels(void)
{
	LCD_PUTS("enter");

	lcd_send_cmd_sequence(Init_Seq);

	LCD_PUTS("done.");

}				/* lcd_init_controller */

void lcd_poweroff_panels(void)
{
	LCD_PUTS("enter");

	lcd_send_cmd_sequence(PowerOff_Seq);

	LCD_PUTS("done");
}
