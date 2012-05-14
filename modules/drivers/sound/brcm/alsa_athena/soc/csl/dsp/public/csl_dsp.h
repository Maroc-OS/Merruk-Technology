/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/
/**
*
*   @file   csl_dsp.h
*
*   @brief  This file accesses the shared memory for the VPU
*
****************************************************************************/
#ifndef _CSL_DSP_H_
#define _CSL_DSP_H_

#include "mobcom_types.h"

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

void VPSHAREDMEM_Init( 
	UInt32	dsp_shared_mem
	);

void VPSHAREDMEM_PostCmdQ(			// Post an entry to the VPU command queue
	VPCmdQ_t *cmd_msg					// Entry to post
	);


Boolean VPSHAREDMEM_ReadStatusQ(			// Read an entry from the VPU status  queue
	VPStatQ_t *status_msg					// Entry from queue
	);


#endif // #if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

#endif //_CSL_DSP_DRV_H_
