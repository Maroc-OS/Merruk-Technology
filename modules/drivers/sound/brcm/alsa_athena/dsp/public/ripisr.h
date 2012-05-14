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

#ifndef _PHY_RIPISR_H_
#define _PHY_RIPISR_H_

#include "mobcom_types.h"
#include "consts.h"

#ifdef DEFINE_FIQ_MEM
#	define	FIQEXTERN
#else
#	define	FIQEXTERN	extern
#endif

FIQEXTERN UInt16 rfic_phase1;
FIQEXTERN UInt16 rfic_phase2;

typedef void (*RIPISR_HISR_CB)(void);
typedef Boolean (*RIPISR_PROCESSSTATUS_CB)(void * msg);

typedef void (*Audio_ISR_Handler_t)(StatQ_t status_msg);
//typedef void (*VPU_ProcessStatus_t)(VPStatQ_t vp_status_msg);
typedef void (*VPU_ProcessStatus_t)( void );

//******************************************************************************
// Global Function prototypes
//******************************************************************************
void RIPISR_Init( void );				// Initialize the RIP ISR
void RIPISR_Register(	RIPISR_HISR_CB hisr_cb,	RIPISR_PROCESSSTATUS_CB msg_cb);

void RIPISR_Register_AudioISR_Handler( Audio_ISR_Handler_t isr_cb );
void RIPISR_Register_VPU_ProcessStatus( VPU_ProcessStatus_t hisr_cb );

void trigger_rip_interrupt( void );


//will figure out how to avoid vpu.c call this function.
//and remove this from public interface...
void Audio_ISR_Handler(StatQ_t msg);
		
#endif

