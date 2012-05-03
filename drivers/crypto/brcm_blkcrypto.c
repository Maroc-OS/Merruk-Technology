/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/crypto/brcm_blkcrypto.c
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <plat/mobcom_types.h>
#include <plat/dma_drv.h>
#include <plat/dma.h>
#include <mach/hardware.h>
#include <crypto/aes.h>
#include <linux/crypto.h>
#include <crypto/algapi.h>
#include "brcm_blkcrypto.h"

#define b_host_2_be32( __buf, __size)       \
do {                                        \
	u32 *b = (u32 *)(__buf);            \
	u32 i;                              \
	for (i = 0; i < __size; i++, b++) { \
	*b = cpu_to_be32(*b);               \
  }                                         \
}while (0)

#define BLKCRYPTO_OP_ENCRYPT 1
#define BLKCRYPTO_OP_DECRYPT 0
#define BLKCRYPTO_AES_QUEUE_LENGTH 10

enum blkcrypto_aes_mode {
	BLKCRYPTO_AES_CBC = 0,
	BLKCRYPTO_AES_CTR,
	BLKCRYPTO_AES_ECB,
};

enum blkcrypto_state {
	NEW = 0,
	QUEUED,
	DONE,
};

struct bulkcrypto_channel_ctx {
	Dma_Chan_Info		dma_info;
	Dma_Buffer_List		*buff_list;
	Dma_Data		dma_data;
	DMADRV_LLI_T		dmaLLI;
	u32			numLLI;
};

struct bulkcrypto_dma_ctx {
	struct bulkcrypto_channel_ctx	rx_channel;
	struct bulkcrypto_channel_ctx	tx_channel;
};

struct blkcrypto_request_context {
	u8	blkcrypto_hdr[DMA_NODE_SIZE];
	u32	state;
};

struct blkcrypto_crypto_device {
	u32				iobase;
	u32				phy_base;
	u32				rx_channel;
	u32				tx_channel;
	atomic_t			dma_rx;
	atomic_t			dma_tx;
	spinlock_t			lock;
	struct list_head	list;
	struct clk			*clk;
	struct crypto_alg		algo;
	struct crypto_queue 		queue;
	struct ablkcipher_request	*curnt;
	struct bulkcrypto_dma_ctx       dma_ctx;
};

struct blkcrypto_aes_ctx {
	u32	key_enc[AES_MAX_KEYLENGTH_U32];
	u32	key_len;
	struct blkcrypto_crypto_device *cdev;
};

static LIST_HEAD(blkcrypto_drv_list);
static DEFINE_SPINLOCK(blkcrypto_drv_lock);

static void blkcrypto_dma_callback_rx(unsigned long data);
static void blkcrypto_dma_callback_tx(unsigned long data);

static void blkcrypto_dma_handler_rx(DMADRV_CALLBACK_STATUS_t Err)
{
	struct blkcrypto_crypto_device *cdev = NULL;

	pr_debug("%s: Entry\n",__FUNCTION__);

	list_for_each_entry(cdev, &blkcrypto_drv_list, list) {
			break;
	}

	blkcrypto_dma_callback_rx((unsigned long)cdev);
}

static void blkcrypto_dma_handler_tx(DMADRV_CALLBACK_STATUS_t Err)
{
	struct blkcrypto_crypto_device *cdev = NULL;

	pr_debug("%s: Entry\n",__FUNCTION__);
	
	list_for_each_entry(cdev, &blkcrypto_drv_list, list) {
			break;
	}

	blkcrypto_dma_callback_tx((unsigned long)cdev);
}

#ifdef DEBUG 
static void hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}
#endif


/*
 * derive number of elements in scatterlist
 */
static int get_sg_count(struct scatterlist *sg_list, int nbytes)
{
	struct scatterlist *sg = sg_list;
	int sg_nents = 0;

	while (nbytes) {
		sg_nents++;
		if (sg->length > nbytes)
			break;
		nbytes -= sg->length;
		sg = sg_next(sg);
	}

	return sg_nents;
}

static int blkcrypto_dma_init(struct blkcrypto_crypto_device *cdev)
{

        pr_debug("%s:Entry.\n",__FUNCTION__);

        /* Obtain DMA channel */
        if(DMADRV_Obtain_Channel(DMA_CLIENT_MEMORY, DMA_CLIENT_BULK_CRYPT_IN,
                (DMA_CHANNEL *)&(cdev->rx_channel)) != DMADRV_STATUS_OK) {
                pr_err("DMADRV_Obtain_Channel failed for Crypt in channel\n");
                return -EPERM;
        }

        /* Setup DMA info. */
        cdev->dma_ctx.rx_channel.dma_info.srcID = DMA_CLIENT_MEMORY;
        cdev->dma_ctx.rx_channel.dma_info.dstID = DMA_CLIENT_BULK_CRYPT_IN;
        cdev->dma_ctx.rx_channel.dma_info.type  = DMA_FCTRL_MEM_TO_PERI;
        cdev->dma_ctx.rx_channel.dma_info.alignment = DMA_ALIGNMENT_32;
        cdev->dma_ctx.rx_channel.dma_info.srcBstSize = DMA_BURST_SIZE_4;
        cdev->dma_ctx.rx_channel.dma_info.dstBstSize = DMA_BURST_SIZE_4;
        cdev->dma_ctx.rx_channel.dma_info.srcDataWidth = DMA_DATA_SIZE_32BIT;
        cdev->dma_ctx.rx_channel.dma_info.dstDataWidth = DMA_DATA_SIZE_32BIT;
        cdev->dma_ctx.rx_channel.dma_info.incMode = DMA_INC_MODE_SRC;
        cdev->dma_ctx.rx_channel.dma_info.xferCompleteCb = (DmaDrv_Callback)blkcrypto_dma_handler_rx;
        cdev->dma_ctx.rx_channel.dma_info.freeChan = TRUE;
        cdev->dma_ctx.rx_channel.dma_info.priority = 0;
        cdev->dma_ctx.rx_channel.dma_info.bCircular = FALSE;


        if(DMADRV_Obtain_Channel(DMA_CLIENT_BULK_CRYPT_OUT, DMA_CLIENT_MEMORY,
                                (DMA_CHANNEL *)&(cdev->tx_channel)) != DMADRV_STATUS_OK) {
                pr_err("DMADRV_Obtain_Channel failed for Crypt out channel\n");
                DMADRV_Release_Channel(cdev->rx_channel);
                return -EPERM;
        }

        cdev->dma_ctx.tx_channel.dma_info.srcID = DMA_CLIENT_BULK_CRYPT_OUT;
        cdev->dma_ctx.tx_channel.dma_info.dstID = DMA_CLIENT_MEMORY;
        cdev->dma_ctx.tx_channel.dma_info.type  = DMA_FCTRL_PERI_TO_MEM_CTRL_PERI;
        cdev->dma_ctx.tx_channel.dma_info.alignment = DMA_ALIGNMENT_32;
        cdev->dma_ctx.tx_channel.dma_info.srcBstSize = DMA_BURST_SIZE_4;
        cdev->dma_ctx.tx_channel.dma_info.dstBstSize = DMA_BURST_SIZE_4;
        cdev->dma_ctx.tx_channel.dma_info.srcDataWidth = DMA_DATA_SIZE_32BIT;
        cdev->dma_ctx.tx_channel.dma_info.dstDataWidth = DMA_DATA_SIZE_32BIT;
        cdev->dma_ctx.tx_channel.dma_info.incMode = DMA_INC_MODE_DST;
        cdev->dma_ctx.tx_channel.dma_info.xferCompleteCb = (DmaDrv_Callback)blkcrypto_dma_handler_tx;
        cdev->dma_ctx.tx_channel.dma_info.freeChan = TRUE;
        cdev->dma_ctx.tx_channel.dma_info.priority = 0;
        cdev->dma_ctx.tx_channel.dma_info.bCircular = FALSE;

        /* Configure DMA channel */
        if (DMADRV_Config_Channel(cdev->rx_channel, &cdev->dma_ctx.rx_channel.dma_info)
            != DMADRV_STATUS_OK) {
                DMADRV_Release_Channel(cdev->rx_channel);
                DMADRV_Release_Channel(cdev->tx_channel);
                pr_err("DMADRV_Config_Channel Failed for Crypto Rx channel\n");
                return -EPERM;
        }

        if (DMADRV_Config_Channel(cdev->tx_channel, &cdev->dma_ctx.tx_channel.dma_info)
            != DMADRV_STATUS_OK) {
                pr_err("DMADRV_Config_Channel Failed for Crypto Tx channel\n");
                DMADRV_Release_Channel(cdev->rx_channel);
                DMADRV_Release_Channel(cdev->tx_channel);
                return -EPERM;
        }

        pr_debug("%s:Exit.\n",__FUNCTION__);
        return 0;
}

static int blkcrypto_dma_setup(struct ablkcipher_request *req)
{
        Dma_Buffer_List *rx_buff_list = NULL, *tx_buff_list = NULL;
	struct blkcrypto_aes_ctx *aes_ctx = crypto_tfm_ctx(req->base.tfm);
	struct blkcrypto_crypto_device *cdev = aes_ctx->cdev;
	struct blkcrypto_request_context *rctx = ablkcipher_request_ctx(req);
        int     sg_cnt, nbytes;
        struct scatterlist *sg;
        u32     i = 0;
        void *ptr;

        pr_debug("%s:Entry.\n",__FUNCTION__);

        /* Setup Memory to BulkCrypto DMA. */
        /* Length of input sg list? */
        sg_cnt = get_sg_count(req->src, req->nbytes);

        rx_buff_list = (Dma_Buffer_List *) kzalloc((sg_cnt+1)*sizeof(Dma_Buffer_List), GFP_KERNEL);
        tx_buff_list = (Dma_Buffer_List *) kzalloc(sg_cnt*sizeof(Dma_Buffer_List), GFP_KERNEL);

        if((!rx_buff_list) || (!tx_buff_list)) {
                pr_err("%s:Memory allocation for dma buffers failed.\n",__FUNCTION__);
                goto exit;
        }

        rx_buff_list[0].buffers[0].srcAddr = dma_map_single(NULL, rctx->blkcrypto_hdr,
                                                ((BLKCRYPTO_HEADER_NUM_WORDS << 2) + AES_IV_SIZE + aes_ctx->key_len),
                                                DMA_TO_DEVICE );
        if (dma_mapping_error(NULL, rx_buff_list[0].buffers[0].srcAddr)) {
                pr_err("dma_map_single failed.\n");
                goto exit;
        }

        rx_buff_list[0].buffers[0].destAddr = (u32)(cdev->phy_base + CRYPTO_DMA_SLV_IN_OFFSET);
        rx_buff_list[0].buffers[0].length  = ((BLKCRYPTO_HEADER_NUM_WORDS<<2) + AES_IV_SIZE +  aes_ctx->key_len);
        if(rx_buff_list[0].buffers[0].length%4)
                rx_buff_list[0].buffers[0].length += (4 - rx_buff_list[0].buffers[0].length%4);
        rx_buff_list[0].buffers[0].bRepeat = 0;
        rx_buff_list[0].buffers[0].interrupt = 0;

        sg = req->src;
        i = 1; nbytes = req->nbytes;

        while(nbytes > 0) {
                u32 len = min((u32)nbytes,sg->length);

                ptr = page_address(sg_page(sg)) + sg->offset;
                rx_buff_list[i].buffers[0].srcAddr = (u32)dma_map_single(NULL, ptr, len, DMA_TO_DEVICE);
                if (dma_mapping_error(NULL, rx_buff_list[i].buffers[0].srcAddr)) {
                        pr_err("dma_map_single failed.\n");
                        goto exit;
                }

                rx_buff_list[i].buffers[0].destAddr = (u32)(cdev->phy_base + CRYPTO_DMA_SLV_IN_OFFSET);
                rx_buff_list[i].buffers[0].length  = len;
                if(rx_buff_list[i].buffers[0].length%4)
                        rx_buff_list[i].buffers[0].length += (4 - rx_buff_list[i].buffers[0].length%4);
                rx_buff_list[i].buffers[0].bRepeat = 0;
                rx_buff_list[i].buffers[0].interrupt = 0;

                nbytes -= len;
                i++;
                sg = sg_next(sg);
        }
        rx_buff_list[--i].buffers[0].interrupt = 1;
        cdev->dma_ctx.rx_channel.dma_data.numBuffer = sg_cnt + 1;
        cdev->dma_ctx.rx_channel.buff_list = rx_buff_list;
        cdev->dma_ctx.rx_channel.dma_data.pBufList = (Dma_Buffer_List *)rx_buff_list;

        if (DMADRV_Bind_Data_Ex(cdev->rx_channel, &(cdev->dma_ctx.rx_channel.dma_data), &(cdev->dma_ctx.rx_channel.dmaLLI)) !=
            DMADRV_STATUS_OK) {
                pr_err("DMADRV_Bind_Data Failed for Crypto RX channel\n");
                goto exit;
        }

        cdev->dma_ctx.rx_channel.numLLI = sg_cnt + 1;

        /* Setup BulkCrypto to Memory DMA. */
        sg_cnt = get_sg_count(req->dst, req->nbytes);

        sg = req->dst;
        i = 0; nbytes = req->nbytes;
        while(nbytes > 0) {
                u32 len = min(nbytes, DMA_NODE_SIZE);
                ptr = page_address(sg_page(sg)) + sg->offset;
                tx_buff_list[i].buffers[0].srcAddr = (u32)(cdev->phy_base + CRYPTO_DMA_SLV_OUT_OFFSET);
                tx_buff_list[i].buffers[0].destAddr = (u32)dma_map_single(NULL, ptr, len, DMA_FROM_DEVICE);
                if (dma_mapping_error(NULL, tx_buff_list[i].buffers[0].destAddr)) {
                        pr_err("dma_map_single failed.\n");
                        goto exit;
                }

                tx_buff_list[i].buffers[0].length  = len;
                if(tx_buff_list[i].buffers[0].length%4)
                        tx_buff_list[i].buffers[0].length += (4 - tx_buff_list[i].buffers[0].length%4);
                tx_buff_list[i].buffers[0].bRepeat = 0;
                tx_buff_list[i].buffers[0].interrupt = 0;

                nbytes -= len;
                i++;
                sg = sg_next(sg);
        }
        tx_buff_list[--i].buffers[0].interrupt = 1;
        cdev->dma_ctx.tx_channel.buff_list = tx_buff_list;
        cdev->dma_ctx.tx_channel.dma_data.pBufList = (Dma_Buffer_List *)tx_buff_list;
        cdev->dma_ctx.tx_channel.numLLI = i+1;
        cdev->dma_ctx.tx_channel.dma_data.numBuffer = cdev->dma_ctx.tx_channel.numLLI;

        if(cdev->dma_ctx.tx_channel.numLLI > 1) {
                if (DMADRV_Bind_Data_Ex(cdev->tx_channel,
                                        &(cdev->dma_ctx.tx_channel.dma_data),
                                        &(cdev->dma_ctx.tx_channel.dmaLLI)) !=
                        DMADRV_STATUS_OK) {
                        pr_err("DMADRV_Bind_Data Failed for Crypto TX channel\n");
                        goto exit;
                }
        }
        else {
                if (DMADRV_Bind_Data(cdev->tx_channel, &(cdev->dma_ctx.tx_channel.dma_data)) !=
                        DMADRV_STATUS_OK) {
                        pr_err("DMADRV_Bind_Data Failed for Crypto TX channel\n");
                        goto exit;
                }
                cdev->dma_ctx.tx_channel.dmaLLI = NULL;
        }

        pr_debug("%s:Exit.\n",__FUNCTION__);

        return 0;

        exit:
                DMADRV_Release_Channel(cdev->tx_channel);
                DMADRV_Release_Channel(cdev->rx_channel);
                if(rx_buff_list) kfree(rx_buff_list);
                if(tx_buff_list) kfree(tx_buff_list);
                return -EPERM;
}

static int blkcrypto_dma_start(DMA_CHANNEL chanID, DMADRV_LLI_T dmaLLI)
{
        pr_debug("%s:Entry.\n",__FUNCTION__);

        if (dmaLLI) {
                if (DMADRV_Start_Transfer_Ex(chanID, dmaLLI) != DMADRV_STATUS_OK) {
                        pr_err("DMADRV_Start_Transfer Failed for channel %d\n",chanID);
                        return -EPERM;
                }
        }
        else {
                if (DMADRV_Start_Transfer(chanID) != DMADRV_STATUS_OK) {
                        pr_err("DMADRV_Start_Transfer Failed for channel %d\n",chanID);
                        return -EPERM;
                }
	}

	pr_debug("%s:Exit.\n",__FUNCTION__);

        return -EINPROGRESS;
}

static int blkcrypto_engine_start(struct blkcrypto_crypto_device *cdev, u32 numLLI)
{
        u32 val = 0x0;

	pr_debug("%s: entry",__func__);

	clk_enable(cdev->clk);

        /* Check if crypto engine already busy. */
        val = readl(cdev->iobase + CRYPTO_STATUS_OFFSET);
        if(val & (CRYPTO_STATUS_INDMA_MASK|CRYPTO_STATUS_OUTDMA_MASK)) {
                pr_err("%s: Crypto engine busy. 0x%x\n",__FUNCTION__,val);
                return -EPERM;
        }

        /* Soft reset crypto. */
        val = CRYPTO_CONFIG1_DMASOFTRESET_MASK;
        writel(val, cdev->iobase + CRYPTO_CONFIG1_OFFSET);
        val &= ~CRYPTO_CONFIG1_DMASOFTRESET_MASK;
        writel(val, cdev->iobase + CRYPTO_CONFIG1_OFFSET);

        val = readl(cdev->iobase + CRYPTO_CONFIG1_OFFSET);
        val |= CRYPTO_CONFIG1_BULKSWAPSIZE_MASK |
                CRYPTO_CONFIG1_BKCRYPTSWPOUT_MASK |
                CRYPTO_CONFIG1_BKCRYPTSWPIN_MASK |
                CRYPTO_CONFIG1_DISCLKGATEDMA_MASK |
                CRYPTO_CONFIG1_DISCLKGATECACHE_MASK;
        writel(val, cdev->iobase + CRYPTO_CONFIG1_OFFSET);

        val = readl(cdev->iobase + CRYPTO_DMA_LDESC_OFFSET);
        val &= ~(CRYPTO_DMA_LDESC_BLKLNKLSTCNT_MASK);
        val |= ((numLLI - 1) << CRYPTO_DMA_LDESC_BLKLNKLSTCNT_SHIFT);
        writel(val, cdev->iobase + CRYPTO_DMA_LDESC_OFFSET);

        val = readl(cdev->iobase + CRYPTO_CTRL_OFFSET);
        val |= CRYPTO_CTRL_DMASINGLESTART_MASK;
        writel(val, cdev->iobase + CRYPTO_CTRL_OFFSET);

	pr_debug("%s: exit",__func__);

        return -EINPROGRESS;
}

static int blkcrypto_dma_req(struct ablkcipher_request *req)
{
	struct blkcrypto_aes_ctx *aes_ctx = crypto_tfm_ctx(req->base.tfm);
	struct blkcrypto_crypto_device *cdev = aes_ctx->cdev;
	int err = 0;

	pr_debug("%s:\n",__FUNCTION__);

	err = blkcrypto_dma_start(cdev->rx_channel, cdev->dma_ctx.rx_channel.dmaLLI);
	if(err != -EINPROGRESS) {
		pr_err("DMA Rx channel failed to start.\n");
		goto err_rx;
	}

	err = blkcrypto_dma_start(cdev->tx_channel, cdev->dma_ctx.tx_channel.dmaLLI);
	if(err != -EINPROGRESS) {
		pr_err("DMA Tx channel failed to start.\n");
		goto err_tx;
	}

	err = blkcrypto_engine_start(cdev, cdev->dma_ctx.rx_channel.numLLI);
	if(err != -EINPROGRESS) {
		pr_err("Bulkcrypto start failed.\n");
		goto err_engine;
	}

	pr_debug("%s:Exit.\n",__FUNCTION__);

        return -EINPROGRESS;

	err_engine:
                DMADRV_Stop_Transfer(cdev->tx_channel);
        err_tx:
                DMADRV_Stop_Transfer(cdev->rx_channel);
        err_rx:
                DMADRV_Release_Channel(cdev->rx_channel);
                DMADRV_Release_Channel(cdev->tx_channel);
                kfree(cdev->dma_ctx.tx_channel.dma_data.pBufList);
                kfree(cdev->dma_ctx.rx_channel.dma_data.pBufList);
                return err;
}

static int blkcrypto_handle_req(struct ablkcipher_request *req)
{
	struct blkcrypto_aes_ctx *aes_ctx = crypto_tfm_ctx(req->base.tfm);
	struct blkcrypto_crypto_device *cdev = aes_ctx->cdev;
	int err= -EBUSY;

	pr_debug("%s: Entry\n",__FUNCTION__);

	err = blkcrypto_dma_init(cdev);
	if(err < 0) {
		atomic_set(&cdev->dma_rx, 1);
		atomic_set(&cdev->dma_tx, 1);
		goto exit;
	}	

	err = blkcrypto_dma_setup(req);
	if(err < 0) {
		atomic_set(&cdev->dma_rx, 1);
		atomic_set(&cdev->dma_tx, 1);
		goto exit;
	}

	err = blkcrypto_dma_req(req);
	if(err != -EINPROGRESS) {
		atomic_set(&cdev->dma_rx, 1);
		atomic_set(&cdev->dma_tx, 1);
	}

	exit:
		return err;
}

static int blkcrypto_process_queue(struct blkcrypto_crypto_device *cdev)
{
	struct crypto_async_request *async_req, *backlog;
	struct blkcrypto_aes_ctx *aes_ctx;
	struct ablkcipher_request *req;
	unsigned long flags;
	int err = 0;

	pr_debug("%s: Entry\n",__FUNCTION__);

	spin_lock_irqsave(&cdev->lock, flags);
	if(atomic_read(&cdev->dma_rx) && atomic_read(&cdev->dma_tx) && cdev->queue.qlen) {
		backlog = crypto_get_backlog(&cdev->queue);
		async_req = crypto_dequeue_request(&cdev->queue);

		if (!async_req) {
			spin_unlock_irqrestore(&cdev->lock, flags);
			return err;
		}	

		atomic_set(&cdev->dma_rx, 0);
		atomic_set(&cdev->dma_tx, 0);
		spin_unlock_irqrestore(&cdev->lock, flags);

		if (backlog)
			backlog->complete(backlog, -EINPROGRESS);

		aes_ctx = crypto_tfm_ctx(async_req->tfm);
		req = container_of(async_req, struct ablkcipher_request, base);
		cdev->curnt = req;

		err = blkcrypto_handle_req(req);
	}
	else {
		spin_unlock_irqrestore(&cdev->lock, flags);
	}

	pr_debug("%s:Exit\n",__FUNCTION__);

	return err;
}

static int blkcrypto_setup_req(struct ablkcipher_request *req, u8 op, u8 mode)
{
	struct blkcrypto_aes_ctx *aes_ctx = crypto_tfm_ctx(req->base.tfm);
	struct blkcrypto_request_context *rctx = ablkcipher_request_ctx(req);
	u32	*aes_hdr = (u32 *)rctx->blkcrypto_hdr;
	u32	hdr_w1 = 0;
	int	err = 0;
	unsigned long flags;

	pr_debug("%s:\n",__FUNCTION__);

	memset(aes_hdr, 0, sizeof(rctx->blkcrypto_hdr));

	switch(op) {
		case BLKCRYPTO_AES_CBC:
			hdr_w1 |= BLKCRYPTO_HEADER_AES_CBC;
			break;
		case BLKCRYPTO_AES_CTR:
			hdr_w1 |= BLKCRYPTO_HEADER_AES_CTR;
			break;
		case BLKCRYPTO_AES_ECB:
			hdr_w1 |= BLKCRYPTO_HEADER_AES_ECB;
			break;
		default:
			pr_err("Invalid AES mode\n");
			err = -EPERM;
			goto exit;
	}

	if(mode == BLKCRYPTO_OP_ENCRYPT)
		hdr_w1 |= BLKCRYPTO_HEADER_AES_ENCRYPT;

	hdr_w1 |= BLKCRYPTO_HEADER_AES_ENABLE;

	switch(aes_ctx->key_len) {
		case AES_KEYSIZE_128:
			hdr_w1 |= BLKCRYPTO_HEADER_AES_KEYSIZE_128;
			break;
		case AES_KEYSIZE_192:
			hdr_w1 |= BLKCRYPTO_HEADER_AES_KEYSIZE_192;
			break;
		case AES_KEYSIZE_256:
			hdr_w1 |= BLKCRYPTO_HEADER_AES_KEYSIZE_256;
			break;
		default:
			err = -EPERM;
			goto exit;
	}

	hdr_w1 |= BLKCRYPTO_HEADER_OUTPUT_SUPPRESS;

	rctx->blkcrypto_hdr[BLKCRYPTO_HEADER_INDEX_FLAGS] = hdr_w1;
	aes_hdr[BLKCRYPTO_HEADER_INDEX_FLAGS] = hdr_w1;

	/* AES offset: from begining of header to IV/Counter. */
	aes_hdr[BLKCRYPTO_HEADER_INDEX_OFFSETS] = (((BLKCRYPTO_HEADER_NUM_WORDS<<2) +
							aes_ctx->key_len) & 0xFFFF);

	/* aes length includes IV */
	aes_hdr[BLKCRYPTO_HEADER_INDEX_AESLEN] =  req->nbytes + AES_IV_SIZE;

	/* Copy key to AES header */
	memcpy((u32 *)(rctx->blkcrypto_hdr + BULKCRYPTO_HEADER_KEY_OFFSET),
					aes_ctx->key_enc, aes_ctx->key_len);

	/* Copy IV to AES header */
	memcpy((rctx->blkcrypto_hdr + aes_hdr[BLKCRYPTO_HEADER_INDEX_OFFSETS]),
					req->info, AES_IV_SIZE);

	/* Swap the byte order for header to big endian. */
	b_host_2_be32(rctx->blkcrypto_hdr, BLKCRYPTO_HEADER_NUM_WORDS);

	rctx->state = NEW;

	spin_lock_irqsave(&aes_ctx->cdev->lock, flags);
	err = ablkcipher_enqueue_request(&aes_ctx->cdev->queue, req);
	spin_unlock_irqrestore(&aes_ctx->cdev->lock, flags);
	rctx->state=QUEUED;

	exit:
		return err;
}

static int blkcrypto_crypto_setup(struct ablkcipher_request *req, u8 op, u8 mode)
{
	struct blkcrypto_aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	int err = -EINPROGRESS;

	pr_debug("%s: Entry\n",__FUNCTION__);

	err = blkcrypto_setup_req(req, op, mode);
	if(err == -EPERM)
		return err;

	blkcrypto_process_queue(ctx->cdev);
	
	return err;
}

static int blkcrypto_aes_cbc_encrypt(struct ablkcipher_request *req)
{
	pr_debug("%s:\n",__FUNCTION__);
	return blkcrypto_crypto_setup(req, BLKCRYPTO_AES_CBC, BLKCRYPTO_OP_ENCRYPT);
}

static int blkcrypto_aes_cbc_decrypt(struct ablkcipher_request *req)
{
	pr_debug("%s:\n",__FUNCTION__);
	return blkcrypto_crypto_setup(req, BLKCRYPTO_AES_CBC, BLKCRYPTO_OP_DECRYPT);
}

static int blkcrypto_aes_setkey(struct crypto_ablkcipher *cipher, const u8 *in_key, unsigned int key_len)
{
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher);
	struct blkcrypto_aes_ctx *aes_ctx = crypto_tfm_ctx(tfm);
	const __le32 *key = (const __le32 *)in_key;
	int ret = 0;

	pr_debug("%s:Entry.%d\n",__FUNCTION__,key_len);
        if((key_len < AES_MIN_KEY_SIZE) || (key_len > AES_MAX_KEY_SIZE)) {
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		ret = -EINVAL;
        }
	else {
		memcpy((u32 *)(aes_ctx->key_enc), key, key_len);
		aes_ctx->key_len = key_len;
	}

	pr_debug("%s:Exit.\n",__FUNCTION__);
	return ret;
}

static int blkcrypto_cra_init(struct crypto_tfm *tfm)
{
	struct crypto_alg *alg = tfm->__crt_alg;
	struct blkcrypto_aes_ctx *ctx = crypto_tfm_ctx(tfm);

	ctx->cdev = container_of(alg, struct blkcrypto_crypto_device, algo);
	tfm->crt_ablkcipher.reqsize = sizeof(struct blkcrypto_request_context);	

	return 0;
}

static struct crypto_alg blkcrypto_cbc_alg = {
	.cra_name		=	"cbc(aes)",
	.cra_driver_name	=	"cbc-aes-blkcrypto",
	.cra_priority		=	110,
	.cra_flags		=	CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize		=	AES_BLOCK_SIZE,
	.cra_ctxsize		=	sizeof(struct blkcrypto_aes_ctx),
	.cra_alignmask		=	15,
	.cra_type			=	&crypto_ablkcipher_type,
	.cra_init			=	blkcrypto_cra_init,
	.cra_module			=	THIS_MODULE,
	.cra_list			=	LIST_HEAD_INIT(blkcrypto_cbc_alg.cra_list),
	.cra_u				=	{
		.ablkcipher	=	{
			.min_keysize	=	AES_MIN_KEY_SIZE,
			.max_keysize	=	AES_MAX_KEY_SIZE,
			.setkey		=	blkcrypto_aes_setkey,
			.encrypt	=	blkcrypto_aes_cbc_encrypt,
			.decrypt	=	blkcrypto_aes_cbc_decrypt,
			.ivsize		=	AES_IV_SIZE,
		},
	},
};

static void blkcrypto_dma_callback_rx(unsigned long data)
{
	struct blkcrypto_crypto_device *cdev = (struct blkcrypto_crypto_device *)data;
	u32 dma_status,i;
	u32 nbytes;
	
	pr_debug("%s: Entry\n",__FUNCTION__);

	dma_status = readl(cdev->iobase + CRYPTO_STATUS_OFFSET);
	if(!(dma_status & CRYPTO_STATUS_INDMA_MASK)) {
		if(DMADRV_Stop_Transfer(cdev->rx_channel) != DMADRV_STATUS_OK)
			pr_err("Rx stop failed.\n");
		dma_unmap_single(NULL, cdev->dma_ctx.rx_channel.buff_list[0].buffers[0].srcAddr,
		cdev->dma_ctx.rx_channel.buff_list[0].buffers[0].length, DMA_TO_DEVICE);

		i = 1; nbytes = cdev->curnt->nbytes;
		while(nbytes) {
			dma_unmap_single(NULL, cdev->dma_ctx.rx_channel.buff_list[i].buffers[0].srcAddr,
				cdev->dma_ctx.rx_channel.buff_list[i].buffers[0].length, DMA_TO_DEVICE);
			nbytes -= cdev->dma_ctx.rx_channel.buff_list[i].buffers[0].length;
			i++;
		}
		atomic_set(&cdev->dma_rx, 1);
		kfree(cdev->dma_ctx.rx_channel.buff_list);
	}

	if(atomic_read(&cdev->dma_rx) && atomic_read(&cdev->dma_tx)) {
		if(cdev->curnt->base.complete)
			cdev->curnt->base.complete(&cdev->curnt->base, 0);
		clk_disable(cdev->clk);
		blkcrypto_process_queue(cdev);
	}

	pr_debug("%s: Exit\n",__FUNCTION__);
}

static void blkcrypto_dma_callback_tx(unsigned long data)
{
	struct blkcrypto_crypto_device *cdev = (struct blkcrypto_crypto_device *)data;
	u32 dma_status,i;
	u32 nbytes;
	
	pr_debug("%s: Entry\n",__FUNCTION__);

	dma_status = readl(cdev->iobase + CRYPTO_STATUS_OFFSET);
        if(!(dma_status & CRYPTO_STATUS_OUTDMA_MASK)) {
                if(DMADRV_Stop_Transfer(cdev->tx_channel) != DMADRV_STATUS_OK)
                        pr_err("DMA Tx stop failed.\n");

                i = 0; nbytes = cdev->curnt->nbytes;
                while(nbytes) {
                        dma_unmap_single(NULL, cdev->dma_ctx.tx_channel.buff_list[i].buffers[0].destAddr,
                                cdev->dma_ctx.tx_channel.buff_list[i].buffers[0].length, DMA_FROM_DEVICE);
                        nbytes -= cdev->dma_ctx.tx_channel.buff_list[i].buffers[0].length;
                        i++;
                }

                atomic_set(&cdev->dma_tx, 1);
                kfree(cdev->dma_ctx.tx_channel.buff_list);
        }

        if(atomic_read(&cdev->dma_rx) && atomic_read(&cdev->dma_tx)) {
                if(cdev->curnt->base.complete)
                        cdev->curnt->base.complete(&cdev->curnt->base, 0);
                clk_disable(cdev->clk);
		blkcrypto_process_queue(cdev);
        }

        pr_debug("%s: Exit\n",__FUNCTION__);
}

static int __devinit brcm_blkcrypto_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct blkcrypto_crypto_device *cdev;
	struct resource *res;

	pr_debug("%s:Entry.\n",__FUNCTION__);

	cdev = kzalloc(sizeof(struct blkcrypto_crypto_device), GFP_KERNEL);
	if(!cdev) {
		pr_err("Failed to allocate crypto device.");
		return -EPERM;
	}

	crypto_init_queue(&cdev->queue, BLKCRYPTO_AES_QUEUE_LENGTH);

	/* Get blkcrypto base address. */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res) {
		pr_err("%s:Invalid resource type.\n",__FUNCTION__);
		ret = -EPERM;
		goto exit;
	}
	cdev->iobase = res->start; 

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if(!res) {
		pr_err("%s:Invalid resource type.\n",__FUNCTION__);
		ret = -EPERM;
		goto exit;
	}
	cdev->phy_base = res->start;

	/* Initializing the clock. */
	cdev->clk = clk_get(NULL, "crypto");
	if (!cdev->clk) {
		pr_err("Clock intialization failed.\n");
		ret = -ENODEV;
		goto exit;
	}

	INIT_LIST_HEAD(&cdev->list);
	spin_lock_init(&cdev->lock);
	spin_lock(&blkcrypto_drv_lock);
	list_add_tail(&cdev->list, &blkcrypto_drv_list);
	spin_unlock(&blkcrypto_drv_lock);

	atomic_set(&cdev->dma_rx, 1);
	atomic_set(&cdev->dma_tx, 1);

	platform_set_drvdata(pdev, cdev);

	cdev->algo = blkcrypto_cbc_alg;
	ret = crypto_register_alg(&cdev->algo);

        if(ret) {
		pr_info("%s: Crypto algo registration failed.\n",__FUNCTION__);
        }
        else {
		pr_info("%s: Crypto algo registration successful.\n",__FUNCTION__);
        }

        return ret;

	exit:
		kfree(cdev);
		return ret;
}

static int __devexit brcm_blkcrypto_remove(struct platform_device *pdev)
{
	struct blkcrypto_crypto_device *cdev;

	cdev = platform_get_drvdata(pdev);
	clk_put(cdev->clk);

        return 0;
}

static struct platform_driver brcm_blkcrypto_driver = {
        .probe  =       brcm_blkcrypto_probe,
        .remove =       brcm_blkcrypto_remove,
        .driver =       {
                .name   =       "brcm_blkcrypto",
                .owner  =       THIS_MODULE,
        },
};

static int __init brcm_blkcrypto_init(void)
{
        pr_info("%s: Bulk Crypto driver init.\n",__FUNCTION__);

        return platform_driver_register(&brcm_blkcrypto_driver);
}

static void __init brcm_blkcrypto_exit(void)
{
        pr_info("%s: Bulk Crypto driver exit.\n",__FUNCTION__);

        platform_driver_unregister(&brcm_blkcrypto_driver);
}

subsys_initcall(brcm_blkcrypto_init);
module_exit(brcm_blkcrypto_exit);

MODULE_LICENSE("GPL V2");

