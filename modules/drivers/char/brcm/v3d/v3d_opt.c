/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/clk.h>
#include <mach/clkmgr.h>
#include <plat/syscfg.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/broadcom/v3d.h>
#include "reg_v3d.h"

#ifndef V3D_DEV_NAME
#define V3D_DEV_NAME	"v3d"
#endif

#ifndef V3D_DEV_MAJOR
#define V3D_DEV_MAJOR	0
#endif

#ifndef BCM_CLK_V3D_POWER_STR_ID
#define BCM_CLK_V3D_POWER_STR_ID	""
#endif

#ifndef BCM21553_V3D_BASE
#define BCM21553_V3D_BASE	0
#endif

#ifndef IRQ_GRAPHICS
#define IRQ_GRAPHICS	0
#endif

#define FLAG0	0x0
#define FLAG1	0x4
#define TURN	0x8

/* Job Error Handling variables */
#define V3D_JOB_CACHE_CLEAR
#ifdef V3D_JOB_CACHE_CLEAR
#define V3D_ISR_TIMEOUT_IN_MS	(250)
#define V3D_CACHE_MAX_RETRIES	(1)
#define V3D_JOB_TIMEOUT_IN_MS	(V3D_ISR_TIMEOUT_IN_MS * (V3D_CACHE_MAX_RETRIES+1))
#else
#define V3D_ISR_TIMEOUT_IN_MS	(500)
#define V3D_JOB_TIMEOUT_IN_MS	(V3D_ISR_TIMEOUT_IN_MS)
#endif

/* Enable the macro to retry the job on timeout, else will skip the job */
//#define V3D_JOB_RETRY_ON_TIMEOUT
#ifdef V3D_JOB_RETRY_ON_TIMEOUT
#define V3D_JOB_MAX_RETRIES (1)
#endif

typedef struct {
	u32 id;
} v3d_t;
v3d_t *v3d_dev;

typedef struct v3d_job_t_ {
	v3d_job_type_e 	job_type;
	u32				dev_id;
	uint32_t 		job_id;
	uint32_t 		v3d_ct0ca;
	uint32_t 		v3d_ct0ea;
	uint32_t 		v3d_ct1ca;
	uint32_t 		v3d_ct1ea;
	uint32_t 		v3d_vpm_size;
	u32 			cache_retry_cnt;
	u32 			retry_cnt;
	volatile v3d_job_status_e job_status;
	u32 			job_intern_state;
	u32				job_wait_state;
	wait_queue_head_t v3d_job_done_q;
	struct v3d_job_t_ *next;
} v3d_job_t;

/* Driver module init variables */
static int v3d_major = V3D_DEV_MAJOR;
static struct class *v3d_class;
static void __iomem *v3d_base = NULL;
static struct clk *gClkPower, *gClkAHB;
static int irq_err = 1;
struct task_struct *v3d_thread_task;
unsigned int v3d_mempool_phys_base;
extern void *v3d_mempool_base;
extern unsigned long v3d_mempool_size;
static int v3d_bin_oom_block;
static int v3d_bin_oom_size = (3*1024*1024);
static void* v3d_bin_oom_cpuaddr;
static int v3d_bin_oom_block2;
static int v3d_bin_oom_size2 = (3*1024*1024);
static void* v3d_bin_oom_cpuaddr2;
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
#include <mach/bcm21553_cpufreq_gov.h>
static struct cpufreq_client_desc *cpufreq_client;
#endif
#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
static spinlock_t v3d_id_lock = SPIN_LOCK_UNLOCKED;
extern void __iomem *sync_flag_base;
#endif

/* Static variable to check if the V3D power island is ON or OFF */
static int v3d_is_on = 0;

/* v3d driver state variables - shared by ioctl, isr, thread */
static u32 v3d_id = 1;
static volatile int v3d_in_use = 0;
	/* event bits 0:rend_done, 1:bin_done, 4:qpu_done, 5:oom_fatal */
static volatile int v3d_flags = 0;
static int v3d_oom_block_used = 0;
v3d_job_t *v3d_job_head = NULL;
volatile v3d_job_t *v3d_job_curr = NULL;

/* Semaphore to lock between ioctl and thread for shared variable access
 * WaitQue on which thread will block for job post or isr_completion or timeout
 */
struct semaphore v3d_sem;
wait_queue_head_t v3d_start_q, v3d_isr_done_q;

/* Enable the macro to enable proc file to see v3d usage */
#define ENABLE_PROCFS
#ifdef ENABLE_PROCFS
#define MAX_STR_SIZE 			(50)
static struct proc_dir_entry *v3d_proc_file;
static struct semaphore v3d_status_sem;
static void v3d_stat_start_point(void);
static void v3d_stat_end_point(void);
#define V3D_PROC_PRINT_D(str, val) \
	if (v3d_proc_print_info(str, val, 0, 0, &curr, &len, count)) goto err;
#define V3D_PROC_PRINT_X(str, val) \
	if (v3d_proc_print_info(str, val, 0, 1, &curr, &len, count)) goto err;
#define V3D_PROC_PRINT_F(str, val, fract) \
		if (v3d_proc_print_info(str, val, fract, 2, &curr, &len, count)) goto err;
#define V3D_PROC_PRINT_HDR(str) \
	if (v3d_proc_print_info(str, 0, 0, 3, &curr, &len, count)) goto err;
#ifndef MIN
#define MIN(A,B)      ((A) > (B) ? (B) : (A))
#endif

#ifndef MAX
#define MAX(A,B)      ((A) < (B) ? (B) : (A))
#endif

#define V3D_STAT_WINDOW_SIZE	(5)
#define V3D_STAT_GROUP_SIZE		(5)
/* Group stat vars */
typedef struct {
	unsigned int start_sec;
	unsigned int end_sec;
	unsigned int avg_time_spent;
	unsigned int min_time_spent;
	unsigned int max_time_spent;
}v3d_stat_group_t;
static v3d_stat_group_t v3d_stat_groups[V3D_STAT_GROUP_SIZE];
static unsigned int v3d_stat_group_idx = 0;
/* Curr stat vars */
static unsigned int v3d_stat_time_spent_db[V3D_STAT_WINDOW_SIZE];
static unsigned int v3d_stat_time_spent_idx = 0;
/* Temp stat vars */
static unsigned int v3d_stat_sec = 0;
static unsigned int v3d_stat_time_spent_acc;
static unsigned int v3d_stat_start_usec = 0;
#endif

/* Enable the macro to bypass the job que; blocks from post to wait */
// #define V3D_BLOCKING_DRIVER
#ifdef V3D_BLOCKING_DRIVER
struct semaphore v3d_dbg_sem;
#define INIT_ACQUIRE init_MUTEX(&v3d_dbg_sem)
#define ACQUIRE_V3D down(&v3d_dbg_sem)
#define RELEASE_V3D up(&v3d_dbg_sem)
#else
#define INIT_ACQUIRE do {} while (0)
#define ACQUIRE_V3D do {} while (0)
#define RELEASE_V3D do {} while (0)
#endif
/* Enable the macro to enable counting isr events */
// #define DEBUG_V3D_ISR
#ifdef DEBUG_V3D_ISR
static int dbg_int_count = 0;
static int dbg_bin_int_count = 0;
static int dbg_rend_int_count = 0;
static int dbg_oom_int_count = 0;
static int dbg_osm_int_count = 0;
static int dbg_qpu_int_count = 0;
static int dbg_spurious_int_count = 0;
static int dbg_v3d_oom_fatal_cnt = 0;
static int dbg_v3d_oom_blk1_usage_cnt = 0;
static int dbg_v3d_oom_blk2_usage_cnt = 0;
static int dbg_v3d_prev_oom_blk2_cnt = 0;
#endif
/* Debug count variables for job activities */
// #define V3D_STATUS_PRINT_INTERVAL	(10000)
static int dbg_job_post_rend_cnt = 0;
static int dbg_job_post_bin_rend_cnt = 0;
static int dbg_job_post_other_cnt = 0;
static int dbg_job_wait_cnt = 0;
static int dbg_job_timeout_cnt = 0;
#ifdef V3D_STATUS_PRINT_INTERVAL
static int dbg_print_status_loop_cnt = 0;
#endif

/* Dumps the control lists received - Enable only for necessary debug */
// #define DEBUG_DUMP_CL
#ifdef DEBUG_DUMP_CL
#define FILE_DUMP
#define DBG_MAX_NUM_LISTS	(100)
#define DBG_MAX_LIST_SIZE	(4*1024)
#define DBG_DATA_SIZE		(DBG_MAX_NUM_LISTS * DBG_MAX_LIST_SIZE)

typedef struct dbg_list_hdr_t_ {
	int valid;
	int bin_start;
	int bin_size;
	int rend_start;
	int rend_size;
	v3d_job_post_t		job_post;
	v3d_job_status_e 	status;
} dbg_list_hdr_t;

static dbg_list_hdr_t dbg_list_hdr [DBG_MAX_NUM_LISTS];
static unsigned char dbg_data[DBG_DATA_SIZE];
static int dbg_list_idx = 0;
static int dbg_list_init_done = 0;

#endif

#define KLOG_TAG	"v3d_opt.c"
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

static void v3d_job_kill(v3d_job_t *p_v3d_job, v3d_job_status_e job_status);


#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
// Athena B0 V3D APB read back bug workaround
static inline void v3d_clean(void)
{
	iowrite32(0, v3d_base + SCRATCH);
	if (ioread32(v3d_base + SCRATCH))
		iowrite32(0, v3d_base + SCRATCH);
}

void sync_mutex_down(volatile void __iomem *base)
{
	iowrite32(1, base + FLAG0);
	iowrite32(1, base + TURN);

	while ((ioread32(base + FLAG1) == 1) && (ioread32(base + TURN) == 1));
}

void sync_mutex_up(volatile void __iomem *base)
{
	iowrite32(0, base + FLAG0);
}
#endif

static inline u32 v3d_read(u32 reg)
{
	u32 flags;
#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
	sync_mutex_down(sync_flag_base);
#endif
	flags = ioread32(v3d_base + reg);
#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
	v3d_clean();
	sync_mutex_up(sync_flag_base);
#endif
	return flags;
}

static void v3d_power(int flag)
{
	KLOG_D("v3d_power [%d] v3d_inuse[%d]", flag, v3d_in_use);
	if (flag) {
		/* Enable V3D island power */
		clk_enable(gClkPower);
	} else {
		/* Disable V3D island power */
		clk_disable(gClkPower);
	}
}

static void v3d_turn_all_on(void)
{
	KLOG_V("dvfs off, gClkAHB on");
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
	cpufreq_bcm_dvfs_disable(cpufreq_client);
#endif
	/* Check if the V3D power island is already ON. If not ON, switch it on */
	if (!v3d_is_on) {
		v3d_power(1);
		v3d_is_on = 1;
	}

	clk_enable(gClkAHB);
}

static void v3d_turn_all_off(void)
{
	KLOG_V("gClkAHB off, dvfs on");
#ifdef ENABLE_PROCFS
	v3d_stat_end_point();
#endif
	clk_disable(gClkAHB);
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
	cpufreq_bcm_dvfs_enable(cpufreq_client);
#endif
}

#ifdef DEBUG_DUMP_CL
static void dbg_list_init (void)
{
	int i;
	
	for (i=0; i<DBG_MAX_NUM_LISTS; i++) {
		dbg_list_hdr[i].valid = 0;
		memset(&dbg_list_hdr[i].job_post, 0, sizeof(v3d_job_post_t));
	}

	memset(dbg_data, 0, DBG_DATA_SIZE);
	
	dbg_list_init_done = 1;
}

static int dbg_list_get_next_idx(int idx)
{
	int new_idx = idx+1;
	if (new_idx >= DBG_MAX_NUM_LISTS) new_idx = 0;
	return new_idx;
}
/*
static int dbg_list_get_prev_idx(int idx)
{
	int new_idx = idx-1;
	if (new_idx < 0) new_idx = DBG_MAX_NUM_LISTS-1;
	return new_idx;
}
*/
static int dbg_list_get_slot (v3d_job_post_t *p_job_post)
{
	int size;
	
	size = (p_job_post->v3d_ct0ea - p_job_post->v3d_ct0ca);
	size += (p_job_post->v3d_ct1ea - p_job_post->v3d_ct1ca);
	if (size > DBG_MAX_LIST_SIZE) {
		KLOG_E("Skipping storing the CL for job[%d] size[%d]", p_job_post->job_id, size);
		return -1;
	}

	dbg_list_hdr[dbg_list_idx].valid = 0;
	return (dbg_list_idx * DBG_MAX_LIST_SIZE);
}

static void dbg_list_store_job (v3d_job_post_t *p_job_post, int start, unsigned char *bin_addr, unsigned char *rend_addr)
{
	int idx, bin_size, rend_size;
	
	idx = dbg_list_idx;
	
	bin_size = (p_job_post->v3d_ct0ea - p_job_post->v3d_ct0ca);
	rend_size = (p_job_post->v3d_ct1ea - p_job_post->v3d_ct1ca);
	dbg_list_hdr[idx].valid = 1;
	dbg_list_hdr[idx].bin_start = start;
	dbg_list_hdr[idx].bin_size = bin_size;
	dbg_list_hdr[idx].rend_start = (start+bin_size);
	dbg_list_hdr[idx].rend_size = rend_size;
	
	memcpy(&dbg_list_hdr[idx].job_post, p_job_post, sizeof(v3d_job_post_t));
	memcpy(&dbg_data[start], bin_addr, bin_size);
	memcpy(&dbg_data[start+bin_size], rend_addr, rend_size);
}

static int dbg_is_dump_needed(void) {
	if (dbg_list_idx == 0) {
		return 1;
	} else {
		return 0;
	}
}

static int dbg_list_dump_all (void)
{
	static int dbg_file_idx = 0;
	static int dbg_last_job_written = 0;
	int idx, i;
	char fname[64];
	int wr_sz, job_id, cnt;
	unsigned char *addr;

#ifdef 	FILE_DUMP
    mm_segment_t oldfs;
    struct file* dbg_filp = NULL;

    oldfs = get_fs();     
    set_fs (KERNEL_DS); 

	snprintf(fname,64,"/data/dbg_dump/kdbg_list_%d.cl",dbg_file_idx);
	dbg_filp = filp_open(fname, (O_WRONLY|O_TRUNC|O_LARGEFILE|O_CREAT), 0666);
	if (dbg_filp == NULL) {
		KLOG_E( "Failed to create [%s]", fname);
		dbg_filp = NULL;
		return -1;
	}
#endif

	idx = dbg_list_idx;
	job_id = -1;
	cnt = 0;
	for (i=0; i<DBG_MAX_NUM_LISTS; i++) {
		if (dbg_list_hdr[idx].valid) {
			if (dbg_list_hdr[idx].job_post.job_id > dbg_last_job_written) {
#if 0				
				KLOG_E("i[%d] idx[%d] v[%d]: id[%d] b[%d] bs[%d] r[%d] rs[%d]",
					i, idx, dbg_list_hdr[idx].valid, dbg_list_hdr[idx].job_post.job_id, 
					dbg_list_hdr[idx].bin_start, dbg_list_hdr[idx].bin_size, 
					dbg_list_hdr[idx].rend_start, dbg_list_hdr[idx].rend_size);
				KLOG_E("[%p] [%p] [%p] [%p]",
					dbg_list_hdr[idx].job_post.v3d_ct0ca, dbg_list_hdr[idx].job_post.v3d_ct0ea, 
					dbg_list_hdr[idx].job_post.v3d_ct1ca, dbg_list_hdr[idx].job_post.v3d_ct1ea);
#endif
#ifdef 	FILE_DUMP
				if (dbg_filp != NULL) {
					if (dbg_filp->f_op && dbg_filp->f_op->write )
					{
						addr = (void*)&dbg_list_hdr[idx];
						wr_sz = sizeof(dbg_list_hdr_t);
						dbg_filp->f_op->write( dbg_filp, addr, wr_sz, &dbg_filp->f_pos );	 
						
						addr = &dbg_data[dbg_list_hdr[idx].bin_start];
						wr_sz = dbg_list_hdr[idx].bin_size + dbg_list_hdr[idx].rend_size;
						dbg_filp->f_op->write( dbg_filp, addr, wr_sz, &dbg_filp->f_pos );	 
					}
					
				}
#endif
				
				if (job_id == -1) {
					job_id = dbg_list_hdr[idx].job_post.job_id;
				}
				cnt++;
				dbg_last_job_written = dbg_list_hdr[idx].job_post.job_id;
			}
		}			
		idx = dbg_list_get_next_idx(idx);
	}

#ifdef 	FILE_DUMP
	if (dbg_filp != NULL) {
        filp_close( dbg_filp ,NULL );
	}
    set_fs (oldfs); 
	if (job_id != -1) {
		KLOG_E("Dump [%d]jobs from job id[%d] to file[%s]", cnt, job_id, fname);
	}
	dbg_file_idx++;
#endif

	return cnt;
}

static int dbg_list (v3d_job_post_t *p_job_post, unsigned char *bin_addr, unsigned char *rend_addr)
{
	int start;
	
	if (dbg_list_init_done == 0) {
		dbg_list_init();
	}
	
	start = dbg_list_get_slot(p_job_post);
	if (start < 0) {
		return -1;
	}
	
	dbg_list_store_job(p_job_post, start, bin_addr, rend_addr);
	dbg_list_idx = dbg_list_get_next_idx(dbg_list_idx);
	return 0;
}

static int dbg_list_kern(v3d_job_t *p_v3d_job, int caller)
{
	unsigned char *bin_addr, *rend_addr;
	v3d_job_post_t job_post;
	v3d_job_post_t *p_job_post = &job_post;
	
	bin_addr = phys_to_virt(p_v3d_job->v3d_ct0ca);
	rend_addr = phys_to_virt(p_v3d_job->v3d_ct1ca);
	p_job_post->job_type = p_v3d_job->job_type;
	p_job_post->job_id = p_v3d_job->job_id;
	p_job_post->v3d_ct0ca = p_v3d_job->v3d_ct0ca;
	p_job_post->v3d_ct0ea = p_v3d_job->v3d_ct0ea;
	p_job_post->v3d_ct1ca = p_v3d_job->v3d_ct1ca;
	p_job_post->v3d_ct1ea = p_v3d_job->v3d_ct1ea;
	dbg_list (p_job_post, bin_addr, rend_addr);

	return 0;
}
#endif

static irqreturn_t v3d_isr(int irq, void *dev_id)
{
	u32 flags, flags_qpu, tmp;
	int irq_retval = 0;

#ifdef DEBUG_V3D_ISR
	dbg_int_count++;
#endif

/* Read the interrupt status registers */
	flags = v3d_read(INTCTL);
	flags_qpu = v3d_read(DBQITC);
	
/* Clear interrupts isr is going to handle */
	tmp = flags & v3d_read(INTENA);
	iowrite32(tmp, v3d_base + INTCTL);
	if (flags_qpu) {
		iowrite32(flags_qpu, v3d_base + DBQITC);
	}

/* Set the bits in shared var for interrupts to be handled outside 
 * bits 0:rend_done, 1:bin_done, 4:qpu_done, 5:oom_fatal
 */
 	if (v3d_flags != 0) {
		KLOG_E("Assert - v3d_flags not handled, next interrupt has come v3d_flags[0x%x] v3d_flags[0x%x]",
			v3d_flags, (flags & 0x7));
 	}
	v3d_flags = (flags & 0x3) | (flags_qpu ? (1 << 4) : 0);

/* Handle oom case */
	if (flags & (1 << 2)) {
		irq_retval = 1;
		if (v3d_oom_block_used == 0) {
#ifdef DEBUG_V3D_ISR
			dbg_v3d_oom_blk1_usage_cnt++;
#endif
			KLOG_V("v3d oom blk not used: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]", 
				flags, v3d_read(INTCTL), v3d_read(BPOA), v3d_read(BPOS), v3d_read(BPCA), v3d_read(BPCS));
			iowrite32(v3d_bin_oom_block,	v3d_base + BPOA);
			iowrite32(v3d_bin_oom_size,		v3d_base + BPOS);
			v3d_oom_block_used = 1;
			KLOG_V("v3d oom blk 1 given: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]", 
				flags, v3d_read(INTCTL), v3d_read(BPOA), v3d_read(BPOS), v3d_read(BPCA), v3d_read(BPCS));
		}else if (v3d_oom_block_used == 1) {
#ifdef DEBUG_V3D_ISR
			dbg_v3d_oom_blk2_usage_cnt++;
#endif
			KLOG_D("v3d oom blk 1 used: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]", 
				flags, v3d_read(INTCTL), v3d_read(BPOA), v3d_read(BPOS), v3d_read(BPCA), v3d_read(BPCS));
			iowrite32(v3d_bin_oom_block2,	v3d_base + BPOA);
			iowrite32(v3d_bin_oom_size2,	v3d_base + BPOS);
			v3d_oom_block_used = 2;
			KLOG_D("v3d oom blk 2 given: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]", 
				flags, v3d_read(INTCTL), v3d_read(BPOA), v3d_read(BPOS), v3d_read(BPCA), v3d_read(BPCS));
		}else {
#ifdef DEBUG_V3D_ISR
			dbg_v3d_oom_fatal_cnt++;
#endif
			KLOG_E("v3d fatal: oom blk 2 used: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]", 
				flags, v3d_read(INTCTL), v3d_read(BPOA), v3d_read(BPOS), v3d_read(BPCA), v3d_read(BPCS));
			v3d_flags |= (1 << 5);
			iowrite32(1 << 2, v3d_base + INTDIS);
		}
		/* Clear the oom interrupt ? */
		iowrite32(1 << 2, v3d_base + INTCTL);
	}
	
	if (v3d_flags) {
		irq_retval = 1;
		v3d_in_use = 0;
		v3d_oom_block_used = 0;
		if ((v3d_flags != 1) && (v3d_flags != 2)) {
			KLOG_D("v3d isr signal, v3d_flags[0x%x]", v3d_flags);
		}
		wake_up_interruptible(&v3d_isr_done_q);
	}
	
#ifdef DEBUG_V3D_ISR
	if (flags & (1 << 0)) {
		dbg_rend_int_count++;
	}
	if (flags & (1 << 1)) {
		dbg_bin_int_count++;
	}
	if (flags & (1 << 2)) {
		dbg_oom_int_count++;
	}
	if (flags & (1 << 3)) {
		dbg_osm_int_count++;
	}
	if (flags_qpu) {
		dbg_qpu_int_count++;
	}
	if (!(irq_retval)) {
		dbg_spurious_int_count++;
	}
#endif

	return IRQ_RETVAL(irq_retval);
}

#ifdef ENABLE_PROCFS
static void v3d_stat_update(struct timeval *p_tv)
{
	int sec_passed;

	/* Update the sliding window */
	v3d_stat_time_spent_db[v3d_stat_time_spent_idx] = v3d_stat_time_spent_acc;
	v3d_stat_time_spent_idx = (v3d_stat_time_spent_idx + 1) % V3D_STAT_WINDOW_SIZE;
	sec_passed = p_tv->tv_sec - v3d_stat_sec;
	if (sec_passed < 0) {
		sec_passed = -sec_passed;
	}
	v3d_stat_groups[v3d_stat_group_idx].avg_time_spent
		= (v3d_stat_groups[v3d_stat_group_idx].avg_time_spent + v3d_stat_time_spent_acc) >> 1;
	v3d_stat_groups[v3d_stat_group_idx].min_time_spent
		= MIN(v3d_stat_groups[v3d_stat_group_idx].min_time_spent, v3d_stat_time_spent_acc);
	v3d_stat_groups[v3d_stat_group_idx].max_time_spent
		= MAX(v3d_stat_groups[v3d_stat_group_idx].max_time_spent, v3d_stat_time_spent_acc);
	if (sec_passed != 1) {
		/* Update group */
		v3d_stat_groups[v3d_stat_group_idx].end_sec = v3d_stat_sec;
		v3d_stat_group_idx = (v3d_stat_group_idx + 1) % V3D_STAT_GROUP_SIZE;
		v3d_stat_groups[v3d_stat_group_idx].start_sec = p_tv->tv_sec;
		v3d_stat_groups[v3d_stat_group_idx].end_sec = 0;
		v3d_stat_groups[v3d_stat_group_idx].avg_time_spent = 0;
		v3d_stat_groups[v3d_stat_group_idx].min_time_spent = 1000001;
		v3d_stat_groups[v3d_stat_group_idx].max_time_spent = 0;
	}
//	KLOG_D("tv[%d][%d] stat[%d][%d]", (int)p_tv->tv_sec, (int)p_tv->tv_usec, v3d_stat_sec, v3d_stat_time_spent_acc);
}

static void v3d_stat_start_point(void)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	down_interruptible(&v3d_status_sem);
	if (v3d_stat_sec != tv.tv_sec) {
		v3d_stat_update(&tv);
		v3d_stat_sec = tv.tv_sec;
		v3d_stat_time_spent_acc = 0;
	}
	v3d_stat_start_usec = tv.tv_usec;
	up(&v3d_status_sem);
}

static void v3d_stat_end_point(void)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	down_interruptible(&v3d_status_sem);
	if (v3d_stat_sec != tv.tv_sec) {
		v3d_stat_time_spent_acc += 1000000 - v3d_stat_start_usec;
		v3d_stat_update(&tv);
		v3d_stat_sec = tv.tv_sec;
		v3d_stat_time_spent_acc = tv.tv_usec;
	}else {
		v3d_stat_time_spent_acc += tv.tv_usec - v3d_stat_start_usec;
	}
	up(&v3d_status_sem);
}

/*
 * v3d_proc_print_info()
 * Description : Support function used by proc interface to print info on
 *		the proc memory (proc read).
 *		0 - print value in integer format
 *		1 - print value in hex format
 *		2 - print header
 */
static int v3d_proc_print_info(char *str, int value, int fract, int print_type,
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
		str_len = sprintf(*curr,"\t%-30s: %d.%d\n", str, value, fract);
	}
	else if (print_type == 3) {
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
 * v3d_proc_get_status()
 * Description : proc read callback to print the v3d status
 */
static int v3d_proc_get_status(char *page, char **start,
		   off_t off, int count, int *eof, void *data)
{
	char *curr = page;
	int len = 0;
	int i, idx, avg;

	KLOG_V("proc read has come: page[%p], off[%d], count[%d], data[%p] \n",
		page, (int)off, count, data);
	if (off != 0) {
		goto err;
	}

	down_interruptible(&v3d_status_sem);
	V3D_PROC_PRINT_HDR("Current Usage Info");
	avg = 0;
	idx = v3d_stat_time_spent_idx;
	for (i=0; i<V3D_STAT_WINDOW_SIZE; i++) {
		idx = (idx + 1) % V3D_STAT_WINDOW_SIZE;
		V3D_PROC_PRINT_D("V3D usage", v3d_stat_time_spent_db[i]);
		avg += v3d_stat_time_spent_db[i];
	}
	avg = avg/V3D_STAT_WINDOW_SIZE;
	V3D_PROC_PRINT_D("Average V3D usage", avg);

	V3D_PROC_PRINT_HDR("Group Usage Info");
	idx = v3d_stat_group_idx;
	for (i=0; i<V3D_STAT_GROUP_SIZE; i++) {
		idx = (idx + 1) % V3D_STAT_GROUP_SIZE;
		V3D_PROC_PRINT_HDR("Group");
		V3D_PROC_PRINT_D("Start Sec", v3d_stat_groups[idx].start_sec);
		V3D_PROC_PRINT_D("End Sec", v3d_stat_groups[idx].end_sec);
		V3D_PROC_PRINT_D("Average", v3d_stat_groups[idx].avg_time_spent);
		V3D_PROC_PRINT_D("Min", v3d_stat_groups[idx].min_time_spent);
		V3D_PROC_PRINT_D("Max", v3d_stat_groups[idx].max_time_spent);
	}
	up(&v3d_status_sem);

err:
	if (start) {
		*start = page;
	}
	if (eof) {
		*eof = 1;
	}
	return (len < count) ? len : count;
}

static int v3d_parse_string(const char *inputStr, u32 *opCode, u32 *arg)
{
	int numArg;
	char tempStr[MAX_STR_SIZE];

	*opCode = 0;

	numArg = sscanf(inputStr, "%s%u", tempStr, arg);

	if (numArg < 1) {
		return -1;
	}

	if (strcmp(tempStr, "reset_stat") == 0) {
		*opCode = 1;
	}
	else if (strcmp(tempStr, "debug") == 0) {
		*opCode = 2;
	}
	else if (strcmp(tempStr, "stat") == 0) {
		*opCode = 3;
	}

	return 0;
}

/*
 * v3d_proc_set_status()
 * Description : proc write callback for multiple purposes
 *		reset_statistics		- clear the min/max values to start statistics fresh
 *		debug %d 				- set the debug level
 *		stat %d 				- set the statistics level
 */
static int v3d_proc_set_status(struct file *file,
		    const char *buffer, unsigned long count, void *data)
{
	char inputStr[MAX_STR_SIZE];
	int len;
	u32 opCode = 0;
	u32 arg;

	if (count > MAX_STR_SIZE)	len = MAX_STR_SIZE;
	else len = count;
	if (copy_from_user(inputStr, buffer, len))
		return -EFAULT;

	inputStr[MAX_STR_SIZE-3] = '\0';
	inputStr[MAX_STR_SIZE-2] = ' ';
	inputStr[MAX_STR_SIZE-1] = '0';
	v3d_parse_string(inputStr, &opCode, &arg);

	switch (opCode) {
	case 1:
		KLOG_D ("Clearing max/min values in v3d status");
		break;

	case 2:
		KLOG_D ("Setting the debug level to [%d]", arg);
		break;

	case 3:
		KLOG_D ("Setting the statistics level to [%d]", arg);
		break;

	default:
		KLOG_D ("reset_statistics        - clear the min/max values to start statistics fresh");
		KLOG_D ("debug n                 - set the debug level");
		KLOG_D ("   1 - Print on all alloc and free");
		KLOG_D ("stat n                  - set the statistics level");
		KLOG_D ("   1 - Do run-time fragmentation check on all alloc/free");
		break;
	}

	return count;
}
#endif

static void v3d_print_all_jobs(int bp)
{
	v3d_job_t *tmp_job;
	u32 n=0;

	KLOG_V("Job post count rend[%d] bin_rend[%d] other[%d] Job wait count[%d] job timeout count[%d]", 
		dbg_job_post_rend_cnt, dbg_job_post_bin_rend_cnt, dbg_job_post_other_cnt, 
		dbg_job_wait_cnt, dbg_job_timeout_cnt);
	switch (bp) {
		case 0:
			KLOG_V("Job queue Status after post...");
			break;
		case 1:
			KLOG_V("Job queue Status after wait...");
			break;
	}
	if (bp > 1) {
		KLOG_D("head[0x%08x] curr[0x%08x]", (u32)v3d_job_head, (u32)v3d_job_curr);
		tmp_job = v3d_job_head;
		while (tmp_job != NULL ){
			KLOG_D("\t job[%d] : [0x%08x] dev_id[%d] job_id[%d] type[%d] status[%d] intern[%d] wait[%d] retry[%d][%d]",
				n, (u32)tmp_job, tmp_job->dev_id, tmp_job->job_id, tmp_job->job_type,
				tmp_job->job_status, tmp_job->job_intern_state, tmp_job->job_wait_state,
				tmp_job->cache_retry_cnt, tmp_job->retry_cnt);
			tmp_job = tmp_job->next;
			n++;
		}
	}
}

static v3d_job_t *v3d_job_create(struct file *filp, v3d_job_post_t *p_job_post)
{
	v3d_t *dev;
	v3d_job_t *p_v3d_job;
	
	dev = (v3d_t *)(filp->private_data);
	p_v3d_job = kmalloc(sizeof(v3d_job_t), GFP_KERNEL);
	if (!p_v3d_job) {
		KLOG_E("kmalloc failed in v3d_job_post");
		return NULL;
	}
	p_v3d_job->dev_id = dev->id;
	p_v3d_job->job_type = p_job_post->job_type;
	p_v3d_job->job_id = p_job_post->job_id;
	p_v3d_job->v3d_ct0ca = p_job_post->v3d_ct0ca;
	p_v3d_job->v3d_ct0ea = p_job_post->v3d_ct0ea;
	p_v3d_job->v3d_ct1ca = p_job_post->v3d_ct1ca;
	p_v3d_job->v3d_ct1ea = p_job_post->v3d_ct1ea;
	p_v3d_job->v3d_vpm_size = p_job_post->v3d_vpm_size;
	p_v3d_job->job_status = V3D_JOB_STATUS_READY;
	p_v3d_job->job_intern_state = 0;
	p_v3d_job->job_wait_state = 0;
	p_v3d_job->cache_retry_cnt = 0;
	p_v3d_job->retry_cnt = 0;
	p_v3d_job->next = NULL;

	KLOG_V("job[0x%08x] dev_id[%d] job_id[%d] job_type[%d] ct0_ca[0x%x] ct0_ea[0x%x] ct1_ca[0x%x] ct1_ea[0x%x]", 
		(u32)p_v3d_job, dev->id, p_job_post->job_id, p_job_post->job_type, p_job_post->v3d_ct0ca, p_job_post->v3d_ct0ea, 
		p_job_post->v3d_ct1ca, p_job_post->v3d_ct1ea);

	return p_v3d_job;
}

static void v3d_job_add(struct file *filp, v3d_job_t *p_v3d_job, int pos)
{
	v3d_t *dev;
	v3d_job_t *tmp_job;
	
	dev = (v3d_t *)(filp->private_data);
	if (NULL == v3d_job_head) {
		KLOG_V("Adding job[0x%08x] to head[NULL]", (u32)p_v3d_job);
		v3d_job_head = p_v3d_job;
	} else {
		tmp_job = v3d_job_head;
		while (tmp_job->next != NULL ){
			tmp_job = tmp_job->next;
		}
		KLOG_V("Adding job[0x%08x] to tail[0x%08x]", (u32)p_v3d_job, (u32)tmp_job);
		tmp_job->next = p_v3d_job;
	}
}

static v3d_job_t *v3d_job_search(struct file *filp, v3d_job_status_t *p_job_status)
{
	v3d_t *dev;
	v3d_job_t *tmp_job;
	v3d_job_t *last_match_job = NULL;
	
	dev = (v3d_t *)(filp->private_data);
	tmp_job = v3d_job_head;
	while (tmp_job != NULL ){
		if (tmp_job->dev_id == dev->id) { // && (tmp_job->job_id == p_job_status->job_id)
			last_match_job = tmp_job;
		}
		tmp_job = tmp_job->next;
	}

	KLOG_V("Last job to wait for hdl[%d]: [0x%08x]", dev->id, (u32)last_match_job);
	return last_match_job;
}

static void v3d_job_free(struct file *filp, v3d_job_t *p_v3d_wait_job)
{
	v3d_t *dev;
	v3d_job_t *tmp_job, *parent_job;
	v3d_job_t *last_match_job = NULL;
	int curr_job_killed = 0;
	
	dev = (v3d_t *)(filp->private_data);

	KLOG_V("Free upto job[0x%08x] for hdl[%d]: ", (u32)p_v3d_wait_job, dev->id);
	if (p_v3d_wait_job == NULL) {
		KLOG_V("Free upto job[0x%08x] for hdl[%d]: ", (u32)p_v3d_wait_job, dev->id);
	}

	if ((v3d_job_head != NULL) && (v3d_job_head != p_v3d_wait_job) ) {
		parent_job = v3d_job_head;
		tmp_job = v3d_job_head->next;
		while (tmp_job != NULL) {
			if (tmp_job->dev_id == dev->id) {
				last_match_job = tmp_job;
				tmp_job = tmp_job->next;
				parent_job->next = tmp_job;
				if (last_match_job == v3d_job_curr) {
					/* Kill the job, free the job, return error if waiting ?? */				
					KLOG_V("Trying to free current job[0x%08x]", (u32)last_match_job);
					v3d_job_kill((v3d_job_t *)v3d_job_curr, V3D_JOB_STATUS_ERROR);
					curr_job_killed = 1;
					v3d_job_curr = v3d_job_curr->next;
				}
				KLOG_V("Free job[0x%08x] for hdl[%d]: ", (u32)last_match_job, dev->id);
				if (p_v3d_wait_job == NULL) {
					KLOG_V("Free job[0x%08x] for hdl[%d]: ", (u32)last_match_job, dev->id);
				}
				kfree(last_match_job);
				if (last_match_job == p_v3d_wait_job) {
					break;
				}
			} else {
				parent_job = tmp_job;
				tmp_job = tmp_job->next;
			}
		}
	}
	if (v3d_job_head != NULL) {
		if (v3d_job_head->dev_id == dev->id) {
			last_match_job = v3d_job_head;
			if (last_match_job == v3d_job_curr) {
				/* Kill the job, free the job, return error if waiting ?? */				
				KLOG_V("Trying to free current job - head[0x%08x]", (u32)last_match_job);
				v3d_job_kill((v3d_job_t *)v3d_job_curr, V3D_JOB_STATUS_ERROR);
				curr_job_killed = 1;
				v3d_job_curr = v3d_job_curr->next;
			}
			v3d_job_head = v3d_job_head->next;
			KLOG_V("Update head to [0x%08x] and free [0x%08x] for hdl[%d]", 
				(u32)v3d_job_head, (u32)last_match_job, dev->id);
			if (p_v3d_wait_job == NULL) {
				KLOG_V("Update head to [0x%08x] and free [0x%08x] for hdl[%d]", 
					(u32)v3d_job_head, (u32)last_match_job, dev->id);
			}
			kfree(last_match_job);
		}
	}
	if (curr_job_killed) {
		KLOG_D("v3d activity reset as part of freeing jobs for dev_id[%d]", 
			dev->id);
                v3d_turn_all_off();
                wake_up_interruptible(&v3d_isr_done_q);
                v3d_print_all_jobs(0);
	}
}

static int v3d_check_status(int state)
{
	int print_status = 0;
	
	if (((v3d_read(INTCTL) & 0x7) != 0) || (v3d_read(PCS) != 0)) {
		KLOG_E("state[%d] INTCTL[0x%08x] PCS[0x%08x]",
			state, v3d_read(INTCTL), v3d_read(PCS));
		print_status = 1;
	}
#ifdef DEBUG_V3D_ISR
	if (dbg_spurious_int_count || dbg_qpu_int_count) {
		KLOG_E("state[%d] dbg_spurious_int_count[%d] dbg_qpu_int_count[%d]",
			state, dbg_spurious_int_count, dbg_qpu_int_count);
		print_status = 1;
	}
	if (dbg_v3d_prev_oom_blk2_cnt != dbg_v3d_oom_blk2_usage_cnt) {
		dbg_v3d_prev_oom_blk2_cnt = dbg_v3d_oom_blk2_usage_cnt;
		KLOG_E("state[%d] dbg_v3d_prev_oom_blk2_cnt[%d] dbg_v3d_oom_blk2_usage_cnt[%d]",
			state, dbg_v3d_prev_oom_blk2_cnt, dbg_v3d_oom_blk2_usage_cnt);
		print_status = 1;
	}
#endif

	switch (state) {
		case 0:
			{
			}
			break;
			
		case 1:
			{
			}
			break;
			
		case 2:
			{
			}
			break;
				
		case 3:
			{
				if ((v3d_read(BFC) != 1) || (v3d_flags != 2)) {
					KLOG_E("state[%d] BFC[0x%08x] v3d_flags[0x%08x]",
						state, v3d_read(BFC), v3d_flags);
					print_status = 1;
				}
			}
			break;

		case 4:
			{
				if ((v3d_read(RFC) != 1) || (v3d_flags != 1)) {
					KLOG_E("state[%d] RFC[0x%08x] v3d_flags[0x%08x]",
						state, v3d_read(RFC), v3d_flags);
					print_status = 1;
				}
#ifdef V3D_STATUS_PRINT_INTERVAL	
				if ((dbg_print_status_loop_cnt % V3D_STATUS_PRINT_INTERVAL) == 0) {
					KLOG_D("state[%d] dbg_print_status_loop_cnt[%d]",
						state, dbg_print_status_loop_cnt);
					print_status = 1;
				}
				dbg_print_status_loop_cnt++;
#endif				
			}
			break;
	}
	/* Log max of remainig size in binning pool */
	/* CT0CS, CT1CS */
//	return print_status;
	return 0;
}

static void v3d_print_status(void)
{
	KLOG_D("v3d reg: ct0_ca[0x%x] ct0_ea[0x%x] ct1_ca[0x%x] ct1_ea[0x%x]",
		v3d_read(CT0CA), v3d_read(CT0EA), v3d_read(CT1CA), v3d_read(CT1EA));
	KLOG_D("v3d reg: intctl[%x] pcs[%x] bfc[%d] rfc[%d] bpoa[0x%08x] bpos[0x%08x]",
		v3d_read(INTCTL), v3d_read(PCS), v3d_read(BFC), v3d_read(RFC), 
		v3d_read(BPOA), v3d_read(BPOS));
	KLOG_D("v3d reg: ct0cs[0x%08x] ct1cs[0x%08x] bpca[0x%08x] bpcs[0x%08x] \n",
		v3d_read(CT0CS), v3d_read(CT1CS), v3d_read(BPCA), v3d_read(BPCS));
#ifdef DEBUG_V3D_ISR
	KLOG_D("Interrupt count[%d] bin[%d] rend[%d] qpu[%d] other[%d]", 
		dbg_int_count, dbg_bin_int_count, dbg_rend_int_count, 
		dbg_qpu_int_count, dbg_spurious_int_count);
	KLOG_D("oom[%d] osm[%d] oom_blk1[%d] oom_blk2[%d] oom_fatal[%d]", 
		dbg_oom_int_count, dbg_osm_int_count, dbg_v3d_oom_blk1_usage_cnt, 
		dbg_v3d_oom_blk2_usage_cnt, dbg_v3d_oom_fatal_cnt);
#endif
}

static void v3d_reg_init(void) 
{
	iowrite32(2, 					v3d_base + L2CACTL);
	iowrite32(0x8000, 				v3d_base + CT0CS);
	iowrite32(0x8000, 				v3d_base + CT1CS);
	iowrite32(1, 					v3d_base + RFC);
	iowrite32(1, 					v3d_base + BFC);
	iowrite32(0x0f0f0f0f, 			v3d_base + SLCACTL);
	iowrite32(0, 					v3d_base + VPMBASE);
	iowrite32(0, 					v3d_base + VPACNTL);
	iowrite32(v3d_bin_oom_block,	v3d_base + BPOA);
	iowrite32(v3d_bin_oom_size,		v3d_base + BPOS);
	iowrite32(0xF, 					v3d_base + INTCTL);
	iowrite32(0x7, 					v3d_base + INTENA);
}

static void v3d_reg_clr_cache(void)
{
	iowrite32(0x0f0f0f0f, 			v3d_base + SLCACTL);
}

static int v3d_job_start(int turn_on) 
{
	v3d_job_t *p_v3d_job;

	p_v3d_job = (v3d_job_t *)v3d_job_curr;
	
	if (v3d_in_use != 0) {
		KLOG_E("v3d not free for starting job[0x%08x]",	(u32)p_v3d_job);
		v3d_print_all_jobs(2);
		return -1;
	}
	if (p_v3d_job->job_status != V3D_JOB_STATUS_READY) {
		if ((p_v3d_job->job_status != V3D_JOB_STATUS_RUNNING) || (p_v3d_job->job_type != V3D_JOB_BIN_REND) || (p_v3d_job->job_intern_state != 1)) {
			KLOG_E("Status not right for starting job[0x%08x] status[%d] type[%d], intern[%d]", 
				(u32)p_v3d_job, p_v3d_job->job_status, p_v3d_job->job_type, p_v3d_job->job_intern_state);
			v3d_print_all_jobs(2);
			return -1;
		}
	}

	if (turn_on) {
#ifdef ENABLE_PROCFS
		v3d_stat_start_point();
#endif
		v3d_turn_all_on();
	} else {
#ifdef ENABLE_PROCFS
		v3d_stat_end_point();
		v3d_stat_start_point();
#endif
	}
	
	p_v3d_job->job_status = V3D_JOB_STATUS_RUNNING;
	v3d_in_use = 1;
	if ((p_v3d_job->job_type == V3D_JOB_REND) && (p_v3d_job->job_intern_state == 0)) {
		KLOG_V("V3D_JOB_REND RENDERER launching...");
		p_v3d_job->job_intern_state = 2;
		board_sysconfig(SYSCFG_V3D, SYSCFG_INIT);
		v3d_reg_init();
		if (v3d_check_status(0)) {
			v3d_print_status();
		}
		iowrite32(p_v3d_job->v3d_ct1ca, v3d_base + CT1CA);
		iowrite32(p_v3d_job->v3d_ct1ea, v3d_base + CT1EA);
	} else if ((p_v3d_job->job_type == V3D_JOB_BIN_REND) && (p_v3d_job->job_intern_state == 0)) {
		KLOG_V("V3D_JOB_BIN_REND BINNER launching...");
		p_v3d_job->job_intern_state = 1;
		board_sysconfig(SYSCFG_V3D, SYSCFG_INIT);
		v3d_reg_init();
		if (v3d_check_status(1)) {
			v3d_print_status();
		}
		iowrite32(p_v3d_job->v3d_ct0ca, v3d_base + CT0CA);
		iowrite32(p_v3d_job->v3d_ct0ea, v3d_base + CT0EA);
	} else if ((p_v3d_job->job_type == V3D_JOB_BIN_REND) && (p_v3d_job->job_intern_state == 1)) {
		KLOG_V("V3D_JOB_BIN_REND RENDERER launching...");
		p_v3d_job->job_intern_state = 2;
		v3d_reg_clr_cache();
		if (v3d_check_status(2)) {
			v3d_print_status();
		}
		iowrite32(p_v3d_job->v3d_ct1ca, v3d_base + CT1CA);
		iowrite32(p_v3d_job->v3d_ct1ea, v3d_base + CT1EA);
	} else {
		KLOG_E("Invalid internal state for starting job[0x%08x] type[%d] intern[%d]",
			(u32)p_v3d_job, p_v3d_job->job_type, p_v3d_job->job_intern_state);
		v3d_print_all_jobs(2);
		v3d_turn_all_off();
		return -1;
	}
	
	return 0;
}

static void v3d_reset(void)
{
	iowrite32(0x8000, 				v3d_base + CT0CS);
	iowrite32(0x8000, 				v3d_base + CT1CS);
	board_sysconfig(SYSCFG_V3D, SYSCFG_INIT);
	v3d_reg_init();
	v3d_in_use = 0;
	v3d_flags = 0;
	v3d_oom_block_used = 0;
}

static void v3d_job_kill(v3d_job_t *p_v3d_job, v3d_job_status_e job_status)
{
	KLOG_V("Kill job[0x%08x]: ", (u32)p_v3d_job);

	v3d_reset();

	p_v3d_job->job_intern_state = 3;
	p_v3d_job->job_status = job_status;
	if (p_v3d_job->job_wait_state) {
		wake_up_interruptible(&p_v3d_job->v3d_job_done_q);
	}
}

#ifdef V3D_JOB_RETRY_ON_TIMEOUT				
static void v3d_job_reset(v3d_job_t *p_v3d_job)
{
	KLOG_V("Reset job[0x%08x]: ", (u32)p_v3d_job);

	v3d_reset();
	
	p_v3d_job->job_status = V3D_JOB_STATUS_READY;
	p_v3d_job->job_intern_state = 0;
	p_v3d_job->cache_retry_cnt = 0;
}
#endif

static int v3d_thread(void *data)
{
	int ret;
	int timeout;
	static int timout_min = 1000;
	
	KLOG_D("v3d_thread launched");
	if (down_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		do_exit(-1);
	}
	while(1) {
		if (v3d_job_curr == NULL) {
/* No jobs pending - wait till a job gets posted */			
			KLOG_V("v3d_thread going to sleep till a post happens");
			while(v3d_job_curr == NULL) {
				up(&v3d_sem);
				if (wait_event_interruptible(v3d_start_q, (v3d_job_curr != NULL))) {
					KLOG_E("wait interrupted");
					do_exit(-1);
				}
				if (down_interruptible(&v3d_sem)) {
					KLOG_E("lock acquire failed");
					do_exit(-1);
				}
			}
/* Launch the job pointed by v3d_job_curr */			
			KLOG_V("Signal received to launch job");
			ret = v3d_job_start(1);
		} else {
/* Job in progress - wait with timeout for completion */ 		
			KLOG_V("v3d_thread going to sleep till job[0x%08x] status[%d] intern[%d]generates interrupt",
				(u32)v3d_job_curr, v3d_job_curr->job_status, v3d_job_curr->job_intern_state);
			up(&v3d_sem);

			timeout = wait_event_interruptible_timeout(v3d_isr_done_q, (v3d_in_use == 0), msecs_to_jiffies(V3D_ISR_TIMEOUT_IN_MS));
			KLOG_V("wait exit, v3d_in_use[%d], timeout[%d]", v3d_in_use, timeout);
			if (timeout && (timeout < timout_min)) {
				timout_min = timeout;
				KLOG_V("Minimum jiffies before timeout[%d]. Actual timeout set in jiffies[%d]", 
					timout_min, (u32)msecs_to_jiffies(V3D_ISR_TIMEOUT_IN_MS));
			}

			if ((timeout != 0) && (v3d_in_use != 0)) {
				KLOG_E("wait interrupted, v3d_in_use[%d], timeout[%d]", v3d_in_use, timeout);
				do_exit(-1);
			}
			if (down_interruptible(&v3d_sem)) {
				KLOG_E("lock acquire failed");
				do_exit(-1);
			}
			if (v3d_job_curr == NULL) {
				continue;
			}
			if (v3d_in_use == 0) {
/* Job completed or fatal oom happened or current job was killed as part of app close */
				if ((v3d_job_curr->job_type == V3D_JOB_BIN_REND) && (v3d_job_curr->job_intern_state == 1)) {
					if(v3d_flags & (1 << 5)) {
						/* 2 blocks of oom insufficient - kill the job and move ahead */
						KLOG_E("Extra oom blocks also not sufficient for job[0x%08x]", (u32)v3d_job_curr);
						v3d_print_status();
						v3d_flags &= ~(1 << 5);
						v3d_job_kill((v3d_job_t *)v3d_job_curr, V3D_JOB_STATUS_ERROR);
#ifdef DEBUG_DUMP_CL
						if (v3d_job_curr->job_id > 1) {
							dbg_list_kern (v3d_job_curr, 1);
							if (dbg_is_dump_needed()) {
								dbg_list_dump_all();
							}
						}
#endif
						
						v3d_job_curr = v3d_job_curr->next;
						if (v3d_job_curr != NULL) {
							ret = v3d_job_start(0);
						} else {
							v3d_turn_all_off();
						}
					} else {
						/* Binning complete, Initiate rendering */
						if (v3d_check_status(3)) {
							v3d_print_status();
						}
						v3d_flags &= ~(1 << 1);
						ret = v3d_job_start(0);
					}
				} else if ( ((v3d_job_curr->job_type == V3D_JOB_BIN_REND) && (v3d_job_curr->job_intern_state == 2)) ||
				 	((v3d_job_curr->job_type == V3D_JOB_REND) && (v3d_job_curr->job_intern_state == 2)) ){
					/* Rendering (job) complete. Launch next job if available, else sleep till next post */
					if (v3d_check_status(4)) {
						v3d_print_status();
					}
					v3d_flags &= ~(1 << 0);
					v3d_job_curr->job_intern_state = 3;
					v3d_job_curr->job_status = V3D_JOB_STATUS_SUCCESS;
#ifdef DEBUG_DUMP_CL
					if (v3d_job_curr->job_id > 1) {
						dbg_list_kern (v3d_job_curr, 1);
						if (dbg_is_dump_needed()) {
							dbg_list_dump_all();
						}
					}
#endif
					if (v3d_job_curr->job_wait_state) {
						wake_up_interruptible(&(((v3d_job_t *)v3d_job_curr)->v3d_job_done_q));
					}
					v3d_job_curr = v3d_job_curr->next;
					if (v3d_job_curr != NULL) {
						ret = v3d_job_start(0);
					} else {
						v3d_turn_all_off();
					}
                                } else if (v3d_job_curr->job_intern_state == 0) {
                                        ret = v3d_job_start(1);
				} else {
                                        KLOG_E("Assert: v3d thread wait exited as 'done' or 'killed' but job state not valid");
				}
			}else {
/* V3D timed out */
                                if (timeout != 0 ) {
                                        KLOG_E("Assert: v3d thread wait exited as timeout but timeout value[%d] is non-zero", timeout);
                                }
#ifdef V3D_JOB_CACHE_CLEAR
				v3d_reg_clr_cache();
				v3d_job_curr->cache_retry_cnt++;
				if (v3d_job_curr->cache_retry_cnt <= V3D_CACHE_MAX_RETRIES) {
					KLOG_V("Cache Timeout[%d]ms Retry[%d] for job[0x%08x] retry[%d]",
						V3D_ISR_TIMEOUT_IN_MS, v3d_job_curr->cache_retry_cnt, (unsigned int)v3d_job_curr, v3d_job_curr->retry_cnt);
					KLOG_V("v3d reg: ct0_ca[0x%x] ct0_ea[0x%x] ct1_ca[0x%x] ct1_ea[0x%x]",
						v3d_read(CT0CA), v3d_read(CT0EA), v3d_read(CT1CA), v3d_read(CT1EA));
					KLOG_V("v3d reg: intctl[%x] pcs[%x] bfc[%d] rfc[%d] bpoa[0x%08x] bpos[0x%08x]",
						v3d_read(INTCTL), v3d_read(PCS), v3d_read(BFC), v3d_read(RFC),
						v3d_read(BPOA), v3d_read(BPOS));
					KLOG_V("v3d reg: ct0cs[0x%08x] ct1cs[0x%08x] bpca[0x%08x] bpcs[0x%08x] \n",
						v3d_read(CT0CS), v3d_read(CT1CS), v3d_read(BPCA), v3d_read(BPCS));
					continue;
				}
#endif
/* Timeout of job happend */
				dbg_job_timeout_cnt++;
				KLOG_E("wait timed out [%d]ms",	V3D_JOB_TIMEOUT_IN_MS);
#if 0				
				KLOG_D("job[0x%08x] dev_id[%d] job_id[%d] type[%d] status[%d] intern[%d] wait[%d] retry[%d][%d]",
					(u32)v3d_job_curr, v3d_job_curr->dev_id, v3d_job_curr->job_id, v3d_job_curr->job_type, 
					v3d_job_curr->job_status, v3d_job_curr->job_intern_state, v3d_job_curr->job_wait_state, 
					v3d_job_curr->cache_retry_cnt, v3d_job_curr->retry_cnt);
				KLOG_D("job: ct0_ca[0x%x] ct0_ea[0x%x] ct1_ca[0x%x] ct1_ea[0x%x]",
					v3d_job_curr->v3d_ct0ca, v3d_job_curr->v3d_ct0ea, v3d_job_curr->v3d_ct1ca, v3d_job_curr->v3d_ct1ea);
				v3d_print_status();
#endif				
#ifdef V3D_JOB_RETRY_ON_TIMEOUT
				v3d_job_reset((v3d_job_t *)v3d_job_curr);
				v3d_job_curr->retry_cnt++;
				if (v3d_job_curr->retry_cnt <= V3D_JOB_MAX_RETRIES) {
					ret = v3d_job_start(0);
					continue;
				}
#endif
#ifdef DEBUG_DUMP_CL
				if (v3d_job_curr->job_id > 1) {
					dbg_list_kern (v3d_job_curr, 1);
					if (dbg_is_dump_needed()) {
						dbg_list_dump_all();
					}
				}
#endif
				v3d_job_kill((v3d_job_t *)v3d_job_curr, V3D_JOB_STATUS_TIMED_OUT);
				v3d_job_curr = v3d_job_curr->next;
				if (v3d_job_curr != NULL) {
					ret = v3d_job_start(0);
				} else {
					v3d_turn_all_off();
				}
			}
		}
	}
}

static int v3d_job_post(struct file *filp, v3d_job_post_t *p_job_post)
{
	v3d_t *dev;
	int ret = 0;
	v3d_job_t *p_v3d_job = NULL;

	ACQUIRE_V3D;
	dev = (v3d_t *)(filp->private_data);
	if ((p_job_post->job_type != V3D_JOB_REND) && (p_job_post->job_type != V3D_JOB_BIN_REND)) {
		KLOG_E("v3d job type [%d] not supported", p_job_post->job_type);
		v3d_print_all_jobs(2);
		ret = -EINVAL;
		goto err;
	}
/* Allocate new job, copy params from user, init other data */
	if ((p_v3d_job = v3d_job_create(filp, p_job_post)) == NULL) {
		ret = -ENOMEM;
		goto err;
	}

/* Lock the code */	
	if (down_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		ret = -ERESTARTSYS;
		goto err;
	}

/* Add the job to queue */
	v3d_job_add(filp, p_v3d_job, -1);

#ifdef DEBUG_DUMP_CL_
		if (p_v3d_job->job_id > 1) {
			dbg_list_kern (p_v3d_job, 0);
			if (dbg_is_dump_needed()) {
				dbg_list_dump_all();
			}
		}
#endif
	
	if (p_job_post->job_type == V3D_JOB_REND) {
		dbg_job_post_rend_cnt++;
	} else if (p_job_post->job_type == V3D_JOB_BIN_REND) {
		dbg_job_post_bin_rend_cnt++;
	}else {
		dbg_job_post_other_cnt++;
	}
	v3d_print_all_jobs(0);
		
/* Signal if no jobs pending in v3d */
	if (NULL == v3d_job_curr) {
		KLOG_V("Signal to v3d thread about post"); 
		v3d_job_curr = p_v3d_job;
		wake_up_interruptible(&v3d_start_q);
	}

/* Unlock the code */ 
	up(&v3d_sem);

	return ret;

err:	
	if (p_v3d_job) {
		kfree(p_v3d_job);
	}
	RELEASE_V3D;
	return ret;
}

static int v3d_job_wait(struct file *filp, v3d_job_status_t *p_job_status)
{
	v3d_t *dev;
	v3d_job_t *p_v3d_wait_job;

	dev = (v3d_t *)(filp->private_data);

/* Lock the code */ 
	if (down_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		p_job_status->job_status = V3D_JOB_STATUS_ERROR;
		RELEASE_V3D;
		return -ERESTARTSYS;
	}
	
	dbg_job_wait_cnt++;

/* Find the last matching job in the queue if present */
	p_v3d_wait_job = v3d_job_search(filp, p_job_status);

	if (p_v3d_wait_job != NULL) {
/* Wait for the job to complete if not yet complete */
		KLOG_V("Wait ioctl going to sleep for job[0x%08x] dev_id[%d]to complete", (u32)p_v3d_wait_job, p_v3d_wait_job->dev_id);
		init_waitqueue_head(&p_v3d_wait_job->v3d_job_done_q);
		p_v3d_wait_job->job_wait_state = 1;
		while((p_v3d_wait_job->job_status == V3D_JOB_STATUS_READY) 
		  || (p_v3d_wait_job->job_status == V3D_JOB_STATUS_RUNNING)) {
			up(&v3d_sem);
			if (wait_event_interruptible(p_v3d_wait_job->v3d_job_done_q, ((p_v3d_wait_job->job_status != V3D_JOB_STATUS_READY) && (p_v3d_wait_job->job_status != V3D_JOB_STATUS_RUNNING)))) {
				KLOG_E("wait interrupted");
				p_job_status->job_status = V3D_JOB_STATUS_ERROR;
				RELEASE_V3D;
				return -ERESTARTSYS;
			}
			if (down_interruptible(&v3d_sem)) {
				KLOG_E("lock acquire failed");
				p_job_status->job_status = V3D_JOB_STATUS_ERROR;
				RELEASE_V3D;
				return -ERESTARTSYS;
			}
		}

		KLOG_V("Wait ioctl to return status[%d] for job[0x%08x]", 
			p_v3d_wait_job->job_status, (u32)p_v3d_wait_job);
/* Return the status recorded by v3d */
		p_job_status->job_status = p_v3d_wait_job->job_status;

/* Remove all jobs from queue from head till the job (inclusive) on which wait was happening */
		v3d_job_free(filp, p_v3d_wait_job);

	} else {
/* No jobs found matching the dev_id and job_id 
 * Might have got cleaned-up or wait for inexistent post */
		p_job_status->job_status = V3D_JOB_STATUS_NOT_FOUND;
	}

	up(&v3d_sem);
	
	RELEASE_V3D;
	return 0;
}

static int v3d_open(struct inode *inode, struct file *filp)
{
	v3d_t *dev;
	int ret = 0;

	KLOG_V("open");
	
	dev = kmalloc(sizeof(v3d_t), GFP_KERNEL);
	if (!dev) {
		KLOG_E("kmalloc failed in v3d_open");
		return -ENOMEM;
	}

	filp->private_data = dev;

	dev->id = 0;
	if (down_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		ret = -ERESTARTSYS;
		goto err;
	}
	if (v3d_id == 0) {
		KLOG_E("v3d_id has overflowed");
	}
	dev->id = v3d_id++;
	KLOG_V("in open for id[%d]", dev->id);
	up(&v3d_sem);
	/* Initializing the phys_base to avoid dependency on ordering
		of v3d_mempool_base init and v3d module init */
	v3d_mempool_phys_base = virt_to_phys(v3d_mempool_base);

	KLOG_V("v3d id[%d]: pool phys[0x%08x] virt[0x%08x] size[0x%08x]",
		dev->id, v3d_mempool_phys_base, (int)v3d_mempool_base, (int)v3d_mempool_size);
	
err:
	return ret;
}

static int v3d_release(struct inode *inode, struct file *filp)
{
	v3d_t *dev = (v3d_t *)filp->private_data;

	KLOG_V("close: id[%d]", dev->id);
	if (down_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		return -ERESTARTSYS;
	}

	v3d_print_all_jobs(0);
	/* Free all jobs posted using this file */
	v3d_job_free(filp, NULL);
	KLOG_V("after free for id[%d]", dev->id);
	v3d_print_all_jobs(0);
	
	up(&v3d_sem);
	
	if (dev)
		kfree(dev);

	return 0;
}

#define pgprot_cached(prot) \
__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITEBACK)

static int v3d_mmap(struct file *filp, struct vm_area_struct *vma)
{
	v3d_t *dev;
	unsigned long vma_size = vma->vm_end - vma->vm_start;

	dev = (v3d_t *)(filp->private_data);
	KLOG_V ("mmap: id[%d] start[0x%08x], size[0x%08x] pgoff[0x%08x]", 
		dev->id, (u32)vma->vm_start, (int)vma_size, (u32)vma->vm_pgoff);
	
	if (vma_size & (~PAGE_MASK)) {
		KLOG_E("mmaps must be aligned to a multiple of pages_size.");
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	}
	else
	{
		vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);
	}
	if (!vma->vm_pgoff) {
		vma->vm_pgoff = BCM21553_V3D_BASE >> PAGE_SHIFT;
	} else if (vma->vm_pgoff != (v3d_mempool_phys_base >> PAGE_SHIFT)) {
		KLOG_E("mmap vm_pgoff[%d] should be v3d pool base or 0.", (u32)vma->vm_pgoff);
		return -EINVAL;
	}


	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			vma->vm_start,
			vma->vm_pgoff,
			vma_size,
			vma->vm_page_prot)) {
		KLOG_E("remap_pfn_range() failed");
		return -EINVAL;
	}

	return 0;
}

static int v3d_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	v3d_t *dev;
	int ret = 0;

	dev = (v3d_t *)(filp->private_data);
	
	if(_IOC_TYPE(cmd) != BCM_V3D_MAGIC) {
		KLOG_E("_IOC_TYPE mismatch cmd[0x%08x]", cmd);
		return -ENOTTY;
	}

	if(_IOC_NR(cmd) > V3D_CMD_LAST) {
		KLOG_E("_IOC_NR mismatch cmd[0x%08x]", cmd);
		return -ENOTTY;
	}

	if(_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));

	if(_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));

	if(ret) {
		KLOG_E ("dev_id[%d] cmd[0x%08x] is not a valid ioctl", dev->id, cmd);
		return -EFAULT;
	}

	KLOG_V ("dev_id[%d] cmd[0x%08x]", dev->id, cmd);

	switch (cmd) {
	case V3D_IOCTL_POST_JOB:
		{
			v3d_job_post_t job_post;
			
			if (copy_from_user(&job_post, (v3d_job_post_t *)arg, sizeof(job_post))) {
				KLOG_E("V3D_IOCTL_POST_JOB copy_from_user failed");
				ret = -EPERM;
				break;
			}
			ret = v3d_job_post(filp, &job_post);
		}
		break;

	case V3D_IOCTL_WAIT_JOB:
		{
			v3d_job_status_t job_status;
			
			ret = v3d_job_wait(filp, &job_status);
			if (copy_to_user((v3d_job_status_t *)arg, &job_status, sizeof(job_status))) {
				KLOG_E("V3D_IOCTL_WAIT_JOB copy_to_user failed");
				ret = -EPERM;
			}
		}
		break;

	case V3D_IOCTL_GET_MEMPOOL:
		{
			mem_t mempool;
			
			mempool.addr = v3d_mempool_phys_base;
			mempool.ptr = v3d_mempool_base;
			mempool.size = v3d_mempool_size;
			if (copy_to_user((mem_t *)arg, &mempool, sizeof(mem_t)))
				ret = -EPERM;
		}
		break;
#if 0
	case V3D_IOCTL_WAIT_IRQ:
		{
			gl_irq_flags_t usr_irq_flags;
			if (copy_to_user((u32 *)arg, &usr_irq_flags, sizeof(usr_irq_flags))) {
				KLOG_E("V3D_IOCTL_WAIT_IRQ copy_to_user failed");
				ret = -EFAULT;
			}
		}
		break;
#endif
	case V3D_IOCTL_READ_REG:
		{
			u32 reg_addr, reg_value;
#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
			unsigned long flags;
#endif
			if (copy_from_user(&reg_addr, (u32 *)arg, sizeof(reg_addr))) {
				KLOG_E("V3D_IOCTL_READ_REG copy_from_user failed");
				ret = -EFAULT;
				break;
			}

#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
			spin_lock_irqsave(&dev->lock, flags);
#endif
		reg_value = v3d_read(reg_addr);
#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
			spin_unlock_irqrestore(&dev->lock, flags);
#endif

			if (copy_to_user((u32 *)arg, &reg_value, sizeof(reg_value))) {
				KLOG_E("V3D_IOCTL_READ_REG copy_to_user failed");
				ret = -EFAULT;
			}
		}
		break;
#if 0
	case V3D_IOCTL_SOFT_RESET:
		board_sysconfig(SYSCFG_V3D, SYSCFG_INIT);
		break;

	case V3D_IOCTL_TURN_ON:
		v3d_turn_all_on();
		break;

	case V3D_IOCTL_TURN_OFF:
		v3d_turn_all_off();
		break;
#endif
	default:
		KLOG_E ("dev_id[%d] cmd[0x%08x] not supported", dev->id, cmd);
		ret = -EINVAL;
		break;
	}
	
	if (ret) {
		KLOG_E ("ioctl[0x%08x] for dev_id[%d] failed[%d]", cmd, dev->id, ret);
	}
	return ret;
}

static struct file_operations v3d_fops =
{
	.open		= v3d_open,
	.release	= v3d_release,
	.mmap		= v3d_mmap,
	.ioctl		= v3d_ioctl,
};


static int v3d_suspend (struct platform_device *pdev, pm_message_t state)
{
	KLOG_D("v3d_suspend");
	
    /* If the V3D power island is On, switch is OFF*/
    if (v3d_is_on) {
        v3d_power(0);
        v3d_is_on = 0;
    }

    return 0;
}

static int v3d_resume (struct platform_device *pdev)
{
	KLOG_D("v3d_resume");
    return 0;
}

struct platform_driver v3d_driver = {
    .driver =   {
        .name   =   "v3d_dev",
        .owner  =   THIS_MODULE,
    },
    .suspend    =   v3d_suspend,
    .resume     =   v3d_resume,
};

int __init v3d_opt_init(void)
{
	int ret = 0;

	KLOG_V("init");
	
	v3d_major = register_chrdev(0, V3D_DEV_NAME, &v3d_fops);
	if (v3d_major < 0) {
		KLOG_E("Registering v3d device[%s] failed", V3D_DEV_NAME);
		return -EINVAL;
	}

	v3d_class = class_create(THIS_MODULE, V3D_DEV_NAME);
	if (IS_ERR(v3d_class)) {
		KLOG_E("class_create failed");
		unregister_chrdev(v3d_major, V3D_DEV_NAME);
		return PTR_ERR(v3d_class);
	}

	device_create(v3d_class, NULL, MKDEV(v3d_major, 0), NULL, V3D_DEV_NAME);

#ifdef ENABLE_PROCFS
	init_MUTEX(&v3d_status_sem);
#endif

	gClkAHB = clk_get(NULL, BCM_CLK_V3D_STR_ID);
	gClkPower = clk_get(NULL, BCM_CLK_V3D_POWER_STR_ID);
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
	cpufreq_client = cpufreq_bcm_client_get("v3d");
#endif
	v3d_turn_all_on();

	v3d_base = (void __iomem *)ioremap_nocache(BCM21553_V3D_BASE, SZ_64K);
	if (v3d_base == NULL) {
		KLOG_E("mapping v3d registers failed");
		ret = -EINVAL;
		goto err;
	}

#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
	if (sync_flag_base == NULL) {
		iounmap(v3d_base);
		ret = -EINVAL;
		goto err;
	}
#endif

#ifdef CONFIG_BCM21553_V3D_SYNC_ENABLE
	{
		unsigned long flags;
		spin_lock_irqsave(&v3d_id_lock, flags);
		ret = v3d_read(IDENT0);
		spin_unlock_irqrestore(&v3d_id_lock, flags);
	}
#else
	ret = v3d_read(IDENT0);
#endif
	KLOG_D("v3d IDENT0[0x%04x]", ret);

	platform_driver_register(&v3d_driver);
	v3d_turn_all_off();
	
	v3d_dev = kmalloc(sizeof(v3d_t), GFP_KERNEL);
	if (!v3d_dev) {
		KLOG_E("kmalloc failed in v3d_init");
		ret = -ENOMEM;
		goto err;
	}
	v3d_dev->id = 0;

	v3d_bin_oom_cpuaddr = dma_alloc_coherent(NULL, v3d_bin_oom_size, &v3d_bin_oom_block, GFP_ATOMIC | GFP_DMA);
	if (v3d_bin_oom_cpuaddr == NULL) {
		KLOG_E("dma_alloc_coherent failed for v3d oom block size[0x%x]", v3d_bin_oom_size);
		v3d_bin_oom_block = 0;
		v3d_bin_oom_size = 0;
		ret = -ENOMEM;
		goto err;
	}
	KLOG_D("v3d bin oom phys[0x%08x], size[0x%08x] cpuaddr[0x%08x]", 
		v3d_bin_oom_block, v3d_bin_oom_size, (int)v3d_bin_oom_cpuaddr);

	v3d_bin_oom_cpuaddr2 = dma_alloc_coherent(NULL, v3d_bin_oom_size2, &v3d_bin_oom_block2, GFP_ATOMIC | GFP_DMA);
	if (v3d_bin_oom_cpuaddr2 == NULL) {
		KLOG_E("dma_alloc_coherent failed for v3d oom block size[0x%x]", v3d_bin_oom_size2);
		v3d_bin_oom_block2 = 0;
		v3d_bin_oom_size2 = 0;
		ret = -ENOMEM;
		goto err;
	}
	KLOG_D("v3d bin oom2 phys[0x%08x], size[0x%08x] cpuaddr[0x%08x]", 
		v3d_bin_oom_block2, v3d_bin_oom_size2, (int)v3d_bin_oom_cpuaddr2);

	v3d_thread_task = kthread_run(&v3d_thread,v3d_dev,"v3d_thread");
	if ((int)v3d_thread_task == -ENOMEM) {
		KLOG_E("Kernel Thread did not start [0x%08x]", (int)v3d_thread_task);
		ret = -ENOMEM;
		goto err;
	}

	irq_err = request_irq(IRQ_GRAPHICS, v3d_isr,
			IRQF_DISABLED | IRQF_SHARED, V3D_DEV_NAME, v3d_dev);
	if (irq_err) {
		KLOG_E("REgistering isr failed");
		goto err;
	}

	v3d_id = 1;
	v3d_in_use = 0;
	init_MUTEX(&v3d_sem);
	INIT_ACQUIRE;
	init_waitqueue_head(&v3d_isr_done_q);
	init_waitqueue_head(&v3d_start_q);
	v3d_job_head = NULL;
	v3d_job_curr = NULL;
	
#ifdef ENABLE_PROCFS
	v3d_proc_file = create_proc_entry(V3D_DEV_NAME, 0644, NULL);
	if (v3d_proc_file) {
		v3d_proc_file->data = NULL;
		v3d_proc_file->read_proc = v3d_proc_get_status;
		v3d_proc_file->write_proc = v3d_proc_set_status;
		// v3d_proc_file->owner = THIS_MODULE;
	}
	else {
		KLOG_E("Failed creating proc entry");
	}
#endif
	KLOG_V("v3d module init over");
	
	return 0;

err:
	KLOG_E("v3d driver error exit");
	if (irq_err == 0)
		free_irq(IRQ_GRAPHICS, v3d_dev);
	if ((int)v3d_thread_task != -ENOMEM)
		kthread_stop(v3d_thread_task);
	if (v3d_bin_oom_cpuaddr2)
		dma_free_coherent(NULL, v3d_bin_oom_size2, v3d_bin_oom_cpuaddr2, v3d_bin_oom_block2);
	if (v3d_bin_oom_cpuaddr)
		dma_free_coherent(NULL, v3d_bin_oom_size, v3d_bin_oom_cpuaddr, v3d_bin_oom_block);
	if (v3d_dev)
		kfree(v3d_dev);
	if (v3d_base)
		iounmap(v3d_base);
	v3d_turn_all_off();
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
	cpufreq_bcm_client_put(cpufreq_client);
#endif

    platform_driver_unregister(&v3d_driver);

	device_destroy(v3d_class, MKDEV(v3d_major, 0));
	class_destroy(v3d_class);
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
	
	return ret;
}

void __exit v3d_opt_exit(void)
{
	KLOG_E("v3d driver exit");
	if (irq_err == 0)
		free_irq(IRQ_GRAPHICS, v3d_dev);
	if ((int)v3d_thread_task != -ENOMEM)
		kthread_stop(v3d_thread_task);
	if (v3d_bin_oom_cpuaddr2)
		dma_free_coherent(NULL, v3d_bin_oom_size2, v3d_bin_oom_cpuaddr2, v3d_bin_oom_block2);
	if (v3d_bin_oom_cpuaddr)
		dma_free_coherent(NULL, v3d_bin_oom_size, v3d_bin_oom_cpuaddr, v3d_bin_oom_block);
	if (v3d_dev)
		kfree(v3d_dev);
	if (v3d_base)
		iounmap(v3d_base);
	v3d_turn_all_off();
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
	cpufreq_bcm_client_put(cpufreq_client);
#endif
    platform_driver_unregister(&v3d_driver);
	device_destroy(v3d_class, MKDEV(v3d_major, 0));
	class_destroy(v3d_class);
	if (v3d_major >= 0)
		unregister_chrdev(v3d_major, V3D_DEV_NAME);
}

module_init(v3d_opt_init);
module_exit(v3d_opt_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
