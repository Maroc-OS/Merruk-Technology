/*****************************************************************************
*  Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/**
*
*  @file   dma_drv.c
*
*  @brief  Linux DMA device driver
*
*  @note   
****************************************************************************/

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include <mach/hardware.h>
#include <plat/cpu.h>
#include <plat/types.h>

#include <plat/dma_drv.h>
#include <plat/csl/csl_dma.h>


static DEFINE_SPINLOCK(bcm_dma_lock);
/*
******************************************************************************
 Local Definitions
******************************************************************************
*/

typedef struct
{
	struct tasklet_struct	task;
	int			proc_id;
	int			irq;
	struct clk		*clk;
    
	DMA_Interrupt_t		intStatus;    

	void			*base;

	UInt32			csl_buf;
	UInt32			csl_buf_phy;
	UInt32			csl_buf_size;
} Dmadrv;
Dmadrv  *dmactl = NULL;

/*
 *******************************************************************************
	Function Definition
 *******************************************************************************
*/

/*******************************************************************************

  Funciton Name:	dma_lisr
  
  Description:	Low level DMA interrupt service routine

 *******************************************************************************
*/
static irqreturn_t dma_isr(int irq, void *dev_id)
{
	Dmadrv *dev	= dev_id;

    	/*pr_info("%s\n", __func__);*/
	csl_dma_get_intr_status(&(dev->intStatus));

	tasklet_schedule(&dev->task);
	return IRQ_HANDLED;
}

/*
******************************************************************************

  Funciton Name:	dma_thread_proc
  
  Description:	High level DMA interrupt service routine

******************************************************************************
*/
static void dma_thread_proc(unsigned long data)
{
	Dmadrv *dev	= (Dmadrv *)data;

        csl_dma_process_callback(&(dev->intStatus));  
}



/******************************************************************************

  Funciton Name:	DMADRV_Obtain_Channel
  
  Description:	Obtain free DMA channel

 ******************************************************************************
*/
DMADRV_STATUS DMADRV_Obtain_Channel(
	DMA_CLIENT srcID, 
	DMA_CLIENT dstID, 
	DMA_CHANNEL *chanID
)
{
	Int32  chan = -1;
	DMADRV_STATUS ret = DMADRV_STATUS_OK;
	unsigned long flags;

	spin_lock_irqsave(&bcm_dma_lock, flags);
	chan = csl_dma_obtain_channel(srcID, dstID);
	if(chan != -1) {
		pr_debug("chan is = %ld\n", chan);
		*chanID = (DMA_CHANNEL)chan;
		ret = DMADRV_STATUS_OK;
	} else {
		*chanID = DMA_CHANNEL_INVALID;
		ret = DMADRV_STATUS_FAIL;
	}
	spin_unlock_irqrestore(&bcm_dma_lock, flags);

	return ret;
}

/*
 ******************************************************************************

  Funciton Name:	DMADRV_Release_Channel
  
  Description:	Release the specified DMA channel 

 ******************************************************************************
*/
DMADRV_STATUS DMADRV_Release_Channel(DMA_CHANNEL chanID)
{
	DMADRV_STATUS ret = DMADRV_STATUS_OK;
	unsigned long flags;

	if (chanID >= TOTAL_DMA_CHANNELS)
		return DMADRV_STATUS_FAIL;

	spin_lock_irqsave(&bcm_dma_lock, flags);
	if(csl_dma_release_channel(chanID) == 0) {
		pr_info("DMA driver: DMADRV_Release_Channel fail, chanID: %d\n",  chanID);
		ret = DMADRV_STATUS_FAIL;
	}
	spin_unlock_irqrestore(&bcm_dma_lock, flags);

	return ret;
}

/*
 ******************************************************************************

  Funciton Name:	DMADRV_Config_Channel
  
  Description:	Configure DMA channel structure

 ******************************************************************************
*/
DMADRV_STATUS DMADRV_Config_Channel(
	DMA_CHANNEL chanID, 
	Dma_Chan_Info *pChanInfo
)
{   
	DMADRV_STATUS ret = DMADRV_STATUS_OK;
	unsigned long flags;
                   
	pr_debug("DMA driver: DMADRV_Config_Channel: %d\n", chanID);

	if (chanID >= TOTAL_DMA_CHANNELS)
		return DMADRV_STATUS_FAIL;

	spin_lock_irqsave(&bcm_dma_lock, flags);
	if(csl_dma_config_channel(chanID, pChanInfo) == -1) {
		ret = DMADRV_STATUS_FAIL;
	}
	spin_unlock_irqrestore(&bcm_dma_lock, flags);

	return ret;
}

/*
 ******************************************************************************

  Funciton Name:	DMADRV_Bind_Data
  
  Description:	Build up scatter gather link list for DMA transfer

 ******************************************************************************
*/
DMADRV_STATUS DMADRV_Bind_Data(DMA_CHANNEL chanID, Dma_Data *pData)
{
	Int32 ret = -1;
	unsigned long flags;
	
	pr_debug("DMA driver: DMADRV_Bind_Data: %d\n", chanID);

	if (chanID >= TOTAL_DMA_CHANNELS)
		return DMADRV_STATUS_FAIL;

	spin_lock_irqsave(&bcm_dma_lock, flags);
	ret = csl_dma_bind_data(chanID, pData, false, NULL);
	spin_unlock_irqrestore(&bcm_dma_lock, flags);

	if (ret) 
		return DMADRV_STATUS_OK;
	else
		return DMADRV_STATUS_FAIL;  
}

/*
 ******************************************************************************

  Funciton Name:	DMADRV_Start_Transfer
  
  Description:	Start the specific DMA channel transfer

 ******************************************************************************
*/
DMADRV_STATUS DMADRV_Start_Transfer(DMA_CHANNEL chanID)
{
	unsigned long flags;

	pr_debug("DMA driver: DMADRV_Start_Transfer: %d\n", chanID);
    
	if (chanID >= TOTAL_DMA_CHANNELS)
		return DMADRV_STATUS_FAIL;

	spin_lock_irqsave(&bcm_dma_lock, flags);
	csl_dma_start_transfer(chanID, NULL);
	spin_unlock_irqrestore(&bcm_dma_lock, flags);

	return DMADRV_STATUS_OK;
}

/*
 ******************************************************************************

  Funciton Name:	DMADRV_Bind_Data_Ex
  
  Description:	Build up scatter gather link list for DMA transfer

 ******************************************************************************
*/
DMADRV_STATUS DMADRV_Bind_Data_Ex(
	DMA_CHANNEL chanID,
	Dma_Data *pData,
	DMADRV_LLI_T *pLLI
)
{
	Int32 ret = -1;
	unsigned long flags;

	if (chanID >= TOTAL_DMA_CHANNELS)
		return DMADRV_STATUS_FAIL;

	spin_lock_irqsave(&bcm_dma_lock, flags);
	ret = csl_dma_bind_data(chanID, pData, true, pLLI);
	spin_unlock_irqrestore(&bcm_dma_lock, flags);

	if (ret)
		return DMADRV_STATUS_OK;
	else
		return DMADRV_STATUS_FAIL;  
}

/*
 ******************************************************************************

  Funciton Name:	DMADRV_Start_Transfer_Ex
  
  Description:	Start the specific DMA channel transfer

 ******************************************************************************
*/
DMADRV_STATUS DMADRV_Start_Transfer_Ex(
	DMA_CHANNEL chanID,
	DMADRV_LLI_T pLLI
)
{
	unsigned long flags;

	if (chanID >= TOTAL_DMA_CHANNELS)
		return DMADRV_STATUS_FAIL;

	spin_lock_irqsave(&bcm_dma_lock, flags);
	csl_dma_start_transfer(chanID, pLLI);
	spin_unlock_irqrestore(&bcm_dma_lock, flags);

	return DMADRV_STATUS_OK;
}

/*
 ******************************************************************************

  Funciton Name:	DMADRV_Stop_Transfer
  
  Description:	Stop the specific DMA channel transfer

 ******************************************************************************
*/
DMADRV_STATUS DMADRV_Stop_Transfer(DMA_CHANNEL chanID)
{
	unsigned long flags;

	if (chanID >= TOTAL_DMA_CHANNELS)
		return DMADRV_STATUS_FAIL;

	spin_lock_irqsave(&bcm_dma_lock, flags);
	csl_dma_stop_transfer(chanID);
	spin_unlock_irqrestore(&bcm_dma_lock, flags);

	return DMADRV_STATUS_OK;
}

/*
 ******************************************************************************

  Funciton Name:	DMADRV_Force_Shutdown_Channel
  
  Description:	Stop the specific DMA channel transfer and lost data

 ******************************************************************************
*/
DMADRV_STATUS DMADRV_Force_Shutdown_Channel(DMA_CHANNEL chanID)
{
	unsigned long flags;

	if (chanID >= TOTAL_DMA_CHANNELS)
		return DMADRV_STATUS_FAIL;

	spin_lock_irqsave(&bcm_dma_lock, flags);
	csl_dma_force_shutdown_channel(chanID);
	spin_unlock_irqrestore(&bcm_dma_lock, flags);

	return DMADRV_STATUS_OK;
}


/*
 ******************************************************************************

  Funciton Name:	brcm_init_dma
  
  Description:	Initialize the DMA driver and its software structure

 ******************************************************************************
*/
static int __init bcm21xx_dma_probe(struct platform_device *pdev)
{
	int rc;
	struct resource *res;

	pr_info("%s\n", __func__);

	dmactl = (Dmadrv *)kzalloc(sizeof(Dmadrv), GFP_KERNEL);
	if (dmactl == NULL) {
		dev_err(&pdev->dev, "memory allocation failed\n");
		return -ENOMEM;
	}
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (!res) {
		dev_err(&pdev->dev, "Can't get device resource\n");
		return -ENOENT;
	}
	dmactl->base	= res->start;
	dmactl->irq	= platform_get_irq(pdev, 0);
	if (dmactl->irq < 0) {
		dev_err(&pdev->dev, "Can't get irq number\n");
		return -ENOENT;
	}


	dmactl->csl_buf_size = 0x6400; /* allocate the same amount as RTOS */
	dmactl->csl_buf = dma_alloc_coherent( NULL, dmactl->csl_buf_size,
				(dma_addr_t *)&dmactl->csl_buf_phy, GFP_KERNEL );

	if ((void *)dmactl->csl_buf == NULL) {
		pr_info("DMA driver: failed to allocate DMA memory\n");
		kfree((void*)dmactl);
		return -ENOMEM;
	}

	tasklet_init(&dmactl->task, dma_thread_proc,(unsigned long)dmactl);

	csl_dma_init(dmactl->base, dmactl->csl_buf, dmactl->csl_buf_phy, dmactl->csl_buf_size);


#if defined (_HERA_)
	for(i = 0; i < DMA_INT_NUM; i ++) {
		IRQ_Register(dma_irq[i], dma_lisr_array[i]);
		IRQ_Clear(dma_irq[i]);
		IRQ_Enable(dma_irq[i]);
	}  
#else
	rc = request_irq(dmactl->irq, dma_isr, IRQF_DISABLED,
			 "bcm215xx-dma", dmactl);

	if (rc < 0) {
		pr_info("dma: %s failed to attach interrupt, rc = %d\n",
		       __FUNCTION__, rc);
		goto fail_irq;
	}
#endif
	return 0;

fail_irq:
	csl_dma_deinit();
	dma_free_coherent(NULL, dmactl->csl_buf_size,
			(void *)&dmactl->csl_buf, dmactl->csl_buf_phy);
	free_irq(dmactl->irq, 0);
	kfree((void*)dmactl);

	return rc;
}

static int bcm21xx_dma_remove(struct platform_device *pdev)
{
	pr_info("%s\n", __func__);
	csl_dma_deinit();
	dma_free_coherent(NULL, dmactl->csl_buf_size,
				 (void *)&dmactl->csl_buf, dmactl->csl_buf_phy);
	free_irq(dmactl->irq, 0);
	kfree((void*)dmactl);
	return 0;
}

static struct platform_driver bcm21xx_dma_driver = {
	.probe = bcm21xx_dma_probe,
	.remove = bcm21xx_dma_remove,
	.driver = {
		.name = "bcm21xx-dma",
		.owner = THIS_MODULE,
		},
};

static __init int brcm_init_dma(void)
{
	pr_info("%s\n", __func__);
	return platform_driver_register(&bcm21xx_dma_driver);
}

static __exit void brcm_exit_dma(void)
{
	pr_info("%s\n", __func__);
	platform_driver_unregister(&bcm21xx_dma_driver);
}
arch_initcall(brcm_init_dma);

EXPORT_SYMBOL(DMADRV_Obtain_Channel);
EXPORT_SYMBOL(DMADRV_Release_Channel);
EXPORT_SYMBOL(DMADRV_Config_Channel);
EXPORT_SYMBOL(DMADRV_Bind_Data);
EXPORT_SYMBOL(DMADRV_Start_Transfer);
EXPORT_SYMBOL(DMADRV_Bind_Data_Ex);
EXPORT_SYMBOL(DMADRV_Start_Transfer_Ex);
EXPORT_SYMBOL(DMADRV_Stop_Transfer);
EXPORT_SYMBOL(DMADRV_Force_Shutdown_Channel);
