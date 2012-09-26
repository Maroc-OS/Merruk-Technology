/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#include <linux/broadcom/bmem_wrapper.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "linux/delay.h"
#include "linux/sched.h"
#include "asm/current.h"
#include <cfg_global.h>
/* Our header */
#include "bmem.h"

#define BMEM_SW_MINOR 0
#define BMEM_SW_MAJOR 1
#define BMEM_SW_BUILD ((BMEM_SW_MAJOR * 1000) + BMEM_SW_MINOR)

#define PAGE_SHIFT 12

#define SMALL_CHUNK_THRESHOLD    (64 * 1024)

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Memory allocation");

#ifndef MIN
#define MIN(A,B)      ((A) > (B) ? (B) : (A))
#endif

#ifndef MAX
#define MAX(A,B)      ((A) < (B) ? (B) : (A))
#endif

#define DBG_PRN_ALLOC    (0x1)
#define DBG_PRN_FRAG     (0x2)
#define DBG_PRN_HEAP     (0x3)

#define STAT_FRAG_CHECK  (0x1)
#define STAT_PRN_HEAP    (0x2)

// #define PRINT_BMEM_HEAP_LIST_ON_ERROR

#define KLOG_TAG    "bmem.c"
#if 1
#define KLOG_D(fmt,args...) \
        do { printk(KERN_INFO KLOG_TAG "[%s:%d] "fmt"\n", __func__, __LINE__, \
            ##args); } \
        while (0)
#else
#define KLOG_D(x...) do {} while (0)
#endif

typedef struct _chunk{
    unsigned int address;
#ifdef BMEM_CHECK_OVERRUN
    void *virt_address;
#endif
    unsigned int size;
    unsigned int handle;
    pid_t pid;
    pid_t tgid;
    bool used;
    struct _chunk* prev;
    struct _chunk* next;
} chunk;

static unsigned int openCounter = 1;
static chunk* memhead = NULL;
static chunk* memtail = NULL;
static unsigned int chunk_threshold = SMALL_CHUNK_THRESHOLD;
static unsigned int debug_level = 0;
static unsigned int stat_level = 0;
static bmem_status_t bmem_status;

#ifdef BMEM_CHECK_OVERRUN
    static int bmem_logic_init(unsigned int memory_size, unsigned int phy_start_address, void *virt_start_address);
#else
static int bmem_logic_init(unsigned int memory_size, unsigned int phy_start_address);
#endif
static int bmem_AllocMemory(BMEM_HDL hdl, unsigned long *busaddr, unsigned int size);
static int bmem_FreeMemory(BMEM_HDL hdl, unsigned long *busaddr);
static int bmem_GetStatus(bmem_status_t *p_bmem_status);
static int bmem_SetStatus(bmem_set_status_t *p_bmem_set_status);
static int bmem_GetFreeMemorySize(void);
static int bmem_GetUsedMemoryByTgid(pid_t tgid);
static int FreeMemoryForHandle(BMEM_HDL hdl );
static void CheckFragmentation(int OnStatus);


/*
 * bmem_open()
 * Description : open() callback function which will be called by wrapper when file is
 *        opened. Returns back a unique handle
 */
static int bmem_open(BMEM_HDL *hdlp)
{
    if (!openCounter) {
        printk(KERN_ERR "Error: bmem handle counter wrapped.");
    }
    *hdlp = (void *)openCounter;
    openCounter++;
    return 0;
}

/*
 * bmem_release()
 * Description : close() callback function which will be called by wrapper when file is
 *        closed. Frees all memory allocated using this file.
 */
static int bmem_release(BMEM_HDL hdl)
{
    FreeMemoryForHandle(hdl);
    return 0;
}

/*
 * bmem_mmap()
 * Description : mmap() callback function which will be called by wrapper when the
 *        physical buffer has to be mapped. Wrapper will call this function to identify
 *        whether the physical address is already allocated.
 */
static int bmem_mmap(unsigned long size, unsigned long pgoff)
{

    chunk* head = memhead;
    while(head != NULL)
    {
#ifdef BMEM_CHECK_OVERRUN
        if (((head->address>> PAGE_SHIFT) == pgoff ) && (size == (head->size - PAGE_SIZE)))
#else
        if (((head->address>> PAGE_SHIFT) == pgoff ) && (size == head->size))
#endif
        {
            break;
        }
        head = head->next;
    }
    if(head == NULL){
        return -1;
    }

    return 0;
}

static struct bmem_logic bmem_fops = {
    .AllocMemory = bmem_AllocMemory,
    .FreeMemory = bmem_FreeMemory,
    .open = bmem_open,
    .release = bmem_release,
    .mmap = bmem_mmap,
    .init = bmem_logic_init,
    .cleanup = NULL,
    .GetStatus = bmem_GetStatus,
    .SetStatus = bmem_SetStatus,
    .GetFreeMemorySize = bmem_GetFreeMemorySize,
    .GetUsedMemoryByTgid = bmem_GetUsedMemoryByTgid,
};

/*
 * bmem_init()
 * Description : module init function which will register itself with the wrapper.
 */
int bmem_init(void)
{
    int result = 0;

    printk(KERN_DEBUG "bmem_init\n");
    printk(KERN_INFO "bmem sw build: %d \n", BMEM_SW_BUILD);

    result = register_bmem_wrapper(&bmem_fops);
    if (result < 0) {
        printk(KERN_ERR "bmem: module not inserted\n");
        return result;
    }

    return 0;
}

/*
 * bmem_logic_init()
 * Description : init() callback function which will be called by wrapper to initialise
 *        the allocator module.
 */
#ifdef BMEM_CHECK_OVERRUN
    static int bmem_logic_init(unsigned int memory_size,
                    unsigned int phy_start_address, void *virt_start_address)
#else
static int bmem_logic_init(unsigned int memory_size,
                unsigned int phy_start_address)
#endif
{
    memhead = kmalloc(sizeof(chunk),GFP_KERNEL);
    if (memhead == NULL) {
        printk(KERN_ERR "bmem_logic_init: kmalloc failed. bmem driver cannot work\n");
        return -1;
    }
    memtail = memhead;

    memset (&bmem_status, 0, sizeof(bmem_status_t));
    bmem_status.total_free_space = memory_size;
    bmem_status.smallest_buf_request = 0x7FFFFFFF;
    bmem_status.num_buf_free = 1;
    bmem_status.max_num_buf_free = 1;
    memhead->address = phy_start_address;
    memhead->size = memory_size;
    memhead->used = false;
    memhead->handle = 0;
    memhead->pid = 0;
    memhead->tgid = 0;
    memhead->prev = NULL;
    memhead->next = NULL;
    chunk_threshold = SMALL_CHUNK_THRESHOLD;
    stat_level = 0;
#ifdef BMEM_CHECK_OVERRUN
    memhead->virt_address = virt_start_address;
    debug_level = 0;
    printk("bmem_logic_init addr: phys[0x%08x] virt[%p]\n", memhead->address, memhead->virt_address);
#else
    debug_level = 0;
    printk(KERN_DEBUG "bmem_logic_init addr: 0x%08x\n", memhead->address) ;
#endif

    return 0;
}

/*
 * bmem_cleanup()
 * Description : module exit() function
 */
void bmem_cleanup(void)
{
    chunk* head = memhead;
    chunk* temp;

    while(head != NULL)
    {
        temp = head;
        head = head->next;
        kfree(temp);
    }
    memhead = NULL;
    memtail = NULL;
    deregister_bmem_wrapper();

    printk(KERN_NOTICE "bmem: module removed\n");

    return;
}

module_init(bmem_init);
module_exit(bmem_cleanup);

/*
 * bmem_AllocMemory()
 * Description : Core allocator routine which will cycle through the buffers we have,
 *        divide the first free buffer which satisfies the size requirement and marks
 *        allocated
 */
static int bmem_AllocMemory(BMEM_HDL hdl, unsigned long *busaddr,
            unsigned int size)
{
    chunk* curr = memhead;
    int direction = 0;
    pid_t pid = current->pid;
    pid_t tgid = current->tgid;

#ifdef BMEM_CHECK_OVERRUN
    unsigned char* virt_addr = NULL;

    size += PAGE_SIZE;
#endif
    if (size <= chunk_threshold) {
        direction = 1;
        curr = memtail;
    }

    *busaddr = 0;

    do
    {
        if (curr->used == false)
        {
            if(curr->size > size)
            {
                chunk* temp = kmalloc(sizeof(chunk),GFP_KERNEL);
                if (temp == NULL) {
                    printk(KERN_ERR "bmem_AllocMemory: kmalloc failed. \n");
                    break;
                }
                temp->next = curr->next;
                temp->prev = curr;
                curr->next = temp;
                if (direction == 0) {
                    temp->address = curr->address + size;
#ifdef BMEM_CHECK_OVERRUN
                    temp->virt_address = curr->virt_address + size;
                    virt_addr = curr->virt_address;
#endif
                    temp->size = curr->size - size;
                    temp->used = false;
                    temp->handle = 0;
                    temp->pid = 0;
                    temp->tgid = 0;
                    curr->size = size;
                    curr->used = true;
                    curr->handle = (unsigned int)hdl;
                    curr->pid = pid;
                    curr->tgid = tgid;
                    *busaddr = (unsigned long) curr->address;
                } else {
                    temp->address = curr->address + (curr->size - size);
#ifdef BMEM_CHECK_OVERRUN
                    temp->virt_address = curr->virt_address + (curr->size - size);
                    virt_addr = temp->virt_address;
#endif
                    temp->size = size;
                    temp->used = true;
                    temp->handle = (unsigned int)hdl;
                    temp->pid = pid;
                    temp->tgid = tgid;
                    curr->size = curr->size - size;
                    curr->used = false;
                    curr->handle = 0;
                    curr->pid = 0;
                    curr->tgid = 0;
                    *busaddr = (unsigned long) temp->address;
                }
                if(temp->next) {
                    temp->next->prev = temp;
                } else {
                    memtail = temp;
                }
                break;
            } else if(curr->size == size) {
                curr->used = true;
                curr->handle = (unsigned int)hdl;
                curr->pid = pid;
                curr->tgid = tgid;
                *busaddr = (unsigned long) curr->address;
#ifdef BMEM_CHECK_OVERRUN
                virt_addr = curr->virt_address;
#endif
                bmem_status.num_buf_free -= 1;
                break;
            }
        }
        if (direction == 0) {
            curr = curr->next;
        } else {
            curr = curr->prev;
        }
    }while(curr != 0);

    if (*busaddr == 0) {
        bmem_status.alloc_fail_cnt++;
#ifdef PRINT_BMEM_HEAP_LIST_ON_ERROR
    debug_level |= 0x40;
    CheckFragmentation(0);
    debug_level &= ~0x40;
#endif
        return 1;
    }
    bmem_status.alloc_pass_cnt++;
    bmem_status.total_used_space += size;
    bmem_status.total_free_space -= size;
    bmem_status.max_used_space = MAX(bmem_status.total_used_space,bmem_status.max_used_space);
    bmem_status.num_buf_used += 1;
    bmem_status.smallest_buf_request = MIN(size,bmem_status.smallest_buf_request);
    bmem_status.biggest_buf_request = MAX(size,bmem_status.biggest_buf_request);
    bmem_status.max_num_buf_free = MAX(bmem_status.num_buf_free,bmem_status.max_num_buf_free);
#ifdef BMEM_CHECK_OVERRUN
    memset ((virt_addr + size - PAGE_SIZE), 0xAA, PAGE_SIZE);
    if (debug_level >= DBG_PRN_ALLOC) {
        KLOG_D("Alloc Handle[%d] pid[%d] tgid[%d] addr[0x%08x] virt_addr[0x%08x] Size[%d] ",
            (unsigned int)hdl, pid, tgid, (unsigned int)*busaddr, (unsigned int)(virt_addr + size - PAGE_SIZE), size);
    }
#else
    if (debug_level >= DBG_PRN_ALLOC) {
        KLOG_D("Alloc Handle[%d] pid[%d] tgid[%d] addr[0x%08x] Size[%d] ",
            (unsigned int)hdl, pid, tgid, (unsigned int)*busaddr, size);
    }
#endif
    CheckFragmentation(0);
    return 0;
}

#ifdef BMEM_CHECK_OVERRUN
static void bmem_check_overrun(chunk* free_hdl)
{
    unsigned char* virt_addr;
    unsigned int size, i;
#ifdef BMEM_CRASH_ON_OVERRUN
    unsigned int *p = (unsigned int *)0;
#endif
    unsigned char tmp_data[16];
    int overwrite_count = 0;

    virt_addr = free_hdl->virt_address;
    size = free_hdl->size;
    virt_addr = virt_addr + size - PAGE_SIZE;
    for (i=0; i<PAGE_SIZE; i++) {
        if (*virt_addr != 0xAA) {
            if (overwrite_count == 0) {
                printk("Memory overrun at i[%d]", i);
                printk("handle[%d] pid[%d], tgid[%d], address[0x%x] virt_address[%p] size[%d]",
                    free_hdl->handle, free_hdl->pid, free_hdl->tgid, free_hdl->address, virt_addr, free_hdl->size);
            }
            if (overwrite_count < 16) {
                tmp_data[overwrite_count] = *virt_addr;
            }
            overwrite_count++;
        }
        virt_addr++;
    }
    if (overwrite_count) {
        printk("Num_overruns[%d] \n", overwrite_count);
        for(i=0; i<overwrite_count && i<16; i++) {
            printk("[%d]:[%d] ", i, tmp_data[i]);
        }
#ifdef BMEM_CRASH_ON_OVERRUN
        msleep(5000);
        *p = 1;
#endif
    }
}
#endif

/*
 * bmem_FreeMemory()
 * Description : Free a buffer based on bus address. Callback used by wrapper to free
 *        the buffer when request comes from user.
 *        While free-ing, it will concatenate to adjacent free buffers to reduce fragmentation.
 */
static int bmem_FreeMemory(BMEM_HDL hdl, unsigned long *busaddr)
{
    chunk* head = memhead;
    int address = *busaddr;
    unsigned int freed_space = 0;
    pid_t pid = -1;
    pid_t tgid = -1;

    while(head != NULL) {
        if ( head->address == address) {
#ifdef BMEM_CHECK_OVERRUN
            bmem_check_overrun(head);
#endif
            break;
        }
        head = head->next;
    }
    if(head != NULL) {
        if (head->handle != (unsigned int)hdl) {
            printk (KERN_ERR "bmem free: handle[%d] pid[%d] tgid[%d] addr[0x%08x] does not match owner handle[%d]",
                (unsigned int)hdl, head->pid, head->tgid, (int)busaddr, head->handle);
        }
        pid = head->pid;
        tgid = head->tgid;
        head->used = false;
        head->handle = 0;
        head->pid = 0;
        head->tgid = 0;
        bmem_status.total_used_space -= head->size;
        bmem_status.total_free_space += head->size;
        bmem_status.num_buf_free += 1;
        freed_space += head->size;
        if(head->prev != NULL ) {
            if(head->prev->used == false){
                chunk* prev = head->prev;
                prev->next = head->next;
                if(head->next) {
                    head->next->prev = prev;
                } else {
                    memtail = prev;
                }
                prev->size += head->size;
                bmem_status.num_buf_free -= 1;
                kfree(head);
                head = prev;
            }
        }
        if(head->next != NULL ) {
            if(head->next->used == false) {
                chunk* next = head->next;
                head->next = next->next;
                head->size += next->size;
                if(next->next) {
                    next->next->prev = head;
                } else {
                    memtail = head;
                }
                bmem_status.num_buf_free -= 1;
                kfree(next);
            }
        }
    } else {
        bmem_status.free_fail_cnt++;
        return -1;
    }

    bmem_status.num_buf_used -= 1;
    bmem_status.free_pass_cnt++;
    bmem_status.max_num_buf_free = MAX(bmem_status.num_buf_free,bmem_status.max_num_buf_free);
    if (debug_level >= DBG_PRN_ALLOC) {
        KLOG_D("Free Handle[%d] pid[%d] tgid[%d] addr[0x%08x] ",
            (unsigned int)hdl, pid, tgid, address);
    }
    CheckFragmentation(0);
    return 0;
}

/*
 * FreeMemoryForHandle()
 * Description : Routine to free all the buffers allocated through a particular handle.
 *        While free-ing, it will concatenate to adjacent free buffers to reduce fragmentation.
 */
static int FreeMemoryForHandle(BMEM_HDL hdl)
{
    chunk* head = memhead;
    unsigned int freed_space = 0;
    pid_t pid = -1;
    pid_t tgid = -1;

    while(head != NULL)
    {
        if ( head->handle == (unsigned int)hdl) {
            pid = head->pid;
            tgid = head->tgid;
            head->used = false;
            head->handle = 0;
            head->pid = 0;
            head->tgid = 0;

#ifdef BMEM_CHECK_OVERRUN
            bmem_check_overrun(head);
#endif
            bmem_status.total_used_space -= head->size;
            bmem_status.total_free_space += head->size;
            bmem_status.num_buf_used -= 1;
            bmem_status.num_buf_free += 1;
            freed_space += head->size;
            if (debug_level >= DBG_PRN_ALLOC) {
                 KLOG_D("Free(On Close) Handle[%d] pid[%d] tgid[%d] addr[0x%08x] ",
                    (unsigned int)hdl, pid, tgid, head->address);
            }
            if(head->prev != NULL ) {
                if(head->prev->used == false) {
                    chunk* prev = head->prev;
                    prev->next = head->next;
                    if(head->next) {
                        head->next->prev = prev;
                    } else {
                        memtail = prev;
                    }
                    prev->size += head->size;
                    bmem_status.num_buf_free -= 1;
                    kfree(head);
                    head = prev;
                }
            }
            if(head->next != NULL ) {
                if(head->next->used == false) {
                    chunk* next = head->next;
                    head->next = next->next;
                    head->size += next->size;
                    if(next->next) {
                        next->next->prev = head;
                    } else {
                        memtail = head;
                    }
                    bmem_status.num_buf_free -= 1;
                    kfree(next);
                }
            }
            bmem_status.max_num_buf_free = MAX(bmem_status.num_buf_free,bmem_status.max_num_buf_free);
        }
        head = head->next;
    }

    CheckFragmentation(0);
    return 0;
}

/*
 * bmem_GetFreeMemorySize()
 * Description : Get the total memory used/allocated from bmem.
 */
static int bmem_GetFreeMemorySize(void)
{
    return bmem_status.total_free_space;
}

/*
 * bmem_GetUsedMemoryByTgid(pid_t tgid)
 * Description : Get the memory used/allocated from bmem by the TGID.
 */
static int bmem_GetUsedMemoryByTgid(pid_t tgid)
{
    chunk* head = memhead;
    unsigned int used_mem = 0;

    while(head != NULL) {
        if ((head->tgid == tgid) && (head->used)) {
            used_mem += head->size;
        }
        head = head->next;
    }
    return used_mem;
}

/*
 * GetAvailMemStatus()
 * Description : Routine to identify biggest and smallest continuous free space available
 *        in the heap. Used for collecting fragmenatation statistics. Based on debug/stat level,
 *        this function will print the entire heap status as well.
 */
static void CheckFragmentation(int OnStatus)
{
    chunk* curr = memhead;
    unsigned int biggest_chunk_avlbl = 0;
    unsigned int smallest_chunk_avlbl = 0x7FFFFFFF;
    unsigned int print_heap, run_loop = 0;;

    if (OnStatus) {
        print_heap = (stat_level >= STAT_PRN_HEAP);
    }else {
        print_heap = (debug_level >= DBG_PRN_HEAP);
    }
    if ((debug_level >= DBG_PRN_FRAG) || (stat_level >= STAT_FRAG_CHECK)) {
        run_loop = 1;
    }
    if (run_loop) {
        if (print_heap) {
            KLOG_D("memhead[0x%08x] addr[0x%08x], memtail[0x%08x] addr[0x%08x]",
                (unsigned int)memhead, memhead->address, (unsigned int)memtail, memtail->address);
        }
        while(curr != NULL)
        {
            if (print_heap) {
                KLOG_D("CURRENT[0x%08x] : addr[0x%08x] size[%d] used[%d] hdl[%d] pid[%d] tgid[%d]",
                    (unsigned int)curr, (unsigned int)curr->address, curr->size, (unsigned int)curr->used,
                    curr->handle, curr->pid, curr->tgid);
            }
            if (!curr->used)
            {
                biggest_chunk_avlbl = MAX(biggest_chunk_avlbl, curr->size);
                smallest_chunk_avlbl = MIN(smallest_chunk_avlbl, curr->size);
            }
            curr = curr->next;
        }
        bmem_status.biggest_chunk_avlbl = biggest_chunk_avlbl;
        bmem_status.smallest_chunk_avlbl = smallest_chunk_avlbl;
        bmem_status.max_fragmented_size = MAX(bmem_status.max_fragmented_size,
            (bmem_status.total_free_space - bmem_status.biggest_chunk_avlbl));
        if (debug_level >= DBG_PRN_FRAG) {
            KLOG_D("Frag Size[%d] : free space[%d] biggest free chunk[%d]",
                (bmem_status.total_free_space - bmem_status.biggest_chunk_avlbl),
                bmem_status.total_free_space, bmem_status.biggest_chunk_avlbl);
        }
    }
}

/*
 * bmem_GetStatus()
 * Description : Callback routine for wrapper to collect the heap status.
 */
static int bmem_GetStatus(bmem_status_t *p_bmem_status)
{
    if (p_bmem_status) {
        CheckFragmentation(1);
        memcpy(p_bmem_status, &bmem_status, sizeof(bmem_status_t));
        return 0;
    }else {
        return 1;
    }
}

/*
 * bmem_GetStatus()
 * Description : Callback routine for wrapper to either clear the statistics or enable/disable
 *        run-time fragmentation info collection.
 */
static int bmem_SetStatus(bmem_set_status_t *p_bmem_set_status)
{
    if (BMEM_CLEAR_STAT == p_bmem_set_status->cmd) {
        bmem_status.max_used_space = 0;
        bmem_status.smallest_buf_request = 0x7FFFFFFF;
        bmem_status.biggest_buf_request = 0;
        bmem_status.max_num_buf_free = 1;
        bmem_status.max_fragmented_size = 0;
    }else if (BMEM_RESET == p_bmem_set_status->cmd) {
        chunk_threshold = SMALL_CHUNK_THRESHOLD;
        debug_level = 0;
        stat_level = 0;
    }else if (BMEM_SET_SMALL_CHUNK_THRESHOLD == p_bmem_set_status->cmd) {
        chunk_threshold = p_bmem_set_status->data.chunk_threshold;
    }else if (BMEM_SET_DEBUG_LEVEL == p_bmem_set_status->cmd) {
        if (p_bmem_set_status->data.debug_level <= DBG_PRN_HEAP) {
            debug_level = p_bmem_set_status->data.debug_level;
        }
    }else if (BMEM_SET_STAT_LEVEL == p_bmem_set_status->cmd) {
        if (p_bmem_set_status->data.stat_level <= STAT_PRN_HEAP) {
            stat_level = p_bmem_set_status->data.stat_level;
        }
    }

    return 0;
}

