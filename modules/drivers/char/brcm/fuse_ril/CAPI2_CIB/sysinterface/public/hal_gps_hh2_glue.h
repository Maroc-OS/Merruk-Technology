/*********************************************************************
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
***************************************************************************/
/**
*
*   @file   hal_gps_hh2_glue.h
*
*
****************************************************************************/

void GL_GPS_SPI_Drv_Init(void);
UInt16 GL_GPS_SPI_Drv_u16Read_RxMax(UInt8* buffer, UInt16 count);
UInt16 GL_GPS_SPI_Drv_u16Write_RxMax(UInt8* buffer, UInt16 count);
void GL_GPS_SPI_Drv_CNTIN(Boolean flag);
UInt16 GL_GPS_SPI_Drv_u16ProcessErrors(void);
void GL_GPS_SPI_Drv_IntAck(void);
void GL_GPS_SPI_Drv_PowerUp(UInt16 timer);

typedef void (*GL_GPS_RX_DATA_CALLBACK)(void);


extern GL_GPS_RX_DATA_CALLBACK glcb_pfnGL_GPS_HandleRxIntrEvent;


