/*
 * Header file for:
 * Cypress TrueTouch(TM) Standard Product (TTSP) touchscreen drivers.
 * For use with Cypress Txx3xx parts.
 * Supported parts include:
 * CY8CTST341
 * CY8CTMA340
 *
 * Copyright (C) 2009, 2010, 2011 Cypress Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contact Cypress Semiconductor at www.cypress.com <kev@cypress.com>
 *
 */


#ifndef __CYTTSP_CORE_H__
#define __CYTTSP_CORE_H__

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/input/cyttsp.h>

#define CY_NUM_RETRY                4 /* max number of retries for read ops */


struct cyttsp_bus_ops {
	s32 (*write)(void *handle, u8 addr, u8 length, const void *values);
	s32 (*read)(void *handle, u8 addr, u8 length, void *values);
	s32 (*ext)(void *handle, void *values);
	struct device *dev;
};

void *cyttsp_core_init(struct cyttsp_bus_ops *bus_ops, struct device *dev);

void cyttsp_core_release(void *handle);
#ifdef CONFIG_PM
int cyttsp_resume(void *handle);
int cyttsp_suspend(void *handle);
#endif

#endif /* __CYTTSP_CORE_H__ */
