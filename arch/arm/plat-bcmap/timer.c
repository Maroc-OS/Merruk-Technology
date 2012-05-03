/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/timer.c
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
 * @file
 * @brief Broadcom specific timer implementation

 * Based on realview platforms
 * @defgroup TimerAPIGroup Timer API's
   @brief Timer API's
 */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/clockchips.h>
#include <linux/jiffies.h>

#include <linux/broadcom/gpt.h>
#include <linux/module.h>
#include <plat/timer.h>

#define SLPTMR_SMTDCLR  0x00    /**< Sleep Mode Timer Down Counter Load Register */
#define SLPTMR_SMTDCR   0x04    /**< Sleep Mode Timer Down Counter Register */
#define SLPTMR_SMTCR    0x08    /**< Sleep Mode Timer Control Register */
#define SLPTMR_SMTCMPR  0x0c    /**< Sleep Mode Timer Compare Register */
#define SLPTMR_SMTSTR   0x10    /**< Sleep Mode Timer System Timer Register */

#define SLPTMR_CLKS             (1 << 1)        /**<Clock Select */
#define SLPTMR_TMREN            (1 << 0)        /**<Timer Enable */
#define SLPTMR_CE_CTRL          (SLPTMR_TMREN)

#define SLPTMR_VALUE(x)         (0x100 * (x) + SLPTMR_SMTSTR)

#define GPT_MAX_COUNT	0xffffffff

static struct gpt_cfg gc_ce;
static struct timer_config config;
bool clkevt_mode_oneshot;

static int bcm_gpt_interrupt(void *dev_id);

static void bcm_gpt_clkevt_mode(enum clock_event_mode mode,
				struct clock_event_device *dev)
{
	unsigned long flags;

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		local_irq_save(flags);
		gc_ce.gmode = GPT_MODE_PERIODIC;
		local_irq_restore(flags);
		gpt_config(config.ce_index, &gc_ce,
				bcm_gpt_interrupt, NULL);
		gpt_start(config.ce_index, -1);
		break;
	case CLOCK_EVT_MODE_SHUTDOWN:
		gpt_stop(config.ce_index);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		local_irq_save(flags);
		gc_ce.gmode = GPT_MODE_ONESHOT;
		local_irq_restore(flags);
		gpt_config(config.ce_index, &gc_ce,
				bcm_gpt_interrupt, NULL);
		break;
	case CLOCK_EVT_MODE_RESUME:
	case CLOCK_EVT_MODE_UNUSED:
		break;
	}
}

static int bcm_gpt_set_nextevt(unsigned long evt,
			       struct clock_event_device *unused)
{
	gpt_config(config.ce_index, &gc_ce,bcm_gpt_interrupt, NULL);
	gpt_start(config.ce_index, evt);
	return 0;
}

static uint32_t setup_slptmr(uint32_t reload, uint32_t control)
{
	writel(0, config.ce_base + SLPTMR_SMTCR);	/* Reset the control register */

	writel(0, config.ce_base + SLPTMR_SMTSTR);	/* System Clock Register */
	writel(reload, config.ce_base + SLPTMR_SMTCMPR);	/* Compare Register */

	writel(control, config.ce_base + SLPTMR_SMTCR);	/* Enable the timer via Control Register */
	return 0;
}

static void bcm_slptmr_clkevt_mode(enum clock_event_mode mode,
				   struct clock_event_device *dev)
{
	unsigned long flags;

	switch (mode) {
	case CLOCK_EVT_MODE_SHUTDOWN:
		local_irq_save(flags);
		writel(0, config.ce_base + SLPTMR_SMTCR);
		local_irq_restore(flags);
		break;
	case CLOCK_EVT_MODE_PERIODIC:
		clkevt_mode_oneshot = false;
		writel((readl(config.ce_base + SLPTMR_SMTCR) | SLPTMR_TMREN),
		       config.ce_base + SLPTMR_SMTCR);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		local_irq_save(flags);
		clkevt_mode_oneshot = true;
		local_irq_restore(flags);
		break;
	case CLOCK_EVT_MODE_RESUME:
	case CLOCK_EVT_MODE_UNUSED:
		break;
	}
}

static int bcm_slptmr_set_nextevt(unsigned long evt,
				  struct clock_event_device *unused)
{
	setup_slptmr(evt, SLPTMR_CE_CTRL);
	return 0;
}

static struct clock_event_device bcm_clkevt = {
	.name = "bcm_clock_event",
	.shift = 32,
	.rating = 200,
};

static void __init bcm_clockevent_init(unsigned int rate)
{
	bcm_clkevt.mult = div_sc(rate, NSEC_PER_SEC, bcm_clkevt.shift);
	bcm_clkevt.max_delta_ns = clockevent_delta2ns(0xfffffffe, &bcm_clkevt);
	bcm_clkevt.min_delta_ns = clockevent_delta2ns(0, &bcm_clkevt);
	bcm_clkevt.cpumask = cpumask_of(0);
	bcm_clkevt.features = CLOCK_EVT_FEAT_ONESHOT;

	if (config.ce_module == TIMER_MODULE_GPT) {
		bcm_clkevt.set_mode = bcm_gpt_clkevt_mode;
		bcm_clkevt.set_next_event = bcm_gpt_set_nextevt;
		bcm_clkevt.features |= CLOCK_EVT_FEAT_PERIODIC;
	} else if (config.ce_module == TIMER_MODULE_SLEEP_TIMER) {
		bcm_clkevt.set_mode = bcm_slptmr_clkevt_mode;
		bcm_clkevt.set_next_event = bcm_slptmr_set_nextevt;
		writel(1, config.ce_base + SLPTMR_SMTDCLR);	/* Setup the Load Register */
	}

	clockevents_register_device(&bcm_clkevt);
	return;
}

/*
 * IRQ handler for the GP timer
 */
static int bcm_gpt_interrupt(void *dev_id)
{
	struct clock_event_device *evt = &bcm_clkevt;

	evt->event_handler(evt);

	return 0;
}

/*
 * IRQ handler for the Sleep timer
 */
static irqreturn_t bcm_slptmr_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &bcm_clkevt;
	if (clkevt_mode_oneshot) {
		writel((readl(config.ce_base + SLPTMR_SMTCR) & ~SLPTMR_TMREN),
		       config.ce_base + SLPTMR_SMTCR);
	} else {
		writel((readl(config.ce_base + SLPTMR_SMTSTR) - readl(config.ce_base + SLPTMR_SMTCMPR)), config.ce_base + SLPTMR_SMTSTR);	/* System Clock Register */
		writel((readl(config.ce_base + SLPTMR_SMTCR) | SLPTMR_TMREN),
		       config.ce_base + SLPTMR_SMTCR);
	}
	evt->event_handler(evt);

	return IRQ_HANDLED;
}

/*
 * Set up timer interrupt, and return the current time in seconds.
 */
static struct irqaction bcm_sleep_timer_irq = {
	.name = "BCM Timer Tick",
	.flags = IRQF_DISABLED | IRQF_TIMER,
	.handler = bcm_slptmr_interrupt,
};

/*
 * clocksource: the GPT device is a decrementing counters, so we negate
 * the value being read.
 */
static cycle_t bcm_read_timer(struct clocksource *cs)
{
	return GPT_MAX_COUNT - gpt_read(config.cs_index);
}

static struct clocksource bcm_clksrc = {
	.name = "bcm_clock_source_gpt5",
	.rating = 200,
	.read = bcm_read_timer,
	.mask = CLOCKSOURCE_MASK(32),
	.shift = 16,
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __init bcm_clocksource_init(unsigned int rate)
{
	bcm_clksrc.mult = clocksource_hz2mult(rate, bcm_clksrc.shift);
	clocksource_register(&bcm_clksrc);
	return;
}

/**
 * Set up the clock source and clock events devices
	@param bcm_config timer configuration
 */
void __init bcm_timer_init(struct timer_config *bcm_config)
{
	uint32_t reload_ce = (CLOCK_TICK_RATE + HZ / 2) / HZ;
	struct gpt_cfg gc_cs;

	config = *bcm_config;
	if (config.ce_module == TIMER_MODULE_GPT) {
		if (gpt_request(config.ce_index) == GPT_ERR_INVALID_INDEX) {
			pr_err("Error configuring CE timer exit\n");
			return;
		}

		gc_ce.gctrl = HW_PEDEMODE_DIS;
		gc_ce.gclk = CLOCK_TICK_RATE;
		gc_ce.gmode = GPT_MODE_PERIODIC;
		gpt_config(config.ce_index, &gc_ce,bcm_gpt_interrupt, NULL);
		gpt_start(config.ce_index, reload_ce);
	} else if (config.ce_module == TIMER_MODULE_SLEEP_TIMER) {
		setup_slptmr(reload_ce, SLPTMR_CE_CTRL);
		setup_irq(config.irq, &bcm_sleep_timer_irq);
	}

	if (config.cs_module == TIMER_MODULE_GPT) {
		if (gpt_request(config.cs_index) == GPT_ERR_INVALID_INDEX) {
			pr_err("Error configuring CE timer exit\n");
			return;
		}

		gc_cs.gctrl = HW_SLPMODE_DIS | SW_SLPMODE;
		gc_cs.gclk = CLOCK_TICK_RATE;
		gc_cs.gmode = GPT_MODE_PERIODIC;
		gpt_config(config.cs_index, &gc_cs,
				NULL, NULL);
		gpt_start(config.cs_index, GPT_MAX_COUNT);
	}

	bcm_clocksource_init(CLOCK_TICK_RATE);
	bcm_clockevent_init(CLOCK_TICK_RATE);

	return;
}

/**
	returns the timer tick count
*/
unsigned int timer_get_tick_count(void)
{
	struct clocksource *cs = 0;
	return bcm_read_timer(cs);
}

/** @} */

EXPORT_SYMBOL(timer_get_tick_count);

/** @addtogroup TimerAPIGroup
	@{
*/

/**
	returns the timer tick count from the CP
*/
unsigned long timer_get_cp_tick_count(void)
{
	if (!config.cp_cs_base) {
		pr_info("CP CS colock source is not specified\n");
		return 0;
	}
	return readl(config.cp_cs_base + SLPTMR_VALUE(config.cp_cs_index)); 
}

/** @} */

EXPORT_SYMBOL(timer_get_cp_tick_count);

/** @addtogroup TimerAPIGroup
	@{
*/

/**
 * Returns current time from boot in nsecs. It's OK for this to wrap
 * around for now, as it's just a relative time stamp.
 */
unsigned long long sched_clock(void)
{
	if (config.cs_module == TIMER_MODULE_NONE)
		return (unsigned long long)(jiffies - INITIAL_JIFFIES)
	                                     * (NSEC_PER_SEC / HZ);
	else
		return clocksource_cyc2ns(bcm_read_timer(NULL),
				bcm_clksrc.mult, bcm_clksrc.shift);
}

/** @} */
