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

/**
*
*  @file   csl_dma.c
*
*  @brief  DMA CSL implementation
*
*  @note
****************************************************************************/

/*
 * This file is taken from //depot/Sources/SystemIntegration/msp/soc/csl/bsp/dma/src/
 * on 7th October 2010
 */
#include <plat/types.h>
#include <plat/osdal_os_driver.h>

#include <plat/chal/chal_types.h>
#include <plat/chal/chal_dma.h>
#include "plat/osabstract/ossemaphore.h"
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
#include "chal_dmux.h"
#endif

#include <plat/dma_drv.h>
#include <plat/csl/csl_dma.h>
#include <plat/syscfg.h>
#include <linux/spinlock.h>

/* ****************************************************************************** */
/* Local Definitions */
/* ****************************************************************************** */

#if defined (_ATHENA_)
#define AUDIO_RESERVED_CHAN     4
#define MEM_TO_MEM_RESERVED     1
#else
#define AUDIO_RESERVED_CHAN     0
#define MEM_TO_MEM_RESERVED     0
#endif

static UInt32 phyChanList;
static Dma_Chan_List *dmaChanList = NULL;
static UInt32 dmaChanListSize;

static Dma_Channel chanArray[TOTAL_DMA_CHANNELS];

typedef struct {
	int initialized;
	UInt32 base;
	CHAL_HANDLE handle;
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	CHAL_DMA_SECURE_STATE_t secState;
	UInt32 dmuxBase;
	CHAL_HANDLE dmuxHandle;
#endif
	ChalDmaDevCapabilities_t pCap;
} dmac_t;
static dmac_t dmac;

const UInt32 dmaChanPriority[DMA_CLIENT_TOTAL] = {
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_BULK_CRYPT_OUT */
	DMA_CHAN_PRI_HIGH,	/* DMA_CLIENT_CAM */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_I2S_TX */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_I2S_RX */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_SIM_RX TX */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_CRC */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_SPI_RX */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_SPI_TX */
	DMA_CHAN_PRI_LOW,	/* DMA_UARTA_RX */
	DMA_CHAN_PRI_LOW,	/* DMA_UARTA_TX */
	DMA_CHAN_PRI_LOW,	/* DMA_UARTB_RX */
	DMA_CHAN_PRI_LOW,	/* DMA_UARTB_TX */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_RESERVED1 */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_RESERVED2 */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_USB_RX */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_USB_TX */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_HSDPA_SCATTER */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_HSDPA_GATHER */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_BULK_CRYPT_IN */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_LCD */
	DMA_CHAN_PRI_LOW,	/* DMA_CLIENT_MSPRO */
	DMA_CHAN_PRI_HIGH,	/* DMA_CLIENT_DSI_CM */
	DMA_CHAN_PRI_HIGH,	/* DMA_CLIENT_DSI_VM */
	DMA_CHAN_PRI_HIGH,	/* DMA_CLIENT_TVENC1 */
	DMA_CHAN_PRI_HIGH,	/* DMA_CLIENT_TVENC2 */
#if defined (_ATHENA_)
	DMA_CHAN_PRI_HIGH,	/* DMA_CLIENT_AUDIO_IN_FIFO */
	DMA_CHAN_PRI_HIGH,	/* DMA_CLIENT_AUDIO_OUT_FIFO */
	DMA_CHAN_PRI_HIGH,	/* DMA_CLIENT_POLYRING_OUT_FIFO */
	DMA_CHAN_PRI_HIGH,	/* DMA_CLIENT_AUDIO_WB_MIXERTAP */
#endif
	DMA_CHAN_PRI_LOW	/* DMA_CLIENT_MEMORY */
};

typedef struct {
	Boolean bUsed;
	DMA_CHANNEL baseChan;
	UInt32 assocChan;
} ASSOC_CHAN_t;

static ASSOC_CHAN_t assocChanArray[ASSOCIATE_CHANNEL];

/* ****************************************************************************** */
/* Function Definition */
/* ****************************************************************************** */

#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
extern void CACHE_CleanByAddr(UInt32 start_addr, UInt32 end_addr);
extern void CACHE_InvalidateByAddr(UInt32 startAddr, UInt32 EndAddr);

/* ****************************************************************************** */
/*  Funciton Name:      dma_ucode_alloc */
/*  Description: */
/* ****************************************************************************** */
static CHAL_DMA_STATUS_t dma_ucode_alloc(chal_dmac_ucode_t * ucode)
{
	UInt32 *pBuf;

	pBuf = (UInt32 *) OSDAL_ALLOCHEAPMEM(ucode->length);
	if (pBuf == NULL) {
		return CHAL_DMA_STATUS_FAILURE;
	}

	ucode->vir_addr = (UInt32) pBuf;
	ucode->phy_addr = ucode->vir_addr;
	return CHAL_DMA_STATUS_SUCCESS;
}

/* ****************************************************************************** */
/*  Funciton Name:      dma_ucode_coherency */
/*  Description: */
/* ****************************************************************************** */
static CHAL_DMA_STATUS_t dma_ucode_coherency(chal_dmac_ucode_t * ucode)
{
	CACHE_CleanByAddr((UInt32) (ucode->vir_addr),
			  (UInt32) (ucode->vir_addr) + (ucode->length) - 1);

	return CHAL_DMA_STATUS_SUCCESS;
}

/* ****************************************************************************** */
/*  Funciton Name:      dma_ucode_free */
/*  Description: */
/* ****************************************************************************** */
static CHAL_DMA_STATUS_t dma_ucode_free(chal_dmac_ucode_t * ucode)
{
	OSDAL_FREEHEAPMEM((UInt32 *) ucode->vir_addr);

	return CHAL_DMA_STATUS_SUCCESS;
}
#endif

/* ****************************************************************************** */
/*  Funciton Name:      dma_set_channel_tc_intr */
/*  Description:        Enable or disable terminal count interrupt for the channel */
/* ****************************************************************************** */
static Int32 dma_set_channel_tc_intr(UInt32 chanID, UInt32 status)
{
	if (chanID >= TOTAL_DMA_CHANNELS)
		return -1;

	if (status)
		chanArray[chanID].chanInfo.dmaCfgReg |= (DMA_CHCFG_ENABLE_ITC);
	else
		chanArray[chanID].chanInfo.dmaCfgReg &= ~(DMA_CHCFG_ENABLE_ITC);

	return 0;
}

/* ****************************************************************************** */
/*  Funciton Name:      dma_set_channel_err_intr */
/*  Description:        Enable or disable error interrupt for the channel */
/* ****************************************************************************** */
static Int32 dma_set_channel_err_intr(UInt32 chanID, UInt32 status)
{
	if (chanID >= TOTAL_DMA_CHANNELS)
		return -1;

	if (status)
		chanArray[chanID].chanInfo.dmaCfgReg |= (DMA_CHCFG_ENABLE_IE);
	else
		chanArray[chanID].chanInfo.dmaCfgReg &= ~(DMA_CHCFG_ENABLE_IE);

	return 0;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_init */
/*  Description:        Initialize DMA hardware and software interface */
/* ****************************************************************************** */
void csl_dma_init(void *base, UInt32 LL_virt, UInt32 LL_phy, UInt32 LL_Size)
{
	dmac_t *pdma = (dmac_t *) & dmac;
	ChalDmaDevNodeMemory_t dmaDevNodeMemory;

#if defined (_ATHENA_)
	OSDAL_SYSCLK_Enable(OSDAL_SYSCLK_ENABLE_DMAC);
#endif

#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	pdma->base = (UInt32) base;	/* NONDMAC_BASE_ADDR; */
	pdma->secState = CHAL_DMA_STATE_SECURE;
	pdma->handle = (CHAL_HANDLE) chal_dma_init(pdma->base, pdma->secState);
#else
	pdma->base = (UInt32) base;
	pdma->handle = (CHAL_HANDLE) chal_dma_init(pdma->base);
#endif
	if (pdma->handle == NULL) {
		/* dprintf(1, "Invalid dmac handle\n"); */
		return;
	}
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	pdma->dmuxBase = DMUX_BASE_ADDR;
	pdma->dmuxHandle = (CHAL_HANDLE) chal_dmux_init(pdma->dmuxBase);
	if (pdma->dmuxHandle == NULL) {
		/* dprintf(1, "Invalid dmac handle\n"); */
		return;
	}
#endif

	pdma->initialized = 1;

	dmaChanList = (Dma_Chan_List *) LL_virt;
	phyChanList = (UInt32) LL_phy;
	dmaChanListSize = LL_Size;

	memset(chanArray, 0, sizeof(Dma_Channel) * TOTAL_DMA_CHANNELS);
/*    memset(dmaChanList, 0, sizeof(Dma_Chan_List)*TOTAL_DMA_CHANNELS); */
	memset(assocChanArray, 0, sizeof(ASSOC_CHAN_t) * ASSOCIATE_CHANNEL);

	dmaDevNodeMemory.LLIVirtualAddr = (UInt32) dmaChanList;
	dmaDevNodeMemory.LLIPhysicalAddr = phyChanList;
	dmaDevNodeMemory.size = dmaChanListSize;

	chal_dma_get_capabilities(pdma->handle,
				  (ChalDmaDevCapabilities_t *) & pdma->pCap);

	chal_dma_config_device_memory(pdma->handle,
				      (ChalDmaDevNodeMemory_t *) &
				      dmaDevNodeMemory);

#if defined (_ATHENA_)
	OSDAL_SYSCLK_Disable(OSDAL_SYSCLK_ENABLE_DMAC);
#endif

	return;
}

/* ****************************************************************************** */
/*  Funciton Name:      DmaChanDeInit */
/*  Description:        Deinitialize DMA hardware and software interface */
/* ****************************************************************************** */
void csl_dma_deinit(void)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return;
	}
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	chal_dmux_protect(pdma->dmuxHandle);
#endif
	chal_dma_shutdown_all_channels(pdma->handle);
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	chal_dmux_unprotect(pdma->dmuxHandle);
#endif

#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	if (chal_dmux_deinit(pdma->dmuxHandle) != CHAL_DMUX_STATUS_SUCCESS) {
		dprintf(1, "DeInit dmux failed\n");
	}
#endif

	dmaChanList = NULL;
	dmaChanListSize = 0;
	phyChanList = 0;

	pdma->handle = NULL;
	pdma->initialized = 0;
	pdma->base = 0;

#if defined (_ATHENA_)
	OSDAL_SYSCLK_Disable(OSDAL_SYSCLK_ENABLE_DMAC);
#endif

	return;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_reset_channel */
/*  Description:        Reset a single channel */
/* ****************************************************************************** */
UInt32 csl_dma_reset_channel(UInt16 chanID)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return 0;
	}

	if (chanID >= TOTAL_DMA_CHANNELS)
		return 0;

	memset(&chanArray[chanID], 0, sizeof(Dma_Channel));

	return chal_dma_reset_channel(pdma->handle, chanID);
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_shutdown_all_channel */
/*  Description:        Disable all active channel and shutdown DMA controller */
/* ****************************************************************************** */
void csl_dma_shutdown_all_channel(void)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return;
	}
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	chal_dmux_protect(pdma->dmuxHandle);
#endif
	chal_dma_shutdown_all_channels(pdma->handle);
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	chal_dmux_unprotect(pdma->dmuxHandle);
#endif

	return;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_config_channel */
/*  Description:        Configure channel structure */
/* ****************************************************************************** */
Int32 csl_dma_config_channel(DMA_CHANNEL chanID, Dma_Chan_Info * pChanInfo)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return -1;
	}

	if (chanID >= TOTAL_DMA_CHANNELS)
		return -1;

	if (pChanInfo == NULL)
		return -1;

	pChanInfo->prot = 0;


	pChanInfo->dmaCfgReg =
	    ((pChanInfo->
	      type & DMA_CHCFG_FLOW_CTRL_MASK) << DMA_CHCFG_FLOW_CTRL_SHIFT) |
	    ((pChanInfo->
	      dstID & DMA_CHCFG_DESTPERI_MASK) << DMA_CHCFG_DESTPERI_SHIFT) |
	    ((pChanInfo->
	      srcID & DMA_CHCFG_SRCPERI_MASK) << DMA_CHCFG_SRCPERI_SHIFT);

	pChanInfo->chanNumber = chanID;
	memcpy(&chanArray[chanID].chanInfo, pChanInfo, sizeof(Dma_Chan_Info));
	chanArray[chanID].bUsed = TRUE;

#if (!defined (_HERA_) && !defined (_RHEA_) && !defined (_SAMOA_))
	dma_set_channel_err_intr(chanID, 1);
	dma_set_channel_tc_intr(chanID, 1);
#endif

	return 0;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_get_channel_info */
/*  Description:        Return the channel info structure specified by chanID */
/* ****************************************************************************** */
Dma_Chan_Info *csl_dma_get_channel_info(UInt32 chanID)
{
	if (chanID >= TOTAL_DMA_CHANNELS)
		return NULL;

	return (&chanArray[chanID].chanInfo);
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_obtain_channel */
/*  Description:        Obtain a free DMA channel */
/* ****************************************************************************** */
Int32 csl_dma_obtain_channel(UInt8 srcID, UInt8 dstID)
{
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	dmac_t *pdma = (dmac_t *) & dmac;
#endif
	UInt32 idx, priority;
	Int32 chan = -1;
	Int32 chan_scan_min, chan_scan_max;

	if ((srcID >= DMA_CLIENT_TOTAL) || (dstID >= DMA_CLIENT_TOTAL))
		return chan;

#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	chal_dmux_alloc_channel(pdma->dmuxHandle, &chan);
#else
	chan_scan_min = DMA_CHANNEL_4;
	chan_scan_max = DMA_CHANNEL_11;

#if defined (_ATHENA_)
	if (srcID == DMA_CLIENT_AUDIO_WB_MIXERTAP) {
		chan = DMA_CHANNEL_2;
	} else if (srcID == DMA_CLIENT_AUDIO_IN_FIFO) {
		chan = DMA_CHANNEL_3;
	} else if (dstID == DMA_CLIENT_AUDIO_OUT_FIFO) {
		chan = DMA_CHANNEL_0;
	} else if (dstID == DMA_CLIENT_POLYRING_OUT_FIFO) {
		chan = DMA_CHANNEL_1;
	} else
#endif
	{
				/* Get priority of the client */
				priority = dmaChanPriority[srcID];

		if (priority > dmaChanPriority[dstID])
			priority = dmaChanPriority[dstID];

		switch (priority) {
		case DMA_CHAN_PRI_LOW:
						/* Find one free channel from low priority to high */
				for (idx = chan_scan_max; idx >= chan_scan_min; idx--) {
					if (chanArray[idx].bUsed == FALSE) {
					chan = idx;
					break;
				}
			}
			break;

		case DMA_CHAN_PRI_MEDIUM:
						/* Find one free channel from medium priority to low first */
						for (idx = chan_scan_min + 2; idx <= chan_scan_max; idx++) {
				if (chanArray[idx].bUsed == FALSE) {
					chan = idx;
					break;
				}
			}
			if (chan == chan_scan_max) {
								/* Find one free channel from medium priority to high */
								for (idx = chan_scan_max / 2; idx >= chan_scan_min; idx--) {
					if (chanArray[idx].bUsed == FALSE) {
						chan = idx;
						break;
					}
				}
			}
			break;

		case DMA_CHAN_PRI_HIGH:
						/* Find one free channel from high priority to low */
						for (idx = chan_scan_min; idx <= chan_scan_max / 2; idx++) {
				if (chanArray[idx].bUsed == FALSE) {
					chan = idx;
					break;
				}
			}
			break;

		default:
			break;
		}
	}
#endif

	if (chan >= 0 && chan <= chan_scan_max) {
		chanArray[chan].bUsed = TRUE;
#if defined (_ATHENA_)
		OSDAL_SYSCLK_Enable(OSDAL_SYSCLK_ENABLE_DMAC);
#endif
	}

	return chan;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_release_channel */
/*  Description:        Release the previously allocated channel */
/* ****************************************************************************** */
UInt32 csl_dma_release_channel(DMA_CHANNEL chanID)
{
	dmac_t *pdma = (dmac_t *) & dmac;
	UInt32 i, ret = 0;

	if (!pdma->initialized) {
		dprintf(1,
			"csl_dma: csl_dma_release_channel: dmac has not been initialized\n");
		return 0;
	}

	if (chanID >= TOTAL_DMA_CHANNELS) {
		dprintf(1,
			"csl_dma: csl_dma_release_channel: invalid chanID\n");
		return 0;
	}

	if (chanArray[chanID].bUsed) {
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
		chal_dmux_dealloc_channel(pdma->dmuxHandle, (UInt32) chanID);
		if ((chanArray[chanID].src_peri_id != 0xFF)
		    || (chanArray[chanID].dst_peri_id != 0xFF)) {
			chal_dmux_dealloc_peripheral(pdma->dmuxHandle,
						     (UInt32) chanID);
		}
#else
		if (chanArray[chanID].multiLLIs) {
			for (i = 0; i < ASSOCIATE_CHANNEL; i++) {
				if (assocChanArray[i].baseChan == chanID) {
					assocChanArray[i].bUsed = FALSE;
					memset(&assocChanArray[i], 0,
					       sizeof(ASSOC_CHAN_t));

					chanArray[chanID].bUsed = FALSE;
					memset(&chanArray[chanID], 0,
					       sizeof(Dma_Channel));

					if (chal_dma_free_channel
					    (pdma->handle,
					     TOTAL_DMA_CHANNELS + i)
					    == CHAL_DMA_STATUS_SUCCESS) {
						ret = 1;
					} else {
						dprintf(1,
							"csl_dma: chal_dma_free_channel fail\n");
						ret = 0;
					}
				}
			}

#if defined (_ATHENA_)
			for (i = 0; i < TOTAL_DMA_CHANNELS; i++) {
				if (chanArray[i].bUsed) {
					return ret;
				}
			}
			OSDAL_SYSCLK_Disable(OSDAL_SYSCLK_ENABLE_DMAC);
#endif

			return ret;
		}
#endif
		chanArray[chanID].bUsed = FALSE;
		memset(&chanArray[chanID], 0, sizeof(Dma_Channel));
	} else {
		dprintf(1,
			"csl_dma: csl_dma_release_channel channel not in use\n");
		return 0;
	}

	if (chal_dma_free_channel(pdma->handle, chanID) ==
	    CHAL_DMA_STATUS_SUCCESS) {
		ret = 1;
	} else {
		dprintf(1, "csl_dma: chal_dma_free_channel fail..02\n");
		ret = 0;
	}
#if defined (_ATHENA_)
	for (i = 0; i < TOTAL_DMA_CHANNELS; i++) {
		if (chanArray[i].bUsed) {
			return ret;
		}
	}
	OSDAL_SYSCLK_Disable(OSDAL_SYSCLK_ENABLE_DMAC);

#endif
	return ret;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_wait_channel */
/*  Description:        Wait channel ready */
/* ****************************************************************************** */
void csl_dma_wait_channel(DMA_CHANNEL chanID)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return;
	}

	chal_dma_wait_channel(pdma->handle, chanID);
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_start_transfer */
/*  Description:        Start DMA channel transfer */
/* ****************************************************************************** */
void csl_dma_start_transfer(DMA_CHANNEL chanID, DMADRV_LLI_T pLLI)
{
	dmac_t *pdma = (dmac_t *) & dmac;
	UInt32 assocChan = ASSOC_CHAN_NONE;
	ASSOC_CHAN_t *pAssocChan = (ASSOC_CHAN_t *) pLLI;

#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	chal_dmac_ucode_t ucode;
	ucode.alloc = dma_ucode_alloc;
	ucode.coherency = dma_ucode_coherency;
	ucode.free = dma_ucode_free;
#endif

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return;
	}
		/* Set AHB clock request generated on per-channel basis
		 * This is to ensure Athena enters deep sleep/pedestal mode
		 */
		chanArray[chanID].busy = TRUE;
		board_sysconfig(SYSCFG_CSL_DMA, SYSCFG_DISABLE);
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	chal_dma_prepare_transfer(pdma->handle, chanID, &ucode);
	chal_dmux_protect(pdma->dmuxHandle);
	chal_dma_start_transfer(pdma->handle, chanID, &ucode);
	chal_dmux_unprotect(pdma->dmuxHandle);
#else
	if (pLLI != NULL) {
		assocChan = pAssocChan->assocChan;
	} else {
		assocChan = ASSOC_CHAN_NONE;
	}
	chal_dma_start_transfer(pdma->handle, chanID, assocChan);
#endif

	return;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_stop_transfer */
/*  Description:        Stop DMA channel transfer */
/* ****************************************************************************** */
void csl_dma_stop_transfer(DMA_CHANNEL chanID)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return;
	}

	chal_dma_shutdown_chan(pdma->handle, chanID);

	return;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_force_shutdown_channel */
/*  Description:        Stop DMA transfer on the specified channel and lost all */
/*                  data in the FIFO. */
/* ****************************************************************************** */
void csl_dma_force_shutdown_channel(DMA_CHANNEL chanID)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		dprintf(1,
			"csl_dma: csl_dma_force_shutdown_channel: dmac has not been initialized\n");
		return;
	}

	/*
	 * stop the DMA channel with data loss
	 * You can disable a DMA channel in the following ways:
	 * 1. Write directly to the Channel Enable bit.
	 *    You lose any outstanding data in the FIFOs if you use this method.
	 * 2. Use the Active and Halt bits in conjunction with the Channel Enable bit.
	 * 3. Wait until the transfer completes. The channel is then automatically disabled.
	 *
	 */

	chal_dma_force_shutdown_chan(pdma->handle, chanID);

	if (csl_dma_release_channel(chanID) == 0) {
		dprintf(1,
			"csl_dma: csl_dma_release_channel fail, chanID: %d\n",
			chanID);
	}

	return;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_shutdown_channel */
/*  Description:        Terminate a DMA transfer gracefully without data lose */
/* ****************************************************************************** */
void csl_dma_shutdown_channel(DMA_CHANNEL chanID)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return;
	}

	/*
	 *  To disable a DMA channel without losing data in the FIFO:
	 *  1. Set the Halt bit in the relevant channel Configuration Register.
	 *     This causes any subsequent DMA requests to be ignored.
	 *  2. Poll the Active bit in the relevant channel Configuration Register until
	 *     it reaches 0. This bit indicates whether there is any data in the
	 *     channel that has to be transferred.
	 *  3. Clear the Channel Enable bit in the relevant channel Configuration Register.
	 *
	 */

#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	chal_dmux_protect(pdma->dmuxHandle);
#endif
	chal_dma_shutdown_chan(pdma->handle, chanID);
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	chal_dmux_unprotect(pdma->dmuxHandle);
#endif

	return;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_set_repeat_mode */
/*  Description:        This function sets the DMA list to form a closed loop so */
/*                  the tail of the list points to the head of the list. By */
/*                  this way, the DMA channel can be put in a continues loop */
/*                  transfer data from the buffer list util it is shut down. */
/* ****************************************************************************** */
UInt32 csl_dma_set_repeat_mode(DMA_CHANNEL channel)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return 0;
	}

	chal_dma_set_circular_mode(pdma->handle, channel);

	return 1;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_bind_data */
/*  Description:        Build up scatter gather link list for DMA transfer. */
/*                  The assumption here is total number of DMA descriptors is big */
/*                  enough for any scatter gather linked list. */
/*                  There are three modes: */
/*                  1. Multi buffer scatter & gather, where multi buffers are */
/*                  passed in for DMA transfer. For now, we assume all the buffers */
/*                  are locked down, meaning data are flushed to memory from cache */
/*                  2. Source incremental mode. Source buffer address is incremented */
/*                  by a specified value multiple times while destination address */
/*                  remains the same */
/*                  3. Source/Destination incremental mode. Source/Destination buffer */
/*                  addresses are both incremented multiple times */
/* ****************************************************************************** */
Int32 csl_dma_bind_data(DMA_CHANNEL chanID,
			Dma_Data * pData,
			Boolean multiLLIs, DMADRV_LLI_T * pLLI)
{
	UInt32 index, i, nodeNum, assoc_chan = 0;
	dmac_t *pdma = (dmac_t *) & dmac;
#if (!defined (_HERA_) && !defined (_RHEA_) && !defined (_SAMOA_))
	ChalDmaChanConfig_t chanConfig;
#endif
	ChalDmaBufferDesc_t pDesc;

	/* dprintf(1, "csl_dma: csl_dma_build_linked_descriptor\n"); */

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return 0;
	}

	if (multiLLIs) {
		chanArray[chanID].multiLLIs = TRUE;
	} else {
		chanArray[chanID].multiLLIs = FALSE;
	}

#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
	pDesc.control.endianSwapSize = 0;
	pDesc.control.dstCacheCtrl = 0;
	pDesc.control.dstProtCtrl = chanArray[chanID].chanInfo.prot;
	pDesc.control.dstBurstLen = chanArray[chanID].chanInfo.dstBstLength;
	pDesc.control.dstBurstSize = chanArray[chanID].chanInfo.dstBstSize;
	pDesc.control.srcCacheCtrl = 0;
	pDesc.control.srcProtCtrl = chanArray[chanID].chanInfo.prot;
	pDesc.control.srcBurstLen = chanArray[chanID].chanInfo.srcBstLength;
	pDesc.control.srcBurstSize = chanArray[chanID].chanInfo.srcBstSize;
	pDesc.event_wait = CHAL_DMA_EVENT_INVALID;
	pDesc.event_send = CHAL_DMA_EVENT_INVALID;
	pDesc.bCircular = chanArray[chanID].chanInfo.bCircular;
#else
	pDesc.control.prot = chanArray[chanID].chanInfo.prot;
	pDesc.control.dstMaster = chanArray[chanID].chanInfo.dstMaster;
	pDesc.control.srcMaster = chanArray[chanID].chanInfo.srcMaster;;
	pDesc.control.dstWidth = chanArray[chanID].chanInfo.dstDataWidth;
	pDesc.control.srcWidth = chanArray[chanID].chanInfo.srcDataWidth;
	pDesc.control.dstBSize = chanArray[chanID].chanInfo.dstBstSize;
	pDesc.control.srcBSize = chanArray[chanID].chanInfo.srcBstSize;
#endif

	switch (chanArray[chanID].chanInfo.incMode) {
	case DMA_INC_MODE_SRC:
		pDesc.control.srcIncrement = 1;
		pDesc.control.dstIncrement = 0;
		break;

	case DMA_INC_MODE_DST:
		pDesc.control.srcIncrement = 0;
		pDesc.control.dstIncrement = 1;
		break;

	case DMA_INC_MODE_BOTH:
		pDesc.control.dstIncrement = 1;
		pDesc.control.srcIncrement = 1;
		break;

	default:
		pDesc.control.dstIncrement = 0;
		pDesc.control.srcIncrement = 0;
		break;
	}

#if (!defined (_HERA_) && !defined (_RHEA_) && !defined (_SAMOA_))
	chanConfig.locked = 0;
	chanConfig.tcIntMask = 1;
	chanConfig.errIntMask = 1;
	chanConfig.flowCtrl = chanArray[chanID].chanInfo.type;
	chanConfig.dstPeripheral = chanArray[chanID].chanInfo.dstID;
	chanConfig.srcPeripheral = chanArray[chanID].chanInfo.srcID;

	/* Configure dma channel */
	nodeNum = 0;
	for (i = 0; i < pData->numBuffer; i++) {
		nodeNum +=
		    (pData->pBufList->buffers[i].length) /
		    (pdma->pCap.maxXferSize) + 1;
	}

	chanConfig.nodeNumRequested = nodeNum;

	if (multiLLIs) {
		for (assoc_chan = 0; assoc_chan < ASSOCIATE_CHANNEL;
		     assoc_chan++) {
			if (assocChanArray[assoc_chan].bUsed == FALSE) {
				assocChanArray[assoc_chan].bUsed = TRUE;
				assocChanArray[assoc_chan].baseChan = chanID;
				assocChanArray[assoc_chan].assocChan =
				    assoc_chan;
				*pLLI =
				    (DMADRV_LLI_T) & assocChanArray[assoc_chan];

				chal_dma_allocate_specific_channel(pdma->handle,
								   TOTAL_DMA_CHANNELS
								   +
								   assoc_chan);

				chal_dma_config_channel(pdma->handle, chanID,
							assoc_chan,
							(ChalDmaChanConfig_t *)
							& chanConfig);
				break;
			}
		}
		if (assoc_chan == ASSOCIATE_CHANNEL) {
			return 0;
		}
	} else {
		if (chal_dma_allocate_specific_channel(pdma->handle, chanID) == 0xFFFFFFFF) {
			pr_err("%s - chal cannot allocate specific channel %d\n", __func__,chanID);
			return 0;
		}

		if (chal_dma_config_channel(pdma->handle, chanID, ASSOC_CHAN_NONE,
					(ChalDmaChanConfig_t *) & chanConfig) == 0) {
			pr_err("%s - chal has exceeded number of LLI's channle %d\n", __func__, chanID);
			return 0;
		}
	}
#else
	chanArray[chanID].src_peri_id = 0xFF;
	chanArray[chanID].dst_peri_id = 0xFF;
	if ((chanArray[chanID].chanInfo.srcID != DMA_CLIENT_MEMORY)
	    && (chanArray[chanID].chanInfo.dstID != DMA_CLIENT_MEMORY)) {
		chal_dmux_alloc_peripheral(pdma->dmuxHandle,
					   chanID,
					   chanArray[chanID].chanInfo.srcID,
					   chanArray[chanID].chanInfo.dstID,
					   &(chanArray[chanID].src_peri_id),
					   &(chanArray[chanID].dst_peri_id)
		    );
	} else if (chanArray[chanID].chanInfo.srcID != DMA_CLIENT_MEMORY) {
		chal_dmux_alloc_peripheral(pdma->dmuxHandle,
					   chanID,
					   chanArray[chanID].chanInfo.srcID,
					   CHAL_DMUX_EP_INVALID,
					   &(chanArray[chanID].src_peri_id),
					   NULL);
	} else if (chanArray[chanID].chanInfo.dstID != DMA_CLIENT_MEMORY) {
		chal_dmux_alloc_peripheral(pdma->dmuxHandle,
					   chanID,
					   chanArray[chanID].chanInfo.dstID,
					   CHAL_DMUX_EP_INVALID,
					   &(chanArray[chanID].dst_peri_id),
					   NULL);
	}
#endif

	for (index = 0; index < pData->numBuffer; index++) {
		if (index == 0) {
			pDesc.firstBuffer = 1;
		} else {
			pDesc.firstBuffer = 0;
		}

		pDesc.src = pData->pBufList->buffers[index].srcAddr;
		pDesc.dst = pData->pBufList->buffers[index].destAddr;
		pDesc.size = pData->pBufList->buffers[index].length;

#if (!defined (_HERA_) && !defined (_RHEA_) && !defined (_SAMOA_))
		if (pData->pBufList->buffers[index].interrupt)
			pDesc.control.tcIntEnable = 1;
		else
			pDesc.control.tcIntEnable = 0;
#endif

		/* Add dma buffer */
#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
		chal_dma_add_buffer(pdma->handle,
				    chanID,
				    chanArray[chanID].src_peri_id,
				    chanArray[chanID].dst_peri_id,
				    (ChalDmaBufferDesc_t *) & pDesc);
#else
		if (multiLLIs) {
			chal_dma_add_buffer(pdma->handle,
					    TOTAL_DMA_CHANNELS + assoc_chan,
					    (ChalDmaBufferDesc_t *) & pDesc,
					    pdma->pCap.maxXferSize);
		} else {
			chal_dma_add_buffer(pdma->handle, chanID,
					    (ChalDmaBufferDesc_t *) & pDesc,
					    pdma->pCap.maxXferSize);
		}
#endif
	}

#if (!defined (_HERA_) && !defined (_RHEA_) && !defined (_SAMOA_))
	if (!multiLLIs) {
		if (chanArray[chanID].chanInfo.bCircular) {
			chal_dma_set_circular_mode(pdma->handle, chanID);
		}
	}
#endif

	return 1;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_process_callback */
/*  Description:        This function goes through all the DMA channels to check */
/*                  if a terminal count or error interrupt is set for the channel. */
/*                  If a interrupt is set, then it checks whether a call back */
/*                  function has been set for the channel. If a callback function */
/*                  is set, then it will invoke the callback function. */
/* ****************************************************************************** */
void csl_dma_process_callback(DMA_Interrupt_t * intStatus)
{
	UInt32 channel, mask, chnl_count;
	DMA_Interrupt_t localIntStatus;
	UInt32 flags;

	DEFINE_SPINLOCK(lock);	
	spin_lock_irqsave(&lock, flags);
	localIntStatus.errInt = intStatus->errInt;
	localIntStatus.tcInt  = intStatus->tcInt;
	intStatus->errInt = 0;
	intStatus->tcInt = 0;
	spin_unlock_irqrestore(&lock, flags);

	/* dprintf(1, "csl_dma: csl_dma_process_callback\n"); */

	for (channel = 0; channel < TOTAL_DMA_CHANNELS; channel++) {
		mask = (0x1 << channel);
		if (chanArray[channel].bUsed == FALSE)
			continue;

#if (defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_))
		if (chanArray[channel].bUsed
		    && chanArray[channel].chanInfo.xferCompleteCb) {
			chanArray[channel].chanInfo.
			    xferCompleteCb((DMADRV_CALLBACK_STATUS_t)
					   DMADRV_CALLBACK_OK);
		}
#else
		
		if (localIntStatus.errInt & mask) {
			/* dprintf(1, "Eirlys errInt, channel: %d, %d\n", channel, TIMER_GetValue()); */
			if (chanArray[channel].bUsed
			    && chanArray[channel].chanInfo.xferCompleteCb) {
				chanArray[channel].chanInfo.
				    xferCompleteCb((DMADRV_CALLBACK_STATUS_t)
						   DMADRV_CALLBACK_FAIL);
				spin_lock_irqsave(&lock, flags);
				if (chanArray[channel].chanInfo.freeChan) {
					csl_dma_release_channel((DMA_CHANNEL)
								channel);
				}
				spin_unlock_irqrestore(&lock, flags);
			}
		} else if (localIntStatus.tcInt & mask) {
			if (chanArray[channel].bUsed
			    && chanArray[channel].chanInfo.xferCompleteCb) {
				/* dprintf(1, "Eirlys tcInt, %d, %d, 0x%x\n", channel, */
				/*        TIMER_GetValue(), chanArray[channel].chanInfo.xferCompleteCb); */

				chanArray[channel].chanInfo.
				    xferCompleteCb((DMADRV_CALLBACK_STATUS_t)
						   DMADRV_CALLBACK_OK);
				spin_lock_irqsave(&lock, flags);
					chanArray[channel].busy = FALSE;
				if (chanArray[channel].chanInfo.freeChan) {
					csl_dma_release_channel((DMA_CHANNEL)
								channel);
				}
				spin_unlock_irqrestore(&lock, flags);
			}
		}
#endif
	}
		for (chnl_count = 0; chnl_count < TOTAL_DMA_CHANNELS;
		     chnl_count++) {
			if (chanArray[chnl_count].busy == TRUE) {
				return;
			}
		}
		/* Set AHB clock request generated on per-channel basis
		 * This is to ensure Athena enters deep sleep/pedestal mode*/
		board_sysconfig(SYSCFG_CSL_DMA, SYSCFG_ENABLE);
	return;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_get_intr_status */
/*  Description:        Get DMA terminal count and error interrupt status */
/* ****************************************************************************** */
void csl_dma_get_intr_status(DMA_Interrupt_t * intStatus)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return;
	}
#if (!defined (_HERA_) && !defined (_RHEA_) && !defined (_SAMOA_))
	chal_dma_get_int_status(pdma->handle, (ChalDmaIntStatus_t *) intStatus);
#endif
	return;
}

/* ****************************************************************************** */
/*  Funciton Name:      csl_dma_clear_intr_status */
/*  Description:        Write a bit mask to clear DMA terminal count and error interrupt */
/* ****************************************************************************** */
void csl_dma_clear_intr_status(UInt32 mask)
{
	dmac_t *pdma = (dmac_t *) & dmac;

	if (!pdma->initialized) {
		/* dprintf(1, "csl_dma: dmac has not been initialized\n"); */
		return;
	}

	chal_dma_clear_int_status(pdma->handle, mask);

	return;
}
