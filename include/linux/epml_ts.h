/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	include/linux/epml_ts.h
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

#ifndef _LINUX_EPML_H
#define _LINUX_EPML_H
#include <plat/bcm_i2c.h>

struct epml_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	void (*reset) (void);
};

#endif /* _LINUX_EPML_H */
