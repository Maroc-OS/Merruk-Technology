/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/mfd/bcm59038/bcm59038.h
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

/**
*
*   @file   bcm59038.h
*
*   @brief  Core/Protocol driver for Broadcom BCM59038 PMU Chip
*
****************************************************************************/

#ifndef __BCM59038_H__
#define __BCM59038_H__

#include <linux/mfd/bcm59038/bcm59038-private.h>

/* BCM59038 Regulator IDs */
enum {
	BCM59038_REGL_ALDO1 = 0,
	BCM59038_REGL_ALDO2,
	BCM59038_REGL_ALDO3,
	BCM59038_REGL_ALDO4,
	BCM59038_REGL_ALDO5,
	BCM59038_REGL_ALDO6,
	BCM59038_REGL_ALDO7,
	BCM59038_REGL_ALDO8,

	BCM59038_REGL_DLDO1,
	BCM59038_REGL_DLDO2,
	BCM59038_REGL_DLDO3,
	BCM59038_REGL_DLDO4,

	BCM59038_REGL_SDLDO,

	BCM59038_REGL_LVLDO,

	BCM59038_REGL_SIMLDO,
};

enum {
	PONKEYB_NO_DB = 0,
	PONKEYB_50_DB,
	PONKEYB_100_DB,
	PONKEYB_500_DB,
	PONKEYB_1000_DB,
	PONKEYB_2000_DB,
	PONKEYB_3000_DB,
	PONKEYB_MAX_DB
};

/* BCM59038 PMU REGISTERS MAPPING -------------------------------------------------------- */
#define BCM59038_REG_I2CCNTRL      0x00

#define BCM59038_REG_HOSTACT       	0x01
#define BCM59038_REG_SMPLSET       	0x02
#define BCM59038_REG_PMUGID        	0x03

#define BCM59038_REG_PLLCTRL       	0x0B
#define BCM59038_REG_PONKEYBDB     	0x0C
#define BCM59038_REG_ACDDB        	0x0D
#define BCM59038_REG_PONKEYBDB1      0x0E

#define BCM59038_REG_CMPCTRL1        0x10
#define BCM59038_REG_CMPCTRL2        0x11
#define BCM59038_REG_CMPCTRL3        0x12
#define BCM59038_REG_CMPCTRL4        0x13
#define BCM59038_REG_CMPCTRL5        0x14
#define BCM59038_REG_CMPCTRL6        0x15
#define BCM59038_REG_CMPCTRL7        0x16
#define BCM59038_REG_CMPCTRL8        0x17
#define BCM59038_REG_CMPCTRL9        0x18
#define BCM59038_REG_CMPCTRL10       0x19
#define BCM59038_REG_CMPCTRL11       0x1A

#define BCM59038_REG_RTCSC        	0x20
#define BCM59038_REG_RTCMN        	0x21
#define BCM59038_REG_RTCHR        	0x22
#define BCM59038_REG_RTCWD        	0x23
#define BCM59038_REG_RTCDT        	0x24
#define BCM59038_REG_RTCMT        	0x25
#define BCM59038_REG_RTCYR        	0x26
#define BCM59038_REG_RTCSC_A1        0x27
#define BCM59038_REG_RTCMN_A1        0x28
#define BCM59038_REG_RTCHR_A1        0x29
#define BCM59038_REG_RTCWD_A1        0x2A
#define BCM59038_REG_RTCDT_A1        0x2B
#define BCM59038_REG_RTCMT_A1        0x2C
#define BCM59038_REG_RTCYR_A1        0x2D
#define BCM59038_REG_RTC_EXSAVE      0x2E
#define BCM59038_REG_BBCCTRL        	0x2F
#define BCM59038_REG_INT1        	0x30
#define BCM59038_REG_INT2        	0x31
#define BCM59038_REG_INT3        	0x32
#define BCM59038_REG_INT4        	0x33
#define BCM59038_REG_INT5        	0x34
#define BCM59038_REG_INT6        	0x35
#define BCM59038_REG_INT7        	0x36
#define BCM59038_REG_INT8        	0x37
#define BCM59038_REG_INT9        	0x38
#define BCM59038_REG_INT10			0x39

#define BCM59038_REG_INT1M        	0x3A
#define BCM59038_REG_INT2M        	0x3B
#define BCM59038_REG_INT3M        	0x3C
#define BCM59038_REG_INT4M        	0x3D
#define BCM59038_REG_INT5M        	0x3E
#define BCM59038_REG_INT6M        	0x3F
#define BCM59038_REG_INT7M        	0x40
#define BCM59038_REG_INT8M        	0x41
#define BCM59038_REG_INT9M        	0x42
#define BCM59038_REG_INT10M          0x43

#define BCM59038_REG_OTGCTRL1        0x44
#define BCM59038_REG_OTGCTRL2        0x45
#define BCM59038_REG_OTGCTRL3        0x46
#define BCM59038_REG_OTGCTRL4        0x47
#define BCM59038_REG_OTGCTRL5        0x48
#define BCM59038_REG_OTGCTRL6        0x49
#define BCM59038_REG_OTGCTRL7        0x4A
#define BCM59038_REG_OTGCTRL8        0x4B
#define BCM59038_REG_MBCCTRL1        0x4C
#define BCM59038_REG_MBCCTRL2        0x4D
#define BCM59038_REG_MBCCTRL3        0x4E
#define BCM59038_REG_MBCCTRL4        0x4F
#define BCM59038_REG_MBCCTRL5        0x50
#define BCM59038_REG_MBCCTRL6        0x51
#define BCM59038_REG_MBCCTRL7        0x52
#define BCM59038_REG_MBCCTRL8        0x53
#define BCM59038_REG_MBCCTRL9        0x54
#define BCM59038_REG_MBCCTRL10       0x55

#define BCM59038_REG_MBCCTRL11       0x56
#define BCM59038_REG_MBCCTRL12       0x57
#define BCM59038_REG_MBCCTRL13       0x58
#define BCM59038_REG_MBCCTRL14       0x59
#define BCM59038_REG_MBCCTRL15       0x5A
#define BCM59038_REG_MBCCTRL16       0x5B

#define BCM59038_REG_A1OPMODCTRL     0x60
#define BCM59038_REG_A2OPMODCTRL     0x61
#define BCM59038_REG_A3OPMODCTRL     0x62
#define BCM59038_REG_A4OPMODCTRL     0x63
#define BCM59038_REG_A5OPMODCTRL     0x64
#define BCM59038_REG_A6OPMODCTRL     0x65
#define BCM59038_REG_A7OPMODCTRL     0x66
#define BCM59038_REG_A8OPMODCTRL     0x67
#define BCM59038_REG_D1OPMODCTRL     0x68
#define BCM59038_REG_D2OPMODCTRL     0x69
#define BCM59038_REG_D3OPMODCTRL     0x6A
#define BCM59038_REG_D4OPMODCTRL     0x6B
#define BCM59038_REG_SDOPMODCTRL     0x6C
#define BCM59038_REG_LVOPMODCTRL     0x6D
#define BCM59038_REG_SIMOPMODCTRL    0x6E
#define BCM59038_REG_CSROPMODCTRL    0x6F
#define BCM59038_REG_IOSROPMODCTRL   0x70
#define BCM59038_REG_ALDO1CTRL       0x71
#define BCM59038_REG_ALDO2CTRL       0x72
#define BCM59038_REG_ALDO3CTRL       0x73
#define BCM59038_REG_ALDO4CTRL       0x74
#define BCM59038_REG_ALDO5CTRL       0x75
#define BCM59038_REG_ALDO6CTRL       0x76
#define BCM59038_REG_ALDO7CTRL       0x77
#define BCM59038_REG_ALDO8CTRL       0x78
#define BCM59038_REG_DLDO1CTRL       0x79
#define BCM59038_REG_DLDO2CTRL       0x7A
#define BCM59038_REG_DLDO3CTRL       0x7B
#define BCM59038_REG_DLDO4CTRL       0x7C
#define BCM59038_REG_SDLDOCTRL       0x7D
#define BCM59038_REG_LVLDOCTRL       0x7E
#define BCM59038_REG_SIMLDOCTRL      0x7F
#define BCM59038_REG_CSRCTRL        	0x80
#define BCM59038_REG_IOSRCTRL        0x81
#define BCM59038_REG_CSRCTRL1        0x82
#define BCM59038_REG_CSRCTRL2        0x83
#define BCM59038_REG_CSRCTRL3        0x84
#define BCM59038_REG_CSRCTRL4        0x85
#define BCM59038_REG_CSRCTRL5        0x86
#define BCM59038_REG_CSRCTRL6        0x87
#define BCM59038_REG_CSRCTRL7        0x88

#define BCM59038_REG_IOSRCTRL1       0x8A
#define BCM59038_REG_IOSRCTRL2       0x8B
#define BCM59038_REG_IOSRCTRL3       0x8C
#define BCM59038_REG_IOSRCTRL4       0x8D

#define BCM59038_REG_ADCCTRL1        0x90
#define BCM59038_REG_ADCCTRL2        0x91
#define BCM59038_REG_ADCCTRL3        0x92
#define BCM59038_REG_ADCCTRL4        0x93
#define BCM59038_REG_ADCCTRL5        0x94
#define BCM59038_REG_ADCCTRL6        0x95
#define BCM59038_REG_ADCCTRL7        0x96
#define BCM59038_REG_ADCCTRL8        0x97
#define BCM59038_REG_ADCCTRL9        0x98
#define BCM59038_REG_ADCCTRL10       0x99
#define BCM59038_REG_ADCCTRL11       0x9A
#define BCM59038_REG_ADCCTRL12       0x9B
#define BCM59038_REG_ADCCTRL13       0x9C
#define BCM59038_REG_ADCCTRL14       0x9D
#define BCM59038_REG_ADCCTRL15       0x9E
#define BCM59038_REG_ADCCTRL16       0x9F
#define BCM59038_REG_ADCCTRL17       0xA0
#define BCM59038_REG_ADCCTRL18       0xA1
#define BCM59038_REG_ADCCTRL19       0xA2
#define BCM59038_REG_ADCCTRL20       0xA3
#define BCM59038_REG_ADCCTRL21       0xA4
#define BCM59038_REG_ADCCTRL22       0xA5
#define BCM59038_REG_ADCCTRL23       0xA6
#define BCM59038_REG_ADCCTRL24       0xA7
#define BCM59038_REG_ADCCTRL25       0xA8
#define BCM59038_REG_ADCCTRL26       0xA9
#define BCM59038_REG_ADCCTRL27       0xAA
#define BCM59038_REG_ADCCTRL28       0xAB
#define BCM59038_REG_ADCCTRL29       0xAC

#define BCM59038_REG_IHFTOP        	0xB0
#define BCM59038_REG_IHFBIASCLK      0xB1
#define BCM59038_REG_IHFLDO        	0xB2
#define BCM59038_REG_IHFPOP        	0xB3
#define BCM59038_REG_IHFRCCALI       0xB4
#define BCM59038_REG_IHFADCI        	0xB5
#define BCM59038_REG_IHFSSP        	0xB6
#define BCM59038_REG_IHFSSPTST       0xB7
#define BCM59038_REG_IHFANARAMP      0xB8
#define BCM59038_REG_IHFPGA1        	0xB9
#define BCM59038_REG_IHFPGA2        	0xBA
#define BCM59038_REG_IHFLOOPFILTER   0xBB
#define BCM59038_REG_IHFCOMPPD       0xBC
#define BCM59038_REG_IHFDRIVER       0xBD
#define BCM59038_REG_IHFFBAMP        0xBE
#define BCM59038_REG_IHFSTIN        	0xBF
#define BCM59038_REG_IHFSTO        	0xC0
#define BCM59038_REG_IHFSCDADCO      0xC1
#define BCM59038_REG_IHFRCCALO       0xC2
#define BCM59038_REG_IHFRCCALRAW1O   0xC3
#define BCM59038_REG_IHFRCCALRAW2O   0xC4
#define BCM59038_REG_IHFMISC			0xC5

#define BCM59038_REG_HSCP1			0xC8
#define BCM59038_REG_HSCP2			0xC9
#define BCM59038_REG_HSCP3			0xCA
#define BCM59038_REG_HSDRV        	0xCB
#define BCM59038_REG_HSLDO        	0xCC
#define BCM59038_REG_HSLF        	0xCD
#define BCM59038_REG_HSPGA1        	0xCE
#define BCM59038_REG_HSPGA2        	0xCF
#define BCM59038_REG_HSPGA3        	0xD0
#define BCM59038_REG_HSRC        	0xD1
#define BCM59038_REG_HSPUP1        	0xD2
#define BCM59038_REG_HSPUP2        	0xD3
#define BCM59038_REG_HSIST        	0xD4
#define BCM59038_REG_HSOC1        	0xD5
#define BCM59038_REG_HSOC2        	0xD6
#define BCM59038_REG_HSOC3        	0xD7
#define BCM59038_REG_HSOC4        	0xD8
#define BCM59038_REG_HSOUT1        	0xD9
#define BCM59038_REG_HSOUT2        	0xDA
#define BCM59038_REG_HSOUT3        	0xDB
#define BCM59038_REG_HSOUT4        	0xDC
#define BCM59038_REG_HSOUT5        	0xDD
#define BCM59038_REG_HSOUT6        	0xDE
#define BCM59038_REG_HSOUT7        	0xDF

#define BCM59038_REG_ENV1        	0xF0
#define BCM59038_REG_ENV2        	0xF1
#define BCM59038_REG_ENV3        	0xF2
#define BCM59038_REG_ENV4        	0xF3
#define BCM59038_REG_ENV5        	0xF4
#define BCM59038_REG_ENV6        	0xF5

#define BCM59038_REG_PDCMPSYN0       0xF7
#define BCM59038_REG_PDCMPSYN1       0xF8
#define BCM59038_REG_PDCMPSYN2       0xF9
#define BCM59038_REG_PDCMPSYN3       0xFA
#define BCM59038_REG_PDCMPSYN4       0xFB
#define BCM59038_REG_PDCMPSYN5       0xFC

/*PMU ID*/
#define BCM59038_REG_PMUID           0xFD

#define BCM59038_REG_PAGESEL         0xFF	/* Page select register is 0xFF in all modes */

/* Register bitmap defines */
#define BCM59038_I2CCNTRL_I2CHSEN			0x80
#define BCM59038_I2CCNTRL_I2CRDBLOCKEN		0x40
#define BCM59038_I2CCNTRL_I2C_HIGHSPEED		0x20
#define BCM59038_I2CCNTRL_I2CFILTEN			0x10
#define BCM59038_I2CCNTRL_I2CSLAVEID			0x0C
#define BCM59038_I2CCNTRL_FIFOFULL_R			0x02
#define BCM59038_I2CCNTRL_I2CRDBLOCK			0x01

#define BCM59038_HOSTACT_WDT_CLR				0x01
#define BCM59038_HOSTACT_WDT_ON			0x02
#define BCM59038_HOSTACT_HOSTDICOFF			0x04
#define BCM59038_HOSTACT_WDT_TIME			0x18

#define BCM59038_SMPLSET_SMPL_ON				0x01
#define BCM59038_SMPLSET_SMPL_TIME			0x0E

#define BCM59038_PMUGID_R					0xFF

#define	BCM59038_PONKEYBDB_BHOLD				0x07
#define	BCM59038_PONKEYBDB_BRF				0x38
#define	BCM59038_PONKEYBDB_KEYLOCK			0x40
#define BCM59035_PONKEYBDB_BRF_SHIFT			0x3

#define	BCM59038_ACDDB_INS_DB				0x03
#define	BCM59038_ACDDB_RM_DB					0x0C
#define	BCM59038_ACDDB_PONKEYDEL				0x70

#define	BCM59038_PONKLEYBDB1_OFF_HOLD		0x07

#define	BCM59038_CMPCTRL1_LOWBATCVS			(0x03 << 4)
#define BCM59038_CMPCTRL1_LOWBATCVS_3_6V		(0x01 << 4)	/* 3.6V */
#define	BCM59038_CMPCTRL2_MAINT_TH			(0x03 << 2)	/* MBMCVS */
#define BCM59038_CMPCTRL2_MAINT_4_10V			(0x03 << 2)	/* 4.10V */
#define	BCM59038_CMPCTRL10_DIE_WARM_TH		0x0C

#define	BCM59038_BBCCTRL_HOSTEN				0x01
#define	BCM59038_BBCCTRL_RES_SEL				0x06
#define	BCM59038_BBCCTRL_CUR_SEL				0x38
#define	BCM59038_BBCCTRL_VOL_SEL				0xC0

#define BCM59038_INT1_PONKEYBR			    0x01
#define BCM59038_INT1_PONKEYBF       		0x02
#define BCM59038_INT1_PONKEYBH       		0x04
#define BCM59038_INT1_SMPL	        		0x08
#define BCM59038_INT1_RTCA1          		0x10
#define BCM59038_INT1_RTC1S		    		0x20
#define BCM59038_INT1_RTC60S         		0x40
#define BCM59038_INT1_RTCADJ         		0x80

#define BCM59038_INT2_CHGINS			        0x01
#define BCM59038_INT2_CHGRM          		0x02
#define BCM59038_INT2_CHGOV         			0x04
#define BCM59038_INT2_EOC            		0x08
#define BCM59038_INT2_USBINS         		0x10
#define BCM59038_INT2_USBRM          		0x20
#define BCM59038_INT2_USBOV         			0x40
#define BCM59038_INT2_MBCCHGERR      		0x80

#define BCM59038_INT3_ACDINS			        0x01
#define BCM59038_INT3_ACDRM          		0x02
#define BCM59038_INT3_WALL_RESUME   			0x04
#define BCM59038_INT3_RTM_DATA_RDY      		0x08
#define BCM59038_INT3_RTM_WHILE_CONT    		0x10
#define BCM59038_INT3_RTM_UPPER_BOUND   		0x20
#define BCM59038_INT3_RTM_IGNORE     		0x40
#define BCM59038_INT3_RTM_OVERRIDDEN   		0x80

#define BCM59038_INT4_VBUS_VALID_F	        0x01
#define BCM59038_INT4_SES_A_VALID_F     		0x02
#define BCM59038_INT4_SES_B_END_F    		0x04
#define BCM59038_INT4_ID_INSERT        		0x08
#define BCM59038_INT4_VBUS_VALID_R      		0x10
#define BCM59038_INT4_SES_A_VALID_R     		0x20
#define BCM59038_INT4_SES_B_END_R    		0x40
#define BCM59038_INT4_ID_REMOVE      		0x80

#define BCM59038_INT5_ID_CHANGE		        0x01
#define BCM59038_INT5_CHGDET_DONE       		0x02
#define BCM59038_INT5_CHGDET_TIMEOUT 		0x04
#define BCM59038_INT5_RID_C_TO_FLOAT    		0x08
#define BCM59038_INT5_VBUS_LOWBOUND    		0x10
#define BCM59038_INT5_RESUME_VBUS      		0x20
#define BCM59038_INT5_AUD_HSAB_SHTCKT		0x40
#define BCM59038_INT5_AUD_IHFD_SHTCKT  		0x80

#define BCM59038_INT6_BBLOW			        0x01
#define BCM59038_INT6_MBC_TF          		0x02
#define BCM59038_INT6_CHGERR_DIS    			0x04
#define BCM59038_INT6_CHG_WDT_EXP       		0x08
#define BCM59038_INT6_IOSR_OVRI         		0x10
#define BCM59038_INT6_CSR_OVRI          		0x20
#define BCM59038_INT6_IOSR_OVRV     			0x40
#define BCM59038_INT6_CSR_OVROV	    		0x80

#define BCM59038_INT7_MBTEMP_FAULT	        0x01
#define BCM59038_INT7_MBTEMP_LOW        		0x02
#define BCM59038_INT7_MBTEMP_HIGH   			0x04
#define BCM59038_INT7_MBOV            		0x08
#define BCM59038_INT7_BATINS         		0x10
#define BCM59038_INT7_BATRM          		0x20
#define BCM59038_INT7_LOWBAT       			0x40
#define BCM59038_INT7_VERYLOWBAT      		0x80

/* ENV2 */
#define BCM59038_ENV2_PONKEYB           		0x01
#define BCM59038_ENV2_ACD            		0x02
#define BCM59038_ENV2_CHIP_TOOHOT       		0x08
#define BCM59038_ENV2_USBOV            		0x10
#define BCM59038_ENV2_P_UBPD_USBDET     		0x20
#define BCM59038_ENV2_TH_FOLD           		0x40
#define BCM59038_ENV2_MB_ERC            		0x80

/* ENV3 */
#define BCM59038_ENV3_MBPD					0x01
#define BCM59038_ENV3_MBTEMPLOW				0x02
#define BCM59038_ENV3_MBTEMPHIGH				0x04
#define BCM59038_ENV3_VERYLOWBAT				0x40
#define BCM59038_ENV3_P_BAT_RM				0x80

/* ENV4 */
#define BCM59038_ENV4_VBUS_VLD				0x01
#define BCM59038_ENV4_A_SESS_VLD				0x02
#define BCM59038_ENV4_B_SESS_END				0x04
#define BCM59038_ENV4_ID_IN					0x38
#define BCM59038_ENV4_OTGID					0x40

/* ENV5 */
#define BCM59038_ENV5_P_CGPD_ONOFF			0x01
#define BCM59038_ENV5_P_UBPD_ONOFF			0x02
#define BCM59038_ENV5_EOC					0x04
#define BCM59038_ENV5_MBUV					0x08
#define BCM59038_ENV5_MB_MAINT_V				0x10
#define BCM59038_ENV5_MBOV					0x20
#define BCM59038_ENV5_P_CGPD_CHG				0x40
#define BCM59038_ENV5_P_UBPD_CHG				0x80

/* ENV6 */
#define BCM59038_ENV6_MBC_ERR				0x01
#define BCM59038_ENV6_OFFVBUSB				0x02
#define BCM59038_ENV6_OTG_SHUTDOWN			0x04

/* OTGCTRL1 */
#define BCM59038_OTGCTRL1_EN_VB_PULSE 		0x01
#define BCM59038_OTGCTRL1_EN_PD_SRP   		0x02
#define BCM59038_OTGCTRL1_OFFVBUSb			0x04
#define BCM59038_OTGCTRL1_IDIN_EN			0x08
#define BCM59038_OTGCTRL1_VA_SESS_EN			0x10
#define BCM59038_OTGCTRL1_EN_VBUS_RBUS		0x20
#define BCM59038_OTGCTRL1_OTG_SHUTDOWN		0x40

/* OTGCTRL2 */
#define BCM59038_OTGCTRL2_VBUS_DEBOUNCE		0x03

/* OTGCTRL3 */
#define	BCM59038_OTGCTRL3_ID_REF1_LSB		0xFF

/* OTGCTRL4 */
#define	BCM59038_OTGCTRL4_ID_REF1_MSB		0x03
#define	BCM59038_OTGCTRL4_ID_REF2_LSB		0xFC

/* OTGCTRL5 */
#define	BCM59038_OTGCTRL5_ID_REF2_MSB		0x0F
#define	BCM59038_OTGCTRL5_ID_REF3_LSB		0xF0

/* OTGCTRL6 */
#define	BCM59038_OTGCTRL6_ID_REF3_MSB		0x3F
#define	BCM59038_OTGCTRL6_ID_REF4_LSB		0xC0

/* OTGCTRL7 */
#define	BCM59038_OTGCTRL7_ID_REF4_MSB		0xFF

/* OTGCTRL8 */
#define	BCM59038_OTGCTRL8_IDIN_SEL			0x1F
#define	BCM59038_OTGCTRL8_IDREF_SEL			0x60

/* Audio bitmaps */
/* IHFTOP register */
#define BCM59038_IHFTOP_IDDQ                    0x1
#define BCM59038_IHFTOP_BYPASS                  (1 << 6)
/* IHFLDO register */
#define BCM59038_IHFLDO_PUP                     0x1
/* IHFPOP register */
#define BCM59038_IHFPOP_AUTOSEC                 (1 << 6)
#define BCM59038_IHFPOP_BYPASS                  (1 << 5)
#define BCM59038_IHFPOP_FAST                    (1 << 4)
#define BCM59038_IHFPOP_PUP                     (1 << 3)
#define BCM59038_IHFPOP_BIASEN                  (1 << 2)
/* PLLCTRL register */
#define BCM59038_PLLCTRL_PLLEN                  (1 << 1)
/* HSPUP1 and HSPUP2 register */
#define BCM59038_HSPUP1_IDDQ_PWRDN              (1 << 4)
#define BCM59038_HSPUP2_PWRUP                   (1 << 6)
/* HSDRV register */
#define BCM59038_HSDRV_SC_ENABLE                (1 << 4)
#define BCM59038_HSDRV_SC_CURRENT_MASK  	0x3
/* HSPGA1 and HSPGA2 registers */
#define BCM59038_HSPGA1_HS_GAIN_R		(1 << 7)
#define BCM59038_HSPGA1_HS_GAIN_L		(1 << 6)
#define BCM59038_HSPGA1_GAINL_MASK		0x3F
#define BCM59038_HSPGA2_GAINR_MASK		0x3F

/* IHFPGA1 and IHFPGA2 registers */
#define BCM59038_IHFPGA2_GAIN_MASK		0x3F

/* SIMLDOCTRL */
#define BCM59038_SIMLDOCTRL_SIMMASK   		0x03
#define BCM59038_SIMLDOCTRL_SIM_3V3    		0x03
#define BCM59038_SIMLDOCTRL_SIM_3V0   		0x02
#define BCM59038_SIMLDOCTRL_SIM_2V5    		0x01
#define BCM59038_SIMLDOCTRL_SIM_1V8    		0x00

/* PONKEY bit fields */
#define BCM59038_PONKEYBDB_ONHOLD_MASK    	0x07
#define BCM59038_PONKEYBDB_PONKEYBRF_MASK 	0x38
#define BCM59038_PONKEYBDB_KEYLOCK        	0x40	/* bit value for keylock bit */
#define BCM59038_ACDDB_PONKEYBDEL_MASK    	0x70	/* shutdown delay mask */
#define BCM59038_ACDDB_PONKEYBDEL_MIN     	0x30	/* 2 sec min delay for keylock */
#define BCM59038_PONKEYBDB1_OFFHOLD_MASK  	0x07	/* bit value for keylock bit */

/* SAR ADC bit fields */
/* ADCCTRL1 */
enum {
	ADCCTRL1_RESET_COUNT_1 = 0,
	ADCCTRL1_RESET_COUNT_3,
	ADCCTRL1_RESET_COUNT_5,
	ADCCTRL1_RESET_COUNT_7
};

#define	BCM59038_ADCCTRL1_MAX_RTM_MASK		0x3	/* Max Reset Count within 20ms time period */
#define	BCM59038_ADCCTRL1_RTM_START		0x4	/* Start RTM conversion */
#define	BCM59038_ADCCTRL1_RTM_CH_MASK		0xF0	/* RTM Channel select mask */

#define BCM59038_ADCCTRL1_RTM_ENABLE		(0x1 << 3)	/* 0: Disable and 1: Enable */
#define BCM59038_ADCCTRL1_RTM_DISABLE		(~BCM59038_ADCCTRL1_RTM_ENABLE)

/* ADCCTRL2 */
enum {
	ADCCTRL2_RTM_DELAY_0_USEC = 0,
	ADCCTRL2_RTM_DELAY_31_25_USEC,
	ADCCTRL2_RTM_DELAY_62_50_USEC,
	ADCCTRL2_RTM_DELAY_93_75_USEC,

	ADCCTRL2_RTM_DELAY_125_USEC,
	ADCCTRL2_RTM_DELAY_156_25_USEC,
	ADCCTRL2_RTM_DELAY_187_50_USEC,
	ADCCTRL2_RTM_DELAY_218_75_USEC,

	ADCCTRL2_RTM_DELAY_250_USEC,
	ADCCTRL2_RTM_DELAY_281_25_USEC,
	ADCCTRL2_RTM_DELAY_312_50_USEC,
	ADCCTRL2_RTM_DELAY_343_75_USEC,

	ADCCTRL2_RTM_DELAY_375_USEC,
	ADCCTRL2_RTM_DELAY_406_25_USEC,
	ADCCTRL2_RTM_DELAY_437_50_USEC,
	ADCCTRL2_RTM_DELAY_468_75_USEC,

	ADCCTRL2_RTM_DELAY_500_USEC,
	ADCCTRL2_RTM_DELAY_531_25_USEC,
	ADCCTRL2_RTM_DELAY_562_50_USEC,
	ADCCTRL2_RTM_DELAY_593_75_USEC,

	ADCCTRL2_RTM_DELAY_625_USEC,
	ADCCTRL2_RTM_DELAY_656_25_USEC,
	ADCCTRL2_RTM_DELAY_687_50_USEC,
	ADCCTRL2_RTM_DELAY_718_75_USEC,

	ADCCTRL2_RTM_DELAY_750_USEC,
	ADCCTRL2_RTM_DELAY_781_25_USEC,
	ADCCTRL2_RTM_DELAY_812_50_USEC,
	ADCCTRL2_RTM_DELAY_843_75_USEC,

	ADCCTRL2_RTM_DELAY_875_USEC,
	ADCCTRL2_RTM_DELAY_906_25_USEC,
	ADCCTRL2_RTM_DELAY_937_50_USEC,
	ADCCTRL2_RTM_DELAY_968_75_USEC
};

#define	BCM59038_ADCCTRL2_RTM_DLY_MASK		0x1F	/* Time delay after conversion start */
#define BCM59038_ADCCTRL1_GSM_DEBOUNCE_EN	(0x1 << 5)
#define BCM59038_ADCCTRL1_GSM_DEBOUNCE_DIS	(~BCM59038_ADCCTRL1_GSM_DEBOUNCE_EN)

#define BCM59038_ADCDATA_INVALID		(0x1 << 2)
#define BCM59038_ADCDATA_MASK			(0x3 << 8)

/* SAR ADC Channel Select ENUM - to be used by user of ADC drivers */
typedef enum {
	SARADC_CH_VMBAT = 0,
	SARADC_CH_VBBAT,
	SARADC_CH_VWALL,
	SARADC_CH_VBUS,
	SARADC_CH_ID,
	SARADC_CH_NTC,
	SARADC_CH_BSI,
	SARADC_CH_BOM,
	SARADC_CH_32KTEMP,
	SARADC_CH_PATEMP,
	SARADC_CH_ALS,
	SARADC_CH_FEM,
	/* Rest 4 are not implemented */
} BCM59038_SARADC_SELECT;

/* HOSTACT */
#define BCM59038_HOSTACT_WDT_CLR     		0x01
#define BCM59038_HOSTACT_WDT_ON      		0x02
#define BCM59038_HOSTACT_HOSTDICOFF  		0x04

/* ****************************************************************************** */
/* Typedefs */
/* ****************************************************************************** */

/* PMU Interrupts IDs  --------------------------------------------------------------- */
typedef enum {
	BCM59038_IRQID_INT1_PONKEYBR,	/* R&C  on key rising */
	BCM59038_IRQID_INT1_PONKEYBF,	/* R&C  on key falling */
	BCM59038_IRQID_INT1_PONKEYBH,	/* R&C  on key pressed > hold debounce time */
	BCM59038_IRQID_INT1_SMPL,	/* R&C  one minute elapsed */
	BCM59038_IRQID_INT1_RTCA1,	/* R&C  alarm 1 */
	BCM59038_IRQID_INT1_RTC1S,	/* R&C  SMPL */
	BCM59038_IRQID_INT1_RTC60S,	/* R&C  real-time clock needs adjustment */
	BCM59038_IRQID_INT1_RTCADJ,	/* R&C  one second elapsed */

	BCM59038_IRQID_INT2_CHGINS,	/* R&C  wall charger inserted (high clears CHGRM) */
	BCM59038_IRQID_INT2_CHGRM,	/* R&C  wall charger removed (high clears CHGINS) */
	BCM59038_IRQID_INT2_CHGOV,	/* R&C  wall charger error (V too high) */
	BCM59038_IRQID_INT2_CHGEOC,	/* R&C  wall/usb charging done */
	BCM59038_IRQID_INT2_USBINS,	/* R&C  usb charger inserted (high clears USBRM) */
	BCM59038_IRQID_INT2_USBRM,	/* R&C  wall charger removed (high clears USBINS) */
	BCM59038_IRQID_INT2_USBOV,	/* R&C  usb charger error (V too high) */
	BCM59038_IRQID_INT2_MBCCHGERR,	/* R&C  main battery charge error (over time) */

	BCM59038_IRQID_INT3_ACDINS,	/* R&C  accessory inserted (high clears ACDRM) */
	BCM59038_IRQID_INT3_ACDRM,	/* R&C  accessory removed (high clears ACDINS) */
	BCM59038_IRQID_INT3_RESUME_VWALL,	/* R&C  Resume Wall charging */
	BCM59038_IRQID_INT3_RTM_DATA_RDY,	/* R&C  Real time measurement data ready interrupt */
	BCM59038_IRQID_INT3_RTM_INTR_CONT,	/* R&C  RTM request while continuous measurement is running */
	BCM59038_IRQID_INT3_RTM_MAX_REQ,	/* R&C  RTM request reached maximum limit */
	BCM59038_IRQID_INT3_RTM_IGNORE,	/* R&C  RTM request ignored */
	BCM59038_IRQID_INT3_RTM_OVERRIDE,	/* R&C  RTM rquest over-ridden */

	BCM59038_IRQID_INT4_VBUSVALID_F,	/* R&C  Vbus valid falling edge */
	BCM59038_IRQID_INT4_A_SESSVALID_F,	/* R&C  A session valid falling edge */
	BCM59038_IRQID_INT4_B_SESSEND_F,	/* R&C  B session end falling edge */
	BCM59038_IRQID_INT4_ID_INSRT,	/* R&C  ID inserted */
	BCM59038_IRQID_INT4_VBUSVALID_R,	/* R&C  Vbus valid rising edge */
	BCM59038_IRQID_INT4_A_SESSVALID_R,	/* R&C  A session valid rising edge */
	BCM59038_IRQID_INT4_B_SESSEND_R,	/* R&C  B session end rising edge */
	BCM59038_IRQID_INT4_ID_RMV,	/* R&C  ID removed */

	BCM59038_IRQID_INT5_ID_CHANGE,	/* R&C ID change interrupt */
	BCM59038_IRQID_INT5_CHGDET_SAMPLED,	/* R&C BC1.1 charger detect sampled */
	BCM59038_IRQID_INT5_CHGDET_TIMEOUT,	/* R&C BC1.1 charger detect time out */
	BCM59038_IRQID_INT5_RID_C2F,	/* R&C ID change from RID_C to Float */
	BCM59038_IRQID_INT5_VBUS_LOWBOUND,	/* R&C VBUS trip low bound */
	BCM59038_IRQID_INT5_RESUME_VBUS,	/* R&C resume VBUS interrupt */
	BCM59038_IRQID_INT5_AUD_HS_SHORT,	/* R&C headset short circuit */
	BCM59038_IRQID_INT5_AUD_D_SHORT,	/* R&C class D short circuit */

	BCM59038_IRQID_INT6_BBLOW,	/* R&C Backup battery low */
	BCM59038_IRQID_INT6_MBC_TF,	/* R&C Main battery thermal foldback */
	BCM59038_IRQID_INT6_CHGERRDIS,	/* R&C charger error disappear */
	BCM59038_IRQID_INT6_CHGWDT_EXP,	/* R&C charger watchdog expired */
	BCM59038_IRQID_INT6_CGPD_FALLING,	/* R&C wall charger CGPD falling */
	BCM59038_IRQID_INT6_UBPD_FALLING,	/* R&C USB charger UBPD falling */
	BCM59038_IRQID_INT6_RESERVED1,	/* R&C reserved */
	BCM59038_IRQID_INT6_RESERVED2,	/* R&C reserved */

	BCM59038_IRQID_INT7_MBTEMPFAULT,	/* R&C main battery temperature fault */
	BCM59038_IRQID_INT7_MBTEMPLOW,	/* R&C main battery temperature low */
	BCM59038_IRQID_INT7_MBTEMPHIGH,	/* R&C main battery temperature high */
	BCM59038_IRQID_INT7_MBOV,	/* R&C main batteyr over voltage */
	BCM59038_IRQID_INT7_BATINS,	/* R&C battery insertion */
	BCM59038_IRQID_INT7_BATRM,	/* R&C battery removal */
	BCM59038_IRQID_INT7_LOWBAT,	/* R&C low battery interrupt */
	BCM59038_IRQID_INT7_VERYLOWBAT,	/* R&C very low battery interrupt */

	BCM59038_TOTAL_IRQ
} BCM59038_InterruptId;

/* Main Battery Control Register - bit representation */

/* MBCCTRL1 bit details */
#define BCM59038_MBCCTRL1_TTR_DISABLE		0x7	/* Trickle charging timer Disable */
#define BCM59038_MBCCTRL1_TCH_DISABLE		(0x7 << 3)	/* disable TCH charge timer */
#define BCM59038_MBCCTRL1_WAC_ERCEN		(0x1 << 6)	/* enable the early rapid charge for WALL charger */
#define BCM59038_MBCCTRL1_USB_ERCEN		(0x1 << 7)	/* enable the early rapid charge for USB charger */

/* MBCCTRL2 bit details */
#define BCM59038_MBCCTRL2_WACHOSTEN		0x1	/* MBC enable from host for WALL charger */
#define BCM59038_MBCCTRL2_USBHOSTEN		(0x1 << 1)	/* MBC enable from host for USb charger */
#define BCM59038_MBCCTRL2_VCHGR_FC2		(0x1 << 2)	/* enable WALL charger FC2 state from the host */
#define BCM59038_MBCCTRL2_VUBGR_FC2		(0x1 << 3)	/* enable USB charger FC2 state from the host */
#define BCM59038_MBCCTRL2_WAC_FC_OPTION		(0x1 << 4)	/* WALL charger regulates the battery voltage at FC1 voltage level */
#define BCM59038_MBCCTRL2_USB_FC_OPTION		(0x1 << 5)	/* USB charger regulates the battery voltage at FC1 voltage level */
#define BCM59038_MBCCTRL2_MAINTCHRG		(0x1 << 6)	/* enable the Maintenance charge */
#define BCM59038_MBCCTRL2_PAUSE_CHARGE		(0x1 << 7)	/* enable pause charge */

/* MBCCTRL3 and MBCCTRL5 FC1/FC2 voltage table for WAC and USB */
enum {
	BCM59038_FC1_FC2_CV_3_60V = 0,
	BCM59038_FC1_FC2_CV_3_70V,
	BCM59038_FC1_FC2_CV_4_00V,
	BCM59038_FC1_FC2_CV_4_05V,
	BCM59038_FC1_FC2_CV_4_10V,
	BCM59038_FC1_FC2_CV_4_15V,
	BCM59038_FC1_FC2_CV_4_20V,
	BCM59038_FC1_FC2_CV_4_25V
};

/* MBCCTRL4, MBCCTRL6 and MBCCTRL11 FC2/FC1/QC charging current selection table for WAC and USB */
enum {
	BCM59038_FC2_FC1_CC_50MA = 0,
	BCM59038_FC2_FC1_CC_60MA,
	BCM59038_FC2_FC1_CC_70MA,
	BCM59038_FC2_FC1_CC_80MA,
	BCM59038_FC2_FC1_CC_90MA,
	BCM59038_FC2_FC1_CC_100MA,
	BCM59038_FC2_FC1_CC_110MA,
	BCM59038_FC2_FC1_CC_120MA,
	BCM59038_FC2_FC1_CC_130MA,
	BCM59038_FC2_FC1_CC_140MA,
	BCM59038_FC2_FC1_CC_150MA,
	BCM59038_FC2_FC1_CC_160MA,
	BCM59038_FC2_FC1_CC_170MA,
	BCM59038_FC2_FC1_CC_180MA,
	BCM59038_FC2_FC1_CC_190MA,
	BCM59038_FC2_FC1_CC_200MA,
	BCM59038_FC2_FC1_CC_250MA,
	BCM59038_FC2_FC1_CC_300MA,
	BCM59038_FC2_FC1_CC_350MA,
	BCM59038_FC2_FC1_CC_400MA,
	BCM59038_FC2_FC1_CC_450MA,
	BCM59038_FC2_FC1_CC_500MA,
	BCM59038_FC2_FC1_CC_550MA,
	BCM59038_FC2_FC1_CC_600MA,
	BCM59038_FC2_FC1_CC_650MA,
	BCM59038_FC2_FC1_CC_700MA,
	BCM59038_FC2_FC1_CC_750MA,
	BCM59038_FC2_FC1_CC_800MA,
	BCM59038_FC2_FC1_CC_R1,	/* reserved 1 */
	BCM59038_FC2_FC1_CC_R2,	/* resereved 2 */
	BCM59038_FC2_FC1_CC_R3,	/* reserverd 3 */
	BCM59038_FC2_FC1_CC_1000MA	/* for collapsing charger */
};

#define BCM59038_FC2_FC1_CC_MASK		0x1F
#define BCM59038_MBCCTRL4_WAC_QC_CC		(0x1 << 5)	/* WAC QC charging current selection (to be combined with MBCCTRL11[3:0]) */
#define BCM59038_MBCCTRL4_USB_QC_CC		(0x1 << 6)	/* USB QC charging current selection (to be combined with MBCCTRL11[7:4]) */

/* MBCCTRL7 EOC CC Threshold Table */
enum {
	BCM59038_EOCS_50MA = 0,
	BCM59038_EOCS_60MA,
	BCM59038_EOCS_70MA,
	BCM59038_EOCS_80MA,
	BCM59038_EOCS_90MA,
	BCM59038_EOCS_100MA,
	BCM59038_EOCS_110MA,
	BCM59038_EOCS_120MA,
	BCM59038_EOCS_130MA,
	BCM59038_EOCS_140MA,
	BCM59038_EOCS_150MA,
	BCM59038_EOCS_160MA,
	BCM59038_EOCS_170MA,
	BCM59038_EOCS_180MA,
	BCM59038_EOCS_190MA,
	BCM59038_EOCS_200MA
};
#define BCM59038_MBCCTRL7_EOCS_MASK		0x0F

/* MBCCTRL8 bit operations */
#define BCM59038_MBCCTRL8_MBCFAULTCNT		0x1	/* Possible value >=1 and <=15 (0 = 1) */
#define BCM59038_MBCCTRL8_MBCFAULTCNT_MASK	0x0F
/* Time setting table for Digital soft-start */
enum {
	BCM59038_DSS_TIME_OFF = 0x0,
	BCM59038_DSS_TIME_20MS,
	BCM59038_DSS_TIME_50MS,
	BCM59038_DSS_TIME_100MS
};
#define BCM59038_MBCCTRL8_DSS_TYP		(0x1 << 6)	/* Enable DSS when MBRDY or HOSTON state */

/* MBCCTRL9 bit operations */
#define BCM59038_MBCCTRL9_CON_TYP		(0x1 << 1)	/* Connector type. Read Only Bit */
#define BCM59038_MBCCTRL9_SP_TYP_WAC		(0x1 << 2)	/* Single connector type. Bit is valid when CON_TYP=1 (0=USB and 1=WALL) */

#define BCM59038_MBCCTRL9_SP_TYP_USB		(~BCM59038_MBCCTRL9_SP_TYP_WAC)

#define BCM59038_MBCCTRL9_ADP_PRI_WAC		(0x1 << 3)	/* Adapter priority bit. Valid when CON_TYP =0. (0=USB and 1=WALL) */

#define BCM59038_MBCCTRL9_ADP_PRI_USB		(~BCM59038_MBCCTRL9_ADP_PRI_WAC)

/* MBCCTRL10 bit operations */
/* The expiration time setting Table of the charger watchdog timer */
enum {
	BCM59038_CHRG_WDT_256S = 0,
	BCM59038_CHRG_WDT_128S,
	BCM59038_CHRG_WDT_64S,
	BCM59038_CHRG_WDT_32S
};
#define BCM59038_MBCCTRL10_CHRG_WDT_CLR		(0x1 << 3)	/* Clear Charger WD timer */
#define BCM59038_MBCCTRL10_TF_CTRL		(0x1 << 5)	/* software control for TF_EN */
#define BCM59038_MBCCTRL10_TF_SW_EN		(0x1 << 6)	/* enable he TF_EN by software */

/* MBCCTRL13 bit operations */
#define BCM59038_MBCCTRL13_USB_DET_LDO_EN 	(0x1 << 2)	/* BCDLDO is turned on by SW */
#define BCM59038_MBCCTRL13_DIS_RID_FLOAT	(0x1 << 3)	/* USBC type detection is able to kick off even if */
enum {
	BCM59038_CHGTYP_NA = 0,
	BCM59038_CHGTYP_SHP,
	BCM59038_CHGTYP_CHP,
	BCM59038_CHGTYP_DC
};

/* MBCCTRL14 bit operations */
#define BCM59038_MBCCTRL14_CHG_EN_ATE		(0x1 << 1)	/* Enable WALL charger switcher in MBC ATE test mode */
#define BCM59038_MBCCTRL14_USB_EN_ATE		(0x1 << 2)	/* Enable USB charger switcher in MBC ATE test mode */
#define BCM59038_MBCCTRL14_MBCEN_ATE		(0x1 << 3)	/* Enable analog MBC circuit in MBC ATE test mode */
#define BCM59038_MBCCTRL14_MBC_SS_ATE		(0x1 << 4)	/* Enable the analog MBC soft-start circuit in same mode */
#define BCM59038_MBCCTRL14_BCDEN_ATE		(0x1 << 5)	/* Enable the BCDLDO in MBC ATE test mode */
#define BCM59038_MBCCTRL14_TF_EN_ATE		(0x1 << 6)	/* Enable the analog thermal foldback circuit */

/* ENV1 bit operations */
#define BCM59038_ENV1_UBMBC          		0x01	/* Usb voltage higher than MB voltage */
#define BCM59038_ENV1_P_CGPD_ENV     		(0x01 << 1)	/* WAC presence detected */
#define BCM59038_ENV1_P_UBPD_ENV     		(0x01 << 2)	/* USB presence detected */
#define BCM59038_ENV1_MBWV           		(0x01 << 3)	/* MB voltage is higher than the working voltage */
#define BCM59038_ENV1_CHGOV          		(0x01 << 4)	/* WAC input over-voltage */
#define BCM59038_ENV1_LOWBAT         		(0x01 << 5)	/* Low main battery comparator */
#define BCM59038_ENV1_BBATUVB        		(0x01 << 6)	/* Low Backup battery level detected */
#define BCM59038_ENV1_CGMBC			(0x01 << 7)	/* WAC voltage is higher than MB voltage */

/* Main Battery Control Register - end */

typedef enum {
	PMU_PAGE_0 = 0x00,
	PMU_PAGE_1,
	PMU_PAGE_2,
} PMU_page;

extern int bcm59038_enable_irq(struct bcm59038 *bcm59038, int irq);
extern int bcm59038_disable_irq(struct bcm59038 *bcm59038, int irq);
extern int bcm59038_free_irq(struct bcm59038 *bcm59038, int irq);
extern int bcm59038_request_irq(struct bcm59038 *bcm59038, int irq,
				bool enable_irq, void (*handler) (int, void *),
				void *data);

extern int bcm59038_write_register(struct bcm59038 *bcm59038, u32 reg, u8 val);
extern int bcm59038_read_register(struct bcm59038 *bcm59038, u32 reg,
				  u8 *regVal);
extern int bcm59038_set_page(struct bcm59038 *bcm59038, PMU_page page);

extern void bcm59038_lock(struct bcm59038 *bcm59038);
extern void bcm59038_unlock(struct bcm59038 *bcm59038);
extern int bcm59038_register_ioctl_handler(struct bcm59038 *bcm59038,
					   u8 sub_dev_id,
					   pmu_subdev_ioctl_handler handler,
					   void *pri_data);

#endif /* __BCM59038_H__ */
