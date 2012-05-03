/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/video/.../bmem_wrapper/bmem_wrapper.c
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
#include <linux/broadcom/bmem_wrapper.h>
#include <linux/broadcom/bcm_gememalloc_ioctl.h>

#include <linux/broadcom/bcm_memalloc_wrapper.h>
#include <linux/broadcom/bcm_memalloc_ioctl.h>

#include <linux/android_pmem.h>
#include <linux/file.h>
#include <linux/debugfs.h>

#include <plat/osdal_os_driver.h>
#include <plat/osdal_os_service.h>
#include <plat/dma_drv.h>

#include <linux/proc_fs.h>

/* module description */
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Braodcom");

#define DRV_NAME "bmem"
#define DEV_NAME "bmem"

#define pgprot_cached(prot) \
__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITEBACK)

static int bmem_major = BCM_GEMEMALLOC_MAJOR;
extern void *bmem_mempool_base;
static dma_addr_t dma_cohr_start_addr;
static struct semaphore bmem_sem;

static struct bmem_logic logic;
static bmem_status_t bmem_status;
static struct proc_dir_entry *bmem_proc_file;
static struct semaphore bmem_status_sem;

#define BMEM_PROC_PRINT_D(str, val) \
	if (bmem_proc_print_info(str, val, 0, &curr, &len, count)) goto err;
#define BMEM_PROC_PRINT_X(str, val) \
		if (bmem_proc_print_info(str, val, 1, &curr, &len, count)) goto err;
#define BMEM_PROC_PRINT_HDR(str) \
			if (bmem_proc_print_info(str, 0, 2, &curr, &len, count)) goto err;


static MemallocwrapParams* hw_OutBuf = NULL;
static struct semaphore bmem_virt_list_sem;

#define DMA_NOT_DONE         0
#define DMA_DONE_SUCCESS     1
#define DMA_DONE_FAILURE     2
static DMA_CHANNEL gDmaChannel;
static OSDAL_Dma_Chan_Info dmaChInfoMem;
static OSDAL_Dma_Buffer_List *dmaBuffListMem=NULL;
static OSDAL_Dma_Data dmaDataMem;
static unsigned int dmaDone = DMA_NOT_DONE;
struct completion dma_complete;

typedef struct {
	unsigned long busAddress;
	unsigned int size;
	char allocated;
	struct file *filp;
	/* Following needed only for pmem users calling CONNECT ioctl */
	char connected;
	int master_fd;
	struct file *master_filp;
} bmem_pmem_data_t;

typedef struct {
	BMEM_HDL bmem_handle;
	bmem_pmem_data_t bmem_pmem_data;
} bmem_wrapper_data_t;


#define MAX_STR_SIZE (50)
#define KLOG_TAG	"bmem_wrapper.c"

#ifndef KLOG_TAG
#define KLOG_TAG __FILE__
#endif

/* Error Logs */
#if 1
#define KLOG_E(fmt,args...) \
					do { printk(KERN_ERR "Error: [%s:%s:%d] "fmt"\n", KLOG_TAG, __func__, __LINE__, \
		    ##args); } \
					while (0)
#else
#define KLOG_E(x...) do {} while (0)
#endif

/* Debug Logs */
#if 1
#define KLOG_D(fmt,args...) \
		do { printk(KERN_INFO KLOG_TAG "[%s:%d] "fmt"\n", __func__, __LINE__, \
		    ##args); } \
		while (0)
#else
#define KLOG_D(x...) do {} while (0)
#endif

/* Verbose Logs */
#if 0
#define KLOG_V(fmt,args...) \
					do { printk(KERN_INFO KLOG_TAG "[%s:%d] "fmt"\n", __func__, __LINE__, \
		    ##args); } \
					while (0)
#else
#define KLOG_V(x...) do {} while (0)
#endif

/*
 * dma_transfer_isr()
 * Description : ISR callback for DMA completion
 */
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

/*
 * dma_cleanup()
 * Description : Cleanup Structures allocated for DMA
 */
static void dma_cleanup(void)
{
	//Free the allocated dmaBuffListMem.
	if(dmaBuffListMem != NULL)
		kfree(dmaBuffListMem);

	// Release the DMA channel.
	//OSDAL_DMA_Release_Channel(gDmaChannel);
}


/*
 * bmem_pmem_connect()
 * Description : Used by pmem user. subregion file connects to a master file
 *		to share buffer allocated by the master
 */
static int bmem_pmem_connect(unsigned long master_fd, struct file *file)
{
	bmem_wrapper_data_t *p_data = file->private_data;
	bmem_wrapper_data_t *p_master_data;
	struct file *master_file;
	int ret = 0, put_needed;

	/* retrieve the src file and check it is a pmem file with an alloc */
	master_file = fget_light(master_fd, &put_needed);
	KLOG_V("master_fd[%d] master_file[%p] subfile[%p], put_needed[%d]\n", (int)master_fd, master_file, file, put_needed);
	if (!master_file) {
		p_data->bmem_pmem_data.connected = -1;
		KLOG_E("pmem_connect: master file not found!\n");
		ret = -EINVAL;
		goto err_no_file;
	}
	p_master_data = master_file->private_data;
	if (p_master_data->bmem_pmem_data.allocated == 0) {
		p_data->bmem_pmem_data.connected = -1;
		KLOG_E("pmem_connect: master file has no alloc!\n");
		ret = -EINVAL;
		goto err_bad_file;
	}
	p_data->bmem_pmem_data.busAddress = p_master_data->bmem_pmem_data.busAddress;
	p_data->bmem_pmem_data.size = p_master_data->bmem_pmem_data.size;
	p_data->bmem_pmem_data.master_fd = master_fd;
	p_data->bmem_pmem_data.master_filp = master_file;
	p_data->bmem_pmem_data.allocated = p_master_data->bmem_pmem_data.allocated;
	p_data->bmem_pmem_data.connected = 1;

err_bad_file:
	fput_light(master_file, put_needed);
err_no_file:
	return ret;
}

/*
 * remove_from_virt_list()
 * Description : Used for removing entry from driver database used to maintain mapping
 *		of physical to user virtual mapping
 */
static int remove_from_virt_list(MemallocwrapParams* entry)
{
	MemallocwrapParams* curr = hw_OutBuf;
	MemallocwrapParams	*prev, *next;

	KLOG_V("Entry");
	if(entry == NULL) {
		KLOG_E("NULL entry passed");
		return 0;
	}

	if(hw_OutBuf == NULL) {
		KLOG_V("NULL hw_OutBuf");
		return 0;
	}
	else if (hw_OutBuf->virtualaddress == entry->virtualaddress) {
		next = hw_OutBuf->nextAddress;
		kfree(hw_OutBuf);
		hw_OutBuf = next;
		KLOG_V("Exit");
		return 1;
	}
	else {
		prev = hw_OutBuf;
		curr = hw_OutBuf->nextAddress;
		while(curr != NULL) {
			if(curr->virtualaddress == entry->virtualaddress) {
				prev->nextAddress = curr->nextAddress;
				kfree(curr);
				KLOG_V("Exit");
				return 1;
			}
			prev = curr;
			curr = curr->nextAddress;
		}
	}
	return 0;
}

/*
 * add_to_virt_list()
 * Description : Used for adding entry to driver database used to maintain mapping
 *		of physical to user virtual mapping
 */
static int add_to_virt_list(MemallocwrapParams* entry)
{
	MemallocwrapParams* curr = hw_OutBuf;
	MemallocwrapParams* new_element;

	KLOG_V("Entry");
	if(entry == NULL) {
		KLOG_E("NULL entry passed");
		return 0;
	}

	new_element = (MemallocwrapParams*)kmalloc(sizeof(MemallocwrapParams),GFP_KERNEL|GFP_DMA);
	if(new_element == NULL) {
		KLOG_E("No memory - kmalloc");
		return 0;
	}
	new_element->busAddress = entry->busAddress;
	new_element->virtualaddress = entry->virtualaddress;
	new_element->size = entry->size;
	new_element->nextAddress = NULL;

	if(hw_OutBuf == NULL) {
		hw_OutBuf = new_element;
	}
	else {
		while(curr->nextAddress != NULL)	{
			curr = curr->nextAddress;
		}
		curr->nextAddress = new_element;
	}
	KLOG_V("Exit");
	return 1;
}

/*
 * get_from_virt_list()
 * Description : Used for querying physical address corresponding to user virtual address
 *		from driver database used to maintain mapping of physical to user virtual mapping
 */
static unsigned long get_from_virt_list(unsigned long virtualaddress)
{
	MemallocwrapParams* curr = hw_OutBuf;

	KLOG_V("Entry");
	while(curr != NULL)	{
		if (curr->virtualaddress == virtualaddress)	{
			KLOG_V("Exit");
			return curr->busAddress;
		}
		curr = curr->nextAddress;
	}

	KLOG_E("Item not found");
	return 0;
}

/*
 * is_mmap_for_pmem_interface()
 * Description : Used for checking whether mmap was called by pmem user or not.
 *		mmap of pmem user (master) expects to allocate memory if not yet allocated
 *		for the file.
 */
static int is_mmap_for_pmem_interface(struct file *file, struct vm_area_struct *vma)
{
	return (0 == vma->vm_pgoff);
}

/*
 * is_pmem_alloc_needed()
 * Description : Used for checking whether pmem mmap needs to allocate.
 *		If owner file or master file has already allocated, we need to allocate again.
 *		Mapping of the already allocated buffer is what user expects.
 */
static int is_pmem_alloc_needed(struct file *file, struct vm_area_struct *vma)
{
	bmem_wrapper_data_t *p_data = file->private_data;
	return (0 == p_data->bmem_pmem_data.allocated);
}

/*
 * pmem_alloc_done()
 * Description : Used for updating file context to mark that buffer is already
 *		allocated
 */
static void pmem_alloc_done(struct file *file, struct vm_area_struct *vma, unsigned long busAddress)
{
	bmem_wrapper_data_t *p_data = file->private_data;

	p_data->bmem_pmem_data.busAddress = busAddress;
	p_data->bmem_pmem_data.size = vma->vm_end - vma->vm_start;
	p_data->bmem_pmem_data.filp = file;
	p_data->bmem_pmem_data.allocated = 1;
}

/*
 * bmem_print_status()
 * Description : Support function used to print the bmem heap status from kernel.
 */
static void bmem_print_status(void)
{
	int result;

	if (logic.GetStatus == NULL) {
		KLOG_E("GetStatus() is NULL");
		goto err;
	}

	down(&bmem_sem);
	result = logic.GetStatus(&bmem_status);
	up(&bmem_sem);
	if (result) {
		KLOG_E("GetStatus failed form the proc call");
		goto err;
	}

	down(&bmem_status_sem);
	KLOG_E("\n  %-30s: ", "Current Usage Info");
	KLOG_E("\t%-30s: %d ", "Used space in bytes", bmem_status.total_used_space);
	KLOG_E("\t%-30s: %d ", "Free space in bytes", bmem_status.total_free_space);
	KLOG_E("\t%-30s: %d ", "Num Buffers in Use", bmem_status.num_buf_used);

	KLOG_E("  %-30s: ", "Statistics");
	KLOG_E("\t%-30s: %d ", "Maximum Memory usage", bmem_status.max_used_space);
	KLOG_E("\t%-30s: %d ", "Biggest Buffer Requested", bmem_status.biggest_buf_request);
	KLOG_E("\t%-30s: %d ", "Smallest Buffer Requested", bmem_status.smallest_buf_request);
	KLOG_E("\t%-30s: %d ", "Allocate Success Count", bmem_status.alloc_pass_cnt);
	KLOG_E("\t%-30s: %d ", "Mem Free Success Count", bmem_status.free_pass_cnt);

	KLOG_E("  %-30s: ", "Fragmentation Info");
	KLOG_E("\t%-30s: %d ", "Num Buffers Free", bmem_status.num_buf_free);
	KLOG_E("\t%-30s: %d ", "Biggest Buffer Available", bmem_status.biggest_chunk_avlbl);
	KLOG_E("\t%-30s: %d ", "Smallest Buffer Available", bmem_status.smallest_chunk_avlbl);
	KLOG_E("\t%-30s: %d ", "Max Num Holes occured", bmem_status.max_num_buf_free);
	KLOG_E("\t%-30s: %d ", "Max Fragmented", bmem_status.max_fragmented_size);

	KLOG_E("  %-30s: ", "Error Info");
	KLOG_E("\t%-30s: %d ", "Allocate Failures", bmem_status.alloc_fail_cnt);
	KLOG_E("\t%-30s: %d \n", "Mem Free Failures", bmem_status.free_fail_cnt);

	up(&bmem_status_sem);

err:
	return;
}

/*
 * bmem_print_status()
 * Description : Support function used by proc interface to print info on
 *		the proc memory (proc read).
 *		0 - print value in integer format
 *		1 - print value in hex format
 *		2 - print header
 */
static int bmem_proc_print_info(char *str, int value, int print_type,
	char **curr, int *len, int max_cnt)
{
	int str_len = 0;

	if (print_type == 0) {
		str_len = sprintf(*curr,"\t%-30s: %d \n", str, value);
	}
	else if (print_type == 1) {
		str_len = sprintf(*curr,"\t%-30s: 0x%08x \n", str, value);
	}
	else if (print_type == 2) {
		str_len = sprintf(*curr,"  %-30s: \n", str);
	}
	*curr += str_len;
	*len += str_len;
	if ((max_cnt - *len) < 100) {
		KLOG_E("proc size[%d] not sufficient. Only [%d] bytes written", max_cnt, *len);
		return 1;
	}
	return 0;
}

/*
 * bmem_proc_get_status()
 * Description : proc read callback to print the bmem heap status
 */
static int bmem_proc_get_status(char *page, char **start,
		   off_t off, int count, int *eof, void *data)
{
	char *curr = page;
	int len = 0;
	int result;

	KLOG_V("proc read has come: page[%p], off[%d], count[%d], data[%p] \n",
		page, (int)off, count, data);
	if (off != 0) {
		goto err;
	}

	if (logic.GetStatus == NULL) {
		KLOG_E("GetStatus() is NULL");
		goto err;
	}

	down(&bmem_sem);
	result = logic.GetStatus(&bmem_status);
	up(&bmem_sem);
	if (result) {
		KLOG_E("GetStatus failed form the proc call");
		goto err;
	}

	down(&bmem_status_sem);
	BMEM_PROC_PRINT_HDR("Current Usage Info");
	BMEM_PROC_PRINT_D("Used space in bytes", bmem_status.total_used_space);
	BMEM_PROC_PRINT_D("Free space in bytes", bmem_status.total_free_space);
	BMEM_PROC_PRINT_D("Num Buffers in Use", bmem_status.num_buf_used);

	BMEM_PROC_PRINT_HDR("Statistics");
	BMEM_PROC_PRINT_D("Maximum Memory usage", bmem_status.max_used_space);
	BMEM_PROC_PRINT_D("Biggest Buffer Requested", bmem_status.biggest_buf_request);
	BMEM_PROC_PRINT_D("Smallest Buffer Requested", bmem_status.smallest_buf_request);
	BMEM_PROC_PRINT_D("Allocate Success Count", bmem_status.alloc_pass_cnt);
	BMEM_PROC_PRINT_D("Mem Free Success Count", bmem_status.free_pass_cnt);

	BMEM_PROC_PRINT_HDR("Fragmentation Info");
	BMEM_PROC_PRINT_D("Num Buffers Free", bmem_status.num_buf_free);
	BMEM_PROC_PRINT_D("Biggest Buffer Available", bmem_status.biggest_chunk_avlbl);
	BMEM_PROC_PRINT_D("Smallest Buffer Available", bmem_status.smallest_chunk_avlbl);
	BMEM_PROC_PRINT_D("Max Num Holes occured", bmem_status.max_num_buf_free);
	BMEM_PROC_PRINT_D("Max Fragmented", bmem_status.max_fragmented_size);

	BMEM_PROC_PRINT_HDR("Error Info");
	BMEM_PROC_PRINT_D("Allocate Failures", bmem_status.alloc_fail_cnt);
	BMEM_PROC_PRINT_D("Mem Free Failures", bmem_status.free_fail_cnt);
	up(&bmem_status_sem);

err:
	if (start) {
		*start = page;
	}
	if (eof) {
		*eof = 1;
	}
	return (len < count) ? len : count;
}

static int bmem_parse_string(const char *inputStr, u32 *opCode, u32 *arg)
{
	int numArg;
	char tempStr[MAX_STR_SIZE];

	*opCode = 0;

	numArg = sscanf(inputStr, "%s%u", tempStr, arg);

	if (numArg < 1) {
		return -1;
	}

	if (strcmp(tempStr, "reset_statistics") == 0) {
		*opCode = 1;
	}
	else if (strcmp(tempStr, "1") == 0) {
		*opCode = 1;
	}
	else if (strcmp(tempStr, "reset_bmem") == 0) {
		*opCode = 2;
	}
	else if (strcmp(tempStr, "2") == 0) {
		*opCode = 2;
	}
	else if (strcmp(tempStr, "threshold") == 0) {
		*opCode = 3;
	}
	else if (strcmp(tempStr, "debug") == 0) {
		*opCode = 4;
	}
	else if (strcmp(tempStr, "stat") == 0) {
		*opCode = 5;
	}

	return 0;
}

/*
 * bmem_proc_set_status()
 * Description : proc write callback for multiple purposes
 *		1 or reset_statistics	- clear the min/max values to start statistics fresh
 *		2 or reset_bmem 		- reset the debug, stat levels and threshold
 *		threshold %d			- set the bmem threshold
 *		debug %d 				- set the debug level
 *			1 - Print on all alloc and free
 *			2 - Print fragmentation info on all alloc/free
 *			3 - Print entire heap partition on all alloc and free
 *		stat %d 				- set the statistics level
 *			1 - Do run-time fragmentation check on all alloc/free
 *			2 - Print entire heap partition on calls to get the status
 */
static int bmem_proc_set_status(struct file *file,
		    const char *buffer, unsigned long count, void *data)
{
	int result;
	bmem_set_status_t bmem_set_status;
	char inputStr[MAX_STR_SIZE];
	int len;
	u32 opCode = 0;
	u32 arg;

	if (logic.SetStatus == NULL) {
		KLOG_E("SetStatus() is NULL");
		goto err;
	}

	if (count > MAX_STR_SIZE)	len = MAX_STR_SIZE;
	else len = count;
	if (copy_from_user(inputStr, buffer, len))
		return -EFAULT;

	inputStr[MAX_STR_SIZE-3] = '\0';
	inputStr[MAX_STR_SIZE-2] = ' ';
	inputStr[MAX_STR_SIZE-1] = '0';
	bmem_parse_string(inputStr, &opCode, &arg);

	switch (opCode) {
	case 1:
		KLOG_D ("Clearing max/min values in bmem status");
		down(&bmem_sem);
		bmem_set_status.cmd = BMEM_CLEAR_STAT;
		result = logic.SetStatus(&bmem_set_status);
		up(&bmem_sem);
		break;

	case 2:
		KLOG_D ("Resetting the debug, stat levels and threshold");
		down(&bmem_sem);
		bmem_set_status.cmd = BMEM_RESET;
		result = logic.SetStatus(&bmem_set_status);
		up(&bmem_sem);
		break;

	case 3:
		KLOG_D ("Setting the debug level to [%d]", arg);
		down(&bmem_sem);
		bmem_set_status.cmd = BMEM_SET_SMALL_CHUNK_THRESHOLD;
		bmem_set_status.data.chunk_threshold = arg;
		result = logic.SetStatus(&bmem_set_status);
		up(&bmem_sem);
		break;

	case 4:
		KLOG_D ("Setting the debug level to [%d]", arg);
		down(&bmem_sem);
		bmem_set_status.cmd = BMEM_SET_DEBUG_LEVEL;
		bmem_set_status.data.debug_level = arg;
		result = logic.SetStatus(&bmem_set_status);
		up(&bmem_sem);
		break;

	case 5:
		KLOG_D ("Setting the statistics level to [%d]", arg);
		down(&bmem_sem);
		bmem_set_status.cmd = BMEM_SET_STAT_LEVEL;
		bmem_set_status.data.stat_level = arg;
		result = logic.SetStatus(&bmem_set_status);
		up(&bmem_sem);
		break;

	default:
		KLOG_D ("1 or reset_statistics   - clear the min/max values to start statistics fresh");
		KLOG_D ("2 or reset_bmem         - reset the debug, stat levels and threshold");
		KLOG_D ("threshold n             - set the threshold level");
		KLOG_D ("debug n                 - set the debug level");
		KLOG_D ("   1 - Print on all alloc and free");
		KLOG_D ("   2 - fragmentation info on all alloc/free");
		KLOG_D ("   3 - Print entire heap partition on all alloc and free");
		KLOG_D ("stat n                  - set the statistics level");
		KLOG_D ("   1 - Do run-time fragmentation check on all alloc/free");
		KLOG_D ("   2 - Print entire heap partition on calls to get the status");
		break;
	}

err:
	return count;
}

/*
 * bmem_wrapper_ioctl()
 * Description : ioctl implementation of the bmem driver
 */
static int bmem_wrapper_ioctl(struct inode *inode, struct file *filp,
				  unsigned int cmd, unsigned long arg)
{
	int result = -1;

	pr_debug(KERN_DEBUG "bmem_wrapper_ioctl: ioctl cmd 0x%08x\n", cmd);

	if (inode == NULL || filp == NULL) {
		KLOG_E("inode or filp found NULL");
		return -EFAULT;
	}
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if ((_IOC_TYPE(cmd) != BMEM_WRAP_MAGIC) && (_IOC_TYPE(cmd) != PMEM_IOCTL_MAGIC)
		&& (_IOC_TYPE(cmd) != HANTRO_WRAP_MAGIC) ) {
		KLOG_E("ioctl IOC_TYPE does not match expected [PMEM, GE, ME] magic : cmd[0x%08x]", cmd);
		return -ENOTTY;
	}

	/* ioctl number check disabled till all mem interfaces unified */
	if (_IOC_NR(cmd) > BMEM_WRAP_MAXNR) {
		KLOG_E("ioctl IOC_NR exceeds max[%d] : cmd[0x%08x]", BMEM_WRAP_MAXNR, cmd);
		return -ENOTTY;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		result = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		result = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
	if (result) {
		KLOG_E("ioctl access type mismatch");
		return -EFAULT;
	}

	KLOG_V ("file[%p] cmd[0x%08x]", filp, cmd);

	switch (cmd) {

	case GEMEMALLOC_WRAP_ACQUIRE_BUFFER:
		{
			int ret = 0;
			bmem_wrapper_data_t *p_data = filp->private_data;
			GEMemallocwrapParams memparams;
			unsigned int page_aligned_size;

			ret = copy_from_user(&memparams, (const void *)arg,
					 sizeof(memparams));
			if(ret != 0) {
				printk("Error in copying user arguments in GEMEMALLOC_WRAP_ACQUIRE_BUFFER");
				return ret;
			}
			page_aligned_size = (((memparams.size + (PAGE_SIZE-1)) >> PAGE_SHIFT) << PAGE_SHIFT);
			if (memparams.size != page_aligned_size) {
				KLOG_V("GEMEMALLOC_WRAP_ACQUIRE_BUFFER: size[0x%08x] made multiple of page size",
					memparams.size);
				memparams.size = page_aligned_size;
			}
			if (page_aligned_size == 0) {
				KLOG_E("Zero size alloction requested");
				memparams.busAddress = 0;
				result = -EINVAL;
			} else {
				if (logic.AllocMemory != NULL) {
					down(&bmem_sem);
					result =
					    logic.AllocMemory(p_data->bmem_handle,
							      &memparams.busAddress,
							      memparams.size);
					up(&bmem_sem);
				} else {
					KLOG_E("AllocMemory() is NULL");
					memparams.busAddress = 0;
					result = 1;
				}
			}
			__copy_to_user((void *)arg, &memparams,
				       sizeof(memparams));

			if (result) {
				KLOG_E("GEMEMALLOC_WRAP_ACQUIRE_BUFFER: Allocate Failed for size[0x%08x]",
					memparams.size);
				bmem_print_status();
			}else {
				KLOG_V ("GEMEMALLOC_WRAP_ACQUIRE_BUFFER: addr[0x%08x] size[0x%08x]",
					(int)memparams.busAddress, memparams.size);
			}
		}
		break;
	case GEMEMALLOC_WRAP_RELEASE_BUFFER:
		{
			bmem_wrapper_data_t *p_data = filp->private_data;
			unsigned long busaddr;

			__get_user(busaddr, (unsigned long *)arg);

			if (logic.FreeMemory != NULL) {
				down(&bmem_sem);
				result = logic.FreeMemory(p_data->bmem_handle, &busaddr);
				up(&bmem_sem);

			} else {
				KLOG_E("FreeMemory() is NULL");
				result = 1;
			}
			if (result) {
				KLOG_E("GEMEMALLOC_WRAP_RELEASE_BUFFER: Free Failed for handle[%d] addr[0x%08x]", (int)p_data->bmem_handle, (int)busaddr);
			}else {
				KLOG_V ("GEMEMALLOC_WRAP_RELEASE_BUFFER: addr[0x%08x]", (int)busaddr);
			}

		}
		break;
	case GEMEMALLOC_WRAP_COPY_BUFFER:
		{
			DmaStruct dma_buffers;
			int ret = 0;

			ret = copy_from_user(&dma_buffers, (const void *)arg,
					 sizeof(dma_buffers));
			if(ret != 0) {
				printk("Error in copying user arguments in GEMEMALLOC_WRAP_COPY_BUFFER");
				return ret;
			}

			KLOG_V ("GEMEMALLOC_WRAP_COPY_BUFFER");
			/*Request the channel */
			if (OSDAL_ERR_OK != OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT_MEMORY, OSDAL_DMA_CLIENT_MEMORY,(UInt32 *)&gDmaChannel))
			{
				KLOG_E("---->OSDAL_DMA_Obtain_Channel failed for channel %d \n",gDmaChannel);
				dma_cleanup();
				return -1;
			}
			KLOG_V("Channel num=%d\n",gDmaChannel);

			//Allocate memory for array of pointers
			dmaBuffListMem = (OSDAL_Dma_Buffer_List *)kmalloc((sizeof(OSDAL_Dma_Buffer_List)),GFP_KERNEL);
			if(!dmaBuffListMem)
			{
				KLOG_E("DMA-Failed to allocate memory for DMA BuffList\n");
				dma_cleanup();
				return -ENOMEM;
			}

			/* Configuring the DMA channel info */
			dmaChInfoMem.type = OSDAL_DMA_FCTRL_MEM_TO_MEM;
			dmaChInfoMem.srcBstSize = OSDAL_DMA_BURST_SIZE_16;
			dmaChInfoMem.dstBstSize = OSDAL_DMA_BURST_SIZE_16;
			dmaChInfoMem.srcDataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
			dmaChInfoMem.dstDataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
			dmaChInfoMem.dstMaster = 1;
			dmaChInfoMem.srcMaster = 1;
			dmaChInfoMem.incMode = OSDAL_DMA_INC_MODE_BOTH;
			dmaChInfoMem.freeChan = TRUE;
			dmaChInfoMem.priority = 1;
			dmaChInfoMem.bCircular = FALSE;
			dmaChInfoMem.alignment = OSDAL_DMA_ALIGNMENT_32;
			dmaChInfoMem.xferCompleteCb = (OSDAL_DMA_CALLBACK)dma_transfer_isr;

			if (OSDAL_DMA_Config_Channel(gDmaChannel, &dmaChInfoMem)!= OSDAL_ERR_OK)
			{
				KLOG_E("---->DMADRV_Config_Channel - Failed for channel %d \n",gDmaChannel);
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
				KLOG_E("DMADRV_Bind_Data - Failed\n");
				dma_cleanup();
				return -EINVAL;
			}

			// Start transfer
			dmaDone = DMA_NOT_DONE;
			if (OSDAL_DMA_Start_Transfer(gDmaChannel) != OSDAL_ERR_OK)
			{
				KLOG_E("DMADRV_Start_Transfer - Failed for channel %d\n",gDmaChannel);
				dma_cleanup();
				return -EINVAL;
			}

			if((wait_for_completion_interruptible_timeout(&dma_complete, msecs_to_jiffies(100))) == 0)
			{
				KLOG_E("Transfer timed out for channel %d\n",gDmaChannel);
				OSDAL_DMA_Force_Shutdown_Channel(gDmaChannel);
			}

			if(dmaDone == DMA_DONE_FAILURE)
			{
				KLOG_E("Error, DMA transfer failed.\n");
				dma_cleanup();
				return -1;
			}
			//Free all the allocated memories.
			dma_cleanup();
		}
 		break;
	case PMEM_GET_PHYS:
		{
			bmem_wrapper_data_t *p_data = filp->private_data;
			struct pmem_region region;

			if (p_data->bmem_pmem_data.allocated == 0) {
				region.offset = 0;
				region.len = 0;
			} else {
				region.offset = p_data->bmem_pmem_data.busAddress;
				region.len = p_data->bmem_pmem_data.size;
			}

			if (p_data->bmem_pmem_data.allocated == 0) {
				KLOG_E("GET_PHYS ioctl called without prior alloc/mmap");
			}else {
				KLOG_V("GET_PHYS ioctl addr[0x%08x] size[0x%08x]", (int)region.offset, (int)region.len);
			}

			if (copy_to_user((void __user *)arg, &region,
						sizeof(struct pmem_region)))
				return -EFAULT;
			result = 0;
		}
		break;
	case PMEM_CONNECT:
		{
			KLOG_V("PMEM_CONNECT ioctl request has come \n");
			result = bmem_pmem_connect(arg, filp);
		}
		break;
	case PMEM_MAP:
		{
			KLOG_V("PMEM_MAP ioctl : Dummy implementation \n");
			result = 0;
		}
		break;
	case PMEM_UNMAP:
	case PMEM_GET_SIZE:
	case PMEM_GET_TOTAL_SIZE:
	case PMEM_ALLOCATE:
	case PMEM_CACHE_FLUSH:
		{
			KLOG_E("pmem ioctl [0x%08x] currently not supported by this driver. \n", cmd);
			result = -1;
		}
		break;

	case HANTRO_WRAP_ACQUIRE_BUFFER:
		{
			bmem_wrapper_data_t *p_data = filp->private_data;
			MemallocwrapParams memparams;
			unsigned int page_aligned_size;
			int ret = 0;

			ret = copy_from_user(&memparams, (const void *)arg,
					 sizeof(memparams));
			if(ret != 0) {
				printk("Error in copying user arguments in HANTRO_WRAP_ACQUIRE_BUFFER");
				return ret;
			}

			page_aligned_size = (((memparams.size + (PAGE_SIZE-1)) >> PAGE_SHIFT) << PAGE_SHIFT);
			if (memparams.size != page_aligned_size) {
				KLOG_V("HANTRO_WRAP_ACQUIRE_BUFFER: size[0x%08x] made multiple of page size",
					memparams.size);
				memparams.size = page_aligned_size;
			}

			if (page_aligned_size == 0) {
				KLOG_E("Zero size alloction requested");
				memparams.busAddress = 0;
				result = -EINVAL;
			} else {
				if (logic.AllocMemory != NULL) {
					down(&bmem_sem);
					result =
						logic.AllocMemory(p_data->bmem_handle,
								  &memparams.busAddress,
								  memparams.size);
					memparams.nextAddress = NULL;
					up(&bmem_sem);
				} else {
					KLOG_E("AllocMemory() is NULL");
					memparams.busAddress = 0;
					result = 1;
				}
			}
			__copy_to_user((void *)arg, &memparams,
				       sizeof(memparams));

			if (result) {
				KLOG_E("HANTRO_WRAP_ACQUIRE_BUFFER: Allocate Failed for size[0x%08x]",
					memparams.size);
				bmem_print_status();
			}else {
				KLOG_V ("HANTRO_WRAP_ACQUIRE_BUFFER: addr[0x%08x] size[0x%08x]",
					(int)memparams.busAddress, memparams.size);
			}
		}
		break;
	case HANTRO_WRAP_RELEASE_BUFFER:
		{
			bmem_wrapper_data_t *p_data = filp->private_data;
			MemallocwrapParams temp_hw_OutBuf;
			int ret = 0;

			ret = copy_from_user(&temp_hw_OutBuf, (const void *)arg,sizeof(MemallocwrapParams));
			if(ret != 0) {
				printk("Error in copying user arguments in HANTRO_WRAP_RELEASE_BUFFER");
				return ret;
			}

			if (logic.AllocMemory != NULL) {
				down(&bmem_sem);
				result = logic.FreeMemory(p_data->bmem_handle, &temp_hw_OutBuf.busAddress);
				up(&bmem_sem);
				down(&bmem_virt_list_sem);
				remove_from_virt_list(&temp_hw_OutBuf);
				up(&bmem_virt_list_sem);
			} else {
				KLOG_E("FreeMemory() is NULL");
				result = 1;
			}

			if (result) {
				KLOG_E("HANTRO_WRAP_RELEASE_BUFFER: Free Failed for addr[0x%08x]", (int)temp_hw_OutBuf.busAddress);
			}else {
				KLOG_V ("HANTRO_WRAP_RELEASE_BUFFER: addr[0x%08x]", (int)temp_hw_OutBuf.busAddress);
			}
		}
		break;

	case HANTRO_STORE_OUTBUFFER:
		{
			MemallocwrapParams temp_hw_OutBuf;
			int ret = 0;
			KLOG_V("HANTRO_STORE_OUTBUFFER\n");
			ret = copy_from_user(&temp_hw_OutBuf, (const void *)arg,sizeof(MemallocwrapParams));
			if(ret != 0) {
				printk("Error in copying user arguments in HANTRO_STORE_OUTBUFFER");
				return ret;
			}
			down(&bmem_virt_list_sem);
			add_to_virt_list(&temp_hw_OutBuf);
			up(&bmem_virt_list_sem);
		}
		break;

	case HANTRO_GET_OUTBUFFER:
		{
			MemallocwrapParams temp_hw_OutBuf;
			int ret = 0;
			KLOG_V("HANTRO_GET_OUTBUFFER\n");
			ret = copy_from_user(&temp_hw_OutBuf, (const void *)arg,sizeof(MemallocwrapParams));
			if(ret != 0) {
				printk("Error in copying user arguments in HANTRO_GET_OUTBUFFER");
				return ret;
			}
			down(&bmem_virt_list_sem);
			temp_hw_OutBuf.busAddress = get_from_virt_list(temp_hw_OutBuf.virtualaddress);
			up(&bmem_virt_list_sem);
			ret = copy_to_user(((MemallocwrapParams*)arg), &temp_hw_OutBuf,sizeof(MemallocwrapParams));
			if(ret != 0) {
				printk("Error in copying user arguments in HANTRO_GET_OUTBUFFER 1");
				return ret;
			}
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

			KLOG_V ("HANTRO_DMA_COPY");
			/*Request the channel */
			/*Request the channel */
			while(OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT_MEMORY, OSDAL_DMA_CLIENT_MEMORY,(UInt32 *)&gDmaChannel) != OSDAL_ERR_OK) {
				msleep(20);
				tries++;
			}
			if (tries)
				pr_debug("memwrapper: channel 11 obtain tried for %d times\n", tries);

			//Allocate memory for array of pointers
			dmaBuffListMem = (OSDAL_Dma_Buffer_List *)kmalloc((sizeof(OSDAL_Dma_Buffer_List)),GFP_KERNEL);
			if(!dmaBuffListMem)
			{
				KLOG_E("DMA-Failed to allocate memory for DMA BuffList\n");
				dma_cleanup();
				return -ENOMEM;
			}

			/* Configuring the DMA channel info */
			dmaChInfoMem.type = OSDAL_DMA_FCTRL_MEM_TO_MEM;
			dmaChInfoMem.srcBstSize = OSDAL_DMA_BURST_SIZE_32;
			dmaChInfoMem.dstBstSize = OSDAL_DMA_BURST_SIZE_32;
			dmaChInfoMem.srcDataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
			dmaChInfoMem.dstDataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
			dmaChInfoMem.incMode = OSDAL_DMA_INC_MODE_BOTH;
			dmaChInfoMem.freeChan = TRUE;
			dmaChInfoMem.priority = 2;
			dmaChInfoMem.bCircular = FALSE;
			dmaChInfoMem.alignment = OSDAL_DMA_ALIGNMENT_32;
			dmaChInfoMem.xferCompleteCb = (OSDAL_DMA_CALLBACK)dma_transfer_isr;

			if (OSDAL_DMA_Config_Channel(gDmaChannel, &dmaChInfoMem)!= OSDAL_ERR_OK)
			{
				KLOG_E("---->DMADRV_Config_Channel - Failed for channel %d \n",gDmaChannel);
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
				KLOG_E("DMADRV_Bind_Data - Failed\n");
				dma_cleanup();
				return -EINVAL;
			}

			// Start transfer
			dmaDone = DMA_NOT_DONE;
			if (OSDAL_DMA_Start_Transfer(gDmaChannel) != OSDAL_ERR_OK)
			{
				KLOG_E("DMADRV_Start_Transfer - Failed for channel %d\n",gDmaChannel);
				dma_cleanup();
				return -EINVAL;
			}

			if((wait_for_completion_interruptible_timeout(&dma_complete, msecs_to_jiffies(100))) == 0)
			{
				KLOG_E("Transfer timed out for channel %d\n",gDmaChannel);
				OSDAL_DMA_Force_Shutdown_Channel(gDmaChannel);
			}

			if(dmaDone == DMA_DONE_FAILURE)
			{
				KLOG_E("Error, DMA transfer failed.\n");
				dma_cleanup();
				return -1;
			}
			//Free all the allocated memories.
			dma_cleanup();
		}
		break;
	default:
		{
			KLOG_E ("Invalid ioctl command : file[%p] cmd[0x%08x]", filp, cmd);
		}
		break;
	}
	return result;
}

/*
 * bmem_wrapper_open()
 * Description : open() implementation of the bmem driver
 */
static int bmem_wrapper_open(struct inode *inode, struct file *filp)
{
	int r;

	bmem_wrapper_data_t *p_data = kmalloc(sizeof(bmem_wrapper_data_t), GFP_KERNEL);
	if (!p_data) {
		KLOG_E("kmalloc failed");
		return -ENOMEM;
	}

	filp->private_data = p_data;
	if (logic.open != NULL) {
		down(&bmem_sem);
		p_data->bmem_pmem_data.allocated = 0;
		p_data->bmem_pmem_data.filp = filp;
		p_data->bmem_pmem_data.connected = 0;
		p_data->bmem_pmem_data.master_fd = -1;
		p_data->bmem_pmem_data.master_filp = NULL;
		r = logic.open(&p_data->bmem_handle);
		up(&bmem_sem);
	} else {
		KLOG_E("open() is NULL");
		r = -1;
	}

	KLOG_V ("file[%p] handle[%p], return[%d]", filp, p_data->bmem_handle, r);

	return r;
}

/*
 * bmem_wrapper_release()
 * Description : close() implementation of the bmem driver.
 *		Would free all memories allocated using this file handle.
 */
static int bmem_wrapper_release(struct inode *inode, struct file *filp)
{

	int r=1;
	bmem_wrapper_data_t *p_data = filp->private_data;

	KLOG_V ("file[%p] handle[%p]", filp, p_data->bmem_handle);

	if ((p_data) && (logic.release)) {
		down(&bmem_sem);
		r = logic.release(p_data->bmem_handle);
		up(&bmem_sem);
	}

	if (p_data)
		kfree(p_data);

	return r;
}

/*
 * bmem_wrapper_mmap()
 * Description : mmap() implementation of the bmem driver
 *		Internally identifies the user (pmem or not) and acts accordingly.
 *		For pmem, cacheability is selected based on O_SYNC flag
 *		For others, mapping is done cacheable.
 */
static int bmem_wrapper_mmap(struct file *file,
				   struct vm_area_struct *vma)
{
	bmem_wrapper_data_t *p_data = file->private_data;
	int r;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long busAddress;
	unsigned long page_aligned_size;

	KLOG_V ("mmap: file[%p] start[0x%08x], size[0x%08x] pgoff[0x%08x]",
		file, (int)vma->vm_start, (int)size, (int)vma->vm_pgoff);
	if ( ((size >> PAGE_SHIFT) << PAGE_SHIFT) != size) {
		KLOG_E("size for mmap not multiple of page size");
	}

	if (logic.mmap != NULL) {
		down(&bmem_sem);
		r = logic.mmap(size, vma->vm_pgoff);
		up(&bmem_sem);
	} else {
		KLOG_E("mmap() is NULL");
		return -EFAULT;
	}

	if (0 == r) {
		/* mmap called for a acquire/free interface - valid pgoff (phys addr)
		   present which is already allocated */
		/* vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot); */
		vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);

		/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
		if (remap_pfn_range(vma,
				    vma->vm_start,
				    vma->vm_pgoff,
				    vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
			KLOG_E("acquire mmap failed: pgoff[0x%08x] virt[0x%08x] size[0x%08x]",
				(int)vma->vm_pgoff, (int)vma->vm_start, (int)(vma->vm_end - vma->vm_start));
			return -EAGAIN;
		}
		KLOG_V("acquire mmap passed: pgoff[0x%08x] virt[0x%08x] size[0x%08x]",
			(int)vma->vm_pgoff, (int)vma->vm_start, (int)(vma->vm_end - vma->vm_start));
	} else if (is_mmap_for_pmem_interface(file, vma)) {
		/* mmap seems to be for pmem type of interface */
		KLOG_V ("mmap for pmem: file[%p] start[0x%08x], size[0x%08x]", file, (int)vma->vm_start, (int)size);
		if (is_pmem_alloc_needed(file, vma)) {
			/* Seems to be the master file malloc request */
			busAddress = 0;
			page_aligned_size = (((size + (PAGE_SIZE-1)) >> PAGE_SHIFT) << PAGE_SHIFT);
			if (size != page_aligned_size) {
				KLOG_V("Alloc for pmem: size[0x%08x] made multiple of page size", (int)size);
			}
			if (page_aligned_size == 0) {
				KLOG_E("Zero size alloction requested");
				busAddress = 0;
				return -EFAULT;
			}			
			if (logic.AllocMemory != NULL) {
				down(&bmem_sem);
				r = logic.AllocMemory(p_data->bmem_handle, &busAddress, page_aligned_size);
				up(&bmem_sem);
			} else {
				KLOG_E("AllocMemory() is NULL");
				return -EAGAIN;
			}
			if (r) {
				KLOG_E ("Allocation of memory for pmem failed for size[0x%08x]", (int)page_aligned_size);
				bmem_print_status();
				return -EAGAIN;
			}
			KLOG_V ("Allocate memory for pmem: addr[0x%08x] size[0x%08x]", (int)busAddress, (int)page_aligned_size);
			pmem_alloc_done(file, vma, busAddress);
		}else {
			busAddress = p_data->bmem_pmem_data.busAddress;
			KLOG_V ("Multiple mmaps for pmem from Hantro: addr[0x%08x]", (int)busAddress);
		}
		/* Setup the page tables for the mapping */
		vma->vm_pgoff = busAddress >> PAGE_SHIFT;
		if (file->f_flags & O_SYNC) {
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			KLOG_V("set page tables in non-cached mode \n");
		}
		else {
			vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);
			KLOG_V("set page tables in cached mode \n");
		}
		if (io_remap_pfn_range(vma,
					vma->vm_start,
					vma->vm_pgoff,
					size, vma->vm_page_prot)) {
			KLOG_E("pmem mmap failed: pgoff[0x%08x] virt[0x%08x] size[0x%08x]",
				(int)vma->vm_pgoff, (int)vma->vm_start, (int)size);
			/* TODO: Free up allocated memory ??*/
			return -EAGAIN;
		}
		KLOG_V("pmem mmap passed: pgoff[0x%08x] virt[0x%08x] size[0x%08x]",
			(int)vma->vm_pgoff, (int)vma->vm_start, (int)size);
		r = 0;
	} else {
		KLOG_E("Invalid mmap params - nonzero[0x%x]pgoff, but not allocated", (int)vma->vm_pgoff);
		r = -1;
	}
	return r;
}

/* VFS methods */
static const struct file_operations bmem_wrapper_fops = {
	.open = bmem_wrapper_open,
	.release = bmem_wrapper_release,
	.ioctl = bmem_wrapper_ioctl,
	.mmap = bmem_wrapper_mmap,
};

/*
 * register_bmem_wrapper()
 * Description : bmem allocator module will register itself (callback functions)
 *		with the wrapper using this function
 */
int register_bmem_wrapper(struct bmem_logic *bmem_fops)
{
	KLOG_D("Entry\n");
	if (bmem_fops != NULL) {
		memcpy(&logic, bmem_fops, sizeof(logic));
	} else {
		KLOG_E("bmem_fops is NULL");
		return -1;
	}

	if (logic.init != NULL) {
		return logic.init(BMEM_SIZE, dma_cohr_start_addr);
	} else {
		KLOG_E("init() is NULL");
		return -1;
	}
}

EXPORT_SYMBOL(register_bmem_wrapper);

/*
 * register_bmem_wrapper()
 * Description : bmem allocator module will deregister itself from the wrapper
 */
void deregister_bmem_wrapper(void)
{
	KLOG_D("Entry\n");
	if (logic.cleanup)
		logic.cleanup();
}

EXPORT_SYMBOL(deregister_bmem_wrapper);

/*
 * bmem_wrapper_init()
 * Description : Driver init function
 */
int __init bmem_wrapper_init(void)
{
	int result;

	KLOG_V("Entry\n");
	result =
	    register_chrdev(bmem_major, "gememalloc",
			    &bmem_wrapper_fops);
	if (result < 0) {
		KLOG_E("unable to get major %d\n", bmem_major);
		return result;
	} else if (result != 0) {	/* this is for dynamic major */
		bmem_major = result;
	}

	init_completion(&dma_complete);

	if (bmem_mempool_base == NULL) {
		result = -ENOMEM;
		KLOG_E("bmem memory not reserved at boot time. \n");
		goto err;
	}
	dma_cohr_start_addr = (dma_addr_t) virt_to_phys(bmem_mempool_base);

	KLOG_D("Module inserted: major[%d]\n", bmem_major);

	init_MUTEX(&bmem_sem);
	init_MUTEX(&bmem_status_sem);
	init_MUTEX(&bmem_virt_list_sem);

	bmem_proc_file = create_proc_entry(DEV_NAME, 0644, NULL);
	if (bmem_proc_file) {
		bmem_proc_file->data = NULL;
		bmem_proc_file->read_proc = bmem_proc_get_status;
		bmem_proc_file->write_proc = bmem_proc_set_status;
		// bmem_proc_file->owner = THIS_MODULE;
	}
	else {
		KLOG_E("Failed creating proc entry");
	}

	return 0;

err:
	return result;
}

/*
 * bmem_wrapper_cleanup()
 * Description : Driver exit function
 */
void __exit bmem_wrapper_cleanup(void)
{

	KLOG_D("Entry\n");

	unregister_chrdev(bmem_major, "gememalloc");
	if (bmem_proc_file) {
		remove_proc_entry(DEV_NAME, bmem_proc_file);
	}

	pr_notice("bmem_wrapper: module removed\n");
	return;
}

module_init(bmem_wrapper_init);
module_exit(bmem_wrapper_cleanup);

