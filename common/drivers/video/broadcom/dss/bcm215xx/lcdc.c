/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/broadcom/dss/bcm215xx/lcdc.c
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

/****************************************************************************
*
*  lcdc.c
*
*  PURPOSE:
*    This implements the code to use a Broadcom LCD host interface.
*
*  NOTES:
*    Uses device minor number to select panel:  0==main 1==sub
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include "lcdc.h"
#include <linux/broadcom/lcdc_dispimg.h>
#include <linux/rtc.h>
/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

static char gBanner[] __initdata =
    KERN_INFO "lcdc: Broadcom LCD Controller Driver: 0.01";

static void __iomem *lcdc_base;
static int lcd_reset_gpio;

#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
static struct cpufreq_client_desc *lcdc_client;
#endif

/* globals to: communicate with the update thread */
/*   control access to LCD registers */
/*  manage DMA channel */
static int gInitialized;
EXPORT_SYMBOL(gInitialized);

#ifdef CONFIG_HAS_WAKELOCK
static struct wake_lock glcdfb_wake_lock;
#endif
static struct semaphore gDmaSema;
#ifdef CONFIG_REGULATOR
static struct regulator *lcdc_regulator = NULL;
#endif
/* forward func declarations */
static void lcd_exit(void);
static int lcd_init(void);
void lcd_pwr_on_controll(int value);
static long lcd_ioctl(struct file *file,
		     unsigned int cmd, unsigned long arg);
static int lcd_mmap(struct file *file, struct vm_area_struct *vma);
static int lcd_open(struct inode *inode, struct file *file);
static int lcd_release(struct inode *inode, struct file *file);
void lcd_display_test(LCD_dev_info_t *dev);
void lcd_display_rect(LCD_dev_info_t *dev, LCD_Rect_t *r);
static void lcd_update_column(LCD_dev_info_t *dev, unsigned int column);
static int lcd_pm_update(PM_CompPowerLevel compPowerLevel,
			 PM_PowerLevel sysPowerLevel);

static void lcd_send_cmd_sequence(Lcd_init_t *init);
static inline void lcd_init_panels(void);
static inline void lcd_poweroff_panels(void);
static void lcd_setup_for_data(LCD_dev_info_t *dev);
static void lcd_csl_cb(CSL_LCD_RES_T, CSL_LCD_HANDLE, void*);

#if defined(CONFIG_ENABLE_QVGA) || defined(CONFIG_ENABLE_HVGA) || defined(CONFIG_BOARD_ACAR)
void display_black_background(void);
#endif

static int __init lcdc_probe(struct platform_device *pdev);
static int lcdc_remove(struct platform_device *pdev);

#ifdef CONFIG_BRCM_KPANIC_UI_IND
int lcdc_disp_img(int img_index);
#endif

uint32_t lcd_id;
int lcd_esd;
int lcd_esd_ckeck=1;
int lcd_enable;
EXPORT_SYMBOL(lcd_enable);

#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
uint8_t	lcd_frame_inversion_during_call = 0;
#endif

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int set_irq_type(unsigned int irq, unsigned int type);

/**************************************************************************/
static CSL_LCDC_INIT_T ctrl_init;
static CSL_LCD_HANDLE handle;
static CSL_LCD_UPD_REQ_T req;
static CSL_LCDC_PAR_CTRL_T busCfg;

#define ON  1
#define OFF 0

static struct workqueue_struct *lcd_wq;
/****************************************************************************
*
*   File Operations (these are the device driver entry points)
*
***************************************************************************/

struct file_operations lcd_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = lcd_ioctl,
	.mmap = lcd_mmap,
	.open = lcd_open,
	.release = lcd_release,
};

static struct platform_driver lcdc_driver = {
	.probe = lcdc_probe,
	.remove = lcdc_remove,
	.driver = {
		   .name = "LCDC",
		   .owner = THIS_MODULE,
		   },
};

//{{ Mark for GetLog - 1/2
struct struct_frame_buf_mark {
u32 special_mark_1;
u32 special_mark_2;
u32 special_mark_3;
u32 special_mark_4;
void *p_fb; // it must be physical address
u32 resX;
u32 resY;
u32 bpp;    // color depth : 16 or 24
u32 frames; // frame buffer count : 2
};

static struct struct_frame_buf_mark  frame_buf_mark = {
.special_mark_1 = (('*' << 24) | ('^' << 16) | ('^' << 8) | ('*' << 0)),
.special_mark_2 = (('I' << 24) | ('n' << 16) | ('f' << 8) | ('o' << 0)),
.special_mark_3 = (('H' << 24) | ('e' << 16) | ('r' << 8) | ('e' << 0)),
.special_mark_4 = (('f' << 24) | ('b' << 16) | ('u' << 8) | ('f' << 0)),
.p_fb   = 0,
.resX   = LCD_WIDTH,    // it has dependency on h/w
.resY   = LCD_HEIGHT,     // it has dependency on h/w
.bpp    = LCD_BITS_PER_PIXEL,      // it has dependency on h/w
.frames = 2
};
//}} Mark for GetLog - 1/2

/* ---- Functions -------------------------------------------------------- */

void lcd_write_cmd(uint32_t cmd)
{
	CSL_LCD_RES_T ret;
	CSL_LCDC_PAR_SetSpeed(handle, &timingReg);
	ret = CSL_LCDC_WrCmnd(handle, cmd);
	if (CSL_LCD_OK != ret)
		pr_info("CSL_LCDC_WrCmnd failed error: %d", ret);
}

void lcd_write_data(uint32_t data)
{
	CSL_LCD_RES_T ret;
	CSL_LCDC_PAR_SetSpeed(handle, &timingReg);
	ret = CSL_LCDC_WrData(handle, data);
	if (CSL_LCD_OK != ret)
		pr_info("CSL_LCDC_WrData failed error: %d", ret);
}

uint32_t lcd_read_reg(uint32_t reg)
{
	CSL_LCD_RES_T ret;
	uint32_t data;

	ret = CSL_LCDC_WrCmnd(handle, reg);
	if (CSL_LCD_OK != ret)
		pr_info("CSL_LCDC_WrCmnd failed error: %d", ret);

	ret = CSL_LCDC_PAR_RdData(handle, &data);
	if (CSL_LCD_OK != ret)
		pr_info("CSL_LCDC_PAR_RdData failed error: %d", ret);

	return data;
}

#if defined (CONFIG_BCM_LCD_S6D05A1X31_COOPERVE)	
uint32_t LCD_DRV_ID_Check(void)
{

	uint8_t Read_buf[3] = {0,};
	uint32_t Read_value = 0;

	CSL_LCD_RES_T ret;
	uint32_t data;
	int i;
	UInt8 CMD_readId[] = {0xDA, 0xDB, 0xDC};

	for(i=0; i< 3; i++)
	{
		ret=CSL_LCDC_WrCmnd(handle, CMD_readId[i]);
		if (CSL_LCD_OK != ret)
			printk("CSL_LCDC_WrCmnd failed error: %d", ret);

		ret = CSL_LCDC_PAR_RdData(handle, &data);		// dummy data
		ret = CSL_LCDC_PAR_RdData(handle, &data);		
		Read_buf[i] =data;		
		printk("lcd_read_reg_id data:%x\n", Read_buf[i]);	
	
	}
 
	Read_value += Read_buf[0] << 16;
	Read_value += Read_buf[1] << 8;
	Read_value += Read_buf[2];

    return Read_value;
}
#else

void lcd_read_reg_id(UInt8 reg, UInt8 * Read_value, UInt8 num)
{
	CSL_LCD_RES_T ret;
	uint32_t data;
      int value;
	int i;

	ret = CSL_LCDC_WrCmnd(handle, reg);
	if (CSL_LCD_OK != ret)
		pr_info("CSL_LCDC_WrCmnd failed error: %d", ret);

     for(i=0; i<num; i++)
       {
	ret = CSL_LCDC_PAR_RdData(handle, &data);
	if (CSL_LCD_OK != ret)
	pr_info("CSL_LCDC_PAR_RdData failed error: %d", ret);
      Read_value[i] =data;
      	printk("lcd_read_reg_id data:%x\n", Read_value[i]);	
	}

}


uint32_t LCD_DRV_ID_Check(UInt8 reg, UInt8 num)
{

 uint8_t Read_buf[4] = {0,};
 uint32_t Read_value = 0;

 //lcd_read_reg_id(0x04, Read_buf, 4);
 lcd_read_reg_id(reg, Read_buf, num);

 //ad_value = Read_buf[0] << 24;
 Read_value += Read_buf[1] << 16;
 Read_value += Read_buf[2] << 8;
 Read_value += Read_buf[3];

    return Read_value;
}
#endif
 

/***************************************************************************
*  lcd_csl_cb
*
*   Callback from CSL after DMA transfer completion
*
***************************************************************************/
static void lcd_csl_cb(CSL_LCD_RES_T res, CSL_LCD_HANDLE handle, void *cbRef)
{
	up(&gDmaSema);
	if ((CSL_LCD_OK != res)&&lcd_enable)
		pr_info("lcd_csl_cb: res =%d\n", res);
}

static inline bool is_unaligned(LCD_dev_info_t * dev)
{
	return ((dev->dirty_rect.left * dev->bits_per_pixel >> 3) & 2);
}

static inline bool is_odd_total(LCD_dev_info_t * dev)
{
	return (((dev->dirty_rect.right - dev->dirty_rect.left + 1) * (dev->dirty_rect.bottom - dev->dirty_rect.top + 1)) % 2);
}

static inline bool is_odd_stride(LCD_dev_info_t * dev)
{
	return (((dev->dirty_rect.right - dev->dirty_rect.left + 1) * dev->bits_per_pixel >> 3) & 2);
}

static inline bool is_out_of_bounds(LCD_dev_info_t * dev)
{
	return ((dev->dirty_rect.right >= dev->width) || (dev->dirty_rect.left >= dev->width));
}

static inline bool is_tx_done_16(LCD_dev_info_t * dev)
{
	return ((dev->dirty_rect.right <= dev->dirty_rect.left) || (dev->dirty_rect.right == 0));
}

static inline bool is_tx_done_32(LCD_dev_info_t * dev)
{
	return (dev->dirty_rect.right < dev->dirty_rect.left);
}

/****************************************************************************
*
*  lcd_dev_dirty_rect
*
*   Marks the indicated rows as dirty and arranges for them to be transferred
*   to the LCD.
*
***************************************************************************/
static void lcd_dev_dirty_rect(LCD_dev_info_t * dev,
			       LCD_DirtyRect_t * dirtyRect)
{
	CSL_LCD_RES_T ret;
	int i;
	int err = -EINVAL;

	OSDAL_Dma_Buffer_List *buffer_list, *temp_list;

	if ((dirtyRect->top > dirtyRect->bottom)
	    || ((dirtyRect->bottom - dirtyRect->top) >= dev->height)
	    || (dirtyRect->left > dirtyRect->right)
	    || ((dirtyRect->right - dirtyRect->left) >= dev->width)) {
		LCD_DEBUG("invalid dirty-rows params - ignoring\n");
		LCD_DEBUG("top = %u,  bottom = %u, left = %u, right = %u\n",
			  dirtyRect->top, dirtyRect->bottom,
			  dirtyRect->left, dirtyRect->right);
		return;
	}

	down_interruptible(&gDmaSema);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock(&glcdfb_wake_lock);
#endif

	dev->dirty_rect = *dirtyRect;
	dev->row_start = dev->dirty_rect.top % dev->height;
	dev->row_end = dev->dirty_rect.bottom % dev->height;

	/*If start address is aligned to odd boundary */
	if (is_unaligned(dev)) {
		dev->col_start = dev->dirty_rect.left;
		dev->col_end = dev->dirty_rect.left;
		lcd_setup_for_data(dev);
		lcd_update_column(dev, dev->dirty_rect.left);
		dev->dirty_rect.left += 1;
	}

	/*If length is odd multiple */
	if (is_odd_stride(dev) || ((dev->bits_per_pixel == 32) && is_odd_total(dev))) {
		dev->col_start = dev->dirty_rect.right;
		dev->col_end = dev->dirty_rect.right;
		lcd_setup_for_data(dev);
		lcd_update_column(dev, dev->dirty_rect.right);
		dev->dirty_rect.right -= 1;
	}

	if (is_out_of_bounds(dev) || ((32 != dev->bits_per_pixel) && is_tx_done_16(dev))
		|| ((32 == dev->bits_per_pixel) && is_tx_done_32(dev))) {
		/* Dirty columns have been transferred. No further processing required.*/
		goto done;
	}

	buffer_list = kzalloc((dev->dirty_rect.bottom - dev->dirty_rect.top + 1) * sizeof(OSDAL_Dma_Buffer_List), GFP_KERNEL);
	if (!buffer_list) {
		pr_info("Couldn't allocate memory for dma buffer list\n");
		goto done;
	}

	temp_list = buffer_list;
	for (i = dev->dirty_rect.top; i <= dev->dirty_rect.bottom; i++) {
		temp_list->buffers[0].srcAddr = (UInt32)dev->frame_buffer.physPtr + (i * dev->width + dev->dirty_rect.left) * dev->bits_per_pixel / 8;
		temp_list->buffers[0].destAddr = REG_LCDC_DATR_PADDR;
		temp_list->buffers[0].length =
		    (dev->dirty_rect.right - dev->dirty_rect.left +
		     1) * dev->bits_per_pixel / 8;
		temp_list->buffers[0].bRepeat = 0;
		temp_list++;
	}
	temp_list--;		/* Go back to the last list item to set interrupt = 1 */
	temp_list->buffers[0].interrupt = 1;

	req.buff = (void *)buffer_list;
	req.buffBpp = dev->bits_per_pixel;
	req.lineLenP = dev->dirty_rect.right - dev->dirty_rect.left + 1;
	req.lineCount = dev->dirty_rect.bottom - dev->dirty_rect.top + 1;
	req.timeOut_ms = 100;
	req.cslLcdCb = lcd_csl_cb;
	req.cslLcdCbRef = NULL;
	req.multiLLI = true;

	dev->col_start = dev->dirty_rect.left;
	dev->col_end = dev->dirty_rect.right;

	lcd_setup_for_data(dev);

	CSL_LCDC_PAR_SetSpeed(handle, &timingMem);

	/*CP processor is setting IOCR6[19], which it shouldn't be doing. Remove this once the CP team fixes it.*/
	if (dev->te_supported) {
		board_sysconfig(SYSCFG_LCD, SYSCFG_ENABLE);
	}

	ret = CSL_LCDC_Update(handle, &req);
	if (CSL_LCD_OK != ret) {
		pr_info("CSL_LCDC_Update failed error: %d", ret);
		goto fail;
	}
	err = 0;

fail:
	kfree(buffer_list);

done:
#ifdef CONFIG_HAS_WAKELOCK
	wake_unlock(&glcdfb_wake_lock);
#endif
	if (err < 0)
		up(&gDmaSema);
}

void lcd_dev_dirty_rows(LCD_dev_info_t * dev, LCD_DirtyRows_t * dirtyRows)
{
	LCD_DirtyRect_t dirtyRect = {
		.top = dirtyRows->top,
		.bottom = dirtyRows->bottom,
		.left = 0,
		.right = LCD_device[LCD_main_panel].width - 1,
	};
	lcd_dev_dirty_rect(dev, &dirtyRect);
}

void lcd_dirty_rows(LCD_DirtyRows_t * dirtyRows)
{
	lcd_dev_dirty_rows(&LCD_device[LCD_main_panel], dirtyRows);
}

void lcd_dirty_rect(LCD_DirtyRect_t * dirtyRect)
{
	lcd_dev_dirty_rect(&LCD_device[LCD_main_panel], dirtyRect);
}

#ifdef CONFIG_BRCM_KPANIC_UI_IND
static void lcd_send_data(uint16_t * p, int len, bool rle)
{
	int i;
	uint32_t rle_count;

	if (16 == LCD_device[0].bits_per_pixel) {
		uint16_t pixel_data;

		if (readl(lcdc_base + REG_LCDC_CR) & REG_LCDC_CR_ENABLE_8_BIT_INTF) {
			if (rle) {
				for (i = 0, rle_count =0; i < len; i++) {
					if (0 >= rle_count) {
						rle_count = *p++;
						pixel_data = *p++;
					}
					lcd_write_data(*p >> 8);
					lcd_write_data(pixel_data);
					--rle_count;
				}
			} else {
				for (i = 0; i < len; i++) {
					lcd_write_data(*p >> 8);
					lcd_write_data(*p++);
				}
			}
		} else {
			if (LCD_18BIT == LCD_Bus)
				CSL_LCDC_Enable_CE(handle, true);
			if (rle) {
				for (i = 0, rle_count =0; i < len; i++) {
					if (0 >= rle_count) {
						rle_count = *p++;
						pixel_data = *p++;
					}
					lcd_write_data(pixel_data);
					--rle_count;
				}
			} else {
				for (i = 0; i < len; i++)
					lcd_write_data(*p++);
			}
			if (LCD_18BIT == LCD_Bus)
				CSL_LCDC_Enable_CE(handle, false);
		}
	} else if (32 == LCD_device[0].bits_per_pixel)  {

		uint32_t pixel_data;

		CSL_LCDC_Enable_RGB888U(handle, true);
		if(rle) {
			for (i = 0, rle_count = 0; i < len; i++) {
				if (0 >= rle_count) {
					rle_count = *p++;
					rle_count |= (*p++ << 16);
					pixel_data = *p++;
					pixel_data |= (*p++ << 16);
				}
				lcd_write_data(pixel_data);
				--rle_count;
			}
		} else {
			for (i = 0; i < len; i++) {
				pixel_data = *p++;
				pixel_data |= (*p++ << 16);
				lcd_write_data(pixel_data);
			}
		}
		CSL_LCDC_Enable_RGB888U(handle, false);
	} else
		pr_info("lcd_send_data bpp=%d is not supported\n", LCD_device[0].bits_per_pixel);
}

static int lcdc_set_dev_border(LCD_dev_info_t *dev, int img_width, int img_height)
{
	if((dev->width < img_width) || (dev->height < img_height)) {
		pr_info("LCDC ERROR:img width %d is bigger than panel size %d!\n", img_width, dev->width);
		pr_info("LCDC ERROR:img height %d is bigger than panel size %d!\n", img_height, dev->height);
		return -1;
	}

	dev->col_start = (dev->width - img_width) / 2;
	dev->col_end = dev->width - (dev->width - img_width) / 2 - 1;
	dev->row_start = (dev->height - img_height) / 2;
	dev->row_end = dev->height - (dev->height - img_height) / 2 - 1;

	return 0;
}

static int lcdc_img_index = -1;
bool lcdc_showing_dump(void)
{
	return (lcdc_img_index!=-1);
}

int lcdc_disp_img(int img_index)
{
	uint16_t *data_ptr;
	int transfer_count;
	LCD_dev_info_t *dev = &LCD_device[0];

	CSL_LCDC_Lock(handle);

	switch (img_index)
	{
	case IMG_INDEX_AP_DUMP: 
		if (lcdc_set_dev_border(dev, dump_ap_start_img_w, dump_ap_start_img_h))
			return -1;
		data_ptr = ap_dump_start_img;
		transfer_count = dump_ap_start_img_w * dump_ap_start_img_h;
		break;
		
	case IMG_INDEX_CP_DUMP: 
		if (lcdc_set_dev_border(dev, dump_cp_start_img_w, dump_cp_start_img_h))
			return -1;
		data_ptr = cp_dump_start_img;
		transfer_count = dump_cp_start_img_w * dump_cp_start_img_h;
		break;

	case IMG_INDEX_END_DUMP: 
		if (lcdc_set_dev_border(dev, dump_end_img_w, dump_end_img_h))
			return -1;
		data_ptr = dump_end_img;
		transfer_count = dump_end_img_w * dump_end_img_h;
		break;
		
	default:
		return -1;
	}

	lcd_setup_for_data(dev);
	lcd_send_data(data_ptr, transfer_count, true);
	CSL_LCDC_Unlock(handle);
	lcdc_img_index = img_index;

	return 0;
}

#endif

/****************************************************************************
*
*  lcd_update_column
*
*  Update one column of LCD in non-DMA mode within dirty region.
*  Currently supports 8-bit and 16-bit bus width.
*
***************************************************************************/
static void lcd_update_column(LCD_dev_info_t * dev, unsigned int column)
{
	int i, stride;
	u32 source;

	stride = dev->width * dev->bits_per_pixel / 8;
	source = (u32)dev->frame_buffer.virtPtr + stride * dev->dirty_rect.top +
			column * dev->bits_per_pixel / 8;

	if (16 == LCD_device[0].bits_per_pixel) {
		uint16_t *p;
		if (readl(lcdc_base + REG_LCDC_CR) & REG_LCDC_CR_ENABLE_8_BIT_INTF) {
			 /* 8 bit bus */
			for (i = dev->dirty_rect.top; i <= dev->dirty_rect.bottom; i++) {
				p = (uint16_t *)source;
				lcd_write_data(*p >> 8);
				lcd_write_data(*p);
				source += stride;
			}
		} else {
			if (LCD_18BIT == LCD_Bus)
				CSL_LCDC_Enable_CE(handle, true);
			for (i = dev->dirty_rect.top; i <= dev->dirty_rect.bottom; i++) {
				p = (uint16_t *)source;
				lcd_write_data(*p);
				source += stride;
			}
			if (LCD_18BIT == LCD_Bus)
				CSL_LCDC_Enable_CE(handle, false);
		}
	} else if (32 == LCD_device[0].bits_per_pixel)  {
		uint32_t *p;
		uint32_t count;
		count = (dev->dirty_rect.bottom - dev->dirty_rect.top + 1);
		count &= ~1; /*Ignore one pixel in case count is odd*/
		CSL_LCDC_Enable_RGB888U(handle, true);
		for (i = 0; i < count; i++) {
			p = (uint32_t *)source;
			lcd_write_data(*p);
			source += stride;
		}
		CSL_LCDC_Enable_RGB888U(handle, false);
 	} else {
 		pr_info("bpp=%d is not supported\n", LCD_device[0].bits_per_pixel);
	}
}


/****************************************************************************
*
*  lcd_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/
void __exit lcd_exit(void)
{
	LCD_PUTS("enter");

#ifdef CONFIG_REGULATOR
	if (!IS_ERR_OR_NULL(lcdc_regulator)) {
		regulator_put(lcdc_regulator);
		lcdc_regulator = NULL;
	}
#endif
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&glcdfb_wake_lock);
#endif

	platform_driver_unregister(&lcdc_driver);
}				/* lcd_exit */

static int lcd_alloc_fb(LCD_dev_info_t * dev)
{
	if (dev->frame_buffer.virtPtr != NULL)
		return 0;

	/* dma_alloc_writecombine allocates uncached, buffered memory, that is */
	/* io_remappable */
	/* DBFrame : */
	dev->frame_buffer.sizeInBytes =
	    dev->width * dev->height * 2 * dev->bits_per_pixel / 8;

	dev->frame_buffer.virtPtr = dma_alloc_writecombine(NULL,
							   dev->
							   frame_buffer.sizeInBytes,
							   &dev->
							   frame_buffer.physPtr,
							   GFP_KERNEL);

	pr_info
	    ("[lcd] lcd_alloc_fb size=%#x virtPtr = 0x%lx, physPtr = 0x%lx \r\n",
	     dev->frame_buffer.sizeInBytes, (long)dev->frame_buffer.virtPtr,
	     (long)dev->frame_buffer.physPtr);
       printk("[lcd] lcd_alloc_fb size=%#x virtPtr = 0x%lx, physPtr = 0x%lx \r\n",
	     dev->frame_buffer.sizeInBytes, (long)dev->frame_buffer.virtPtr,
	     (long)dev->frame_buffer.physPtr);
	     
	if (dev->frame_buffer.virtPtr == NULL)
		return -ENOMEM;

	if ((dev->frame_buffer.physPtr & ~PAGE_MASK) != 0) {
		panic("lcd_init: We didn't get a page aligned buffer");
		return -ENOMEM;
	}

	memset(dev->frame_buffer.virtPtr, 0, dev->frame_buffer.sizeInBytes);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void lcd_early_suspend(struct early_suspend *h)
{
    	if (h->level == EARLY_SUSPEND_LEVEL_BLANK_SCREEN+5){
                   disable_irq(lcd_esd);
                   	gpio_direction_output(LCD_DET, 0);
                   lcd_esd_ckeck=0;
    	}
      else if (h->level == EARLY_SUSPEND_LEVEL_DISABLE_FB + 1) {
	lcd_pm_update(PM_COMP_PWR_OFF, 0);
}
}

static void lcd_late_resume(struct early_suspend *h)
{
      if (h->level == EARLY_SUSPEND_LEVEL_BLANK_SCREEN+5){
                   	gpio_direction_output(LCD_DET, 0);
                   	gpio_direction_input(LCD_DET); 
                   enable_irq(lcd_esd);
                   lcd_esd_ckeck=1;
    	}
    	else if (h->level == EARLY_SUSPEND_LEVEL_DISABLE_FB + 1) {
	lcd_pm_update(PM_COMP_PWR_ON, 0);
}
}

static struct early_suspend lcd_early_suspend_desc = {
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 1,
	.suspend = lcd_early_suspend,
	.resume = lcd_late_resume,
};

static struct early_suspend lcd_early_suspend_esd = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN+5,
	.suspend = lcd_early_suspend,
	.resume = lcd_late_resume,
};
#endif

/****************************************************************************
*
*  lcd_pm_update
*
*     Called by power manager to update component power level
*
***************************************************************************/
 int lcd_pm_update(PM_CompPowerLevel compPowerLevel,
			 PM_PowerLevel sysPowerLevel)
{
	static PM_CompPowerLevel powerLevel = PM_COMP_PWR_ON;
       struct timespec ts;
	struct rtc_time tm;

      LCD_DirtyRect_t init_dirtyRect = {
		.top = LCD_device[LCD_main_panel].height- 1,
		.left = 0,
		.right = LCD_device[LCD_main_panel].width - 1,
		.bottom = (LCD_device[LCD_main_panel].height*2)-1,
    };

	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);

	/* Nothing to do if power level did not change */
	if (compPowerLevel == powerLevel)
		return 0;
	/* Save new power level */
	powerLevel = compPowerLevel;
	switch (powerLevel) {
	case PM_COMP_PWR_OFF:
	case PM_COMP_PWR_STANDBY:
		{
			pr_info("\n[%02d:%02d:%02d.%03lu] LCDC: Power off panel\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
#ifdef CONFIG_REGULATOR
			if (!IS_ERR_OR_NULL(lcdc_regulator))
				regulator_disable(lcdc_regulator);
#endif
			lcd_poweroff_panels();              
			lcd_pwr_on_controll(OFF);
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
			cpufreq_bcm_dvfs_enable(lcdc_client);
#endif
			gInitialized = 0;
			break;
		}
	case PM_COMP_PWR_ON:
		{
			pr_info("\n[%02d:%02d:%02d.%03lu]  LCDC: Power on panel\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
			cpufreq_bcm_dvfs_disable(lcdc_client);
#endif
#ifdef CONFIG_REGULATOR
			if (!IS_ERR_OR_NULL(lcdc_regulator))
				regulator_enable(lcdc_regulator);
#endif
			lcd_pwr_on_controll(ON);
#if defined(CONFIG_ENABLE_QVGA) || defined(CONFIG_ENABLE_HVGA)
//comment for bug194796			display_black_background();
#endif

			lcd_init_panels();
            lcd_dev_dirty_rows(&LCD_device[LCD_main_panel], &init_dirtyRect);
			gInitialized = 1;
			break;
		}
	default:
		break;
	}

	return 0;
}
EXPORT_SYMBOL(lcd_pm_update);
    
static void lcd_esd_detect(struct work_struct *work)
{
	struct lcdc_platform_data_t *pdata = container_of(work, struct lcdc_platform_data_t, work);

      LCD_DirtyRect_t init_esd_dirtyRect = {
		.top = LCD_device[LCD_main_panel].height- 1,
		.left = 0,
		.right = LCD_device[LCD_main_panel].width - 1,
		.bottom = (LCD_device[LCD_main_panel].height*2)-1,
    };

      gpio_direction_output(LCD_DET, 1);    
	printk("[LCD] lcd_esd_detect_enter\n");

      if(lcd_esd_ckeck==1){
	printk("[LCD] %s, %d\n", __func__, __LINE__ );
	lcd_poweroff_panels();              
	lcd_pwr_on_controll(OFF);
      msleep(50); 
	lcd_pwr_on_controll(ON);
      	lcd_init_panels();
      lcd_esd_ckeck=1;      
      mdelay(500); 
      lcd_dev_dirty_rows(&LCD_device[LCD_main_panel], &init_esd_dirtyRect);
      }
      gpio_direction_output(LCD_DET, 0);
      gpio_direction_input(LCD_DET); 
      enable_irq(lcd_esd);
}    

static irqreturn_t lcd_esd_irq_handler(int irq, void *dev_id)
{
	struct lcdc_platform_data_t *pdata = dev_id;

	printk("[LCD] %s, %d\n", __func__, __LINE__ );
    
	disable_irq_nosync(lcd_esd);
	queue_work(lcd_wq, &pdata->work);

	return IRQ_HANDLED;
}

static int __init lcdc_probe(struct platform_device *pdev)
{
	int rc, i;
	struct resource *res;
	struct lcdc_platform_data_t *pdata;
	CSL_LCD_RES_T ret;

       lcd_enable=1;

	pr_info("%s for %s\n", gBanner, LCD_panel_name);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "can't get device resources\n");
		return -ENOENT;
	}

	lcdc_base = (void __iomem *)res->start;
	if (!lcdc_base) {
		dev_err(&pdev->dev, "couldn't get lcdc_base!\n");
		return -ENOENT;
	}

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "platform_data NULL!\n");
		return -ENOENT;
	}

	lcd_reset_gpio = pdata->gpio;
	if (!lcd_reset_gpio) {
		dev_err(&pdev->dev, "couldn't get GPIO!\n");
		return -ENOENT;
	}
	pr_info("lcd:probe lcdc_base = 0x%x gpio = %d\n", (int)lcdc_base,
		lcd_reset_gpio);

	/* Disable Tearing Effect control if the board doesn't support*/
	if (!pdata->te_supported) {
		pr_info("lcdc: Disabling te_support since the board doesn't support\n");
		LCD_device[LCD_main_panel].te_supported = false;
	}
      INIT_WORK(&pdata->work, lcd_esd_detect);
#ifdef CONFIG_REGULATOR
	lcdc_regulator = regulator_get(NULL, "lcd_vcc");
	if (!IS_ERR_OR_NULL(lcdc_regulator))
		regulator_enable(lcdc_regulator);
#endif

	/* Register our device with Linux */
	rc = register_chrdev(BCM_LCD_MAJOR, "lcd", &lcd_fops);
	if (rc < 0) {
		pr_warning("lcd: register_chrdev failed for major %d\n",
			   BCM_LCD_MAJOR);
		return rc;
	}
	/* Allocate memory for the framebuffers. */
	for (i = 0; i < LCD_num_panels; i++) {
		LCD_dev_info_t *dev = &LCD_device[i];

		rc = lcd_alloc_fb(dev);
		if (rc)
			return rc;
	}
	
	//{{ Mark for GetLog - 2/2
	frame_buf_mark.p_fb = (void*)(LCD_device[0].frame_buffer.physPtr - CONFIG_SDRAM_BASE_ADDR);  // it has dependency on project
	frame_buf_mark.bpp = LCD_device[0].bits_per_pixel;     // it has dependency on h/w 
	//}} Mark for GetLog - 2/2

      lcd_esd=GPIO_TO_IRQ(LCD_DET);
    
    #if defined(CONFIG_BCM_LCD_ILI9341_BOE) || defined(CONFIG_BCM_LCD_ILI9341_BOE_REV05)
       ret = request_irq(lcd_esd, lcd_esd_irq_handler,/* IRQF_TRIGGER_RISING |*/ IRQF_TRIGGER_RISING , "lcdc", pdata);
    #else
       ret = request_irq(lcd_esd, lcd_esd_irq_handler,/* IRQF_TRIGGER_RISING |*/ IRQF_TRIGGER_FALLING , "lcdc", pdata);
    #endif
    
        if(ret<0)
        {
            printk("ESD_IRQ setup failed!!\n");
        }


#ifndef CONFIG_BCM_LCD_SKIP_INIT
	/*GPIO configuration must be done before CSL Init */
	lcd_pwr_on_controll(ON);
#endif

	sema_init(&gDmaSema, 1);

	ctrl_init.maxBusWidth = LCD_Bus;
	ctrl_init.lcdc_base_address = (UInt32) lcdc_base;
	ret = CSL_LCDC_Init(&ctrl_init);
	if (CSL_LCD_OK != ret) {
		printk("Error: CSL_LCDC_Init returned %d\n", ret);
		return ret;
	}

	busCfg.cfg.busType = LCDC_BUS_Z80;
	busCfg.cfg.csBank = BUS_CH_0;
	busCfg.cfg.busWidth = LCD_Bus;
	if (32 == LCD_device[0].bits_per_pixel) {
		busCfg.cfg.colModeIn = LCD_IF_CM_I_RGB888U;
		busCfg.cfg.colModeOut = LCD_IF_CM_O_RGB888;
	} else {
		busCfg.cfg.colModeIn = LCD_IF_CM_I_RGB565P;
		busCfg.cfg.colModeOut = LCD_IF_CM_O_RGB666;
	}
	busCfg.speed.rdHold = 24;
	busCfg.speed.rdPulse = 25;
	busCfg.speed.rdSetup = 0;
	busCfg.speed.wrHold = 3;
	busCfg.speed.wrPulse = 3;
	busCfg.speed.wrSetup = 0;
	busCfg.io.slewFast = FALSE;
	busCfg.io.driveStrength = 3;
	if (LCD_device[LCD_main_panel].te_supported) {
		pr_info("Tearing Effect control enabled\n");
		busCfg.teCfg.type = LCDC_TE_CTRLR;
		busCfg.teCfg.delay = 0;
		busCfg.teCfg.pinSel = 0;
		busCfg.teCfg.edgeRising = true;
	} else {
		pr_info("Tearing Effect control disabled\n");
		busCfg.teCfg.type = LCDC_TE_NONE;
	}
	ret = CSL_LCDC_PAR_Open(&busCfg, &handle);
	if (CSL_LCD_OK != ret) {
		printk("Error: CSL_LCDC_PAR_Open returned %d\n", ret);
		return ret;
	}

	CSL_LCDC_Enable_CE(handle, false);

#if defined (CONFIG_BCM_LCD_S6D05A1X31_COOPERVE)	
    lcd_id = LCD_DRV_ID_Check();
    printk("lcd_probe : %x\n",   lcd_id );
#else 
    lcd_id = LCD_DRV_ID_Check(0x04,4);
    printk("lcd_probe : %x\n",   lcd_id );
#endif /*CONFIG_BCM_LCD_S6D05A1X31_COOPERVE*/

    if(lcd_id == 0)
        lcd_enable=0;
    
#ifndef CONFIG_BCM_LCD_SKIP_INIT
	lcd_init_panels();

#if defined(CONFIG_ENABLE_QVGA) || defined(CONFIG_ENABLE_HVGA)
	display_black_background();
#endif

#endif

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&glcdfb_wake_lock, WAKE_LOCK_SUSPEND, "lcdfb_wake_lock");
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&lcd_early_suspend_desc);
	register_early_suspend(&lcd_early_suspend_esd);
#endif

#ifdef CONFIG_CPU_FREQ_GOV_BCM21553
	lcdc_client = cpufreq_bcm_client_get("lcdc");
	if (!lcdc_client) {
		pr_err("%s: cpufreq_bcm_client_get failed\n", __func__);
		return -EIO;
	}

	/* Turn off dvfs at boot up so that the boot process is not
	 * slowed down.
	 */
	cpufreq_bcm_dvfs_disable(lcdc_client);
#endif
	gInitialized = 1;

	return 0;
}

#if defined(CONFIG_ENABLE_QVGA) || defined(CONFIG_ENABLE_HVGA)
void display_black_background(void)
{
	int transfer_count = PANEL_HEIGHT * PANEL_WIDTH;
#ifdef CONFIG_ARGB8888
	transfer_count *= 2;
#endif
	LCD_dev_info_t saved_dev_info = LCD_device[0];
	LCD_device[0].row_start = 0 - (PANEL_HEIGHT - LCD_HEIGHT) / 2;
	LCD_device[0].row_end = PANEL_HEIGHT - (PANEL_HEIGHT - LCD_HEIGHT) / 2;
	LCD_device[0].col_start = 0 - (PANEL_WIDTH - LCD_WIDTH) / 2;
	LCD_device[0].col_end = PANEL_WIDTH - (PANEL_WIDTH - LCD_WIDTH) / 2;
	lcd_setup_for_data(&LCD_device[0]);

	if (LCD_18BIT == LCD_Bus)
		CSL_LCDC_Enable_CE(handle, true);
	while (transfer_count--)
		lcd_write_data(0);
	if (LCD_18BIT == LCD_Bus)
		CSL_LCDC_Enable_CE(handle, false);
	LCD_device[0] = saved_dev_info;
}
#endif

static int lcdc_remove(struct platform_device *pdev)
{
	gpio_free(lcd_reset_gpio);
	return 0;
}

/****************************************************************************
*
*  lcd_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/
static int __init lcd_init(void)
{
	LCD_PUTS("enter");

    	gpio_request(LCD_DET, "lcd_esd");
	gpio_direction_input(LCD_DET);

	lcd_wq = create_workqueue("lcd_wq");
	if (!lcd_wq)
		return -ENOMEM;
 
	return platform_driver_register(&lcdc_driver);
}

/****************************************************************************
*
*  lcd_pwr_on
*
*     Power on controller
*
***************************************************************************/
void lcd_pwr_on_controll(int value)
{
	LCD_PUTS("enter");

	board_sysconfig(SYSCFG_LCD, SYSCFG_INIT);
	gpio_request(lcd_reset_gpio, "LCD Reset");
	/* Configure the GPIO pins */
	gpio_direction_output(lcd_reset_gpio, value);
	msleep(2);
	board_sysconfig(SYSCFG_LCD, SYSCFG_DISABLE);

}

/****************************************************************************
*
*  lcd_ioctl - TODO - lots of stuff needs to be filled in
*
***************************************************************************/
static long lcd_ioctl(struct file *file,
		     unsigned int cmd, unsigned long arg)
{
	int err = 0;
	LCD_dev_info_t *dev = (LCD_dev_info_t *) file->private_data;

	LCD_DEBUG("[lcd] lcdioctl: %d  type: '%c' cmd: 0x%x\r\n", dev->panel,
		  _IOC_TYPE(cmd), _IOC_NR(cmd));

	switch (cmd) {
	case LCD_IOCTL_RESET:
		gpio_set_value(lcd_reset_gpio, (int)arg);
		break;

	case LCD_IOCTL_ENABLE_BACKLIGHT:
		break;

	case LCD_IOCTL_ENABLE_SUB_BACKLIGHT:
		break;

	case LCD_IOCTL_ENABLE_CS:
		break;

	case LCD_IOCTL_SCOPE_TIMEOUT:
		break;

	case LCD_IOCTL_INIT:
		lcd_init_panels();
		break;

	case LCD_IOCTL_INIT_ALL:
		break;

	case LCD_IOCTL_SETUP:
		break;

	case LCD_IOCTL_HOLD:
		break;

	case LCD_IOCTL_PULSE:
		break;

	case LCD_IOCTL_REG:
		{
			LCD_Reg_t r;

			if (copy_from_user(&r, (LCD_Reg_t *) arg, sizeof(r)) !=
			    0)
				return -EFAULT;

			break;
		}

	case LCD_IOCTL_RECT:
		{
			LCD_Rect_t r;

			if (copy_from_user(&r, (LCD_Rect_t *) arg, sizeof(r)) !=
			    0)
				return -EFAULT;

			lcd_display_rect(dev, &r);
			break;
		}

	case LCD_IOCTL_COLOR_TEST:
		break;

	case LCD_IOCTL_DIRTY_ROWS:
		{
			LCD_DirtyRows_t dirtyRows;

			if (copy_from_user(&dirtyRows, (LCD_DirtyRows_t *) arg,
					   sizeof dirtyRows) != 0)
				return -EFAULT;

			lcd_dev_dirty_rows(dev, &dirtyRows);
			break;
		}

	case LCD_IOCTL_DIRTY_RECT:
		{
			LCD_DirtyRect_t dirtyRect;

			if (copy_from_user(&dirtyRect, (LCD_DirtyRect_t *) arg,
					   sizeof dirtyRect) != 0)
				return -EFAULT;

			lcd_dev_dirty_rect(dev, &dirtyRect);
			break;
		}

	case LCD_IOCTL_PRINT_REGS:
		break;

	case LCD_IOCTL_PRINT_DATA:
		break;

	case LCD_IOCTL_PWR_OFF:
		break;

	case LCD_IOCTL_INFO:
		{
			LCD_Info_t lcdInfo;

			LCD_DEBUG("cmd=LCD_IOCTL_INFO arg=0x%08lX", arg);
			lcd_get_info(&lcdInfo);
			err =
			    copy_to_user((void *)arg, &lcdInfo,
					 sizeof(LCD_Info_t));
			break;
		}

	default:
		LCD_DEBUG("Unrecognized ioctl: '0x%x'\n", cmd);
		return -ENOTTY;
	}

	return err;
}

/****************************************************************************
*
*  lcd_get_info
*
*
*
***************************************************************************/
void lcd_get_info(LCD_Info_t * lcdInfo)
{
	LCD_dev_info_t *dev = &LCD_device[LCD_main_panel];

	lcdInfo->bitsPerPixel = dev->bits_per_pixel;
	lcdInfo->height = dev->height;
	lcdInfo->width = dev->width;
	lcdInfo->physical_width = dev->physical_width;
	lcdInfo->physical_height = dev->physical_height;
}

/****************************************************************************
*
*  lcd_mmap
*
*   Note that the bulk of this code came from the fb_mmap routine found in
*   drivers/video/fbmem.c
*
***************************************************************************/
static int lcd_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long offset;
	unsigned long start;
	unsigned long len;
	LCD_dev_info_t *dev;
	LCD_FrameBuffer_t *fb;

	/* vma->vm_start    is the start of the memory region, in user space */
	/* vma->vm_end      is one byte beyond the end of the memory region, in user space */
	/* vma->vm_pgoff    is the offset (in pages) within the vm_start to vm_end region */

	LCD_PUTS("enter");
	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT)) {
		pr_info("lcd: vm_pgoff is out of range\n");
		return -EINVAL;
	}

	if (file == NULL || file->private_data == NULL) {
		pr_info("lcd: bad file pointer or LCD-device pointer\n");
		return -EINVAL;
	}

	dev = (LCD_dev_info_t *) file->private_data;
	fb = &dev->frame_buffer;

	/* Convert offset into a byte offset, rather than a page offset */
	offset = vma->vm_pgoff << PAGE_SHIFT;
	start = (unsigned long)fb->physPtr;	/* already page-aligned */
	pr_info("[lcd] lcd_mmap %lx %lx  \r\n", offset, start);

	len = PAGE_ALIGN(start + fb->sizeInBytes);

	if (offset > len) {
		/* The pointer requested by the user isn't inside of our frame buffer */
		LCD_DEBUG("offset is too large, offset = %lu, len = %lu\n",
			  offset, len);
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	offset += start;
	vma->vm_pgoff = offset >> PAGE_SHIFT;

	if (0 != io_remap_pfn_range(vma,
				    vma->vm_start,
				    offset >> PAGE_SHIFT,
				    vma->vm_end - vma->vm_start,
				    vma->vm_page_prot)) {
		LCD_DEBUG("remap_page_range failed\n");
		return -EAGAIN;
	}

	return 0;
}

/****************************************************************************
*
*  lcd_open
*
***************************************************************************/
static int lcd_open(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);

	LCD_DEBUG("major = %d, minor = %d\n", imajor(inode), minor);

	/* minor number must match values for LCD_panel_t */
	if (minor < 0 || minor >= LCD_num_panels) {
		pr_info("lcd: bad minor number %d; range is 0..%d\n",
			minor, LCD_num_panels - 1);
		return -EINVAL;
	}
	/* set our private pointer to the correct LCD_dev_info_t */
	file->private_data = (void *)&LCD_device[minor];

	return 0;
}

/****************************************************************************
*
*  lcd_release
*
***************************************************************************/
static int lcd_release(struct inode *inode, struct file *file)
{
	LCD_PUTS("enter");
	return 0;
}

/****************************************************************************
*
*  lcd_get_framebuffer_addr
*
*   Gets the address of the primary frame buffer
*
***************************************************************************/
void *lcd_get_framebuffer_addr(int *frame_size, dma_addr_t * dma_addr)
{
	int rc;
	LCD_dev_info_t *dev = &LCD_device[LCD_main_panel];

	/*  Check if we have been initialized yet.  If not, another driver wants */
	/*  access to our framebuffer before we have been inited.  In this case, */
	/*  allocate the framebuffer now to avoid the other driver failing. */
	/* (lcd_alloc_fb() takes care not to reinitialize itself.) */

	rc = lcd_alloc_fb(dev);

	if (rc)
		return NULL;

	if (dma_addr)
		*dma_addr = dev->frame_buffer.physPtr;

	if (frame_size)
		*frame_size = dev->frame_buffer.sizeInBytes;

	pr_info("[lcd] lcd_get_frame %d 0x%x \r\n",
		dev->frame_buffer.sizeInBytes,
		(unsigned)dev->frame_buffer.virtPtr);

	return dev->frame_buffer.virtPtr;
}

/*Function related to panel*/
static void lcd_send_cmd_sequence(Lcd_init_t *init)
{
    int i;

    for (i = 0; init[i].type != CTRL_END; i++) {
	switch (init[i].type) {
	case WR_CMND:
		lcd_write_cmd(LCD_CMD(init[i].cmd));
		break;
	case WR_DATA:
		lcd_write_data(LCD_CMD(init[i].data));
		break;
	case WR_CMND_DATA:
		lcd_write_cmd(LCD_CMD(init[i].cmd));
		lcd_write_data(LCD_DATA(init[i].data));
		break;
	case SLEEP_MS:
		msleep(init[i].data);
		break;
	default:
		break;
	}
    }
}

static inline void lcd_init_panels(void)
{
    	if(!lcd_id) {
#if defined (CONFIG_BCM_LCD_S6D05A1X31_COOPERVE)
	   	lcd_id = LCD_DRV_ID_Check();
#else
	   	lcd_id = LCD_DRV_ID_Check(0x04,4);
#endif
	}
		
#if 0
	lcd_pwr_on_controll(OFF);
    mdelay(100);
	lcd_pwr_on_controll(ON);
        mdelay(100);
#endif

#if defined(CONFIG_BCM_LCD_S6D04H0A01) || defined(CONFIG_BCM_LCD_ILI9341_BOE) || defined(CONFIG_BCM_LCD_ILI9341_BOE_REV05) //TOTORO
    if(lcd_id==PANEL_BOE)
        lcd_send_cmd_sequence(power_on_seq_s6d04h0_boe);
    else if(lcd_id==PANEL_SMD)
	lcd_send_cmd_sequence(power_on_seq_s6d04h0_smd);
#elif defined(CONFIG_BCM_LCD_S6D04K1)//LUISA_HW00
	lcd_send_cmd_sequence(power_on_seq_s6d04k1_sdi);
#elif defined(CONFIG_BCM_LCD_S6D04K1_LUISA_HW02)
	#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
	if (lcd_frame_inversion_during_call == 1)//during call
		lcd_send_cmd_sequence(power_on_seq_s6d04k1_sdi_during_call);
	else//idle
	#endif
	lcd_send_cmd_sequence(power_on_seq_s6d04k1_sdi);
#elif defined(CONFIG_BCM_LCD_S6D04H0A01_TASSVE)
	lcd_send_cmd_sequence(power_on_seq_s6d04h0_boe);
#elif defined(CONFIG_BCM_LCD_S6D05A1X31_COOPERVE)
	if(lcd_id == PANEL_DTC)
		lcd_send_cmd_sequence(power_on_seq_s5d05a1x31_cooperve_DTC);
	else if(lcd_id==PANEL_AUO)
		lcd_send_cmd_sequence(power_on_seq_s5d05a1x31_cooperve_AUO);
	else if(lcd_id==PANEL_SHARP)
		lcd_send_cmd_sequence(power_on_seq_s5d05a1x31_cooperve_SHARP);
	else
	{
		printk("[Caution] Unknown lcd (id: 0x%x )\r\n", lcd_id);	
		lcd_send_cmd_sequence(power_on_seq_s5d05a1x31_cooperve_DTC);
	}
#endif

}

static inline void lcd_poweroff_panels(void)
{
#if defined(CONFIG_BCM_LCD_S6D05A1X31_COOPERVE)
	if(lcd_id == PANEL_DTC)	
		lcd_send_cmd_sequence(power_off_seq_DTC);
	else if(lcd_id==PANEL_AUO)
		lcd_send_cmd_sequence(power_off_seq_AUO);
	else if(lcd_id==PANEL_SHARP)
		lcd_send_cmd_sequence(power_off_seq_SHARP);		
	else
		lcd_send_cmd_sequence(power_off_seq_DTC);
#else
	lcd_send_cmd_sequence(power_off_seq);
#endif
}


static void lcd_setup_for_data(LCD_dev_info_t *dev)
{
#if defined(CONFIG_ENABLE_QVGA) || defined(CONFIG_ENABLE_HVGA)
    /*Centre the display by offsetting the co-ordinates*/
    dev->row_start += (PANEL_HEIGHT - LCD_HEIGHT) / 2;
    dev->row_end += (PANEL_HEIGHT - LCD_HEIGHT) / 2;
    dev->col_start += (PANEL_WIDTH - LCD_WIDTH) / 2;
    dev->col_end += (PANEL_WIDTH - LCD_WIDTH) / 2;
#endif

	Lcd_init_t resetSeq[] = {
		RESET_SEQ,
		{CTRL_END, 0, 0},
	};
	lcd_send_cmd_sequence(resetSeq);

#if defined(CONFIG_ENABLE_QVGA) || defined(CONFIG_ENABLE_HVGA)
    /* Roll back to (0,0) relative co-ordindates*/
    dev->row_start -= (PANEL_HEIGHT - LCD_HEIGHT) / 2;
    dev->row_end -= (PANEL_HEIGHT - LCD_HEIGHT) / 2;
    dev->col_start -= (PANEL_WIDTH - LCD_WIDTH) / 2;
    dev->col_end -= (PANEL_WIDTH - LCD_WIDTH) / 2;
#endif
}


/* XXX - temp hacks */
void lcd_display_test(LCD_dev_info_t * dev)
{
	int i, j;
	uint16_t *fb;
	LCD_DirtyRows_t dirtyRows;

	fb = dev->frame_buffer.virtPtr;
	dirtyRows.top = 0;
	dirtyRows.bottom = 479;
	for (i = 50; i < 100; i++)
		for (j = 50; j < 100; j++)
			if (i < j)
				fb[i * dev->width + j] = LCD_COLOR_YELLOW >> 1;

	lcd_dev_dirty_rows(dev, &dirtyRows);
}

void lcd_display_rect(LCD_dev_info_t * dev, LCD_Rect_t * r)
{
	int i, j;
	uint16_t *fb;
	LCD_DirtyRows_t dirtyRows;

	fb = dev->frame_buffer.virtPtr;

	if (r->top < 0)
		r->top = 0;
	if (r->top > dev->height)
		r->top = dev->height;
	if (r->left < 0)
		r->left = 0;
	if (r->left > dev->width)
		r->left = dev->width;

	for (i = r->top; i < (r->top + r->height); i++)
		for (j = r->left; j < (r->left + r->width); j++)
			fb[i * dev->width + j] = r->color;

	dirtyRows.top = r->top;
	dirtyRows.bottom = r->top + r->height;

	if (dirtyRows.bottom >= dev->height)
		dirtyRows.bottom -= 1;

	lcd_dev_dirty_rows(dev, &dirtyRows);
}

#if defined(CONFIG_LCD_FRAME_INVERSION_DURING_CALL)
/* ++ LCD frame inversion during call ++ */
/* during call */
void lcd_enter_during_call(void)
{
	printk("[LCD] %s, %d\n", __func__, __LINE__ );
	lcd_frame_inversion_during_call = 1;
	lcd_send_cmd_sequence(enter_during_call_seq);
}
EXPORT_SYMBOL(lcd_enter_during_call);

/* idle */
void lcd_restore_during_call(void)
{
	printk("[LCD] %s, %d\n", __func__, __LINE__ );
	lcd_frame_inversion_during_call = 0;
	lcd_send_cmd_sequence(restore_during_call_seq);
}
EXPORT_SYMBOL(lcd_restore_during_call);
/* -- LCD frame inversion during call -- */
#endif

/****************************************************************************/

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom L2F50219P00 LCD Driver");
