/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
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

//============================================================================
//
// \file   chal_dma.c
// \brief  OS independent code of DMA hardware abstraction APIs.
// \note
//============================================================================
#include <plat/types.h>
#include "plat/chal/chal_types.h"
#include "plat/chal/chal_common.h"
#include "plat/rdb/brcm_rdb_dmac.h"
#include "plat/chal/chal_dma.h"


#define DMA_LLI_NODE_SIZE       16
#define DMA_MAX_XFER_SIZE       4095
#define DMA_MAX_BURST_SIZE      256

#define LLI_NODE_BAND_NUM       32
#define MAX_BAND_NUM_PER_CHAN   24

#define CHAN_REG_INCREMENT      0x20

typedef enum
{
    DMA_CHANNEL_0 = 0,
    DMA_CHANNEL_1,
    DMA_CHANNEL_2,
    DMA_CHANNEL_3,
    DMA_CHANNEL_4,
    DMA_CHANNEL_5,
    DMA_CHANNEL_6,
    DMA_CHANNEL_7,
    DMA_CHANNEL_8,
    DMA_CHANNEL_9,
    DMA_CHANNEL_10,
    DMA_CHANNEL_11,
    TOTAL_DMA_CHANNELS
} DMA_CHANNEL;


typedef union
{
    cUInt32 DWord;
    struct
    {
        cUInt32  xferSize      : 12;
        cUInt32  srcBSize      : 3;
        cUInt32  dstBSize      : 3;
        cUInt32  srcWidth      : 3;
        cUInt32  dstWidth      : 3;
        cUInt32  srcMaster     : 1;
        cUInt32  dstMaster     : 1;
        cUInt32  srcIncrement  : 1;
        cUInt32  dstIncrement  : 1;
        cUInt32  prot          : 3;                                
        cUInt32  tcIntEnable   : 1;
    } ChanCtrlBitField;
}DmaChanControl_t;


typedef union
{
    cUInt32 DWord;
    struct
    {
        cUInt32  chanEnabled   : 1;
        cUInt32  srcPeripheral : 5;
        cUInt32  dstPeripheral : 5;
        cUInt32  flowCtrl      : 3;
        cUInt32  errIntMask    : 1;                
        cUInt32  tcIntMask     : 1;
        cUInt32  locked        : 1;
        cUInt32  active        : 1;
        cUInt32  halt          : 1;                
        cUInt32  bit31_19      : 13;
    } ChanConfigBitField;
}DmaChanConfig_t;

typedef struct
{
    cUInt32  src;
    cUInt32  dst;
    cUInt32  next;
    DmaChanControl_t control;
}ChalDmaLinkNode_t;

typedef struct
{
    ChalDmaLinkNode_t *firstNode;
    cUInt32 nodeNum;
    cUInt32 currNode;
    cBool bUsed;
}ChalLLIBand_t;
    

typedef struct 
{
    ChalLLIBand_t *chanNodeBand[MAX_BAND_NUM_PER_CHAN];
    cUInt32 bandNum;
    ChalDmaLinkNode_t *headNode;
    ChalDmaLinkNode_t *currNode;
    cUInt32 active;
    cUInt32 refCount;
    cBool bUsed;
}ChalDmaChanInfo_t;


typedef struct
{
    cUInt32                  baseAddr;
    ChalDmaDevCapabilities_t capabilities;
    ChalDmaDevNodeMemory_t   nodeMem;
    ChalLLIBand_t            LLIBand[LLI_NODE_BAND_NUM];
    ChalDmaChanInfo_t        chanInfo[TOTAL_DMA_CHANNELS + ASSOCIATE_CHANNEL];
    cUInt32                  nodeNumPerBand;
    cUInt32                  currNodeLocation;
}ChalDmaDev_t;


//******************************************************************************
// Local Variables
//******************************************************************************

static ChalDmaDev_t DmaDev;
static const cUInt32 TRANSFER_WIDTH[]={1, 2, 4};

//******************************************************************************
// Local Functions
//******************************************************************************


//******************************************************************************
//
// Function Name:  chal_dma_get_first_node
//
// Description:  Get first LLI allocated for the channel
//      
//******************************************************************************
static ChalDmaLinkNode_t *chal_dma_get_first_node(CHAL_HANDLE handle, cUInt32 chan)
{
    ChalDmaLinkNode_t *node;
    cUInt32 i;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
   
    if(pDmaDev->chanInfo[chan].bandNum > 0)
    {
        //reset current node position = 0
        for(i=0;i<pDmaDev->chanInfo[chan].bandNum;i++)
        {
            pDmaDev->chanInfo[chan].chanNodeBand[i]->currNode = 0;
        }
         
        //get first node   
        node = pDmaDev->chanInfo[chan].chanNodeBand[0]->firstNode;
        pDmaDev->chanInfo[chan].chanNodeBand[0]->currNode ++;
        
    }
    else
    {
        node = NULL;
    }
    
    return node;    
}

//******************************************************************************
//
// Function Name:  chal_dma_get_next_node
//
// Description:  Get an unused LLI for a channel
//      
//******************************************************************************
static ChalDmaLinkNode_t *chal_dma_get_next_node(CHAL_HANDLE handle, cUInt32 chan)
{
    ChalDmaLinkNode_t *node;
    ChalDmaLinkNode_t *nodeTemp;
    cUInt32 i;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
       
    //reset current node position = 0
    for(i=0;i<pDmaDev->chanInfo[chan].bandNum;i++)
    {
        if(pDmaDev->chanInfo[chan].chanNodeBand[i]->currNode < pDmaDev->chanInfo[chan].chanNodeBand[i]->nodeNum)
            break;
    }
    if(i != pDmaDev->chanInfo[chan].bandNum)
    {
        nodeTemp = pDmaDev->chanInfo[chan].chanNodeBand[i]->firstNode;
        node = nodeTemp + pDmaDev->chanInfo[chan].chanNodeBand[i]->currNode;
        pDmaDev->chanInfo[chan].chanNodeBand[i]->currNode ++;
    }
    else 
    {
        node = NULL;
    }       
            
    return node;
}

//******************************************************************************
//
// Function Name:  chal_dma_get_lli_physical_addr
//
// Description:  Convert LLI virtual address to physical address
//      
//******************************************************************************
static cUInt32 chal_dma_get_lli_physical_addr(CHAL_HANDLE handle, ChalDmaLinkNode_t *node)
{
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    return (pDmaDev->nodeMem.LLIPhysicalAddr + (cUInt32)node - pDmaDev->nodeMem.LLIVirtualAddr);
}
    
//******************************************************************************
//
// Function Name:  chal_dma_disable_dmac
//
// Description:  disable dma
//      
//******************************************************************************
void chal_dma_disable_dmac(CHAL_HANDLE handle)
{
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    temp = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_CONFIG_OFFSET); 
   
    if (!(temp & DMAC_CONFIG_E_MASK))
    {
        
    }
    else {
        temp &= ~DMAC_CONFIG_E_MASK;
        CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CONFIG_OFFSET, temp);
    }
}

//******************************************************************************
//
// Function Name:  chal_dma_enable_dmac
//
// Description:  enable dma
//      
//******************************************************************************
void chal_dma_enable_dmac(CHAL_HANDLE handle)
{
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    temp = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_CONFIG_OFFSET);
    if (temp & DMAC_CONFIG_E_MASK)
    {
        //TRACE_Printf_Sio("Already Ensabled");
    }
    else {
        temp |= DMAC_CONFIG_E_MASK;
        CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CONFIG_OFFSET, temp);
    }
}

//******************************************************************************
//
// Function Name:  chal_dma_mark_channel_active
//
// Description:  Mark DMA channel active
//      
//******************************************************************************
static void chal_dma_mark_channel_active(CHAL_HANDLE handle, cUInt32 channel)
{
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    if( pDmaDev->chanInfo[channel].active == 0 ) 
    {
        pDmaDev->chanInfo[channel].active = 1;
        pDmaDev->chanInfo[channel].refCount ++;
        
        if( pDmaDev->chanInfo[channel].refCount == 1 )
            chal_dma_enable_dmac(handle);
    }
}

//******************************************************************************
//
// Function Name:  chal_dma_mark_channel_inactive
//
// Description:  Mark DMA channel inactive
//      
//******************************************************************************
static void chal_dma_mark_channel_inactive(CHAL_HANDLE handle, cUInt32 channel)
{
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    cUInt32 i;    

    if (pDmaDev->chanInfo[channel].active == 1) {
        pDmaDev->chanInfo[channel].active = 0;
        pDmaDev->chanInfo[channel].refCount--;
    }

    for(i = 0; i < TOTAL_DMA_CHANNELS + ASSOCIATE_CHANNEL; i ++)
    {
        if (pDmaDev->chanInfo[i].refCount != 0)
            return;
    }
    chal_dma_disable_dmac(handle);
}

//******************************************************************************
//
// Function Name: chal_dma_init
//
// Description:   Initialize DMA hardware and software interface
//
//******************************************************************************
CHAL_HANDLE chal_dma_init(cUInt32 baseAddress)
{
    int i;
    ChalDmaDev_t *pDmaDev;
         
    pDmaDev = &DmaDev;
    
    pDmaDev->baseAddr = baseAddress;
    
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTTCCLEAR_OFFSET, 0xFF);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTERRCLR_OFFSET, 0xFF);

    //
    // clear any m_DMAC_ENBLDCHNS before using dma
    //
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_ENBLDCHNS_OFFSET, 0);
    for (i=0; i<TOTAL_DMA_CHANNELS; i++)
       CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0CONFIG_OFFSET 
                        + CHAN_REG_INCREMENT * i, 0);

    //
    // clear any PENDING interrupt
    //
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTTCCLEAR_OFFSET, 0xFF);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTERRCLR_OFFSET, 0xFF);

    // disable DMA controller for now to save power
    chal_dma_disable_dmac((CHAL_HANDLE)pDmaDev);
    
    //populate capability info
    pDmaDev->capabilities.numOfChannel = TOTAL_DMA_CHANNELS;
    pDmaDev->capabilities.chanLLISize = DMA_LLI_NODE_SIZE;
    pDmaDev->capabilities.maxBurstSize = DMA_MAX_BURST_SIZE;
    pDmaDev->capabilities.maxXferSize = DMA_MAX_XFER_SIZE;
    pDmaDev->capabilities.maxLLINodeNum = 0; //unlimited
    pDmaDev->capabilities.fixedWordWidth = 0;
    
    return (CHAL_HANDLE) pDmaDev;
}

//******************************************************************************
//
// Function Name: chal_dma_config_device_memory
//
// Description:   Device level configuration
//
//******************************************************************************
cUInt32 chal_dma_config_device_memory(CHAL_HANDLE handle, ChalDmaDevNodeMemory_t *pDevNode)
{
    cUInt32 nodeNumPerBand;
    cUInt32 i;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    pDmaDev->nodeMem.LLIVirtualAddr = pDevNode->LLIVirtualAddr;
    pDmaDev->nodeMem.LLIPhysicalAddr = pDevNode->LLIPhysicalAddr;
    pDmaDev->nodeMem.size = pDevNode->size;

    nodeNumPerBand = pDevNode->size/(DMA_LLI_NODE_SIZE*LLI_NODE_BAND_NUM); 
    pDmaDev->nodeNumPerBand = nodeNumPerBand;
    
    for(i=0;i<LLI_NODE_BAND_NUM; i++) //sub-divide LLI memory to different bands
    {
        pDmaDev->LLIBand[i].firstNode = (ChalDmaLinkNode_t *)(pDevNode->LLIVirtualAddr + 
                                                             i*nodeNumPerBand*DMA_LLI_NODE_SIZE);
        pDmaDev->LLIBand[i].nodeNum = nodeNumPerBand;
        pDmaDev->LLIBand[i].bUsed = FALSE;
    }
    
    //update maximum real LLI node num supported
    pDmaDev->capabilities.maxLLINodeNum = MAX_BAND_NUM_PER_CHAN * nodeNumPerBand;
 
    return 1;         
}

//******************************************************************************
//
// Function Name: chal_dma_get_capabilities
//
// Description:   Get individual capability of the device
//
//******************************************************************************
cUInt32 chal_dma_get_capabilities(CHAL_HANDLE handle, ChalDmaDevCapabilities_t *pCap)
{
    cUInt32 rc=0;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    pCap->numOfChannel = pDmaDev->capabilities.numOfChannel;
    pCap->chanLLISize = pDmaDev->capabilities.chanLLISize;
    pCap->maxBurstSize = pDmaDev->capabilities.maxBurstSize;
    pCap->maxXferSize = pDmaDev->capabilities.maxXferSize;
    pCap->maxLLINodeNum = pDmaDev->capabilities.maxLLINodeNum;
    pCap->fixedWordWidth = pDmaDev->capabilities.fixedWordWidth;

    return rc;
}

//******************************************************************************
//
//
// Function Name:  chal_dma_reset_channel
//
// Description:    Reset a single channel
//
//******************************************************************************
cUInt32 chal_dma_reset_channel(CHAL_HANDLE handle, cUInt16 channel)
{
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    if (channel >= TOTAL_DMA_CHANNELS)
        return 0;

    //chal_dprintf(CDBG_ERRO, "chal_dma_reset_channel() channel: 0x%x\n", channel);

    //
    // clear any pending DMA interrupts, wirte zeros to other
    // bits does not clear interrupt for these bits
    //    
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTTCCLEAR_OFFSET, 0x1 << channel);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTERRCLR_OFFSET, 0x1 << channel);

    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0CONFIG_OFFSET 
                        + CHAN_REG_INCREMENT * channel, 0);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0SRCADDR_OFFSET 
                        + CHAN_REG_INCREMENT * channel, 0);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0DESTADDR_OFFSET 
                        + CHAN_REG_INCREMENT * channel, 0);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0LLI_OFFSET 
                        + CHAN_REG_INCREMENT * channel, 0);

    return 1;
}

//******************************************************************************
//
// Function Name:  chal_dma_shutdown_chan
//
// Description:    Terminate a DMA transfer gracefully without data lose
//
//******************************************************************************
void chal_dma_shutdown_chan(CHAL_HANDLE handle, cUInt32  channel)
{
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    //
    //  To disable a DMA channel without losing data in the FIFO:
    //  1. Set the Halt bit in the relevant channel Configuration Register.
    //     This causes any subsequent DMA requests to be ignored.
    //  2. Poll the Active bit in the relevant channel Configuration Register until 
    //     it reaches 0. This bit indicates whether there is any data in the 
    //     channel that has to be transferred.
    //  3. Clear the Channel Enable bit in the relevant channel Configuration Register.
    //
    //
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTTCCLEAR_OFFSET, 0x1<<channel);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTERRCLR_OFFSET, 0x1<<channel);

    temp = CHAL_REG_READ32(pDmaDev->baseAddr+DMAC_CH0CONFIG_OFFSET 
                                       + CHAN_REG_INCREMENT * channel);
    temp |= DMAC_CH0CONFIG_H_MASK;
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0CONFIG_OFFSET 
                     + CHAN_REG_INCREMENT * channel, temp);

    while (temp & DMAC_CH0CONFIG_A_MASK)
    {
        temp = CHAL_REG_READ32(pDmaDev->baseAddr+DMAC_CH0CONFIG_OFFSET 
                                       + CHAN_REG_INCREMENT * channel);
    }

    temp &= ~DMAC_CH0CONFIG_E_MASK;
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0CONFIG_OFFSET 
                     + CHAN_REG_INCREMENT * channel, temp);
    
    chal_dma_mark_channel_inactive(handle, channel);
}


//******************************************************************************
//
// Function Name: chal_dma_shutdown_all_channels
//
// Description:   disable all the acitve channel and shutdown DMA controller
//
//******************************************************************************
void chal_dma_shutdown_all_channels(CHAL_HANDLE handle)
{
    cUInt32 i, temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;

    //disable all active channels
    temp = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_ENBLDCHNS_OFFSET);
    for (i=0; i < TOTAL_DMA_CHANNELS; i++)
    {
       if (temp & (0x1 << i))
       {
           chal_dma_shutdown_chan(handle,i);
           chal_dma_reset_channel(handle,i);
       }
    }

    // disable DMA controller
    chal_dma_disable_dmac(handle);
}

//******************************************************************************
//
// Function Name: chal_dma_allocate_specific_channel
//
// Description:   Get a specific DMA channel 
//
//******************************************************************************
cUInt32 chal_dma_allocate_specific_channel(CHAL_HANDLE handle, cUInt32 chan)
{
    cInt32  rc = 0xffffffff;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;

    if(pDmaDev->chanInfo[chan].bUsed == FALSE)
    {
        pDmaDev->chanInfo[chan].bUsed = TRUE;
        return (cInt32)chan;
    }
    
    pr_err("%s - requested channel %d is currently used\n", __func__, chan);  
    return rc;
}
                   
//******************************************************************************
//
// Function Name: chal_dma_config_channel
//
// Description:   Config a dma channel, i.e, in this case channel config reg
//
//******************************************************************************
cUInt32 chal_dma_config_channel(
    CHAL_HANDLE handle, 
    cUInt32 chan, 
    cUInt32 assocChan, 
    ChalDmaChanConfig_t *pConfig
)
{
    DmaChanConfig_t chanConfig;
    cUInt32 bandRequested;
    cUInt32 i, j, k, configChan;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
        
    chal_dma_enable_dmac(handle);
    chal_dma_reset_channel(handle, chan);

    chanConfig.DWord = CHAL_REG_READ32(pDmaDev->baseAddr+DMAC_CH0CONFIG_OFFSET 
                                       + CHAN_REG_INCREMENT * chan);

    chanConfig.ChanConfigBitField.locked = pConfig->locked;
    chanConfig.ChanConfigBitField.tcIntMask = pConfig->tcIntMask;
    chanConfig.ChanConfigBitField.errIntMask = pConfig->errIntMask;
    chanConfig.ChanConfigBitField.flowCtrl = pConfig->flowCtrl;
    chanConfig.ChanConfigBitField.dstPeripheral = pConfig->dstPeripheral;
    chanConfig.ChanConfigBitField.srcPeripheral = pConfig->srcPeripheral;
 
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0CONFIG_OFFSET 
                     + CHAN_REG_INCREMENT * chan, chanConfig.DWord);

//    chal_dma_disable_dmac(handle);

    if(assocChan == ASSOC_CHAN_NONE)
    {
        configChan = chan;
    }
    else
    {
        configChan = TOTAL_DMA_CHANNELS + assocChan;
    }

    //allocate pre-allocated node
    if(pDmaDev->chanInfo[configChan].bUsed == TRUE)
    {
        pDmaDev->chanInfo[configChan].bandNum = 0;
        bandRequested = pConfig->nodeNumRequested/pDmaDev->nodeNumPerBand + 1; //always allocate at least a band 
        for(i=0;i<bandRequested;i++)
        {
            for(j=0;j<LLI_NODE_BAND_NUM; j++)
            {
               
                if(pDmaDev->LLIBand[j].bUsed == FALSE) //free band
                {
                    pDmaDev->chanInfo[configChan].chanNodeBand[i] = &pDmaDev->LLIBand[j]; //reserve the band
                    pDmaDev->chanInfo[configChan].bandNum ++;
                    pDmaDev->LLIBand[j].bUsed = TRUE; //mark it
                    break;
                }
            }
            if(j==LLI_NODE_BAND_NUM) //no more LLI node available
            {
                for(k=0;k<pDmaDev->chanInfo[configChan].bandNum;k++)
                {
                    pDmaDev->chanInfo[configChan].chanNodeBand[k]->bUsed = FALSE;
                    
                }
                pDmaDev->chanInfo[configChan].bandNum = 0; //reset count
	        	pr_err("%s - exceeded available LLI's\n", __func__);
                return 0;
            }
        }
                    
    } 
    return 1;
}

//******************************************************************************
//
// Function Name: chal_dma_add_buffer
//
// Description:   Add first buffer to a DMA channel
//
//******************************************************************************
cUInt32 chal_dma_add_buffer(CHAL_HANDLE handle, cUInt32 chan, ChalDmaBufferDesc_t *pDesc,
                            UInt32 lliXferSize)
{
    ChalDmaLinkNode_t *currNode;
    ChalDmaLinkNode_t *nextNode;
    cUInt32 src, dst;
    DmaChanControl_t control;
    cUInt32 xfer;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
  
#if 0
    chal_dprintf(CDBG_ERRO, "chal_dma_add_buffer() chan: %d\n", chan);
    chal_dprintf(CDBG_ERRO, "%d, 0x%x, 0x%x, 0x%x\n", 
        pDesc->firstBuffer,
        pDesc->src,
        pDesc->dst,
        pDesc->size
        );
#endif  

    if (lliXferSize < DMA_MAX_XFER_SIZE)
      return 0;
          
    xfer = pDesc->size/TRANSFER_WIDTH[pDesc->control.srcWidth];
    src = pDesc->src;
    dst = pDesc->dst;

    control.DWord = 0;
    control.ChanCtrlBitField.tcIntEnable = 0; //always disabled until last one
    control.ChanCtrlBitField.prot = pDesc->control.prot;
    control.ChanCtrlBitField.dstIncrement = pDesc->control.dstIncrement;
    control.ChanCtrlBitField.srcIncrement = pDesc->control.srcIncrement;
    control.ChanCtrlBitField.srcMaster = pDesc->control.srcMaster;
    control.ChanCtrlBitField.dstMaster = pDesc->control.dstMaster;
    control.ChanCtrlBitField.dstWidth = pDesc->control.dstWidth;
    control.ChanCtrlBitField.srcWidth = pDesc->control.srcWidth;
    control.ChanCtrlBitField.dstBSize = pDesc->control.dstBSize;
    control.ChanCtrlBitField.srcBSize = pDesc->control.srcBSize;
    
    if(pDesc->firstBuffer == 1)
    {
        currNode = chal_dma_get_first_node(handle, chan);
        if(currNode == NULL)
        {
            return pDesc->size; //nothing is added
        }
        
        pDmaDev->chanInfo[chan].headNode = currNode;
    }
    else
    {
        currNode = pDmaDev->chanInfo[chan].currNode;
        
        nextNode = chal_dma_get_next_node(handle, chan);
        if(nextNode == NULL)
        {
            return xfer * TRANSFER_WIDTH[pDesc->control.srcWidth];
        }
         
        currNode->next = chal_dma_get_lli_physical_addr(handle, nextNode); 
        currNode = nextNode;
    }
    
    while(xfer > 0) 
    {        
        currNode->src = src;
        currNode->dst = dst;
        currNode->control.DWord =  control.DWord;
        currNode->next = 0;
        
        if(xfer <= lliXferSize)
        {
            currNode->control.ChanCtrlBitField.xferSize = xfer;
            currNode->control.ChanCtrlBitField.tcIntEnable = pDesc->control.tcIntEnable;
            xfer = 0; //done transfer
            currNode->next = NULL;

#if 0
            chal_dprintf(1, "done currNode->src = 0x%x\n", currNode->src);
            chal_dprintf(1, "done currNode->dst = 0x%x\n", currNode->dst);
            chal_dprintf(1, "done currNode->control.DWord = 0x%x\n", currNode->control.DWord);
            chal_dprintf(1, "done currNode->next = 0x%x\n", currNode->next);
#endif

            break;
        }
        else
        {
            currNode->control.ChanCtrlBitField.xferSize = lliXferSize;
            xfer -= lliXferSize;
        }
        
        //get next node
        nextNode = chal_dma_get_next_node(handle, chan);
        if(nextNode == NULL)
        {

            break;
        }
        
        currNode->next = chal_dma_get_lli_physical_addr(handle, nextNode);
        
#if 0
        chal_dprintf(1, "xfer = 0x%x\n", xfer);
        chal_dprintf(1, "currNode->src = 0x%x\n", currNode->src);
        chal_dprintf(1, "currNode->dst = 0x%x\n", currNode->dst);
        chal_dprintf(1, "currNode->control.DWord = 0x%x\n", currNode->control.DWord);
        chal_dprintf(1, "currNode->next = 0x%x\n", currNode->next);
#endif

        //increment buffer point
        if(control.ChanCtrlBitField.srcIncrement)
        {
            src += lliXferSize * TRANSFER_WIDTH[pDesc->control.srcWidth];
        }
        if(control.ChanCtrlBitField.dstIncrement)
        {
            dst += lliXferSize * TRANSFER_WIDTH[pDesc->control.dstWidth];
        }
        
        currNode = nextNode;
    }
                        
    pDmaDev->chanInfo[chan].currNode = currNode;
    
    //chal_dprintf(1, "chal_dma_add_buffer, OUT xfer = 0x%x\n", xfer);
    return xfer*TRANSFER_WIDTH[pDesc->control.srcWidth];
}

//******************************************************************************
//
// Function Name:  chal_dma_free_channel
//
// Description:    Free the channel previously allocated 
//
//******************************************************************************
CHAL_DMA_STATUS_t chal_dma_free_channel(CHAL_HANDLE handle, cUInt32 channel)
{
    cUInt32 i;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    if (channel >= TOTAL_DMA_CHANNELS + ASSOCIATE_CHANNEL)
        return CHAL_DMA_STATUS_FAILURE;
  
    pDmaDev->chanInfo[channel].bUsed = FALSE;
    pDmaDev->chanInfo[channel].active = 0;
    pDmaDev->chanInfo[channel].refCount = 0;
    pDmaDev->chanInfo[channel].headNode = NULL;
    pDmaDev->chanInfo[channel].currNode = NULL;
        
    //unmark LLI band
    for(i=0;i<pDmaDev->chanInfo[channel].bandNum;i++)
    {
        pDmaDev->chanInfo[channel].chanNodeBand[i]->bUsed = FALSE;
        pDmaDev->chanInfo[channel].chanNodeBand[i]->currNode = 0;
    }
    
    return CHAL_DMA_STATUS_SUCCESS;
}

//******************************************************************************
//
// Function Name:  chal_dma_wait_channel
//
// Description:    Wait for a channel 
//
//******************************************************************************
void chal_dma_wait_channel(CHAL_HANDLE handle, cUInt32 channel)
{
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    do
    {
        temp = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_ENBLDCHNS_OFFSET);
    }while (temp & (0x1 << channel));
}

//******************************************************************************
//
// Function Name:  chal_dma_start_transfer
//
// Description:    Start a DMA transfer by reloading DMA registers
//
//******************************************************************************
void chal_dma_start_transfer(
    CHAL_HANDLE handle, 
    cUInt32 channel,
    cUInt32 assocChan
)
{
    cUInt32 temp;
    ChalDmaLinkNode_t *head;
    DmaChanConfig_t config;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    //chal_dprintf(1, "chal_dma_start_transfer, channel = %d, assocChan = %d\n", channel, assocChan);
  
    //
    // wait the channel to be free
    // You must fully initialize the channel before you enable it. Additionally, you must set the
    // Enable bit of the DMAC before you enable any channels.
    //
  
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTTCCLEAR_OFFSET, 0x1<<channel);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTERRCLR_OFFSET, 0x1<<channel);
  
    do
    {
        temp = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_ENBLDCHNS_OFFSET);
    }while (temp & (0x1 << channel));
  
    if(assocChan == ASSOC_CHAN_NONE)
    {
    //
    // Mark channel active and enable DMA core 
    //
    chal_dma_mark_channel_active(handle, channel);
  
    // 
    //  1. Clear any pending interrupts on the channel you want to use.
    //     The privious channel operation might have left interrupts active.
    //  2. Write the source address into the SRC Register. 
    //  3. Write the destination address into the DEST Register. 
    //  4. Write the address of the next SRC into the NEXT Register. If the transfer 
    //     consists of a single packet of data, you must write 0 into this register.
    //  5. Write the control information into the CONTROL Register. 
    //  6. Write the channel configuration information into the CONFIG register.
    //  7. Set channel enable bit, then the DMA channel is automatically enabled.
    //
  
    head = pDmaDev->chanInfo[channel].headNode;
    }
    else
    {
        chal_dma_mark_channel_active(handle, TOTAL_DMA_CHANNELS + assocChan);
  
        head = pDmaDev->chanInfo[TOTAL_DMA_CHANNELS + assocChan].headNode;
    }
  

#if 0
    chal_dprintf(1, "chal_dma_start_transfer, head->src = 0x%x\n", head->src);
    chal_dprintf(1, "chal_dma_start_transfer, head->dst = 0x%x\n", head->dst);
    chal_dprintf(1, "chal_dma_start_transfer, head->control.DWord = 0x%x\n", head->control.DWord);
    chal_dprintf(1, "chal_dma_start_transfer, head->next = 0x%x\n", head->next);
#endif

    //
    // set up DMA for transfer
    //
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0CONTROL_OFFSET 
                        + CHAN_REG_INCREMENT * channel, head->control.DWord);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0SRCADDR_OFFSET 
                        + CHAN_REG_INCREMENT * channel, head->src);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0DESTADDR_OFFSET 
                        + CHAN_REG_INCREMENT * channel, head->dst);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0LLI_OFFSET 
                        + CHAN_REG_INCREMENT * channel, head->next);

    config.DWord = CHAL_REG_READ32(pDmaDev->baseAddr+DMAC_CH0CONFIG_OFFSET 
                                       + CHAN_REG_INCREMENT * channel);

    config.ChanConfigBitField.chanEnabled = 1;

    //chal_dprintf(1, "chal_dma_start_transfer, config.DWord = 0x%x\n", config.DWord);

    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0CONFIG_OFFSET 
                        + CHAN_REG_INCREMENT * channel, config.DWord);
}

//******************************************************************************
//
//
// Function Name:  chal_dma_force_shutdown_chan
//
// Description:    Stop DMA transfer on the specified channel and lose all
//                 data in the FIFO
//
//******************************************************************************
void chal_dma_force_shutdown_chan(CHAL_HANDLE handle, cUInt32  channel)
{ 
    // 
    // stop the DMA channel with data loss 
    // You can disable a DMA channel in the following ways:
    // 1. Write directly to the Channel Enable bit.
    //    You lose any outstanding data in the FIFOs if you use this method.
    // 2. Use the Active and Halt bits in conjunction with the Channel Enable bit.
    // 3. Wait until the transfer completes. The channel is then automatically disabled.
    //
    //
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    temp = CHAL_REG_READ32(pDmaDev->baseAddr+DMAC_CH0CONFIG_OFFSET 
                           + CHAN_REG_INCREMENT * channel);

    //chal_dprintf(CDBG_ERRO, "chal_dma_force_shutdown_chan() temp: 0x%x\n", temp);

    temp &= ~DMAC_CH0CONFIG_E_MASK;

    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0CONFIG_OFFSET 
                        + CHAN_REG_INCREMENT * channel, temp);

    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTTCCLEAR_OFFSET, 0x1<<channel);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTERRCLR_OFFSET, 0x1<<channel);

    chal_dma_mark_channel_inactive(handle, channel);
}

//******************************************************************************
//
//
// Function Name:  chal_dma_halt_chan
//
// Description:    Halt DMA transfer on the specified channel and lose all
//                 data in the FIFO
//
//******************************************************************************
void chal_dma_halt_chan(CHAL_HANDLE handle, cUInt32  channel)
{ 
    // 
    // stop the DMA channel with data loss 
    // You can disable a DMA channel in the following ways:
    // 1. Write directly to the Channel Enable bit.
    //    You lose any outstanding data in the FIFOs if you use this method.
    // 2. Use the Active and Halt bits in conjunction with the Channel Enable bit.
    // 3. Wait until the transfer completes. The channel is then automatically disabled.
    //
    //
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    temp = CHAL_REG_READ32(pDmaDev->baseAddr+DMAC_CH0CONFIG_OFFSET 
                           + CHAN_REG_INCREMENT * channel);

    temp &= ~DMAC_CH0CONFIG_E_MASK;

    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_CH0CONFIG_OFFSET 
                        + CHAN_REG_INCREMENT * channel, temp);
}

//******************************************************************************
//
// Function Name:  void chal_dma_disable_device
//
// Description:    Disable DMA device. This call does not reset channel and
//                 clean up related data structure
//
//******************************************************************************
void chal_dma_disable_device(CHAL_HANDLE handle)
{    
    chal_dma_shutdown_all_channels(handle);
}

//******************************************************************************
//
// Function Name:  chal_dma_set_circular_mode
//
// Description:    Set circular mode for LLI
//
//******************************************************************************
cUInt32 chal_dma_set_circular_mode(CHAL_HANDLE handle, cUInt32 channel)
{
    ChalDmaLinkNode_t *head;
    ChalDmaLinkNode_t *tail;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;

    if (channel >= TOTAL_DMA_CHANNELS + ASSOCIATE_CHANNEL)
        return 0;

    head = pDmaDev->chanInfo[channel].headNode;
    tail = pDmaDev->chanInfo[channel].currNode;
    
    tail->next = chal_dma_get_lli_physical_addr(handle, head);
    
    return 1;
}

//******************************************************************************
//
// Function Name:  chal_dma_get_int_status
//
// Description:    Returns the DMA controller's interrupt status
//
//******************************************************************************
void chal_dma_get_int_status(CHAL_HANDLE handle, ChalDmaIntStatus_t *intStatus)
{
    UInt32 rc;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
   
    rc = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_INTTCSTAT_OFFSET);
    intStatus->tcInt |= (rc & 0xfff);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTTCCLEAR_OFFSET, rc);
    
    rc = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_INTERRSTAT_OFFSET);
    intStatus->errInt |= (rc & 0xfff);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTERRCLR_OFFSET, rc);
}

//******************************************************************************
//
// Function Name:  chal_dma_clear_int_status
//
// Description:    Clear the DMA controller's interrupt status
//
//******************************************************************************
void chal_dma_clear_int_status(CHAL_HANDLE handle, cUInt32 mask)
{
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTTCCLEAR_OFFSET, mask);
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_INTERRCLR_OFFSET, mask);
}

//******************************************************************************
//
// Function Name:  chal_dma_soft_burst_request
//
// Description:    Set a device to generate software DMA burst request
//
//******************************************************************************
void chal_dma_soft_burst_request(CHAL_HANDLE handle, cUInt32 device)
{
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    temp = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_SOFTBREQ_OFFSET);
    temp |= (0x1 << device);

    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_SOFTBREQ_OFFSET, temp);
}

//******************************************************************************
//
// Function Name:  chal_dma_soft_bst_last_request
//
// Description:    Set a device to generate software DMA burst request
//
//******************************************************************************
void chal_dma_soft_bst_last_request(CHAL_HANDLE handle, cUInt32 device)
{
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    temp = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_SOFTLBREQ_OFFSET);
    temp |= (0x1 << device);

    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_SOFTLBREQ_OFFSET, temp);
}

//******************************************************************************
//
// Function Name:  chal_dma_soft_single_request
//
// Description:    Set a device to generate software single DMA request
//
//******************************************************************************
void chal_dma_soft_single_request(CHAL_HANDLE handle, cUInt32 device)
{
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    temp = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_SOFTSREQ_OFFSET);
    temp |= (0x1 << device);

    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_SOFTSREQ_OFFSET, temp);
}

//******************************************************************************
//
// Function Name:  chal_dma_soft_sig_last_request
//
// Description:    Set a device to generate software single DMA request
//
//******************************************************************************
void chal_dma_soft_sig_last_request(CHAL_HANDLE handle, cUInt32 device)
{
    cUInt32 temp;
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    temp = CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_SOFTLSREQ_OFFSET);
    temp |= (0x1 << device);

    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_SOFTLSREQ_OFFSET, temp);
}

//******************************************************************************
//
// Function Name:  chal_dma_set_sync
//
// Description:    Enables or disables synchronization logic for the DMA 
//                 request signals. A bit set to 0 enables the synchronization 
//                 logic for a particular group of DMA requests. A bit set to 
//                 1 disables the synchronization logic for a particular group 
//                 of DMA requests. By default it is reset to 0, and synchronization
//                 logic enabled
//
//******************************************************************************
void chal_dma_set_sync(CHAL_HANDLE handle, cUInt16 mask)
{
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;
    
    //
    //  You must use synchronization logic when the peripheral generating
    //  the DMA request runs on a different clock to the DMAC. 
    //  For peripherals running on the same clock as the DMAC, disabling 
    //  the synchronization logic improves the DMA request response
    //  time. If necessary, synchronize the DMA response signals, DMACCLR and
    //  DMACTC, in the peripheral.
    //
    //
    
    CHAL_REG_WRITE32(pDmaDev->baseAddr + DMAC_SYNC_OFFSET, mask);
}

//******************************************************************************
//
// Function Name:  chal_dma_get_channel_status
//
// Description:    Return DMAC channel status
//
//******************************************************************************
cUInt32 chal_dma_get_channel_status(CHAL_HANDLE handle)
{
    ChalDmaDev_t *pDmaDev = (ChalDmaDev_t *)handle;

    return CHAL_REG_READ32(pDmaDev->baseAddr + DMAC_ENBLDCHNS_OFFSET);
}
