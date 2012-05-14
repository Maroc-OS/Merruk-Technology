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

#ifndef _GE_DRV_H_
#define _GE_DRV_H_
#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */
#undef PDEBUG			/* undef it, just in case */
#ifdef GE_DEBUG
#  ifdef __KERNEL__
    /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_INFO "x170: " fmt, ## args)
#  else
    /* This one for user space */
#    define PDEBUG(fmt, args...) printf(__FILE__ ":%d: " fmt, __LINE__ , ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)	/* not debugging: nothing */
#endif


#define GE_IOC_MAGIC  'g'
#define GE_IOC_RESERVE           _IO(GE_IOC_MAGIC,  6)
#define GE_IOC_UNRESERVE           _IO(GE_IOC_MAGIC,  7)
#define GE_IOC_WAIT           _IO(GE_IOC_MAGIC,  8)

#define GE_IOC_MAXNR 9


#define	GE_REG_OFFSET			((volatile char*)(pRegBase))

#define V3D_IDENT0		((volatile int*)(GE_REG_OFFSET+	0x0000	))
#define V3D_IDENT1		((volatile int*)(GE_REG_OFFSET+	0x0004	))
#define V3D_IDENT2		((volatile int*)(GE_REG_OFFSET+	0x0008	))
#define V3D_IDENT3		((volatile int*)(GE_REG_OFFSET+	0x000c	))
#define V3D_SCRATCH		((volatile int*)(GE_REG_OFFSET+	0x0010	))
#define V3D_L2CACTL		((volatile int*)(GE_REG_OFFSET+	0x0020	))
#define V3D_SLCACTL		((volatile int*)(GE_REG_OFFSET+	0x0024	))
#define V3D_INTCTL		((volatile int*)(GE_REG_OFFSET+	0x0030	))
#define V3D_INTENA		((volatile int*)(GE_REG_OFFSET+	0x0034	))
#define V3D_INTDIS		((volatile int*)(GE_REG_OFFSET+	0x0038	))
#define V3D_CT0CS		((volatile int*)(GE_REG_OFFSET+	0x0100	))
#define V3D_CT1CS		((volatile int*)(GE_REG_OFFSET+	0x0104	))
#define V3D_CT0EA		((volatile int*)(GE_REG_OFFSET+	0x0108	))
#define V3D_CT1EA		((volatile int*)(GE_REG_OFFSET+	0x010c	))
#define V3D_CT0CA		((volatile int*)(GE_REG_OFFSET+	0x0110	))
#define V3D_CT1CA		((volatile int*)(GE_REG_OFFSET+	0x0114	))
#define V3D_CT00RA0		((volatile int*)(GE_REG_OFFSET+	0x0118	))
#define V3D_CT01RA0		((volatile int*)(GE_REG_OFFSET+	0x011c	))
#define V3D_CT0LC		((volatile int*)(GE_REG_OFFSET+	0x0120	))
#define V3D_CT1LC		((volatile int*)(GE_REG_OFFSET+	0x0124	))
#define V3D_CT0PC		((volatile int*)(GE_REG_OFFSET+	0x0128	))
#define V3D_CT1PC		((volatile int*)(GE_REG_OFFSET+	0x012c	))
#define V3D_PCS			((volatile int*)(GE_REG_OFFSET+	0x0130	))
#define V3D_BFC			((volatile int*)(GE_REG_OFFSET+	0x0134	))
#define V3D_RFC			((volatile int*)(GE_REG_OFFSET+	0x0138	))
#define V3D_BPCA		((volatile int*)(GE_REG_OFFSET+	0x0300	))
#define V3D_BPCS		((volatile int*)(GE_REG_OFFSET+	0x0304	))
#define V3D_BPOA		((volatile int*)(GE_REG_OFFSET+	0x0308	))
#define V3D_BPOS		((volatile int*)(GE_REG_OFFSET+	0x030c	))
#define V3D_BXCF		((volatile int*)(GE_REG_OFFSET+	0x0310	))
#define V3D_SQRSV0		((volatile int*)(GE_REG_OFFSET+	0x0410	))
#define V3D_SQRSV1		((volatile int*)(GE_REG_OFFSET+	0x0414	))
#define V3D_SQCNTL		((volatile int*)(GE_REG_OFFSET+	0x0418	))
#define V3D_SQCSTAT		((volatile int*)(GE_REG_OFFSET+	0x041c	))
#define V3D_SRQPC		((volatile int*)(GE_REG_OFFSET+	0x0430	))
#define V3D_SRQUA		((volatile int*)(GE_REG_OFFSET+	0x0434	))
#define V3D_SRQUL		((volatile int*)(GE_REG_OFFSET+	0x0438	))
#define V3D_SRQCS		((volatile int*)(GE_REG_OFFSET+	0x043c	))
#define V3D_VPACNTL		((volatile int*)(GE_REG_OFFSET+	0x0500	))
#define V3D_VPMBASE		((volatile int*)(GE_REG_OFFSET+	0x0504	))
#define V3D_PCTRC		((volatile int*)(GE_REG_OFFSET+	0x0670	))
#define V3D_PCTRE		((volatile int*)(GE_REG_OFFSET+	0x0674	))
#define V3D_PCTR0		((volatile int*)(GE_REG_OFFSET+	0x0680	))
#define V3D_PCTRS0		((volatile int*)(GE_REG_OFFSET+	0x0684	))
#define V3D_PCTR1		((volatile int*)(GE_REG_OFFSET+	0x0688	))
#define V3D_PCTRS1		((volatile int*)(GE_REG_OFFSET+	0x068c	))
#define V3D_PCTR2		((volatile int*)(GE_REG_OFFSET+	0x0690	))
#define V3D_PCTRS2		((volatile int*)(GE_REG_OFFSET+	0x0694	))
#define V3D_PCTR3		((volatile int*)(GE_REG_OFFSET+	0x0698	))
#define V3D_PCTRS3		((volatile int*)(GE_REG_OFFSET+	0x069c	))
#define V3D_PCTR4		((volatile int*)(GE_REG_OFFSET+	0x06a0	))
#define V3D_PCTRS4		((volatile int*)(GE_REG_OFFSET+	0x06a4	))
#define V3D_PCTR5		((volatile int*)(GE_REG_OFFSET+	0x06a8	))
#define V3D_PCTRS5		((volatile int*)(GE_REG_OFFSET+	0x06ac	))
#define V3D_PCTR6		((volatile int*)(GE_REG_OFFSET+	0x06b0	))
#define V3D_PCTRS6		((volatile int*)(GE_REG_OFFSET+	0x06b4	))
#define V3D_PCTR7		((volatile int*)(GE_REG_OFFSET+	0x06b8	))
#define V3D_PCTRS7		((volatile int*)(GE_REG_OFFSET+	0x06bc	))
#define V3D_PCTR8		((volatile int*)(GE_REG_OFFSET+	0x06c0	))
#define V3D_PCTRS8		((volatile int*)(GE_REG_OFFSET+	0x06c4	))
#define V3D_PCTR9		((volatile int*)(GE_REG_OFFSET+	0x06c8	))
#define V3D_PCTRS9		((volatile int*)(GE_REG_OFFSET+	0x06cc	))
#define V3D_PCTR10		((volatile int*)(GE_REG_OFFSET+	0x06d0	))
#define V3D_PCTRS10		((volatile int*)(GE_REG_OFFSET+	0x06d4	))
#define V3D_PCTR11		((volatile int*)(GE_REG_OFFSET+	0x06d8	))
#define V3D_PCTRS11		((volatile int*)(GE_REG_OFFSET+	0x06dc	))
#define V3D_PCTR12		((volatile int*)(GE_REG_OFFSET+	0x06e0	))
#define V3D_PCTRS12		((volatile int*)(GE_REG_OFFSET+	0x06e4	))
#define V3D_PCTR13		((volatile int*)(GE_REG_OFFSET+	0x06e8	))
#define V3D_PCTRS13		((volatile int*)(GE_REG_OFFSET+	0x06ec	))
#define V3D_PCTR14		((volatile int*)(GE_REG_OFFSET+	0x06f0	))
#define V3D_PCTRS14		((volatile int*)(GE_REG_OFFSET+	0x06f4	))
#define V3D_PCTR15		((volatile int*)(GE_REG_OFFSET+	0x06f8	))
#define V3D_PCTRS15		((volatile int*)(GE_REG_OFFSET+	0x06fc	))
#define V3D_DBCFG		((volatile int*)(GE_REG_OFFSET+	0x0e00	))
#define V3D_DBSCS		((volatile int*)(GE_REG_OFFSET+	0x0e04	))
#define V3D_DBSCFG		((volatile int*)(GE_REG_OFFSET+	0x0e08	))
#define V3D_DBSSR		((volatile int*)(GE_REG_OFFSET+	0x0e0c	))
#define V3D_DBSDR0		((volatile int*)(GE_REG_OFFSET+	0x0e10	))
#define V3D_DBSDR1		((volatile int*)(GE_REG_OFFSET+	0x0e14	))
#define V3D_DBSDR2		((volatile int*)(GE_REG_OFFSET+	0x0e18	))
#define V3D_DBSDR3		((volatile int*)(GE_REG_OFFSET+	0x0e1c	))
#define V3D_DBQRUN		((volatile int*)(GE_REG_OFFSET+	0x0e20	))
#define V3D_DBQHLT		((volatile int*)(GE_REG_OFFSET+	0x0e24	))
#define V3D_DBQSTP		((volatile int*)(GE_REG_OFFSET+	0x0e28	))
#define V3D_DBQITE		((volatile int*)(GE_REG_OFFSET+	0x0e2c	))
#define V3D_DBQITC		((volatile int*)(GE_REG_OFFSET+	0x0e30	))
#define V3D_DBQGHC		((volatile int*)(GE_REG_OFFSET+	0x0e34	))
#define V3D_DBQGHG		((volatile int*)(GE_REG_OFFSET+	0x0e38	))
#define V3D_DBQGHH		((volatile int*)(GE_REG_OFFSET+	0x0e3c	))
#define V3D_DBGE		((volatile int*)(GE_REG_OFFSET+	0x0f00	))
#define V3D_FDBGO		((volatile int*)(GE_REG_OFFSET+	0x0f04	))
#define V3D_FDBGB		((volatile int*)(GE_REG_OFFSET+	0x0f08	))
#define V3D_FDBGR		((volatile int*)(GE_REG_OFFSET+	0x0f0c	))
#define V3D_FDBGS		((volatile int*)(GE_REG_OFFSET+	0x0f10	))
#define V3D_ERRSTAT		((volatile int*)(GE_REG_OFFSET+	0x0f20	))


#endif /* !_GE_H_ */
