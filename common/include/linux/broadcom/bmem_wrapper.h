/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	include/linux/broadcom/bcm_memalloc_wrapper.h
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

#ifndef _BMEM_WRAPPER_H_
#define _BMEM_WRAPPER_H_

#include "linux/types.h"

#if defined (CONFIG_BRCM_V3D_OPT)
#define BMEM_SIZE  (1024*1024*100)
#else
#define BMEM_SIZE  (1024*1024*24)
#endif

typedef void *BMEM_HDL;

typedef enum {
	BMEM_CLEAR_STAT,
	BMEM_RESET,
	BMEM_SET_SMALL_CHUNK_THRESHOLD,
	BMEM_SET_DEBUG_LEVEL,
	BMEM_SET_STAT_LEVEL,
} bmem_set_status_cmd_e;

typedef union {
	unsigned int chunk_threshold;
	unsigned int debug_level;
	unsigned int stat_level;
}bmem_set_status_data_t;

typedef struct {
	unsigned int total_used_space;
	unsigned int total_free_space;
	unsigned int max_used_space;
	unsigned int num_buf_used;
	unsigned int num_buf_free;
	unsigned int max_num_buf_free;
	unsigned int max_fragmented_size;
	unsigned int biggest_chunk_avlbl;
	unsigned int smallest_chunk_avlbl;
	unsigned int smallest_buf_request;
	unsigned int biggest_buf_request;
	unsigned int alloc_pass_cnt;
	unsigned int alloc_fail_cnt;
	unsigned int free_pass_cnt;
	unsigned int free_fail_cnt;
} bmem_status_t;

typedef struct {
	bmem_set_status_cmd_e cmd;
	bmem_set_status_data_t data;
} bmem_set_status_t ;

struct bmem_logic {
	int (*AllocMemory)(BMEM_HDL hdl, unsigned long *busaddr,
			unsigned int size);
	int (*FreeMemory)(BMEM_HDL hdl, unsigned long *busaddr);
	int (*open)(BMEM_HDL *hdlp);
	int (*release)(BMEM_HDL hdl);
	int (*mmap)(unsigned long size, unsigned long pgoff);
#ifdef BMEM_CHECK_OVERRUN
	int (*init)(unsigned int memory_size, unsigned int phy_start_address, void *virt_start_address);
#else
	int (*init)(unsigned int memory_size, unsigned int phy_start_address);
#endif
	int (*cleanup)(void);
	int (*GetStatus)(bmem_status_t *p_bmem_status);
	int (*SetStatus)(bmem_set_status_t *p_bmem_set_status);
	int (*GetFreeMemorySize)(void);
	int (*GetUsedMemoryByTgid)(pid_t tgid);
};

int register_bmem_wrapper(struct bmem_logic *logic);
void deregister_bmem_wrapper(void);

#endif /* _BMEM_WRAPPER_H_ */
