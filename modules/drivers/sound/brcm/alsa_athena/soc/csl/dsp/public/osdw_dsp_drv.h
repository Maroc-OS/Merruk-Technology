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
*   @file   csl_dsp_drv.h
*
*   @brief  This file accesses the shared memory for the VPU
*
****************************************************************************/
#ifndef _OSDW_DSP_DRV_H_
#define _OSDW_DSP_DRV_H_


#include "mobcom_types.h"


typedef void (*Audio_ISR_Handler_t)(StatQ_t status_msg);
//typedef void (*VPU_ProcessStatus_t)(VPStatQ_t vp_status_msg);
typedef void (*VPU_ProcessStatus_t)( void );

void DSPDRV_Init(void);

void RIPISR_Register_AudioISR_Handler( Audio_ISR_Handler_t isr_cb );

void RIPISR_Register_VPU_ProcessStatus( VPU_ProcessStatus_t hisr_cb );

void Audio_ISR_Handler(StatQ_t msg);

void VPSHAREDMEM_TriggerRIPInt(void);

#endif //_OSDW_DSP_DRV_H_
