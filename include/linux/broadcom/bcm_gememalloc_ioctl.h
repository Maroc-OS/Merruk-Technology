/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	include/linux/broadcom/bcm_memalloc_ioctl.h
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
#if defined (CONFIG_BMEM)

#ifndef _BMEM_IOCTL_H_
#define _BMEM_IOCTL_H_
#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */

/*
 * Ioctl definitions
 */
#define BMEM_WRAP_MAGIC  'B'
#define GEMEMALLOC_WRAP_ACQUIRE_BUFFER _IOWR(BMEM_WRAP_MAGIC,  1, unsigned long)
#define GEMEMALLOC_WRAP_RELEASE_BUFFER _IOW(BMEM_WRAP_MAGIC,  2, unsigned long)
#define GEMEMALLOC_WRAP_COPY_BUFFER _IOW(BMEM_WRAP_MAGIC,  3, unsigned long)

#define BMEM_WRAP_MAXNR 15

typedef struct {
	unsigned long busAddress;
	unsigned int size;
} GEMemallocwrapParams;

#ifndef _BMEM_DMA_STRUCT_
#define _BMEM_DMA_STRUCT_
typedef struct {
	unsigned long src_busAddr; 	// input physical addr
	unsigned long dst_busAddr;	// ouptut physical addr
	unsigned int size;			// size to be transferred.
} DmaStruct;
#endif

#endif /* _BMEM_IOCTL_H_ */

#else

#ifndef _GEMEMALLOC_IOCTL_H_
#define _GEMEMALLOC_IOCTL_H_
#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */

/*
 * Ioctl definitions
 */
#define GEMEMALLOC_WRAP_MAGIC  'B'
#define GEMEMALLOC_WRAP_ACQUIRE_BUFFER _IOWR(GEMEMALLOC_WRAP_MAGIC,  1, unsigned long)
#define GEMEMALLOC_WRAP_RELEASE_BUFFER _IOW(GEMEMALLOC_WRAP_MAGIC,  2, unsigned long)
#define GEMEMALLOC_WRAP_COPY_BUFFER _IOW(GEMEMALLOC_WRAP_MAGIC,  3, unsigned long)

#define GEMEMALLOC_WRAP_MAXNR 15

typedef struct {
	unsigned long busAddress;
	unsigned int size;
} GEMemallocwrapParams;

typedef struct {
	unsigned long src_busAddr; 	// input physical addr
	unsigned long dst_busAddr;	// ouptut physical addr
	unsigned int size;			// size to be transferred.
} DmaStruct;

#endif /* _MEMALLOC_IOCTL_H_ */

#endif
