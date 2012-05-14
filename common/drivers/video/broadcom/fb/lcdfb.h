/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/broadcom/fb/lcdfb.h
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

/*
*
*****************************************************************************
*
*  lcdfb.h
*
*  PURPOSE:
*
*     Global definitions for the CD used in the OnePhone
*
*  NOTES:
*
*****************************************************************************/

#if !defined(LCDFB_H)
#define LCDFB_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/fb.h>

/* ---- Constants and Types ---------------------------------------------- */

#define LCDFB_NAME    "LCDfb"
#define MODULE_NAME   "LCDfb"

/* Indicies into fb_info.rgb array */

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

int __init lcdfb_init(void);

#endif /* LCDFB_H */
