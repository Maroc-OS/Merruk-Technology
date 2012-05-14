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
*   @file   osdw_dsp_drv.c
*
*   @brief  This file accesses the shared memory for the VPU
*
****************************************************************************/

#include "mobcom_types.h"
#include "chip_version.h"

#include <linux/sched.h>
#include <linux/interrupt.h>
#include "consts.h"
#include "msconsts.h"
#include "log.h"
#include "sharedmem.h"
//#include "memmap.h"
//#include "csl_dsp_defs.h"
#include "csl_dsp.h"
//#include "osinterrupt.h"
#include "nandsdram_memmap.h"
#include "brcm_rdb_sysmap.h"
#include "chal_intc_inc.h"

//#include "sysmap_types.h"
//#include "csl_sysmap.h"
#include "osdw_dsp_drv.h"

//static Interrupt_t rip_hisr;
static Audio_ISR_Handler_t	client_Audio_ISR_Handler = NULL;
static VPU_ProcessStatus_t	client_VPU_ProcessStatus = NULL;

static AP_SharedMem_t 			*global_shared_mem = NULL;

typedef struct
{
	struct tasklet_struct	task;
    CHAL_HANDLE             h;
} Dspdrv;

static Dspdrv dsp_drv;


static void dsp_thread_proc(unsigned long data);
static irqreturn_t rip_isr(int irq, void *dev_id);

//static void RIPISR_LISR(void);
//static void RIPISR_HISR(void);
static UInt32 DSPDRV_GetSharedMemoryAddress(void);

AP_SharedMem_t *SHAREDMEM_GetDsp_SharedMemPtr();

//******************************************************************************
//
// Function Name:	DSPDRV_Init
//
// Description: Initialize DSP driver
//
// Notes:
//
//******************************************************************************

void DSPDRV_Init( )
{
	UInt32 dsp_shared_mem;
    int rc;

#if !(defined(_ATHENA_))
	IRQ_EnableRIPInt();
#endif
	
	Log_DebugPrintf(LOGID_AUDIO, " DSPDRV_Init:  \n");

    // initialize interrupt controller chal
    dsp_drv.h = chal_intc_init(INTC_BASE_ADDR);
	
    //Create Tasklet
    tasklet_init(&(dsp_drv.task), dsp_thread_proc,(unsigned long)(&dsp_drv));

    //Plug in the ISR
	rc = request_irq(IRQ_DSPIRQ, rip_isr, IRQF_DISABLED,
			 "bcm215xx-dsp", &(dsp_drv));

	if (rc < 0) {
		Log_DebugPrintf(LOGID_AUDIO,"RIPISR: %s failed to attach interrupt, rc = %d\n",
		       __FUNCTION__, rc);
		return;
	}

	dsp_shared_mem = DSPDRV_GetSharedMemoryAddress();

	VPSHAREDMEM_Init(dsp_shared_mem);
	return;
}


//******************************************************************************
//
// Function Name:	DSPDRV_GetSharedMemoryAddress
//
// Description: Initialize DSP driver
//
// Notes:
//
//******************************************************************************
static UInt32 DSPDRV_GetSharedMemoryAddress( )
{
	UInt32 dsp_shared_mem;

    dsp_shared_mem = SHAREDMEM_GetDsp_SharedMemPtr();
#if 0
    
    if(global_shared_mem == NULL)
    {
        global_shared_mem = ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);
        if (!global_shared_mem) {
            Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* mapping dsp shared memory failed\n\r");
            return NULL;
        }
    }
   //dsp_shared_mem = ((UInt8*)global_shared_mem + sizeof(NOT_USE_Dsp_SharedMem_t));
	dsp_shared_mem = (UInt8*)global_shared_mem;
#endif
	return dsp_shared_mem;
}

//******************************************************************************
//
// Function Name:	rip_isr
//
// Description:		This function is the Low Level ISR for the RIP interrupt.
//					It simply triggers the dsp_thread_proc.
//
// Notes:
//
//******************************************************************************
static irqreturn_t rip_isr(int irq, void *dev_id)
{
	Dspdrv *dev	= dev_id;

	tasklet_schedule(&dev->task);
	return IRQ_HANDLED;
}

//******************************************************************************
//
// Function Name:	dsp_thread_proc
//
// Description:		This function is the RIP interrupt service routine.
//
// Notes:
//
//******************************************************************************
static void dsp_thread_proc(unsigned long data)
{
	if(client_VPU_ProcessStatus != NULL)
	{
		//Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AP dsp_thread_proc =0x%x\n\r", client_VPU_ProcessStatus);
		client_VPU_ProcessStatus();
	}
	
}

//******************************************************************************
//
// Function Name:	Audio_ISR_Handler
//
// Description:		
//
// Notes:
//
//******************************************************************************

//will figure out how to avoid vpu.c call this function.
/*static*/ void Audio_ISR_Handler(StatQ_t msg)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AP Audio_ISR_Handler\n\r");

	if( client_Audio_ISR_Handler != NULL )
	{
		client_Audio_ISR_Handler( msg );
	}
}

//******************************************************************************
//
// Function Name:	RIPISR_Register_AudioISR_Handler
//
// Description:		This function registers audio isr handler.
//
// Notes:
//
//******************************************************************************

void RIPISR_Register_AudioISR_Handler( Audio_ISR_Handler_t isr_cb )
{
	client_Audio_ISR_Handler = isr_cb;

}

//******************************************************************************
//
// Function Name:	RIPISR_Register_VPU_ProcessStatus
//
// Description:		This function registers VPU Process handler
//
// Notes:
//
//******************************************************************************
void RIPISR_Register_VPU_ProcessStatus( VPU_ProcessStatus_t hisr_cb )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AP RIPISR_Register_VPU_ProcessStatus, 0x%x\n\r", hisr_cb);
	client_VPU_ProcessStatus = hisr_cb;
}

//******************************************************************************
//
// Function Name:	VPSHAREDMEM_TriggerRIPInt
//
// Description: This function triggers DSP interrupt
//
// Notes:
//
//******************************************************************************
void VPSHAREDMEM_TriggerRIPInt()
{
    Chal_Intc_ConfigReg_t config;
    
    config.mICCR_dword = chal_intc_read_irq_config_reg(dsp_drv.h);
    config.mICCR_field.mICCR_intrip = 1;
    chal_intc_set_irq_config_reg( dsp_drv.h, config.mICCR_dword);
}
