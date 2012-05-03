/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/broadcom/fb/lcdfb.c
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

/*
*
*****************************************************************************
*
*  lcdfb.c
*
*  PURPOSE:
*
*   This implements the LCD FrameBuffer driver used on the Wifi Phone.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/broadcom/bcm_sysctl.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/console.h>
#include <linux/kthread.h>

#include <linux/broadcom/lcd.h>	/*  We make use of a lower level driver for the LCD */

#include <cfg_global.h>

#include <cfg_global.h>

#include "lcdfb.h"

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#if 0
#define LCD_DEBUG(fmt, args...) pr_info("%s: " fmt, __FUNCTION__,  ## args)
#else
#define LCD_DEBUG(fmt, args...)
#endif

static int gLcdDisplayWidth;
static int gLcdDisplayHeight;
static int gLcdBitsPerPixel;
static int gLcdBytesPerPixel;
static int gLcdFrameBufferSizeBytes;

/* array to store 16 color index */
static u32 gLcdPseudoPalette[256] = {
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
	0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800
};

/*  These defines come from cfbimgblt.c for the BitBLIT function. */
#define FB_WRITEL fb_writel
#define FB_READL  fb_readl

#if defined (__BIG_ENDIAN)
#define LEFT_POS(bpp)          (32 - bpp)
#define SHIFT_HIGH(val, bits)  ((val) >> (bits))
#define SHIFT_LOW(val, bits)   ((val) << (bits))
#else
#define LEFT_POS(bpp)          (0)
#define SHIFT_HIGH(val, bits)  ((val) << (bits))
#define SHIFT_LOW(val, bits)   ((val) >> (bits))
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
/* early suspend support */
int gLcdfbEarlySuspendStopDraw;
EXPORT_SYMBOL(gLcdfbEarlySuspendStopDraw);
#endif

/* ---- Private Variables ------------------------------------------------ */

static char gBanner[] __initdata =
    KERN_INFO "Broadcom LCD Frame Buffer Driver: 0.03\n";

/* ---- Private Function Prototypes -------------------------------------- */

static int lcdfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			   u_int trans, struct fb_info *info);
static int lcdfb_ioctl(struct fb_info *info, u_int cmd, u_long arg);

int lcdfb_init(void);
int lcdfb_setup(char *);
static int lcdfb_blank(int blank_mode, struct fb_info *info);
static int lcdfb_sync(struct fb_info *info);

static int lcdfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info);
static int lcdfb_set_par(struct fb_info *info);
static int lcdfb_pan_display(struct fb_var_screeninfo *var,
			     struct fb_info *info);
#ifdef CONFIG_FB_VC_ACCELE
static void lcdfb_cfb_imageblit(struct fb_info *p,
				const struct fb_image *image);
static void lcdfb_copyarea(struct fb_info *p, const struct fb_copyarea *area);
static void lcdfb_fillrect(struct fb_info *p, const struct fb_fillrect *rect);
static void lcdfb_fillrect_color(struct fb_info *p,
				 LCD_FillRectColor_t * rect_c);
#endif

extern int current_intensity;

/*  Thse variables are used to keep track of the Video memory if we need to */
/*  allocate our own framebuffer. */
static void *videomemory;
static int videomemorysize;
static dma_addr_t physmemory;
module_param(videomemorysize, int, 0);

/*  Thsee variables are used to control the periodic LCD refresh. */
#define DEFAULT_LCD_REFRESH_TIMEOUT HZ
/* enable timer based auto-update of dirty buffers */
static int gLcdRefreshOn;
static int gLcdRefreshFreq = HZ / 2;
static int gLcdRefreshTimeout = DEFAULT_LCD_REFRESH_TIMEOUT;
static long gLcdRefreshThreadId;
static struct task_struct *gLcdRefreshtask;
static struct completion gLcdRefreshExited;
static wait_queue_head_t gLcdRefreshWaitQueue;

static struct ctl_table_header *gSysCtlHeader;

static struct ctl_table gSysCtlFb[] = {
	{
	 .procname = "refresh-on",
	 .data = &gLcdRefreshOn,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{
	 .procname = "refresh-freq",
	 .data = &gLcdRefreshFreq,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{}
};

static struct ctl_table gSysCtl[] = {
	{
	 .procname = "fb",
	 .mode = 0555,
	 .child = gSysCtlFb},
};

/*  This is the default value for the FB info block: */
static struct fb_var_screeninfo lcdfb_default __initdata = {
	/* .xres =               calculated */
	/* .yres =               calculated */
	/* .xres_virtual =       calculated */
	/* .yres_virtual =       calculated */
	/* .bits_per_pixel = calculated */
	.xoffset = 0,
	.yoffset = 0,
	.bits_per_pixel = 16,
	.grayscale = 0,
	.red = {11, 5, 0},	/* for 16 bpp */
	.green = {5, 6, 0},	/* for 16 bpp */
	.blue = {0, 5, 0},	/* for 16 bpp */
	.transp = {0, 0, 0},
	.activate = FB_ACTIVATE_NOW,
	.height = 320,
	.width = 240,
	.pixclock = 0,
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.vmode = FB_VMODE_NONINTERLACED,
};

/*  This is the default value for the FB fix block: */
static struct fb_fix_screeninfo lcdfb_fix __initdata = {
	.id = LCDFB_NAME,
	.type = FB_TYPE_PACKED_PIXELS,
	.visual = FB_VISUAL_TRUECOLOR,
	/* .line_length      = calculated */
	.xpanstep = 1,
	.ypanstep = 1,
	.ywrapstep = 1,
	.accel = FB_ACCEL_NONE,
	/* .smem_len     = calculated */
};

static int lcdfb_enable __initdata;	/* disabled by default */
module_param(lcdfb_enable, int, 0);

/*  These are the frambuffer functions we are implementing: */
static struct fb_ops lcdfb_ops = {
	.owner = THIS_MODULE,
	.fb_check_var = lcdfb_check_var,
	.fb_set_par = lcdfb_set_par,
	.fb_setcolreg = lcdfb_setcolreg,
	.fb_blank = lcdfb_blank,
	.fb_pan_display = lcdfb_pan_display,
#ifdef CONFIG_FB_VC_ACCELE
	.fb_fillrect = lcdfb_fillrect,
	.fb_copyarea = lcdfb_copyarea,
/*      .fb_imageblit   = lcdfb_cfb_imageblit, */
	.fb_imageblit = cfb_imageblit,
#else
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
#endif
	.fb_rotate = NULL,
	.fb_sync = lcdfb_sync,
	.fb_ioctl = lcdfb_ioctl,
};

/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
*
*  ChanToField
*
*   Converts a value and field definition into the appropriately shifted bitfield.
*
***************************************************************************/

static inline u32 ChanToField(u32 chan, struct fb_bitfield *bf, u32 bpp)
{
	switch (bpp) {
	case 16:
		chan &= 0xFFFF;
		break;

	case 32:
		break;

	default:		/* only support 16 or 32 bpp */
		return 0;
	}

	chan >>= bpp - bf->length;
	return chan << bf->offset;

}				/* ChanToField */

/****************************************************************************
*
*  ExtractField
*
*   Extract the specified channel field from a given raw color.
*
***************************************************************************/
static inline u32 ExtractField(u32 rawColor, struct fb_bitfield *bf, u32 bpp)
{
	switch (bpp) {
	case 16:
		rawColor &= 0xFFFF;
		break;

	case 32:
		break;

	default:		/* only support 16 or 32 bpp */
		return 0;
	}

	rawColor = rawColor >> bf->offset;
	rawColor = rawColor << (bpp - bf->length);

	return rawColor;
}				/* ExtractField */

/****************************************************************************
*
*  lcdfb_refresh_thread
*
*  Worker thread to perodically update the LCD.  Updates are only performed if
*  enabled by proc entry /proc/sys/fb/refresh-on.  The update rate is controlled by
*  proc entry /proc/sys/fb/refresh-freq.
*
****************************************************************************/

static int lcdfb_refresh_thread(void *data)
{
	/* This thread doesn't need any user-level access, so get rid of */
	/* all our resources */

	daemonize("lcd_periodic_refresh");

	while (wait_event_interruptible_timeout
	       (gLcdRefreshWaitQueue, 0, gLcdRefreshTimeout) == 0) {
		if (gLcdRefreshOn == 0) {
			/* Periodic refresh is disabled, so apply a relatively long timeout */
			gLcdRefreshTimeout = DEFAULT_LCD_REFRESH_TIMEOUT;
		} else {
			/* Periodic refresh is enabled, so apply the specified timeout and update */
			/* the display */
			LCD_DirtyRows_t dirtyRows;

			gLcdRefreshTimeout = gLcdRefreshFreq;
			dirtyRows.top = 0;
			dirtyRows.bottom = gLcdDisplayHeight - 1;
			lcd_dirty_rows(&dirtyRows);
		}
	}

	complete_and_exit(&gLcdRefreshExited, 0);

}				/* lcdfb_refresh_thread */

/****************************************************************************
*
*  lcdfb_init_lcd
*
*   Initialization routine, called from lcdfb_init.
*
***************************************************************************/

int lcdfb_init_lcd(void)
{
	/* Initialize our side of the controller hardware */
	/* NOTE:  This is all currently being done by the LCD */
	/*        character driver. */

	/* Initialize the thread that will, if enabled by the user, do periodic */
	/* LCD updates */
	init_waitqueue_head(&gLcdRefreshWaitQueue);
	init_completion(&gLcdRefreshExited);
	gLcdRefreshtask = kthread_run(lcdfb_refresh_thread, NULL,
				      "lcd_periodic_refresh");
	gLcdRefreshThreadId = gLcdRefreshtask->pid;

#ifdef CONFIG_LOGO
	{
		/* We call a routine in the LCD character driver to update the LCD. */
		LCD_DirtyRows_t dirtyRows;
		dirtyRows.top = 0;
		dirtyRows.bottom = gLcdDisplayHeight - 1;
		lcd_dirty_rows(&dirtyRows);
	}
#endif

	/* Since we got this far, everything was successful */

	return 0;

}				/* lcdfb_init_lcd */

/****************************************************************************
*
*  lcdfb_ioctl
*
*   Adds specialized functionality to the our framebuffer.
*
***************************************************************************/

static int lcdfb_ioctl(struct fb_info *info, u_int cmd, u_long arg)
{
	int err = 0;
	LCD_DirtyRows_t dirtyRows;

#ifdef CONFIG_HAS_EARLYSUSPEND
	if (gLcdfbEarlySuspendStopDraw)
		return -ENODEV;
#endif

	switch (cmd) {
	case LCDFB_IOCTL_UPDATE_LCD:
		{
			/*  If the use has called this command, it means they want to */
			/*  control LCD refreshes.  Turn off our timer. */
			gLcdRefreshOn = 0;

			if (copy_from_user
			    (&dirtyRows, (LCD_DirtyRows_t *) arg,
			     sizeof(dirtyRows)) != 0) {
				return -EFAULT;
			}
			/*  We call a routine in the LCD character driver to update the LCD. */
			lcd_dirty_rows(&dirtyRows);
		}
		break;

	case LCD_IOCTL_DIRTY_ROW_BITS:
		{
			return -EINVAL;
		}

	case LCD_IOCTL_IS_DISPLAY_REGION_SUPPORTED:
		{
			return -EINVAL;
		}
		break;

	case LCD_IOCTL_IS_DIRTY_ROW_UPDATE_SUPPORTED:
		{
			return 0;
		}
		break;

	case LCD_IOCTL_COPYAREA:
		{
			struct fb_copyarea area;

			if (copy_from_user(&area, (struct fb_copyarea *)arg,
					   sizeof(area)) != 0) {
				return -EFAULT;
			}

			info->fbops->fb_copyarea(info, &area);
			return 0;
		}
		break;

	case LCD_IOCTL_FILLRECT_COLOR:
		{
			LCD_FillRectColor_t rect_c;

			if (copy_from_user(&rect_c, (LCD_FillRectColor_t *) arg,
					   sizeof(rect_c)) != 0) {
				return -EFAULT;
			}

			info->fbops->fb_fillrect(info,
						 (const struct fb_fillrect *)
						 &rect_c);
			return 0;
		}
		break;

	default:
		return -EINVAL;
	}
	return err;

}				/* lcdfb_ioctl */

/****************************************************************************
*
*  lcdfb_set_colreg
*
*   Sets a color register
*
***************************************************************************/

static int lcdfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			   u_int trans, struct fb_info *info)
{
	u_int val;
	u32 bpp = info->var.bits_per_pixel;
	int rc = 1;

#ifdef CONFIG_HAS_EARLYSUSPEND
	if (gLcdfbEarlySuspendStopDraw)
		return -ENODEV;
#endif

	/*
	 * If greyscale is true, then we convert the RGB value
	 * to greyscale no mater what visual we are using.
	 */
	if (info->var.grayscale) {
		red = green = blue = ((19595 * red)
				      + (38470 * green)
				      + (7471 * blue)) >> 16;
	}

	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
	case FB_VISUAL_DIRECTCOLOR:
		{
			/*
			 * 12 or 16-bit True Colour.  We encode the RGB value
			 * according to the RGB bitfield information.
			 */

			if (regno < 16) {
				u32 *pal = info->pseudo_palette;

				val = ChanToField(red, &info->var.red, bpp);
				val |=
				    ChanToField(green, &info->var.green, bpp);
				val |= ChanToField(blue, &info->var.blue, bpp);
				val |=
				    ChanToField(trans, &info->var.transp, bpp);

				pal[regno] = val;
				rc = 0;
			}
			break;
		}
	}

	return rc;

}				/* lcdfb_set_colreg */

static int lcdfb_blank(int blank_mode, struct fb_info *info)
{
	return 0;
}

/**
 * lcdfb_sync - NOT a required function. Normally the accel engine
 *     for a graphics card take a specific amount of time.
 *     Often we have to wait for the accelerator to finish
 *     its operation before we can write to the framebuffer
 *     so we can have consistent display output.
 *
 * @info: frame buffer structure that represents a single frame buffer
 */
static int lcdfb_sync(struct fb_info *info)
{
	return 0;
}

/****************************************************************************
*
*  lcdfb_check_var
*
*   Validates the user params, rounding as appropriate, and returns -EINVAL
*   if the value is completely out of range.
*
***************************************************************************/

static int lcdfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	int rc = -EINVAL;

#ifdef CONFIG_HAS_EARLYSUSPEND
	if (gLcdfbEarlySuspendStopDraw)
		return -ENODEV;
#endif

	/* Our screen size is fixed. */

	var->xres = gLcdDisplayWidth;
	var->yres = gLcdDisplayHeight;

	var->xres_virtual = gLcdDisplayWidth;
	/* var->yres_virtual = gLcdDisplayHeight; */
	/* DBFrame : */
	var->yres_virtual = gLcdDisplayHeight * 2;

	/* Our pixel depth is fixed. */

	if (var->bits_per_pixel == gLcdBitsPerPixel) {
		rc = 0;
	}

	return rc;

}				/* lcdfb_check_var */

/****************************************************************************
*
*  get_line_length
*
*   Determines the line length.
*
***************************************************************************/

static u_long get_line_length(int xres_virtual, int bpp)
{
	u_long length;

	length = xres_virtual * bpp;
	length = (length + 31) & ~31;
	length >>= 3;
	return length;
}				/* get_line_length */

/****************************************************************************
*
*  lcdfb_set_par
*
*   Sets the line length in the fix structure.
*
***************************************************************************/

static int lcdfb_set_par(struct fb_info *info)
{
#ifdef CONFIG_HAS_EARLYSUSPEND
	if (gLcdfbEarlySuspendStopDraw)
		return -ENODEV;
#endif

	info->fix.line_length = get_line_length(info->var.xres_virtual,
						info->var.bits_per_pixel);
	return 0;
}				/* lcdfb_set_par */

/****************************************************************************
*
*  lcdfb_pan_display
*
*   Pans the display.  This routine came from the virtual frame buffer.
*
***************************************************************************/

static int lcdfb_pan_display(struct fb_var_screeninfo *var,
			     struct fb_info *info)
{
	/* pr_info(" ++lcdfb_pan_display++ \r\n"); */
#ifdef CONFIG_HAS_EARLYSUSPEND
	if (gLcdfbEarlySuspendStopDraw)
		return -ENODEV;
#endif

	if (var->vmode & FB_VMODE_YWRAP) {
		if (var->yoffset < 0
		    || var->yoffset >= info->var.yres_virtual || var->xoffset)
			return -EINVAL;
	} else {
		if (var->xoffset + var->xres > info->var.xres_virtual ||
		    var->yoffset + var->yres > info->var.yres_virtual)
			return -EINVAL;
	}
	info->var.xoffset = var->xoffset;
	info->var.yoffset = var->yoffset;
	if (var->vmode & FB_VMODE_YWRAP)
		info->var.vmode |= FB_VMODE_YWRAP;
	else
		info->var.vmode &= ~FB_VMODE_YWRAP;
	{
		if (var->reserved[0] == 'TDPU') {
			/* Android partial update */
			/* reserved[0] = 0x54445055; "UPDT" */
			/* reserved[1] = left | (top << 16); */
			/* reserved[2] = (left + width) | \
			   ((top + height) << 16); */
			LCD_DirtyRect_t dirtyRect;
			dirtyRect.left = (u16) var->reserved[1];
			dirtyRect.top = var->yoffset + (var->reserved[1] >> 16);
			/* adjust by 1 as dirtyRect.right is inclusive */
			dirtyRect.right = (u16) var->reserved[2] - 1;
			/* adjust by 1 as dirtyRect.bottom is inclusive */
			dirtyRect.bottom = var->yoffset +
			    ((var->reserved[2] >> 16) - 1);
			lcd_dirty_rect(&dirtyRect);
		} else {
			LCD_DirtyRows_t dirtyRows;
			dirtyRows.top = var->yoffset;
			dirtyRows.bottom = var->yoffset + info->var.yres - 1;
			lcd_dirty_rows(&dirtyRows);
		}
	}

	return 0;

}				/* lcdfb_pan_display */

/****************************************************************************
*
*  color_imageblit
*
*   Utility function for the BitBLT function.
*   This is a modified version of the one in cfbimgblt.c.
*
***************************************************************************/

static inline void color_imageblit(const struct fb_image *image,
				   struct fb_info *p, u8 * dst1,
				   u32 start_index, u32 pitch_index)
{
	/* Draw the penguin */
	u32 *dst, *dst2;
	u32 color = 0, val, shift;
	int i, n, bpp = p->var.bits_per_pixel;
	u32 null_bits = 32 - bpp;
	u32 *palette = (u32 *) p->pseudo_palette;
	const u8 *src = image->data;

	dst2 = (u32 *) dst1;
	for (i = image->height; i--;) {
		n = image->width;
		dst = (u32 *) dst1;
		shift = 0;
		val = 0;

		if (start_index) {
			u32 start_mask = ~(SHIFT_HIGH(~(u32) 0, start_index));
			val = FB_READL(dst) & start_mask;
			shift = start_index;
		}
		while (n--) {
			if (p->fix.visual == FB_VISUAL_TRUECOLOR
			    || p->fix.visual == FB_VISUAL_DIRECTCOLOR)
				color = palette[*src];
			else
				color = *src;
			color <<= LEFT_POS(bpp);
			val |= SHIFT_HIGH(color, shift);

			if (shift >= null_bits) {
				FB_WRITEL(val, dst++);

				val = (shift == null_bits) ? 0 :
				    SHIFT_LOW(color, 32 - shift);
			}
			shift += bpp;
			shift &= (32 - 1);
			src++;
		}
		if (shift) {
			u32 end_mask = SHIFT_HIGH(~(u32) 0, shift);

			FB_WRITEL((FB_READL(dst) & end_mask) | val, dst);
		}
		dst1 += p->fix.line_length;
		if (pitch_index) {
			dst2 += p->fix.line_length;
			dst1 = (u8 *) ((long __force)dst2 & ~(sizeof(u32) - 1));

			start_index += pitch_index;
			start_index &= 32 - 1;
		}
	}
}				/* color_imageblit */

#ifdef CONFIG_FB_VC_ACCELE
/****************************************************************************
*
*  lcdfb_cfb_imageblit
*
*   BitBLT function specific to this framebuffer design.
*   This is a modified version of the one in cfbimgblt.c.
*
***************************************************************************/

void lcdfb_cfb_imageblit(struct fb_info *p, const struct fb_image *image)
{
	u32 start_index, bitstart, pitch_index = 0;
	u32 bpl = sizeof(u32), bpp = p->var.bits_per_pixel;
	u32 width = image->width, height = image->height;
	u32 dx = image->dx, dy = image->dy;
	int x2, y2, vxres, vyres;
	u8 *dst1;

	if (p->state != FBINFO_STATE_RUNNING)
		return;

	vxres = p->var.xres_virtual;
	vyres = p->var.yres_virtual;

	if (image->dx > vxres || image->dy > vyres)
		return;

	x2 = image->dx + image->width;
	y2 = image->dy + image->height;
	dx = image->dx > 0 ? image->dx : 0;
	dy = image->dy > 0 ? image->dy : 0;
	x2 = x2 < vxres ? x2 : vxres;
	y2 = y2 < vyres ? y2 : vyres;
	width = x2 - dx;
	height = y2 - dy;

	bitstart = (dy * p->fix.line_length * 8) + (dx * bpp);
	start_index = bitstart & (32 - 1);
	pitch_index = (p->fix.line_length & (bpl - 1)) * 8;

	bitstart /= 8;
	bitstart &= ~(bpl - 1);
	dst1 = (u8 *) videomemory + bitstart;

	if (p->fbops->fb_sync)
		p->fbops->fb_sync(p);

	color_imageblit(image, p, dst1, start_index, pitch_index);

}				/* lcdfb_cfb_imageblit */

/*
 * Hardware accelerated copyarea to override cfb_copyarea.
 */
static void lcdfb_copyarea(struct fb_info *p, const struct fb_copyarea *area)
{
	struct fb_copyarea new_area;
	u32 width, height;

	LCD_DEBUG("lcdfb_copyarea()\n");

	if (area->width == 0 || area->height == 0 ||
	    area->sx > p->var.xres || area->dx > p->var.xres ||
	    area->sy > p->var.yres || area->dy > p->var.yres) {
		pr_warning("lcdfb_copyarea: Invalid parameters "
			   "sx=%u sy=%u dx=%u dy=%u width=%u height=%u\n",
			   area->sx, area->sy, area->dx, area->dy, area->width,
			   area->height);
		return;
	}

	memcpy(&new_area, area, sizeof(new_area));

	/* chop off some out of bound area if there's any */
	width = area->width;
	height = area->height;
	if (area->sx + width > p->var.xres) {
		width = p->var.xres - area->sx;
	}
	if (area->dx + width > p->var.xres) {
		u32 new_width = p->var.xres - area->dx;
		width = width < new_width ? width : new_width;
	}
	if (area->sy + height > p->var.yres) {
		height = p->var.yres - area->sy;
	}
	if (area->dy + height > p->var.yres) {
		u32 new_height = p->var.yres - area->dy;
		height = height < new_height ? height : new_height;
	}
	new_area.width = width;
	new_area.height = height;

	/* update the virtual frame buffer using the Linux built-in function */
	cfb_copyarea(p, &new_area);

#ifdef CONFIG_FB_VC_ACCELE
	/* update the real frame buffers */
	lcd_copyarea(&new_area);
#endif
}

/*
 * Hardware accelerated fillrect to override cfb_fillrect.
 */
static void lcdfb_fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
	struct fb_fillrect new_rect;
	LCD_FillRectColor_t rect_c;
	u32 width, height;

	LCD_DEBUG("lcdfb_fillrect()\n");

	if (rect->width == 0 || rect->height == 0 ||
	    rect->dx > info->var.xres || rect->dy > info->var.xres ||
	    rect->color >= 255 || rect->rop != ROP_COPY) {
		pr_warning("lcdfb_fillrect: Invalid parameters\n"
			   "width=%u height=%u dx=%u dy=%u color=%u rop=%u\n",
			   rect->width, rect->height, rect->dx, rect->dy,
			   rect->color, rect->rop);
		return;
	}

	memcpy(&new_rect, rect, sizeof(new_rect));

	/* chop off some out of bound area if there's any */
	width = rect->width;
	height = rect->height;
	if (rect->dx + width > info->var.xres) {
		width = info->var.xres - rect->dx;
	}
	if (rect->dy + height > info->var.yres) {
		height = info->var.yres - rect->dy;
	}
	new_rect.width = width;
	new_rect.height = height;

	/* update the virtual frame buffer using the Linux built-in function */
	cfb_fillrect(info, &new_rect);

	/* construct the rect_c structure */
	rect_c.dx = new_rect.dx;
	rect_c.dy = new_rect.dy;
	rect_c.width = new_rect.width;
	rect_c.height = new_rect.height;

	if (info->fix.visual == FB_VISUAL_TRUECOLOR ||
	    info->fix.visual == FB_VISUAL_DIRECTCOLOR) {
		/* get color from the palette array */
		rect_c.rawColor =
		    ((u32 *) info->pseudo_palette)[new_rect.color];
	} else {
		/* use color directly */
		rect_c.rawColor = new_rect.color;
	}

#ifdef CONFIG_FB_VC_ACCELE
	/* update the real frame buffers */
	lcd_fillrect(&rect_c);
#endif
}

/*
 * Wrapper of lcdfb_fillrect that allows the user to use the raw color
 * code directly, since lcdfb_fillrect uses the palette interface in
 * true/direct color mode
 */
static void lcdfb_fillrect_color(struct fb_info *info,
				 LCD_FillRectColor_t * rect_c)
{
	u32 red, green, blue, transp;
	u32 bpp = info->var.bits_per_pixel;
	struct fb_fillrect rect;

	LCD_DEBUG("lcdfb_fillrect_color()\n");

	if (rect_c->width == 0 || rect_c->height == 0 ||
	    rect_c->dx > info->var.xres || rect_c->dy > info->var.xres) {
		pr_err("error lcdfb_fillrect_color: Invalid parameters\n"
		       "width=%u height=%u dx=%u dy=%u rawColor=0x%x\n",
		       rect_c->width, rect_c->height, rect_c->dx, rect_c->dy,
		       rect_c->rawColor);
		return;
	}

	if (info->fix.visual == FB_VISUAL_TRUECOLOR ||
	    info->fix.visual == FB_VISUAL_DIRECTCOLOR) {
		/* extract color information */
		red = ExtractField(rect_c->rawColor, &info->var.red, bpp);
		green = ExtractField(rect_c->rawColor, &info->var.green, bpp);
		blue = ExtractField(rect_c->rawColor, &info->var.blue, bpp);
		transp = ExtractField(rect_c->rawColor, &info->var.transp, bpp);

		/*
		 * Update color information stored in the palette, use the reserved
		 * array index of 15
		 */
		lcdfb_setcolreg(255, red, green, blue, transp, info);

		/* fillrect */
		rect.dx = rect_c->dx;
		rect.dy = rect_c->dy;
		rect.width = rect_c->width;
		rect.height = rect_c->height;
		rect.color = 255;
		rect.rop = ROP_COPY;	/* support copy only */
		cfb_fillrect(info, &rect);
	} else {		/* use rawColor directly for other color modes */

		/* fillrect */
		rect.dx = rect_c->dx;
		rect.dy = rect_c->dy;
		rect.width = rect_c->width;
		rect.height = rect_c->height;
		rect.color = rect_c->rawColor;
		rect.rop = ROP_COPY;	/* support copy only */
		cfb_fillrect(info, &rect);
	}
}
#endif /* CONFIG_FB_VC_ACCELE */

/****************************************************************************
*
*  lcdfb early suspend operations
*
***************************************************************************/
#ifdef CONFIG_HAS_EARLYSUSPEND
#undef BCM_LCD_EARLYSUSPEND_DBG
/* #define BCM_LCD_EARLYSUSPEND_DBG */

static void lcdfb_early_suspend(struct early_suspend *h)
{
#ifdef BCM_LCD_EARLYSUSPEND_DBG
	pr_info("[lcdfb__suspend]+++ level=%d\n", h->level);
#endif
      if (current_intensity==0)
      {
	if (h->level == EARLY_SUSPEND_LEVEL_STOP_DRAWING)
	{
            //printk("[LCD] lcdfb_early_suspend==>EARLY_SUSPEND_LEVEL_STOP_DRAWING\n");
		gLcdfbEarlySuspendStopDraw = 1;
	}
	else if (h->level == EARLY_SUSPEND_LEVEL_DISABLE_FB) {
		int i;
		struct fb_info *fbinfo;
        
		for (i = 0; i < FB_MAX; i++) {
			fbinfo = registered_fb[i];
			if (!fbinfo)
				break;
			/* set fb state to SUSPEND */
			acquire_console_sem();
			fb_set_suspend(fbinfo, FBINFO_STATE_SUSPENDED);
			release_console_sem();
               //printk("[LCD] lcdfb_early_suspend==> EARLY_SUSPEND_LEVEL_DISABLE_FB\n");
		}
	}
}
}

static void lcdfb_late_resume(struct early_suspend *h)
{
#ifdef BCM_LCD_EARLYSUSPEND_DBG
	pr_info("[lcdfb__resume]+++ level=%d\n", h->level);
#endif

	if (h->level == EARLY_SUSPEND_LEVEL_STOP_DRAWING)
	{
             //printk("[LCD] lcdfb_late_resume==> EARLY_SUSPEND_LEVEL_STOP_DRAWING\n");
               gLcdfbEarlySuspendStopDraw = 0;
	}
	else if (h->level == EARLY_SUSPEND_LEVEL_DISABLE_FB) {
		int i;
		struct fb_info *fbinfo;
		for (i = 0; i < FB_MAX; i++) {
			fbinfo = registered_fb[i];
			if (!fbinfo)
				break;
			/* set fb state to SUSPEND */
			acquire_console_sem();
			fb_set_suspend(fbinfo, FBINFO_STATE_RUNNING);
			release_console_sem();
                 //printk("[LCD] lcdfb_late_resume==> EARLY_SUSPEND_LEVEL_DISABLE_FB\n");
		}
	}
}

static struct early_suspend lcdfb_early_suspend_stopdraw_desc = {
	.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING,
	.suspend = lcdfb_early_suspend,
	.resume = lcdfb_late_resume,
};

static struct early_suspend lcdfb_early_suspend_disablefb_desc = {
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = lcdfb_early_suspend,
	.resume = lcdfb_late_resume,
};
#endif

/****************************************************************************
*
*  lcdfb_setup
*
*   Parse out the options at initialization time.
*
***************************************************************************/

int __init lcdfb_setup(char *options)
{
	char *this_opt;

	lcdfb_enable = 1;

	if (!options || !*options)
		return 1;

	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt)
			continue;
		if (!strncmp(this_opt, "disable", 7))
			lcdfb_enable = 0;
	}
	return 1;
}				/* lcdfb_setup */

/****************************************************************************
*
*  lcdfb_probe
*
*   When the driver is registered, this routine will be automatically
*   envoked.  The man job here is to either allocate a framebuffer or
*   to map us to the LCD driver's framebuffer depending of whether the
*   LCD driver has been built or not.
*
***************************************************************************/

static int __init lcdfb_probe(struct platform_device *pdev)
{
	struct fb_info *info;
	int retval = -ENOMEM;

	videomemory = (void *)lcd_get_framebuffer_addr(&videomemorysize,
						       &physmemory);

	info = framebuffer_alloc(sizeof(u32) * 256, &pdev->dev);
	if (!info)
		goto err;

	info->screen_base = (char __iomem *)videomemory;	/* virtual address */
	info->fbops = &lcdfb_ops;

	retval = fb_find_mode(&info->var, info, NULL, NULL, 0, NULL, 8);

	if (!retval || (retval == 4))
		info->var = lcdfb_default;

	info->fix = lcdfb_fix;
	info->fix.smem_start = (unsigned long)physmemory;
	info->fix.smem_len = videomemorysize;
	info->pseudo_palette = gLcdPseudoPalette;
	info->par = NULL;
#ifdef CONFIG_FB_VC_ACCELE
	info->flags = FBINFO_FLAG_DEFAULT
	    | FBINFO_HWACCEL_COPYAREA
	    | FBINFO_HWACCEL_FILLRECT | FBINFO_HWACCEL_IMAGEBLIT;
#else
	info->flags = FBINFO_FLAG_DEFAULT | FBINFO_HWACCEL_IMAGEBLIT;
#endif

	retval = fb_alloc_cmap(&info->cmap, 256, 0);
	if (retval < 0)
		goto err1;

	retval = register_framebuffer(info);
	if (retval < 0)
		goto err2;
	platform_set_drvdata(pdev, info);

	pr_info("%s", gBanner);

#ifdef CONFIG_LOGO
	/*  Display the default logo/splash screen. */
	fb_prepare_logo(info, 0);
	fb_show_logo(info, 0);

	pr_info("Logo is now in framebuffer\n");

	{
		LCD_DirtyRows_t dirtyRows;

		dirtyRows.top = 0;
		dirtyRows.bottom = info->var.yres - 1;
		pr_info("Abt to call LCD dirty rows\n");
		lcd_dirty_rows(&dirtyRows);
	}

#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef BCM_LCD_EARLYSUSPEND_DBG
	pr_info("lcdfb register early suspends\n");
#endif
	register_early_suspend(&lcdfb_early_suspend_stopdraw_desc);
	register_early_suspend(&lcdfb_early_suspend_disablefb_desc);
#endif

	return 0;
err2:
	fb_dealloc_cmap(&info->cmap);
err1:
	framebuffer_release(info);
err:
	return retval;

}				/* lcdfb_probe */

/****************************************************************************
*
*  lcdfb_remove
*
*   Called to removed this device.
*
***************************************************************************/

static int lcdfb_remove(struct platform_device *device)
{
	struct fb_info *info = platform_get_drvdata(device);

#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef BCM_LCD_EARLYSUSPEND_DBG
	pr_info("lcdfb unregister early suspends\n");
#endif
	unregister_early_suspend(&lcdfb_early_suspend_stopdraw_desc);
	unregister_early_suspend(&lcdfb_early_suspend_disablefb_desc);
#endif

	if (info) {
		/* deinit gLcdRefreshWaitQueue here? */
		unregister_framebuffer(info);
		framebuffer_release(info);
	}
	return 0;

}				/* lcdfb_remove */

/*  These are the driver routines we have implemeneted: */
static struct platform_driver lcdfb_driver = {
	.probe = lcdfb_probe,
	.remove = lcdfb_remove,
	.driver = {
		   .name = MODULE_NAME,
		   .owner = THIS_MODULE,
		   },
};

/****************************************************************************
*
*  Module Initialization
*
***************************************************************************/

int __init lcdfb_init(void)
{
	LCD_Info_t lcdInfo;
	int ret = 0;

#ifndef MODULE
	char *option = NULL;

	if (fb_get_options(LCDFB_NAME, &option))
		return -ENODEV;
	lcdfb_setup(option);
#endif
	pr_info("%s\n", __FUNCTION__);

	/* Get LCD width and height. */
	lcd_get_info(&lcdInfo);

	if ((lcdInfo.bitsPerPixel != 16) && (lcdInfo.bitsPerPixel != 32)) {
		/* This driver only supports 16 bits-per-pixel (565) and */
		/* 32 bit RGBA frame-buffers. */

		pr_err("Unsupported pixel depth: %d bpp\n",
		       lcdInfo.bitsPerPixel);

		return -EINVAL;
	}

	gLcdDisplayWidth = lcdInfo.width;
	gLcdDisplayHeight = lcdInfo.height;
	gLcdBitsPerPixel = lcdInfo.bitsPerPixel;

	pr_info("LCD %d x %d, %d bpp\n", gLcdDisplayWidth,
		gLcdDisplayHeight, gLcdBitsPerPixel);

	gLcdBytesPerPixel = gLcdBitsPerPixel >> 3;
	gLcdFrameBufferSizeBytes =
	    gLcdDisplayWidth * gLcdDisplayHeight * gLcdBytesPerPixel;

	lcdfb_default.xres = gLcdDisplayWidth;
	lcdfb_default.yres = gLcdDisplayHeight;
	lcdfb_default.xres_virtual = gLcdDisplayWidth;
	/* lcdfb_default.yres_virtual    =  gLcdDisplayHeight; */
	/* DBFrame : */
	lcdfb_default.yres_virtual = gLcdDisplayHeight * 2;
	lcdfb_default.bits_per_pixel = gLcdBitsPerPixel;
	if ((lcdInfo.physical_width > 0) && (lcdInfo.physical_height > 0)) {
		lcdfb_default.width = lcdInfo.physical_width;
		lcdfb_default.height = lcdInfo.physical_height;
	}

	if (lcdInfo.bitsPerPixel == 32) {
		lcdfb_default.red.offset = 16;
		lcdfb_default.red.length = 8;

		lcdfb_default.green.offset = 8;
		lcdfb_default.green.length = 8;

		lcdfb_default.blue.offset = 0;
		lcdfb_default.blue.length = 8;

		lcdfb_default.transp.offset = 24;
		lcdfb_default.transp.length = 8;
	}

	lcdfb_fix.line_length = gLcdDisplayWidth * gLcdBytesPerPixel;
	lcdfb_fix.smem_len = gLcdFrameBufferSizeBytes;

	videomemorysize = gLcdFrameBufferSizeBytes;

	if (!lcdfb_enable)
		return -ENXIO;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 21)
	gSysCtlHeader = register_sysctl_table(gSysCtl, 0);
	if (gSysCtlHeader != NULL) {
		gSysCtlHeader->ctl_table->child->de->owner = THIS_MODULE;
	}
#else
	pr_info("Proc ADDRs gSysCtl 0x%x gsysctlfb0 0x%x gsysctlfb1 0x%x\n",
		(unsigned int)gSysCtl, (unsigned int)&gSysCtlFb[0],
		(unsigned int)&gSysCtlFb[1]);
	gSysCtlHeader = register_sysctl_table(gSysCtl);
#endif

	ret = platform_driver_register(&lcdfb_driver);

#ifndef CONFIG_BCM_LCD_SKIP_INIT
	if (!ret) {
		ret = lcdfb_init_lcd();
		if (ret)
			platform_driver_unregister(&lcdfb_driver);
	}
#endif
	return ret;
}

module_init(lcdfb_init);

#if defined(MODULE)

static void __exit lcdfb_exit(void)
{
	if (gSysCtlHeader != NULL) {
		unregister_sysctl_table(gSysCtlHeader);
	}
	platform_driver_unregister(&lcdfb_driver);
}

module_exit(lcdfb_exit);

#endif

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Loadable FrameBuffer Driver for Broadcom");
MODULE_LICENSE("GPL");
