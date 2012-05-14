/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/gpt.c
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

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/io.h>

#include <mach/gpt.h>
#include <linux/broadcom/gpt.h>

#define MHZ(x)	(x * 1024 * 1024)

/*
 * The GPT device hosts six different counters, with 6 set of
 * registers. These are register names.
 */

#define GPT_ISR         0x00    /**< Interrupt Stuff Register */

/* per-timer registers take 0..5 as argument */
#define GPT_CSR(x)      (0x10 * (x) + 0x04)     /**< Control & Status */
#define GPT_RELOAD(x)   (0x10 * (x) + 0x08)     /**< Reload value */
#define GPT_VALUE(x)    (0x10 * (x) + 0x0c)     /**< Current value */

/* bits for the control register */
#define GPT_CSR_EN              (1 << 31)       /**< Timer enable */
#define GPT_CSR_CLKSEL          (1 << 30)       /**< Timer Clock Select */
#define GPT_CSR_SW_SLPMODE      (1 << 29)       /**< Run timer in sleep mode */
#define GPT_CSR_HW_SLPMODE_DIS  (1 << 28)       /**< Disable hw sleep signal */
#define GPT_CSR_INT_EN          (1 << 27)       /**< Enable Interrupt */
#define GPT_CSR_INT2_ASSIGN     (1 << 26)       /**< Assign Intr of this GPT */
#define GPT_CSR_CLKSEL1         (1 << 25)       /**< Select Tick Clock */
#define GPT_CSR_TIMER_PWRON     (1 << 24)       /**< Power on */
#define GPT_CSR_SW_PEDEMODE     (1 << 23)       /**< Pedestal mode */
#define GPT_CSR_HW_PEDEMODE_DIS (1 << 22)       /**< Disable Pedestal mode */
#define GPT_CSR_PEDESTAL_STATE  (1 << 9)        /**< Debug purpose */
#define GPT_CSR_SLEEP_STATE     (1 << 8)        /**<Debug purpose */
#define GPT_CSR_INT_FLAG        (1 << 0)        /**<Terminal Count Status */

struct gpt_desc_config {
	void *arg;
	enum gpt_mode gmode;
	gpt_callback gcallback;
	unsigned flag;
#define GPT_FREE	0x01
#define GPT_ASSIGNED	0x02
#define GPT_CONFIGURED	0x04
#define GPT_RUNNING	0x08
#define GPT_RESERVED	0x10
};

struct gpt_config {
	struct gpt_base_config base_config;
	struct gpt_desc_config gpt_desc[BCM_NUM_GPT];
};

static DEFINE_SPINLOCK(gpt_lock);
static struct gpt_config gpt;

static irqreturn_t gpt_interrupt(int irq, void *dev_id)
{
	unsigned int gpt_isr;
	struct gpt_config *gpt_ptr = (struct gpt_config *) dev_id;
	int i;

	gpt_isr = readl(gpt_ptr->base_config.base_addr + GPT_ISR);

	for (i = 0 ; i < BCM_NUM_GPT; ++i) {
		if ((gpt_isr & (1 << i))) {
			writel(readl(gpt_ptr->base_config.base_addr + GPT_CSR(i)) 
				| GPT_CSR_INT_FLAG, 
				gpt_ptr->base_config.base_addr + GPT_CSR(i));

			if (gpt_ptr->gpt_desc[i].gmode == GPT_MODE_ONESHOT) {
				writel(readl(gpt_ptr->base_config.base_addr + GPT_CSR(i)) 
					& ~GPT_CSR_EN , 
					gpt_ptr->base_config.base_addr + GPT_CSR(i));
			}

			gpt_ptr->gpt_desc[i].gcallback(gpt_ptr->gpt_desc[i].arg);
		}
	}
	
	return IRQ_HANDLED;
}

/*
 * This function is used to configure various GPT parameters like
 * 	    Ctrl : SLPMODE, PEDMODE, 
 *	    Mode : Oneshot/Periodic
 *	     Clk : Frequency in at which timer need to run (26Mhz, 1Mhz, 32Khz)
 * 	Callback : Interrupts are enabled for the GPT if callback is provided.
 * 	     Arg : Argument to receive when callback is called.
 *
 * This API should be called after requesting a GPT and could be called any number 
 * of times before calling gpt_free.
 *
 * Return:
 * 	GPT_SUCCESS	       : On success
 *	GPT_ERR_INVALID_INDEX  : If the gpt index provided is invalid
 *	GPT_ERR_NOT_CONFIGURED : If the GPT is not requested before
 */
int gpt_config(int index, struct gpt_cfg *gc,
		gpt_callback gcallback, void *arg)
{
	int ret = GPT_SUCCESS;
	uint32_t ctrl_reg;
	unsigned long flags;

	spin_lock_irqsave(&gpt_lock, flags);

	if ((index < 0) || (index >= BCM_NUM_GPT)) {
		pr_err("Invalid index %d\n", index);
		ret = GPT_ERR_INVALID_INDEX;
		goto err;
	}

	if (!(gpt.gpt_desc[index].flag & GPT_ASSIGNED)) {
		pr_err("Trying to configure the gpt before request\n");
		ret = GPT_ERR_NOT_CONFIGURED;
		goto err;
	}

	ctrl_reg = readl(gpt.base_config.base_addr + GPT_CSR(index));
	ctrl_reg |= GPT_CSR_TIMER_PWRON;

	if (gc->gclk >= MHZ(26)) {
		ctrl_reg |= GPT_CSR_CLKSEL;
		ctrl_reg &= ~GPT_CSR_CLKSEL1; 
	} else if (gc->gclk >= MHZ(1)) {
		ctrl_reg &= ~GPT_CSR_CLKSEL;
		ctrl_reg |= GPT_CSR_CLKSEL1;
	} else {
		ctrl_reg &= ~(GPT_CSR_CLKSEL1 | GPT_CSR_CLKSEL);
	}

	ctrl_reg |= gc->gctrl;

	if ((gpt.gpt_desc[index].gcallback = gcallback) != NULL)
		ctrl_reg |= GPT_CSR_INT_EN | GPT_CSR_INT_FLAG;
	writel(ctrl_reg, gpt.base_config.base_addr + GPT_CSR(index));

	gpt.gpt_desc[index].arg = arg;
	gpt.gpt_desc[index].gmode = gc->gmode;
	gpt.gpt_desc[index].flag |= GPT_CONFIGURED;
err:
	spin_unlock_irqrestore(&gpt_lock, flags);
	return ret;
}
EXPORT_SYMBOL(gpt_config);

/* 
 * This function is used request/reserve a particular GPT. The input the function is
 * the index of the GPT that needs to be used (this index should be > 0 and < BCM_NUM_GPT).
 * If an index of -1 is supplied then the next free GPT index is assigned and returned.
 *
 * Return value:
 * 	GPT_ERR_INVALID_INDEX: If the index supplied is > Max GPTs.
 * For index non-zero : 
 * 	Index		     : The same index as requested if the requested timer index if free
 * 	GPT_ERR_NOT_FREE     : If the requested GPT is not free 
 * For index zero     :
 * 	Index		     : Index of the first GPT that is free.
 * 	GPT_ERR_NOT_FREE     : If no GPT are free in the system.
 */
int gpt_request(int index)
{
	unsigned long flags;
	int ret = GPT_ERR_NOT_FREE, i;

	spin_lock_irqsave(&gpt_lock, flags);

	if (index > BCM_NUM_GPT) {
		pr_err("Invalid index %d\n", index);
		ret = GPT_ERR_INVALID_INDEX;
		goto err;
	}

	if (index < 0) {
		for (i = 0; i < BCM_NUM_GPT; ++i) {
			if (gpt.gpt_desc[i].flag == GPT_FREE) {
				index = i;
			}
		}
	}

	if (index >= 0 && index < BCM_NUM_GPT) {
		if (gpt.gpt_desc[index].flag == GPT_FREE) {
			gpt.gpt_desc[index].flag = GPT_ASSIGNED;
			ret = index;
		}
	}

err:
	spin_unlock_irqrestore(&gpt_lock, flags);
	return ret;
}
EXPORT_SYMBOL(gpt_request);

/* 
 * This function should be used free the GPT requested (using gpt_request).
 *
 * Return:
 * 	GPT_SUCCESS	       : On success
 *	GPT_ERR_INVALID_INDEX  : If the gpt index provided is invalid
 *	GPT_ERR_NOT_CONFIGURED : If the GPT is not requested before
 */
int gpt_free(int index)
{
	unsigned long flags;
	int ret = GPT_SUCCESS;

	spin_lock_irqsave(&gpt_lock, flags);
	if ((index < 0) || (index >= BCM_NUM_GPT)) {
		pr_err("Invalid index\n");
		ret = GPT_ERR_INVALID_INDEX;
		goto err;
	}

	if (!(gpt.gpt_desc[index].flag & GPT_ASSIGNED)) {
		pr_err("Trying to free the GPT without requesting\n");
		ret = GPT_ERR_NOT_ASSIGNED;
		goto err;
	}

	gpt.gpt_desc[index].flag = GPT_FREE;

err:
	spin_unlock_irqrestore(&gpt_lock, flags);
	return ret;
}
EXPORT_SYMBOL(gpt_free);

/*
 * This function will be used to start the timer with a specific reload value. 
 * Providing a reload value of < 0 would let the timer continue (with the existing
 * reload value)
 *
 * Return:
 * 	GPT_SUCCESS	       : On success
 *	GPT_ERR_INVALID_INDEX  : If the gpt index provided is invalid
 *	GPT_ERR_NOT_CONFIGURED : If the GPT is not requested before
 */
int gpt_start(int index, s64 reload)
{
	unsigned long flags;
	uint32_t ctrl_reg;
	int ret = GPT_SUCCESS;

	spin_lock_irqsave(&gpt_lock, flags);

	if ((index < 0) || (index >= BCM_NUM_GPT)) {
		pr_err("Invalid index\n");
		ret = GPT_ERR_INVALID_INDEX;
		goto err;
	}

	if (!(gpt.gpt_desc[index].flag & GPT_CONFIGURED)) {
		pr_err("Trying to start the GPT without requesting\n");
		ret = GPT_ERR_NOT_CONFIGURED;
		goto err;
	}

	ctrl_reg = readl(gpt.base_config.base_addr + GPT_CSR(index));
	ctrl_reg |= GPT_CSR_TIMER_PWRON;
	writel(ctrl_reg, gpt.base_config.base_addr + GPT_CSR(index));

	if (reload >= 0)
		writel(reload, gpt.base_config.base_addr + GPT_RELOAD(index));

	ctrl_reg |= GPT_CSR_EN;
	writel(ctrl_reg, gpt.base_config.base_addr + GPT_CSR(index));
err:
	spin_unlock_irqrestore(&gpt_lock, flags);

	return ret;

}
EXPORT_SYMBOL(gpt_start);

/* 
 * This function wull stop the timer from further execution. The reload value will
 * not be touched.
 *
 * Return:
 * 	GPT_SUCCESS	       : On success
 *	GPT_ERR_INVALID_INDEX  : If the gpt index provided is invalid
 *	GPT_ERR_NOT_CONFIGURED : If the GPT is not requested before
 */
int gpt_stop(int index)
{
	unsigned long flags;
	uint32_t ctrl_reg;
	int ret = GPT_SUCCESS;

	spin_lock_irqsave(&gpt_lock, flags);

	if ((index < 0) || (index >= BCM_NUM_GPT)) {
		pr_err("Invalid index\n");
		ret = GPT_ERR_INVALID_INDEX;
		goto err;
	}

	if (!(gpt.gpt_desc[index].flag & GPT_CONFIGURED)) {
		pr_err("Trying to stop the GPT without requesting\n");
		ret = GPT_ERR_NOT_CONFIGURED;
		goto err;
	}

	ctrl_reg = readl(gpt.base_config.base_addr + GPT_CSR(index));
	writel(ctrl_reg & ~(GPT_CSR_TIMER_PWRON | GPT_CSR_EN), 
			gpt.base_config.base_addr + GPT_CSR(index));

err:
	spin_unlock_irqrestore(&gpt_lock, flags);
	return ret;
}
EXPORT_SYMBOL(gpt_stop);

/*
 * This function will return the current count on a give GPT.
 *
 * Return:
 * 	Non-zero counter value : This is the current count of the requested timer
 *	Zero  		       : On any error (if an invalid GPT index is specified,
 *				 or If the gpt_read is performed with a proper request.
 */
unsigned int gpt_read(int index)
{
	unsigned long flags;
	uint32_t counter;

	spin_lock_irqsave(&gpt_lock, flags);

	if ((index < 0) || (index >= BCM_NUM_GPT)) {
		pr_err("Invalid index\n");
		counter = 0;
		goto err;
	}

	if (!(gpt.gpt_desc[index].flag & GPT_CONFIGURED)) {
		pr_err("Trying to read the GPT without requesting\n");
		counter = 0;
		goto err;
	}

	counter = readl(gpt.base_config.base_addr + GPT_VALUE(index));

err:
	spin_unlock_irqrestore(&gpt_lock, flags);
	return counter;
}
EXPORT_SYMBOL(gpt_read);

/*
 * Set up timer interrupt, and return the current time in seconds.
 */
static struct irqaction bcm_timer_irq = {
	.name = "BCM Timer Tick",
	.flags = IRQF_DISABLED | IRQF_TIMER | IRQF_TRIGGER_HIGH,
	.handler = gpt_interrupt,
	.dev_id = (void *) &gpt,
};

void __init gpt_init(struct gpt_base_config *base_config)
{
	int i;

	gpt.base_config = *base_config;

	for (i = 0; i < BCM_NUM_GPT; ++i) {
		if ((1 << i) & gpt.base_config.avail_bitmap) {
			writel(0, gpt.base_config.base_addr + GPT_CSR(i));
			writel(0, gpt.base_config.base_addr + GPT_RELOAD(i));
			gpt.gpt_desc[i].flag = GPT_FREE;
		} else {
			gpt.gpt_desc[i].flag = GPT_RESERVED;
		}
	}

	if (gpt.base_config.avail_bitmap) {
		setup_irq(gpt.base_config.irq, &bcm_timer_irq);
	}
}

