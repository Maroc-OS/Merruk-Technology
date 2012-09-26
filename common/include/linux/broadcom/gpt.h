/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/gpt.h
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
#ifndef _GPT_H_

#include <linux/types.h>

enum gpt_ctrl {
	HW_PEDEMODE_DIS = (1 << 22),
	SW_PEDEMODE     = (1 << 23),
	HW_SLPMODE_DIS  = (1 << 28),
	SW_SLPMODE      = (1 << 29),
};

enum gpt_mode {
	GPT_MODE_PERIODIC = 0,
	GPT_MODE_ONESHOT,
};

enum gpt_error {
	GPT_SUCCESS = 0,
	GPT_ERR_INVALID_INDEX = -1,
	GPT_ERR_RESERVED = -2,
	GPT_ERR_NOT_FREE = -3,
	GPT_ERR_NOT_CONFIGURED = -4,
	GPT_ERR_NOT_ASSIGNED = -5,
	GPT_ERR_RELOAD_VAL = -6,
};

struct gpt_cfg {
	enum gpt_ctrl gctrl;
	unsigned int gclk;
	enum gpt_mode gmode;
};

typedef int (*gpt_callback)(void *data);

/*
 * This function is used to configure various GPT parameters like
 * 	    Ctrl : SLPMODE, PEDMODE, 
 *	    Mode : Oneshot/Periodic
 *	     Clk : Frequency in at which timer need to run (26Mhz, 1Mhz, 32Khz)
 * 	Callback : Interrupts are enabled for the GPT if callback is provided.
 * 	     Arg : Argument to receive when callback is called.
 *
 * This API should be called after requesting a GPT and could be called any number 
 * of times before calling gpt_free.
 *
 * Return:
 * 	GPT_SUCCESS	       : On success
 *	GPT_ERR_INVALID_INDEX  : If the gpt index provided is invalid
 *	GPT_ERR_NOT_CONFIGURED : If the GPT is not requested before
 */

extern int gpt_config(int gpt_index, struct gpt_cfg *gc,
                gpt_callback gcallback, void *arg);

/* 
 * This function is used request/reserve a particular GPT. The input the function is
 * the index of the GPT that needs to be used (this index should be > 0 and < BCM_NUM_GPT).
 * If an index of -1 is supplied then the next free GPT index is assigned and returned.
 *
 * Return value:
 * 	GPT_ERR_INVALID_INDEX: If the index supplied is > Max GPTs.
 * For index non-zero : 
 * 	Index		     : The same index as requested if the requested timer index if free
 * 	GPT_ERR_NOT_FREE     : If the requested GPT is not free 
 * For index zero     :
 * 	Index		     : Index of the first GPT that is free.
 * 	GPT_ERR_NOT_FREE     : If no GPT are free in the system.
 */
extern int gpt_request(int gpt_index);

/* 
 * This function should be used free the GPT requested (using gpt_request).
 *
 * Return:
 * 	GPT_SUCCESS	       : On success
 *	GPT_ERR_INVALID_INDEX  : If the gpt index provided is invalid
 *	GPT_ERR_NOT_CONFIGURED : If the GPT is not requested before
 */
extern int gpt_free(int gpt_index);

/*
 * This function will be used to start the timer with a specific reload value. 
 * Providing a reload value of < 0 would let the timer continue (with the existing
 * reload value)
 *
 * Return:
 * 	GPT_SUCCESS	       : On success
 *	GPT_ERR_INVALID_INDEX  : If the gpt index provided is invalid
 *	GPT_ERR_NOT_CONFIGURED : If the GPT is not requested before
 */
extern int gpt_start(int gpt_index, s64 reload);

/* 
 * This function wull stop the timer from further execution. The reload value will
 * not be touched.
 *
 * Return:
 * 	GPT_SUCCESS	       : On success
 *	GPT_ERR_INVALID_INDEX  : If the gpt index provided is invalid
 *	GPT_ERR_NOT_CONFIGURED : If the GPT is not requested before
 */
extern int gpt_stop(int gpt_index);

/*
 * This function will return the current count on a give GPT.
 *
 * Return:
 * 	Non-zero counter value : This is the current count of the requested timer
 *	Zero  		       : On any error (if an invalid GPT index is specified,
 *				 or If the gpt_read is performed with a proper request.
 */
extern unsigned int gpt_read(int gpt_index);

/*
 * This function will return the current reload count on a give GPT.
 *
 * Return:
 * 	Non-zero counter value : This is the current reload count of the requested timer
 *	Zero  		       : On any error (if an invalid GPT index is specified,
 *				 or If the gpt_read is performed with a proper request.
 */
extern unsigned int gpt_reload_read(int index);

/* Platform specific data */
struct gpt_base_config {
	int irq;
	void __iomem *base_addr;
	unsigned int avail_bitmap;
};

#endif /* _GPT_H_ */
