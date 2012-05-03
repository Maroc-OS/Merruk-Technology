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

#ifndef _MEMALLOC_IOCTL_H_
#define _MEMALLOC_IOCTL_H_
#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */
/*
 * Ioctl definitions
 */

#if defined (CONFIG_BMEM)
#define HANTRO_WRAP_MAGIC  'M'
#else
#define HANTRO_WRAP_MAGIC  'B'
#endif
#define HANTRO_WRAP_ACQUIRE_BUFFER _IOWR(HANTRO_WRAP_MAGIC,  1, unsigned long)
#define HANTRO_WRAP_RELEASE_BUFFER _IOW(HANTRO_WRAP_MAGIC,  2, unsigned long)

#define HANTRO_STORE_OUTBUFFER_0 _IOWR(HANTRO_WRAP_MAGIC,  3, unsigned long)
#define HANTRO_GET_OUTBUFFER_0 _IOWR(HANTRO_WRAP_MAGIC, 4, unsigned long)

#define HANTRO_STORE_OUTBUFFER_1 _IOWR(HANTRO_WRAP_MAGIC,  5, unsigned long)
#define HANTRO_GET_OUTBUFFER_1 _IOWR(HANTRO_WRAP_MAGIC, 6, unsigned long)

#define HANTRO_STORE_OUTBUFFER _IOWR(HANTRO_WRAP_MAGIC,  7, unsigned long)
#define HANTRO_SET_OUTBUFFER_VIRTUAL_ADDRESS (HANTRO_WRAP_MAGIC,  8, unsigned long)
#define HANTRO_GET_OUTBUFFER _IOWR(HANTRO_WRAP_MAGIC, 9, unsigned long)


#define HANTRO_DMA_COPY 			_IOW(HANTRO_WRAP_MAGIC,  10, unsigned long)

#define HANTRO_WRAP_MAXNR 15

typedef struct MemallocwrapParams{
	unsigned long busAddress;
	unsigned long virtualaddress;
	unsigned int size;
	struct MemallocwrapParams* nextAddress;
} MemallocwrapParams;

#ifndef _BMEM_DMA_STRUCT_
#define _BMEM_DMA_STRUCT_
typedef struct {
	unsigned long src_busAddr; 	// input physical addr
	unsigned long dst_busAddr;	// ouptut physical addr
	unsigned int size;			// size to be transferred.
} DmaStruct;
#endif



#endif /* _MEMALLOC_IOCTL_H_ */
