/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/clock.c
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
 *   @file   clock.c
 *   @brief  Broadcom platform clock functions.
 *
 ****************************************************************************/
/**
*   @defgroup   ClockAPIGroup   Clock API's
*   @brief      This group defines the clock API's
*
*****************************************************************************/

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/string.h>

#include <mach/hardware.h>
#include <plat/clock.h>

static DEFINE_SPINLOCK(clocks_lock);

/*-------------------------------------------------------------------------
 * Standard clock functions defined in include/linux/clk.h
 *-------------------------------------------------------------------------*/

static void __clk_disable(struct clk *clk)
{
	volatile u32 regVal;

	if (clk == NULL || IS_ERR(clk))
		return;

	if (clk->flags & BCM_CLK_ALWAYS_ENABLED) {
		pr_info("Clock:%u cannot be disabled\n", clk->id);
	} else {
		if (clk->cnt > 0) {
			/* Decrement usage count of this clock. */
			clk->cnt = clk->cnt - 1;

			/* if clk->cnt == 0, then it means all the modules that are using this clock have stopped using it. */
			/* Ideally clk_put needs to be called to completely free/disable the clock. But freeing it here for */
			/* power save in case modules don't call clk_put. */
			if (clk->cnt == 0) {
				/* Disable parent, if any */
				if (clk->parent
				    &&
				    ((clk->
				      parent->flags & BCM_CLK_ALWAYS_ENABLED) ==
				     0))
					__clk_disable(clk->parent);

				if (clk->disable)
					clk->disable(clk);

				/* Generic implementation */
				else {
					if (clk->enable_reg == NULL
					    || clk->enable_bit_mask == 0) {
						pr_info
						    ("Invalid enable_reg / enable_bit_mask values for clk -> %u\n",
						     clk->id);
						return;
					}
					regVal = readl(clk->enable_reg);
					if (clk->flags & BCM_CLK_INVERT_ENABLE) {
						regVal |= clk->enable_bit_mask;
						writel(regVal, clk->enable_reg);
					} else {
						regVal &=
						    ~(clk->enable_bit_mask);
						writel(regVal, clk->enable_reg);
					}

				}
			}
		}
	}
}

static int __clk_enable(struct clk *clk)
{
	int ret = 0;
	volatile u32 regVal;

	if (clk == NULL || IS_ERR(clk))
		return -EINVAL;

	if (clk->flags & BCM_CLK_ALWAYS_ENABLED) {
		pr_info("Clock %u cannot be enabled. it is always enabled\n",
			clk->id);
	} else {
		/* Enable the clock, only if the cnt is 0, which means no one is using it, so mostly it is disabled. */
		if (clk->cnt++ == 0) {
			/* enable parent, if any */
			if (clk->parent
			    && ((clk->parent->flags & BCM_CLK_ALWAYS_ENABLED) ==
				0))
				__clk_enable(clk->parent);

			if (clk->enable)
				ret = clk->enable(clk);
			else {	/* Generic implementation */

				if (clk->enable_reg == NULL
				    || clk->enable_bit_mask == 0)
				{
					pr_info
					    ("Invalid enable_reg / enable_bit_mask values for clk -> %u\n",
					     clk->id);
					return -EINVAL;
				}
				regVal = readl(clk->enable_reg);
				if (clk->flags & BCM_CLK_INVERT_ENABLE) {
					regVal &= ~(clk->enable_bit_mask);
					writel(regVal, clk->enable_reg);

				} else {
					regVal |= clk->enable_bit_mask;
					writel(regVal, clk->enable_reg);

				}
				ret = 0;
			}
		}

	}
	return ret;
}

/**
	@addtogroup ClockAPIGroup
	@{
*/

/**
	Enable the Clock
	@param	clk structure defining the clock the needs to be enabled.
	@return	EINVAL or zero

	This function increments the reference count on the clock and enables the
	clock if not already enabled. The parent clock tree is recursively enabled
 */
int clk_enable(struct clk *clk)
{
	int ret = 0;
	unsigned long flags;

	if (clk == NULL || IS_ERR(clk))
		return -EINVAL;

	spin_lock_irqsave(&clocks_lock, flags);
	ret = __clk_enable(clk);
	spin_unlock_irqrestore(&clocks_lock, flags);

	return ret;
}

/**
	Disable the Clock
	@param	clk structure defining the clock the needs to be disabled.

	This function decrements the reference count on the clock and disables
	the clock when reference count is 0. The parent clock tree is
	recursively disabled
 */
void clk_disable(struct clk *clk)
{
	unsigned long flags;
	if (clk == NULL || IS_ERR(clk))
		return;

	spin_lock_irqsave(&clocks_lock, flags);
	__clk_disable(clk);
	spin_unlock_irqrestore(&clocks_lock, flags);
}

/**
	Get the Clock rate
	@param clk structure defining the clock.
	@return OUL or the clock rate

	Retrieve the current clock rate. If the clock itself does not provide a special
	calculation routine, ask its parent and so on, until one is able to return a valid
	clock rate
 */
unsigned long clk_get_rate(struct clk *clk)
{
	if (clk == NULL || IS_ERR(clk))
		return 0UL;

	if (clk->get_rate)
		return clk->get_rate(clk);

	return clk_get_rate(clk->parent);
}

/**
	Round clock rate
	@param clk structure defining the clock.
	@param rate clock rate
	@return zero if error **TBD**

	Round the requested clock rate to the nearest supported rate that is less than or
	equal to the requested rate.  This is dependent on the clock's current parent.
 */
long clk_round_rate(struct clk *clk, unsigned long rate)
{
	if (clk == NULL || IS_ERR(clk) || !clk->round_rate)
		return 0;

	return clk->round_rate(clk, rate);
}

/**
	Set clock rate
	@param clk structure defining the clock.
	@param rate clock rate
	@return EINVAL if error **TBD**

	Set the clock to the requested clock rate. The rate must match a supported rate
	exactly based on what clk_round_rate returns

 */
int clk_set_rate(struct clk *clk, unsigned long rate)
{
	int ret = -EINVAL;
	unsigned long flags;

	if (clk == NULL || IS_ERR(clk) || clk->set_rate == NULL || rate == 0)
		return ret;

	spin_lock_irqsave(&clocks_lock, flags);
	ret = clk->set_rate(clk, rate);
	if (ret == 0)
		clk->rate = rate;
	spin_unlock_irqrestore(&clocks_lock, flags);

	return ret;
}

/**
	Set clock parent
	@param clk structure defining the clock.
	@param parent structure defining the clock parent
	@return EINVAL if error **TBD**

	Set the clock's parent to another clock source
*/
int clk_set_parent(struct clk *clk, struct clk *parent)
{
	int ret = -EINVAL;
	unsigned long flags;

	if (clk == NULL || IS_ERR(clk) || parent == NULL ||
	    IS_ERR(parent) || clk->set_parent == NULL)
		return ret;

	spin_lock_irqsave(&clocks_lock, flags);
	ret = clk->set_parent(clk, parent);
	if (ret == 0)
		clk->parent = parent;
	spin_unlock_irqrestore(&clocks_lock, flags);

	return ret;
}

/**
	Get clock parent
	@param clk structure defining the clock.
	@return NULL if error **TBD**

	Retrieve the clock's parent clock source
*/
struct clk *clk_get_parent(struct clk *clk)
{
	struct clk *ret = NULL;

	if (clk == NULL || IS_ERR(clk))
		return ret;

	return clk->parent;
}

/** @} */

EXPORT_SYMBOL(clk_get_parent);
EXPORT_SYMBOL(clk_enable);
EXPORT_SYMBOL(clk_disable);
EXPORT_SYMBOL(clk_get_rate);
EXPORT_SYMBOL(clk_round_rate);
EXPORT_SYMBOL(clk_set_rate);
EXPORT_SYMBOL(clk_set_parent);
