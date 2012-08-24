/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/csl/csl_lcdc.c	
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
*  @file   csl_lcdc.c
*
*  @brief  LCD Z80/M86/DBI-B/DBI-C(LCDC) CSL driver
*
*  @note   Platforms supported: ZEUS/ATHENA
****************************************************************************/

#if ( defined (_BCM21551_) || defined (_ATHENA_) )

#define LCDC_MAX_HANDLES                2
#define LCDC_USE_DYN_AHB_CLK
#undef  LCDC_USE_SYSCFG_OPEN_OS_RDB

#if ( !defined(BSP_PLUS_BUILD_INCLUDED) )
/* #define LCDC_PEDESTAL_CTRL */
#endif

#include <asm/io.h>
#include <mach/io.h>
#include <mach/hardware.h>
#include <linux/string.h>
#include <plat/types.h>
#include "plat/osabstract/ostypes.h"
#include "plat/osabstract/ostask.h"
#include "plat/osabstract/ossemaphore.h"
#include "plat/osabstract/osqueue.h"
#include <plat/dma_drv.h>
#include <plat/rdb/brcm_rdb_syscfg.h>
#include <plat/rdb/brcm_rdb_lcdc.h>
#include <plat/chal/chal_common.h>
#include <plat/chal/chal_lcdc.h>
#include <plat/csl/csl_lcd.h>
#include <plat/csl/csl_lcdc.h>

#ifdef LCDC_PEDESTAL_CTRL
#include "pedestal.h"
#endif

/* ############################################################################# */
/* CSL LCDC Specific */
/* ############################################################################# */

#if ( defined(_ATHENA_) && defined(LCDC_USE_DYN_AHB_CLK) )
/* SYSCFG_PERIPH_LCD_AHB_CLK_EN */
#define CSL_LCDC_ENA_AHB_CLK() *(volatile UInt32*) (io_p2v(0x0888012C)) = 1;
#define CSL_LCDC_DIS_AHB_CLK() *(volatile UInt32*) (io_p2v(0x0888012C)) = 0;
#else
#define CSL_LCDC_ENA_AHB_CLK()
#define CSL_LCDC_DIS_AHB_CLK()
#endif

typedef struct {
	CHAL_LCDC_DBI_CFG_t cfg;
	CHAL_LCDC_DBI_SPEED_t speed;
	CSL_LCDC_TE_T teCfg;
} CSL_DBI_CFG_T;

typedef struct {
	CHAL_LCDC_PAR_CFG_t cfg;
	CHAL_LCDC_PAR_SPEED_t speed;
	CSL_LCDC_TE_T teCfg;
} CSL_PAR_CFG_T;

typedef union {
	CSL_DBI_CFG_T ctrlDbi;
	CSL_PAR_CFG_T ctrlLeg;
} CSL_LCDC_CTRL_T;

typedef enum {
	CTRL_LEGACY = 1,
	CTRL_DBI = 2
} LCDC_CTRL_CLASS_T;

typedef struct {
	Boolean open;
	Boolean usesTE;
	Boolean usesCE;
#if (CHIP_REVISION >= 20)
	Boolean uses888U;
#endif
	Boolean hasLock;
	UInt32 buffBpp;
	CSL_LCD_BUS_CH_T csBank;
	LCDC_CTRL_CLASS_T ctrlClass;
	CSL_LCDC_CTRL_T config;
} LCDC_HANDLE_T, *LCDC_HANDLE;

typedef struct {
	Boolean init;
	UInt32 instance;
	CSL_LCD_HANDLE activeHandle;
	void *chalH;
	LCDC_HANDLE_T lcdc[LCDC_MAX_HANDLES];
} LCDC_DRV_st;

typedef struct {
	Dma_Buffer_List dmaBuffList;	/* /< local DMA config */
	Dma_Data dmaData;	/* /< local DMA config */
	Dma_Chan_Info dmaChInfo;	/* /< local DMA config */
	DMA_CHANNEL dmaCh;	/* /< local DMA config */
	LCDC_HANDLE lcdH;	/* /< LCDC handle */
	CSL_LCD_UPD_REQ_T updReq;	/* /< update Request */
} LCDC_UPD_REQ_MSG_T;

#define     FLUSH_Q_SIZE              1

/* move to \public\msconsts.h */
#define     TASKNAME_LCDC             (TName_t)"CSL_LCDC"

/* IDLE, LOWEST, BELOW_NORMAL, NORMAL, ABOVE_NORMAL, HIGHEST */
#define     TASKPRI_LCDC              (TPriority_t)(ABOVE_NORMAL)

#define     STACKSIZE_LCDC            STACKSIZE_BASIC*5

#define     LCDC_HISR_STACK_SIZE      256

static Queue_t updReqQ;
static Semaphore_t eofDmaSema;
static Semaphore_t lcdcSema;
static LCDC_DRV_st cslLcdcDrv;

#ifdef LCDC_PEDESTAL_CTRL
static UInt8 lcdc_pedestal_id = 0xff;
#endif

#define     INLINE                    __inline

extern int lcd_enable;

/* ***************************************************************************** */
/* Local Functions */
/* ***************************************************************************** */
static CSL_LCD_RES_T cslLcdcDmaStop(LCDC_UPD_REQ_MSG_T * updMsg);
#ifdef _ATHENA_
static void cslLcdcTeCfg(CSL_LCDC_TE_T te);
static void cslLcdcEnableTeGpio(UInt32 pinSel);
INLINE static void cslLcdcWaitWhileLcdcIsBusy(void);
#endif
static Boolean cslLcdcSetIoWidth(UInt32 busWidth);
static void cslLcdcSetIoEnable(Boolean enable);

INLINE static void cslLcdcWaitWhileFifoIsFull(void);
INLINE static void cslLcdcWaitUntilFifoIsEmpty(void);
INLINE static void cslLcdcWaitUntilFifoHasData(void);
INLINE static UInt32 cslLcdcRdFifoData(void);
INLINE static void cslLcdcParRdReqCycleD(void);
INLINE static void cslLcdcParRdReqCycleCx(void);
static void cslLcdcDbiSelect(LCDC_HANDLE lcdc);
static void cslLcdcParSelect(LCDC_HANDLE lcdc);
INLINE static void cslLcdcSwitchBank(LCDC_HANDLE lcdc);

/* ############################################################################# */

/* ############################################################################# */

/* ***************************************************************************** */
/* Function Name: cslLcdcGetBitFieldVal */
/* Description:   Returns bit-field value based on bit mask & 0-based value */
/* ***************************************************************************** */
static UInt32 cslLcdcGetBitFieldVal(UInt32 mask, UInt32 value)
{
	UInt32 sc = 0;

	if (mask == 0)
		return (0);

	while ((mask & 0x00000001) == 0) {
		mask = mask >> 1;
		sc++;
	}

	value = value << sc;
	return (value);
}

/* ***************************************************************************** */
/* Function Name: cslLcdcEofDma */
/* Description: */
/* ***************************************************************************** */
static void cslLcdcEofDma(DMADRV_CALLBACK_STATUS_t status)
{
	if (status != DMADRV_CALLBACK_OK) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] cslLcdcEofDma(): ERR DMA!\n\r");
	}

	OSSEMAPHORE_Release(eofDmaSema);
}

/* ***************************************************************************** */
/* Function Name: cslLcdcUpdateTask */
/* Description: */
/* ***************************************************************************** */
static void cslLcdcUpdateTask(void)
{
	LCDC_UPD_REQ_MSG_T updMsg;
	OSStatus_t osStat = OSSTATUS_FAILURE; /* prevent CID58586*/
	CSL_LCD_RES_T res;

	for (;;) {
		OSQUEUE_Pend(updReqQ, (QMsg_t *) & updMsg, TICKS_FOREVER);
            if (lcd_enable)
		osStat = OSSEMAPHORE_Obtain(eofDmaSema,
					    TICKS_IN_MILLISECONDS(updMsg.updReq.
								  timeOut_ms));

		if (osStat != OSSTATUS_SUCCESS) {
			if (osStat == OSSTATUS_TIMEOUT) {
				LCD_DBG(LCD_DBG_ID,
					"[CSL LCDC] cslLcdcUpdateTask: "
					"ERR EOFDMA Timeout!\n\r");
				res = CSL_LCD_OS_TOUT;
			} else {
				LCD_DBG(LCD_DBG_ID,
					"[CSL LCDC] cslLcdcUpdateTask: "
					"ERR EOFDMA OS Error!\n\r");
				res = CSL_LCD_OS_ERR;
			}

			cslLcdcDmaStop(&updMsg);
			OSSEMAPHORE_Reset(eofDmaSema, 0);
		} else {
			res = CSL_LCD_OK;
		}

#ifdef _ATHENA_
		if (updMsg.lcdH->usesTE) {
			chal_lcdc_set_te_enable(cslLcdcDrv.chalH, FALSE);
			chal_lcdc_set_te_new_frame(cslLcdcDrv.chalH, FALSE);
		}
#endif /* #ifdef _ATHENA_ */

		cslLcdcWaitWhileLcdcIsBusy();

		if (updMsg.lcdH->usesCE)
			chal_lcdc_par_set_ce(cslLcdcDrv.chalH, FALSE);

#if (CHIP_REVISION >= 20)
		if (updMsg.lcdH->uses888U)
			chal_lcdc_par_set_rgb888u(cslLcdcDrv.chalH, FALSE);
#endif

		chal_lcdc_set_dma(cslLcdcDrv.chalH, FALSE);

#ifdef LCDC_PEDESTAL_CTRL
		PEDESTAL_EnablePedestalMode(lcdc_pedestal_id);
#endif
		CSL_LCDC_DIS_AHB_CLK();
		if ((!updMsg.lcdH->hasLock)&&lcd_enable)
			OSSEMAPHORE_Release(lcdcSema);

		if (updMsg.updReq.cslLcdCb) {
			updMsg.updReq.cslLcdCb(res, updMsg.lcdH,
					       updMsg.updReq.cslLcdCbRef);
		}
	}
}				/* cslLcdcUpdateTask */

/* ***************************************************************************** */
/* Function Name: cslLcdcOsInit */
/* Description: */
/* ***************************************************************************** */
Boolean static cslLcdcOsInit(void)
{
	Task_t task;
	Boolean res = TRUE;

	updReqQ = OSQUEUE_Create(FLUSH_Q_SIZE,
				 sizeof(LCDC_UPD_REQ_MSG_T),
				 OSSUSPEND_PRIORITY);

	if (!updReqQ) {
		LCD_DBG(LCD_DBG_ID,
			"[CSL LCDC] cslLcdcOsInit: ERR Queue creation\n");
		res = FALSE;
	} else {
		OSQUEUE_ChangeName(updReqQ, "LcdQ");
	}

	task = OSTASK_Create(cslLcdcUpdateTask, TASKNAME_LCDC,
			     TASKPRI_LCDC, STACKSIZE_LCDC);

	if (!task) {
		LCD_DBG(LCD_DBG_ID,
			"[CSL LCDC] cslLcdcOsInit: ERR Task creation!\n");
		res = FALSE;
	}

	eofDmaSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	if (!eofDmaSema) {
		LCD_DBG(LCD_DBG_ID,
			"[CSL LCDC] cslLcdcOsInit: ERR Sema Creation!\n");
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(eofDmaSema, "LcdDma");
	}

	lcdcSema = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
	if (!lcdcSema) {
		LCD_DBG(LCD_DBG_ID,
			"[CSL LCDC] cslLcdcOsInit: ERR Sema Creation!\n");
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(lcdcSema, "LcdM");
	}

	return (res);
}				/* cslLcdcOsInit */

/* ***************************************************************************** */
/* Function Name:  LCDDRV_SetIoWidth */
/* Description:    Set up Muxes For LCD Pads */
/*                 Returns False If Req LCD bus width is greater than interface */
/* ***************************************************************************** */
Boolean static cslLcdcSetIoWidth(UInt32 bus_width)
{
	UInt32 val, mask;

	mask = 0;		/* set bit to 1 to clear register bit */
	val = 0;		/* set bit value to set */

#if defined (_BCM21551_)
	/* ------------------------------------------------------------------------- */
	/*                             Z E U S */
	/* ------------------------------------------------------------------------- */
	/* ZEUS IOCR5.29 IOCR0.29 LCD.D[0] & LCD Ctrl  set to 00 to enable */
	/* ZEUS          IOCR0.28 LCD.D[7:1]           set to  0 to enable */
	/* ZEUS Looks Only 8-bit LCD Bus */
	/* ------------------------------------------------------------------------- */
	if (bus_width > 8) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] cslLcdcSetIoWidth: "
			"ERR ZEUS Supports Only 8-bit LCD bus\n");
		return (FALSE);
	}
	mask |= SYSCFG_IOCR0_LCD;	/* IOCR0.29 */

	if (bus_width < 8) {
		mask |= SYSCFG_IOCR0_LCDD1_LCDD7;	/* IOCR0.28 */
	}

	*(volatile UInt32 *)SYSCFG_IOCR0 &= ~mask;;
	*(volatile UInt32 *)SYSCFG_IOCR0 |= val;

	*(volatile UInt32 *)SYSCFG_IOCR5 &= ~SYSCFG_IOCR5_LCD;	/* bit 29, set to 0 */
	return (TRUE);
#endif /* #if defined (_BCM21551_) */

#if defined (_ATHENA_)
	/* all bits are 0-active. so value will be kept at 0 for all variations */

	mask |= SYSCFG_IOCR0_MPHI_mux;	/* force disable of MPHI over LCD pads */
	mask |= SYSCFG_IOCR0_LCD_CTRL_MUX;	/* select LCD Ctrl & LCD D[0] */

	/* select LCD D[15..01] */
	if (bus_width > 1) {
		mask |= SYSCFG_IOCR0_LCDD1_LCDD15_MUX;
	}

	if (bus_width == 18) {
		mask |= SYSCFG_IOCR0_LCDD16_LCDD17_MUX;
	}

	*(volatile UInt32 *)(io_p2v(SYSCFG_BASE_ADDR + SYSCFG_IOCR0_OFFSET)) &=
	    ~mask;;
	*(volatile UInt32 *)(io_p2v(SYSCFG_BASE_ADDR + SYSCFG_IOCR0_OFFSET)) |=
	    val;

	return (TRUE);
#endif /* #if defined (_ATHENA_) */
}				/* LCDDRV_SetIoWidth */

/* **************************************************************************** */
/*  Function Name:  cslLcdcSetIoEnable */
/*  Description:    Enables|Disables LCD Interface Pins */
/* **************************************************************************** */
void static cslLcdcSetIoEnable(Boolean enable)
{
#ifndef LCDC_USE_SYSCFG_OPEN_OS_RDB
	if (enable)
		*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR3)) &=
		    ~SYSCFG_IOCR3_LCD_DIS;
	else
		*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR3)) |=
		    SYSCFG_IOCR3_LCD_DIS;
#else
	if (enable)
		BRCM_WRITE_REG_FIELD(io_p2v(SYSCFG_BASE_ADDR), SYSCFG_IOCR3,
				     LCD_DIS, 0);
	else
		BRCM_WRITE_REG_FIELD(io_p2v(SYSCFG_BASE_ADDR), SYSCFG_IOCR3,
				     LCD_DIS, 1);
#endif
}

/* **************************************************************************** */
/*  Function Name:  chal_lcdc_set_io_drive */
/*  Description:    Configures LCD Interface Pins */
/* **************************************************************************** */
void static cslLcdcSetIoDrive(CSL_LCDC_IO_DRIVE_t io)
{
	/* ATHENA has slewFast ? */
	if (io.slewFast)
		*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR4)) &=
		    ~SYSCFG_IOCR4_LCD_slew;
	else
		*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR4)) |=
		    SYSCFG_IOCR4_LCD_slew;

	*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR4)) &= ~SYSCFG_IOCR4_LCD_drive;
	*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR4)) |=
	    cslLcdcGetBitFieldVal(SYSCFG_IOCR4_LCD_drive, io.driveStrength);
}

/* **************************************************************************** */
/*  Function Name:  cslLcdcParSetIntfType */
/*  Description:    Selects Z80 | M68 Interface Type */
/* **************************************************************************** */
void static cslLcdcParSetIntfType(CHAL_PAR_BUS_TYPE_t busType)
{
#ifdef LCDC_USE_SYSCFG_OPEN_OS_RDB
	if (busType == CHAL_BUS_M68)
		BRCM_WRITE_REG_FIELD(io_p2v(SYSCFG_BASE_ADDR), SYSCFG_IOCR0,
				     M68, 1);
	else
		BRCM_WRITE_REG_FIELD(io_p2v(SYSCFG_BASE_ADDR), SYSCFG_IOCR0,
				     M68, 0);
#else
#if defined (_BCM21551_)
	if (busType == CHAL_BUS_M68)
		*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR0)) |= SYSCFG_IOCR0_M68;
	else
		*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR0)) &=
		    ~SYSCFG_IOCR0_M68;
#endif /* #if defined (_BCM21551_) */

#if defined (_ATHENA_)
	if (busType == CHAL_BUS_M68)
		*(volatile cUInt32 *)(io_p2v(SYSCFG_BASE_ADDR) +
				      SYSCFG_IOCR0_OFFSET) |= SYSCFG_IOCR0_M68;
	else
		*(volatile cUInt32 *)(io_p2v(SYSCFG_BASE_ADDR) +
				      SYSCFG_IOCR0_OFFSET) &= ~SYSCFG_IOCR0_M68;
#endif /* #if defined (_ATHENA_) */
#endif /* #ifdef LCDC_USE_SYSCFG_OPEN_OS_RDB */
}

/* ***************************************************************************** */
/* Function Name:   cslLcdcWaitWhileFifoIsFull */
/* Description:     (Z80/M68/DBI) */
/* ***************************************************************************** */
INLINE static void cslLcdcWaitWhileFifoIsFull(void)
{
	/* wait while TX FIFO is FULL  [BEFORE SENDING COMMAND] */
	while (chal_lcdc_rd_status(cslLcdcDrv.chalH) &
	       LCDC_STATUS_FFFULL_MASK) ;
}

#ifdef _ATHENA_
/* ***************************************************************************** */
/* Function Name:   cslLcdcWaitWhileLcdcIsBusy */
/* Description:     (Z80/M68/DBI) */
/* ***************************************************************************** */
INLINE static void cslLcdcWaitWhileLcdcIsBusy(void)
{
	/* wait while CTRLR IS BUSY */
	/* ZEUS DON'T HAVE BUSY FLAG */
	while (chal_lcdc_rd_status(cslLcdcDrv.chalH) &
	       LCDC_STATUS_LCD_BUSY_MASK) ;
}
#endif

/* ***************************************************************************** */
/* Function Name:   cslLcdcWaitUntilFifoIsEmpty */
/* Description:     (Z80/M68/DBI) */
/*                  !!! RD FIFO EMPTY STATUS */
/* ***************************************************************************** */
INLINE static void cslLcdcWaitUntilFifoIsEmpty(void)
{
	/* wait until RX FIFO is EMPTY [BEFORE ISSUING RD REQEST] */
	while (!
	       (chal_lcdc_rd_status(cslLcdcDrv.chalH) &
		LCDC_STATUS_FFEMPTY_MASK)) ;
}

/* ***************************************************************************** */
/* Function Name:   cslLcdcWaitUntilFifoHasData */
/* Description:     (Z80/M68/DBI) */
/* ***************************************************************************** */
INLINE static void cslLcdcWaitUntilFifoHasData(void)
{
	/* wait until RX FIFO has data [BEFORE READING DATA FROM FIFO] */
	while (!
	       (chal_lcdc_rd_status(cslLcdcDrv.chalH) &
		LCDC_STATUS_RREADY_MASK)) ;
}				/* cslLcdcWaitUntilFifoHasData */

/* ***************************************************************************** */
/* Function Name:   cslLcdcRdFifoData */
/* Description:     Read From FIFO (Z80/M68/DBI) */
/* ***************************************************************************** */
INLINE static UInt32 cslLcdcRdFifoData(void)
{
	return (chal_lcdc_rd_data(cslLcdcDrv.chalH));
}

#ifdef _ATHENA_
/* **************************************************************************** */
/*  Function Name:  cslLcdcEnableTeGpio */
/*  Description:    Select TE Input,  GPIO25 | LCDD16 */
/* **************************************************************************** */
static void cslLcdcEnableTeGpio(UInt32 pinSel)
{
	if (pinSel == 0) {
		/* select TE on GPIO25  IOCR2.3=1  IOCR6.19=0 */
		*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR2)) |= 0x00000008;
		*(volatile cUInt32 *)(io_p2v(SYSCFG_IOCR6)) &= ~0x00080000;
	} else {
		/* select TE on LCDD16, IOCR0.26 IOCR0.25 IOCR0.19  b'001 */
		*(volatile cUInt32 *)(io_p2v(SYSCFG_BASE_ADDR) +
				      SYSCFG_IOCR0_OFFSET) &= ~0x06080000;
		*(volatile cUInt32 *)(io_p2v(SYSCFG_BASE_ADDR) +
				      SYSCFG_IOCR0_OFFSET) |= 0x00080000;
	}
}

/* ***************************************************************************** */
/* Function Name:  cslLcdcTeCfg */
/* Description:    Config TE details */
/* ***************************************************************************** */
static void cslLcdcTeCfg(CSL_LCDC_TE_T te)
{
	CHAL_LCDC_TE_CFG_t teCfg;

	if (te.type == LCDC_TE_CTRLR) {
		teCfg.delay_ahb_clks = te.delay;
		if (te.edgeRising)
			teCfg.edge = TE_EDGE_POS;
		else
			teCfg.edge = TE_EDGE_NEG;

		chal_lcdc_set_te_cfg(cslLcdcDrv.chalH, &teCfg);
	}
}				/* cslLcdcSetTe */
#endif /* #ifdef _ATHENA_ */

/* ############################################################################# */
/*                     PARALLEL LCD CONTROLLER SUPPORT */
/*           HANDLES Z80/M68 8-bit 16-bit 18-bit parallel data bus */
/* ############################################################################# */

/* ***************************************************************************** */
/* Function Name:   cslLcdcParRdReqCycleD */
/* Description:     issue D-cyle Read on LCD bus */
/* ***************************************************************************** */
INLINE static void cslLcdcParRdReqCycleD(void)
{
	CHAL_LCDC_PAR_RD_REQ_t rdAcc;

	rdAcc.type = PAR_RD_D;
	cslLcdcWaitUntilFifoIsEmpty();
	chal_lcdc_par_rdreq(cslLcdcDrv.chalH, &rdAcc);

}				/* cslLcdcParRdReqCycleD */

/* ***************************************************************************** */
/* Function Name:   cslLcdcParRdReqCycleCx */
/* Description: */
/* ***************************************************************************** */
INLINE static void cslLcdcParRdReqCycleCx(void)
{
	CHAL_LCDC_PAR_RD_REQ_t rdAcc;

	rdAcc.type = PAR_RD_Cx;
	cslLcdcWaitUntilFifoIsEmpty();
	chal_lcdc_par_rdreq(cslLcdcDrv.chalH, &rdAcc);
}				/* cslLcdcParRdReqCycleCx */

/* ############################################################################# */
/*                             DMA INTERFACE */
/* ############################################################################# */

/* ***************************************************************************** */
/* Function Name:   cslLcdcDmaStart */
/* Description:     Preapre & Start DMA Transfer ( Uses EX LLI interface ) */
/* ***************************************************************************** */
static CSL_LCD_RES_T cslLcdcDmaStart(LCDC_UPD_REQ_MSG_T * updMsg)
{
	UInt32 width;
	UInt32 height;
	CSL_LCD_RES_T result = CSL_LCD_OK;

	/* Get Channel */
	if (DMADRV_Obtain_Channel(DMA_CLIENT_MEMORY,
				  DMA_CLIENT_LCD,
				  &updMsg->dmaCh) != DMADRV_STATUS_OK) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] cslLcdcDmaStart:"
			" ERR DMADRV_Obtain_Channel\r\n ");
		return CSL_LCD_DMA_ERR;
	}
	/* Configure Channel */
	updMsg->dmaChInfo.srcID = DMA_CLIENT_MEMORY;
	updMsg->dmaChInfo.dstID = DMA_CLIENT_LCD;
	updMsg->dmaChInfo.type = DMA_FCTRL_MEM_TO_PERI;
	updMsg->dmaChInfo.srcBstSize = DMA_BURST_SIZE_32;
	updMsg->dmaChInfo.dstBstSize = DMA_BURST_SIZE_16;
	updMsg->dmaChInfo.srcDataWidth = DMA_DATA_SIZE_32BIT;
	updMsg->dmaChInfo.dstDataWidth = DMA_DATA_SIZE_32BIT;
	updMsg->dmaChInfo.incMode = DMA_INC_MODE_SRC;
	updMsg->dmaChInfo.xferCompleteCb = (DmaDrv_Callback) cslLcdcEofDma;
	updMsg->dmaChInfo.freeChan = TRUE;
	updMsg->dmaChInfo.prot = 0;	/* ? */
	updMsg->dmaChInfo.bCircular = FALSE;
	updMsg->dmaChInfo.alignment = DMA_ALIGNMENT_32;	/* ? */
/*  updMsg->dmaChInfo.priority       = 0;                 */

	if (DMADRV_Config_Channel(updMsg->dmaCh, &updMsg->dmaChInfo)
	    != DMADRV_STATUS_OK) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] cslLcdcDmaStart:"
			" ERR DMADRV_Config_Channel\r\n ");
		return CSL_LCD_DMA_ERR;
	}

	width = updMsg->updReq.lineLenP * updMsg->lcdH->buffBpp;
	height = updMsg->updReq.lineCount;

	if (updMsg->updReq.multiLLI) {
		DMADRV_LLI_T dmaLLI;
		updMsg->dmaData.numBuffer = height;
		updMsg->dmaData.pBufList =
		    (Dma_Buffer_List *) updMsg->updReq.buff;

		if (DMADRV_Bind_Data_Ex
		    (updMsg->dmaCh, &updMsg->dmaData, &dmaLLI)
		    != DMADRV_STATUS_OK) {
			LCD_DBG(LCD_DBG_ID, "[CSL LCDC] cslLcdcDmaStart:"
				" ERR DMADRV_Bind_Data_Ex\r\n ");
			return CSL_LCD_DMA_ERR;
		}

		if (DMADRV_Start_Transfer_Ex(updMsg->dmaCh, dmaLLI) !=
		    DMADRV_STATUS_OK) {
			LCD_DBG(LCD_DBG_ID,
				"[CSL LCDC] cslLcdcDmaStart:"
				" ERR DMADRV_Start_Transfer_Ex\r\n ");
			return CSL_LCD_DMA_ERR;
		}
	} else {
		updMsg->dmaBuffList.buffers[0].srcAddr =
		    (UInt32) updMsg->updReq.buff;
		updMsg->dmaBuffList.buffers[0].destAddr =
		    chal_lcdc_get_dma_address(cslLcdcDrv.chalH);
		updMsg->dmaBuffList.buffers[0].length = width * height;
		updMsg->dmaBuffList.buffers[0].bRepeat = 0;
		updMsg->dmaBuffList.buffers[0].interrupt = 1;
		updMsg->dmaData.numBuffer = 1;
		updMsg->dmaData.pBufList =
		    (Dma_Buffer_List *) & updMsg->dmaBuffList;

		if (DMADRV_Bind_Data(updMsg->dmaCh, &updMsg->dmaData)
		    != DMADRV_STATUS_OK) {
			LCD_DBG(LCD_DBG_ID, "[CSL LCDC] cslLcdcDmaStart:"
				" ERR DMADRV_Bind_Data\r\n ");
			return CSL_LCD_DMA_ERR;
		}

		if (DMADRV_Start_Transfer(updMsg->dmaCh) != DMADRV_STATUS_OK) {
			LCD_DBG(LCD_DBG_ID, "[CSL LCDC] cslLcdcDmaStart:"
				" ERR DMADRV_Start_Transfer\r\n ");
			return CSL_LCD_DMA_ERR;
		}
	}
	return (result);
}

/* ***************************************************************************** */
/* Function Name:   cslLcdcDmaStop */
/* Description:     Stops DMA */
/* ***************************************************************************** */
static CSL_LCD_RES_T cslLcdcDmaStop(LCDC_UPD_REQ_MSG_T * updMsg)
{
	CSL_LCD_RES_T result = CSL_LCD_OK;

	DMADRV_Force_Shutdown_Channel((DMA_CHANNEL) updMsg->dmaCh);

	/* DMADRV_Force_Shutdown_Channel releases the channel too.*/
	/*DMADRV_Release_Channel((DMA_CHANNEL)updMsg->dmaCh);*/

	updMsg->dmaCh = DMA_CHANNEL_INVALID;
	return result;
}				/* LCD_DmaStop */

/* ***************************************************************************** */
/* Function Name:  cslLcdcDbi2cHal */
/* Description:    Convert Interface from CSL 2 CHAL */
/*                 Verify Configuration. */
/* ***************************************************************************** */
CSL_LCD_RES_T cslLcdcDbi2cHal(CSL_LCDC_DBI_CTRL_T * busCfg, LCDC_HANDLE lcdc)
{

	CHAL_LCDC_DBI_CFG_t *pDbiCfg;
	CHAL_LCDC_DBI_SPEED_t *pDbiSpeed;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	pDbiCfg = &lcdc->config.ctrlDbi.cfg;

	switch (busCfg->cfg.busType) {
	case LCDC_BUS_DBI_B:
		pDbiCfg->busType = CHAL_BUS_DBI_B;
		break;
	case LCDC_BUS_DBI_C_OPT1:
		pDbiCfg->busType = CHAL_BUS_DBI_C_OPT1;
		break;
	case LCDC_BUS_DBI_C_OPT3:
		pDbiCfg->busType = CHAL_BUS_DBI_C_OPT3;
		break;
	default:
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC DBI]: Invalid Bus Type!\n");
		return (CSL_LCD_BUS_TYPE);
	}

	switch (busCfg->cfg.colModeIn) {
	case LCD_IF_CM_I_RGB565P:
		pDbiCfg->colModeIn = DBI_COL_IN_16_RGB565;
		lcdc->buffBpp = 2;
		break;
	case LCD_IF_CM_I_RGB888U:
		pDbiCfg->colModeIn = DBI_COL_IN_32_RGB888_U;
		lcdc->buffBpp = 4;
		break;
	case LCD_IF_CM_I_RGB888P:
		pDbiCfg->colModeIn = DBI_COL_IN_24_RGB888_P;
		lcdc->buffBpp = 3;
		break;
	default:
		LCD_DBG(LCD_DBG_ID,
			"[CSL LCDC DBI]: Invalid Input Color Mode!\n");
		return (CSL_LCD_COL_MODE);
	}

	switch (busCfg->cfg.colModeOut) {
	case LCD_IF_CM_O_RGB888:
		pDbiCfg->colModeOut = DBI_COL_OUT_RGB888;
		break;
	case LCD_IF_CM_O_RGB666:
		pDbiCfg->colModeOut = DBI_COL_OUT_RGB666;
		break;
	case LCD_IF_CM_O_RGB565:
		pDbiCfg->colModeOut = DBI_COL_OUT_RGB565;
		break;
	case LCD_IF_CM_O_RGB444:
		pDbiCfg->colModeOut = DBI_COL_OUT_RGB444;
		break;
	case LCD_IF_CM_O_RGB332:
		pDbiCfg->colModeOut = DBI_COL_OUT_RGB332;
		break;
	default:
		LCD_DBG(LCD_DBG_ID,
			"[CSL LCDC DBI]: Invalid Output Color Mode!\n");
		return (CSL_LCD_COL_MODE);
	}

	pDbiCfg->busWidth = busCfg->cfg.busWidth;
	pDbiCfg->colModeInBE = busCfg->cfg.colModeInBE;

#ifdef _ATHENA_
	pDbiCfg->dbicSclRouteToLcdCd = busCfg->cfg.dbicSclRouteToLcdCd;
#endif

	pDbiSpeed = &lcdc->config.ctrlDbi.speed;

	pDbiSpeed->tAhbClkC = busCfg->speed.tAhbClkC;
#ifdef _ATHENA_
	pDbiSpeed->dcxHi = busCfg->speed.dcxHi;
	pDbiSpeed->dcxLo = busCfg->speed.dcxLo;
	pDbiSpeed->rdHi = busCfg->speed.rdHi;
	pDbiSpeed->rdLo = busCfg->speed.rdLo;
	pDbiSpeed->wrHi = busCfg->speed.wrHi;
	pDbiSpeed->wrLo = busCfg->speed.wrLo;
#endif

	return (res);
}

/* ***************************************************************************** */
/* Function Name:  cslLcdcDbiSelect */
/* Description:    Activate DBI bank controller setting */
/* ***************************************************************************** */
static void cslLcdcDbiSelect(LCDC_HANDLE lcdc)
{
	chal_lcdc_dbi_config(cslLcdcDrv.chalH, &lcdc->config.ctrlDbi.cfg);

	if (lcdc->config.ctrlDbi.cfg.busType == CHAL_BUS_DBI_B)
		chal_lcdc_dbib_set_speed(cslLcdcDrv.chalH,
					 &lcdc->config.ctrlDbi.speed);
	else
		chal_lcdc_dbic_set_speed(cslLcdcDrv.chalH,
					 &lcdc->config.ctrlDbi.speed);

#ifdef _ATHENA_
	cslLcdcTeCfg(lcdc->config.ctrlDbi.teCfg);
#endif

	if (lcdc->csBank == BUS_CH_0)
		chal_lcdc_set_cs(cslLcdcDrv.chalH, TRUE);
	else
		chal_lcdc_set_cs(cslLcdcDrv.chalH, FALSE);
}				/* cslLcdcDbiSelect */

/* ***************************************************************************** */
/* Function Name:  cslLcdcPar2cHal */
/* Description:    Convert Interface from CSL 2 CHAL. */
/*                 Verify Configuration. */
/* ***************************************************************************** */
CSL_LCD_RES_T cslLcdcPar2cHal(CSL_LCDC_PAR_CTRL_T * busCfg, LCDC_HANDLE lcdc)
{

	CHAL_LCDC_PAR_CFG_t *pParCfg;
	CHAL_LCDC_PAR_SPEED_t *pParSpeed;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	pParCfg = &lcdc->config.ctrlLeg.cfg;

	switch (busCfg->cfg.busType) {
	case LCDC_BUS_Z80:
		pParCfg->busType = CHAL_BUS_Z80;
		break;
	case LCDC_BUS_M68:
		pParCfg->busType = CHAL_BUS_M68;
		break;
	default:
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC PAR]: Invalid Bus Type!\n");
		return (CSL_LCD_BUS_TYPE);
	}

	switch (busCfg->cfg.colModeIn) {
#if (CHIP_REVISION >= 20)
	case LCD_IF_CM_I_RGB888U:
		lcdc->buffBpp = 4;
		break;
#endif
	case LCD_IF_CM_I_RGB565P:
		lcdc->buffBpp = 2;
		break;
	default:
		LCD_DBG(LCD_DBG_ID,
			"[CSL LCDC PAR]: Invalid Input Color Mode!\n");
		return (CSL_LCD_COL_MODE);
	}

	switch (busCfg->cfg.colModeOut) {
	case LCD_IF_CM_O_RGB888:
		break;
	case LCD_IF_CM_O_RGB666:
		break;
	case LCD_IF_CM_O_RGB565:
		break;
	case LCD_IF_CM_O_RGB444:
		break;
	case LCD_IF_CM_O_RGB332:
		break;
	default:
		LCD_DBG(LCD_DBG_ID,
			"[CSL LCDC PAR]: Invalid Output Color Mode!\n");
		return (CSL_LCD_COL_MODE);
	}

	pParCfg->bitSwap = busCfg->cfg.bitSwap;
	pParCfg->busWidth = busCfg->cfg.busWidth;
	pParCfg->byteSwap = busCfg->cfg.byteSwap;
	pParCfg->wordSwap = busCfg->cfg.wordSwap;

	pParSpeed = &lcdc->config.ctrlLeg.speed;

	pParSpeed->rdHold = busCfg->speed.rdHold;
	pParSpeed->rdPulse = busCfg->speed.rdPulse;
	pParSpeed->rdSetup = busCfg->speed.rdSetup;
	pParSpeed->wrHold = busCfg->speed.wrHold;
	pParSpeed->wrPulse = busCfg->speed.wrPulse;
	pParSpeed->wrSetup = busCfg->speed.wrSetup;

	return (res);
}

/* ***************************************************************************** */
/* Function Name:  cslLcdcParSelect */
/* Description:    Activate PAR bank controller setting */
/* ***************************************************************************** */
static void cslLcdcParSelect(LCDC_HANDLE lcdc)
{
	cslLcdcParSetIntfType(lcdc->config.ctrlLeg.cfg.busType);
	chal_lcdc_par_config(cslLcdcDrv.chalH, &lcdc->config.ctrlLeg.cfg);
	chal_lcdc_par_set_speed(cslLcdcDrv.chalH, &lcdc->config.ctrlLeg.speed);

#ifdef _ATHENA_
	cslLcdcTeCfg(lcdc->config.ctrlLeg.teCfg);
#endif

	if (lcdc->csBank == BUS_CH_0)
		chal_lcdc_set_cs(cslLcdcDrv.chalH, TRUE);
	else
		chal_lcdc_set_cs(cslLcdcDrv.chalH, FALSE);

}				/* cslLcdcParSelect */

/* ***************************************************************************** */
/* Function Name:   cslLcdcSwitchBank */
/* Description:     Bank Switch ( cs bank, mode, timing ... ) */
/* ***************************************************************************** */
INLINE void cslLcdcSwitchBank(LCDC_HANDLE lcdc)
{
	if (cslLcdcDrv.activeHandle != lcdc) {
		if (lcdc->ctrlClass == CTRL_LEGACY)
			cslLcdcParSelect(lcdc);
		else
			cslLcdcDbiSelect(lcdc);

		cslLcdcDrv.activeHandle = lcdc;
	}
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_Lock */
/* Description: */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_Lock(CSL_LCD_HANDLE lcdcH)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;

	if ((!lcdc->hasLock)&&lcd_enable) {
		if (!in_interrupt())
			OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);
		lcdc->hasLock = TRUE;
	}
	return (CSL_LCD_OK);
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_Release */
/* Description: */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_Unlock(CSL_LCD_HANDLE lcdcH)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;

	if ((!lcdc->hasLock)&&lcd_enable) {
		if (!in_interrupt())
			OSSEMAPHORE_Release(lcdcSema);
		lcdc->hasLock = FALSE;
	}
	return (CSL_LCD_OK);
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_WrCmnd */
/* Description:    Write To LCD Using 'Cx' WR Cycle */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_WrCmnd(CSL_LCD_HANDLE lcdcH, UInt32 command)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	CSL_LCDC_ENA_AHB_CLK();

	cslLcdcSwitchBank(lcdc);

	cslLcdcWaitWhileFifoIsFull();

	chal_lcdc_wr_cmnd(cslLcdcDrv.chalH, command);

	cslLcdcWaitWhileLcdcIsBusy();

	CSL_LCDC_DIS_AHB_CLK();

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Release(lcdcSema);

	return (res);
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_WrData */
/* Description:    Write To LCD Using 'D' WR Cycle */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_WrData(CSL_LCD_HANDLE lcdcH, UInt32 data)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	CSL_LCDC_ENA_AHB_CLK();

	cslLcdcSwitchBank(lcdc);

	cslLcdcWaitWhileFifoIsFull();

	chal_lcdc_wr_data(cslLcdcDrv.chalH, data);

	cslLcdcWaitWhileLcdcIsBusy();

	CSL_LCDC_DIS_AHB_CLK();

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Release(lcdcSema);

	return (CSL_LCD_OK);
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_PAR_RdCmnd */
/* Description:    Return Value Read From LCD Using 'Cx' RD Cycle */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_PAR_RdCmnd(CSL_LCD_HANDLE lcdcH, UInt32 * data)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	CSL_LCDC_ENA_AHB_CLK();

	cslLcdcSwitchBank(lcdc);

	cslLcdcWaitWhileFifoIsFull();

	cslLcdcParRdReqCycleCx();

	cslLcdcWaitUntilFifoHasData();
	*data = cslLcdcRdFifoData();

	CSL_LCDC_DIS_AHB_CLK();

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Release(lcdcSema);

	return (res);
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_PAR_RdData */
/* Description:    Return Value Read From LCD Using 'D' RD Cycle */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_PAR_RdData(CSL_LCD_HANDLE lcdcH, UInt32 * data)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	CSL_LCDC_ENA_AHB_CLK();

	cslLcdcSwitchBank(lcdc);

	cslLcdcWaitWhileFifoIsFull();

	cslLcdcParRdReqCycleD();

	cslLcdcWaitUntilFifoHasData();
	*data = cslLcdcRdFifoData();

	CSL_LCDC_DIS_AHB_CLK();

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Release(lcdcSema);

	return (res);
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_PAR_SetSpeed */
/* Description:    Change Default RD/WR Timing for Z80/M68 LCD Bus */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_PAR_SetSpeed(CSL_LCD_HANDLE lcdcH,
				    pCSL_LCDC_PAR_SPEED pSpeed)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	CHAL_LCDC_PAR_SPEED_t *pParSpeed;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	pParSpeed = &lcdc->config.ctrlLeg.speed;

	pParSpeed->rdHold = pSpeed->rdHold;
	pParSpeed->rdPulse = pSpeed->rdPulse;
	pParSpeed->rdSetup = pSpeed->rdSetup;
	pParSpeed->wrHold = pSpeed->wrHold;
	pParSpeed->wrPulse = pSpeed->wrPulse;
	pParSpeed->wrSetup = pSpeed->wrSetup;

	if (cslLcdcDrv.activeHandle == lcdc) {
		CSL_LCDC_ENA_AHB_CLK();
		chal_lcdc_par_set_speed(cslLcdcDrv.chalH, pParSpeed);
		CSL_LCDC_DIS_AHB_CLK();
	}

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Release(lcdcSema);

	return (res);
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_DBI_SetSpeed */
/* Description:    Change Default RD/WR Timing for DBI Modes */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_DBI_SetSpeed(CSL_LCD_HANDLE lcdcH,
				    pCSL_LCDC_DBI_SPEED pSpeed)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	CHAL_LCDC_DBI_SPEED_t *pDbiSpeed;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	pDbiSpeed = &lcdc->config.ctrlDbi.speed;

	pDbiSpeed->tAhbClkC = pSpeed->tAhbClkC;
#ifdef _ATHENA_
	pDbiSpeed->dcxHi = pSpeed->dcxHi;
	pDbiSpeed->dcxLo = pSpeed->dcxLo;
	pDbiSpeed->rdHi = pSpeed->rdHi;
	pDbiSpeed->rdLo = pSpeed->rdLo;
	pDbiSpeed->wrHi = pSpeed->wrHi;
	pDbiSpeed->wrLo = pSpeed->wrLo;
#endif

	if (cslLcdcDrv.activeHandle == lcdc) {
		CSL_LCDC_ENA_AHB_CLK();
		if (lcdc->config.ctrlDbi.cfg.busType == CHAL_BUS_DBI_B)
			chal_lcdc_dbib_set_speed(cslLcdcDrv.chalH,
						 &lcdc->config.ctrlDbi.speed);
		else
			chal_lcdc_dbic_set_speed(cslLcdcDrv.chalH,
						 &lcdc->config.ctrlDbi.speed);

		CSL_LCDC_DIS_AHB_CLK();
	}

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Release(lcdcSema);

	return (res);
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_DBI_RdAccess */
/* Description: */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_DBI_RdAccess(CSL_LCD_HANDLE lcdcH,
				    pCSL_LCDC_DBI_RD_REQ rdAcc)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	CHAL_LCDC_DBI_RD_REQ_t chalRdAcc;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	CSL_LCDC_ENA_AHB_CLK();

	cslLcdcSwitchBank(lcdc);

	cslLcdcWaitUntilFifoIsEmpty();
	cslLcdcWaitWhileFifoIsFull();

	chalRdAcc.type = (UInt32) rdAcc->type;
	chalRdAcc.command = (UInt32) rdAcc->command;
	chal_lcdc_dbi_rdreq(cslLcdcDrv.chalH, &chalRdAcc);

	cslLcdcWaitUntilFifoHasData();
	rdAcc->data = cslLcdcRdFifoData();

	CSL_LCDC_DIS_AHB_CLK();

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Release(lcdcSema);

	return (res);
}

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_DBI_WrCmndCmOn */
/* Description:    Writes 8-bit command End Enables DBI DATA Pixel Mode. */
/*                 All DATA writes after the commadn will be treated as pixels, */
/*                 and thus converted accoding to DBI IN/OUT color modes set. */
/*                 Next command write will cancel PIXEL DATA mode */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_DBI_WrCmndCmOn(CSL_LCD_HANDLE lcdcH, UInt32 command)
{
	CSL_LCD_RES_T res;

	res = CSL_LCDC_WrCmnd(lcdcH, command | LCDC_CMDR_DBI_PIXEL_MODE_MASK);
	return (res);
}

/* ***************************************************************************** */
/* Function Name: CSL_LCDC_Update */
/* Description:   Update External Frame Buffer Using DMA */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_Update(CSL_LCD_HANDLE lcdcH, pCSL_LCD_UPD_REQ req)
{
	LCDC_UPD_REQ_MSG_T updM;
	OSStatus_t status;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	CSL_LCDC_ENA_AHB_CLK();

#ifdef LCDC_PEDESTAL_CTRL
	PEDESTAL_DisablePedestalMode(lcdc_pedestal_id);
#endif

	cslLcdcSwitchBank(lcdc);

	updM.updReq = *req;
	updM.lcdH = lcdc;

	if (lcdc->usesCE)
		chal_lcdc_par_set_ce(cslLcdcDrv.chalH, TRUE);
#if (CHIP_REVISION >= 20)
	if (lcdc->uses888U)
		chal_lcdc_par_set_rgb888u(cslLcdcDrv.chalH, TRUE);
#endif

#ifdef _ATHENA_
	if (lcdc->usesTE) {
		chal_lcdc_set_te_enable(cslLcdcDrv.chalH, TRUE);
		chal_lcdc_set_te_new_frame(cslLcdcDrv.chalH, TRUE);
	}
#endif

	chal_lcdc_set_dma(cslLcdcDrv.chalH, TRUE);

	/* !!! A MUST - START DMA CH ONLY WHEN LCDC IS SET-UP */
	if ((res = cslLcdcDmaStart(&updM)) != CSL_LCD_OK) {
		if ((!lcdc->hasLock)&&lcd_enable)
			OSSEMAPHORE_Release(lcdcSema);
		CSL_LCDC_DIS_AHB_CLK();
		return (res);
	}

	if (req->cslLcdCb == NULL) {	/* BLOCKING */
		status = OSSEMAPHORE_Obtain(eofDmaSema,
					    TICKS_IN_MILLISECONDS(req->
								  timeOut_ms));

#ifdef _ATHENA_
		if (lcdc->usesTE) {
			chal_lcdc_set_te_enable(cslLcdcDrv.chalH, FALSE);
			chal_lcdc_set_te_new_frame(cslLcdcDrv.chalH, FALSE);
		}
#endif
		if (lcdc->usesCE)
			chal_lcdc_par_set_ce(cslLcdcDrv.chalH, FALSE);

#if (CHIP_REVISION >= 20)
		if (lcdc->uses888U)
			chal_lcdc_par_set_rgb888u(cslLcdcDrv.chalH, FALSE);
#endif

		chal_lcdc_set_dma(cslLcdcDrv.chalH, FALSE);

		if (status != OSSTATUS_SUCCESS) {
			cslLcdcDmaStop(&updM);

			if (status == OSSTATUS_TIMEOUT) {
				LCD_DBG(LCD_DBG_ID,
					"[CSL LCDC] CSL_LCDC_Update: "
					"ERR EOFDMA Timeout!\n\r");
				res = CSL_LCD_OS_TOUT;
			} else {
				LCD_DBG(LCD_DBG_ID,
					"[CSL LCDC] CSL_LCDC_Update: "
					"ERR EOFDMA OS Error!\n\r");
				res = CSL_LCD_OS_ERR;
			}
		}

		cslLcdcWaitWhileLcdcIsBusy();

#ifdef LCDC_PEDESTAL_CTRL
		PEDESTAL_EnablePedestalMode(lcdc_pedestal_id);
#endif
		CSL_LCDC_DIS_AHB_CLK();
		if ((!lcdc->hasLock)&&lcd_enable)
			OSSEMAPHORE_Release(lcdcSema);
	} else {		/* NON_BLOCKING */
		status =
		    OSQUEUE_Post(updReqQ, (QMsg_t *) & updM, TICKS_NO_WAIT);
		if (status != OSSTATUS_SUCCESS) {
			LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_Update: "
				"ERROR Posting Update Message\n");
			res = CSL_LCD_OS_ERR;
		}
	}

	return (res);
}				/* CSL_LCDC_Update */

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_Close */
/* Description:    Close Z80/M68 or DBI CS Bank */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_Close(CSL_LCD_HANDLE * lcdcH)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;
	CSL_LCD_RES_T res;

	if (lcdc->open) {
		lcdc->open = FALSE;
		cslLcdcDrv.instance--;
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_Close: OK,"
			"Closing CSBANK[%d], Clients Left [%d]\n\r",
			lcdc->csBank, cslLcdcDrv.instance);
		res = CSL_LCD_OK;
	} else {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_Close: ERR,"
			"LCDC Handle Not In Open State\n\r");
		res = CSL_LCD_NOT_OPEN;
	}

	return (res);
}				/* CSL_LCDC_Close */

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_DBI_Open */
/* Description:    Open DBI CS Bank */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_DBI_Open(pCSL_LCDC_DBI_CTRL busCfg,
				CSL_LCD_HANDLE * lcdcH)
{
	LCDC_HANDLE lcdc;
	CSL_LCD_RES_T res;

	if (!cslLcdcDrv.init) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_DBI_Open: "
			"ERR Not Intialized!\n\r");
		*lcdcH = NULL;
		return (CSL_LCD_NOT_INIT);
	}

	if (cslLcdcDrv.instance >= LCDC_MAX_HANDLES) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_DBI_Open: "
			"ERR Instance Count!\n\r");
		*lcdcH = NULL;
		return (CSL_LCD_INST_COUNT);
	}

	if (cslLcdcDrv.lcdc[0].open)
		lcdc = &cslLcdcDrv.lcdc[1];
	else
		lcdc = &cslLcdcDrv.lcdc[0];

	if ((res = cslLcdcDbi2cHal(busCfg, lcdc)) != CSL_LCD_OK) {
		*lcdcH = NULL;
		return (res);
	}

	lcdc->config.ctrlDbi.teCfg = busCfg->teCfg;
	lcdc->usesTE = FALSE;

#ifdef _ATHENA_
	if (busCfg->teCfg.type == LCDC_TE_CTRLR) {
		lcdc->usesTE = TRUE;
		cslLcdcEnableTeGpio(busCfg->teCfg.pinSel);
	} else {
		lcdc->usesTE = FALSE;
		if (busCfg->teCfg.type != LCDC_TE_NONE) {
			LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_DBI_Open: "
				"WARNING Disabling Tearing Support, "
				"Only TE_CTRLR Supported!\n\r");
		}
	}
#else
	if (busCfg->teCfg.type != LCDC_TE_NONE) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_DBI_Open: "
			"WARNING Disabling Tearing Support, "
			"Not Implemented Yet!\n\r");
	}
#endif

	cslLcdcDrv.instance++;
	cslLcdcDrv.activeHandle = lcdc;

	lcdc->open = TRUE;
	lcdc->csBank = busCfg->cfg.csBank;
	lcdc->ctrlClass = CTRL_DBI;
	lcdc->usesCE = FALSE;

	CSL_LCDC_ENA_AHB_CLK();

	cslLcdcDbiSelect(lcdc);

	/* should be part of shared bus init */
	cslLcdcSetIoDrive(busCfg->io);
	cslLcdcSetIoEnable(TRUE);

	*lcdcH = (CSL_LCD_HANDLE) lcdc;

	CSL_LCDC_DIS_AHB_CLK();

	LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_Open: OK,"
		"MODE[DBI] CSBANK[%d], TE[%s]\n\r",
		lcdc->csBank, lcdc->usesTE ? "YES" : "NO");

	return (CSL_LCD_OK);
}				/* CSL_LCDC_DBI_Open */

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_PAR_Open */
/* Description:    Open Z80/M68 CS Bank */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_PAR_Open(pCSL_LCDC_PAR_CTRL busCfg,
				CSL_LCD_HANDLE * lcdcH)
{

	LCDC_HANDLE lcdc;
	CSL_LCD_RES_T res;

	if (!cslLcdcDrv.init) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_PAR_Open: "
			"ERR LCDC Not Intialized!\n\r");
		*lcdcH = NULL;
		return (CSL_LCD_NOT_INIT);
	}

	if (cslLcdcDrv.instance == LCDC_MAX_HANDLES) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_PAR_Open: "
			"ERR Instance Count!\n\r");
		*lcdcH = NULL;
		return (CSL_LCD_INST_COUNT);
	}

	if (cslLcdcDrv.lcdc[0].open)
		lcdc = &cslLcdcDrv.lcdc[1];
	else
		lcdc = &cslLcdcDrv.lcdc[0];

	if ((res = cslLcdcPar2cHal(busCfg, lcdc)) != CSL_LCD_OK) {
		return (res);
	}

	cslLcdcDrv.instance++;

	lcdc->config.ctrlLeg.teCfg = busCfg->teCfg;
	lcdc->usesTE = FALSE;

#if (CHIP_REVISION >= 20)
	if (busCfg->cfg.colModeIn == LCD_IF_CM_I_RGB888U)
		lcdc->uses888U = TRUE;
	else
		lcdc->uses888U = FALSE;
#endif

#ifdef _ATHENA_
	if (busCfg->teCfg.type == LCDC_TE_CTRLR) {
		lcdc->usesTE = TRUE;
		cslLcdcEnableTeGpio(busCfg->teCfg.pinSel);
	} else {
		lcdc->usesTE = FALSE;
		if (busCfg->teCfg.type != LCDC_TE_NONE) {
			LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_PAR_Open: "
				"WARNING Disabling Tearing Support, "
				"Only CTRLR_SYNC Supported!\n\r");
		}
	}
#else
	if (busCfg->teCfg.type != LCDC_TE_NONE) {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_PAR_Open: "
			"WARNING Disabling Tearing Support, "
			"Not Implemented Yet!\n\r");
	}
#endif

	cslLcdcDrv.activeHandle = lcdc;

	lcdc->open = TRUE;
	lcdc->csBank = busCfg->cfg.csBank;
	lcdc->ctrlClass = CTRL_LEGACY;

	if (busCfg->cfg.busWidth == 18)
		lcdc->usesCE = TRUE;
	else
		lcdc->usesCE = FALSE;

	CSL_LCDC_ENA_AHB_CLK();

	cslLcdcParSelect(lcdc);

	/* should be part of shared bus init */
	cslLcdcSetIoDrive(busCfg->io);

	cslLcdcSetIoEnable(TRUE);

	*lcdcH = (CSL_LCD_HANDLE) lcdc;

	CSL_LCDC_DIS_AHB_CLK();

	LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_Open: OK,"
		"MODE[Z80/M68] CSBANK[%d], TE[%s]\n\r",
		lcdc->csBank, lcdc->usesTE ? "YES" : "NO");

	return (CSL_LCD_OK);
}				/* CSL_LCDC_PAR_Open */

/* ***************************************************************************** */
/* Function Name:  CSL_LCDC_Init */
/* Description:    Init CSL LCDC Driver */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_Init(pCSL_LCDC_INIT ctrlInit)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;

	LCD_DBG(LOGID_MISC, "[CSL LCDC] +%s\n\r", __FUNCTION__);

	if (!cslLcdcDrv.init) {
		memset((void *)&cslLcdcDrv, 0, sizeof(LCDC_DRV_st));

#ifdef LCDC_PEDESTAL_CTRL
		if (lcdc_pedestal_id == 0xff)
			lcdc_pedestal_id = PEDESTAL_AllocId();
#endif

		if (!cslLcdcSetIoWidth(ctrlInit->maxBusWidth)) {
			LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_Init: "
				"ERR IO Width[%d]\n\r", ctrlInit->maxBusWidth);
			return (CSL_LCD_BUS_CFG);
		}
		if (cslLcdcOsInit()) {
			if (ctrlInit->lcdc_base_address)
				cslLcdcDrv.chalH =
				    chal_lcdc_init(ctrlInit->lcdc_base_address);
			else
				cslLcdcDrv.chalH =
				    chal_lcdc_init(LCDC_BASE_ADDR);
			cslLcdcSetIoWidth(ctrlInit->maxBusWidth);
			cslLcdcDrv.init = TRUE;
			LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_Init: OK\n\r");
		} else {
			LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_Init: "
				"ERR OS Init\n\r");
			res = CSL_LCD_ERR;
		}
	} else {
		LCD_DBG(LCD_DBG_ID, "[CSL LCDC] CSL_LCDC_Init: "
			"WARNING Already Init\n\r");
	}
	return (res);
}

/* ***************************************************************************** */
/* Function Name: CSL_LCDC_Enable_CE */
/* Description:   Enable / Disable Colour expansion in legacy modes */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_Enable_CE(CSL_LCD_HANDLE lcdcH, bool enable)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	CSL_LCDC_ENA_AHB_CLK();

	chal_lcdc_par_set_ce(cslLcdcDrv.chalH, enable);

	CSL_LCDC_DIS_AHB_CLK();

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Release(lcdcSema);

	return (res);
}

#if (CHIP_REVISION >= 20)
/* ***************************************************************************** */
/* Function Name: CSL_LCDC_Enable_RGB888U */
/* Description:   Enable / Disable Unpacked ARGB8888 input in legacy modes */
/* ***************************************************************************** */
CSL_LCD_RES_T CSL_LCDC_Enable_RGB888U(CSL_LCD_HANDLE lcdcH, bool enable)
{
	LCDC_HANDLE lcdc = (LCDC_HANDLE) lcdcH;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Obtain(lcdcSema, TICKS_FOREVER);

	CSL_LCDC_ENA_AHB_CLK();

	chal_lcdc_par_set_rgb888u(cslLcdcDrv.chalH, enable);

	CSL_LCDC_DIS_AHB_CLK();

	if ((!lcdc->hasLock)&&lcd_enable)
		OSSEMAPHORE_Release(lcdcSema);

	return (res);
}
#endif


#else
#error "[CSL LCDC] ERROR: Available For ZEUS/ATHENA Only!"
#endif /* #if ( defined (_BCM21551_) || defined (_ATHENA_) ) */
