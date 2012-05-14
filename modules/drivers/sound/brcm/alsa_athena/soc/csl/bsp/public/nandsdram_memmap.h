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
//		name				address			size
//		================	==========		==========
#define LOG_MVA_BASE		0x0A000000					// 4M logical memory of protected MVA
#define LOG_MVA_SIZE						0x00300000  // 3MB, down from 4MB to reduce the size of MMU tables
//#define MMU_L1L2_MVA_BASE   0x0B000000

#if (!defined (_HERA_)) && (!defined (_RHEA_)) && (!defined(_SAMOA_))

#define ITCM_MVA_BASE       0x00000000
// size for a virtual ITCM page: 1 MMU section = 1MB
#if defined(DISABLE_PAGED_ITCM)
#define ITCM_PAGE1_MVA      (ITCM_MVA_BASE+ITCM_PAGE_SIZE)
#else
#define ITCM_VPAGE_SIZE     (1024*1024)
#define ITCM_PAGE1_MVA      (ITCM_MVA_BASE+1*ITCM_VPAGE_SIZE+ITCM_PAGE_SIZE)
#define ITCM_PAGE2_MVA      (ITCM_MVA_BASE+2*ITCM_VPAGE_SIZE+ITCM_PAGE_SIZE)
#define ITCM_PAGE3_MVA      (ITCM_MVA_BASE+3*ITCM_VPAGE_SIZE+ITCM_PAGE_SIZE)
#endif

#define DSP_SH_BASE			0x80000000					// 320k DSP sharedmem
#define DSP_SH_SIZE			0x00050000
#define CP_SH_BASE			DSP_SH_BASE					
#define CP_SH_SIZE			0x00020000					// 128k CP/DSP sharedmem
#define AP_SH_BASE			(DSP_SH_BASE+CP_SH_SIZE)	
#define AP_SH_SIZE			0x00010000					// 64k  AP/DSP sharedmem
// for compatibility with existing code
#define SHARED_RAM_BASE		DSP_SH_BASE

#define DSP_PROM_BASE		0x80100000					// 64k DSP PROM patch
#define DSP_PROM_SIZE						0x00010000
#define DSP_DROM_BASE		0x80110000					// 64k DSP DROM patch
#define DSP_DROM_SIZE						0x00010000
#define PARM_IND_BASE		0x80120000					// 128k independent sysparm
#define PARM_IND_SIZE						0x00020000
#define PARM_DEP_BASE		0x80140000					// 64k dependent sysparm
#define PARM_DEP_SIZE						0x00010000

#define PARM_SPML_IND_BASE		0x801E0000					// 64k hybrid SPML
#define PARM_SPML_IND_SIZE						0x00010000
#define PARM_SPML_DEP_BASE		0x801F0000					// 64k hybrid SPML (Address is verified on HERA... need to check for Athena)
#define PARM_SPML_DEP_SIZE						0x00010000

#define MMU_L1L2_SIZE						0x00020000
#define UMTS_PI_BASE		0x80200000					// Spinner PI
#define UMTS_PI_SIZE						0x000B0000
#define UMTS_PS_BASE		0x802B0000					// Spinner PS
#define UMTS_PS_SIZE						0x0001F000
//#define UMTS_CAL_BASE		0x802CF000					// Spinner CAL
//#define UMTS_CAL_SIZE						0x00001000
#define UMTS_CAL_BASE		0x82160000					// Spinner CAL
#if defined(_RFBIN_64K_)
#define UMTS_CAL_SIZE						0x00010000
#define SL1_MEM_ADDR		0x802E0000					// Cacheable Router Shared Memory
#define SL1_MEM_SIZE						0x00017000
#define SL1_NCMEM_ADDR		0x802F7000					// Non-cacheable Router Shared Memory
#else
#define UMTS_CAL_SIZE						0x00003000
#define SL1_MEM_ADDR		0x802F0000					// Cacheable Router Shared Memory
#define SL1_MEM_SIZE						0x0000A000
#define SL1_NCMEM_ADDR		0x802FA000					// Non-cacheable Router Shared Memory
#endif
#define SL1_NCMEM_SIZE						0x00004000

#define DSP_PRAM_BASE		0x80160000					// 7pages*64k DSP PRAM load;;For TL3:256K for TL3 PRAM, 256K for TL3 DRAM
#define DSP_PRAM_SIZE						0x00080000
#if defined (ATHENARAY)
#define DSP_DRAM_BASE       0x81800000     
#define DSP_DRAM_SIZE       0x00200000
#else
#define DSP_DRAM_BASE       0x80100000     // 32k+16K DSP DRAM load
#define DSP_DRAM_SIZE       0x00010000
#endif


#define TL3_DSP_EXT_BASE	DSP_DRAM_BASE					// 2M TL3 Ext_PRAM
#define TL3_DSP_EXT_SIZE	DSP_DRAM_SIZE

#ifdef DORMANT_ENABLE
// if the Dormant mode is enabedl in the system assign some memory
// to be used to save the processor context ITCM and DTCM
#define DORMANT_TEST_AREA		0x802d0000
#define DORMANT_TEST_SIZE						0x00001000	//  Dormant Mode Save Restore area	
#define DORMANT_ITCM_AREA		0x802d1000
#define DORMANT_ITCM_SIZE						0x00008000
#define DORMANT_DTCM_AREA		0x802D9000
#define DORMANT_DTCM_SIZE						0x00004000
#endif

#if defined(INCLUDE_BSP_TESTING)
#define BSP_TEST_BASE		0x81000000
#define BSP_TEST_SIZE						0x01000000
#endif	// INCLUDE_BSP_TESTING

#ifdef MM_DATA_INCLUDED
#define GAP_BEFORE_MMDATA         0x00001000
#define MM_DATA_BASE              0x00000000
#define MM_DATA_SIZE              0x02200000
#endif

#ifdef POD_ENABLED
	//
	//	Define starting virtual address for demand-paged (POD) code.  The POD
	//	virtual addresses will be mapped to physical address by the POD 
	//	infrastructure -- the virtual addresses should not conflict with
	//	any physical addresses.  The starting virtual address must be a multiple
	//	of 1 Mbyte.  
	//
	#ifdef FUSE_APPS_PROCESSOR
		#define AP_POD_VM_BASE				0xa0000000	// multiple of 1 Mbyte
	#else // FUSE_APPS_PROCESSOR
		#define MSP_POD_VM_BASE				0xa0000000	// multiple of 1 Mbyte
	#endif // FUSE_APPS_PROCESSOR
#endif // POD_ENABLED

#endif //#if (!defined (_HERA_)) && (!defined (_RHEA_)) && (!defined (_SAMOA_))

// !< --- PreProcess --- >!
#if (!defined (_HERA_)) && (!defined (_RHEA_)) && (!defined (_SAMOA_))
#ifndef FUSE_APPS_PROCESSOR
#define MMU_L1L2_BASE             0x80150000					// 128k MMU L1/L2 tables
#define RO_BASE                   0x80300000					// RO starts here 
#else
#define RO_BASE                   0x82300000					// RO starts here 
#define MMU_L1L2_BASE             0x82140000					// 128k MMU L1/L2 tables
#define CP_RO_BASE                0x80300000					// RO starts here 
#endif /* FUSE_APPS_PROCESSOR */

#define IPC_BASE                  0x81E00000					// 2MB IPC shared RAM
#define IPC_SIZE                                   0x00200000

#else /* start of Hera/Rhea/Samoa */


// The address map for HERA APE is completely different to that for BRCM modem chipsets
#define DSP_SH_BASE			0x80000000					// DSP sharedmem
// for compatibility with existing code
#define SHARED_RAM_BASE		DSP_SH_BASE

#define DSP_SH_SIZE			0x00050000
#define CP_SH_BASE			DSP_SH_BASE					
#define CP_SH_SIZE			0x00020000					// 128k CP/DSP sharedmem
#define AP_SH_BASE			(DSP_SH_BASE+CP_SH_SIZE)	
#define AP_SH_SIZE			0x00020000					// 64k  AP/DSP sharedmem

#define PARM_IND_BASE		0x80120000					// 128k independent sysparm
#define PARM_IND_SIZE						0x00020000
#define PARM_DEP_BASE       0x80140000					// 64k dependent sysparm
#define PARM_DEP_SIZE                       0x00010000

#define PARM_SPML_IND_BASE		0x801E0000					// 64k hybrid SPML
#define PARM_SPML_IND_SIZE						0x00010000
#define PARM_SPML_DEP_BASE		0x801F0000					// 64k hybrid SPML (Address is verified on HERA... need to check for Athena)
#define PARM_SPML_DEP_SIZE						0x00010000

#define MMU_L1L2_SIZE						0x00020000


#define DSP_PRAM_BASE		0x80160000					// DSP PRAM load;;For x TL3: for TL3 PRAM, x for TL3 DRAM
#define DSP_PRAM_SIZE						0x00004000


#define TL3_DSP_EXT_BASE	0x81800000					// 2M TL3 Ext_PRAM
#define TL3_DSP_EXT_SIZE					0x00200000

#ifdef FUSE_APPS_PROCESSOR
/* Hera AP */
#define RO_BASE	                  0x82300000                       // RO starts here 
#define MMU_L1L2_BASE             0x82140000                       // 128k MMU L1/L2 tables. AP only
#define CP_RO_BASE                0x80300000                       //use for CP start in AP
#else
/* Hera CP */
#if (!defined (_RHEA_)) && (!defined (_SAMOA_))
#define SL1_MEM_ADDR              0x80070000					// Cacheable Router Shared Memory
#define SL1_MEM_SIZE                              0x0000A000
#define SL1_NCMEM_ADDR	          0x8007A000					// Non-cacheable Router Shared Memory
#define SL1_NCMEM_SIZE                            0x00004000
#endif

#define UMTS_CAL_BASE             0x80064000					// Spinner CAL
#if defined(_RFBIN_64K_)
#define UMTS_CAL_SIZE                             0x00010000
#else
#define UMTS_CAL_SIZE                             0x00003000
#endif

//#define RO_BASE                   0x100    //test use TCM
#define RO_BASE                   0x80300000

#define STACK_GUARD_BASE          0xF0000000                     // Hyperspace
#define STACK_GUARD_SIZE          1024
#endif

#define IPC_BASE                  0x81E00000                       // 256kB IPC shared RAM
#define IPC_SIZE                  0x00200000
#ifdef MM_DATA_INCLUDED
#define GAP_BEFORE_MMDATA         0x00001000
#define MM_DATA_BASE              0x00000000
#define MM_DATA_SIZE              0x02200000
#endif
#endif // #if (!defined (_HERA_)) && (!defined (_RHEA_)) && (!defined (_SAMOA_))
// !< --- PreProcess --- >!

#define GAP_BEFORE_RW                              0x00001000	
#define RW_BASE                   0x00000000					// RW starts after RO, 4k padding
#define GAP_BEFORE_ZI                              0x00001000	
#define ZI_BASE                   0x00000000					// ZI starts after RW, 4k padding
#define GAP_BEFORE_NOINIT                          0x00001000	
#define NOINIT_BASE               0x00000000					// NOINIT starts after ZI, 4k padding
#define GAP_BEFORE_UNCACHE			   0x00001000	
#define UNCACHE_BASE              0x00000000					// UNCACHE starts after NOINIT, 4k padding
#define GAP_BEFORE_PROTECTED                       0x00001000
#define PROTECTED_BASE            0x00000000					// PROTECTED starts after UNCACHE
#define PROTECTED_SIZE                             0x00200000

#if defined(_ATHENA_) || defined (_HERA_) || defined (_RHEA_) || defined (_SAMOA_)
#ifdef FUSE_APPS_PROCESSOR
#define GAP_BEFORE_EXCEPTSTACK			   0x00001000	
#define EXCEPTSTACK_BASE 0x00000000
#define EXCEPTSTACK_SIZE 0x00004000
#endif
#endif

