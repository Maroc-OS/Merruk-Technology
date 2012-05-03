/*******************************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this software
 * in any way with any other Broadcom software provided under a license other than
 * the GPL, without Broadcom's express prior written consent.
 * *******************************************************************************************/

/****************************************************************************************************
*
*   @file   cacheops.h
*
*   @brief  This header file declares input/output params struct and ioctl interface for cache operation.
*
*
********************************************************************************************************/
#define CACHE_OPS_DEVICE             "cache-ops"
#define MAG_NUM                 'c'
#define CACHE_OPS   _IO(MAG_NUM,1)
static struct class *cache_class;
struct cache_interface {
	char                        i_f;            // cache invalidate or flush
    	unsigned int                pstart;
    	unsigned int                vstart;
    	unsigned int                size;
    	int                 	    out_rc;         // return code from the test
};
typedef struct cache_interface cache_interface_t;

