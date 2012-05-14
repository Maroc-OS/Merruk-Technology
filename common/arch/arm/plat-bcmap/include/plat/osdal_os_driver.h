/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/plat-bcmap/include/plat/osdal_os_driver.h
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
*   @file   osdal_os_driver.h
*
*   @brief  For DMA driver. Use the standard abstraction.
*
****************************************************************************/

#ifndef OSDAL_OS_DRIVER_H
#define OSDAL_OS_DRIVER_H

#define __ATHENA__
#include <plat/osabstract/ossemaphore.h>
#include <plat/osabstract/ostypes.h>
#include <plat/osabstract/ostask.h>
#include <plat/osabstract/osqueue.h>
#define FUSE_APPS_PROCESSOR
#include "osdal_os.h"		/* abstract os types */
#if defined(_ATHENA_) && defined(FUSE_APPS_PROCESSOR)
#include "osdal_os_service.h"	/* abstract drivers */
#else
#define OSDAL_CLK_ENABLE_DMAC  0	/* dummy */
#define OSDAL_CLK_Enable(a)
#define OSDAL_CLK_Disable(a)
#endif

/* OS Task */
#define OSDAL_TASK_CREATE(e,n,p,s)      OSTASK_Create( (TEntry_t)e,(TName_t)n,(TPriority_t)p,(TStackSize_t)s )
#define OSDAL_TASK_DELETE(t)            OSTASK_Destroy( (Task_t)t )

/* OS Semaphore */
#define OSDAL_SEMA_CREATE(c,m)          OSSEMAPHORE_Create( (SCount_t)c,(OSSuspend_t)m )
#define OSDAL_SEMA_DELETE(s)            OSSEMAPHORE_Destroy( (Semaphore_t)s )
#define OSDAL_SEMA_COUNT(s)             OSSEMAPHORE_GetCnt( (Semaphore_t)s )
#define OSDAL_SEMA_NAMECHANGE(s,n)      OSSEMAPHORE_ChangeName( (Semaphore_t)s,(const char*)n )

/* OS INTERRUPT */
#define OSDAL_IRQ_CREATE(e,n,p,s)       OSINTERRUPT_Create( (IEntry_t)e,(IName_t)n,(IPriority_t)p,(IStackSize_t)s )
#define OSDAL_IRQ_DELETE(isr)           OSINTERRUPT_Destroy( (Interrupt_t)isr )
#define OSDAL_IRQ_TRIGGER(isr)          OSINTERRUPT_Trigger( (Interrupt_t)isr )
#define OSDAL_IRQ_REGISTER(id,isr)      IRQ_Register( (InterruptId_t)id, (isr_t)isr )
#define OSDAL_IRQ_DEREGISTER(id)        IRQ_Deregister( (InterruptId_t)id )

#endif
