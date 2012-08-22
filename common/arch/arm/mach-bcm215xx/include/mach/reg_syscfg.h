/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/include/mach/reg_syscfg.h
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

#ifndef __REG_BCM21553_SYSCFG_H__
#define __REG_BCM21553_SYSCFG_H__

#include <mach/io.h>
#include <mach/hardware.h>

#define HW_SYSCFG_BASE                      IO_ADDRESS(BCM21553_SYSCFG_BASE)
#define HW_GPIO_BASE                        IO_ADDRESS(BCM21553_GPIO_BASE)
#define HW_BSC2_BASE                        IO_ADDRESS(BCM21553_I2C2_BASE)

#define ADDR_BSC2_CS                        (HW_BSC2_BASE + 0x000)                        

/* System configuration module register mapping */
#define ADDR_SYSCFG_IOCR0			    (HW_SYSCFG_BASE + 0x000)
#define   SYSCFG_IOCR0_SD3_MUX(x)		    (((x) & 0x3) << 0)
#define   SYSCFG_IOCR0_SD1_MUX(x)		    (((x) & 0x3) << 3)
#define   SYSCFG_IOCR0_GPIO2928_GPEN1312_MUX(x)     (((x) & 0x3) << 7)
#define   SYSCFG_IOCR0_GPIO17_PWM0_MUX	    	    (1 << 9)
#define   SYSCFG_IOCR0_SPI_UARTC_MUX		    (1 << 11)
#define	  SYSCFG_IOCR0_GPIO52_GPEN7_MUX	    	    (1 << 12)
#define   SYSCFG_IOCR0_GPIO53_GPEN8_MUX	    	    (1 << 13)
#define	  SYSCFG_IOCR0_GPEN9_SPI_GPIO54_L_MUX 	    (1 << 14)
#define	  SYSCFG_IOCR0_GPIO55_GPEN10_MUX(x)   	    (((x) & 0x3) << 15)
#define   SYSCFG_IOCR0_SPIL_GPIO_MUX	    	    (1 << 11)
#define   SYSCFG_IOCR0_GPEN11_SPI_GPIO56_L_MUX	    (1 << 17)
#define   SYSCFG_IOCR0_GPEN11_SPI_GPIO56_H_MUX	    (1 << 20)
#define   SYSCFG_IOCR0_SPIH_GPIO_MUX	    	    (1 << 21)
#define	  SYSCFG_IOCR0_PCM_SPI2_GPIO4043_MUX(x)	    (((x) & 0x3) << 22)
#define	  SYSCFG_IOCR0_DIGMIC_GPIO6362_MUX    	    (1 << 24)
#define	  SYSCFG_IOCR0_GPEN9_SPI_GPIO54_H_MUX 	    (1 << 27)
#define   SYSCFG_IOCR0_LCD_CTRL_MUX	    	    (1 << 29)
#define   SYSCFG_IOCR0_MPHI_MUX		    	    (1 << 25)
#define	  SYSCFG_IOCR0_FLASH_SD2_MUX	    	    (1 << 30)
#define   SYSCFG_IOCR0_CAMCK_GPIO_MUX	    	    (1 << 31)
#define ADDR_SYSCFG_IOCR1			    (HW_SYSCFG_BASE + 0x004)
#define   SYSCFG_IOCR1_KEY_ROW(x)		    (((x) & 0xff) << 0)
#define   SYSCFG_IOCR1_KEY_COL(x)		    (((x) & 0xff) << 8)
#define ADDR_SYSCFG_SUCR			    (HW_SYSCFG_BASE + 0x008)
#define ADDR_SYSCFG_IOCR2			    (HW_SYSCFG_BASE + 0x00c)
#define   SYSCFG_IOCR2_OSC2_ENABLE                  (1 << 3)
#define   SYSCFG_IOCR2_LCDTE_PULL_CTRL(x)	    (((x) & 0x3) << 8)
#define   SYSCFG_IOCR2_LCDDATA_PULL_CTRL(x)	    (((x) & 0x3) << 10)
#define   SYSCFG_IOCR2_UARTA_PULL_CTRL(x)	    (((x) & 0x3) << 12)
#define   SYSCFG_IOCR2_SD3CMD_PULL_CTRL(x)	    (((x) & 0x3) << 19)
#define   SYSCFG_IOCR2_SD3DAT_PULL_CTRL(x)	    (((x) & 0x3) << 22)
#define   SYSCFG_IOCR2_SD1CMD_PULL_CTRL(x)	    (((x) & 0x3) << 24)
#define   SYSCFG_IOCR2_SD1DAT_PULL_CTRL(x)	    (((x) & 0x3) << 26)
#define   SYSCFG_IOCR2_SD2CMD_PULL_CTRL(x)	    (((x) & 0x3) << 28)
#define   SYSCFG_IOCR2_SD2DATL_PULL_CTRL(x)	    (((x) & 0x3) << 30)
#define	     SD_PULL_UP				    (0x01)
#define	     SD_PULL_DOWN			    (0x02)
#define ADDR_SYSCFG_PUMR			    (HW_SYSCFG_BASE + 0x018)
#define   PUMR_VAL_OFF			    	    (0x000)
#define   PUMR_VAL_SOFT_DOWNLOAD		    (0x001)
#define   PUMR_VAL_CALIBRATION		    	    (0x002)
#define   PUMR_VAL_SECURITY_DOWNLOAD	    	    (0x004)
#define   PUMR_VAL_CALL_TEST		    	    (0x008)
#define   PUMR_VAL_USB_DOWNLOAD		     	    (0x010)
#define   PUMR_VAL_BOOTLOADER_UART_DOWNLOAD   	    (0x020)
#define   PUMR_VAL_AP_ONLY_BOOT			    (0x040)
#define   PUMR_VAL_SOFT_RESET		    	    (0x100)
#define   PUMR_VAL_POWEROFF_CHARGING	    	    (0x200)
#define ADDR_SYSCFG_IOCR3                           (HW_SYSCFG_BASE + 0x01C)
#define   SYSCFG_IOCR3_CAMCK_DIS		    (1 << 4)
#define   SYSCFG_IOCR3_CAMDCK_PU		    (1 << 5)
#define   SYSCFG_IOCR3_CAMDCK_PD		    (1 << 6)
#define   SYSCFG_IOCR3_CAMHVS_PU	    	    (1 << 7)
#define   SYSCFG_IOCR3_CAMHVS_PD	    	    (1 << 8)
#define   SYSCFG_IOCR3_CAMD_PU	    	    	    (1 << 9)
#define   SYSCFG_IOCR3_CAMD_PD	    	    	    (1 << 10)
#define   SYSCFG_IOCR3_TWIF_ENB			    (1 << 21)
#define   SYSCFG_IOCR3_SIMDAT_PU		    (1 << 27)
#define   SYSCFG_IOCR3_I2C3_EN			    (1 << 31)
#define ADDR_SYSCFG_IOCR4                           (HW_SYSCFG_BASE + 0x020)
#define	  SYSCFG_IOCR4_CAM_DRV_STGTH(x)  	    (((x) & 0x7) << 12)
#define	  SYSCFG_IOCR4_SD1_DAT_DRV_STGTH(x)  	    (((x) & 0x7) << 27)
#define	SYSCFG_IOCR4_SD3_DAT_DRV_STGTH(x)           (((x) & 0x7) << 6)
#define	SYSCFG_IOCR4_SD3_CLK_DRV_STGTH(x)  	    	(((x) & 0x7) << 9)


#define ADDR_SYSCFG_IOCR5                           (HW_SYSCFG_BASE + 0x024)
#define   SYSCFG_IOCR5_GPIO34_UARTA_OUT1N_MUX(x)    (((x) & 0x3) << 8)
#define	  SYSCFG_IOCR5_GPIO35_UARTA_OUT2N_MUX(x)    (((x) & 0x3) << 10)
#define   SYSCFG_IOCR5_GPIO15L_DRV_STGTH(x)	    (((x) & 0x7) << 14)
#define   SYSCFG_IOCR5_CAM_TRACE_EN	    	    (1 << 26)
#define	  SYSCFG_IOCR5_GPIO32_UARTA_DSR_MUX(x)		(((x) & 0x3) << 4)
#define	  SYSCFG_IOCR5_GPIO33_UARTA_DTR_MUX(x)		(((x) & 0x3) << 6)
#define	  SYSCFG_IOCR5_GPIO31_MUX(x)				(((x) & 0x3) << 2)
#define ADDR_SYSCFG_IOCR6                           (HW_SYSCFG_BASE + 0x028)
#define   SYSCFG_IOCR6_GPIO25_24_MUX	    	    (1 << 19)
#define   SYSCFG_IOCR6_SD2DATH_PULL_CTRL(x)	    (((x) & 0x3) << 20)
#define   SYSCFG_IOCR6_CAM_MODE(x)	    	    (((x) & 0x3) << 27)
#define	  SYSCFG_IOCR6_CAM2_CAM1_B	    	    (1 << 30)
#define	  SYSCFG_IOCR6_SD1_CLK_DRV_STGTH(x)  	    (((x) & 0x7) << 10)
#define ADDR_SYSCFG_IOCR7			    (HW_SYSCFG_BASE + 0x02c)
#define   SYSCFG_IOCR7_RFGPIO5_GPIO7_MUX	    (1 << 31)
#define ADDR_SYSCFG_V3DRSTR                         (HW_SYSCFG_BASE + 0x030)
#define   SYSCFG_V3DRSTR_RST			    (1 << 0)
#define ADDR_SYSCFG_DSICR			    (HW_SYSCFG_BASE + 0x038)
#define ADDR_SYSCFG_ANACR2			    (HW_SYSCFG_BASE + 0x088)
#define	  SYSCFG_MON5_MIC_PWR_DOWN	    	    (1 << 1)
#define ADDR_SYSCFG_AHB_CLK_GATE_MASK               (HW_SYSCFG_BASE + 0x0D8)
#define ADDR_SYSCFG_AHB_CLK_GATE_FORCE              (HW_SYSCFG_BASE + 0x0DC)
#define ADDR_SYSCFG_AHB_CLK_GATE_MONITOR_RAW        (HW_SYSCFG_BASE + 0x0E0)
#define ADDR_SYSCFG_AHB_CLK_GATE_MONITOR            (HW_SYSCFG_BASE + 0x0E4)
#define ADDR_SYSCFG_VIDEO_CODEC_AHB_CLK_EN          (HW_SYSCFG_BASE + 0x100)
#define ADDR_SYSCFG_CAMARA_INTERFACE_AHB_CLK_EN     (HW_SYSCFG_BASE + 0x104)
#define ADDR_SYSCFG_USB_AHB_CLK_EN                  (HW_SYSCFG_BASE + 0x108)
#define ADDR_SYSCFG_GEA_AHB_CLK_EN                  (HW_SYSCFG_BASE + 0x10C)
#define ADDR_SYSCFG_CRYPTO_AHB_CLK_EN               (HW_SYSCFG_BASE + 0x110)
#define ADDR_SYSCFG_PKA_AHB_CLK_EN                  (HW_SYSCFG_BASE + 0x114)
#define ADDR_SYSCFG_UARTA_AHB_CLK_EN                (HW_SYSCFG_BASE + 0x118)
#define ADDR_SYSCFG_UARTB_AHB_CLK_EN                (HW_SYSCFG_BASE + 0x11C)
#define ADDR_SYSCFG_DA_AHB_CLK_EN                   (HW_SYSCFG_BASE + 0x120)
#define ADDR_SYSCFG_MPCLK_AHB_CLK_EN                (HW_SYSCFG_BASE + 0x124)
#define ADDR_SYSCFG_LCD_AHB_CLK_EN                  (HW_SYSCFG_BASE + 0x12C)
#define ADDR_SYSCFG_V3D_AHB_CLK_EN                  (HW_SYSCFG_BASE + 0x130)
#define ADDR_SYSCFG_DMAC_AHB_CLK_EN                 (HW_SYSCFG_BASE + 0x134)
#define ADDR_SYSCFG_SDIO1_AHB_CLK_EN                (HW_SYSCFG_BASE + 0x138)
#define ADDR_SYSCFG_SDIO2_AHB_CLK_EN                (HW_SYSCFG_BASE + 0x13C)
#define ADDR_SYSCFG_DES_AHB_CLK_EN                  (HW_SYSCFG_BASE + 0x144)
#define ADDR_SYSCFG_UARTC_AHB_CLK_EN                (HW_SYSCFG_BASE + 0x14C)
#define ADDR_SYSCFG_RNG_AHB_CLK_EN                  (HW_SYSCFG_BASE + 0x150)
#define ADDR_SYSCFG_SDIO3_AHB_CLK_EN                (HW_SYSCFG_BASE + 0x154)
#define ADDR_SYSCFG_FSUSBHOST_AHB_CLK_EN            (HW_SYSCFG_BASE + 0x15C)
#define ADDR_SYSCFG_MPHI_AHB_CLK_EN                 (HW_SYSCFG_BASE + 0x160)
#define ADDR_SYSCFG_DMAC_AHB_CLK_MODE               (HW_SYSCFG_BASE + 0x164)
#define ADDR_SYSCFG_HUCM_FW_CLK_EN                  (HW_SYSCFG_BASE + 0x168)
#define ADDR_SYSCFG_HTM_CLK_EN                      (HW_SYSCFG_BASE + 0x16C)
#define SYSCFG_AHB_PER_CLK_EN			    (1)
#define ADDR_SYSCFG_TESTABILITY_ACCESS              (HW_SYSCFG_BASE + 0x170)
#define ADDR_SYSCFG_DISABLE_OTP_REGION_READ_ACCESS  (HW_SYSCFG_BASE + 0x174)
#define ADDR_SYSCFG_DISABLE_OTP_REGION_WRITE_ACCESS (HW_SYSCFG_BASE + 0x178)
#define ADDR_SYSCFG_OTP_DEVICE_STATUS               (HW_SYSCFG_BASE + 0x17C)
#define ADDR_SYSCFG_IRDROP_MON3                     (HW_SYSCFG_BASE + 0x180)
#define ADDR_SYSCFG_IRDROP_MON4                     (HW_SYSCFG_BASE + 0x184)
#define ADDR_SYSCFG_IRDROP_MON5                     (HW_SYSCFG_BASE + 0x188)
#define ADDR_SYSCFG_IRDROP_MON6                     (HW_SYSCFG_BASE + 0x18C)
#define ADDR_SYSCFG_CIPHER_FW_CLK_EN                (HW_SYSCFG_BASE + 0x190)
#define ADDR_SYSCFG_SYSCONF_AHB_CLK_EXTEND0         (HW_SYSCFG_BASE + 0x1A0)
#define ADDR_SYSCFG_SYSCONF_AHB_CLK_EXTEND1         (HW_SYSCFG_BASE + 0x1A4)
#define ADDR_SYSCFG_OTP_CHIP_FEATURE_ID             (HW_SYSCFG_BASE + 0x1C0)
#define ADDR_SYSCFG_OTP_WCDMA_CAT                   (HW_SYSCFG_BASE + 0x1C4)
#define ADDR_SYSCFG_OTP_MM_FEAT_CFG                 (HW_SYSCFG_BASE + 0x1C8)
#define ADDR_SYSCFG_OTP_MM_FEAT_DIS                 (HW_SYSCFG_BASE + 0x1CC)
#define ADDR_SYSCFG_BRIDGE_INCR_EN                  (HW_SYSCFG_BASE + 0x1E0)
#define ADDR_SYSCFG_FPGA_VER                        (HW_SYSCFG_BASE + 0x1FC)
#define ADDR_SYSCFG_DP_AHB_CLK_EN                   (HW_SYSCFG_BASE + 0x200)
#define ADDR_SYSCFG_IOCR10                          (HW_SYSCFG_BASE + 0x204)
#define   SYSCFG_IOCR10_DEC_SYNC_BRG_INCR_TO_INCR4  (1 << 27)
#define   SYSCFG_IOCR10_ENC_SYNC_BRG_INCR_TO_INCR4  (1 << 26)
#define   SYSCFG_IOCR10_BSC3_GPIOH_ENABLE  			(1 << 29)
#define ADDR_SYSCFG_AP_L2C_IDLE_STAT                (HW_SYSCFG_BASE + 0x220)
#define ADDR_SYSCFG_V3D_X2H_INCR4_LMT_EN            (HW_SYSCFG_BASE + 0x240)
#define ADDR_SYSCFG_V3D_X2H_IDMON_EN                (HW_SYSCFG_BASE + 0x244)

#define ADDR_SYSCFG_IOCR3_PHYS                      \
	(BCM21553_SYSCFG_BASE + 0x01C)
#define ADDR_SYSCFG_IOCR6_PHYS                      \
	(BCM21553_SYSCFG_BASE + 0x028)
#define ADDR_SYSCFG_AHB_CLK_GATE_MASK_PHYS          \
	(BCM21553_SYSCFG_BASE + 0x0D8)
#define ADDR_SYSCFG_UARTA_AHB_CLK_EN_PHYS           \
	(BCM21553_SYSCFG_BASE + 0x118)

#define GPIO_GPOPS0_OFF                0x10
#define GPIO_GPISR0                    0x50
#define GPIO_GPISR1                    0x54

/*
GPIO I/O Type Register 
Each GPIO pin can be defined to operate in one of the following ways:
00 General-purpose input pin without interrupt(pin appears to be tristated)
01 General-purpose input pin with interrupt (pin apperars to be tristated)
10 General-purpose output pin
11 not used
*/
#define ADDR_GPIO_IOTR0				(HW_GPIO_BASE + 0x000) //0x088CE000  GPIO 0 - 15
#define ADDR_GPIO_IOTR1				(HW_GPIO_BASE + 0x004) //0x088CE004  GPIO16 - 31
#define ADDR_GPIO_IOTR2				(HW_GPIO_BASE + 0x008) //0x088CE008  GPIO32 - 47
#define ADDR_GPIO_IOTR3				(HW_GPIO_BASE + 0x00C) //0x088CE00C  GPIO48 - 63

/*
GPIO General-Purpose Output Register 
0 - Drive Low
1 - Drive High
*/
#define ADDR_GPIO_GPOR0			(HW_GPIO_BASE + 0x010) //0x088CE010 GPIO 0 -31
#define ADDR_GPIO_GPOR1			(HW_GPIO_BASE + 0x014) //0x088CE014 GPIO 0 - 63

/*
 * Register bit defines
 */
 // GPIO pull up/down enable (0 : disable, 1 : enable)
#define ADDR_GPIO_GPIPEN0			(HW_GPIO_BASE + 0x020) //0x088CE020 GPIO 0 - 31
#define ADDR_GPIO_GPIPEN1			(HW_GPIO_BASE + 0x024) //0x088CE024 GPIO 32 - 63

/* PERIPH_AHB_CLK_GATE_MON register bit defines */
#define PERIPH_AHB_CLK_GATE_MON_DMAC         (1 << 4)

/* SYSCFG_IOCR6 bits */
#define SYSCFG_IOCR6_GPIO23_MUX              (1 << 6)
#define SYSCFG_IOCR6_GPIO22_MUX              (1 << 5)

/*SYSCFG_IOCR10 bits*/
#define V3D_PREADY_FORCE_LOW				(1 << 19)

/*
 * SYSCONF_AHB_CLK_EXTEND0/1
 * Field encoding:
 * 00 - no clock request extension
 * 01 - clock request extended by 8 AHB-L cycles
 * 10 - clock request extended by 16 AHB-L cycles
 * 11 - clock request extended by 32 AHB-L cycles
 */
#define NO_CLOCK_REQUEST	0x0
#define EXT_BY_8AHB_L_CYC	0x1
#define EXT_BY_16AHB_L_CYC	0x2
#define EXT_BY_32AHB_L_CYC	0x3


#endif /*__REG_BCM21553_SYSCFG_H__*/
