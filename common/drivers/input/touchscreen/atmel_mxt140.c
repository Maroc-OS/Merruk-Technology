/*
 * AT42ATMXT140/ATMXT224 Touchscreen driver
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/i2c/atmel_mxt140.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

/* Version */
#define ATMXT140_VER_20			0x20
#define ATMXT140_VER_21			0x21
#define ATMXT140_VER_22			0x22

/* Slave addresses */
#define ATMXT140_APP_LOW		0x4a
#define ATMXT140_APP_HIGH		0x4b
#define ATMXT140_BOOT_LOW		0x24
#define ATMXT140_BOOT_HIGH		0x25

/* Firmware */
#define ATMXT140_FW_NAME		"atmxt140.fw"

/* Registers */
#define ATMXT140_FAMILY_ID		0x00
#define ATMXT140_VARIANT_ID		0x01
#define ATMXT140_VERSION		0x02
#define ATMXT140_BUILD			0x03
#define ATMXT140_MATRIX_X_SIZE		0x04
#define ATMXT140_MATRIX_Y_SIZE		0x05
#define ATMXT140_OBJECT_NUM		0x06
#define ATMXT140_OBJECT_START		0x07

#define ATMXT140_OBJECT_SIZE		6

/* Object types */
#define ATMXT140_DEBUG_DIAGNOSTIC	37
#define ATMXT140_GEN_MESSAGE		5
#define ATMXT140_GEN_COMMAND		6
#define ATMXT140_GEN_POWER		7
#define ATMXT140_GEN_ACQUIRE		8
#define ATMXT140_TOUCH_MULTI		9
#define ATMXT140_TOUCH_KEYARRAY		15
#define ATMXT140_TOUCH_PROXIMITY	23
#define ATMXT140_PROCI_GRIPFACE		20
#define ATMXT140_PROCG_NOISE		22
#define ATMXT140_PROCI_ONETOUCH		24
#define ATMXT140_PROCI_TWOTOUCH		27
#define ATMXT140_SPT_COMMSCONFIG	18	/* firmware ver 21 over */
#define ATMXT140_SPT_GPIOPWM		19
#define ATMXT140_SPT_SELFTEST		25
#define ATMXT140_SPT_CTECONFIG		28
#define ATMXT140_SPT_USERDATA		38	/* firmware ver 21 over */

/* ATMXT140_GEN_COMMAND field */
#define ATMXT140_COMMAND_RESET		0
#define ATMXT140_COMMAND_BACKUPNV	1
#define ATMXT140_COMMAND_CALIBRATE	2
#define ATMXT140_COMMAND_REPORTALL	3
#define ATMXT140_COMMAND_DIAGNOSTIC	5

/* ATMXT140_GEN_POWER field */
#define ATMXT140_POWER_IDLEACQINT	0
#define ATMXT140_POWER_ACTVACQINT	1
#define ATMXT140_POWER_ACTV2IDLETO	2

/* ATMXT140_GEN_ACQUIRE field */
#define ATMXT140_ACQUIRE_CHRGTIME	0
#define ATMXT140_ACQUIRE_TCHDRIFT	2
#define ATMXT140_ACQUIRE_DRIFTST	3
#define ATMXT140_ACQUIRE_TCHAUTOCAL	4
#define ATMXT140_ACQUIRE_SYNC		5
#define ATMXT140_ACQUIRE_ATCHCALST	6
#define ATMXT140_ACQUIRE_ATCHCALSTHR	7

/* ATMXT140_TOUCH_MULTI field */
#define ATMXT140_TOUCH_CTRL		0
#define ATMXT140_TOUCH_XORIGIN		1
#define ATMXT140_TOUCH_YORIGIN		2
#define ATMXT140_TOUCH_XSIZE		3
#define ATMXT140_TOUCH_YSIZE		4
#define ATMXT140_TOUCH_BLEN		6
#define ATMXT140_TOUCH_TCHTHR		7
#define ATMXT140_TOUCH_TCHDI		8
#define ATMXT140_TOUCH_ORIENT		9
#define ATMXT140_TOUCH_MOVHYSTI		11
#define ATMXT140_TOUCH_MOVHYSTN		12
#define ATMXT140_TOUCH_NUMTOUCH		14
#define ATMXT140_TOUCH_MRGHYST		15
#define ATMXT140_TOUCH_MRGTHR		16
#define ATMXT140_TOUCH_AMPHYST		17
#define ATMXT140_TOUCH_XRANGE_LSB	18
#define ATMXT140_TOUCH_XRANGE_MSB	19
#define ATMXT140_TOUCH_YRANGE_LSB	20
#define ATMXT140_TOUCH_YRANGE_MSB	21
#define ATMXT140_TOUCH_XLOCLIP		22
#define ATMXT140_TOUCH_XHICLIP		23
#define ATMXT140_TOUCH_YLOCLIP		24
#define ATMXT140_TOUCH_YHICLIP		25
#define ATMXT140_TOUCH_XEDGECTRL	26
#define ATMXT140_TOUCH_XEDGEDIST	27
#define ATMXT140_TOUCH_YEDGECTRL	28
#define ATMXT140_TOUCH_YEDGEDIST	29
#define ATMXT140_TOUCH_JUMPLIMIT	30	/* firmware ver 22 over */

/* ATMXT140_PROCI_GRIPFACE field */
#define ATMXT140_GRIPFACE_CTRL		0
#define ATMXT140_GRIPFACE_XLOGRIP	1
#define ATMXT140_GRIPFACE_XHIGRIP	2
#define ATMXT140_GRIPFACE_YLOGRIP	3
#define ATMXT140_GRIPFACE_YHIGRIP	4
#define ATMXT140_GRIPFACE_MAXTCHS	5
#define ATMXT140_GRIPFACE_SZTHR1	7
#define ATMXT140_GRIPFACE_SZTHR2	8
#define ATMXT140_GRIPFACE_SHPTHR1	9
#define ATMXT140_GRIPFACE_SHPTHR2	10
#define ATMXT140_GRIPFACE_SUPEXTTO	11

/* ATMXT140_PROCI_NOISE field */
#define ATMXT140_NOISE_CTRL		0
#define ATMXT140_NOISE_OUTFLEN		1
#define ATMXT140_NOISE_GCAFUL_LSB	3
#define ATMXT140_NOISE_GCAFUL_MSB	4
#define ATMXT140_NOISE_GCAFLL_LSB	5
#define ATMXT140_NOISE_GCAFLL_MSB	6
#define ATMXT140_NOISE_ACTVGCAFVALID	7
#define ATMXT140_NOISE_NOISETHR		8
#define ATMXT140_NOISE_FREQHOPSCALE	10
#define ATMXT140_NOISE_FREQ0		11
#define ATMXT140_NOISE_FREQ1		12
#define ATMXT140_NOISE_FREQ2		13
#define ATMXT140_NOISE_FREQ3		14
#define ATMXT140_NOISE_FREQ4		15
#define ATMXT140_NOISE_IDLEGCAFVALID	16

/* ATMXT140_SPT_COMMSCONFIG */
#define ATMXT140_COMMS_CTRL		0
#define ATMXT140_COMMS_CMD		1

/* ATMXT140_SPT_CTECONFIG field */
#define ATMXT140_CTE_CTRL		0
#define ATMXT140_CTE_CMD		1
#define ATMXT140_CTE_MODE		2
#define ATMXT140_CTE_IDLEGCAFDEPTH	3
#define ATMXT140_CTE_ACTVGCAFDEPTH	4
#define ATMXT140_CTE_VOLTAGE		5	/* firmware ver 21 over */

#define ATMXT140_VOLTAGE_DEFAULT	2700000
#define ATMXT140_VOLTAGE_STEP		10000

/* Define for ATMXT140_GEN_COMMAND */
#define ATMXT140_BOOT_VALUE		0xa5
#define ATMXT140_BACKUP_VALUE		0x55
#define ATMXT140_BACKUP_TIME		25	/* msec */
#define ATMXT140_RESET_TIME		65	/* msec */

#define ATMXT140_FWRESET_TIME		175	/* msec */

/* Command to unlock bootloader */
#define ATMXT140_UNLOCK_CMD_MSB		0xaa
#define ATMXT140_UNLOCK_CMD_LSB		0xdc

/* Bootloader mode status */
#define ATMXT140_WAITING_BOOTLOAD_CMD	0xc0	/* valid 7 6 bit only */
#define ATMXT140_WAITING_FRAME_DATA	0x80	/* valid 7 6 bit only */
#define ATMXT140_FRAME_CRC_CHECK	0x02
#define ATMXT140_FRAME_CRC_FAIL		0x03
#define ATMXT140_FRAME_CRC_PASS		0x04
#define ATMXT140_APP_CRC_FAIL		0x40	/* valid 7 8 bit only */
#define ATMXT140_BOOT_STATUS_MASK	0x3f

/* Touch status */
#define ATMXT140_SUPPRESS		(1 << 1)
#define ATMXT140_AMP			(1 << 2)
#define ATMXT140_VECTOR			(1 << 3)
#define ATMXT140_MOVE			(1 << 4)
#define ATMXT140_RELEASE		(1 << 5)
#define ATMXT140_PRESS			(1 << 6)
#define ATMXT140_DETECT			(1 << 7)

/* Touchscreen absolute values */
#define ATMXT140_MAX_XC			0x3ff
#define ATMXT140_MAX_YC			0x3ff
#define ATMXT140_MAX_AREA		0xff

#define ATMXT140_MAX_FINGER		10

/* Initial register values recommended from chip vendor */
static const u8 init_vals_ver_20[] = {
	/* ATMXT140_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_GEN_POWER(7) */
	0x20, 0xff, 0x32,
	/* ATMXT140_GEN_ACQUIRE(8) */
	0x08, 0x05, 0x05, 0x00, 0x00, 0x00, 0x05, 0x14,
	/* ATMXT140_TOUCH_MULTI(9) */
	0x00, 0x00, 0x00, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x02, 0x00,
	0x00, 0x01, 0x01, 0x0e, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x64,
	/* ATMXT140_TOUCH_KEYARRAY(15) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* ATMXT140_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
	/* ATMXT140_PROCI_GRIPFACE(20) */
	0x00, 0x64, 0x64, 0x64, 0x64, 0x00, 0x00, 0x1e, 0x14, 0x04,
	0x1e, 0x00,
	/* ATMXT140_PROCG_NOISE(22) */
	0x05, 0x00, 0x00, 0x19, 0x00, 0xe7, 0xff, 0x04, 0x32, 0x00,
	0x01, 0x0a, 0x0f, 0x14, 0x00, 0x00, 0xe8,
	/* ATMXT140_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	/* ATMXT140_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_PROCI_TWOTOUCH(27) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x00, 0x04, 0x08,
};

static const u8 init_vals_ver_21[] = {
	/* ATMXT140_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_GEN_POWER(7) */
	0x20, 0xff, 0x32,
	/* ATMXT140_GEN_ACQUIRE(8) */
	0x06, 0x00, 0x0a, 0x0a, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00,
	/* ATMXT140_TOUCH_MULTI(9) */
	0x83, 0x00, 0x02, 0x0f, 0x09, 0x00, 0x10, 0x2a, 0x02, 0x01,
	0x00, 0x06, 0x02, 0x00, 0x05, 0x0a, 0x0a, 0x0a, 0x07, 0x02,
	0xdf, 0x01, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x40, 0x00,
	0x0f, 0x05,
	/* ATMXT140_TOUCH_KEYARRAY(15) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* ATMXT140_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_PROCI_GRIPFACE(20) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x28, 0x04,
	0x0f, 0x0a,
	/* ATMXT140_PROCG_NOISE(22) */
	0x0d, 0x00, 0x00, 0x20, 0x00, 0xe0, 0xff, 0x08, 0x20, 0x00,
	0x00, 0x10, 0x1f, 0x39, 0x41, 0x0, 0x08,
	/* ATMXT140_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,

	/* ATMXT140_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x02, 0x08, 0x08, 0x1e,
};

static const u8 init_vals_ver_22[] = {
	/* ATMXT140_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_GEN_POWER(7) */
	0x20, 0x10, 0x32,
	/* ATMXT140_GEN_ACQUIRE(8) */
	0x0a, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x23,
	/* ATMXT140_TOUCH_MULTI(9) */
	0x00, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00,
	0x00, 0x01, 0x01, 0x0e, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* ATMXT140_TOUCH_KEYARRAY(15) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* ATMXT140_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_PROCI_GRIPFACE(20) */
	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x28, 0x04,
	0x0f, 0x0a,
	/* ATMXT140_PROCG_NOISE(22) */
	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x23, 0x00,
	0x00, 0x05, 0x0f, 0x19, 0x23, 0x2d, 0x03,
	/* ATMXT140_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_PROCI_TWOTOUCH(27) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* ATMXT140_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x00, 0x08, 0x10, 0x00,
};

struct atmxt140_info {
	u8 family_id;
	u8 variant_id;
	u8 version;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 object_num;
};

struct atmxt140_object {
	u8 type;
	u16 start_address;
	u8 size;
	u8 instances;
	u8 num_report_ids;

	/* to map object and message */
	u8 max_reportid;
};

struct atmxt140_message {
	u8 reportid;
	u8 message[7];
	u8 checksum;
};

struct atmxt140_finger {
	int status;
	int x;
	int y;
	int area;
};

/* Each client has this additional data */
struct atmxt140_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	const struct atmxt140_platform_data *pdata;
	struct atmxt140_object *object_table;
	struct atmxt140_info info;
	struct atmxt140_finger finger[ATMXT140_MAX_FINGER];
	unsigned int irq;
	int pen_down;
	struct early_suspend suspend_desc;

#ifdef USE_BRCM_WORK_QUEUE
	struct delayed_work work;
#endif

};

static bool atmxt140_object_readable(unsigned int type)
{
	switch (type) {
	case ATMXT140_GEN_MESSAGE:
	case ATMXT140_GEN_COMMAND:
	case ATMXT140_GEN_POWER:
	case ATMXT140_GEN_ACQUIRE:
	case ATMXT140_TOUCH_MULTI:
	case ATMXT140_TOUCH_KEYARRAY:
	case ATMXT140_TOUCH_PROXIMITY:
	case ATMXT140_PROCI_GRIPFACE:
	case ATMXT140_PROCG_NOISE:
	case ATMXT140_PROCI_ONETOUCH:
	/*case ATMXT140_PROCI_TWOTOUCH:*/
	case ATMXT140_SPT_COMMSCONFIG:
	case ATMXT140_SPT_GPIOPWM:
	case ATMXT140_SPT_SELFTEST:
	case ATMXT140_SPT_CTECONFIG:
	case ATMXT140_SPT_USERDATA:
		return true;
	default:
		return false;
	}
}

static bool atmxt140_object_writable(unsigned int type)
{
	switch (type) {
	case ATMXT140_GEN_COMMAND:
	case ATMXT140_GEN_POWER:
	case ATMXT140_GEN_ACQUIRE:
	case ATMXT140_TOUCH_MULTI:
	case ATMXT140_TOUCH_KEYARRAY:
	case ATMXT140_TOUCH_PROXIMITY:
	case ATMXT140_PROCI_GRIPFACE:
	case ATMXT140_PROCG_NOISE:
	case ATMXT140_PROCI_ONETOUCH:
	/*case ATMXT140_PROCI_TWOTOUCH:*/
	case ATMXT140_SPT_GPIOPWM:
	case ATMXT140_SPT_SELFTEST:
	case ATMXT140_SPT_CTECONFIG:
		return true;
	default:
		return false;
	}
}

static void atmxt140_dump_message(struct device *dev,
				  struct atmxt140_message *message)
{
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

static int atmxt140_check_bootloader(struct i2c_client *client,
				     unsigned int state)
{
	u8 val;

recheck:
	if (i2c_master_recv(client, &val, 1) != 1) {
		dev_err(&client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	switch (state) {
	case ATMXT140_WAITING_BOOTLOAD_CMD:
	case ATMXT140_WAITING_FRAME_DATA:
		val &= ~ATMXT140_BOOT_STATUS_MASK;
		break;
	case ATMXT140_FRAME_CRC_PASS:
		if (val == ATMXT140_FRAME_CRC_CHECK)
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

static int atmxt140_unlock_bootloader(struct i2c_client *client)
{
	u8 buf[2];

	buf[0] = ATMXT140_UNLOCK_CMD_LSB;
	buf[1] = ATMXT140_UNLOCK_CMD_MSB;

	if (i2c_master_send(client, buf, 2) != 2) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int atmxt140_fw_write(struct i2c_client *client,
			     const u8 * data, unsigned int frame_size)
{
	if (i2c_master_send(client, data, frame_size) != frame_size) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int __atmxt140_read_reg(struct i2c_client *client,
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

static int atmxt140_read_reg(struct i2c_client *client, u16 reg, u8 * val)
{
	return __atmxt140_read_reg(client, reg, 1, val);
}

static int atmxt140_write_reg(struct i2c_client *client, u16 reg, u8 val)
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

static int atmxt140_read_object_table(struct i2c_client *client,
				      u16 reg, u8 * object_buf)
{
	return __atmxt140_read_reg(client, reg, ATMXT140_OBJECT_SIZE,
				   object_buf);
}

static struct atmxt140_object *atmxt140_get_object(struct atmxt140_data *data,
						   u8 type)
{
	struct atmxt140_object *object;
	int i;

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;
		if (object->type == type)
			return object;
	}

	dev_err(&data->client->dev, "Invalid object type\n");
	return NULL;
}

static int atmxt140_read_message(struct atmxt140_data *data,
				 struct atmxt140_message *message)
{
	struct atmxt140_object *object;
	u16 reg;

	object = atmxt140_get_object(data, ATMXT140_GEN_MESSAGE);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return __atmxt140_read_reg(data->client, reg,
				   sizeof(struct atmxt140_message), message);
}

static int atmxt140_read_object(struct atmxt140_data *data,
				u8 type, u8 offset, u8 * val)
{
	struct atmxt140_object *object;
	u16 reg;

	object = atmxt140_get_object(data, type);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return __atmxt140_read_reg(data->client, reg + offset, 1, val);
}

static int atmxt140_write_object(struct atmxt140_data *data,
				 u8 type, u8 offset, u8 val)
{
	struct atmxt140_object *object;
	u16 reg;

	object = atmxt140_get_object(data, type);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return atmxt140_write_reg(data->client, reg + offset, val);
}

static void atmxt140_input_report(struct atmxt140_data *data, int single_id)
{
	struct atmxt140_finger *finger = data->finger;
	struct input_dev *input_dev = data->input_dev;
	int status = finger[single_id].status;
	int finger_num = 0;
	int id;

#if defined (CONFIG_TOUCHSCREEN_ATMXT140_MULTITOUCH)
	for (id = 0; id < ATMXT140_MAX_FINGER; id++) {
		if (!finger[id].status)
			continue;
		input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR,
				 finger[id].status != ATMXT140_RELEASE ?
				 finger[id].area : 0);
		input_report_abs(input_dev, ABS_MT_POSITION_X, finger[id].x);
		input_report_abs(input_dev, ABS_MT_POSITION_Y, finger[id].y);
		input_mt_sync(input_dev);

		if (finger[id].status == ATMXT140_RELEASE)
			finger[id].status = 0;
		else
			finger_num++;
	}

#else
	if (status != ATMXT140_RELEASE)
		finger_num = 1;
#endif
	input_report_key(input_dev, BTN_TOUCH, finger_num > 0);

	if (status != ATMXT140_RELEASE) {
		data->pen_down = 1;
		input_report_abs(input_dev, ABS_X, finger[single_id].x);
		input_report_abs(input_dev, ABS_Y, finger[single_id].y);
	} else {
		data->pen_down = 0;
	}

	input_sync(input_dev);
}

static void atmxt140_input_touchevent(struct atmxt140_data *data,
				      struct atmxt140_message *message, int id)
{
	struct atmxt140_finger *finger = data->finger;
	struct device *dev = &data->client->dev;
	u8 status = message->message[0];
	int x;
	int y;
	int area;

	/* Check the touch is present on the screen */
	if (!(status & ATMXT140_DETECT)) {
		if (status & ATMXT140_RELEASE) {
			dev_dbg(dev, "[%d] released\n", id);

			finger[id].status = ATMXT140_RELEASE;
			atmxt140_input_report(data, id);
		}
		return;
	}

	/* Check only AMP detection */
	if (!(status & (ATMXT140_PRESS | ATMXT140_MOVE)))
		return;

	x = (message->message[1] << 2) | ((message->message[3] & ~0x3f) >> 6);
	y = (message->message[2] << 2) | ((message->message[3] & ~0xf3) >> 2);
	area = message->message[4];

	dev_dbg(dev, "[%d] %s x: %d, y: %d, area: %d\n", id,
		status & ATMXT140_MOVE ? "moved" : "pressed", x, y, area);

	finger[id].status = status & ATMXT140_MOVE ?
	    ATMXT140_MOVE : ATMXT140_PRESS;

#if defined(CONFIG_ENABLE_HVGA) && defined(CONFIG_BCM_LCD_NT35582)
	/*HVGA on WVGA panel*/
	finger[id].x = (x - 40 - 1024 * (480 - 320) / 2 / 480) * 480 / 320;
	finger[id].y = (y - 40 - 1024 * (800 - 480) / 2 / 800) * 800 / 480;
#elif defined(CONFIG_ENABLE_QVGA)
#if defined(CONFIG_BCM_LCD_R61581)
	/* QVGA on HVGA panel*/
	finger[id].x = (x - 40 - 1024 * (320 - 240) / 2 / 320) * 320 / 240;
	finger[id].y = (y - 40 - 1024 * (480 - 320) / 2 / 480) * 480 / 320;
#elif defined(CONFIG_BCM_LCD_NT35582)
	/*QVGA on HVGA panel*/
	finger[id].x = (x - 40 - 1024 * (480 - 240) / 2 / 480) * 480 / 240;
	finger[id].y = (y - 40 - 1024 * (800 - 320) / 2 / 800) * 800 / 320;
#endif
#else
	#if defined (CONFIG_TOUCHSCREEN_ATMXT140_MULTITOUCH)

		finger[id].x = (x - 10 ) * 1024 / (1024 - 10);
		finger[id].y = (y - 50) * 1024 / (1024 - 50);
	#else
		finger[id].x = (x + 50) * 1024 / (1024 + 50);
		finger[id].y = (y + 50) * 1024 / (1024 + 50);
	#endif
#endif
	finger[id].area = area;

	atmxt140_input_report(data, id);
}

#ifndef USE_BRCM_WORK_QUEUE
static irqreturn_t atmxt140_interrupt(int irq, void *dev_id)
{
	struct atmxt140_data *data = dev_id;
	struct atmxt140_message message;
	struct atmxt140_object *object;
	struct device *dev = &data->client->dev;
	int id;
	u8 reportid;
	u8 max_reportid;
	u8 min_reportid;

	do {
		if (atmxt140_read_message(data, &message)) {
			dev_err(dev, "Failed to read message\n");
			goto end;
		}

		reportid = message.reportid;

		/* whether reportid is thing of ATMXT140_TOUCH_MULTI */
		object = atmxt140_get_object(data, ATMXT140_TOUCH_MULTI);
		if (!object)
			goto end;

		max_reportid = object->max_reportid;
		min_reportid = max_reportid - object->num_report_ids + 1;
		id = reportid - min_reportid;

		if (reportid >= min_reportid && reportid <= max_reportid)
			atmxt140_input_touchevent(data, &message, id);
		else
			atmxt140_dump_message(dev, &message);
	} while (reportid != 0xff);

end:
	return IRQ_HANDLED;
}
#endif

#ifdef USE_BRCM_WORK_QUEUE

#define TS_POLL_DELAY	(0)	/* ms delay between irq & sample */
#define to_delayed_work(_work) container_of(_work, struct delayed_work, work)

static void atmxt140_read_input(struct ATMXT140_data *data);

static void atmxt140_touch_work(struct work_struct *work)
{
	struct atmxt140_data *p_data =
	    container_of(to_delayed_work(work), struct atmxt140_data, work);

	/* Peform the actual read and send the events to input sub-system */
	atmxt140_read_input(p_data);

	/* Re-enable the interrupts from the device */
	enable_irq(p_data->irq);
}

static void atmxt140_read_input(struct atmxt140_data *data)
{
	/* Implement what is implemented in the threaded IRQ implementation of the original driver */
	struct atmxt140_message message;
	struct atmxt140_object *object;
	struct device *dev = &data->client->dev;
	int id;
	u8 reportid;
	u8 max_reportid;
	u8 min_reportid;

	do {
		if (atmxt140_read_message(data, &message)) {
			dev_err(dev, "Failed to read message\n");
			goto end;
		}
		reportid = message.reportid;

		/* whether reportid is thing of ATMXT140_TOUCH_MULTI */
		object = atmxt140_get_object(data, ATMXT140_TOUCH_MULTI);
		if (!object)
			goto end;

		max_reportid = object->max_reportid;
		min_reportid = max_reportid - object->num_report_ids + 1;
		id = reportid - min_reportid;

		if (reportid >= min_reportid && reportid <= max_reportid)
			atmxt140_input_touchevent(data, &message, id);
		else {
			atmxt140_dump_message(dev, &message);
		}
	} while (reportid != 0xff);
end:
	return;
}

static irqreturn_t atmxt140_touch_interrupt(int irq, void *dev_id)
{
	struct atmxt140_data *p_data = dev_id;

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
	schedule_delayed_work(&p_data->work, msecs_to_jiffies(TS_POLL_DELAY));
	return IRQ_HANDLED;
}
#endif

static int atmxt140_check_reg_init(struct atmxt140_data *data)
{
	struct atmxt140_object *object;
	struct device *dev = &data->client->dev;
	int index = 0;
	int i, j;
	u8 version = data->info.version;
	u8 *init_vals;

	switch (version) {
	case ATMXT140_VER_20:
		init_vals = (u8 *) init_vals_ver_20;
		break;
	case ATMXT140_VER_21:
		init_vals = (u8 *) init_vals_ver_21;
		break;
	case ATMXT140_VER_22:
		init_vals = (u8 *) init_vals_ver_22;
		break;
	default:
		dev_err(dev, "Firmware version %d doesn't support\n", version);
		return -EINVAL;
	}

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;

		if (!atmxt140_object_writable(object->type))
			continue;

		for (j = 0; j < object->size + 1; j++)
			atmxt140_write_object(data, object->type, j,
					      init_vals[index + j]);

		index += object->size + 1;
	}

	return 0;
}

static int atmxt140_check_matrix_size(struct atmxt140_data *data)
{
	const struct atmxt140_platform_data *pdata = data->pdata;
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

	error = atmxt140_read_object(data, ATMXT140_SPT_CTECONFIG,
				     ATMXT140_CTE_MODE, &val);
	if (error)
		return error;

	if (mode == val)
		return 0;

	/* Change the CTE configuration */
	atmxt140_write_object(data, ATMXT140_SPT_CTECONFIG,
			      ATMXT140_CTE_CTRL, 1);
	atmxt140_write_object(data, ATMXT140_SPT_CTECONFIG,
			      ATMXT140_CTE_MODE, mode);
	atmxt140_write_object(data, ATMXT140_SPT_CTECONFIG,
			      ATMXT140_CTE_CTRL, 0);

	return 0;
}

static int atmxt140_make_highchg(struct atmxt140_data *data)
{
	struct device *dev = &data->client->dev;
	struct atmxt140_message message;
	int count = 10;
	int error;

	/* Read dummy message to make high CHG pin */
	do {
		error =
		    atmxt140_read_message(data, &message);
		if (error)
			return error;
	} while (message.reportid != 0xff && --count);

	if (!count) {
		dev_err(dev, "CHG pin isn't cleared\n");
		return -EBUSY;
	}

	return 0;
}

static void atmxt140_handle_pdata(struct atmxt140_data *data)
{
	const struct atmxt140_platform_data *pdata = data->pdata;
	u8 voltage;

	/* Set touchscreen lines */
	atmxt140_write_object(data, ATMXT140_TOUCH_MULTI, ATMXT140_TOUCH_XSIZE,
			      pdata->x_line);
	atmxt140_write_object(data, ATMXT140_TOUCH_MULTI, ATMXT140_TOUCH_YSIZE,
			      pdata->y_line);

	/* Set touchscreen orient */
	atmxt140_write_object(data, ATMXT140_TOUCH_MULTI, ATMXT140_TOUCH_ORIENT,
			      pdata->orient);

	/* Set touchscreen burst length */
	atmxt140_write_object(data, ATMXT140_TOUCH_MULTI,
			      ATMXT140_TOUCH_BLEN, pdata->blen);

	/* Set touchscreen threshold */
	atmxt140_write_object(data, ATMXT140_TOUCH_MULTI,
			      ATMXT140_TOUCH_TCHTHR, pdata->threshold);

	/* Set touchscreen resolution */
	atmxt140_write_object(data, ATMXT140_TOUCH_MULTI,
			      ATMXT140_TOUCH_XRANGE_LSB,
			      (pdata->x_size - 1) & 0xff);
	atmxt140_write_object(data, ATMXT140_TOUCH_MULTI,
			      ATMXT140_TOUCH_XRANGE_MSB,
			      (pdata->x_size - 1) >> 8);
	atmxt140_write_object(data, ATMXT140_TOUCH_MULTI,
			      ATMXT140_TOUCH_YRANGE_LSB,
			      (pdata->y_size - 1) & 0xff);
	atmxt140_write_object(data, ATMXT140_TOUCH_MULTI,
			      ATMXT140_TOUCH_YRANGE_MSB,
			      (pdata->y_size - 1) >> 8);

	/* Set touchscreen voltage */
	if (data->info.version >= ATMXT140_VER_21 && pdata->voltage) {
		if (pdata->voltage < ATMXT140_VOLTAGE_DEFAULT) {
			voltage = (ATMXT140_VOLTAGE_DEFAULT - pdata->voltage) /
			    ATMXT140_VOLTAGE_STEP;
			voltage = 0xff - voltage + 1;
		} else
			voltage = (pdata->voltage - ATMXT140_VOLTAGE_DEFAULT) /
			    ATMXT140_VOLTAGE_STEP;

		atmxt140_write_object(data, ATMXT140_SPT_CTECONFIG,
				      ATMXT140_CTE_VOLTAGE, voltage);
	}
}

static int atmxt140_get_info(struct atmxt140_data *data)
{
	struct i2c_client *client = data->client;
	struct atmxt140_info *info = &data->info;
	int error;
	u8 val;

	error = atmxt140_read_reg(client, ATMXT140_FAMILY_ID, &val);
	if (error)
		return error;
	info->family_id = val;

	error = atmxt140_read_reg(client, ATMXT140_VARIANT_ID, &val);
	if (error)
		return error;
	info->variant_id = val;

	error = atmxt140_read_reg(client, ATMXT140_VERSION, &val);
	if (error)
		return error;
	info->version = val;

	error = atmxt140_read_reg(client, ATMXT140_BUILD, &val);
	if (error)
		return error;
	info->build = val;

	error = atmxt140_read_reg(client, ATMXT140_OBJECT_NUM, &val);
	if (error)
		return error;
	info->object_num = val;

	return 0;
}

static int atmxt140_get_object_table(struct atmxt140_data *data)
{
	int error;
	int i;
	u16 reg;
	u8 reportid = 0;
	u8 buf[ATMXT140_OBJECT_SIZE];

	for (i = 0; i < data->info.object_num; i++) {
		struct atmxt140_object *object = data->object_table + i;

		reg = ATMXT140_OBJECT_START + ATMXT140_OBJECT_SIZE * i;
		error = atmxt140_read_object_table(data->client, reg, buf);
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
	}
	return 0;
}

static int atmxt140_initialize(struct atmxt140_data *data)
{
	struct i2c_client *client = data->client;
	struct atmxt140_info *info = &data->info;
	int error;
	u8 val;

	error = atmxt140_get_info(data);
	if (error)
		return error;

	data->object_table = kcalloc(info->object_num,
				     sizeof(struct atmxt140_data), GFP_KERNEL);
	if (!data->object_table) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	/* Get object table information */
	error = atmxt140_get_object_table(data);
	if (error)
		return error;

	/* Check register init values */
	error = atmxt140_check_reg_init(data);
	if (error)
		return error;

	/* Check X/Y matrix size */
	error = atmxt140_check_matrix_size(data);
	if (error)
		return error;


	atmxt140_handle_pdata(data);

	/* Backup to memory */
	atmxt140_write_object(data, ATMXT140_GEN_COMMAND,
			      ATMXT140_COMMAND_BACKUPNV, ATMXT140_BACKUP_VALUE);
	msleep(ATMXT140_BACKUP_TIME);

	/* Soft reset */
	atmxt140_write_object(data, ATMXT140_GEN_COMMAND,
			      ATMXT140_COMMAND_RESET, 1);
	msleep(ATMXT140_RESET_TIME);

	/* Update matrix size at info struct */
	error = atmxt140_read_reg(client, ATMXT140_MATRIX_X_SIZE, &val);
	if (error)
		return error;
	info->matrix_xsize = val;

	error = atmxt140_read_reg(client, ATMXT140_MATRIX_Y_SIZE, &val);
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

static ssize_t atmxt140_object_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct atmxt140_data *data = dev_get_drvdata(dev);
	struct atmxt140_object *object;
	int count = 0;
	int i, j;
	int error;
	u8 val;

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;

		count += sprintf(buf + count,
				 "Object Table Element %d(Type %d)\n",
				 i + 1, object->type);

		if (!atmxt140_object_readable(object->type)) {
			count += sprintf(buf + count, "\n");
			continue;
		}

		for (j = 0; j < object->size + 1; j++) {
			error = atmxt140_read_object(data,
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

static int atmxt140_load_fw(struct device *dev, const char *fn)
{
	struct atmxt140_data *data = dev_get_drvdata(dev);
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
	atmxt140_write_object(data, ATMXT140_GEN_COMMAND,
			      ATMXT140_COMMAND_RESET, ATMXT140_BOOT_VALUE);
	msleep(ATMXT140_RESET_TIME);

	/* Change to slave address of bootloader */
	if (client->addr == ATMXT140_APP_LOW)
		client->addr = ATMXT140_BOOT_LOW;
	else
		client->addr = ATMXT140_BOOT_HIGH;

	ret = atmxt140_check_bootloader(client, ATMXT140_WAITING_BOOTLOAD_CMD);
	if (ret)
		goto out;

	/* Unlock bootloader */
	atmxt140_unlock_bootloader(client);

	while (pos < fw->size) {
		ret = atmxt140_check_bootloader(client,
						ATMXT140_WAITING_FRAME_DATA);
		if (ret)
			goto out;

		frame_size = ((*(fw->data + pos) << 8) | *(fw->data + pos + 1));

		/* We should add 2 at frame size as the the firmware data is not
		 * included the CRC bytes.
		 */
		frame_size += 2;

		/* Write one frame to device */
		atmxt140_fw_write(client, fw->data + pos, frame_size);

		ret = atmxt140_check_bootloader(client,
						ATMXT140_FRAME_CRC_PASS);
		if (ret)
			goto out;

		pos += frame_size;

		dev_dbg(dev, "Updated %d bytes / %zd bytes\n", pos, fw->size);
	}

out:
	release_firmware(fw);

	/* Change to slave address of application */
	if (client->addr == ATMXT140_BOOT_LOW)
		client->addr = ATMXT140_APP_LOW;
	else
		client->addr = ATMXT140_APP_HIGH;

	return ret;
}

static ssize_t atmxt140_update_fw_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct atmxt140_data *data = dev_get_drvdata(dev);
	unsigned int version;
	int error;

	if (sscanf(buf, "%u", &version) != 1) {
		dev_err(dev, "Invalid values\n");
		return -EINVAL;
	}

	if (data->info.version < ATMXT140_VER_21 || version < ATMXT140_VER_21) {
		dev_err(dev, "FW update supported starting with version 21\n");
		return -EINVAL;
	}

	disable_irq(data->irq);

	error = atmxt140_load_fw(dev, ATMXT140_FW_NAME);
	if (error) {
		dev_err(dev, "The firmware update failed(%d)\n", error);
		count = error;
	} else {
		dev_dbg(dev, "The firmware update succeeded\n");

		/* Wait for reset */
		msleep(ATMXT140_FWRESET_TIME);

		kfree(data->object_table);
		data->object_table = NULL;

		atmxt140_initialize(data);
	}

	enable_irq(data->irq);

	return count;
}

static DEVICE_ATTR(object, 0444, atmxt140_object_show, NULL);
static DEVICE_ATTR(update_fw, 0664, NULL, atmxt140_update_fw_store);

#if defined(DEBUG_PEN_STATUS)
static ssize_t pen_status_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct atmxt140_data *p_data =
	    (struct atmxt140_data *)dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", p_data->pen_down);
}

static DEVICE_ATTR(pen_status, 0644, pen_status_show, NULL);
#endif

static struct attribute *atmxt140_attrs[] = {
	&dev_attr_object.attr,
	&dev_attr_update_fw.attr,
#if defined(DEBUG_PEN_STATUS)
	&dev_attr_pen_status.attr,
#endif
	NULL
};

static const struct attribute_group atmxt140_attr_group = {
	.attrs = atmxt140_attrs,
};

static void atmxt140_start(struct atmxt140_data *data)
{
	/* Touch enable */

	/*
	 * Writing 0x03 does not enable the SCANEN bit.
	 * which is optimized for not notifying the Host with unnecessary
	 * messages.
	 * The data sheet says that, if there are some issues in detecting
	 * multiple touches try, enabling this bit i.e write 0x83
	 */
	atmxt140_write_object(data,
			      ATMXT140_TOUCH_MULTI, ATMXT140_TOUCH_CTRL, 0x83);
}

static void atmxt140_stop(struct atmxt140_data *data)
{
	/* Touch disable */
	atmxt140_write_object(data,
			      ATMXT140_TOUCH_MULTI, ATMXT140_TOUCH_CTRL, 0);
}

static int atmxt140_input_open(struct input_dev *dev)
{
	struct atmxt140_data *data = input_get_drvdata(dev);

	atmxt140_start(data);

	return 0;
}

static void atmxt140_input_close(struct input_dev *dev)
{
	struct atmxt140_data *data = input_get_drvdata(dev);

	atmxt140_stop(data);
}

#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
static int atmxt140_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct atmxt140_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev = data->input_dev;

	mutex_lock(&input_dev->mutex);

	if (input_dev->users) {
		atmxt140_stop(data);

		atmxt140_write_object(data, ATMXT140_GEN_POWER,
				ATMXT140_POWER_IDLEACQINT, 0);
		atmxt140_write_object(data, ATMXT140_GEN_POWER,
				ATMXT140_POWER_ACTVACQINT, 0);
	}
	mutex_unlock(&input_dev->mutex);

	return 0;
}

static int atmxt140_resume(struct i2c_client *client)
{
	struct atmxt140_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev = data->input_dev;

	mutex_lock(&input_dev->mutex);

	if (input_dev->users) {
		atmxt140_write_object(data, ATMXT140_GEN_POWER,
				ATMXT140_POWER_IDLEACQINT, init_vals_ver_22[14]);
		atmxt140_write_object(data, ATMXT140_GEN_POWER,
				ATMXT140_POWER_ACTVACQINT, init_vals_ver_22[15]);
		atmxt140_start(data);

	}

	mutex_unlock(&input_dev->mutex);

	return 0;
}
#else
#define atmxt140_suspend	NULL
#define atmxt140_resume		NULL
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void atmxt140_early_suspend(struct early_suspend *desc)
{
	struct atmxt140_data *data = container_of(desc, struct atmxt140_data, suspend_desc);
	pm_message_t mesg = {
		.event = PM_EVENT_SUSPEND,
		};
	atmxt140_suspend(data->client, mesg);
}

static void atmxt140_late_resume(struct early_suspend *desc)
{
	struct atmxt140_data *data = container_of(desc, struct atmxt140_data, suspend_desc);
	atmxt140_resume(data->client);
}
#endif

static int __devinit atmxt140_probe(struct i2c_client *client,
				    const struct i2c_device_id *id)
{
	struct atmxt140_data *data;
	struct input_dev *input_dev;
	int error;

	if (!client->dev.platform_data)
		return -EINVAL;

	data = kzalloc(sizeof(struct atmxt140_data), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!data || !input_dev) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	input_dev->name = "MXT140 Touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	input_dev->open = atmxt140_input_open;
	input_dev->close = atmxt140_input_close;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);

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

	i2c_set_clientdata(client, data);

	if (data->pdata->platform_init)
		data->pdata->platform_init();

	error = atmxt140_initialize(data);
	if (error)
		goto err_free_object;

	data->pen_down = 0;

#ifdef USE_BRCM_WORK_QUEUE
	INIT_DELAYED_WORK(&data->work, atmxt140_touch_work);

	error = request_irq(client->irq, atmxt140_touch_interrupt,
			    IRQF_TRIGGER_FALLING, "mxt140_ts", data);

	if (error < 0) {
		printk("atmxt140_touch: Failed to register interrupt \r\n");
		goto err_free_object;
	}

	printk("atmxt140_touch: registered the interrupt \r\n");
#else
	error = request_threaded_irq(client->irq, NULL, atmxt140_interrupt,
				     IRQF_TRIGGER_FALLING,
				     client->dev.driver->name, data);
	if (error) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_free_object;
	}
#endif

	error = atmxt140_make_highchg(data);
	if (error)
		return error;

	error = input_register_device(input_dev);
	if (error)
		goto err_free_irq;

	error = sysfs_create_group(&client->dev.kobj, &atmxt140_attr_group);
	if (error)
		goto err_unregister_device;

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
	data->suspend_desc.suspend = atmxt140_early_suspend,
	data->suspend_desc.resume = atmxt140_late_resume,
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

static int __devexit atmxt140_remove(struct i2c_client *client)
{
	struct atmxt140_data *data = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &atmxt140_attr_group);
	free_irq(data->irq, data);
	input_unregister_device(data->input_dev);
	kfree(data->object_table);
	kfree(data);

	return 0;
}

void atmxt140_shutdown(struct i2c_client *client)
{
	struct atmxt140_data *data = i2c_get_clientdata(client);
	/* Soft reset  controller here to prevent it from
	 *getting into an un-responsive state */
	atmxt140_write_object(data, ATMXT140_GEN_COMMAND,
			      ATMXT140_COMMAND_RESET, 1);
	msleep(ATMXT140_RESET_TIME);

}

static const struct i2c_device_id atmxt140_id[] = {
	{"mxt140_ts", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, atmxt140_id);

static struct i2c_driver atmxt140_driver = {
	.driver = {
		   .name = "mxt140_ts",
		   .owner = THIS_MODULE,
		   },
	.probe = atmxt140_probe,
	.remove = __devexit_p(atmxt140_remove),
	.shutdown = atmxt140_shutdown,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = atmxt140_suspend,
	.resume = atmxt140_resume,
#endif
	.id_table = atmxt140_id,
};

static int __init atmxt140_init(void)
{
	return i2c_add_driver(&atmxt140_driver);
}

static void __exit atmxt140_exit(void)
{
	i2c_del_driver(&atmxt140_driver);
}

module_init(atmxt140_init);
module_exit(atmxt140_exit);

/* Module information */
MODULE_AUTHOR("Joonyoung Shim <jy0922.shim@samsung.com>");
MODULE_DESCRIPTION("AT42ATMXT140/ATMXT224 Touchscreen driver");
MODULE_LICENSE("GPL");
