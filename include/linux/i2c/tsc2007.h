/*******************************************************************************
* Copyright 2010 Broadcom Corporation. All rights reserved.
*
*	@file	include/linux/i2c/tsc2007.h
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

#ifndef __LINUX_I2C_TSC2007_H
#define __LINUX_I2C_TSC2007_H

/* linux/i2c/tsc2007.h */

#include <plat/bcm_i2c.h>

struct tsc2007_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	u16 model;		/* 2007. */
	u16 x_plate_ohms;
/* The X and Y values are added for Broadcom specific
 * calculations. These values needs to updated in board-<name>.c,
 * according to display screen size and touch screen valid contact point
 * for the same.
 */
	u16 validx_left;
	u16 validy_up;
	u16 validx_right;
	u16 validy_down;
	int (*get_pendown_state) (void);
	void (*clear_penirq) (void);	/* If needed, clear 2nd level
					   interrupt source */
	int (*init_platform_hw) (void);
	void (*exit_platform_hw) (void);

/* Returning the x, y values really depends on how the screen is mounted.
 * The upper layers should not be bothered about it. Hence the board
 * specific code should return x,y values which is knowlegable about
 * the screen orientation.
 */
	int (*get_x_value) (struct tsc2007_platform_data *, int, u32);
	int (*get_y_value) (struct tsc2007_platform_data *, int, u32);
};

#endif
