/*
 * Gadget Driver for Android ADB
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
* 
* 	@file	drivers/usb/gadget/android.h
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
/**
	@file
	@brief Gadget Driver for Android ADB
	@defgroup USBGadgetDriverADB Gadget Driver for Android ADB
*/

#ifndef __F_ANDROID_H
#define __F_ANDROID_H

/** @addtogroup USBGadgetDriverADB
	@{
*/

/**
 * Android_switch_usb_conf() - switch the USB configuration
 * @param new_usb_conf: The value of the USB configuration
 *
 */
int Android_switch_usb_conf (unsigned char new_usb_conf);

/** @} */

#endif /* __ANDROID_H */
