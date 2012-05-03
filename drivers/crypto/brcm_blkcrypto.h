/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/crypto/brcm_blkcrypto.h
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

#ifndef __BRCM_BLKCRYPTO_H__
#define __BRCM_BLKCRYPTO_H__

#define __LINUX_MFD_BCM59055_A0_H

#define BLKCRYPTO_HEADER_NUM_WORDS 		5
#define BULKCRYPTO_HEADER_KEY_OFFSET		(BLKCRYPTO_HEADER_NUM_WORDS << 2)		

/* Header words index. */
#define BLKCRYPTO_HEADER_INDEX_FLAGS		0
#define BLKCRYPTO_HEADER_INDEX_MSGLEN		1
#define BLKCRYPTO_HEADER_INDEX_OFFSETS		2
#define BLKCRYPTO_HEADER_INDEX_AESLEN		3
#define BLKCRYPTO_HEADER_INDEX_AUTHLEN		4

/* First Word definitions. */
#define BLKCRYPTO_HEADER_AES_ENABLE		0x00000001
#define BLKCRYPTO_HEADER_AUTH_ENABLE		0x00000002
#define BLKCRYPTO_HEADER_AUTH_FIRST		0x00000004
#define BLKCRYPTO_HEADER_AES_ENCRYPT		0x00000008

#define BLKCRYPTO_HEADER_AES_CBC		0x00000000
#define BLKCRYPTO_HEADER_AES_CTR		0x00000010
#define BLKCRYPTO_HEADER_AES_ECB		0x00000020

#define BLKCRYPTO_HEADER_AES_KEYSIZE_128	0x00000000 /* 16 bytes */
#define BLKCRYPTO_HEADER_AES_KEYSIZE_192	0x00000100 /* 24 bytes */
#define BLKCRYPTO_HEADER_AES_KEYSIZE_256	0x00000200 /* 32 bytes */

#define BLKCRYPTO_HEADER_AUTH_CMD_HCTX		0x00001000 /* generate inner/outer hash context for hmac */
#define BLKCRYPTO_HEADER_AUTH_CMD_HASH		0x00002000 /* simple sha1 hash, the SHA1_xxx below defines the hash type */
#define BLKCRYPTO_HEADER_AUTH_CMD_HMAC		0x00004000 /* HMAC, starts from the key and default state */
#define BLKCRYPTO_HEADER_AUTH_CMD_FMAC		0x00008000 /* HMAC, starts from inner/outer hash context  */

#define BLKCRYPTO_HEADER_HASH_INIT		0x00000000 /* HASH-INIT */
#define BLKCRYPTO_HEADER_HASH_UPDATE		0x00010000 /* HASH-UPDATE */
#define BLKCRYPTO_HEADER_HASH_FINISH		0x00020000 /* HASH-FINISH */
#define BLKCRYPTO_HEADER_HASH_ALL		0x00030000 /* ALL OF 3 - will be added by HW */

#define BLKCRYPTO_HEADER_HASH_MD5		0x00080000 /* MD5 - Not supported on Athena. */
#define BLKCRYPTO_HEADER_HASH_SHA1		0x00000000 /* SHA1 */
#define BLKCRYPTO_HEADER_HASH_SHA224		0x00040000 /* SHA224 */
#define BLKCRYPTO_HEADER_HASH_SHA256		0x000C0000 /* SHA256 */ 

#define BLKCRYPTO_HEADER_CHECK_KEY_LEN		0x00100000 /* only valid with CMD_HMAC/FMAC */
                                               /* when set, auth_keysize is valid. when cleared, default key len is 20 bytes */
#define BLKCRYPTO_HEADER_LEN_OVERRIDE		0x00200000 /* only valid with CMD_HASH & SHA1_FINISH */
                                               /* when set, pad_len overrides internal len counter */
#define BLKCRYPTO_HEADER_OUTPUT_SUPPRESS	0x00400000 /* do not output plaintext in auth operation */

#define BLKCRYPTO_HEADER_AUTH_KEYSIZE_SHIFT	24      /* bits 31-24. Must be divisible by 4 */

#define AES_IV_SIZE	16
#define DMA_NODE_SIZE	0x3FF0

#define CRYPTO_CONFIG0_OFFSET                                             0x00000000
#define CRYPTO_CONFIG0_TYPE                                               UInt32
#define CRYPTO_CONFIG0_RESERVED_MASK                                      0xFFFFF02C
#define    CRYPTO_CONFIG0_WIN_RO_SHIFT                                    8
#define    CRYPTO_CONFIG0_WIN_RO_MASK                                     0x00000F00
#define    CRYPTO_CONFIG0_CACHE_PROT_SHIFT                                7
#define    CRYPTO_CONFIG0_CACHE_PROT_MASK                                 0x00000080
#define    CRYPTO_CONFIG0_RANGE_ERR_SHIFT                                 6
#define    CRYPTO_CONFIG0_RANGE_ERR_MASK                                  0x00000040
#define    CRYPTO_CONFIG0_CRYPT_SHIFT                                     4
#define    CRYPTO_CONFIG0_CRYPT_MASK                                      0x00000010
#define    CRYPTO_CONFIG0_CSIZE_SHIFT                                     0
#define    CRYPTO_CONFIG0_CSIZE_MASK                                      0x00000003

#define CRYPTO_STATUS_OFFSET                                              0x00000004
#define CRYPTO_STATUS_TYPE                                                UInt32
#define CRYPTO_STATUS_RESERVED_MASK                                       0xFFFFFFF0
#define    CRYPTO_STATUS_RANGE_STAT_SHIFT                                 3
#define    CRYPTO_STATUS_RANGE_STAT_MASK                                  0x00000008
#define    CRYPTO_STATUS_AUTHERR_STAT_SHIFT                               2
#define    CRYPTO_STATUS_AUTHERR_STAT_MASK                                0x00000004
#define    CRYPTO_STATUS_OUTDMA_SHIFT                                     1
#define    CRYPTO_STATUS_OUTDMA_MASK                                      0x00000002
#define    CRYPTO_STATUS_INDMA_SHIFT                                      0
#define    CRYPTO_STATUS_INDMA_MASK                                       0x00000001

#define CRYPTO_CTRL_OFFSET                                                0x00000008
#define CRYPTO_CTRL_TYPE                                                  UInt32
#define CRYPTO_CTRL_RESERVED_MASK                                         0xFFFFFFF0
#define    CRYPTO_CTRL_DMASINGLESTART_SHIFT                               3
#define    CRYPTO_CTRL_DMASINGLESTART_MASK                                0x00000008
#define    CRYPTO_CTRL_LDLSFR_SHIFT                                       0
#define    CRYPTO_CTRL_LDLSFR_MASK                                        0x00000007

#define CRYPTO_CONFIG1_OFFSET                                             0x0000000C
#define CRYPTO_CONFIG1_TYPE                                               UInt32
#define CRYPTO_CONFIG1_RESERVED_MASK                                      0xFFFFF002
#define    CRYPTO_CONFIG1_SWAPSIZE_SHIFT                                  11
#define    CRYPTO_CONFIG1_SWAPSIZE_MASK                                   0x00000800
#define    CRYPTO_CONFIG1_CRYPTOSWAPOUT_SHIFT                             10
#define    CRYPTO_CONFIG1_CRYPTOSWAPOUT_MASK                              0x00000400
#define    CRYPTO_CONFIG1_CRYPTOSWAPIN_SHIFT                              9
#define    CRYPTO_CONFIG1_CRYPTOSWAPIN_MASK                               0x00000200
#define    CRYPTO_CONFIG1_BULKSWAPSIZE_SHIFT                              8
#define    CRYPTO_CONFIG1_BULKSWAPSIZE_MASK                               0x00000100
#define    CRYPTO_CONFIG1_BKCRYPTSWPOUT_SHIFT                             7
#define    CRYPTO_CONFIG1_BKCRYPTSWPOUT_MASK                              0x00000080
#define    CRYPTO_CONFIG1_BKCRYPTSWPIN_SHIFT                              6
#define    CRYPTO_CONFIG1_BKCRYPTSWPIN_MASK                               0x00000040
#define    CRYPTO_CONFIG1_DISCLKGATEDMA_SHIFT                             5
#define    CRYPTO_CONFIG1_DISCLKGATEDMA_MASK                              0x00000020
#define    CRYPTO_CONFIG1_DISCLKGATECACHE_SHIFT                           4
#define    CRYPTO_CONFIG1_DISCLKGATECACHE_MASK                            0x00000010
#define    CRYPTO_CONFIG1_DMASOFTRESET_SHIFT                              2
#define    CRYPTO_CONFIG1_DMASOFTRESET_MASK                               0x0000000C
#define    CRYPTO_CONFIG1_CACHE_EN_SHIFT                                  0
#define    CRYPTO_CONFIG1_CACHE_EN_MASK                                   0x00000001

#define CRYPTO_INT_STATUS_OFFSET                                          0x00000014
#define CRYPTO_INT_STATUS_TYPE                                            UInt32
#define CRYPTO_INT_STATUS_RESERVED_MASK                                   0xFFFFFFE0
#define    CRYPTO_INT_STATUS_DWRAP_STAT_SHIFT                             4
#define    CRYPTO_INT_STATUS_DWRAP_STAT_MASK                              0x00000010
#define    CRYPTO_INT_STATUS_DPROT_STAT_SHIFT                             3
#define    CRYPTO_INT_STATUS_DPROT_STAT_MASK                              0x00000008
#define    CRYPTO_INT_STATUS_FLUSH_STAT_SHIFT                             2
#define    CRYPTO_INT_STATUS_FLUSH_STAT_MASK                              0x00000004
#define    CRYPTO_INT_STATUS_DESTDMADONE_SHIFT                            1
#define    CRYPTO_INT_STATUS_DESTDMADONE_MASK                             0x00000002
#define    CRYPTO_INT_STATUS_SRCDMADONE_SHIFT                             0
#define    CRYPTO_INT_STATUS_SRCDMADONE_MASK                              0x00000001

#define CRYPTO_INT_MASK_OFFSET                                            0x00000018
#define CRYPTO_INT_MASK_TYPE                                              UInt32
#define CRYPTO_INT_MASK_RESERVED_MASK                                     0xFFFFFFE0
#define    CRYPTO_INT_MASK_DWRAP_STAT_SHIFT                               4
#define    CRYPTO_INT_MASK_DWRAP_STAT_MASK                                0x00000010
#define    CRYPTO_INT_MASK_DPROT_STAT_SHIFT                               3
#define    CRYPTO_INT_MASK_DPROT_STAT_MASK                                0x00000008
#define    CRYPTO_INT_MASK_FLUSH_STAT_SHIFT                               2
#define    CRYPTO_INT_MASK_FLUSH_STAT_MASK                                0x00000004
#define    CRYPTO_INT_MASK_DESTDMADONE_SHIFT                              1
#define    CRYPTO_INT_MASK_DESTDMADONE_MASK                               0x00000002
#define    CRYPTO_INT_MASK_SRCDMADONE_SHIFT                               0
#define    CRYPTO_INT_MASK_SRCDMADONE_MASK                                0x00000001

#define CRYPTO_INT_CLR_OFFSET                                             0x0000001C
#define CRYPTO_INT_CLR_TYPE                                               UInt32
#define CRYPTO_INT_CLR_RESERVED_MASK                                      0xFFFFFFE0
#define    CRYPTO_INT_CLR_DWRAP_STAT_SHIFT                                4
#define    CRYPTO_INT_CLR_DWRAP_STAT_MASK                                 0x00000010
#define    CRYPTO_INT_CLR_DPROT_STAT_SHIFT                                3
#define    CRYPTO_INT_CLR_DPROT_STAT_MASK                                 0x00000008
#define    CRYPTO_INT_CLR_FLUSH_STAT_SHIFT                                2
#define    CRYPTO_INT_CLR_FLUSH_STAT_MASK                                 0x00000004
#define    CRYPTO_INT_CLR_DESTDMADONE_SHIFT                               1
#define    CRYPTO_INT_CLR_DESTDMADONE_MASK                                0x00000002
#define    CRYPTO_INT_CLR_SRCDMADONE_SHIFT                                0
#define    CRYPTO_INT_CLR_SRCDMADONE_MASK                                 0x00000001

#define CRYPTO_LOCK0_OFFSET                                               0x00000020
#define CRYPTO_LOCK0_TYPE                                                 UInt32
#define CRYPTO_LOCK0_RESERVED_MASK                                        0x00000000
#define    CRYPTO_LOCK0_LOCK_SHIFT                                        0
#define    CRYPTO_LOCK0_LOCK_MASK                                         0xFFFFFFFF

#define CRYPTO_LOCK1_OFFSET                                               0x00000024
#define CRYPTO_LOCK1_TYPE                                                 UInt32
#define CRYPTO_LOCK1_RESERVED_MASK                                        0x00000000
#define    CRYPTO_LOCK1_LOCK_SHIFT                                        0
#define    CRYPTO_LOCK1_LOCK_MASK                                         0xFFFFFFFF

#define CRYPTO_LOCK2_OFFSET                                               0x00000028
#define CRYPTO_LOCK2_TYPE                                                 UInt32
#define CRYPTO_LOCK2_RESERVED_MASK                                        0x00000000
#define    CRYPTO_LOCK2_LOCK_SHIFT                                        0
#define    CRYPTO_LOCK2_LOCK_MASK                                         0xFFFFFFFF

#define CRYPTO_LOCK3_OFFSET                                               0x0000002C
#define CRYPTO_LOCK3_TYPE                                                 UInt32
#define CRYPTO_LOCK3_RESERVED_MASK                                        0x00000000
#define    CRYPTO_LOCK3_LOCK_SHIFT                                        0
#define    CRYPTO_LOCK3_LOCK_MASK                                         0xFFFFFFFF

#define CRYPTO_HMAC_DIG0_OFFSET                                           0x00000030
#define CRYPTO_HMAC_DIG0_TYPE                                             UInt32
#define CRYPTO_HMAC_DIG0_RESERVED_MASK                                    0x0000001F
#define    CRYPTO_HMAC_DIG0_HMAC_BASE0_SHIFT                              5
#define    CRYPTO_HMAC_DIG0_HMAC_BASE0_MASK                               0xFFFFFFE0

#define CRYPTO_HMAC_DIG1_OFFSET                                           0x00000034
#define CRYPTO_HMAC_DIG1_TYPE                                             UInt32
#define CRYPTO_HMAC_DIG1_RESERVED_MASK                                    0x0000001F
#define    CRYPTO_HMAC_DIG1_HMAC_BASE1_SHIFT                              5
#define    CRYPTO_HMAC_DIG1_HMAC_BASE1_MASK                               0xFFFFFFE0

#define CRYPTO_HMAC_DIG2_OFFSET                                           0x00000038
#define CRYPTO_HMAC_DIG2_TYPE                                             UInt32
#define CRYPTO_HMAC_DIG2_RESERVED_MASK                                    0x0000001F
#define    CRYPTO_HMAC_DIG2_HMAC_BASE2_SHIFT                              5
#define    CRYPTO_HMAC_DIG2_HMAC_BASE2_MASK                               0xFFFFFFE0

#define CRYPTO_HMAC_DIG3_OFFSET                                           0x0000003C
#define CRYPTO_HMAC_DIG3_TYPE                                             UInt32
#define CRYPTO_HMAC_DIG3_RESERVED_MASK                                    0x0000001F
#define    CRYPTO_HMAC_DIG3_HMAC_BASE3_SHIFT                              5
#define    CRYPTO_HMAC_DIG3_HMAC_BASE3_MASK                               0xFFFFFFE0

#define CRYPTO_WIN_BASE0_OFFSET                                           0x00000040
#define CRYPTO_WIN_BASE0_TYPE                                             UInt32
#define CRYPTO_WIN_BASE0_RESERVED_MASK                                    0x000000FF
#define    CRYPTO_WIN_BASE0_CLR_BASE_SHIFT                                24
#define    CRYPTO_WIN_BASE0_CLR_BASE_MASK                                 0xFF000000
#define    CRYPTO_WIN_BASE0_CLR_ADDR_SHIFT                                8
#define    CRYPTO_WIN_BASE0_CLR_ADDR_MASK                                 0x00FFFF00

#define CRYPTO_WIN_BASE1_OFFSET                                           0x00000044
#define CRYPTO_WIN_BASE1_TYPE                                             UInt32
#define CRYPTO_WIN_BASE1_RESERVED_MASK                                    0x000000FF
#define    CRYPTO_WIN_BASE1_CLR_BASE_SHIFT                                24
#define    CRYPTO_WIN_BASE1_CLR_BASE_MASK                                 0xFF000000
#define    CRYPTO_WIN_BASE1_CLR_ADDR_SHIFT                                8
#define    CRYPTO_WIN_BASE1_CLR_ADDR_MASK                                 0x00FFFF00

#define CRYPTO_WIN_BASE2_OFFSET                                           0x00000048
#define CRYPTO_WIN_BASE2_TYPE                                             UInt32
#define CRYPTO_WIN_BASE2_RESERVED_MASK                                    0x000000FF
#define    CRYPTO_WIN_BASE2_CLR_BASE_SHIFT                                24
#define    CRYPTO_WIN_BASE2_CLR_BASE_MASK                                 0xFF000000
#define    CRYPTO_WIN_BASE2_CLR_ADDR_SHIFT                                8
#define    CRYPTO_WIN_BASE2_CLR_ADDR_MASK                                 0x00FFFF00

#define CRYPTO_WIN_BASE3_OFFSET                                           0x0000004C
#define CRYPTO_WIN_BASE3_TYPE                                             UInt32
#define CRYPTO_WIN_BASE3_RESERVED_MASK                                    0x000000FF
#define    CRYPTO_WIN_BASE3_CLR_BASE_SHIFT                                24
#define    CRYPTO_WIN_BASE3_CLR_BASE_MASK                                 0xFF000000
#define    CRYPTO_WIN_BASE3_CLR_ADDR_SHIFT                                8
#define    CRYPTO_WIN_BASE3_CLR_ADDR_MASK                                 0x00FFFF00

#define CRYPTO_WIN_SIZE0_OFFSET                                           0x00000050
#define CRYPTO_WIN_SIZE0_TYPE                                             UInt32
#define CRYPTO_WIN_SIZE0_RESERVED_MASK                                    0xFF0000FF
#define    CRYPTO_WIN_SIZE0_WIN_MASK0_SHIFT                               8
#define    CRYPTO_WIN_SIZE0_WIN_MASK0_MASK                                0x00FFFF00

#define CRYPTO_WIN_SIZE1_OFFSET                                           0x00000054
#define CRYPTO_WIN_SIZE1_TYPE                                             UInt32
#define CRYPTO_WIN_SIZE1_RESERVED_MASK                                    0xFF0000FF
#define    CRYPTO_WIN_SIZE1_WIN_MASK1_SHIFT                               8
#define    CRYPTO_WIN_SIZE1_WIN_MASK1_MASK                                0x00FFFF00

#define CRYPTO_WIN_SIZE2_OFFSET                                           0x00000058
#define CRYPTO_WIN_SIZE2_TYPE                                             UInt32
#define CRYPTO_WIN_SIZE2_RESERVED_MASK                                    0xFF0000FF
#define    CRYPTO_WIN_SIZE2_WIN_MASK2_SHIFT                               8
#define    CRYPTO_WIN_SIZE2_WIN_MASK2_MASK                                0x00FFFF00

#define CRYPTO_WIN_SIZE3_OFFSET                                           0x0000005C
#define CRYPTO_WIN_SIZE3_TYPE                                             UInt32
#define CRYPTO_WIN_SIZE3_RESERVED_MASK                                    0xFF0000FF
#define    CRYPTO_WIN_SIZE3_WIN_MASK3_SHIFT                               8
#define    CRYPTO_WIN_SIZE3_WIN_MASK3_MASK                                0x00FFFF00

#define CRYPTO_AES_KEY_0_OFFSET                                           0x00000060
#define CRYPTO_AES_KEY_0_TYPE                                             UInt32
#define CRYPTO_AES_KEY_0_RESERVED_MASK                                    0x00000000
#define    CRYPTO_AES_KEY_0_AES_KEY_SHIFT                                 0
#define    CRYPTO_AES_KEY_0_AES_KEY_MASK                                  0xFFFFFFFF

#define CRYPTO_AES_KEY_1_OFFSET                                           0x00000064
#define CRYPTO_AES_KEY_1_TYPE                                             UInt32
#define CRYPTO_AES_KEY_1_RESERVED_MASK                                    0x00000000
#define    CRYPTO_AES_KEY_1_AES_KEY_SHIFT                                 0
#define    CRYPTO_AES_KEY_1_AES_KEY_MASK                                  0xFFFFFFFF

#define CRYPTO_AES_KEY_2_OFFSET                                           0x00000068
#define CRYPTO_AES_KEY_2_TYPE                                             UInt32
#define CRYPTO_AES_KEY_2_RESERVED_MASK                                    0x00000000
#define    CRYPTO_AES_KEY_2_AES_KEY_SHIFT                                 0
#define    CRYPTO_AES_KEY_2_AES_KEY_MASK                                  0xFFFFFFFF

#define CRYPTO_AES_KEY_3_OFFSET                                           0x0000006C
#define CRYPTO_AES_KEY_3_TYPE                                             UInt32
#define CRYPTO_AES_KEY_3_RESERVED_MASK                                    0x00000000
#define    CRYPTO_AES_KEY_3_AES_KEY_SHIFT                                 0
#define    CRYPTO_AES_KEY_3_AES_KEY_MASK                                  0xFFFFFFFF

#define CRYPTO_AUTH_KEY_0_OFFSET                                          0x00000070
#define CRYPTO_AUTH_KEY_0_TYPE                                            UInt32
#define CRYPTO_AUTH_KEY_0_RESERVED_MASK                                   0x00000000
#define    CRYPTO_AUTH_KEY_0_AUTH_IKEY_SHIFT                              0
#define    CRYPTO_AUTH_KEY_0_AUTH_IKEY_MASK                               0xFFFFFFFF

#define CRYPTO_AUTH_KEY_1_OFFSET                                          0x00000074
#define CRYPTO_AUTH_KEY_1_TYPE                                            UInt32
#define CRYPTO_AUTH_KEY_1_RESERVED_MASK                                   0x00000000
#define    CRYPTO_AUTH_KEY_1_AUTH_IKEY_SHIFT                              0
#define    CRYPTO_AUTH_KEY_1_AUTH_IKEY_MASK                               0xFFFFFFFF

#define CRYPTO_AUTH_KEY_2_OFFSET                                          0x00000078
#define CRYPTO_AUTH_KEY_2_TYPE                                            UInt32
#define CRYPTO_AUTH_KEY_2_RESERVED_MASK                                   0x00000000
#define    CRYPTO_AUTH_KEY_2_AUTH_IKEY_SHIFT                              0
#define    CRYPTO_AUTH_KEY_2_AUTH_IKEY_MASK                               0xFFFFFFFF

#define CRYPTO_AUTH_KEY_3_OFFSET                                          0x0000007C
#define CRYPTO_AUTH_KEY_3_TYPE                                            UInt32
#define CRYPTO_AUTH_KEY_3_RESERVED_MASK                                   0x00000000
#define    CRYPTO_AUTH_KEY_3_AUTH_IKEY_SHIFT                              0
#define    CRYPTO_AUTH_KEY_3_AUTH_IKEY_MASK                               0xFFFFFFFF

#define CRYPTO_AUTH_KEY_4_OFFSET                                          0x00000080
#define CRYPTO_AUTH_KEY_4_TYPE                                            UInt32
#define CRYPTO_AUTH_KEY_4_RESERVED_MASK                                   0x00000000
#define    CRYPTO_AUTH_KEY_4_AUTH_IKEY_SHIFT                              0
#define    CRYPTO_AUTH_KEY_4_AUTH_IKEY_MASK                               0xFFFFFFFF

#define CRYPTO_DMA_SLV_IN_OFFSET                                          0x00000084
#define CRYPTO_DMA_SLV_IN_TYPE                                            UInt32
#define CRYPTO_DMA_SLV_IN_RESERVED_MASK                                   0x00000000
#define    CRYPTO_DMA_SLV_IN_INPUT_SHIFT                                  0
#define    CRYPTO_DMA_SLV_IN_INPUT_MASK                                   0xFFFFFFFF

#define CRYPTO_DMA_SLV_OUT_OFFSET                                         0x00000088
#define CRYPTO_DMA_SLV_OUT_TYPE                                           UInt32
#define CRYPTO_DMA_SLV_OUT_RESERVED_MASK                                  0x00000000
#define    CRYPTO_DMA_SLV_OUT_OUTPUT_SHIFT                                0
#define    CRYPTO_DMA_SLV_OUT_OUTPUT_MASK                                 0xFFFFFFFF

#define CRYPTO_DMA_DESC_OFFSET                                            0x0000008C
#define CRYPTO_DMA_DESC_TYPE                                              UInt32
#define CRYPTO_DMA_DESC_RESERVED_MASK                                     0x80008000
#define    CRYPTO_DMA_DESC_DESTWORDADDR_SHIFT                             16
#define    CRYPTO_DMA_DESC_DESTWORDADDR_MASK                              0x7FFF0000
#define    CRYPTO_DMA_DESC_SRCWORDADDR_SHIFT                              0
#define    CRYPTO_DMA_DESC_SRCWORDADDR_MASK                               0x00007FFF

#define CRYPTO_DMA_LDESC_OFFSET                                           0x00000090
#define CRYPTO_DMA_LDESC_TYPE                                             UInt32
#define CRYPTO_DMA_LDESC_RESERVED_MASK                                    0x00008000
#define    CRYPTO_DMA_LDESC_BLKLNKLSTCNT_SHIFT                            20
#define    CRYPTO_DMA_LDESC_BLKLNKLSTCNT_MASK                             0xFFF00000
#define    CRYPTO_DMA_LDESC_CHAIN_SHIFT                                   19
#define    CRYPTO_DMA_LDESC_CHAIN_MASK                                    0x00080000
#define    CRYPTO_DMA_LDESC_CRYPTBY_SHIFT                                 18
#define    CRYPTO_DMA_LDESC_CRYPTBY_MASK                                  0x00040000
#define    CRYPTO_DMA_LDESC_DEST_SHIFT                                    17
#define    CRYPTO_DMA_LDESC_DEST_MASK                                     0x00020000
#define    CRYPTO_DMA_LDESC_SRC_SHIFT                                     16
#define    CRYPTO_DMA_LDESC_SRC_MASK                                      0x00010000
#define    CRYPTO_DMA_LDESC_SCRATCHLGTH_SHIFT                             0
#define    CRYPTO_DMA_LDESC_SCRATCHLGTH_MASK                              0x00007FFF

#define CRYPTO_LSFR_SEED_OFFSET                                           0x00000094
#define CRYPTO_LSFR_SEED_TYPE                                             UInt32
#define CRYPTO_LSFR_SEED_RESERVED_MASK                                    0xFFFF0000
#define    CRYPTO_LSFR_SEED_LSFR_SEED_SHIFT                               0
#define    CRYPTO_LSFR_SEED_LSFR_SEED_MASK                                0x0000FFFF

#define CRYPTO_DATA_RTOT_OFFSET                                           0x000000C0
#define CRYPTO_DATA_RTOT_TYPE                                             UInt32
#define CRYPTO_DATA_RTOT_RESERVED_MASK                                    0x00000000
#define    CRYPTO_DATA_RTOT_DATA_RTOT_SHIFT                               0
#define    CRYPTO_DATA_RTOT_DATA_RTOT_MASK                                0xFFFFFFFF

#define CRYPTO_DATA_WTOT_OFFSET                                           0x000000C4
#define CRYPTO_DATA_WTOT_TYPE                                             UInt32
#define CRYPTO_DATA_WTOT_RESERVED_MASK                                    0x00000000
#define    CRYPTO_DATA_WTOT_DATA_WTOT_SHIFT                               0
#define    CRYPTO_DATA_WTOT_DATA_WTOT_MASK                                0xFFFFFFFF

#define CRYPTO_DATA_RMISS_OFFSET                                          0x000000C8
#define CRYPTO_DATA_RMISS_TYPE                                            UInt32
#define CRYPTO_DATA_RMISS_RESERVED_MASK                                   0x00000000
#define    CRYPTO_DATA_RMISS_DATA_RMISS_SHIFT                             0
#define    CRYPTO_DATA_RMISS_DATA_RMISS_MASK                              0xFFFFFFFF

#define CRYPTO_DATA_WMISS_OFFSET                                          0x000000CC
#define CRYPTO_DATA_WMISS_TYPE                                            UInt32
#define CRYPTO_DATA_WMISS_RESERVED_MASK                                   0x00000000
#define    CRYPTO_DATA_WMISS_DATA_WMISS_SHIFT                             0
#define    CRYPTO_DATA_WMISS_DATA_WMISS_MASK                              0xFFFFFFFF

#define CRYPTO_DEBUG_OFFSET                                               0x000000E0
#define CRYPTO_DEBUG_TYPE                                                 UInt32
#define CRYPTO_DEBUG_RESERVED_MASK                                        0x00000000
#define    CRYPTO_DEBUG_DEBUG_SHIFT                                       0
#define    CRYPTO_DEBUG_DEBUG_MASK                                        0xFFFFFFFF


#endif // __BRCM_BLKCRYPTO_H__
