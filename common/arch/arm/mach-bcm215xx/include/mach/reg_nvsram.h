/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/include/mach/reg_nvsram.h
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

#ifndef __REG_BCM21553_NVSRAM_H__
#define __REG_BCM21553_NVSRAM_H__

#include <mach/io.h>
#include <mach/hardware.h>

/* Constants and Types */

/* Chip Select Macros */
#define CS0_BASE_ADDR	0x00000000
#define CS1_BASE_ADDR	0x02000000
#define CS2_BASE_ADDR	0x04000000
#define CS3_BASE_ADDR	0x06000000

/* NVSRAM SMC chip select base address */
#define NVSRAM_SMC_CS_BASE_ADDR          0x02000000

/* NVSRAM SMC command phase transaction */
#define NVSRAM_SMC_CMD_NUM_ADDR_CYCLE    21
#define NVSRAM_SMC_NAND_END_CMD          11
#define NVSRAM_SMC_NAND_START_CMD        3

/* NVSRAM SMC data phase transaction */
#define NVSRAM_SMC_CLEAR_CS              (1    << 21)
#define NVSRAM_SMC_RESERVED              (1    << 19)
#define NVSRAM_SMC_NAND_W_END_CMD        (0x10 << 11)

#define NVSRAM_SMC_NAND_END_CMD_REQ      (1 << 20)

/* NVSRAM MEMIF Config struct */
typedef struct
{
	uint32_t  memif_cfg;
	uint32_t  nv_conf;
} nvsram_cfg_memif;

/* NVSRAM NAND Config struct */
typedef struct
{
    uint32_t  direct_cmd;
    uint32_t  set_cycles;
    uint32_t  set_opmode;
} nvsram_cfg_nand;

/* NVSRAM address struct for holding data_phase_addr and command_phase_addr */
typedef struct
{
	uint32_t cmd_phase_addr;
	uint32_t data_phase_addr;
} nvsram_cmd;

/* Global enum */
enum {
	disabled = 0,
	enabled
};

#define MICRON_ON_DIE_ECC_ENABLED	0x80	/* Bit map for Micron on-die ECC */

/**
 * struct nand_special_dev - Special NAND Flash Device ID Structure
 *
 * @maf_id:		NAND Manufacture ID
 * @features:	Bit map that defines the special feature
 * @id_byte:	The correct byte of the ID string that defines the
 *				special feature. id_byte starts from 0.
 */
struct nand_special_dev {
	int maf_id;
	unsigned int features;
	unsigned int id_byte;
};


/* Register Definitions */
#define NVSRAM_REG_BASE                      IO_ADDRESS(BCM21553_NVSRAM_BASE)

#define NVSRAM_MEMC_STATUS_OFFSET                                         0x00000000
#define NVSRAM_MEMC_STATUS_TYPE                                           uint32_t
#define NVSRAM_MEMC_STATUS_RESERVED_MASK                                  0xFFFFEAAA
#define    NVSRAM_MEMC_STATUS_RAW_ECC_INT_SHIFT                           12
#define    NVSRAM_MEMC_STATUS_RAW_ECC_INT_MASK                            0x00001000
#define    NVSRAM_MEMC_STATUS_ECC_INT_SHIFT                               10
#define    NVSRAM_MEMC_STATUS_ECC_INT_MASK                                0x00000400
#define    NVSRAM_MEMC_STATUS_ECC_INT_EN_SHIFT                            8
#define    NVSRAM_MEMC_STATUS_ECC_INT_EN_MASK                             0x00000100
#define    NVSRAM_MEMC_STATUS_RAW_INT_STATUS_SHIFT                        6
#define    NVSRAM_MEMC_STATUS_RAW_INT_STATUS_MASK                         0x00000040
#define    NVSRAM_MEMC_STATUS_INT_STATUS_SHIFT                            4
#define    NVSRAM_MEMC_STATUS_INT_STATUS_MASK                             0x00000010
#define    NVSRAM_MEMC_STATUS_INT_EN_SHIFT                                2
#define    NVSRAM_MEMC_STATUS_INT_EN_MASK                                 0x00000004
#define    NVSRAM_MEMC_STATUS_STATE_SHIFT                                 0
#define    NVSRAM_MEMC_STATUS_STATE_MASK                                  0x00000001

#define NVSRAM_MEMIF_CFG_OFFSET                                           0x00000004
#define NVSRAM_MEMIF_CFG_TYPE                                             uint32_t
#define NVSRAM_MEMIF_CFG_RESERVED_MASK                                    0xFFFCC080
#define    NVSRAM_MEMIF_CFG_EXCLUSIVE_MONITORS_SHIFT                      16
#define    NVSRAM_MEMIF_CFG_EXCLUSIVE_MONITORS_MASK                       0x00030000
#define    NVSRAM_MEMIF_CFG_NAND_MEMORY_WIDTH_SHIFT                       12
#define    NVSRAM_MEMIF_CFG_NAND_MEMORY_WIDTH_MASK                        0x00003000
#define    NVSRAM_MEMIF_CFG_NAND_MEMORY_CHIPS_SHIFT                       10
#define    NVSRAM_MEMIF_CFG_NAND_MEMORY_CHIPS_MASK                        0x00000C00
#define    NVSRAM_MEMIF_CFG_NAND_MEMORY_TYPE_SHIFT                        8
#define    NVSRAM_MEMIF_CFG_NAND_MEMORY_TYPE_MASK                         0x00000300
#define    NVSRAM_MEMIF_CFG_REMAP_SHIFT                                   6
#define    NVSRAM_MEMIF_CFG_REMAP_MASK                                    0x00000040
#define    NVSRAM_MEMIF_CFG_SRAM_MEMORY_WIDTH_SHIFT                       4
#define    NVSRAM_MEMIF_CFG_SRAM_MEMORY_WIDTH_MASK                        0x00000030
#define    NVSRAM_MEMIF_CFG_SRAM_MEMORY_CHIPS_SHIFT                       2
#define    NVSRAM_MEMIF_CFG_SRAM_MEMORY_CHIPS_MASK                        0x0000000C
#define    NVSRAM_MEMIF_CFG_SRAM_MEMORY_TYPE_SHIFT                        0
#define    NVSRAM_MEMIF_CFG_SRAM_MEMORY_TYPE_MASK                         0x00000003

#define NVSRAM_MEMC_CFG_SET_OFFSET                                        0x00000008
#define NVSRAM_MEMC_CFG_SET_TYPE                                          uint32_t
#define NVSRAM_MEMC_CFG_SET_RESERVED_MASK                                 0xFFFFFFB8
#define    NVSRAM_MEMC_CFG_SET_NAND_ECC_INT_ENABLE_SHIFT                  6
#define    NVSRAM_MEMC_CFG_SET_NAND_ECC_INT_ENABLE_MASK                   0x00000040
#define    NVSRAM_MEMC_CFG_SET_LOW_POWER_REQ_SHIFT                        2
#define    NVSRAM_MEMC_CFG_SET_LOW_POWER_REQ_MASK                         0x00000004
#define    NVSRAM_MEMC_CFG_SET_NAND_INT_ENABLE_SHIFT                      1
#define    NVSRAM_MEMC_CFG_SET_NAND_INT_ENABLE_MASK                       0x00000002
#define    NVSRAM_MEMC_CFG_SET_SRAM_INT_ENABLE_SHIFT                      0
#define    NVSRAM_MEMC_CFG_SET_SRAM_INT_ENABLE_MASK                       0x00000001

#define NVSRAM_MEMC_CFG_CLR_OFFSET                                        0x0000000C
#define NVSRAM_MEMC_CFG_CLR_TYPE                                          uint32_t
#define NVSRAM_MEMC_CFG_CLR_RESERVED_MASK                                 0xFFFFFFA0
#define    NVSRAM_MEMC_CFG_CLR_NAND_ECC_INT_DISABLE_SHIFT                 6
#define    NVSRAM_MEMC_CFG_CLR_NAND_ECC_INT_DISABLE_MASK                  0x00000040
#define    NVSRAM_MEMC_CFG_CLR_NAND_INT_CLR_SHIFT                         4
#define    NVSRAM_MEMC_CFG_CLR_NAND_INT_CLR_MASK                          0x00000010
#define    NVSRAM_MEMC_CFG_CLR_SRAM_INT_CLR_SHIFT                         3
#define    NVSRAM_MEMC_CFG_CLR_SRAM_INT_CLR_MASK                          0x00000008
#define    NVSRAM_MEMC_CFG_CLR_LOW_POWER_EXIT_SHIFT                       2
#define    NVSRAM_MEMC_CFG_CLR_LOW_POWER_EXIT_MASK                        0x00000004
#define    NVSRAM_MEMC_CFG_CLR_NAND_INT_DISABLE_SHIFT                     1
#define    NVSRAM_MEMC_CFG_CLR_NAND_INT_DISABLE_MASK                      0x00000002
#define    NVSRAM_MEMC_CFG_CLR_SRAM_INT_DISABLE_SHIFT                     0
#define    NVSRAM_MEMC_CFG_CLR_SRAM_INT_DISABLE_MASK                      0x00000001

#define NVSRAM_DIRECT_CMD_OFFSET                                          0x00000010
#define NVSRAM_DIRECT_CMD_TYPE                                            uint32_t
#define NVSRAM_DIRECT_CMD_RESERVED_MASK                                   0xFC000000
#define    NVSRAM_DIRECT_CMD_CHIP_SELECT_SHIFT                            23
#define    NVSRAM_DIRECT_CMD_CHIP_SELECT_MASK                             0x03800000
#define    NVSRAM_DIRECT_CMD_CMD_TYPE_SHIFT                               21
#define    NVSRAM_DIRECT_CMD_CMD_TYPE_MASK                                0x00600000
#define    NVSRAM_DIRECT_CMD_SET_CRE_SHIFT                                20
#define    NVSRAM_DIRECT_CMD_SET_CRE_MASK                                 0x00100000
#define    NVSRAM_DIRECT_CMD_ADDR_SHIFT                                   0
#define    NVSRAM_DIRECT_CMD_ADDR_MASK                                    0x000FFFFF

#define NVSRAM_SET_CYCLES_OFFSET                                          0x00000014
#define NVSRAM_SET_CYCLES_TYPE                                            uint32_t
#define NVSRAM_SET_CYCLES_RESERVED_MASK                                   0xFF000000
#define    NVSRAM_SET_CYCLES_SET_T6_SHIFT                                 20
#define    NVSRAM_SET_CYCLES_SET_T6_MASK                                  0x00F00000
#define    NVSRAM_SET_CYCLES_SET_T5_SHIFT                                 17
#define    NVSRAM_SET_CYCLES_SET_T5_MASK                                  0x000E0000
#define    NVSRAM_SET_CYCLES_SET_T4_SHIFT                                 14
#define    NVSRAM_SET_CYCLES_SET_T4_MASK                                  0x0001C000
#define    NVSRAM_SET_CYCLES_SET_T3_SHIFT                                 11
#define    NVSRAM_SET_CYCLES_SET_T3_MASK                                  0x00003800
#define    NVSRAM_SET_CYCLES_SET_T2_SHIFT                                 8
#define    NVSRAM_SET_CYCLES_SET_T2_MASK                                  0x00000700
#define    NVSRAM_SET_CYCLES_SET_T1_SHIFT                                 4
#define    NVSRAM_SET_CYCLES_SET_T1_MASK                                  0x000000F0
#define    NVSRAM_SET_CYCLES_SET_T0_SHIFT                                 0
#define    NVSRAM_SET_CYCLES_SET_T0_MASK                                  0x0000000F

#define NVSRAM_SET_OPMODE_OFFSET                                          0x00000018
#define NVSRAM_SET_OPMODE_TYPE                                            uint32_t
#define NVSRAM_SET_OPMODE_RESERVED_MASK                                   0xFFFF0000
#define    NVSRAM_SET_OPMODE_SET_BURST_ALIGN_SHIFT                        13
#define    NVSRAM_SET_OPMODE_SET_BURST_ALIGN_MASK                         0x0000E000
#define    NVSRAM_SET_OPMODE_SET_BLS_SHIFT                                12
#define    NVSRAM_SET_OPMODE_SET_BLS_MASK                                 0x00001000
#define    NVSRAM_SET_OPMODE_SET_ADV_SHIFT                                11
#define    NVSRAM_SET_OPMODE_SET_ADV_MASK                                 0x00000800
#define    NVSRAM_SET_OPMODE_SET_BAA_SHIFT                                10
#define    NVSRAM_SET_OPMODE_SET_BAA_MASK                                 0x00000400
#define    NVSRAM_SET_OPMODE_SET_WR_BL_SHIFT                              7
#define    NVSRAM_SET_OPMODE_SET_WR_BL_MASK                               0x00000380
#define    NVSRAM_SET_OPMODE_SET_WR_SYNC_SHIFT                            6
#define    NVSRAM_SET_OPMODE_SET_WR_SYNC_MASK                             0x00000040
#define    NVSRAM_SET_OPMODE_SET_RD_BL_SHIFT                              3
#define    NVSRAM_SET_OPMODE_SET_RD_BL_MASK                               0x00000038
#define    NVSRAM_SET_OPMODE_SET_RD_SYNC_SHIFT                            2
#define    NVSRAM_SET_OPMODE_SET_RD_SYNC_MASK                             0x00000004
#define    NVSRAM_SET_OPMODE_SET_MW_SHIFT                                 0
#define    NVSRAM_SET_OPMODE_SET_MW_MASK                                  0x00000003

#define NVSRAM_REFRESH_PERIOD_OFFSET                                      0x00000020
#define NVSRAM_REFRESH_PERIOD_TYPE                                        uint32_t
#define NVSRAM_REFRESH_PERIOD_RESERVED_MASK                               0xFFFFFFF0
#define    NVSRAM_REFRESH_PERIOD_PERIOD_SHIFT                             0
#define    NVSRAM_REFRESH_PERIOD_PERIOD_MASK                              0x0000000F

#define NVSRAM_SRAM_CYCLES_CS0_OFFSET                                     0x00000100
#define NVSRAM_SRAM_CYCLES_CS0_TYPE                                       uint32_t
#define NVSRAM_SRAM_CYCLES_CS0_RESERVED_MASK                              0xFFE00000
#define    NVSRAM_SRAM_CYCLES_CS0_WE_TIME_SHIFT                           20
#define    NVSRAM_SRAM_CYCLES_CS0_WE_TIME_MASK                            0x00100000
#define    NVSRAM_SRAM_CYCLES_CS0_T_TR_SHIFT                              17
#define    NVSRAM_SRAM_CYCLES_CS0_T_TR_MASK                               0x000E0000
#define    NVSRAM_SRAM_CYCLES_CS0_T_PC_SHIFT                              14
#define    NVSRAM_SRAM_CYCLES_CS0_T_PC_MASK                               0x0001C000
#define    NVSRAM_SRAM_CYCLES_CS0_T_WP_SHIFT                              11
#define    NVSRAM_SRAM_CYCLES_CS0_T_WP_MASK                               0x00003800
#define    NVSRAM_SRAM_CYCLES_CS0_T_CEOE_SHIFT                            8
#define    NVSRAM_SRAM_CYCLES_CS0_T_CEOE_MASK                             0x00000700
#define    NVSRAM_SRAM_CYCLES_CS0_T_WC_SHIFT                              4
#define    NVSRAM_SRAM_CYCLES_CS0_T_WC_MASK                               0x000000F0
#define    NVSRAM_SRAM_CYCLES_CS0_T_RC_SHIFT                              0
#define    NVSRAM_SRAM_CYCLES_CS0_T_RC_MASK                               0x0000000F

#define NVSRAM_SRAM_OPMODE_CS0_OFFSET                                     0x00000104
#define NVSRAM_SRAM_OPMODE_CS0_TYPE                                       uint32_t
#define NVSRAM_SRAM_OPMODE_CS0_RESERVED_MASK                              0x00000000
#define    NVSRAM_SRAM_OPMODE_CS0_ADDRESS_MATCH_SHIFT                     24
#define    NVSRAM_SRAM_OPMODE_CS0_ADDRESS_MATCH_MASK                      0xFF000000
#define    NVSRAM_SRAM_OPMODE_CS0_ADDRESS_MASK_SHIFT                      16
#define    NVSRAM_SRAM_OPMODE_CS0_ADDRESS_MASK_MASK                       0x00FF0000
#define    NVSRAM_SRAM_OPMODE_CS0_BURST_ALIGN_SHIFT                       13
#define    NVSRAM_SRAM_OPMODE_CS0_BURST_ALIGN_MASK                        0x0000E000
#define    NVSRAM_SRAM_OPMODE_CS0_BLS_SHIFT                               12
#define    NVSRAM_SRAM_OPMODE_CS0_BLS_MASK                                0x00001000
#define    NVSRAM_SRAM_OPMODE_CS0_ADV_SHIFT                               11
#define    NVSRAM_SRAM_OPMODE_CS0_ADV_MASK                                0x00000800
#define    NVSRAM_SRAM_OPMODE_CS0_BAA_SHIFT                               10
#define    NVSRAM_SRAM_OPMODE_CS0_BAA_MASK                                0x00000400
#define    NVSRAM_SRAM_OPMODE_CS0_WR_BL_SHIFT                             7
#define    NVSRAM_SRAM_OPMODE_CS0_WR_BL_MASK                              0x00000380
#define    NVSRAM_SRAM_OPMODE_CS0_WR_SYNC_SHIFT                           6
#define    NVSRAM_SRAM_OPMODE_CS0_WR_SYNC_MASK                            0x00000040
#define    NVSRAM_SRAM_OPMODE_CS0_RD_BL_SHIFT                             3
#define    NVSRAM_SRAM_OPMODE_CS0_RD_BL_MASK                              0x00000038
#define    NVSRAM_SRAM_OPMODE_CS0_RD_SYNC_SHIFT                           2
#define    NVSRAM_SRAM_OPMODE_CS0_RD_SYNC_MASK                            0x00000004
#define    NVSRAM_SRAM_OPMODE_CS0_MW_SHIFT                                0
#define    NVSRAM_SRAM_OPMODE_CS0_MW_MASK                                 0x00000003

#define NVSRAM_SRAM_CYCLES_CS1_OFFSET                                     0x00000120
#define NVSRAM_SRAM_CYCLES_CS1_TYPE                                       uint32_t
#define NVSRAM_SRAM_CYCLES_CS1_RESERVED_MASK                              0xFFE00000
#define    NVSRAM_SRAM_CYCLES_CS1_WE_TIME_SHIFT                           20
#define    NVSRAM_SRAM_CYCLES_CS1_WE_TIME_MASK                            0x00100000
#define    NVSRAM_SRAM_CYCLES_CS1_T_TR_SHIFT                              17
#define    NVSRAM_SRAM_CYCLES_CS1_T_TR_MASK                               0x000E0000
#define    NVSRAM_SRAM_CYCLES_CS1_T_PC_SHIFT                              14
#define    NVSRAM_SRAM_CYCLES_CS1_T_PC_MASK                               0x0001C000
#define    NVSRAM_SRAM_CYCLES_CS1_T_WP_SHIFT                              11
#define    NVSRAM_SRAM_CYCLES_CS1_T_WP_MASK                               0x00003800
#define    NVSRAM_SRAM_CYCLES_CS1_T_CEOE_SHIFT                            8
#define    NVSRAM_SRAM_CYCLES_CS1_T_CEOE_MASK                             0x00000700
#define    NVSRAM_SRAM_CYCLES_CS1_T_WC_SHIFT                              4
#define    NVSRAM_SRAM_CYCLES_CS1_T_WC_MASK                               0x000000F0
#define    NVSRAM_SRAM_CYCLES_CS1_T_RC_SHIFT                              0
#define    NVSRAM_SRAM_CYCLES_CS1_T_RC_MASK                               0x0000000F

#define NVSRAM_SRAM_OPMODE_CS1_OFFSET                                     0x00000124
#define NVSRAM_SRAM_OPMODE_CS1_TYPE                                       uint32_t
#define NVSRAM_SRAM_OPMODE_CS1_RESERVED_MASK                              0x00000000
#define    NVSRAM_SRAM_OPMODE_CS1_ADDRESS_MATCH_SHIFT                     24
#define    NVSRAM_SRAM_OPMODE_CS1_ADDRESS_MATCH_MASK                      0xFF000000
#define    NVSRAM_SRAM_OPMODE_CS1_ADDRESS_MASK_SHIFT                      16
#define    NVSRAM_SRAM_OPMODE_CS1_ADDRESS_MASK_MASK                       0x00FF0000
#define    NVSRAM_SRAM_OPMODE_CS1_BURST_ALIGN_SHIFT                       13
#define    NVSRAM_SRAM_OPMODE_CS1_BURST_ALIGN_MASK                        0x0000E000
#define    NVSRAM_SRAM_OPMODE_CS1_BLS_SHIFT                               12
#define    NVSRAM_SRAM_OPMODE_CS1_BLS_MASK                                0x00001000
#define    NVSRAM_SRAM_OPMODE_CS1_ADV_SHIFT                               11
#define    NVSRAM_SRAM_OPMODE_CS1_ADV_MASK                                0x00000800
#define    NVSRAM_SRAM_OPMODE_CS1_BAA_SHIFT                               10
#define    NVSRAM_SRAM_OPMODE_CS1_BAA_MASK                                0x00000400
#define    NVSRAM_SRAM_OPMODE_CS1_WR_BL_SHIFT                             7
#define    NVSRAM_SRAM_OPMODE_CS1_WR_BL_MASK                              0x00000380
#define    NVSRAM_SRAM_OPMODE_CS1_WR_SYNC_SHIFT                           6
#define    NVSRAM_SRAM_OPMODE_CS1_WR_SYNC_MASK                            0x00000040
#define    NVSRAM_SRAM_OPMODE_CS1_RD_BL_SHIFT                             3
#define    NVSRAM_SRAM_OPMODE_CS1_RD_BL_MASK                              0x00000038
#define    NVSRAM_SRAM_OPMODE_CS1_RD_SYNC_SHIFT                           2
#define    NVSRAM_SRAM_OPMODE_CS1_RD_SYNC_MASK                            0x00000004
#define    NVSRAM_SRAM_OPMODE_CS1_MW_SHIFT                                0
#define    NVSRAM_SRAM_OPMODE_CS1_MW_MASK                                 0x00000003

#define NVSRAM_SRAM_CYCLES_CS2_OFFSET                                     0x00000140
#define NVSRAM_SRAM_CYCLES_CS2_TYPE                                       uint32_t
#define NVSRAM_SRAM_CYCLES_CS2_RESERVED_MASK                              0xFFE00000
#define    NVSRAM_SRAM_CYCLES_CS2_WE_TIME_SHIFT                           20
#define    NVSRAM_SRAM_CYCLES_CS2_WE_TIME_MASK                            0x00100000
#define    NVSRAM_SRAM_CYCLES_CS2_T_TR_SHIFT                              17
#define    NVSRAM_SRAM_CYCLES_CS2_T_TR_MASK                               0x000E0000
#define    NVSRAM_SRAM_CYCLES_CS2_T_PC_SHIFT                              14
#define    NVSRAM_SRAM_CYCLES_CS2_T_PC_MASK                               0x0001C000
#define    NVSRAM_SRAM_CYCLES_CS2_T_WP_SHIFT                              11
#define    NVSRAM_SRAM_CYCLES_CS2_T_WP_MASK                               0x00003800
#define    NVSRAM_SRAM_CYCLES_CS2_T_CEOE_SHIFT                            8
#define    NVSRAM_SRAM_CYCLES_CS2_T_CEOE_MASK                             0x00000700
#define    NVSRAM_SRAM_CYCLES_CS2_T_WC_SHIFT                              4
#define    NVSRAM_SRAM_CYCLES_CS2_T_WC_MASK                               0x000000F0
#define    NVSRAM_SRAM_CYCLES_CS2_T_RC_SHIFT                              0
#define    NVSRAM_SRAM_CYCLES_CS2_T_RC_MASK                               0x0000000F

#define NVSRAM_SRAM_OPMODE_CS2_OFFSET                                     0x00000144
#define NVSRAM_SRAM_OPMODE_CS2_TYPE                                       uint32_t
#define NVSRAM_SRAM_OPMODE_CS2_RESERVED_MASK                              0x00000000
#define    NVSRAM_SRAM_OPMODE_CS2_ADDRESS_MATCH_SHIFT                     24
#define    NVSRAM_SRAM_OPMODE_CS2_ADDRESS_MATCH_MASK                      0xFF000000
#define    NVSRAM_SRAM_OPMODE_CS2_ADDRESS_MASK_SHIFT                      16
#define    NVSRAM_SRAM_OPMODE_CS2_ADDRESS_MASK_MASK                       0x00FF0000
#define    NVSRAM_SRAM_OPMODE_CS2_BURST_ALIGN_SHIFT                       13
#define    NVSRAM_SRAM_OPMODE_CS2_BURST_ALIGN_MASK                        0x0000E000
#define    NVSRAM_SRAM_OPMODE_CS2_BLS_SHIFT                               12
#define    NVSRAM_SRAM_OPMODE_CS2_BLS_MASK                                0x00001000
#define    NVSRAM_SRAM_OPMODE_CS2_ADV_SHIFT                               11
#define    NVSRAM_SRAM_OPMODE_CS2_ADV_MASK                                0x00000800
#define    NVSRAM_SRAM_OPMODE_CS2_BAA_SHIFT                               10
#define    NVSRAM_SRAM_OPMODE_CS2_BAA_MASK                                0x00000400
#define    NVSRAM_SRAM_OPMODE_CS2_WR_BL_SHIFT                             7
#define    NVSRAM_SRAM_OPMODE_CS2_WR_BL_MASK                              0x00000380
#define    NVSRAM_SRAM_OPMODE_CS2_WR_SYNC_SHIFT                           6
#define    NVSRAM_SRAM_OPMODE_CS2_WR_SYNC_MASK                            0x00000040
#define    NVSRAM_SRAM_OPMODE_CS2_RD_BL_SHIFT                             3
#define    NVSRAM_SRAM_OPMODE_CS2_RD_BL_MASK                              0x00000038
#define    NVSRAM_SRAM_OPMODE_CS2_RD_SYNC_SHIFT                           2
#define    NVSRAM_SRAM_OPMODE_CS2_RD_SYNC_MASK                            0x00000004
#define    NVSRAM_SRAM_OPMODE_CS2_MW_SHIFT                                0
#define    NVSRAM_SRAM_OPMODE_CS2_MW_MASK                                 0x00000003

#define NVSRAM_SRAM_CYCLES_CS3_OFFSET                                     0x00000160
#define NVSRAM_SRAM_CYCLES_CS3_TYPE                                       uint32_t
#define NVSRAM_SRAM_CYCLES_CS3_RESERVED_MASK                              0xFFE00000
#define    NVSRAM_SRAM_CYCLES_CS3_WE_TIME_SHIFT                           20
#define    NVSRAM_SRAM_CYCLES_CS3_WE_TIME_MASK                            0x00100000
#define    NVSRAM_SRAM_CYCLES_CS3_T_TR_SHIFT                              17
#define    NVSRAM_SRAM_CYCLES_CS3_T_TR_MASK                               0x000E0000
#define    NVSRAM_SRAM_CYCLES_CS3_T_PC_SHIFT                              14
#define    NVSRAM_SRAM_CYCLES_CS3_T_PC_MASK                               0x0001C000
#define    NVSRAM_SRAM_CYCLES_CS3_T_WP_SHIFT                              11
#define    NVSRAM_SRAM_CYCLES_CS3_T_WP_MASK                               0x00003800
#define    NVSRAM_SRAM_CYCLES_CS3_T_CEOE_SHIFT                            8
#define    NVSRAM_SRAM_CYCLES_CS3_T_CEOE_MASK                             0x00000700
#define    NVSRAM_SRAM_CYCLES_CS3_T_WC_SHIFT                              4
#define    NVSRAM_SRAM_CYCLES_CS3_T_WC_MASK                               0x000000F0
#define    NVSRAM_SRAM_CYCLES_CS3_T_RC_SHIFT                              0
#define    NVSRAM_SRAM_CYCLES_CS3_T_RC_MASK                               0x0000000F

#define NVSRAM_SRAM_OPMODE_CS3_OFFSET                                     0x00000164
#define NVSRAM_SRAM_OPMODE_CS3_TYPE                                       uint32_t
#define NVSRAM_SRAM_OPMODE_CS3_RESERVED_MASK                              0x00000000
#define    NVSRAM_SRAM_OPMODE_CS3_ADDRESS_MATCH_SHIFT                     24
#define    NVSRAM_SRAM_OPMODE_CS3_ADDRESS_MATCH_MASK                      0xFF000000
#define    NVSRAM_SRAM_OPMODE_CS3_ADDRESS_MASK_SHIFT                      16
#define    NVSRAM_SRAM_OPMODE_CS3_ADDRESS_MASK_MASK                       0x00FF0000
#define    NVSRAM_SRAM_OPMODE_CS3_BURST_ALIGN_SHIFT                       13
#define    NVSRAM_SRAM_OPMODE_CS3_BURST_ALIGN_MASK                        0x0000E000
#define    NVSRAM_SRAM_OPMODE_CS3_BLS_SHIFT                               12
#define    NVSRAM_SRAM_OPMODE_CS3_BLS_MASK                                0x00001000
#define    NVSRAM_SRAM_OPMODE_CS3_ADV_SHIFT                               11
#define    NVSRAM_SRAM_OPMODE_CS3_ADV_MASK                                0x00000800
#define    NVSRAM_SRAM_OPMODE_CS3_BAA_SHIFT                               10
#define    NVSRAM_SRAM_OPMODE_CS3_BAA_MASK                                0x00000400
#define    NVSRAM_SRAM_OPMODE_CS3_WR_BL_SHIFT                             7
#define    NVSRAM_SRAM_OPMODE_CS3_WR_BL_MASK                              0x00000380
#define    NVSRAM_SRAM_OPMODE_CS3_WR_SYNC_SHIFT                           6
#define    NVSRAM_SRAM_OPMODE_CS3_WR_SYNC_MASK                            0x00000040
#define    NVSRAM_SRAM_OPMODE_CS3_RD_BL_SHIFT                             3
#define    NVSRAM_SRAM_OPMODE_CS3_RD_BL_MASK                              0x00000038
#define    NVSRAM_SRAM_OPMODE_CS3_RD_SYNC_SHIFT                           2
#define    NVSRAM_SRAM_OPMODE_CS3_RD_SYNC_MASK                            0x00000004
#define    NVSRAM_SRAM_OPMODE_CS3_MW_SHIFT                                0
#define    NVSRAM_SRAM_OPMODE_CS3_MW_MASK                                 0x00000003

#define NVSRAM_NAND_CYCLES_CS0_OFFSET                                     0x00000180
#define NVSRAM_NAND_CYCLES_CS0_TYPE                                       uint32_t
#define NVSRAM_NAND_CYCLES_CS0_RESERVED_MASK                              0xFF000000
#define    NVSRAM_NAND_CYCLES_CS0_T_RR_SHIFT                              20
#define    NVSRAM_NAND_CYCLES_CS0_T_RR_MASK                               0x00F00000
#define    NVSRAM_NAND_CYCLES_CS0_T_AR_SHIFT                              17
#define    NVSRAM_NAND_CYCLES_CS0_T_AR_MASK                               0x000E0000
#define    NVSRAM_NAND_CYCLES_CS0_T_CLR_SHIFT                             14
#define    NVSRAM_NAND_CYCLES_CS0_T_CLR_MASK                              0x0001C000
#define    NVSRAM_NAND_CYCLES_CS0_T_WP_SHIFT                              11
#define    NVSRAM_NAND_CYCLES_CS0_T_WP_MASK                               0x00003800
#define    NVSRAM_NAND_CYCLES_CS0_T_REA_SHIFT                             8
#define    NVSRAM_NAND_CYCLES_CS0_T_REA_MASK                              0x00000700
#define    NVSRAM_NAND_CYCLES_CS0_T_WC_SHIFT                              4
#define    NVSRAM_NAND_CYCLES_CS0_T_WC_MASK                               0x000000F0
#define    NVSRAM_NAND_CYCLES_CS0_T_RC_SHIFT                              0
#define    NVSRAM_NAND_CYCLES_CS0_T_RC_MASK                               0x0000000F

#define NVSRAM_NAND_OPMODE_CS0_OFFSET                                     0x00000184
#define NVSRAM_NAND_OPMODE_CS0_TYPE                                       uint32_t
#define NVSRAM_NAND_OPMODE_CS0_RESERVED_MASK                              0x0000FFFC
#define    NVSRAM_NAND_OPMODE_CS0_ADDRESS_MATCH_SHIFT                     24
#define    NVSRAM_NAND_OPMODE_CS0_ADDRESS_MATCH_MASK                      0xFF000000
#define    NVSRAM_NAND_OPMODE_CS0_ADDRESS_MASK_SHIFT                      16
#define    NVSRAM_NAND_OPMODE_CS0_ADDRESS_MASK_MASK                       0x00FF0000
#define    NVSRAM_NAND_OPMODE_CS0_MW_SHIFT                                0
#define    NVSRAM_NAND_OPMODE_CS0_MW_MASK                                 0x00000003

#define NVSRAM_NAND_CYCLES_CS1_OFFSET                                     0x000001A0
#define NVSRAM_NAND_CYCLES_CS1_TYPE                                       uint32_t
#define NVSRAM_NAND_CYCLES_CS1_RESERVED_MASK                              0xFF000000
#define    NVSRAM_NAND_CYCLES_CS1_T_RR_SHIFT                              20
#define    NVSRAM_NAND_CYCLES_CS1_T_RR_MASK                               0x00F00000
#define    NVSRAM_NAND_CYCLES_CS1_T_AR_SHIFT                              17
#define    NVSRAM_NAND_CYCLES_CS1_T_AR_MASK                               0x000E0000
#define    NVSRAM_NAND_CYCLES_CS1_T_CLR_SHIFT                             14
#define    NVSRAM_NAND_CYCLES_CS1_T_CLR_MASK                              0x0001C000
#define    NVSRAM_NAND_CYCLES_CS1_T_WP_SHIFT                              11
#define    NVSRAM_NAND_CYCLES_CS1_T_WP_MASK                               0x00003800
#define    NVSRAM_NAND_CYCLES_CS1_T_REA_SHIFT                             8
#define    NVSRAM_NAND_CYCLES_CS1_T_REA_MASK                              0x00000700
#define    NVSRAM_NAND_CYCLES_CS1_T_WC_SHIFT                              4
#define    NVSRAM_NAND_CYCLES_CS1_T_WC_MASK                               0x000000F0
#define    NVSRAM_NAND_CYCLES_CS1_T_RC_SHIFT                              0
#define    NVSRAM_NAND_CYCLES_CS1_T_RC_MASK                               0x0000000F

#define NVSRAM_NAND_OPMODE_CS1_OFFSET                                     0x000001A4
#define NVSRAM_NAND_OPMODE_CS1_TYPE                                       uint32_t
#define NVSRAM_NAND_OPMODE_CS1_RESERVED_MASK                              0x0000FFFC
#define    NVSRAM_NAND_OPMODE_CS1_ADDRESS_MATCH_SHIFT                     24
#define    NVSRAM_NAND_OPMODE_CS1_ADDRESS_MATCH_MASK                      0xFF000000
#define    NVSRAM_NAND_OPMODE_CS1_ADDRESS_MASK_SHIFT                      16
#define    NVSRAM_NAND_OPMODE_CS1_ADDRESS_MASK_MASK                       0x00FF0000
#define    NVSRAM_NAND_OPMODE_CS1_MW_SHIFT                                0
#define    NVSRAM_NAND_OPMODE_CS1_MW_MASK                                 0x00000003

#define NVSRAM_NAND_CYCLES_CS2_OFFSET                                     0x000001C0
#define NVSRAM_NAND_CYCLES_CS2_TYPE                                       uint32_t
#define NVSRAM_NAND_CYCLES_CS2_RESERVED_MASK                              0xFF000000
#define    NVSRAM_NAND_CYCLES_CS2_T_RR_SHIFT                              20
#define    NVSRAM_NAND_CYCLES_CS2_T_RR_MASK                               0x00F00000
#define    NVSRAM_NAND_CYCLES_CS2_T_AR_SHIFT                              17
#define    NVSRAM_NAND_CYCLES_CS2_T_AR_MASK                               0x000E0000
#define    NVSRAM_NAND_CYCLES_CS2_T_CLR_SHIFT                             14
#define    NVSRAM_NAND_CYCLES_CS2_T_CLR_MASK                              0x0001C000
#define    NVSRAM_NAND_CYCLES_CS2_T_WP_SHIFT                              11
#define    NVSRAM_NAND_CYCLES_CS2_T_WP_MASK                               0x00003800
#define    NVSRAM_NAND_CYCLES_CS2_T_REA_SHIFT                             8
#define    NVSRAM_NAND_CYCLES_CS2_T_REA_MASK                              0x00000700
#define    NVSRAM_NAND_CYCLES_CS2_T_WC_SHIFT                              4
#define    NVSRAM_NAND_CYCLES_CS2_T_WC_MASK                               0x000000F0
#define    NVSRAM_NAND_CYCLES_CS2_T_RC_SHIFT                              0
#define    NVSRAM_NAND_CYCLES_CS2_T_RC_MASK                               0x0000000F

#define NVSRAM_NAND_OPMODE_CS2_OFFSET                                     0x000001C4
#define NVSRAM_NAND_OPMODE_CS2_TYPE                                       uint32_t
#define NVSRAM_NAND_OPMODE_CS2_RESERVED_MASK                              0x0000FFFC
#define    NVSRAM_NAND_OPMODE_CS2_ADDRESS_MATCH_SHIFT                     24
#define    NVSRAM_NAND_OPMODE_CS2_ADDRESS_MATCH_MASK                      0xFF000000
#define    NVSRAM_NAND_OPMODE_CS2_ADDRESS_MASK_SHIFT                      16
#define    NVSRAM_NAND_OPMODE_CS2_ADDRESS_MASK_MASK                       0x00FF0000
#define    NVSRAM_NAND_OPMODE_CS2_MW_SHIFT                                0
#define    NVSRAM_NAND_OPMODE_CS2_MW_MASK                                 0x00000003

#define NVSRAM_NAND_CYCLES_CS3_OFFSET                                     0x000001E0
#define NVSRAM_NAND_CYCLES_CS3_TYPE                                       uint32_t
#define NVSRAM_NAND_CYCLES_CS3_RESERVED_MASK                              0xFF000000
#define    NVSRAM_NAND_CYCLES_CS3_T_RR_SHIFT                              20
#define    NVSRAM_NAND_CYCLES_CS3_T_RR_MASK                               0x00F00000
#define    NVSRAM_NAND_CYCLES_CS3_T_AR_SHIFT                              17
#define    NVSRAM_NAND_CYCLES_CS3_T_AR_MASK                               0x000E0000
#define    NVSRAM_NAND_CYCLES_CS3_T_CLR_SHIFT                             14
#define    NVSRAM_NAND_CYCLES_CS3_T_CLR_MASK                              0x0001C000
#define    NVSRAM_NAND_CYCLES_CS3_T_WP_SHIFT                              11
#define    NVSRAM_NAND_CYCLES_CS3_T_WP_MASK                               0x00003800
#define    NVSRAM_NAND_CYCLES_CS3_T_REA_SHIFT                             8
#define    NVSRAM_NAND_CYCLES_CS3_T_REA_MASK                              0x00000700
#define    NVSRAM_NAND_CYCLES_CS3_T_WC_SHIFT                              4
#define    NVSRAM_NAND_CYCLES_CS3_T_WC_MASK                               0x000000F0
#define    NVSRAM_NAND_CYCLES_CS3_T_RC_SHIFT                              0
#define    NVSRAM_NAND_CYCLES_CS3_T_RC_MASK                               0x0000000F

#define NVSRAM_NAND_OPMODE_CS3_OFFSET                                     0x000001E4
#define NVSRAM_NAND_OPMODE_CS3_TYPE                                       uint32_t
#define NVSRAM_NAND_OPMODE_CS3_RESERVED_MASK                              0x0000FFFC
#define    NVSRAM_NAND_OPMODE_CS3_ADDRESS_MATCH_SHIFT                     24
#define    NVSRAM_NAND_OPMODE_CS3_ADDRESS_MATCH_MASK                      0xFF000000
#define    NVSRAM_NAND_OPMODE_CS3_ADDRESS_MASK_SHIFT                      16
#define    NVSRAM_NAND_OPMODE_CS3_ADDRESS_MASK_MASK                       0x00FF0000
#define    NVSRAM_NAND_OPMODE_CS3_MW_SHIFT                                0
#define    NVSRAM_NAND_OPMODE_CS3_MW_MASK                                 0x00000003

#define NVSRAM_USER_STATUS_OFFSET                                         0x00000200
#define NVSRAM_USER_STATUS_TYPE                                           uint32_t
#define NVSRAM_USER_STATUS_RESERVED_MASK                                  0xFFFFFF00
#define    NVSRAM_USER_STATUS_USER_STATUS_SHIFT                           0
#define    NVSRAM_USER_STATUS_USER_STATUS_MASK                            0x000000FF

#define NVSRAM_USER_CONFIG_OFFSET                                         0x00000204
#define NVSRAM_USER_CONFIG_TYPE                                           uint32_t
#define NVSRAM_USER_CONFIG_RESERVED_MASK                                  0xFFFFFF00
#define    NVSRAM_USER_CONFIG_USER_CONFIG_SHIFT                           0
#define    NVSRAM_USER_CONFIG_USER_CONFIG_MASK                            0x000000FF

#define NVSRAM_ECC_STATUS_OFFSET                                          0x00000400
#define NVSRAM_ECC_STATUS_TYPE                                            uint32_t
#define NVSRAM_ECC_STATUS_RESERVED_MASK                                   0xC0000000
#define    NVSRAM_ECC_STATUS_ECC_READ_SHIFT                               25
#define    NVSRAM_ECC_STATUS_ECC_READ_MASK                                0x3E000000
#define    NVSRAM_ECC_STATUS_ECC_CAN_CORRECT_SHIFT                        20
#define    NVSRAM_ECC_STATUS_ECC_CAN_CORRECT_MASK                         0x01F00000
#define    NVSRAM_ECC_STATUS_ECC_FAIL_SHIFT                               15
#define    NVSRAM_ECC_STATUS_ECC_FAIL_MASK                                0x000F8000
#define    NVSRAM_ECC_STATUS_ECC_VALUE_VALID_SHIFT                        10
#define    NVSRAM_ECC_STATUS_ECC_VALUE_VALID_MASK                         0x00007C00
#define    NVSRAM_ECC_STATUS_ECC_READ_NOT_WRITE_SHIFT                     9
#define    NVSRAM_ECC_STATUS_ECC_READ_NOT_WRITE_MASK                      0x00000200
#define    NVSRAM_ECC_STATUS_ECC_LAST_SHIFT                               7
#define    NVSRAM_ECC_STATUS_ECC_LAST_MASK                                0x00000180
#define    NVSRAM_ECC_STATUS_ECC_STATUS_SHIFT                             6
#define    NVSRAM_ECC_STATUS_ECC_STATUS_MASK                              0x00000040
#define    NVSRAM_ECC_STATUS_RAW_INT_STATUS_SHIFT                         0
#define    NVSRAM_ECC_STATUS_RAW_INT_STATUS_MASK                          0x0000003F

#define NVSRAM_ECC_MEMCFG_OFFSET                                          0x00000404
#define NVSRAM_ECC_MEMCFG_TYPE                                            uint32_t
#define NVSRAM_ECC_MEMCFG_RESERVED_MASK                                   0xFFFFE000
#define    NVSRAM_ECC_MEMCFG_ECC_EXTRA_BLOCK_SIZE_SHIFT                   11
#define    NVSRAM_ECC_MEMCFG_ECC_EXTRA_BLOCK_SIZE_MASK                    0x00001800
#define    NVSRAM_ECC_MEMCFG_ECC_EXTRA_BLOCK_SHIFT                        10
#define    NVSRAM_ECC_MEMCFG_ECC_EXTRA_BLOCK_MASK                         0x00000400
#define    NVSRAM_ECC_MEMCFG_ECC_INT_ABORT_SHIFT                          9
#define    NVSRAM_ECC_MEMCFG_ECC_INT_ABORT_MASK                           0x00000200
#define    NVSRAM_ECC_MEMCFG_ECC_INT_PASS_SHIFT                           8
#define    NVSRAM_ECC_MEMCFG_ECC_INT_PASS_MASK                            0x00000100
#define    NVSRAM_ECC_MEMCFG_ECC_IGNORE_ADD_EIGHT_SHIFT                   7
#define    NVSRAM_ECC_MEMCFG_ECC_IGNORE_ADD_EIGHT_MASK                    0x00000080
#define    NVSRAM_ECC_MEMCFG_ECC_JUMP_SHIFT                               5
#define    NVSRAM_ECC_MEMCFG_ECC_JUMP_MASK                                0x00000060
#define    NVSRAM_ECC_MEMCFG_ECC_READ_END_SHIFT                           4
#define    NVSRAM_ECC_MEMCFG_ECC_READ_END_MASK                            0x00000010
#define    NVSRAM_ECC_MEMCFG_ECC_MODE_SHIFT                               2
#define    NVSRAM_ECC_MEMCFG_ECC_MODE_MASK                                0x0000000C
#define    NVSRAM_ECC_MEMCFG_PAGE_SIZE_SHIFT                              0
#define    NVSRAM_ECC_MEMCFG_PAGE_SIZE_MASK                               0x00000003

#define NVSRAM_ECC_MEMCOMMAND1_OFFSET                                     0x00000408
#define NVSRAM_ECC_MEMCOMMAND1_TYPE                                       uint32_t
#define NVSRAM_ECC_MEMCOMMAND1_RESERVED_MASK                              0xFE000000
#define    NVSRAM_ECC_MEMCOMMAND1_NAND_RD_CMD_END_VALID_SHIFT             24
#define    NVSRAM_ECC_MEMCOMMAND1_NAND_RD_CMD_END_VALID_MASK              0x01000000
#define    NVSRAM_ECC_MEMCOMMAND1_NAND_RD_CMD_END_SHIFT                   16
#define    NVSRAM_ECC_MEMCOMMAND1_NAND_RD_CMD_END_MASK                    0x00FF0000
#define    NVSRAM_ECC_MEMCOMMAND1_NAND_RD_CMD_SHIFT                       8
#define    NVSRAM_ECC_MEMCOMMAND1_NAND_RD_CMD_MASK                        0x0000FF00
#define    NVSRAM_ECC_MEMCOMMAND1_NAND_WR_CMD_SHIFT                       0
#define    NVSRAM_ECC_MEMCOMMAND1_NAND_WR_CMD_MASK                        0x000000FF

#define NVSRAM_ECC_MEMCOMMAND2_OFFSET                                     0x0000040C
#define NVSRAM_ECC_MEMCOMMAND2_TYPE                                       uint32_t
#define NVSRAM_ECC_MEMCOMMAND2_RESERVED_MASK                              0xFE000000
#define    NVSRAM_ECC_MEMCOMMAND2_NAND_RD_COL_CHANGE_END_VALID_SHIFT      24
#define    NVSRAM_ECC_MEMCOMMAND2_NAND_RD_COL_CHANGE_END_VALID_MASK       0x01000000
#define    NVSRAM_ECC_MEMCOMMAND2_NAND_RD_COL_CHANGE_END_SHIFT            16
#define    NVSRAM_ECC_MEMCOMMAND2_NAND_RD_COL_CHANGE_END_MASK             0x00FF0000
#define    NVSRAM_ECC_MEMCOMMAND2_NAND_RD_COL_CHANGE_SHIFT                8
#define    NVSRAM_ECC_MEMCOMMAND2_NAND_RD_COL_CHANGE_MASK                 0x0000FF00
#define    NVSRAM_ECC_MEMCOMMAND2_NAND_WR_COL_CHANGE_SHIFT                0
#define    NVSRAM_ECC_MEMCOMMAND2_NAND_WR_COL_CHANGE_MASK                 0x000000FF

#define NVSRAM_ECC_ADDR0_OFFSET                                           0x00000410
#define NVSRAM_ECC_ADDR0_TYPE                                             uint32_t
#define NVSRAM_ECC_ADDR0_RESERVED_MASK                                    0x00000000
#define    NVSRAM_ECC_ADDR0_ECC_ADDR_SHIFT                                0
#define    NVSRAM_ECC_ADDR0_ECC_ADDR_MASK                                 0xFFFFFFFF

#define NVSRAM_ECC_ADDR1_OFFSET                                           0x00000414
#define NVSRAM_ECC_ADDR1_TYPE                                             uint32_t
#define NVSRAM_ECC_ADDR1_RESERVED_MASK                                    0xFF000000
#define    NVSRAM_ECC_ADDR1_ECC_ADDR_SHIFT                                0
#define    NVSRAM_ECC_ADDR1_ECC_ADDR_MASK                                 0x00FFFFFF

#define NVSRAM_ECC_VALUE0_OFFSET                                          0x00000418
#define NVSRAM_ECC_VALUE0_TYPE                                            uint32_t
#define NVSRAM_ECC_VALUE0_RESERVED_MASK                                   0x07000000
#define    NVSRAM_ECC_VALUE0_ECC_INT_SHIFT                                31
#define    NVSRAM_ECC_VALUE0_ECC_INT_MASK                                 0x80000000
#define    NVSRAM_ECC_VALUE0_ECC_VALID_SHIFT                              30
#define    NVSRAM_ECC_VALUE0_ECC_VALID_MASK                               0x40000000
#define    NVSRAM_ECC_VALUE0_ECC_READ_SHIFT                               29
#define    NVSRAM_ECC_VALUE0_ECC_READ_MASK                                0x20000000
#define    NVSRAM_ECC_VALUE0_ECC_FAIL_SHIFT                               28
#define    NVSRAM_ECC_VALUE0_ECC_FAIL_MASK                                0x10000000
#define    NVSRAM_ECC_VALUE0_ECC_CORRECT_SHIFT                            27
#define    NVSRAM_ECC_VALUE0_ECC_CORRECT_MASK                             0x08000000
#define    NVSRAM_ECC_VALUE0_ECC_VALUE_SHIFT                              0
#define    NVSRAM_ECC_VALUE0_ECC_VALUE_MASK                               0x00FFFFFF

#define NVSRAM_ECC_VALUE1_OFFSET                                          0x0000041C
#define NVSRAM_ECC_VALUE1_TYPE                                            uint32_t
#define NVSRAM_ECC_VALUE1_RESERVED_MASK                                   0x07000000
#define    NVSRAM_ECC_VALUE1_ECC_INT_SHIFT                                31
#define    NVSRAM_ECC_VALUE1_ECC_INT_MASK                                 0x80000000
#define    NVSRAM_ECC_VALUE1_ECC_VALID_SHIFT                              30
#define    NVSRAM_ECC_VALUE1_ECC_VALID_MASK                               0x40000000
#define    NVSRAM_ECC_VALUE1_ECC_READ_SHIFT                               29
#define    NVSRAM_ECC_VALUE1_ECC_READ_MASK                                0x20000000
#define    NVSRAM_ECC_VALUE1_ECC_FAIL_SHIFT                               28
#define    NVSRAM_ECC_VALUE1_ECC_FAIL_MASK                                0x10000000
#define    NVSRAM_ECC_VALUE1_ECC_CORRECT_SHIFT                            27
#define    NVSRAM_ECC_VALUE1_ECC_CORRECT_MASK                             0x08000000
#define    NVSRAM_ECC_VALUE1_ECC_VALUE_SHIFT                              0
#define    NVSRAM_ECC_VALUE1_ECC_VALUE_MASK                               0x00FFFFFF

#define NVSRAM_ECC_VALUE2_OFFSET                                          0x00000420
#define NVSRAM_ECC_VALUE2_TYPE                                            uint32_t
#define NVSRAM_ECC_VALUE2_RESERVED_MASK                                   0x07000000
#define    NVSRAM_ECC_VALUE2_ECC_INT_SHIFT                                31
#define    NVSRAM_ECC_VALUE2_ECC_INT_MASK                                 0x80000000
#define    NVSRAM_ECC_VALUE2_ECC_VALID_SHIFT                              30
#define    NVSRAM_ECC_VALUE2_ECC_VALID_MASK                               0x40000000
#define    NVSRAM_ECC_VALUE2_ECC_READ_SHIFT                               29
#define    NVSRAM_ECC_VALUE2_ECC_READ_MASK                                0x20000000
#define    NVSRAM_ECC_VALUE2_ECC_FAIL_SHIFT                               28
#define    NVSRAM_ECC_VALUE2_ECC_FAIL_MASK                                0x10000000
#define    NVSRAM_ECC_VALUE2_ECC_CORRECT_SHIFT                            27
#define    NVSRAM_ECC_VALUE2_ECC_CORRECT_MASK                             0x08000000
#define    NVSRAM_ECC_VALUE2_ECC_VALUE_SHIFT                              0
#define    NVSRAM_ECC_VALUE2_ECC_VALUE_MASK                               0x00FFFFFF

#define NVSRAM_ECC_VALUE3_OFFSET                                          0x00000424
#define NVSRAM_ECC_VALUE3_TYPE                                            uint32_t
#define NVSRAM_ECC_VALUE3_RESERVED_MASK                                   0x07000000
#define    NVSRAM_ECC_VALUE3_ECC_INT_SHIFT                                31
#define    NVSRAM_ECC_VALUE3_ECC_INT_MASK                                 0x80000000
#define    NVSRAM_ECC_VALUE3_ECC_VALID_SHIFT                              30
#define    NVSRAM_ECC_VALUE3_ECC_VALID_MASK                               0x40000000
#define    NVSRAM_ECC_VALUE3_ECC_READ_SHIFT                               29
#define    NVSRAM_ECC_VALUE3_ECC_READ_MASK                                0x20000000
#define    NVSRAM_ECC_VALUE3_ECC_FAIL_SHIFT                               28
#define    NVSRAM_ECC_VALUE3_ECC_FAIL_MASK                                0x10000000
#define    NVSRAM_ECC_VALUE3_ECC_CORRECT_SHIFT                            27
#define    NVSRAM_ECC_VALUE3_ECC_CORRECT_MASK                             0x08000000
#define    NVSRAM_ECC_VALUE3_ECC_VALUE_SHIFT                              0
#define    NVSRAM_ECC_VALUE3_ECC_VALUE_MASK                               0x00FFFFFF

#define NVSRAM_ECC_VALUE4_OFFSET                                          0x00000428
#define NVSRAM_ECC_VALUE4_TYPE                                            uint32_t
#define NVSRAM_ECC_VALUE4_RESERVED_MASK                                   0x07000000
#define    NVSRAM_ECC_VALUE4_ECC_INT_SHIFT                                31
#define    NVSRAM_ECC_VALUE4_ECC_INT_MASK                                 0x80000000
#define    NVSRAM_ECC_VALUE4_ECC_VALID_SHIFT                              30
#define    NVSRAM_ECC_VALUE4_ECC_VALID_MASK                               0x40000000
#define    NVSRAM_ECC_VALUE4_ECC_READ_SHIFT                               29
#define    NVSRAM_ECC_VALUE4_ECC_READ_MASK                                0x20000000
#define    NVSRAM_ECC_VALUE4_ECC_FAIL_SHIFT                               28
#define    NVSRAM_ECC_VALUE4_ECC_FAIL_MASK                                0x10000000
#define    NVSRAM_ECC_VALUE4_ECC_CORRECT_SHIFT                            27
#define    NVSRAM_ECC_VALUE4_ECC_CORRECT_MASK                             0x08000000
#define    NVSRAM_ECC_VALUE4_ECC_VALUE_SHIFT                              0
#define    NVSRAM_ECC_VALUE4_ECC_VALUE_MASK                               0x00FFFFFF

#define NVSRAM_INT_CFG_OFFSET                                             0x00000E00
#define NVSRAM_INT_CFG_TYPE                                               uint32_t
#define NVSRAM_INT_CFG_RESERVED_MASK                                      0xFFFFFFFE
#define    NVSRAM_INT_CFG_INT_TEST_EN_SHIFT                               0
#define    NVSRAM_INT_CFG_INT_TEST_EN_MASK                                0x00000001

#define NVSRAM_INT_INPUTS_OFFSET                                          0x00000E04
#define NVSRAM_INT_INPUTS_TYPE                                            uint32_t
#define NVSRAM_INT_INPUTS_RESERVED_MASK                                   0xFFFFFC00
#define    NVSRAM_INT_INPUTS_NAND_MSYNC_SHIFT                             9
#define    NVSRAM_INT_INPUTS_NAND_MSYNC_MASK                              0x00000200
#define    NVSRAM_INT_INPUTS_NAND_ASYNC_SHIFT                             8
#define    NVSRAM_INT_INPUTS_NAND_ASYNC_MASK                              0x00000100
#define    NVSRAM_INT_INPUTS_NAND_EBIBACKOFF_SHIFT                        7
#define    NVSRAM_INT_INPUTS_NAND_EBIBACKOFF_MASK                         0x00000080
#define    NVSRAM_INT_INPUTS_NAND_EBIGNT_SHIFT                            6
#define    NVSRAM_INT_INPUTS_NAND_EBIGNT_MASK                             0x00000040
#define    NVSRAM_INT_INPUTS_SRAM_MSYNC_SHIFT                             5
#define    NVSRAM_INT_INPUTS_SRAM_MSYNC_MASK                              0x00000020
#define    NVSRAM_INT_INPUTS_SRAM_ASYNC_SHIFT                             4
#define    NVSRAM_INT_INPUTS_SRAM_ASYNC_MASK                              0x00000010
#define    NVSRAM_INT_INPUTS_SRAM_EBIBACKOFF_SHIFT                        3
#define    NVSRAM_INT_INPUTS_SRAM_EBIBACKOFF_MASK                         0x00000008
#define    NVSRAM_INT_INPUTS_SRAM_EBIGNT_SHIFT                            2
#define    NVSRAM_INT_INPUTS_SRAM_EBIGNT_MASK                             0x00000004
#define    NVSRAM_INT_INPUTS_USE_EBI_SHIFT                                1
#define    NVSRAM_INT_INPUTS_USE_EBI_MASK                                 0x00000002
#define    NVSRAM_INT_INPUTS_CSYSREQ_SHIFT                                0
#define    NVSRAM_INT_INPUTS_CSYSREQ_MASK                                 0x00000001

#define NVSRAM_INT_OUTPUTS_OFFSET                                         0x00000E08
#define NVSRAM_INT_OUTPUTS_TYPE                                           uint32_t
#define NVSRAM_INT_OUTPUTS_RESERVED_MASK                                  0xFFFFFE80
#define    NVSRAM_INT_OUTPUTS_NAND_ECC_INT_SHIFT                          8
#define    NVSRAM_INT_OUTPUTS_NAND_ECC_INT_MASK                           0x00000100
#define    NVSRAM_INT_OUTPUTS_NAND_SMC_INT_SHIFT                          6
#define    NVSRAM_INT_OUTPUTS_NAND_SMC_INT_MASK                           0x00000040
#define    NVSRAM_INT_OUTPUTS_SRAM_SMC_INT_SHIFT                          5
#define    NVSRAM_INT_OUTPUTS_SRAM_SMC_INT_MASK                           0x00000020
#define    NVSRAM_INT_OUTPUTS_SMC_INT_SHIFT                               4
#define    NVSRAM_INT_OUTPUTS_SMC_INT_MASK                                0x00000010
#define    NVSRAM_INT_OUTPUTS_NAND_EBIREQ_SHIFT                           3
#define    NVSRAM_INT_OUTPUTS_NAND_EBIREQ_MASK                            0x00000008
#define    NVSRAM_INT_OUTPUTS_SRAM_EBIREQ_SHIFT                           2
#define    NVSRAM_INT_OUTPUTS_SRAM_EBIREQ_MASK                            0x00000004
#define    NVSRAM_INT_OUTPUTS_CSYSACK_SHIFT                               1
#define    NVSRAM_INT_OUTPUTS_CSYSACK_MASK                                0x00000002
#define    NVSRAM_INT_OUTPUTS_CACTIVE_SHIFT                               0
#define    NVSRAM_INT_OUTPUTS_CACTIVE_MASK                                0x00000001

#define NVSRAM_PERIPH_ID_0_OFFSET                                         0x00000FE0
#define NVSRAM_PERIPH_ID_0_TYPE                                           uint32_t
#define NVSRAM_PERIPH_ID_0_RESERVED_MASK                                  0xFFFFFF00
#define    NVSRAM_PERIPH_ID_0_PART_NUMBER_0_SHIFT                         0
#define    NVSRAM_PERIPH_ID_0_PART_NUMBER_0_MASK                          0x000000FF

#define NVSRAM_PERIPH_ID_1_OFFSET                                         0x00000FE4
#define NVSRAM_PERIPH_ID_1_TYPE                                           uint32_t
#define NVSRAM_PERIPH_ID_1_RESERVED_MASK                                  0xFFFFFF00
#define    NVSRAM_PERIPH_ID_1_DESIGNER_0_SHIFT                            4
#define    NVSRAM_PERIPH_ID_1_DESIGNER_0_MASK                             0x000000F0
#define    NVSRAM_PERIPH_ID_1_PART_NUMBER_1_SHIFT                         0
#define    NVSRAM_PERIPH_ID_1_PART_NUMBER_1_MASK                          0x0000000F

#define NVSRAM_PERIPH_ID_2_OFFSET                                         0x00000FE8
#define NVSRAM_PERIPH_ID_2_TYPE                                           uint32_t
#define NVSRAM_PERIPH_ID_2_RESERVED_MASK                                  0xFFFFFF00
#define    NVSRAM_PERIPH_ID_2_REVISION_SHIFT                              4
#define    NVSRAM_PERIPH_ID_2_REVISION_MASK                               0x000000F0
#define    NVSRAM_PERIPH_ID_2_DESIGNER_1_SHIFT                            0
#define    NVSRAM_PERIPH_ID_2_DESIGNER_1_MASK                             0x0000000F

#define NVSRAM_PERIPH_ID_3_OFFSET                                         0x00000FEC
#define NVSRAM_PERIPH_ID_3_TYPE                                           uint32_t
#define NVSRAM_PERIPH_ID_3_RESERVED_MASK                                  0xFFFFFFFE
#define    NVSRAM_PERIPH_ID_3_INTEGRATION_CFG_SHIFT                       0
#define    NVSRAM_PERIPH_ID_3_INTEGRATION_CFG_MASK                        0x00000001

#define NVSRAM_PCELL_ID_0_OFFSET                                          0x00000FF0
#define NVSRAM_PCELL_ID_0_TYPE                                            uint32_t
#define NVSRAM_PCELL_ID_0_RESERVED_MASK                                   0xFFFFFF00
#define    NVSRAM_PCELL_ID_0_PCELL_ID_0_SHIFT                             0
#define    NVSRAM_PCELL_ID_0_PCELL_ID_0_MASK                              0x000000FF

#define NVSRAM_PCELL_ID_1_OFFSET                                          0x00000FF4
#define NVSRAM_PCELL_ID_1_TYPE                                            uint32_t
#define NVSRAM_PCELL_ID_1_RESERVED_MASK                                   0xFFFFFF00
#define    NVSRAM_PCELL_ID_1_PCELL_ID_1_SHIFT                             0
#define    NVSRAM_PCELL_ID_1_PCELL_ID_1_MASK                              0x000000FF

#define NVSRAM_PCELL_ID_2_OFFSET                                          0x00000FF8
#define NVSRAM_PCELL_ID_2_TYPE                                            uint32_t
#define NVSRAM_PCELL_ID_2_RESERVED_MASK                                   0xFFFFFF00
#define    NVSRAM_PCELL_ID_2_PCELL_ID_2_SHIFT                             0
#define    NVSRAM_PCELL_ID_2_PCELL_ID_2_MASK                              0x000000FF

#define NVSRAM_PCELL_ID_3_OFFSET                                          0x00000FFC
#define NVSRAM_PCELL_ID_3_TYPE                                            uint32_t
#define NVSRAM_PCELL_ID_3_RESERVED_MASK                                   0xFFFFFF00
#define    NVSRAM_PCELL_ID_3_PCELL_ID_3_SHIFT                             0
#define    NVSRAM_PCELL_ID_3_PCELL_ID_3_MASK                              0x000000FF

#define NVSRAM_ADDR_MASK_OFFSET                                           0x00002000
#define NVSRAM_ADDR_MASK_TYPE                                             uint32_t
#define NVSRAM_ADDR_MASK_RESERVED_MASK                                    0x00000000
#define    NVSRAM_ADDR_MASK_CS3_ADDR_MASK_SHIFT                           24
#define    NVSRAM_ADDR_MASK_CS3_ADDR_MASK_MASK                            0xFF000000
#define    NVSRAM_ADDR_MASK_CS2_ADDR_MASK_SHIFT                           16
#define    NVSRAM_ADDR_MASK_CS2_ADDR_MASK_MASK                            0x00FF0000
#define    NVSRAM_ADDR_MASK_CS1_ADDR_MASK_SHIFT                           8
#define    NVSRAM_ADDR_MASK_CS1_ADDR_MASK_MASK                            0x0000FF00
#define    NVSRAM_ADDR_MASK_CS0_ADDR_MASK_SHIFT                           0
#define    NVSRAM_ADDR_MASK_CS0_ADDR_MASK_MASK                            0x000000FF

#define NVSRAM_ADDR_MATCH_OFFSET                                          0x00002004
#define NVSRAM_ADDR_MATCH_TYPE                                            uint32_t
#define NVSRAM_ADDR_MATCH_RESERVED_MASK                                   0x00000000
#define    NVSRAM_ADDR_MATCH_CS3_ADDR_MATCH_SHIFT                         24
#define    NVSRAM_ADDR_MATCH_CS3_ADDR_MATCH_MASK                          0xFF000000
#define    NVSRAM_ADDR_MATCH_CS2_ADDR_MATCH_SHIFT                         16
#define    NVSRAM_ADDR_MATCH_CS2_ADDR_MATCH_MASK                          0x00FF0000
#define    NVSRAM_ADDR_MATCH_CS1_ADDR_MATCH_SHIFT                         8
#define    NVSRAM_ADDR_MATCH_CS1_ADDR_MATCH_MASK                          0x0000FF00
#define    NVSRAM_ADDR_MATCH_CS0_ADDR_MATCH_SHIFT                         0
#define    NVSRAM_ADDR_MATCH_CS0_ADDR_MATCH_MASK                          0x000000FF

#define NVSRAM_CONF_OFFSET                                                0x00002008
#define NVSRAM_CONF_TYPE                                                  uint32_t
#define NVSRAM_CONF_RESERVED_MASK                                         0xCF0000C0
#define    NVSRAM_CONF_NAND_CSL_SHIFT                                     29
#define    NVSRAM_CONF_NAND_CSL_MASK                                      0x20000000
#define    NVSRAM_CONF_WP_SHIFT                                           28
#define    NVSRAM_CONF_WP_MASK                                            0x10000000
#define    NVSRAM_CONF_FORCE_BUFFERABLE_CS_SHIFT                          20
#define    NVSRAM_CONF_FORCE_BUFFERABLE_CS_MASK                           0x00F00000
#define    NVSRAM_CONF_SRAM_MUX_MODE_CS_SHIFT                             16
#define    NVSRAM_CONF_SRAM_MUX_MODE_CS_MASK                              0x000F0000
#define    NVSRAM_CONF_NAND_EBI_TIMEOUT7_5_3_1_SHIFT                      12
#define    NVSRAM_CONF_NAND_EBI_TIMEOUT7_5_3_1_MASK                       0x0000F000
#define    NVSRAM_CONF_SRAM_EBI_TIMEOUT7_5_3_1_SHIFT                      8
#define    NVSRAM_CONF_SRAM_EBI_TIMEOUT7_5_3_1_MASK                       0x00000F00
#define    NVSRAM_CONF_NAND_DISABLE_SHIFT                                 5
#define    NVSRAM_CONF_NAND_DISABLE_MASK                                  0x00000020
#define    NVSRAM_CONF_SRAM_DISABLE_SHIFT                                 4
#define    NVSRAM_CONF_SRAM_DISABLE_MASK                                  0x00000010
#define    NVSRAM_CONF_SRAM_NAND_CS3_EN_SHIFT                             3
#define    NVSRAM_CONF_SRAM_NAND_CS3_EN_MASK                              0x00000008
#define       NVSRAM_CONF_SRAM_NAND_CS3_EN_CMD_NAND                       0x00000000
#define       NVSRAM_CONF_SRAM_NAND_CS3_EN_CMD_SRAM                       0x00000001
#define    NVSRAM_CONF_SRAM_NAND_CS2_EN_SHIFT                             2
#define    NVSRAM_CONF_SRAM_NAND_CS2_EN_MASK                              0x00000004
#define       NVSRAM_CONF_SRAM_NAND_CS2_EN_CMD_NAND                       0x00000000
#define       NVSRAM_CONF_SRAM_NAND_CS2_EN_CMD_SRAM                       0x00000001
#define    NVSRAM_CONF_SRAM_NAND_CS1_EN_SHIFT                             1
#define    NVSRAM_CONF_SRAM_NAND_CS1_EN_MASK                              0x00000002
#define       NVSRAM_CONF_SRAM_NAND_CS1_EN_CMD_NAND                       0x00000000
#define       NVSRAM_CONF_SRAM_NAND_CS1_EN_CMD_SRAM                       0x00000001
#define    NVSRAM_CONF_SRAM_NAND_CS0_EN_SHIFT                             0
#define    NVSRAM_CONF_SRAM_NAND_CS0_EN_MASK                              0x00000001
#define       NVSRAM_CONF_SRAM_NAND_CS0_EN_CMD_NAND                       0x00000000
#define       NVSRAM_CONF_SRAM_NAND_CS0_EN_CMD_SRAM                       0x00000001

#endif /*__REG_BCM21553_NVSRAM_H__*/
