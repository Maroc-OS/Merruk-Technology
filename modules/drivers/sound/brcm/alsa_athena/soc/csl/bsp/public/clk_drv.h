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
*   @file   clk_drv.h
*   @brief  System clock configuration driver 
*
****************************************************************************/
/**
*
* @defgroup CLKGroup Broadcom Clock Controller
* @brief This group defines the APIs for Clock interfaces.
*
* @ingroup CSLGroup

Click here to navigate back to the Chip Support Library Overview page: \ref CSLOverview
*****************************************************************************/

#ifndef __CLOCK_DRV_H
#define __CLOCK_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
Global types
*****************************************************************************/
#include <plat/osdal_os_service.h>

#define CLK_AUDIO_RX_ADC_CLOCK  OSDAL_CLK_AUDIO_RX_ADC_CLOCK
#define CLK_AUDIO_TX_DAC_CLOCK  OSDAL_CLK_AUDIO_TX_DAC_CLOCK


#define CLKDRV_Open()  NULL


#if 1    
// Stop clock

#define CLKDRV_Stop_Clock(clk_handle,id)    { \
                                                OSDAL_CLK_HANDLE hClk; \
                                                hClk = OSDAL_CLK_Open(id); \
                                                OSDAL_CLK_Stop(hClk); \
                                                OSDAL_CLK_Close(hClk); \
                                            }


// Start Clock
#define CLKDRV_Start_Clock(clk_handle,id)    { \
                                                OSDAL_CLK_HANDLE hClk; \
                                                hClk = OSDAL_CLK_Open(id); \
                                                OSDAL_CLK_Start(hClk); \
                                                OSDAL_CLK_Close(hClk); \
                                            }

#else

#define CLKDRV_Stop_Clock(clk_handle,id)    


// Start Clock
#define CLKDRV_Start_Clock(clk_handle,id)    

#endif

#endif // __SYSCFG_H
