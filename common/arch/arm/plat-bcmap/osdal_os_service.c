#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <plat/types.h>
#include <plat/osdal_os.h>
#include <plat/osdal_os_service.h>
#include <plat/dma_drv.h>

typedef int OSDAL_DMA_CLIENT_T;
typedef int OSDAL_INT_ID_T;
static DMA_CLIENT dma_clients[TOTAL_DMA_CHANNELS][2];
/* For DMA access */
#ifdef _ATHENA_
static Boolean _validate_dmaclient(OSDAL_DMA_CLIENT id,
					DMA_CLIENT *validated_id)
{
	Boolean ok = TRUE;

	switch (id) {
	case OSDAL_DMA_CLIENT_BULK_CRYPT_OUT:
		*validated_id = DMA_CLIENT_BULK_CRYPT_OUT;
		break;
	case OSDAL_DMA_CLIENT_CAM:
		*validated_id = DMA_CLIENT_CAM;
		break;
	case OSDAL_DMA_CLIENT_I2S_TX:
		*validated_id = DMA_CLIENT_I2S_TX;
		break;
	case OSDAL_DMA_CLIENT_I2S_RX:
		*validated_id = DMA_CLIENT_I2S_RX;
		break;
	case OSDAL_DMA_CLIENT_SIM:
		*validated_id = DMA_CLIENT_SIM_RX;
		break;
	case OSDAL_DMA_CLIENT_CRC:
		*validated_id = DMA_CLIENT_CRC;
		break;
	case OSDAL_DMA_CLIENT_SPI_RX:
		*validated_id = DMA_CLIENT_SPI_RX;
		break;
	case OSDAL_DMA_CLIENT_SPI_TX:
		*validated_id = DMA_CLIENT_SPI_TX;
		break;
	case OSDAL_DMA_CLIENT_UARTA_RX:
		*validated_id = DMA_CLIENT_UARTA_RX;
		break;
	case OSDAL_DMA_CLIENT_UARTA_TX:
		*validated_id = DMA_CLIENT_UARTA_TX;
		break;
	case OSDAL_DMA_CLIENT_UARTB_RX:
		*validated_id = DMA_CLIENT_UARTB_RX;
		break;
	case OSDAL_DMA_CLIENT_UARTB_TX:
		*validated_id = DMA_CLIENT_UARTB_TX;
		break;
	case OSDAL_DMA_CLIENT_DES_IN:
		*validated_id = DMA_CLIENT_DES_IN;
		break;
	case OSDAL_DMA_CLIENT_DES_OUT:
		*validated_id = DMA_CLIENT_DES_OUT;
		break;
	case OSDAL_DMA_CLIENT_USB_RX:
		*validated_id = DMA_CLIENT_USB_RX;
		break;
	case OSDAL_DMA_CLIENT_USB_TX:
		*validated_id = DMA_CLIENT_USB_TX;
		break;
	case OSDAL_DMA_CLIENT_HSDPA_SCATTER:
		*validated_id = DMA_CLIENT_HSDPA_SCATTER;
		break;
	case OSDAL_DMA_CLIENT_HSDPA_GATHER:
		*validated_id = DMA_CLIENT_HSDPA_GATHER;
		break;
	case OSDAL_DMA_CLIENT_BULK_CRYPT_IN:
		*validated_id = DMA_CLIENT_BULK_CRYPT_IN;
		break;
	case OSDAL_DMA_CLIENT_LCD:
		*validated_id = DMA_CLIENT_LCD;
		break;
	case OSDAL_DMA_CLIENT_MSPRO:
		*validated_id = DMA_CLIENT_MSPRO;
		break;
	case OSDAL_DMA_CLIENT_DSI_CM:
		*validated_id = DMA_CLIENT_DSI_CM;
		break;
	case OSDAL_DMA_CLIENT_DSI_VM:
		*validated_id = DMA_CLIENT_DSI_VM;
		break;
	case OSDAL_DMA_CLIENT_TVENC1:
		*validated_id = DMA_CLIENT_TVENC1;
		break;
	case OSDAL_DMA_CLIENT_TVENC2:
		*validated_id = DMA_CLIENT_TVENC2;
		break;
	case OSDAL_DMA_CLIENT_AUDIO_IN_FIFO:
		*validated_id = DMA_CLIENT_AUDIO_IN_FIFO;
		break;
	case OSDAL_DMA_CLIENT_AUDIO_OUT_FIFO:
		*validated_id = DMA_CLIENT_AUDIO_OUT_FIFO;
		break;
	case OSDAL_DMA_CLIENT_POLYRING_OUT_FIFO:
		*validated_id = DMA_CLIENT_POLYRING_OUT_FIFO;
		break;
	case OSDAL_DMA_CLIENT_AUDIO_WB_MIXERTAP:
		*validated_id = DMA_CLIENT_AUDIO_WB_MIXERTAP;
		break;
	case OSDAL_DMA_CLIENT_MEMORY:
		*validated_id = DMA_CLIENT_MEMORY;
		break;
	default:
		ok = FALSE;
		break;
	}

	return ok;
}
#endif

OSDAL_Status OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT srcID,
					OSDAL_DMA_CLIENT dstID,
					UInt32 *pChanNum)
{
	OSDAL_Status ret = OSDAL_ERR_OK;
	DMA_CLIENT src, dst;
	DMA_CHANNEL chan;
    
	if (_validate_dmaclient(srcID, &src) && _validate_dmaclient(dstID, &dst)) {
		if (DMADRV_Obtain_Channel(src, dst, &chan) != DMADRV_STATUS_OK) {
			ret = OSDAL_ERR_IO;
		}
		else {
			if (chan == DMA_CHANNEL_INVALID)
				ret = OSDAL_ERR_INVAL;
			else {
				dma_clients[chan][0] = src;
				dma_clients[chan][1] = dst;
				*pChanNum = (UInt32)chan;       
			}
		}
	} else
		ret = OSDAL_ERR_INVAL;
	return ret;
}
EXPORT_SYMBOL(OSDAL_DMA_Obtain_Channel);
OSDAL_Status OSDAL_DMA_Release_Channel(UInt32 chanNum)
{
	if (DMADRV_Release_Channel((DMA_CHANNEL)chanNum) == DMADRV_STATUS_OK)
		return OSDAL_ERR_OK;

	return OSDAL_ERR_IO;
}
EXPORT_SYMBOL(OSDAL_DMA_Release_Channel);
static DMA_BSIZE _translate_dma_bsize(OSDAL_DMA_BSIZE bsize)
{
	DMA_BSIZE drv_bsize = DMA_BURST_SIZE_1;

	switch(bsize) {
	case OSDAL_DMA_BURST_SIZE_1:
		drv_bsize = DMA_BURST_SIZE_1;
        	break;
#if defined (_HERA_)
	case OSDAL_DMA_BURST_SIZE_2:
		drv_bsize = DMA_BURST_SIZE_2;
		break;
#endif
	case OSDAL_DMA_BURST_SIZE_4:
		drv_bsize = DMA_BURST_SIZE_4;
		break;
	case OSDAL_DMA_BURST_SIZE_8:
		drv_bsize = DMA_BURST_SIZE_8;
		break;
	case OSDAL_DMA_BURST_SIZE_16:
		drv_bsize = DMA_BURST_SIZE_16;
		break;
	case OSDAL_DMA_BURST_SIZE_32:
		drv_bsize = DMA_BURST_SIZE_32;
		break;
	case OSDAL_DMA_BURST_SIZE_64:
		drv_bsize = DMA_BURST_SIZE_64;
		break;
	case OSDAL_DMA_BURST_SIZE_128:
		drv_bsize = DMA_BURST_SIZE_128;
		break;
	}
	
	return drv_bsize;
}

OSDAL_Status OSDAL_DMA_Config_Channel(UInt32 chanNum,
					OSDAL_Dma_Chan_Info *pChanInfo)
{
	Dma_Chan_Info chanInfo;
    
	chanInfo.srcID = dma_clients[chanNum][0];
	chanInfo.dstID = dma_clients[chanNum][1];
	chanInfo.type = (DMA_CHAN_TYPE)pChanInfo->type;
	chanInfo.alignment = (DMA_ALIGN)pChanInfo->alignment;
	chanInfo.srcBstSize = _translate_dma_bsize(pChanInfo->srcBstSize);
	chanInfo.dstBstSize = _translate_dma_bsize(pChanInfo->dstBstSize);
	chanInfo.srcDataWidth = (DMA_DWIDTH)pChanInfo->srcDataWidth;
	chanInfo.dstDataWidth = (DMA_DWIDTH)pChanInfo->dstDataWidth;
	chanInfo.srcMaster = pChanInfo->dstMaster;
	chanInfo.dstMaster = pChanInfo->dstMaster;
	chanInfo.priority = pChanInfo->priority;
	chanInfo.incMode = pChanInfo->incMode;
	chanInfo.xferCompleteCb = (DmaDrv_Callback)pChanInfo->xferCompleteCb;
	chanInfo.freeChan = pChanInfo->freeChan;
	chanInfo.bCircular = pChanInfo->bCircular;
    
#if defined (_HERA_)
	chanInfo.srcBstLength = pChanInfo->srcBstLength;
	chanInfo.dstBstLength = = pChanInfo->dstBstLength;
#endif

	if (DMADRV_Config_Channel((DMA_CHANNEL)chanNum, &chanInfo) == DMADRV_STATUS_OK) 
		return OSDAL_ERR_OK;
    
	return OSDAL_ERR_IO;
}
EXPORT_SYMBOL(OSDAL_DMA_Config_Channel);
OSDAL_Status OSDAL_DMA_Bind_Data(UInt32 chanNum, OSDAL_Dma_Data *pData)
{
	Dma_Data data;
   
	data.numBuffer = pData->numBuffer;

	/* We make sure OSDAL_Dma_Buffer_List and Dma_Buffer_List are the same */
	data.pBufList = (Dma_Buffer_List*)pData->pBufList;    
         
	if (DMADRV_Bind_Data((DMA_CHANNEL)chanNum, &data) == DMADRV_STATUS_OK)
		return OSDAL_ERR_OK;
	return OSDAL_ERR_IO;
}
EXPORT_SYMBOL(OSDAL_DMA_Bind_Data);
OSDAL_Status OSDAL_DMA_Start_Transfer(UInt32 chanNum)
{
	/* use chanID from pChannel */
	if (DMADRV_Start_Transfer(chanNum) == DMADRV_STATUS_OK)
		return OSDAL_ERR_OK;
	return OSDAL_ERR_IO;
}
EXPORT_SYMBOL(OSDAL_DMA_Start_Transfer);
OSDAL_Status OSDAL_DMA_Stop_Transfer(UInt32 chanNum)
{
	if (DMADRV_Stop_Transfer((DMA_CHANNEL)chanNum) == DMADRV_STATUS_OK)    
		return OSDAL_ERR_OK;
	return OSDAL_ERR_IO;  
}
EXPORT_SYMBOL(OSDAL_DMA_Stop_Transfer);
OSDAL_Status OSDAL_DMA_Force_Shutdown_Channel(UInt32 chanNum)
{
    if (DMADRV_Force_Shutdown_Channel((DMA_CHANNEL)chanNum) == DMADRV_STATUS_OK)
        return OSDAL_ERR_OK;
    return OSDAL_ERR_IO;
}
EXPORT_SYMBOL(OSDAL_DMA_Force_Shutdown_Channel);
OSDAL_Status OSDAL_DMA_Register_Callback(void *pChannel, void * cb)
{
	OSDAL_Status ret = OSDAL_ERR_OK;
	return ret;
}
EXPORT_SYMBOL(OSDAL_DMA_Register_Callback);
int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
int bcm_gpio_set_db_val(unsigned int gpio, unsigned int db_val);
int bcm_gpio_get_direction(unsigned int gpio);
int bcm_gpio_get_irq_type(unsigned int gpio);
int bcm_gpio_get_pull_up_down(unsigned int gpio);
int bcm_gpio_get_db_val(unsigned int gpio);
int bcm_gpio_pull_up(unsigned int gpio, bool up);

#define	GPIO_PULL_DISABLE        0x00
#define GPIO_PULL_ENABLE         0x01
#define GPIO_PULL_DOWN        	 (0 << 1)
#define GPIO_PULL_UP         	 (1 << 1)
typedef enum {
	GPIO_DIR_INPUT = 0x00,
	GPIO_DIR_OUTPUT = 0x01,
	GPIO_INT_DISABLE = (0 << 1),
	GPIO_INT_RISING = (1 << 1),
	GPIO_INT_FALLING = (2 << 1),
	GPIO_INT_BOTH = (3 << 1),
} OSDAL_GPIO_Stats;

/* GPIO Service API's */
static irqreturn_t  osdal_gpio_isr(int irq, void *cb)
{
	OSDAL_GPIO_CB gpio_cb = cb;
	gpio_cb();
	return IRQ_HANDLED;
}

OSDAL_Status OSDAL_GPIO_Set_Config(UInt32 pin, OSDAL_GPIO_CFG_T type, UInt32 val)
{
	if (type == OSDAL_GPIO_CFG_MODE) {
		if (val == GPIO_DIR_INPUT)
			gpio_direction_input(pin);
		else if (val == GPIO_DIR_OUTPUT)
			gpio_direction_output(pin, 0);
		else if (val == GPIO_INT_DISABLE)
			set_irq_type(GPIO_TO_IRQ(pin), IRQ_TYPE_NONE);
		else if (val == GPIO_INT_RISING)
			set_irq_type(GPIO_TO_IRQ(pin), IRQ_TYPE_EDGE_RISING);
		else if (val == GPIO_INT_FALLING)
			set_irq_type(GPIO_TO_IRQ(pin), IRQ_TYPE_EDGE_FALLING);
		else if (val == GPIO_INT_BOTH)
			set_irq_type(GPIO_TO_IRQ(pin), IRQ_TYPE_EDGE_BOTH);
	} else if (type == OSDAL_GPIO_CFG_PULL) {
		bcm_gpio_pull_up(pin, (val >> 1));
		bcm_gpio_pull_up_down_enable(pin, val);
	} else if (type == OSDAL_GPIO_CFG_DEBOUNCE)
		bcm_gpio_set_db_val(pin, val);
	else
		return OSDAL_ERR_INVAL;

	return OSDAL_ERR_OK;
}

OSDAL_Status OSDAL_GPIO_Get_Config(UInt32 pin, OSDAL_GPIO_CFG_T type, UInt32 *val)
{
	UInt32 direction, irqtype;

	if (type == OSDAL_GPIO_CFG_MODE) {
		direction = bcm_gpio_get_direction(pin);
		irqtype = bcm_gpio_get_irq_type(pin);
		*val = direction;
		*val |= (irqtype << 2);
	} else if (type == OSDAL_GPIO_CFG_PULL)
		*val = bcm_gpio_get_pull_up_down(pin);
	else if (type == OSDAL_GPIO_CFG_DEBOUNCE)
		*val = bcm_gpio_get_db_val(pin);
	else
		return OSDAL_ERR_INVAL;

	return OSDAL_ERR_OK;
}

void OSDAL_GPIO_Set(UInt32 id, Boolean high)
{
	gpio_set_value(id, high);
}

Boolean OSDAL_GPIO_Get(UInt32 id)
{
	bool val;
	val = gpio_get_value(id);
	return val;
}

OSDAL_Status OSDAL_GPIO_Register_Callback(UInt32 pin,  OSDAL_GPIO_CB cb)
{
	int rc;
	rc = request_irq(GPIO_TO_IRQ(pin), osdal_gpio_isr, IRQF_DISABLED,
						"osdal_gpio_irq", cb);
	return OSDAL_ERR_OK;
}

