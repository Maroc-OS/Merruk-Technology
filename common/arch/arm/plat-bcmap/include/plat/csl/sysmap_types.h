/*****************************************************************************
*  Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/* Generated file */
/* Do not modify */

#ifndef _SYSMAP_TYPES_H_
#define _SYSMAP_TYPES_H_

/**
* Sysmap base address type
* 
*****************************************************************************/
typedef enum
{
    BMARBH,	/* 0x08010000 */
    BMARBL,	/* 0x08018000 */
    DMAC,	/* 0x08020000 */
    LCDC,	/* 0x08030000 */
    MPHI,	/* 0x08050000 */
    MP,	/* 0x08070000 */
    NVSRAM,	/* 0x08090000 */
    DP,	/* 0x080A0000 */
    DES,	/* 0x080B0000 */
    SDIO1,	/* 0x08110000 */
    SDIO2,	/* 0x08120000 */
    CLK,	/* 0x08140000 */
    CIPHER,	/* 0x08150000 */
    CRC,	/* 0x08180000 */
    VIDEO_DEC,	/* 0x081A0000 */
    VIDEO_ENC,	/* 0x081A8000 */
    ECT0_CTI0,	/* 0x081C0000 */
    ECT0_CTI1,	/* 0x081C1000 */
    ECT1_CTI0,	/* 0x081C2000 */
    ECT1_CTI1,	/* 0x081C3000 */
    HSOTG,	/* 0x08200000 */
    HSOTG_CTRL,	/* 0x08280000 */
    FSHOST,	/* 0x08300000 */
    FSHOST_CTRL,	/* 0x08380000 */
    SDIO3,	/* 0x08400000 */
    EMI,	/* 0x08420000 */
    CAM,	/* 0x08440000 */
    HUCM,	/* 0x08480000 */
    DSI,	/* 0x084A0000 */
    L210,	/* 0x084A1000 */
    SLPTMR,	/* 0x08800000 */
    SLPTMR2,	/* 0x08800100 */
    INTC,	/* 0x08810000 */
    UARTA,	/* 0x08820000 */
    UARTB,	/* 0x08821000 */
    UARTC,	/* 0x08822000 */
    AUXADC,	/* 0x08830020 */
    GPTIMER,	/* 0x08830100 */
    BB2PMU_ADCSYNC,	/* 0x08830200 */
    SIMI,	/* 0x08860000 */
    SCLKCAL,	/* 0x08870010 */
    SYSCFG,	/* 0x08880000 */
    GEA,	/* 0x08890000 */
    WATCHDOG,	/* 0x088A0000 */
    BSC1,	/* 0x088A0000 */
    WATCHDOG2,	/* 0x088A0010 */
    BSC2,	/* 0x088B0000 */
    I2S,	/* 0x088C0000 */
    GPIO,	/* 0x088CE000 */
    KEYPAD,	/* 0x088CE000 */
    SPI,	/* 0x088D0000 */
    SOFTRST,	/* 0x088E0000 */
    BSC3,	/* 0x088F0000 */
    MSPRO,	/* 0x08900000 */
    AUXMIC,	/* 0x08911000 */
    SPI2,	/* 0x08920000 */
    SIMI2,	/* 0x08930000 */
    PWM,	/* 0x08940000 */
    V3D,	/* 0x08950000 */
    PWRWDOG,	/* 0x08960000 */
    HTM0,	/* 0x08C00000 */
    HTM1,	/* 0x08C01000 */
    TFUNNEL,	/* 0x08C02000 */
    TPIU,	/* 0x08C03000 */
    APS_ETM,	/* 0x08C04000 */
    MDM_ETM,	/* 0x08C05000 */
    CRYPTO,	/* 0x0C080000 */
    PKA,	/* 0x0C0C8000 */
    OTP,	/* 0x0C0C9000 */
    RNG,	/* 0x0C0CA000 */
    WCDMACM,	/* 0x08168000 */
    WCDMAMPD0_15,	/* 0x08168400 */
    WCDMAMPD0_30,	/* 0x08168500 */
    WCDMASSYNC,	/* 0x08168C00 */
    WCDMAPSYNC,	/* 0x08169000 */
    WCDMATX,	/* 0x08169100 */
    WCDMAFNGMUX,	/* 0x08169200 */
    WCDMARXBIT,	/* 0x08169280 */
    WCDMARXDEINT,	/* 0x08169300 */
    WCDMAAGC,	/* 0x08169380 */
    WCDMATWIF,	/* 0x08169400 */
    WCDMASTTD,	/* 0x08169480 */
    WCDMAAFC,	/* 0x08169500 */
    WCDMATNYRAKE_1,	/* 0x08169600 */
    WCDMATNYRAKE_2,	/* 0x08169700 */
    WCDMATNYRAKE_3,	/* 0x08169800 */
    WCDMATNYCTRL,	/* 0x08169900 */
    WCDMASCRAMXY,	/* 0x08169980 */
    WCDMABBRX,	/* 0x08169A00 */
    WCDMAPRISM,	/* 0x08169B00 */
    WCDMACPP,	/* 0x08169C00 */
    WCDMACPCHARQ,	/* 0x08169E00 */
    WCDMAMC,	/* 0x08169F00 */
    WCDMASPINCLK1,	/* 0x08169F80 */
    WCDMASPINCLK2,	/* 0x08169FC0 */
    WCDMAHTDM,	/* 0x0816A100 */
    WCDMASCCH,	/* 0x0816A200 */
    WCDMAHARQ,	/* 0x0816A300 */
    WCDMATNYRAKE_4,	/* 0x0816A400 */
    WCDMATNYRAKE_5,	/* 0x0816A500 */
    WCDMATNYRAKE_6,	/* 0x0816A600 */
    WCDMARFIC,	/* 0x0816A780 */
    WCDMARFICBUFF,	/* 0x0816A800 */
    WCDMAMPD1_15,	/* 0x0816AA00 */
    WCDMAMPD1_30,	/* 0x0816AB00 */
    WCDMASCHED,	/* 0x0816AC00 */
    WCDMABBRFTX,	/* 0x0816AE00 */
    WCDMARXADC,	/* 0x0816B000 */
    WCDMAGP,	/* 0x0816B080 */
    WCDMAPDM,	/* 0x0816B100 */
    WCDMATXDAC,	/* 0x0816B180 */
    WCDMAGPTIMER,	/* 0x0816C000 */
    WCDMAL2INT,	/* 0x0816C180 */
    WCDMAL1IRQ,	/* 0x0816C280 */
    WCDMAL1FIQ,	/* 0x0816C2C0 */
    WCDMAASICCLK,	/* 0x0816C300 */
    WCDMAGPIO,	/* 0x0816C500 */
    WCDMAUART1,	/* 0x0816C600 */
    WCDMARAKE_1,	/* 0x0816D000 */
    WCDMARAKE_2,	/* 0x0816D200 */
    WCDMARAKE_3,	/* 0x0816D400 */
    WCDMARAKE_4,	/* 0x0816D600 */
    WCDMARAKE_5,	/* 0x0816D800 */
    WCDMARAKE_6,	/* 0x0816DA00 */
    WCDMARAKE_7,	/* 0x0816DC00 */
    WCDMARAKE_8,	/* 0x0816DE00 */
    WCDMAL2INT_ASYNC,	/* 0x0816F000 */
    AHB_TL3R,	/* 0x30400000 */
    AHB_INTC,	/* 0x30800000 */
    AHB_TPIF,	/* 0x30800000 */
    AHB_CIPHER,	/* 0x30800000 */
    AHB_EQUALIZER,	/* 0x30800000 */
    AHB_EVENT_CTRL,	/* 0x30800000 */
    AHB_RF_GPO,	/* 0x30800000 */
    ARM_RFSPI,	/* 0x30800000 */
    AHB_3WIRE_INTF,	/* 0x30800000 */
    AHB_SYS_TIMER,	/* 0x30800000 */
    AHB_DIGRF,	/* 0x30800000 */
    AHB_AC,	/* 0x30800000 */
    AHB_RX_CONTROL,	/* 0x30800000 */
    AHB_TX_CONTROL,	/* 0x30800000 */
    AHB_AUDIO,	/* 0x30800000 */
    AHB_PCM,	/* 0x30800000 */
    WCDMA_BOOT,	/* 0xFFFF0000 */
    SYSMAP_BASE_MAX,
}SYSMAP_BASE_t;

#endif //_SYSMAP_TYPES_H_
