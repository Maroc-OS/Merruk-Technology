/*
 * AT42MXT224/ATMXT224 Touchscreen driver
 *
 * Copyright (C) 2010 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

/* #define DEBUG */
#include <linux/device.h>

#define DEBUG_PEN_STATUS

//by wangpl
#define USE_BRCM_WORK_QUEUE

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/i2c/mxt224_ts.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

//by wangpl
#include <mach/gpio.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

/* Version */
#define MXT224_VER_20			20
#define MXT224_VER_21			21
#define MXT224_VER_22			22

//by wangpl
#define MXT224_VER_33			33
static struct workqueue_struct *mxt224_wq;

/* Slave addresses */
#define MXT224_APP_LOW		0x4a
#define MXT224_APP_HIGH		0x4b
#define MXT224_BOOT_LOW		0x24
#define MXT224_BOOT_HIGH		0x25

/* Firmware */
#define MXT224_FW_NAME		"mxt224.fw"

/* Registers */
#define MXT224_FAMILY_ID		0x00
#define MXT224_VARIANT_ID		0x01
#define MXT224_VERSION		0x02
#define MXT224_BUILD			0x03
#define MXT224_MATRIX_X_SIZE		0x04
#define MXT224_MATRIX_Y_SIZE		0x05
#define MXT224_OBJECT_NUM		0x06
#define MXT224_OBJECT_START		0x07

#define MXT224_OBJECT_SIZE		6

/* Object types */
#define MXT224_DEBUG_DIAGNOSTIC	37
#define MXT224_GEN_MESSAGE		5
#define MXT224_GEN_COMMAND		6
#define MXT224_GEN_POWER		7
#define MXT224_GEN_ACQUIRE		8
#define MXT224_TOUCH_MULTI		9
#define MXT224_TOUCH_KEYARRAY		15
#define MXT224_TOUCH_PROXIMITY	23
#define MXT224_PROCI_GRIPFACE		20
#define MXT224_PROCG_NOISE		22
#define MXT224_PROCI_ONETOUCH		24
#define MXT224_PROCI_TWOTOUCH		27
#define MXT224_SPT_COMMSCONFIG	18	/* firmware ver 21 over */
#define MXT224_SPT_GPIOPWM		19
#define MXT224_SPT_SELFTEST		25
#define MXT224_SPT_CTECONFIG		28
#define MXT224_SPT_USERDATA		38	/* firmware ver 21 over */

/* MXT224_GEN_COMMAND field */
#define MXT224_COMMAND_RESET		0
#define MXT224_COMMAND_BACKUPNV	1
#define MXT224_COMMAND_CALIBRATE	2
#define MXT224_COMMAND_REPORTALL	3
#define MXT224_COMMAND_DIAGNOSTIC	5

/* MXT224_GEN_POWER field */
#define MXT224_POWER_IDLEACQINT	0
#define MXT224_POWER_ACTVACQINT	1
#define MXT224_POWER_ACTV2IDLETO	2

/* MXT224_GEN_ACQUIRE field */
#define MXT224_ACQUIRE_CHRGTIME	0
#define MXT224_ACQUIRE_TCHDRIFT	2
#define MXT224_ACQUIRE_DRIFTST	3
#define MXT224_ACQUIRE_TCHAUTOCAL	4
#define MXT224_ACQUIRE_SYNC		5
#define MXT224_ACQUIRE_ATCHCALST	6
#define MXT224_ACQUIRE_ATCHCALSTHR	7

/* MXT224_TOUCH_MULTI field */
#define MXT224_TOUCH_CTRL		0
#define MXT224_TOUCH_XORIGIN		1
#define MXT224_TOUCH_YORIGIN		2
#define MXT224_TOUCH_XSIZE		3
#define MXT224_TOUCH_YSIZE		4
#define MXT224_TOUCH_BLEN		6
#define MXT224_TOUCH_TCHTHR		7
#define MXT224_TOUCH_TCHDI		8
#define MXT224_TOUCH_ORIENT		9
#define MXT224_TOUCH_MOVHYSTI		11
#define MXT224_TOUCH_MOVHYSTN		12
#define MXT224_TOUCH_NUMTOUCH		14
#define MXT224_TOUCH_MRGHYST		15
#define MXT224_TOUCH_MRGTHR		16
#define MXT224_TOUCH_AMPHYST		17
#define MXT224_TOUCH_XRANGE_LSB	18
#define MXT224_TOUCH_XRANGE_MSB	19
#define MXT224_TOUCH_YRANGE_LSB	20
#define MXT224_TOUCH_YRANGE_MSB	21
#define MXT224_TOUCH_XLOCLIP		22
#define MXT224_TOUCH_XHICLIP		23
#define MXT224_TOUCH_YLOCLIP		24
#define MXT224_TOUCH_YHICLIP		25
#define MXT224_TOUCH_XEDGECTRL	26
#define MXT224_TOUCH_XEDGEDIST	27
#define MXT224_TOUCH_YEDGECTRL	28
#define MXT224_TOUCH_YEDGEDIST	29
#define MXT224_TOUCH_JUMPLIMIT	30	/* firmware ver 22 over */

/* MXT224_PROCI_GRIPFACE field */
#define MXT224_GRIPFACE_CTRL		0
#define MXT224_GRIPFACE_XLOGRIP	1
#define MXT224_GRIPFACE_XHIGRIP	2
#define MXT224_GRIPFACE_YLOGRIP	3
#define MXT224_GRIPFACE_YHIGRIP	4
#define MXT224_GRIPFACE_MAXTCHS	5
#define MXT224_GRIPFACE_SZTHR1	7
#define MXT224_GRIPFACE_SZTHR2	8
#define MXT224_GRIPFACE_SHPTHR1	9
#define MXT224_GRIPFACE_SHPTHR2	10
#define MXT224_GRIPFACE_SUPEXTTO	11

/* MXT224_PROCI_NOISE field */
#define MXT224_NOISE_CTRL		0
#define MXT224_NOISE_OUTFLEN		1
#define MXT224_NOISE_GCAFUL_LSB	3
#define MXT224_NOISE_GCAFUL_MSB	4
#define MXT224_NOISE_GCAFLL_LSB	5
#define MXT224_NOISE_GCAFLL_MSB	6
#define MXT224_NOISE_ACTVGCAFVALID	7
#define MXT224_NOISE_NOISETHR		8
#define MXT224_NOISE_FREQHOPSCALE	10
#define MXT224_NOISE_FREQ0		11
#define MXT224_NOISE_FREQ1		12
#define MXT224_NOISE_FREQ2		13
#define MXT224_NOISE_FREQ3		14
#define MXT224_NOISE_FREQ4		15
#define MXT224_NOISE_IDLEGCAFVALID	16

/* MXT224_SPT_COMMSCONFIG */
#define MXT224_COMMS_CTRL		0
#define MXT224_COMMS_CMD		1

/* MXT224_SPT_CTECONFIG field */
#define MXT224_CTE_CTRL		0
#define MXT224_CTE_CMD		1
#define MXT224_CTE_MODE		2
#define MXT224_CTE_IDLEGCAFDEPTH	3
#define MXT224_CTE_ACTVGCAFDEPTH	4
#define MXT224_CTE_VOLTAGE		5	/* firmware ver 21 over */

// by wangpl
#define MXT224_VOLTAGE_DEFAULT	2700000
#define MXT224_VOLTAGE_STEP		10000

/* Define for MXT224_GEN_COMMAND */
#define MXT224_BOOT_VALUE		0xa5
#define MXT224_BACKUP_VALUE		0x55
#define MXT224_BACKUP_TIME		25	/* msec */
#define MXT224_RESET_TIME		65	/* msec */

#define MXT224_FWRESET_TIME		175	/* msec */

/* Command to unlock bootloader */
#define MXT224_UNLOCK_CMD_MSB		0xaa
#define MXT224_UNLOCK_CMD_LSB		0xdc

/* Bootloader mode status */
#define MXT224_WAITING_BOOTLOAD_CMD	0xc0	/* valid 7 6 bit only */
#define MXT224_WAITING_FRAME_DATA	0x80	/* valid 7 6 bit only */
#define MXT224_FRAME_CRC_CHECK	0x02
#define MXT224_FRAME_CRC_FAIL		0x03
#define MXT224_FRAME_CRC_PASS		0x04
#define MXT224_APP_CRC_FAIL		0x40	/* valid 7 8 bit only */
#define MXT224_BOOT_STATUS_MASK	0x3f

/* Touch status */
#define MXT224_SUPPRESS		(1 << 1)
#define MXT224_AMP			(1 << 2)
#define MXT224_VECTOR			(1 << 3)
#define MXT224_MOVE			(1 << 4)
#define MXT224_RELEASE		(1 << 5)
#define MXT224_PRESS			(1 << 6)
#define MXT224_DETECT			(1 << 7)

/* Touchscreen absolute values */
#define MXT224_MAX_XC			0x3ff
#define MXT224_MAX_YC			0x3ff
#define MXT224_MAX_AREA		0xff

#define MXT224_MAX_FINGER		10

//by wangpl
#define MXT224_INT 	29
#define MXT224_WK 	28

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);



/* Initial register values recommended from chip vendor */
static const u8 init_vals_ver_20[] = {
	/* MXT224_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_GEN_POWER(7) */
	0x20, 0xff, 0x32,
	/* MXT224_GEN_ACQUIRE(8) */
	0x08, 0x05, 0x05, 0x00, 0x00, 0x00, 0x05, 0x14,
	/* MXT224_TOUCH_MULTI(9) */
	0x00, 0x00, 0x00, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x02, 0x00,
	0x00, 0x01, 0x01, 0x0e, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x64,
	/* MXT224_TOUCH_KEYARRAY(15) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* MXT224_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
	/* MXT224_PROCI_GRIPFACE(20) */
	0x00, 0x64, 0x64, 0x64, 0x64, 0x00, 0x00, 0x1e, 0x14, 0x04,
	0x1e, 0x00,
	/* MXT224_PROCG_NOISE(22) */
	0x05, 0x00, 0x00, 0x19, 0x00, 0xe7, 0xff, 0x04, 0x32, 0x00,
	0x01, 0x0a, 0x0f, 0x14, 0x00, 0x00, 0xe8,
	/* MXT224_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	/* MXT224_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	/* MXT224_PROCI_TWOTOUCH(27) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x00, 0x04, 0x08,
};

static const u8 init_vals_ver_21[] = {
	/* MXT224_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_GEN_POWER(7) */
	0x20, 0xff, 0x32,
	/* MXT224_GEN_ACQUIRE(8) */
	0x0a, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x23,
	/* MXT224_TOUCH_MULTI(9) */
	0x00, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00,
	0x00, 0x01, 0x01, 0x0e, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_TOUCH_KEYARRAY(15) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* MXT224_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_PROCI_GRIPFACE(20) */
	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x28, 0x04,
	0x0f, 0x0a,
	/* MXT224_PROCG_NOISE(22) */
	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x23, 0x00,
	0x00, 0x05, 0x0f, 0x19, 0x23, 0x2d, 0x03,
	/* MXT224_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	/* MXT224_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	/* MXT224_PROCI_TWOTOUCH(27) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x00, 0x08, 0x10, 0x00,
};

static const u8 init_vals_ver_22[] = {
	/* MXT224_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* [SPT_USERDATA_T38 INSTANCE 0] */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_GEN_POWER(7) */
	/*
	 * IDLEACQINT (Idle Acquisition Interval) - 60ms
	 * ACTVACQINT (Active Acquisition Interval) - 37ms
	 * ACTV2IDLETO (Active to Idle time out) - 60ms
	 *
	 * Configuring the ACTVACQINT to 50ms in conjunction with
	 * making the controller operate on Mode1 (MXT224_SPT_COMMSCONFIG)
	 * brought down the number of interrupts sent to the Host.
	 * This value seems optimal with respect to sytem load when multi-touch
	 * is enabled.
	 */
	0x3C, 0x1B, 0x3C,
	/* MXT224_GEN_ACQUIRE(8) */
	0x0a, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x23,
	/* MXT224_TOUCH_MULTI(9) */
	/*
	 * Touch Threshold TCHTHR - 30 (typical 30 to 80)
	 * Lower the Threshold value the more sensitive the touch is.
	 * Programming the least value recommended in the Data sheet
	 */
	0x83, 0x00, 0x00, 0x0f, 0x0b, 0x00, 0x21, 0x46, 0x02, 0x07,
	0x00, 0x01, 0x01, 0x00, 0x0a, 0x0a, 0x0a, 0x0a, 0xFF, 0x03,
	0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* MXT224_TOUCH_KEYARRAY(15) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* MXT224_SPT_COMMSCONFIG(18) */
	0x01, 0x00,
	/* MXT224_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_PROCI_GRIPFACE(20) */
	0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x28, 0x04,
	0x0f, 0x0a,
	/* MXT224_PROCG_NOISE(22) */
	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x2d, 0x00,
	0x00, 0x05, 0x0f, 0x19, 0x23, 0x2b, 0x03,
	/* MXT224_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	/* MXT224_PROCI_TWOTOUCH(27) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* MXT224_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x00, 0x10, 0x20, 0x3c,
};



//add by wangpl 
#if 0
static const u8 init_vals_ver_33[] = {
	/* mxt224_GEN_POWER(7) */
	255, 255, 50,
	/* mxt224_GEN_ACQUIRE(8) */
	6, 5, 10, 10, 0, 0, 255, 1, 0, 0,
	/* mxt224_TOUCH_MULTI(9) */
	139, 0, 2, 15, 9, 0, 17, 40, 2, 1,
	0, 1, 1, 0, 5, 10, 10, 10, 28, 2,
	63, 1, 5, 0, 2, 4, 208, 70, 64, 0,
	10, 5,
	/* mxt224_TOUCH_KEYARRAY(15) */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
	/* mxt224_SPT_GPIOPWM(18) */
	0, 0,
	/* mxt224_SPT_GPIOPWM(19) */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	/* mxt224_PROCI_GRIPFACE(20) */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	/* mxt224_PROCG_NOISE(22) */
	13, 0, 0, 25, 0, 231, 255, 4, 35, 0,
	0, 16, 31, 57, 65, 0, 4,
	/* mxt224_TOUCH_PROXIMITY(23) */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	/* mxt224_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* mxt224_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,

	/* mxt224_SPT_CTECONFIG(28) */
	0, 0, 2, 8, 8, 30,
};

#endif


static const u8 init_vals_ver_33[] = {
	/* mxt224_GEN_POWER(7)   size: 3*/
	255, 255, 25,
	/* mxt224_GEN_ACQUIRE(8)    size: 10*/
	6, 0, 10, 10, 0, 0, 255, 1, 0, 0,
	/* mxt224_TOUCH_MULTI(9)    size: 32*/
	139, 0, 2, 15, 9, 0, 16, 45, 2, 1,
	10, 3, 1, 0, 5, 10, 10, 10, 0x1c, 2,
  	0x3f, 1, 5, 0, 2, 4, 208, 70, 64, 0, 10,  5,
	/* mxt224_TOUCH_KEYARRAY(15)    size: 11*/
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* mxt224_SPT_GPIOPWM(18)    size: 2*/
	0, 0,
	/* mxt224_SPT_GPIOPWM(19)    size: 16*/
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	/* mxt224_PROCI_GRIPFACE(20)    size: 12*/
	0x15, 0, 0, 0, 0, 0, 0, 25, 35, 4, 20, 0,
	/* mxt224_PROCG_NOISE(22)    size: 17 ????*/
	0x0d, 0, 0, 25, 0, 231, 255, 4, 35, 0,
	0, 16, 31, 57, 65, 0, 4,
	/* mxt224_TOUCH_PROXIMITY(23)    size: 15*/
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	/* mxt224_PROCI_ONETOUCH(24)    size: 19*/
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* mxt224_SPT_SELFTEST(25)    size: 14*/
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,

	/* mxt224_SPT_CTECONFIG(28)    size: 6*/
	0, 0, 2, 8, 8, 30,
};


struct mxt224_info {
	u8 family_id;
	u8 variant_id;
	u8 version;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 object_num;
};

struct mxt224_object {
	u8 type;
	u16 start_address;
	u8 size;
	u8 instances;
	u8 num_report_ids;

	/* to map object and message */
	u8 max_reportid;
};

struct mxt224_message {
	u8 reportid;
	u8 message[7];
	u8 checksum;
};

struct mxt224_finger {
	int status;
	int x;
	int y;
	int area;
};

/* Each client has this additional data */
struct mxt224_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	const struct mxt224_platform_data *pdata;
	struct mxt224_object *object_table;
	struct mxt224_info info;
	struct mxt224_finger finger[MXT224_MAX_FINGER];
	unsigned int irq;
	int pen_down;
	struct early_suspend suspend_desc;

#ifdef USE_BRCM_WORK_QUEUE
//	struct delayed_work work;
	struct work_struct work;
#endif

};

static bool mxt224_object_readable(unsigned int type)
{
	switch (type) {
	case MXT224_GEN_MESSAGE:
	case MXT224_GEN_COMMAND:
	case MXT224_GEN_POWER:
	case MXT224_GEN_ACQUIRE:
	case MXT224_TOUCH_MULTI:
	case MXT224_TOUCH_KEYARRAY:
	case MXT224_TOUCH_PROXIMITY:
	case MXT224_PROCI_GRIPFACE:
	case MXT224_PROCG_NOISE:
	case MXT224_PROCI_ONETOUCH:
	case MXT224_PROCI_TWOTOUCH:
	case MXT224_SPT_COMMSCONFIG:
	case MXT224_SPT_GPIOPWM:
	case MXT224_SPT_SELFTEST:
	case MXT224_SPT_CTECONFIG:
	case MXT224_SPT_USERDATA:
		return true;
	default:
		return false;
	}
}

static bool mxt224_object_writable(unsigned int type)
{
	switch (type) {
//	case MXT224_GEN_COMMAND:
	case MXT224_GEN_POWER:
	case MXT224_GEN_ACQUIRE:
	case MXT224_TOUCH_MULTI:
	case MXT224_TOUCH_KEYARRAY:
	case MXT224_SPT_COMMSCONFIG:
	case MXT224_SPT_GPIOPWM:
	case MXT224_PROCI_GRIPFACE:
	case MXT224_PROCG_NOISE:
	case MXT224_TOUCH_PROXIMITY:
	case MXT224_PROCI_ONETOUCH:
//	case MXT224_PROCI_TWOTOUCH:
	case MXT224_SPT_SELFTEST:
	case MXT224_SPT_CTECONFIG:
//	case MXT224_SPT_USERDATA:
	return true;
	default:
		return false;
	}
}

#if 0
static bool mxt_object_writable(unsigned int type)
{
	switch (type) {
//	case MXT_GEN_COMMAND:
	case MXT_GEN_POWER:
	case MXT_GEN_ACQUIRE:
	case MXT_TOUCH_MULTI:
	case MXT_TOUCH_KEYARRAY:
	case MXT_SPT_COMMSCONFIG:
	case MXT_SPT_GPIOPWM:	
	case MXT_PROCI_GRIPFACE:
	case MXT_PROCG_NOISE:		
	case MXT_TOUCH_PROXIMITY:
	case MXT_PROCI_ONETOUCH:
//	case MXT_PROCI_TWOTOUCH:
//	case MXT_PROCI_GRIP:
//	case MXT_PROCI_PALM:
	case MXT_SPT_SELFTEST:
	case MXT_SPT_CTECONFIG:
		return true;
	default:
		return false;
	}
}

#endif
static void mxt224_dump_message(struct device *dev,
				  struct mxt224_message *message)
{
#if 0
	printk("\n\n---by wangpl, in func <%s>---\n\n",__func__);


	printk("\n\nreportid:\t0x%x\n", message->reportid);
	printk("message1:\t0x%x\n", message->message[0]);
	printk("message2:\t0x%x\n", message->message[1]);
	printk("message3:\t0x%x\n", message->message[2]);
	printk("message4:\t0x%x\n", message->message[3]);
	printk("message5:\t0x%x\n", message->message[4]);
	printk("message6:\t0x%x\n", message->message[5]);
	printk("message7:\t0x%x\n", message->message[6]);
	printk("checksum:\t0x%x\n\n\n", message->checksum);
#endif
	
	dev_dbg(dev, "reportid:\t0x%x\n", message->reportid);
	dev_dbg(dev, "message1:\t0x%x\n", message->message[0]);
	dev_dbg(dev, "message2:\t0x%x\n", message->message[1]);
	dev_dbg(dev, "message3:\t0x%x\n", message->message[2]);
	dev_dbg(dev, "message4:\t0x%x\n", message->message[3]);
	dev_dbg(dev, "message5:\t0x%x\n", message->message[4]);
	dev_dbg(dev, "message6:\t0x%x\n", message->message[5]);
	dev_dbg(dev, "message7:\t0x%x\n", message->message[6]);
	dev_dbg(dev, "checksum:\t0x%x\n", message->checksum);
}

static int mxt224_check_bootloader(struct i2c_client *client,
				     unsigned int state)
{
	u8 val;

recheck:
	if (i2c_master_recv(client, &val, 1) != 1) {
		dev_err(&client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	switch (state) {
	case MXT224_WAITING_BOOTLOAD_CMD:
	case MXT224_WAITING_FRAME_DATA:
		val &= ~MXT224_BOOT_STATUS_MASK;
		break;
	case MXT224_FRAME_CRC_PASS:
		if (val == MXT224_FRAME_CRC_CHECK)
			goto recheck;
		break;
	default:
		return -EINVAL;
	}

	if (val != state) {
		dev_err(&client->dev, "Unvalid bootloader mode state\n");
		return -EINVAL;
	}

	return 0;
}

static int mxt224_unlock_bootloader(struct i2c_client *client)
{
	u8 buf[2];

	buf[0] = MXT224_UNLOCK_CMD_LSB;
	buf[1] = MXT224_UNLOCK_CMD_MSB;

	if (i2c_master_send(client, buf, 2) != 2) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int mxt224_fw_write(struct i2c_client *client,
			     const u8 * data, unsigned int frame_size)
{
	if (i2c_master_send(client, data, frame_size) != frame_size) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int __mxt224_read_reg(struct i2c_client *client,
			       u16 reg, u16 len, void *val)
{
	struct i2c_msg xfer[2];
	u8 buf[2];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = buf;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = val;

	if (i2c_transfer(client->adapter, xfer, 2) != 2) {
		dev_err(&client->dev, "%s: i2c transfer failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int mxt224_read_reg(struct i2c_client *client, u16 reg, u8 * val)
{
	return __mxt224_read_reg(client, reg, 1, val);
}

static int mxt224_write_reg(struct i2c_client *client, u16 reg, u8 val)
{
	u8 buf[3];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;
	buf[2] = val;

	if (i2c_master_send(client, buf, 3) != 3) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int mxt224_read_object_table(struct i2c_client *client,
				      u16 reg, u8 * object_buf)
{
	return __mxt224_read_reg(client, reg, MXT224_OBJECT_SIZE,
				   object_buf);
}

static struct mxt224_object *mxt224_get_object(struct mxt224_data *data,
						   u8 type)
{
	struct mxt224_object *object;
	int i;

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;
		if (object->type == type)
			return object;
	}

	dev_err(&data->client->dev, "Invalid object type\n");
	return NULL;
}


//by wangpl
static int mxt224_read_config_each_T_byte(struct i2c_client *client,
				      u16 reg, u8 offset, u8 * val)
{
	return __mxt224_read_reg(client, reg+offset, 1, val);
}


static int mxt224_read_config_each_T(struct mxt224_data * data, u8 type)
{
	int j, ret=0;
	u8 val[40];
	struct mxt224_object * object;
	
	object = mxt224_get_object(data, type);
	if (!object)
		return -EINVAL;
	
	for(j=0; j < object->size + 1; j++){
		mxt224_read_config_each_T_byte(data->client, object->start_address, j, val+j);
	}

	printk("\n\n---by wangpl, in <%s>, type=<%d>, size=<%d>+1, config is:---\n\n", __func__, type, object->size);
	
	for(j=0; j < object->size + 1; j++){
		printk("%d,", val[j]);
	}
	printk("\n----------end---------------\n\n");
	
	return 1;
}


static int mxt224_read_all_config(struct mxt224_data * data)
{

	struct mxt224_object *object;
	int i;

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;

		if (!mxt224_object_writable(object->type))
			continue;

//		printk("---by wangpl, in func <%s>, type is <%d>---\n",__func__, object->type);

		mxt224_read_config_each_T(data, object->type);
	}
	return 1;
}

//end by wangpl



static int mxt224_read_message(struct mxt224_data *data,
				 struct mxt224_message *message)
{
	struct mxt224_object *object;
	u16 reg;

//	printk("\n\n---by wangpl, in func <%s>---\n\n",__func__);
	
	object = mxt224_get_object(data, MXT224_GEN_MESSAGE);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return __mxt224_read_reg(data->client, reg,
				   sizeof(struct mxt224_message), message);
}

static int mxt224_read_object(struct mxt224_data *data,
				u8 type, u8 offset, u8 * val)
{
	struct mxt224_object *object;
	u16 reg;

	object = mxt224_get_object(data, type);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return __mxt224_read_reg(data->client, reg + offset, 1, val);
}

static int mxt224_write_object(struct mxt224_data *data,
				 u8 type, u8 offset, u8 val)
{
	struct mxt224_object *object;
	u16 reg;

	object = mxt224_get_object(data, type);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return mxt224_write_reg(data->client, reg + offset, val);
}



static void mxt224_input_report(struct mxt224_data *data, int single_id)
{
	struct mxt224_finger *finger = data->finger;
	struct input_dev *input_dev = data->input_dev;
	int status = finger[single_id].status;
	int finger_num = 0;
	int id, i;

//	for (id = 0; id < MXT224_MAX_FINGER; id++) {
	for (id = 0; id < 2; id++) {
		if (!finger[id].status)
			continue;
		
//		printk("\n\n---by wangpl, in func <%s>, status is <0x%x>, finger id = <%d>---\n\n",__func__, status, id);

//		printk("---by wangpl, x=<%d>, y=<%d>---\n\n", finger[id].x, finger[id].y);
		
		if(finger[id].y< 480){
			input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR,
				 finger[id].status != MXT224_RELEASE ?
				 finger[id].area : 0);
			input_report_abs(input_dev, ABS_MT_POSITION_X, finger[id].x);
			input_report_abs(input_dev, ABS_MT_POSITION_Y, finger[id].y);
			input_mt_sync(input_dev);
#if 0
			if(finger[id].status == MXT224_MOVE){
				for(i=0; i<20; i++) {
					input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR,
						 finger[id].status != MXT224_RELEASE ?
						 finger[id].area : 0);
					input_report_abs(input_dev, ABS_MT_POSITION_X, finger[id].x);
					input_report_abs(input_dev, ABS_MT_POSITION_Y, finger[id].y);
					input_mt_sync(input_dev);
					
					input_sync(input_dev);
					if (finger[id].x > 240){
						finger[id].x = 0;
					}else{
						finger[id].x += 10;
					}
				}
			}
#endif 			
		}
		else{
			if(finger[id].status == MXT224_RELEASE){
				
				if(finger[id].x < 75){
					input_report_key(input_dev, KEY_HOME, 0);
				}else if((finger[id].x > 85) && (finger[id].x < 155)){
					input_report_key(input_dev, KEY_MENU, 0);
				}else if((finger[id].x > 165) && (finger[id].x < 235)){
					input_report_key(input_dev, KEY_BACK, 0);
				}else if(finger[id].x > 245){
					input_report_key(input_dev, KEY_SEARCH, 0);
				}

			}else if(finger[id].status == MXT224_PRESS){
				
				if(finger[id].x < 75){
					input_report_key(input_dev, KEY_HOME, 1);
				}else if((finger[id].x > 85) && (finger[id].x < 155)){
					input_report_key(input_dev, KEY_MENU, 1);
				}else if((finger[id].x > 165) && (finger[id].x < 235)){
					input_report_key(input_dev, KEY_BACK, 1);
				}else if(finger[id].x > 245){
					input_report_key(input_dev, KEY_SEARCH, 1);
				}
			
			}
		}		

		if (finger[id].status == MXT224_RELEASE)
			finger[id].status = 0;
		else
			finger_num++;
	}

	input_report_key(input_dev, BTN_TOUCH, finger_num > 0);

	if (status != MXT224_RELEASE) {
		data->pen_down = 1;
		input_report_abs(input_dev, ABS_X, finger[single_id].x);
		input_report_abs(input_dev, ABS_Y, finger[single_id].y);
	} else {
		data->pen_down = 0;
	}

	input_sync(input_dev);
}


static void mxt224_input_touchevent(struct mxt224_data *data,
				      struct mxt224_message *message, int id)
{
	struct mxt224_finger *finger = data->finger;
	struct device *dev = &data->client->dev;
	u8 status = message->message[0];
	int x;
	int y;
	int area;

//	printk("\n\n---by wangpl, in func <%s>, status is <0x%x>---\n\n",__func__, status);

	/* Check the touch is present on the screen */
	if (!(status & MXT224_DETECT)) {
		if (status & MXT224_RELEASE) {
			dev_dbg(dev, "[%d] released\n", id);

			finger[id].status = MXT224_RELEASE;
			mxt224_input_report(data, id);
		}
		return;
	}

	/* Check only AMP detection */
	if (!(status & (MXT224_PRESS | MXT224_MOVE)))
		return;

	x = (message->message[1] << 2) | ((message->message[3] & ~0x3f) >> 6);
	y = (message->message[2] << 2) | ((message->message[3] & ~0xf3) >> 2);
	area = message->message[4];

//	printk("\n\n--by wangpl---[%d] %s x: %d, y: %d, area: %d\n\n", id, status & MXT224_MOVE ? "moved" : "pressed", x, y, area);
	
	dev_dbg(dev, "[%d] %s x: %d, y: %d, area: %d\n", id,
		status & MXT224_MOVE ? "moved" : "pressed", x, y, area);


	finger[id].status = status & MXT224_MOVE ?
	    MXT224_MOVE : MXT224_PRESS;

	finger[id].x = x;
	finger[id].y = y;
	finger[id].area = area;

	mxt224_input_report(data, id);

//	printk("\n\n---by wangpl, end of func <%s>---\n\n",__func__);

}

#ifndef USE_BRCM_WORK_QUEUE
static irqreturn_t mxt224_interrupt(int irq, void *dev_id)
{
	struct mxt224_data *data = dev_id;
	struct mxt224_message message;
	struct mxt224_object *object;
	struct device *dev = &data->client->dev;
	int id;
	u8 reportid;
	u8 max_reportid;
	u8 min_reportid;

	do {
		if (mxt224_read_message(data, &message)) {
			dev_err(dev, "Failed to read message\n");
			goto end;
		}

		reportid = message.reportid;

		/* whether reportid is thing of MXT224_TOUCH_MULTI */
		object = mxt224_get_object(data, MXT224_TOUCH_MULTI);
		if (!object)
			goto end;

		max_reportid = object->max_reportid;
		min_reportid = max_reportid - object->num_report_ids + 1;
		id = reportid - min_reportid;

		if (reportid >= min_reportid && reportid <= max_reportid)
			mxt224_input_touchevent(data, &message, id);
		else
			mxt224_dump_message(dev, &message);
	} while (reportid != 0xff);

end:
	return IRQ_HANDLED;
}
#endif

#ifdef USE_BRCM_WORK_QUEUE


#define TS_POLL_DELAY	(0)	/* ms delay between irq & sample */
#define to_delayed_work(_work) container_of(_work, struct delayed_work, work)

static void mxt224_read_input(struct mxt224_data *data);

static void mxt224_touch_work(struct work_struct *work)
{
	struct mxt224_data *p_data =
	    container_of(to_delayed_work(work), struct mxt224_data, work);

//	printk("\n\n---by wangpl, in func <%s>---\n\n",__func__);
	
	/* Peform the actual read and send the events to input sub-system */
	mxt224_read_input(p_data);

	/* Re-enable the interrupts from the device */
	enable_irq(p_data->irq);
}

static void mxt224_read_input(struct mxt224_data *data)
{
	/* Implement what is implemented in the threaded IRQ implementation of the original driver */
	struct mxt224_message message;
	struct mxt224_object *object;
	struct device *dev = &data->client->dev;
	int id;
	u8 reportid;
	u8 max_reportid;
	u8 min_reportid;


	do {
		if (mxt224_read_message(data, &message)) {
			dev_err(dev, "Failed to read message\n");
			goto end;
		}
		reportid = message.reportid;

//		printk("\n\n---by wangpl, in func <%s>, message.reportid = <%d>---\n\n",__func__, reportid );

		/* whether reportid is thing of MXT224_TOUCH_MULTI */
		object = mxt224_get_object(data, MXT224_TOUCH_MULTI);
		if (!object)
			goto end;

		max_reportid = object->max_reportid;
		min_reportid = max_reportid - object->num_report_ids + 1;
		id = reportid - min_reportid;

		if (reportid >= min_reportid && reportid <= max_reportid)
			mxt224_input_touchevent(data, &message, id);
		else {
			mxt224_dump_message(dev, &message);
		}
	} while (reportid != 0xff);
end:
	return;
}

static irqreturn_t mxt224_touch_interrupt(int irq, void *dev_id)
{
	struct mxt224_data *p_data = dev_id;

//	printk("\n\n---by wangpl, in func <%s>---\n\n",__func__);
	
	/*
	 * Disable further interrupts from the device
	 * Note that, we assume that the device is configured
	 * to operate in  Mode1 where the interrupt line is
	 * pulled low until the last available valid message is
	 * read from the device. Since the irq is configured as
	 * edge sensitive for falling edge, we explicitly disable
	 * further interrupts from the device until we finish
	 * processing all the messages from the bottom half and
	 * re-enable it from the bottom half.
	 */
	disable_irq_nosync(p_data->irq);
//	schedule_delayed_work(&p_data->work, msecs_to_jiffies(TS_POLL_DELAY));
	queue_work(mxt224_wq, &p_data->work);
	return IRQ_HANDLED;
}
#endif

static int mxt224_check_reg_init(struct mxt224_data *data)
{
	struct mxt224_object *object;
	struct device *dev = &data->client->dev;
	int index = 0;
	int i, j;
	u8 version = data->info.version;
	u8 *init_vals;

	switch (version) {
	case MXT224_VER_20:
		init_vals = (u8 *) init_vals_ver_20;
		break;
	case MXT224_VER_21:
		init_vals = (u8 *) init_vals_ver_21;
		break;
	case MXT224_VER_22:
		init_vals = (u8 *) init_vals_ver_22;
		break;
//by wangpl
	case MXT224_VER_33:
		init_vals = (u8 *) init_vals_ver_33;
		break;
	default:
		dev_err(dev, "Firmware version %d doesn't support\n", version);
		return -EINVAL;
	}

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;

		if (!mxt224_object_writable(object->type))
			continue;

//		printk("---by wangpl, in func <%s>, write object->type= <%d>---\n",__func__, object->type);

		for (j = 0; j < object->size + 1; j++)
			mxt224_write_object(data, object->type, j,
					      init_vals[index + j]);

		index += object->size + 1;

//		printk("---by wangpl, in func <%s>, index= <%d>---\n",__func__, index);

	}

	return 0;
}

static int mxt224_check_matrix_size(struct mxt224_data *data)
{
	const struct mxt224_platform_data *pdata = data->pdata;
	struct device *dev = &data->client->dev;
	int mode = -1;
	int error;
	u8 val;

	dev_dbg(dev, "Number of X lines: %d\n", pdata->x_line);
	dev_dbg(dev, "Number of Y lines: %d\n", pdata->y_line);

	switch (pdata->x_line) {
	case 0 ... 15:
		if (pdata->y_line <= 14)
			mode = 0;
		break;
	case 16:
		if (pdata->y_line <= 12)
			mode = 1;
		if (pdata->y_line == 13 || pdata->y_line == 14)
			mode = 0;
		break;
	case 17:
		if (pdata->y_line <= 11)
			mode = 2;
		if (pdata->y_line == 12 || pdata->y_line == 13)
			mode = 1;
		break;
	case 18:
		if (pdata->y_line <= 10)
			mode = 3;
		if (pdata->y_line == 11 || pdata->y_line == 12)
			mode = 2;
		break;
	case 19:
		if (pdata->y_line <= 9)
			mode = 4;
		if (pdata->y_line == 10 || pdata->y_line == 11)
			mode = 3;
		break;
	case 20:
		mode = 4;
	}

	if (mode < 0) {
		dev_err(dev, "Invalid X/Y lines\n");
		return -EINVAL;
	}

	error = mxt224_read_object(data, MXT224_SPT_CTECONFIG,
				     MXT224_CTE_MODE, &val);
	if (error)
		return error;

	if (mode == val)
		return 0;

	/* Change the CTE configuration */
	mxt224_write_object(data, MXT224_SPT_CTECONFIG,
			      MXT224_CTE_CTRL, 1);
	mxt224_write_object(data, MXT224_SPT_CTECONFIG,
			      MXT224_CTE_MODE, mode);
	mxt224_write_object(data, MXT224_SPT_CTECONFIG,
			      MXT224_CTE_CTRL, 0);

	return 0;
}

static int mxt224_make_highchg(struct mxt224_data *data)
{
	struct device *dev = &data->client->dev;
	struct mxt224_message message;
	int count = 10;
	int error;

//	printk("\n\n---by wangpl, in func <%s>---\n\n",__func__);

	/* Read dummy message to make high CHG pin */
	do {
		error =
		    mxt224_read_message(data, &message);
		if (error)
			return error;
	} while (message.reportid != 0xff && --count);

	if (!count) {
		dev_err(dev, "CHG pin isn't cleared\n");
		return -EBUSY;
	}

	return 0;
}

static void mxt224_handle_pdata(struct mxt224_data *data)
{
	const struct mxt224_platform_data *pdata = data->pdata;
	u8 voltage;

	/* Set touchscreen lines */
	mxt224_write_object(data, MXT224_TOUCH_MULTI, MXT224_TOUCH_XSIZE,
			      pdata->x_line);
	mxt224_write_object(data, MXT224_TOUCH_MULTI, MXT224_TOUCH_YSIZE,
			      pdata->y_line);

	/* Set touchscreen orient */
	mxt224_write_object(data, MXT224_TOUCH_MULTI, MXT224_TOUCH_ORIENT,
			      pdata->orient);

	/* Set touchscreen burst length */
	mxt224_write_object(data, MXT224_TOUCH_MULTI,
			      MXT224_TOUCH_BLEN, pdata->blen);

	/* Set touchscreen threshold */
	mxt224_write_object(data, MXT224_TOUCH_MULTI,
			      MXT224_TOUCH_TCHTHR, pdata->threshold);

	/* Set touchscreen resolution */
	mxt224_write_object(data, MXT224_TOUCH_MULTI,
			      MXT224_TOUCH_XRANGE_LSB,
			      (pdata->x_size - 1) & 0xff);
	mxt224_write_object(data, MXT224_TOUCH_MULTI,
			      MXT224_TOUCH_XRANGE_MSB,
			      (pdata->x_size - 1) >> 8);
	mxt224_write_object(data, MXT224_TOUCH_MULTI,
			      MXT224_TOUCH_YRANGE_LSB,
			      (pdata->y_size - 1) & 0xff);
	mxt224_write_object(data, MXT224_TOUCH_MULTI,
			      MXT224_TOUCH_YRANGE_MSB,
			      (pdata->y_size - 1) >> 8);

	/* Set touchscreen voltage */
	
	if (data->info.version >= MXT224_VER_21 && pdata->voltage) {
		if (pdata->voltage < MXT224_VOLTAGE_DEFAULT) {
			voltage = (MXT224_VOLTAGE_DEFAULT - pdata->voltage) /
			    MXT224_VOLTAGE_STEP;
			voltage = 0xff - voltage + 1;
		} else
			voltage = (pdata->voltage - MXT224_VOLTAGE_DEFAULT) /
			    MXT224_VOLTAGE_STEP;

		mxt224_write_object(data, MXT224_SPT_CTECONFIG,
				      MXT224_CTE_VOLTAGE, voltage);
	}

}

static int mxt224_get_info(struct mxt224_data *data)
{
	struct i2c_client *client = data->client;
	struct mxt224_info *info = &data->info;
	int error;
	u8 val;

	error = mxt224_read_reg(client, MXT224_FAMILY_ID, &val);
	if (error)
		return error;
	info->family_id = val;

	error = mxt224_read_reg(client, MXT224_VARIANT_ID, &val);
	if (error)
		return error;
	info->variant_id = val;

//TP ID detect of ft5x02 & mxt224
	if(info->family_id != 0x80 || info->variant_id != 0x7){
		printk("\n\n---by wangpl, in func <%s>, TP is not MXT224, abort mxt224 probe---\n\n",__func__);
		return 1;
	}
	

	error = mxt224_read_reg(client, MXT224_VERSION, &val);
	if (error)
		return error;
	info->version = val;

	error = mxt224_read_reg(client, MXT224_BUILD, &val);
	if (error)
		return error;
	info->build = val;

	error = mxt224_read_reg(client, MXT224_OBJECT_NUM, &val);
	if (error)
		return error;
	info->object_num = val;
	
	printk("\n\n---by wangpl, end of func <%s>, info->family_id=<0x%x>, info->variant_id=<0x%x>," 
		"info->version=<0x%x>, info->build=<0x%x>, info->object_num=<%d>---\n\n",
		__func__, info->family_id, info->variant_id, info->version, info->build, info->object_num);
	
	return 0;
}

static int mxt224_get_object_table(struct mxt224_data *data)
{
	int error;
	int i;
	u16 reg;
	u8 reportid = 0;
	u8 buf[MXT224_OBJECT_SIZE];

//	printk("\n\n---by wangpl, in func <%s>---\n\n",__func__);
	
	for (i = 0; i < data->info.object_num; i++) {
		struct mxt224_object *object = data->object_table + i;

		reg = MXT224_OBJECT_START + MXT224_OBJECT_SIZE * i;
		error = mxt224_read_object_table(data->client, reg, buf);
		if (error)
			return error;

		object->type = buf[0];
		object->start_address = (buf[2] << 8) | buf[1];
		object->size = buf[3];
		object->instances = buf[4];
		object->num_report_ids = buf[5];

		if (object->num_report_ids) {
			reportid += object->num_report_ids *
			    (object->instances + 1);
			object->max_reportid = reportid;
		}
//by wangpl
#if 0
		printk("\n\n---by wangpl, object->type=<%d>, object->start_address=<0x%x>,"
		"object->size=<%d>,object->instances=<%d>,object->num_report_ids=<%d>, object->max_reportid=<%d>---\n\n",
		object->type, object->start_address, object->size, object->instances, object->num_report_ids, object->max_reportid);
#endif		
	}
	return 0;
}

static int mxt224_initialize(struct mxt224_data *data)
{
	struct i2c_client *client = data->client;
	struct mxt224_info *info = &data->info;
	int error;
	u8 val;

	printk("\n\n---by wangpl, in func <%s>---\n\n",__func__);
	
	error = mxt224_get_info(data);
	if (error)
		return error;

	data->object_table = kcalloc(info->object_num,
				     sizeof(struct mxt224_data), GFP_KERNEL);
	if (!data->object_table) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	/* Get object table information */
	error = mxt224_get_object_table(data);
	if (error)
		return error;

	/* Check register init values */
	error = mxt224_check_reg_init(data);
	if (error)
		return error;
//by wangpl
#if 0
	/* Check X/Y matrix size */
	error = mxt224_check_matrix_size(data);
	if (error)
		return error;
#endif

	mxt224_handle_pdata(data);


	/* Backup to memory */
	mxt224_write_object(data, MXT224_GEN_COMMAND,
			      MXT224_COMMAND_BACKUPNV, MXT224_BACKUP_VALUE);
	msleep(MXT224_BACKUP_TIME);

	/* Soft reset */
	mxt224_write_object(data, MXT224_GEN_COMMAND,
			      MXT224_COMMAND_RESET, 1);
	msleep(MXT224_RESET_TIME);

	/* Update matrix size at info struct */
	error = mxt224_read_reg(client, MXT224_MATRIX_X_SIZE, &val);
	if (error)
		return error;
	info->matrix_xsize = val;

	error = mxt224_read_reg(client, MXT224_MATRIX_Y_SIZE, &val);
	if (error)
		return error;
	info->matrix_ysize = val;

	dev_info(&client->dev,
		 "Family ID: %d Variant ID: %d Version: %d Build: %d\n",
		 info->family_id, info->variant_id, info->version, info->build);

	dev_info(&client->dev,
		 "Matrix X Size: %d Matrix Y Size: %d Object Num: %d\n",
		 info->matrix_xsize, info->matrix_ysize, info->object_num);

	return 0;
}

static ssize_t mxt224_object_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct mxt224_data *data = dev_get_drvdata(dev);
	struct mxt224_object *object;
	int count = 0;
	int i, j;
	int error;
	u8 val;

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;

		count += sprintf(buf + count,
				 "Object Table Element %d(Type %d)\n",
				 i + 1, object->type);

		if (!mxt224_object_readable(object->type)) {
			count += sprintf(buf + count, "\n");
			continue;
		}

		for (j = 0; j < object->size + 1; j++) {
			error = mxt224_read_object(data,
						     object->type, j, &val);
			if (error)
				return error;

			count += sprintf(buf + count,
					 "  Byte %d: 0x%x (%d)\n", j, val, val);
		}

		count += sprintf(buf + count, "\n");
	}

	return count;
}

static int mxt224_load_fw(struct device *dev, const char *fn)
{
	struct mxt224_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	const struct firmware *fw = NULL;
	unsigned int frame_size;
	unsigned int pos = 0;
	int ret;

	ret = request_firmware(&fw, fn, dev);
	if (ret) {
		dev_err(dev, "Unable to open firmware %s\n", fn);
		return ret;
	}

	/* Change to the bootloader mode */
	mxt224_write_object(data, MXT224_GEN_COMMAND,
			      MXT224_COMMAND_RESET, MXT224_BOOT_VALUE);
	msleep(MXT224_RESET_TIME);

	/* Change to slave address of bootloader */
	if (client->addr == MXT224_APP_LOW)
		client->addr = MXT224_BOOT_LOW;
	else
		client->addr = MXT224_BOOT_HIGH;

	ret = mxt224_check_bootloader(client, MXT224_WAITING_BOOTLOAD_CMD);
	if (ret)
		goto out;

	/* Unlock bootloader */
	mxt224_unlock_bootloader(client);

	while (pos < fw->size) {
		ret = mxt224_check_bootloader(client,
						MXT224_WAITING_FRAME_DATA);
		if (ret)
			goto out;

		frame_size = ((*(fw->data + pos) << 8) | *(fw->data + pos + 1));

		/* We should add 2 at frame size as the the firmware data is not
		 * included the CRC bytes.
		 */
		frame_size += 2;

		/* Write one frame to device */
		mxt224_fw_write(client, fw->data + pos, frame_size);

		ret = mxt224_check_bootloader(client,
						MXT224_FRAME_CRC_PASS);
		if (ret)
			goto out;

		pos += frame_size;

		dev_dbg(dev, "Updated %d bytes / %zd bytes\n", pos, fw->size);
	}

out:
	release_firmware(fw);

	/* Change to slave address of application */
	if (client->addr == MXT224_BOOT_LOW)
		client->addr = MXT224_APP_LOW;
	else
		client->addr = MXT224_APP_HIGH;

	return ret;
}

static ssize_t mxt224_update_fw_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct mxt224_data *data = dev_get_drvdata(dev);
	unsigned int version;
	int error;

	if (sscanf(buf, "%u", &version) != 1) {
		dev_err(dev, "Invalid values\n");
		return -EINVAL;
	}

	if (data->info.version < MXT224_VER_21 || version < MXT224_VER_21) {
		dev_err(dev, "FW update supported starting with version 21\n");
		return -EINVAL;
	}

	disable_irq(data->irq);

	error = mxt224_load_fw(dev, MXT224_FW_NAME);
	if (error) {
		dev_err(dev, "The firmware update failed(%d)\n", error);
		count = error;
	} else {
		dev_dbg(dev, "The firmware update succeeded\n");

		/* Wait for reset */
		msleep(MXT224_FWRESET_TIME);

		kfree(data->object_table);
		data->object_table = NULL;

		mxt224_initialize(data);
	}

	enable_irq(data->irq);

	return count;
}

static DEVICE_ATTR(object, 0444, mxt224_object_show, NULL);
static DEVICE_ATTR(update_fw, 0664, NULL, mxt224_update_fw_store);

#if defined(DEBUG_PEN_STATUS)
static ssize_t pen_status_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mxt224_data *p_data =
	    (struct mxt224_data *)dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", p_data->pen_down);
}

static DEVICE_ATTR(pen_status, 0644, pen_status_show, NULL);
#endif

static struct attribute *mxt224_attrs[] = {
	&dev_attr_object.attr,
	&dev_attr_update_fw.attr,
#if defined(DEBUG_PEN_STATUS)
	&dev_attr_pen_status.attr,
#endif
	NULL
};

static const struct attribute_group mxt224_attr_group = {
	.attrs = mxt224_attrs,
};

static void mxt224_start(struct mxt224_data *data)
{
	/* Touch enable */

	/*
	 * Writing 0x03 does not enable the SCANEN bit.
	 * which is optimized for not notifying the Host with unnecessary
	 * messages.
	 * The data sheet says that, if there are some issues in detecting
	 * multiple touches try, enabling this bit i.e write 0x83
	 */
	mxt224_write_object(data,
			      MXT224_TOUCH_MULTI, MXT224_TOUCH_CTRL, 0x03);
}

static void mxt224_stop(struct mxt224_data *data)
{
	/* Touch disable */
	mxt224_write_object(data,
			      MXT224_TOUCH_MULTI, MXT224_TOUCH_CTRL, 0);
}

static int mxt224_input_open(struct input_dev *dev)
{
	struct mxt224_data *data = input_get_drvdata(dev);

	mxt224_start(data);

	return 0;
}

static void mxt224_input_close(struct input_dev *dev)
{
	struct mxt224_data *data = input_get_drvdata(dev);

	mxt224_stop(data);
}

#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
static int mxt224_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct mxt224_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev = data->input_dev;

	mutex_lock(&input_dev->mutex);

	if (input_dev->users) {
		mxt224_stop(data);

		mxt224_write_object(data, MXT224_GEN_POWER,
				MXT224_POWER_IDLEACQINT, 0);
		mxt224_write_object(data, MXT224_GEN_POWER,
				MXT224_POWER_ACTVACQINT, 0);
	}
	mutex_unlock(&input_dev->mutex);

	return 0;
}

static int mxt224_resume(struct i2c_client *client)
{
	struct mxt224_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev = data->input_dev;

	mutex_lock(&input_dev->mutex);

	if (input_dev->users) {
		mxt224_write_object(data, MXT224_GEN_POWER,
				MXT224_POWER_IDLEACQINT, init_vals_ver_22[14]);
		mxt224_write_object(data, MXT224_GEN_POWER,
				MXT224_POWER_ACTVACQINT, init_vals_ver_22[15]);
		mxt224_start(data);

	}

	mutex_unlock(&input_dev->mutex);

	return 0;
}
#else
#define mxt224_suspend	NULL
#define mxt224_resume		NULL
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mxt224_early_suspend(struct early_suspend *desc)
{
	struct mxt224_data *data = container_of(desc, struct mxt224_data, suspend_desc);
	pm_message_t mesg = {
		.event = PM_EVENT_SUSPEND,
		};
	mxt224_suspend(data->client, mesg);
}

static void mxt224_late_resume(struct early_suspend *desc)
{
	struct mxt224_data *data = container_of(desc, struct mxt224_data, suspend_desc);
	mxt224_resume(data->client);
}
#endif

static int __devinit mxt224_probe(struct i2c_client *client,
				    const struct i2c_device_id *id)
{
	struct mxt224_data *data;
	struct input_dev *input_dev;
	int error;

	printk("\n\n---by wangpl, in func <%s>---\n\n",__func__);

	if (!client->dev.platform_data)
		return -EINVAL;
	
    gpio_request(MXT224_WK, "gpio_tw_wakeup");
    gpio_direction_output(MXT224_WK, 1);

	gpio_set_value(MXT224_WK,1);
	gpio_set_value(MXT224_WK,0);
	udelay(100);
	gpio_set_value(MXT224_WK,1);
	mdelay(50);
	
	
	data = kzalloc(sizeof(struct mxt224_data), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!data || !input_dev) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	input_dev->name = "AT42MXT224/ATMXT224 Touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	input_dev->open = mxt224_input_open;
	input_dev->close = mxt224_input_close;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);

//by wangpl
	__set_bit(EV_SYN, input_dev->evbit);
	__set_bit(BTN_2, input_dev->keybit);
	__set_bit(KEY_MENU, input_dev->keybit);
	__set_bit(KEY_SEARCH, input_dev->keybit);
	__set_bit(KEY_BACK, input_dev->keybit);
	__set_bit(KEY_HOME, input_dev->keybit);

	
	data->client = client;
	data->input_dev = input_dev;
	data->pdata = client->dev.platform_data;
	data->irq = client->irq;

	/* For single touch */
	input_set_abs_params(input_dev, ABS_X,
			     data->pdata->x_min, data->pdata->x_size, 0, 0);
	input_set_abs_params(input_dev, ABS_Y,
			     data->pdata->y_min, data->pdata->y_size, 0, 0);

	/* For multi touch */
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
			     0, data->pdata->max_area, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
			     0, data->pdata->x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
			     0, data->pdata->y_max, 0, 0);
	input_set_drvdata(input_dev, data);

//by wangpl
	input_set_abs_params(input_dev, ABS_HAT0X, 0, 320, 0, 0);
	input_set_abs_params(input_dev, ABS_HAT0Y, 0, 540, 0, 0); // ??



	i2c_set_clientdata(client, data);

	if (data->pdata->platform_init)
		data->pdata->platform_init();

	error = mxt224_initialize(data);
	if (error)
		goto err_free_object;

	data->pen_down = 0;

#ifdef USE_BRCM_WORK_QUEUE
//	INIT_DELAYED_WORK(&data->work, mxt224_touch_work);
	INIT_WORK(&data->work, mxt224_touch_work);
	
	error = request_irq(client->irq, mxt224_touch_interrupt,
			    IRQF_TRIGGER_FALLING, "mxt224_touch", data);
	if (error < 0) {
		printk("mxt224_touch: Failed to register interrupt \r\n");
		goto err_free_object;
	}


	printk("mxt224_touch: registered the interrupt \r\n");
#else
	error = request_threaded_irq(client->irq, NULL, mxt224_interrupt,
				     IRQF_TRIGGER_FALLING,
				     client->dev.driver->name, data);
	if (error) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_free_object;
	}
#endif

	error = mxt224_make_highchg(data);
	if (error)
		return error;

//by wangpl
#if 0
	printk("\n\n---by wangpl, in func <%s>, before mxt224_read_all_config---\n\n",__func__);

	mxt224_read_all_config(data);

	printk("\n\n---by wangpl, in func <%s>, after mxt224_read_all_config---\n\n",__func__);
#endif

	
	error = input_register_device(input_dev);
	if (error)
		goto err_free_irq;

	error = sysfs_create_group(&client->dev.kobj, &mxt224_attr_group);
	if (error)
		goto err_unregister_device;

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
	data->suspend_desc.suspend = mxt224_early_suspend,
	data->suspend_desc.resume = mxt224_late_resume,
	register_early_suspend(&data->suspend_desc);
#endif
	return 0;

err_unregister_device:
	input_unregister_device(input_dev);
	input_dev = NULL;
err_free_irq:
	free_irq(client->irq, data);
err_free_object:
	kfree(data->object_table);
err_free_mem:
	input_free_device(input_dev);
	kfree(data);
	return error;
}

static int __devexit mxt224_remove(struct i2c_client *client)
{
	struct mxt224_data *data = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &mxt224_attr_group);
	free_irq(data->irq, data);
	input_unregister_device(data->input_dev);
	kfree(data->object_table);
	kfree(data);

	return 0;
}

void mxt224_shutdown(struct i2c_client *client)
{
	struct mxt224_data *data = i2c_get_clientdata(client);
	/* Soft reset  controller here to prevent it from
	 *getting into an un-responsive state */
	mxt224_write_object(data, MXT224_GEN_COMMAND,
			      MXT224_COMMAND_RESET, 1);
	msleep(MXT224_RESET_TIME);

}

static const struct i2c_device_id mxt224_id[] = {
	{"mxt224_ts", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, mxt224_id);

static struct i2c_driver mxt224_driver = {
	.driver = {
		   .name = "mxt224_ts",
		   .owner = THIS_MODULE,
		   },
	.probe = mxt224_probe,
	.remove = __devexit_p(mxt224_remove),
	.shutdown = mxt224_shutdown,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = mxt224_suspend,
	.resume = mxt224_resume,
#endif
	.id_table = mxt224_id,
};

static int __init mxt224_init(void)
{
	printk("\n\n---by wangpl, in func <%s>---\n\n",__func__);

	mxt224_wq = create_singlethread_workqueue("mxt224_wq");
	if (!mxt224_wq)
		return -ENOMEM;
	
	return i2c_add_driver(&mxt224_driver);
}

static void __exit mxt224_exit(void)
{
	i2c_del_driver(&mxt224_driver);

	if (mxt224_wq)
		destroy_workqueue(mxt224_wq);
}

module_init(mxt224_init);
module_exit(mxt224_exit);

/* Module information */
MODULE_AUTHOR("Joonyoung Shim <jy0922.shim@samsung.com>");
MODULE_DESCRIPTION("AT42MXT224/ATMXT224 Touchscreen driver");
MODULE_LICENSE("GPL");
