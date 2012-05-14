/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <linux/delay.h>
#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/bcm_rpc.h>
#include <linux/broadcom/bcm_reset_utils.h>
#include "mobcom_types.h"
#include "rpc_global.h"

#include "resultcode.h"
#include "taskmsgs.h"
#include "ipcinterface.h"
#include "ipcproperties.h"

#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_internal_api.h"
#include "rpc_debug.h"
#include "rpc_sync_api.h"


static int __init bcm_fuse_rpc_init_module(void);
static void __exit bcm_fuse_rpc_exit_module(void);

static struct class *rpc_class;

// maximum number of times we will check for CP RPC
// to be ready
#define MAX_ITERATIONS_CP_READY_CHECK   100
// time to sleep (ms) between iterations of checking
// for CP RPC to be ready
#define CP_READY_CHECK_SLEEP_MS         20

/***************************************************************************/
/**
 *  Called by Linux power management system when AP enters and exits sleep.
 *  Api will notify CP of current AP sleep state, and CP will suspend
 *  unnecessary CP->AP notifications to avoid waking AP.
 *
 *  @param  inSleep (in)   TRUE if AP entering deep sleep, FALSE if exiting
 */
void BcmRpc_SetApSleep( bool inSleep )
{
    RPC_SetProperty( RPC_PROP_AP_IN_DEEPSLEEP, (inSleep?1:0) );
}

static int rpc_open(struct inode *inode, struct file *filp)
{

    return 0;
}

static int rpc_release(struct inode *inode, struct file *filp)
{

    return 0;
}

int rpc_read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
    int rc = 0;

    return rc;
}

int rpc_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
    int rc = 0;

    return rc;
}

static long rpc_ioctl(struct file *filp, unsigned int cmd, UInt32 arg)
{
    int rc = 0;
    return(rc);
}


static struct file_operations rpc_ops = 
{
	.owner = THIS_MODULE,
	.open  = rpc_open,
	.read  = rpc_read,
	.write = rpc_write,
	.unlocked_ioctl = rpc_ioctl,
	.mmap	 = NULL,
	.release = rpc_release,
};


/****************************************************************************
*
*  RPC_Init(void);
*
*  Register RPC module.
*
***************************************************************************/
int RPC_Init(void)
{
    UInt32 cpReady, sleepCount;

    if(RPC_SYS_Init(NULL) == RESULT_ERROR)
    {
        RPC_DEBUG(DBG_ERROR, "RPC device init fail...!\n");
        return -1;
    }
	if(RPC_SYS_EndPointRegister(RPC_APPS) == RPC_RESULT_ERROR)
    {
        RPC_DEBUG(DBG_ERROR, "registering the RPC device fail...!\n");
        return -1;
    }

    // initialize the synchronous RPC interface
	if( RPC_SyncInitialize() != RESULT_OK )
	{
        RPC_DEBUG(DBG_ERROR, "RPC_SyncInitialize fail...!\n");
        return -1;
	}

    // check for AP only boot mode
    if ( AP_ONLY_BOOT == get_ap_boot_mode() )
    {
        // so RPC module completes init without error
        RPC_DEBUG(DBG_INFO, "RPC_Init: AP only boot, not waiting for CP\n" );
        cpReady = 1;
    }
    else
    {
        // not AP only mode, so check for CP RPC being ready before continuing...
    sleepCount = 0;
    do
    {
	    RPC_GetProperty(RPC_PROP_CP_TASKMSGS_READY, &cpReady);
	    if ( !cpReady )
	    {
            msleep(CP_READY_CHECK_SLEEP_MS);
            sleepCount++;
	}
	} while ( !cpReady && (sleepCount < MAX_ITERATIONS_CP_READY_CHECK));

    RPC_DEBUG(DBG_INFO, "RPC_Init: cpReady:%d sleepcount:%d sleeptime:%dms\n", (int)cpReady, (int)sleepCount, (int)(sleepCount*CP_READY_CHECK_SLEEP_MS) );
    }

    return (cpReady?0:-1);
}

/****************************************************************************
*
*  bcm_fuse_rpc_init_module(void);
*
*  Init module.
*
***************************************************************************/
static int __init bcm_fuse_rpc_init_module(void)
{
    int ret = 0;

    pr_info("RPC Support 1.00 (BUILD TIME "__DATE__" "__TIME__")\n" );

    if (( ret = register_chrdev( BCM_RPC_MAJOR, "bcm_rpc", &rpc_ops )) < 0 )
    {
        RPC_DEBUG(DBG_ERROR, "rpc: register_chrdev failed for major %d\n", BCM_RPC_MAJOR);
        goto out;
    }

    rpc_class = class_create(THIS_MODULE, "bcm_rpc");
    if (IS_ERR(rpc_class)) {
        return PTR_ERR(rpc_class);
    }

    device_create(rpc_class, NULL, MKDEV(BCM_RPC_MAJOR, 0),NULL, "bcm_rpc");

    RPC_DEBUG(DBG_INFO, "%s driver(major %d) installed.\n", "bcm_rpc", BCM_RPC_MAJOR);

    /** Init RPC Driver */
    ret = RPC_Init();

    if (ret) 
    {
        RPC_DEBUG(DBG_ERROR, "RPC_Init fail...!\n");
    }

out:
    return ret;
}

/****************************************************************************
*
*  bcm_fuse_rpc_exit_module(void);
*
*  Exit module.
*
***************************************************************************/
static void __exit bcm_fuse_rpc_exit_module(void)
{

    return;
}

#define PERM_KMEM_NUM 2
#define MEM_SIZE_THRESHOLD 16*1024
#define MAX_PERM_MEM MEM_SIZE_THRESHOLD*4
static void* perm_pkmem[PERM_KMEM_NUM];
static atomic_t perm_pkmem_flag[PERM_KMEM_NUM];

void *rpc_mem_alloc(size_t size)
{
    int i;

    if(perm_pkmem[0] == NULL){
        for(i=0;i<PERM_KMEM_NUM;i++){
            perm_pkmem[i] = kmalloc(MAX_PERM_MEM, GFP_KERNEL);
            if(perm_pkmem[i] == NULL){
                RPC_DEBUG(DBG_ERROR, "rpc_mem_alloc: Failed to initialize array of perm_pkmem, slot: %d\n",i);
                atomic_set(&perm_pkmem_flag[i],0);
                continue;
            }
            atomic_set(&perm_pkmem_flag[i],1);
            RPC_DEBUG(DBG_INFO, "rpc_mem_alloc: successfully preserved array of perm_pkmem, slot: %d\n",i);
        }
    }

    if(size <= MEM_SIZE_THRESHOLD)
        return(kmalloc(size, GFP_KERNEL));

    for(i=0;i<PERM_KMEM_NUM;i++){
        if(atomic_cmpxchg(&perm_pkmem_flag[i],1,0)){
            RPC_DEBUG(DBG_INFO, "rpc_mem_alloc: Allocated a block from array of perm_pkmem, size: %d slot: %d\n",size,i);
            return(perm_pkmem[i]);
        }
    }

    RPC_DEBUG(DBG_INFO, "rpc_mem_alloc: Cannot allocate a block from array of perm_pkmem, size: %d\n",size);

    return(kmalloc(size, GFP_KERNEL));
}

void rpc_mem_free(void *p)
{
    int i;

    for(i=0;i<PERM_KMEM_NUM;i++){
        if(p == perm_pkmem[i]){
            atomic_set(&perm_pkmem_flag[i],1);
            RPC_DEBUG(DBG_INFO, "rpc_mem_free: Free a block to array of perm_pkmem, slot: %d\n",i);
            return;
        }
    }
    kfree(p);
}

module_init(bcm_fuse_rpc_init_module);
module_exit(bcm_fuse_rpc_exit_module);
