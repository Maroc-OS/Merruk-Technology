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
*   @file   csl_dsp.c
*
*   @brief  This file accesses the shared memory for the VPU
*
****************************************************************************/

//#include <string.h>
#include "assert.h"
#include "mobcom_types.h"
#include "consts.h"
#include "ostypes.h"
#include "log.h"
#include "shared.h"
#include "csl_dsp.h"
#include "osdw_dsp_drv.h"
//#include "memmap.h"



//typedef        AP_SharedMem_t VPSharedMem_t;

VPSharedMem_t		   *vp_shared_mem;

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )


//******************************************************************************
//
// Function Name:	VPSHAREDMEM_Init
//
// Description: Initialize Shared Memory for UPU
//
// Notes:
//
//******************************************************************************

void VPSHAREDMEM_Init(UInt32	dsp_shared_mem)
{
	AP_SHAREDMEM_Init();
	
	vp_shared_mem = (VPSharedMem_t*) dsp_shared_mem;
	
	Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_Init: dsp_shared_mem=0x%x, \n", dsp_shared_mem);

	vp_shared_mem->vp_shared_cmdq_in = 0;
	vp_shared_mem->vp_shared_cmdq_out = 0;
	vp_shared_mem->vp_shared_statusq_in = 0;
	vp_shared_mem->vp_shared_statusq_out = 0;

}

//******************************************************************************
//
// Function Name:	VPSHAREDMEM_PostCmdQ
//
// Description:   This function posts an entry to the VPU command queue
//
// Notes:
//
//******************************************************************************

void VPSHAREDMEM_PostCmdQ(			// Post an entry to the VPU command queue
	VPCmdQ_t *cmd_msg					// Entry to post
	)
{
	VPCmdQ_t 	*p;
	UInt8 	next_cmd_in;

	//Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_PostCmdQ: cmdq_in=0x%x, cmdq_out=0x%x \n", vp_shared_mem->vp_shared_cmdq_in, vp_shared_mem->vp_shared_cmdq_out);
	next_cmd_in = (UInt8)(( vp_shared_mem->vp_shared_cmdq_in + 1 ) % VP_CMDQ_SIZE);

	assert( next_cmd_in != vp_shared_mem->vp_shared_cmdq_out );

	p = &vp_shared_mem->vp_shared_cmdq[ vp_shared_mem->vp_shared_cmdq_in ];
	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;

	vp_shared_mem->vp_shared_cmdq_in = next_cmd_in;
	//Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_PostCmdQ: cmd=0x%x, arg0=0x%x, arg1=%d, arg2=%d \n", cmd_msg->cmd, cmd_msg->arg0, cmd_msg->arg1, cmd_msg->arg2);

	VPSHAREDMEM_TriggerRIPInt();

}


//******************************************************************************
//
// Function Name:	VPSHAREDMEM_ReadStatusQ
//
// Description: This function read an entry from the VPU status  queue
//
// Notes:
//
//******************************************************************************

Boolean VPSHAREDMEM_ReadStatusQ(			// Read an entry from the VPU status  queue
	VPStatQ_t *status_msg					// Entry from queue
	)
{
	VPStatQ_t *p;
	UInt8	status_out = vp_shared_mem->vp_shared_statusq_out;
	UInt8	status_in = vp_shared_mem->vp_shared_statusq_in;

	//Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_ReadStatusQ: status_in=0x%x, status_out=0x%x \n", vp_shared_mem->vp_shared_statusq_in, vp_shared_mem->vp_shared_statusq_out);
	if ( status_out == status_in )
	{
		return FALSE;
	}
	else
	{
		p = &vp_shared_mem->vp_shared_statusq[ status_out ];
		status_msg->status = p->status;
		status_msg->arg0 = (UInt16)p->arg0;
		status_msg->arg1 = (UInt16)p->arg1;
		status_msg->arg2 = (UInt16)p->arg2;
		status_msg->arg3 = (UInt16)p->arg3;
		//Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_ReadStatusQ: status=%d, arg0=%d, arg1=%d, arg2=%d, arg3=%d \n", p->status, p->arg0, p->arg1, p->arg2, p->arg3);

		vp_shared_mem->vp_shared_statusq_out = ( status_out + 1 ) % VP_STATUSQ_SIZE;

		return TRUE;
	}

}


#endif // #if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

