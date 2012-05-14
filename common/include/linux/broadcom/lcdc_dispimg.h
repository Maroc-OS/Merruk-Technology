/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	 	include/linux/broadcom/lcdc_dispimg.h
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

#ifndef __LCDC_DISPIMG_H
#define __LCDC_DISPIMG_H

#ifdef CONFIG_BRCM_KPANIC_UI_IND

#define IMG_INDEX_AP_DUMP 		0
#define IMG_INDEX_CP_DUMP 		2
#define IMG_INDEX_END_DUMP	1

int lcdc_disp_img(int img_index);
bool lcdc_showing_dump(void);

#endif


#endif

