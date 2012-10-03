/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/video/.../memalloc_wrapper/memalloc_wrapper.c
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

#include <linux/kernel.h>
#include <linux/module.h>
/* needed for __init,__exit directives */
#include <linux/init.h>
/* needed for remap_page_range */
#include <linux/mm.h>
/* obviously, for kmalloc */
#include <linux/slab.h>
/* for struct file_operations, register_chrdev() */
#include <linux/fs.h>
/* standard error codes */
#include <linux/errno.h>
/* this header files wraps some common module-space operations ...
   here we use mem_map_reserve() macro */
#include <linux/dma-mapping.h>
#include <linux/ioport.h>
#include <linux/list.h>
/* for current pid */
#include <linux/sched.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/bcm_memalloc_wrapper.h>
#include <linux/broadcom/bcm_memalloc_ioctl.h>

#include <linux/android_pmem.h>

#include <plat/osdal_os_driver.h>
#include <plat/osdal_os_service.h>
#include <plat/dma_drv.h>


/* module description */
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Braodcom");

#define DRV_NAME "memalloc"
#define DEV_NAME "memalloc"

#define MEMALLOC_BASIC 0
#define MEMALLOC_MAX_OUTPUT 1

static dma_addr_t dma_cohr_start_addr;
static void *alloc_mem;
extern void *memalloc_mempool_base;
//#define MEMALLOC_PMEM_START_ADDRESS (dma_cohr_start_addr + (1024*1024*6))
#define MEMALLOC_SIZE  (1024*1024*6)
#define DMA_NOT_DONE         0
#define DMA_DONE_SUCCESS     1
#define DMA_DONE_FAILURE     2
static DMA_CHANNEL gDmaChannel;
static OSDAL_Dma_Chan_Info dmaChInfoMem;
static OSDAL_Dma_Buffer_List *dmaBuffListMem=NULL;
static OSDAL_Dma_Data dmaDataMem;
static unsigned int dmaDone = DMA_NOT_DONE;


static dma_addr_t dma_cohr_start_addr;
static void *alloc_mem;

static MemallocwrapParams* hw_OutBuf = NULL;

#ifndef BCM_MEMALLOC_WRAPPER_MAJOR
#define BCM_MEMALLOC_WRAPPER_MAJOR	0	/* dynamic */
#endif

static int memalloc_major = BCM_MEMALLOC_MAJOR;

struct list_head heap_list;

DEFINE_SPINLOCK(mem_lock);

struct memalloc_logic logic;
static struct semaphore irq_sem;
static struct completion dma_complete;

static void dma_transfer_isr(DMADRV_CALLBACK_STATUS_t status)
{
	if(status == DMADRV_CALLBACK_OK)
	{
		dmaDone = DMA_DONE_SUCCESS;
	}
	else
	{
		dmaDone = DMA_DONE_FAILURE;
	}
	complete(&dma_complete);
}

static void dma_cleanup()
{
	//Free the allocated dmaBuffListMem.
	if(dmaBuffListMem != NULL)
		kfree(dmaBuffListMem);
}

static int removeFromList(MemallocwrapParams* entry)
{
	MemallocwrapParams	*temp,*temp1;
	MemallocwrapParams* tempEntry;
	int i;
	int hastoContinue = 0;
	temp = hw_OutBuf;
	if(entry == NULL)
	{
		return 0;
	}
	else
	{
		if(temp == NULL)
		{
			return 0;
		}
		else if (temp->virtualaddress == entry->virtualaddress)
		{
			temp1 = temp->nextAddress;
			kfree(temp);
			temp = NULL;
			hw_OutBuf = temp1;
		}
		else if(temp->nextAddress == NULL)
		{
			if (temp->virtualaddress == entry->virtualaddress)
			{
				kfree(temp);
				temp = NULL;
				hw_OutBuf = NULL;
			}
		}
		else
		{
			while(temp->nextAddress != NULL)
			{
				temp1 = temp;
				temp = temp->nextAddress;
				if(temp->virtualaddress == entry->virtualaddress)
				{
					temp1->nextAddress = temp->nextAddress;
					kfree(temp);
					temp=NULL;
					break;

				}
			}
		}
	}
	return 1;
}

static int addToList(MemallocwrapParams* entry)
{
	MemallocwrapParams* temp;
	if(entry == NULL)
	{
		return 0;
	}
	temp = (MemallocwrapParams*)kmalloc(sizeof(MemallocwrapParams),GFP_KERNEL|GFP_DMA);
	if(temp == NULL)
	{
		return 0;
	}
	temp->busAddress = entry->busAddress;
	temp->virtualaddress = entry->virtualaddress;
	temp->size = entry->size;
	temp->nextAddress = NULL;
	if(hw_OutBuf == NULL)
	{
		hw_OutBuf = temp;
	}
	else
	{
		MemallocwrapParams* tempLoc = hw_OutBuf;
		while(tempLoc->nextAddress != NULL)
		{
			tempLoc = tempLoc->nextAddress;
		}
		tempLoc->nextAddress = temp;
	}
	return 1;
}

static unsigned long getItemFromList(unsigned long virtualaddress)
{
	MemallocwrapParams* temp;
	unsigned long busAddress;
	temp = hw_OutBuf;
	if(temp == NULL)
	{
		return 0;
	}
	while(temp->virtualaddress != virtualaddress)
	{
		temp = temp->nextAddress;
		if (temp == NULL)
		{
			return 0;
		}
	}
	return temp->busAddress;
}


static int memalloc_wrapper_ioctl(struct inode *inode, struct file *filp,
				  unsigned int cmd, unsigned long arg)
{
	int result = -1;

	pr_debug(KERN_DEBUG "memalloc_wrapper: ioctl cmd 0x%08x\n", cmd);

	if (inode == NULL || filp == NULL || arg == 0)
		return -EFAULT;
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if ((_IOC_TYPE(cmd) != HANTRO_WRAP_MAGIC) && (_IOC_TYPE(cmd) != PMEM_IOCTL_MAGIC) )
		return -ENOTTY;
	if (_IOC_NR(cmd) > HANTRO_WRAP_MAXNR)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		result = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		result = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
	if (result)
		return -EFAULT;

	switch (cmd) {
	case PMEM_GET_PHYS:
		{
			struct pmem_region memparams;

			pr_debug(KERN_DEBUG "memalloc_wrapper: ALLOC BUFFER\n");
			spin_lock(&mem_lock);
			memparams.offset = (dma_cohr_start_addr + MEMALLOC_SIZE);//MEMALLOC_PMEM_START_ADDRESS;
			memparams.len = SZ_2M;
			__copy_to_user((void *)arg, &memparams,
					   sizeof(memparams));
			spin_unlock(&mem_lock);
		}
		break;
	case PMEM_MAP:
	case PMEM_GET_SIZE:
	case PMEM_UNMAP:
	case PMEM_ALLOCATE:
	case PMEM_CONNECT:
	case PMEM_GET_TOTAL_SIZE:
		{
		}
		break;

	case HANTRO_WRAP_ACQUIRE_BUFFER:
		{
			MemallocwrapParams memparams;
			int ret = 0;
			pr_debug(KERN_DEBUG "memalloc_wrapper: ALLOC BUFFER\n");
			spin_lock(&mem_lock);

			ret = copy_from_user(&memparams, (const void *)arg,
					 sizeof(memparams));
			if(ret != 0) {
				printk("Error in copying user arguments in HANTRO_WRAP_ACQUIRE_BUFFER");
				return ret;
			}

			result =
			    logic.AllocMemory(filp->private_data,
					      &memparams.busAddress,
					      memparams.size);
			memparams.nextAddress = NULL;

			__copy_to_user((void *)arg, &memparams,
				       sizeof(memparams));

			spin_unlock(&mem_lock);
		}
		break;
	/*case HANTRO_WRAP_RELEASE_BUFFER:
		{

			unsigned long busaddr;
			pr_debug(KERN_DEBUG
				 "memalloc_wrapper: RELEASE BUFFER\n");
			spin_lock(&mem_lock);
			__get_user(busaddr, (unsigned long *)arg);
			result = logic.FreeMemory(filp->private_data, &busaddr);

			spin_unlock(&mem_lock);
		}
		break;
		*/
	case HANTRO_WRAP_RELEASE_BUFFER:
	{
			unsigned long busaddr;
			int ret = 0;
			MemallocwrapParams temp_hw_OutBuf;
			pr_debug(KERN_DEBUG	"memalloc_wrapper: RELEASE BUFFER\n");
			spin_lock(&mem_lock);
			ret = copy_from_user(&temp_hw_OutBuf, (const void *)arg,sizeof(MemallocwrapParams));
			if(ret != 0) {
				printk("Error in copying user arguments in HANTRO_WRAP_RELEASE_BUFFER");
				return ret;
			}
			result = logic.FreeMemory(filp->private_data, &temp_hw_OutBuf.busAddress);
			/*--Remove the Instance from the List---*/
			removeFromList(&temp_hw_OutBuf);
			/*---Remove this instance from the List Done--*/
			spin_unlock(&mem_lock);
		}
		break;

	case HANTRO_DMA_COPY:
		{
			DmaStruct dma_buffers;
			int tries = 0;
			int ret = 0;
			ret = copy_from_user(&dma_buffers, (const void *)arg,
					 sizeof(dma_buffers));
			if(ret != 0) {
				printk("Error in copying user arguments in HANTRO_DMA_COPY");
				return ret;
			}

			/*Request the channel */
			while(OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT_MEMORY, OSDAL_DMA_CLIENT_MEMORY,(UInt32 *)&gDmaChannel) != OSDAL_ERR_OK) {
				msleep(20);
				tries++;
			}
			if (tries)
				pr_debug("memwrapper: channel 11 obtain tried for %d times\n", tries);
/*
			if (OSDAL_ERR_OK != OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT_MEMORY, OSDAL_DMA_CLIENT_MEMORY,(UInt32 *)&gDmaChannel))
			{
				pr_info("---->OSDAL_DMA_Obtain_Channel failed for channel %d \n",gDmaChannel);
				dma_cleanup();
				return -1;
			}
*/

			//Allocate memory for array of pointers
			dmaBuffListMem = (OSDAL_Dma_Buffer_List *)kmalloc((sizeof(OSDAL_Dma_Buffer_List)),GFP_KERNEL);
			if(!dmaBuffListMem)
			{
				pr_info("DMA-Failed to allocate memory for DMA BuffList\n");
				dma_cleanup();
				return -ENOMEM;
			}

			/* Configuring the DMA channel info */
			dmaChInfoMem.type = OSDAL_DMA_FCTRL_MEM_TO_MEM;
			dmaChInfoMem.srcBstSize = OSDAL_DMA_BURST_SIZE_32;
			dmaChInfoMem.dstBstSize = OSDAL_DMA_BURST_SIZE_32;
			dmaChInfoMem.srcDataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
			dmaChInfoMem.dstDataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
			dmaChInfoMem.dstMaster = 1;
			dmaChInfoMem.srcMaster = 1;
			dmaChInfoMem.incMode = OSDAL_DMA_INC_MODE_BOTH;
			dmaChInfoMem.freeChan = TRUE;
			dmaChInfoMem.priority = 2;
			dmaChInfoMem.bCircular = FALSE;
			dmaChInfoMem.alignment = OSDAL_DMA_ALIGNMENT_32;
			dmaChInfoMem.xferCompleteCb = (OSDAL_DMA_CALLBACK)dma_transfer_isr;

			if (OSDAL_DMA_Config_Channel(gDmaChannel, &dmaChInfoMem)!= OSDAL_ERR_OK)
			{
				pr_info("---->DMADRV_Config_Channel - Failed for channel %d \n",gDmaChannel);
				dma_cleanup();
				return -1;
			}
			/* end of DMA channel configuration. */

			//Bind the channel
			dmaBuffListMem->buffers[0].srcAddr = dma_buffers.src_busAddr;
			dmaBuffListMem->buffers[0].destAddr = dma_buffers.dst_busAddr;
			dmaBuffListMem->buffers[0].length = dma_buffers.size;
			dmaBuffListMem->buffers[0].bRepeat = 0;
			dmaBuffListMem->buffers[0].interrupt = 1;
			dmaDataMem.numBuffer = 1;
			dmaDataMem.pBufList = (OSDAL_Dma_Buffer_List *) dmaBuffListMem;

			if (OSDAL_DMA_Bind_Data(gDmaChannel, &dmaDataMem) != OSDAL_ERR_OK)
			{
				pr_info("DMADRV_Bind_Data - Failed\n");
				dma_cleanup();
				return -EINVAL;
			}

			// Start transfer
			dmaDone = DMA_NOT_DONE;
			if (OSDAL_DMA_Start_Transfer(gDmaChannel) != OSDAL_ERR_OK)
			{
				pr_info("DMADRV_Start_Transfer - Failed for channel %d\n",gDmaChannel);
				dma_cleanup();
				return -EINVAL;
			}

			if((wait_for_completion_interruptible_timeout(&dma_complete, msecs_to_jiffies(100))) == 0)
			{
				pr_info("Transfer timed out for channel %d\n",gDmaChannel);
				OSDAL_DMA_Force_Shutdown_Channel(gDmaChannel);
			}

			if(dmaDone == DMA_DONE_FAILURE)
			{
				printk("Error, DMA transfer failed.\n");
				dma_cleanup();
				return -1;
			}
			//Free all the allocated memories.
			dma_cleanup();
		}
		break;
	case HANTRO_STORE_OUTBUFFER_0:
	{
		int ret = 0;
		ret = copy_from_user(&hw_OutBuf[0], (const void *)arg,
					 sizeof(MemallocwrapParams));
		if(ret != 0) {
			printk("Error in copying user arguments in HANTRO_STORE_OUTBUFFER_0");
			return ret;
		}
		break;
	}
	case HANTRO_GET_OUTBUFFER_0:
	{
		int ret = 0;
		ret = copy_to_user((void *)arg, &hw_OutBuf[0],
				       sizeof(MemallocwrapParams));
		if(ret != 0) {
			printk("Error in copying user arguments in HANTRO_GET_OUTBUFFER_0");
			return ret;
		}
		break;
	}
	case HANTRO_STORE_OUTBUFFER_1:
	{
		int ret = 0;
		ret = copy_from_user(&hw_OutBuf[1], (const void *)arg,
					 sizeof(MemallocwrapParams));
		if(ret != 0) {
			printk("Error in copying user arguments in HANTRO_STORE_OUTBUFFER_1");
			return ret;
		}
		break;
	}
	case HANTRO_GET_OUTBUFFER_1:
	{
		int ret = 0;
		ret = copy_to_user((void *)arg, &hw_OutBuf[1],
				       sizeof(MemallocwrapParams));
		if(ret != 0) {
			printk("Error in copying user arguments in HANTRO_GET_OUTBUFFER_1");
			return ret;
		}
		break;
	}
	case HANTRO_STORE_OUTBUFFER:
	{
		int ret = 0;
		pr_debug(KERN_DEBUG "memalloc_wrapper: HANTRO_STORE_OUTBUFFER\n");
		spin_lock(&mem_lock);
		MemallocwrapParams temp_hw_OutBuf;
		ret = copy_from_user(&temp_hw_OutBuf, (const void *)arg,sizeof(MemallocwrapParams));
		if(ret != 0) {
			printk("Error in copying user arguments in HANTRO_STORE_OUTBUFFER");
			return ret;
		}
		addToList(&temp_hw_OutBuf);
		spin_unlock(&mem_lock);
	}
	break;
	case HANTRO_GET_OUTBUFFER:
	{
		unsigned long busaddr;
		int ret = 0;
		MemallocwrapParams temp_hw_OutBuf;
		pr_debug(KERN_DEBUG "memalloc_wrapper: HANTRO_GET_OUTBUFFER\n");
		spin_lock(&mem_lock);
		ret = copy_from_user(&temp_hw_OutBuf, (const void *)arg,sizeof(MemallocwrapParams));
		if(ret != 0) {
			printk("Error in copying user arguments in HANTRO_GET_OUTBUFFER");
			return ret;
		}
		temp_hw_OutBuf.busAddress = getItemFromList(temp_hw_OutBuf.virtualaddress);
		ret = copy_to_user(((MemallocwrapParams*)arg), &temp_hw_OutBuf,sizeof(MemallocwrapParams));
		if(ret != 0) {
			printk("HANTRO_GET_OUTBUFFER: Error in copying to user");
			spin_unlock(&mem_lock);
			return ret;
		}
		spin_unlock(&mem_lock);
	}
	break;
}

	return result;
}

static int memalloc_wrapper_open(struct inode *inode, struct file *filp)
{
	int r;

	spin_lock(&mem_lock);
	r = logic.open(&filp->private_data);
	spin_unlock(&mem_lock);

	pr_debug(KERN_DEBUG "memalloc_wrapper_open\n");

	return r;
}

static int memalloc_wrapper_release(struct inode *inode, struct file *filp)
{

	int r;

	spin_lock(&mem_lock);
	r = logic.release(filp->private_data);
	filp->private_data = NULL;
	spin_unlock(&mem_lock);

	pr_debug(KERN_DEBUG "memalloc_wrapper_release\n");

	return r;
}

#define pgprot_cached(prot) \
__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITEBACK)

static int memalloc_wrapper_mmap(struct file *file, struct vm_area_struct *vma)
{
	int r =0;
	unsigned long size = vma->vm_end - vma->vm_start;
	if(vma->vm_pgoff == 0)
	{
		vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);
		/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
		if (remap_pfn_range(vma,
					vma->vm_start,
					(dma_cohr_start_addr + MEMALLOC_SIZE) >> PAGE_SHIFT,
					vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
			pr_err("%s(): remap_pfn_range() failed\n", __FUNCTION__);
			return -1;
		}
		return r;
	}

	r = logic.mmap(size, vma->vm_pgoff);

	vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);
	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		pr_err("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -1;
	}

	return r;
}

/* VFS methods */
static const struct file_operations memalloc_wrapper_fops = {
	.open = memalloc_wrapper_open,
	.release = memalloc_wrapper_release,
	.ioctl = memalloc_wrapper_ioctl,
	.mmap = memalloc_wrapper_mmap,
};

int register_memalloc_wrapper(struct memalloc_logic *memalloc_fops)
{
	pr_debug(KERN_DEBUG "register_memalloc_wrapper\n");
	memcpy(&logic, memalloc_fops, sizeof(logic));

	return logic.init(MEMALLOC_SIZE, dma_cohr_start_addr);
}

EXPORT_SYMBOL(register_memalloc_wrapper);

void deregister_memalloc_wrapper(void)
{
	if (logic.cleanup)
		logic.cleanup();
}

EXPORT_SYMBOL(deregister_memalloc_wrapper);

int __init memalloc_wrapper_init(void)
{
	int result;

	pr_debug(KERN_DEBUG "memalloc_init\n");

	init_completion(&dma_complete);

	result =
	    register_chrdev(memalloc_major, "memalloc", &memalloc_wrapper_fops);
	if (result < 0) {
		pr_debug(KERN_ERR "memalloc_wrapper: unable to get major %d\n",
			 memalloc_major);
		return result;
	} else if (result != 0) {	/* this is for dynamic major */
		memalloc_major = result;
	}
#if 0
	alloc_mem = dma_alloc_coherent(NULL, MEMALLOC_SIZE,
				       &dma_cohr_start_addr,
				       GFP_ATOMIC | GFP_DMA);

	if (alloc_mem == NULL) {
		result = -ENOMEM;
		pr_err("memalloc_wrapper: DMA Allocation failed\n");
		goto err;
	}
#else
	alloc_mem = memalloc_mempool_base;
	dma_cohr_start_addr = (dma_addr_t) virt_to_phys(alloc_mem);
	// assigning the start address for allocation.
	pr_info("Assigned the start address 0x%x to HANTRO ******\n", dma_cohr_start_addr);
#endif
	pr_info("memalloc_wrapper: module inserted. Major = %d\n",
		memalloc_major);

	return 0;

}

void __exit memalloc_wrapper_cleanup(void)
{

	pr_debug(KERN_DEBUG "memalloc_wrapper_cleanup\n");

	if (alloc_mem != NULL)
		dma_free_coherent(NULL, MEMALLOC_SIZE, alloc_mem,
				  dma_cohr_start_addr);
	unregister_chrdev(memalloc_major, "memalloc");

	pr_notice("memalloc_wrapper: module removed\n");
	return;
}

module_init(memalloc_wrapper_init);
module_exit(memalloc_wrapper_cleanup);
