/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/i2c/busses/i2c-bcm1161.c
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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/platform_device.h>
#include <plat/bcm_i2c.h>
#include <linux/broadcom/bcm_sysctl.h>
#include <plat/timer.h>
#include <linux/clk.h>

/* ----- global variables --------------------------------------------- */

#define I2C_POLL_COMMAND_DONE 0

/* Debug logging */
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 1

#define DBG_ERROR   0x01
#define DBG_INFO    0x02
#define DBG_TRACE   0x04
#define DBG_TRACE2  0x08
#define DBG_DATA    0x10
#define DBG_DATA2   0x20

#if defined(CONFIG_ARCH_BCM116X) || defined(CONFIG_ARCH_BCMRING)
#define ADD_TURNAROUND_DELAY    1
#else
#define ADD_TURNAROUND_DELAY    0
#endif

#define I2C_ISR_MASK_ALL	(REG_I2C_ISR_SES_DONE | \
				REG_I2C_ISR_I2CERR | \
				REG_I2C_ISR_TXFIFOEMPTY | \
				REG_I2C_ISR_NOACK)

#if ADD_TURNAROUND_DELAY

#define BSC_TIM_P_VAL_BIT_SHIFT 3
#define BSC_MASTER_CLK_FREQ 13000000UL

#endif

#define DBG_DEFAULT_LEVEL  (DBG_ERROR)
static int i2c_debug_level = DBG_DEFAULT_LEVEL;

#if DEBUG
#define I2C_DEBUG(level, fmt, args...) \
			do { if (level & i2c_debug_level) \
				pr_info("%s: " fmt, __FUNCTION__, ##args); } \
			while (0)
#else
#define I2C_DEBUG(level, fmt, args...)
#endif

#define I2CTIM_DIV_812500HZ	0
#define I2CTIM_DIV_1625000HZ	1
#define I2CTIM_DIV_3250000HZ	2
#define I2CTIM_DIV_6500000HZ	3

static unsigned int i2c_errors;
static unsigned int i2c_max_irq_wait;
static unsigned int i2c_max_cmdbusy_cnt;
static unsigned int i2c_max_cmdbusy_clk;

static struct ctl_table gSysCtlLocal[] = {
	{
	 .procname = "level",
	 .data = &i2c_debug_level,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{
	 .procname = "errors",
	 .data = &i2c_errors,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{
	 .procname = "max_irq_wait",
	 .data = &i2c_max_irq_wait,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{
	 .procname = "max_cmdbusy_cnt",
	 .data = &i2c_max_cmdbusy_cnt,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{
	 .procname = "max_cmdbusy_clk",
	 .data = &i2c_max_cmdbusy_clk,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{}
};

static struct ctl_table gSysCtl[] = {
	{
	 .procname = "i2c",
	 .mode = 0555,
	 .child = gSysCtlLocal},
	{}
};

struct bcm1161_i2c {
	struct i2c_adapter adapter;
	u32 interrupt;
	char i2c_name[50];
	struct clk *clk;
#if !I2C_POLL_COMMAND_DONE
	struct completion cmd_complete;
#endif
	/* struct i2c_adapter adap; */
	struct ctl_table_header *sysCtlHeader;	/* sysctl table */
	int irq;
	unsigned long i2c_bus_clk_period;
	i2c_speed_t cur_spd;
	/* For debug */
	int addr;
};

void bcm1161_i2c_set_autosense(void *base, bool on)
{
	u32 tmp;

	tmp = REG_I2C_CLKEN(base);

	if (on) {
		REG_I2C_TOUT(base) |= REG_I2C_TOUT_EN;
		REG_I2C_TOUT(base) |= 0x7F;
#ifdef CONFIG_ARCH_BCM215XX
		REG_I2C_TOUT(base) |= 0x700;
#else
		REG_I2C_TOUTH(base) |= 0xFF;
#endif

		tmp &= ~(REG_I2C_CLKEN_AUTOSENSE_EN);
	} else {
		tmp |= REG_I2C_CLKEN_AUTOSENSE_EN;
		REG_I2C_TOUT(base) &= ~REG_I2C_TOUT_EN;
	}
	REG_I2C_CLKEN(base) = tmp;
}

/* static struct bcm1161_i2c *i2c = NULL; */

#if ADD_TURNAROUND_DELAY

/* Turn-around delay (2 bus clocks time) after write */
/* read needed for BCM2153 BSC */
#define TURN_AROUND_DELAY   (i2c->i2c_bus_clk_period * 2)

/* compute the clock period required for the delay between i2c transactions */
static unsigned long bcm1161_i2c_bus_clk_period(struct bcm1161_i2c *i2c)
{
	u8 tim, div, p;

	tim = REG_I2C_TIM(i2c->adapter.algo_data);
	div = tim & REG_I2C_TIM_DIVMSK;
	p = (tim & REG_I2C_TIM_PMSK) >> BSC_TIM_P_VAL_BIT_SHIFT;

	I2C_DEBUG(DBG_INFO, " tim[%d]\t div[%d]\t p[%d]\n", tim, div, p);

	div = 1 << (4 - div);
	p = 2 * (p + 1) + 1 + 2 * (p + 1) + 2;

	return (div * p * 1000000 + (clk_get_rate(i2c->clk) >> 1) - 1) /
	    BSC_MASTER_CLK_FREQ;
}
#endif

static int bcm1161_check_cmdbusy(void *base)
{
	int i = 0;
#if defined (CONFIG_ARCH_BCM116X)
	timer_tick_count_t clk = timer_get_tick_count();
	timer_tick_count_t clk2;
#endif

	if (REG_I2C_ISR(base) & REG_I2C_ISR_CMD_BUSY) {
		while (REG_I2C_ISR(base) & REG_I2C_ISR_CMD_BUSY) {
			/* wait for I2C CMD not busy */
			if (i > 100000) {
				i2c_errors++;
				REG_I2C_ISR(base) = 0x80;
				return -ETIMEDOUT;
			}
			i++;
		}
	}
#if defined (CONFIG_ARCH_BCM116X)
	clk2 = timer_get_tick_count();
	clk = clk2 - clk;
	if (i > i2c_max_cmdbusy_cnt) {
		i2c_max_cmdbusy_cnt = i;
	}
	if (clk > i2c_max_cmdbusy_clk) {
		i2c_max_cmdbusy_clk = clk;
	}
#endif
	return 0;
}

#if !I2C_POLL_COMMAND_DONE
static irqreturn_t bcm1161_i2c_isr(int irq, void *dev_id)
{
	struct bcm1161_i2c *i2c = dev_id;
	/* pr_info("bcm1161_i2c_isr 0x%08X\n", i2c->adapter.algo_data); */
	if (REG_I2C_ISR(i2c->adapter.algo_data)) {
		/* Read again to allow register to stabilise */
		i2c->interrupt = REG_I2C_ISR(i2c->adapter.algo_data);
		/* clear interrupts */
		REG_I2C_ISR(i2c->adapter.algo_data) = i2c->interrupt;
		complete(&i2c->cmd_complete);
	}
	return IRQ_HANDLED;
}

static int bcm1161_wait_interrupt(struct bcm1161_i2c *i2c)
{
	int res = 0;
#if defined (CONFIG_ARCH_BCM116X)
	timer_tick_count_t clk;

	clk = timer_get_tick_count();
#endif
	res =
	    wait_for_completion_timeout(&i2c->cmd_complete,
					msecs_to_jiffies(5000));
#if defined (CONFIG_ARCH_BCM116X)
	clk = timer_get_tick_count() - clk;
	if (clk > i2c_max_irq_wait) {
		i2c_max_irq_wait = clk;
	}
#endif
	if (res == 0) {
		dev_err(&i2c->adapter.dev, "Timeout on wait\n");
		return -ETIMEDOUT;
	} else {
		res = i2c->interrupt;
	}

	i2c->interrupt = 0;

	return res;
}

#else

static int bcm1161_wait_interrupt(void *base, struct bcm1161 *i2c)
{
	/* wait for I2C Controller Interrupt */
	unsigned int i = 0;
#if defined (CONFIG_ARCH_BCM116X)
	timer_tick_count_t clk;
	timer_tick_count_t clk2;
#endif
	int isr;

#if defined (CONFIG_ARCH_BCM116X)
	clk = timer_get_tick_count();
#endif
	while ((REG_I2C_ISR(base) & I2C_ISR_MASK_ALL) == 0) {
		i++;
		if (i == (unsigned int)(-1)) {
			I2C_DEBUG(DBG_ERROR, "wait timed out, %d\n", clk);
			return -ETIMEDOUT;
		}
	}

#if defined (CONFIG_ARCH_BCM116X)
	clk2 = timer_get_tick_count();
	clk = clk2 - clk;
	if (clk > i2c_max_irq_wait) {
		i2c_max_irq_wait = clk;
	}
#endif
	isr = REG_I2C_ISR(base);
	REG_I2C_ISR(base) = isr;

	return isr;
}
#endif

static void bcm1161_i2c_reset_controller(void *base)
{
#if defined(CONFIG_ARCH_BCM21553_B0) || defined(CONFIG_ARCH_BCM21553_B1)
	int timeout = 100;

	REG_I2C_SFTRST(base) = 0x01;

	while ((REG_I2C_SFTRST(base) != 0x00) && --timeout)
		udelay(10);

	if (!timeout)
		pr_err("Error resetting I2C controller %x\n", base);
#else
	REG_I2C_CS(base) = 0;
	udelay(80);
	REG_I2C_CS(base) = REG_I2C_CS_SCL;
	udelay(80);
	REG_I2C_CS(base) = REG_I2C_CS_SDA | REG_I2C_CS_SCL;
	udelay(80);
	REG_I2C_CS(base) = REG_I2C_CS_EN;
#endif
}

static int bcm1161_wait_sesdone(int *base, int clear_cmd,
				struct bcm1161_i2c *i2c)
{
	int ret;
	u8 isr;
	u8 cs = REG_I2C_CS(base);

#if !I2C_POLL_COMMAND_DONE
	ret = bcm1161_wait_interrupt(i2c);
#else
	ret = bcm1161_wait_interrupt(base, i2c);
#endif
	if (ret < 0) {
		I2C_DEBUG(DBG_ERROR,
			  "wait interrupt timed out %d cs = 0x%02X\n", ret, cs);
		i2c_errors++;
		bcm1161_i2c_reset_controller(base);
		return ret;
	}
	/* Get status from interrupt status register */
	isr = ret;

	/* Clear command */
	cs = REG_I2C_CS(base);
	if (clear_cmd) {
		REG_I2C_CS(base) = (cs & ~REG_I2C_CS_CMDMASK);
	}

	if (isr & REG_I2C_ISR_I2CERR) {
		I2C_DEBUG(DBG_ERROR, "bus error detected, slave addr = 0x%04X\n",
								i2c->addr);
		i2c_errors++;
		return -EIO;
	}

	if (isr & REG_I2C_ISR_NOACK) {
		I2C_DEBUG(DBG_ERROR, "no ack, slave addr = 0x%04X\n",
								i2c->addr);
		i2c_errors++;
	}

	if (!(isr & REG_I2C_ISR_SES_DONE)) {
		I2C_DEBUG(DBG_ERROR, "ses done timedout\n");
		i2c_errors++;
		return -EIO;
	}

	return (cs & REG_I2C_CS_ACK) ? 0 : 1;
}

/*TODO Have separte definitions in i2c-bcm1161.h
 * Clock enabling has to be done in clk manager
 */
void setrate(void *base, struct bcm1161_i2c *i2c, unsigned short spd)
{

	unsigned char I2C_M = 1, I2C_N = 1;
	unsigned char DIV = I2CTIM_DIV_1625000HZ, I2C_P = 2;
	int temp = 0;
#ifdef CONFIG_ARCH_BCM215XX
	unsigned char NO_DIV = 0, PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
	__REG32(CLK_HSBSC_ENABLE) = 1;
	__REG32(CLK_HSBSC2_ENABLE) = 1;
#endif
	if (i2c->cur_spd != spd) {
		switch (spd) {
		case I2C_SPD_32K:
			I2C_M = 0x01;
			I2C_N = 0x01;
			DIV = I2CTIM_DIV_812500HZ;
			I2C_P = 0x04;
			break;
		case I2C_SPD_50K:
			I2C_M = 0x01;
			I2C_N = 0x01;
			DIV = I2CTIM_DIV_812500HZ;
			I2C_P = 0x02;
			break;
		case I2C_SPD_220K:
			I2C_M = 0x07;
			I2C_N = 0x00;
			DIV = I2CTIM_DIV_3250000HZ;
			I2C_P = 0x00;
			break;
		case I2C_SPD_360K:
			I2C_M = 0x02;
			I2C_N = 0x04;
			DIV = I2CTIM_DIV_6500000HZ;
			I2C_P = 0x01;
			break;
		case I2C_SPD_400K:
		case I2C_SPD_MAXIMUM:
			I2C_M = 0x02;
			I2C_N = 0x02;
			DIV = I2CTIM_DIV_6500000HZ;
			I2C_P = 0x01;
			break;
		case I2C_SPD_100K:
		default:
			I2C_M = 0x01;
			I2C_N = 0x01;
			DIV = I2CTIM_DIV_3250000HZ;
			I2C_P = 0x06;
			break;
		}
		i2c->cur_spd = spd;
		temp = (I2C_P << 3) | DIV;
#ifdef CONFIG_ARCH_BCM215XX
		temp |= (NO_DIV << 2) | (PRESCALE << 6);
#endif
		REG_I2C_TIM(base) = temp;

		REG_I2C_CLKEN(base) = (REG_I2C_CLKEN(base) &
				       REG_I2C_CLKEN_AUTOSENSE_EN) |
		    (I2C_M << 4) | (I2C_N << 1) | REG_I2C_CLKEN_CLKEN;
	}
	return 0;

	/* pr_info("setrate rate=%d, m=%d, n=%d, p=%d, div=%d, tim=0x%X,
	   clk_en=0x%X\n", spd, I2C_M, I2C_N, I2C_P, DIV,
	   REG_I2C_TIM(base), REG_I2C_CLKEN(base)); */
}

static int bcm1161_i2c_start(void *base, struct bcm1161_i2c *i2c)
{
	u8 cs;
	int i = 0;
	I2C_DEBUG(DBG_TRACE, "\n");

#if ADD_TURNAROUND_DELAY
	udelay(TURN_AROUND_DELAY);
#endif

	while (((cs = REG_I2C_CS(base)) &
		(REG_I2C_CS_SDA | REG_I2C_CS_SCL)) !=
	       (REG_I2C_CS_SDA | REG_I2C_CS_SCL)) {
		if ((i % 100) == 0) {
			i2c_errors++;
			I2C_DEBUG(DBG_ERROR,
				  "waiting for cs = 0x%2x [i = %d] base = 0x%p\n",
				  cs, i, (unsigned int)base);
			bcm1161_i2c_reset_controller(base);
		}
		i++;
	}

	/* Wait for controller to be done with last command */
	if (bcm1161_check_cmdbusy(base) < 0) {
		I2C_DEBUG(DBG_ERROR, "cmdbusy always high\n");
		return -EIO;
	}

	/* Send normal start condition */
	REG_I2C_CS(base) = REG_I2C_CS_SDA | REG_I2C_CS_SCL |
	    REG_I2C_CS_CMDSTART | REG_I2C_CS_EN;

	/* Wait for command to be done */
	if (bcm1161_wait_sesdone(base, 1, i2c) < 0) {
		I2C_DEBUG(DBG_ERROR, "sesdone timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int bcm1161_i2c_repstart(void *base, struct bcm1161_i2c *i2c)
{
	I2C_DEBUG(DBG_TRACE, "\n");

	/* Wait for controller to be done with last command */
	if (bcm1161_check_cmdbusy(base) < 0) {
		I2C_DEBUG(DBG_ERROR, "cmdbusy always high\n");
		return -EIO;
	}
	/* Send repeated start condition */
	REG_I2C_CS(base) = REG_I2C_CS_SDA | REG_I2C_CS_SCL |
	    REG_I2C_CS_CMDRESTART | REG_I2C_CS_EN;

	/* Wait for command to be done */
	if (bcm1161_wait_sesdone(base, 1, i2c) < 0) {
		I2C_DEBUG(DBG_ERROR, "sesdone timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int bcm1161_i2c_stop(void *base, struct bcm1161_i2c *i2c)
{
	u8 cs = REG_I2C_CS(base);

	I2C_DEBUG(DBG_TRACE, "\n");

	/* Wait for controller to be done with last command */
	if (bcm1161_check_cmdbusy(base) < 0) {
		I2C_DEBUG(DBG_ERROR, "cmdbusy always high\n");
		return -EIO;
	}

	/* Send stop condition */
	REG_I2C_CS(base) = REG_I2C_CS_SDA | REG_I2C_CS_SCL |
	    REG_I2C_CS_CMDSTOP | REG_I2C_CS_EN;

	/* Wait for command to be done */
	if (bcm1161_wait_sesdone(base, 1, i2c) < 0) {
		I2C_DEBUG(DBG_ERROR, "sesdone timed out, cs = 0x%02x\n", cs);
		return -ETIMEDOUT;
	}

	return 0;
}

/* send a byte without start cond., look for arbitration,
   check ackn. from slave */
/* returns:
 * 1 if the device acknowledged
 * 0 if the device did not ack
 * -ETIMEDOUT if an error occurred (while raising the scl line)
 */
static int bcm1161_i2c_outb(void *base, char c, struct bcm1161_i2c *i2c)
{
	int ack;

	I2C_DEBUG(DBG_TRACE, "0x%2x\n", c);

	udelay(5);

	/* Wait for controller to be done with last command */
	if (bcm1161_check_cmdbusy(base) < 0) {
		I2C_DEBUG(DBG_ERROR, "cmdbusy always high\n");
		return -EIO;
	}
	/* Send data */
	REG_I2C_CS(base) = REG_I2C_CS_SDA | REG_I2C_CS_SCL | REG_I2C_CS_EN;
	REG_I2C_DAT(base) = (u8) c;

	/* Wait for command to be done */
	ack = bcm1161_wait_sesdone(base, 0, i2c);
	if (ack < 0) {
		I2C_DEBUG(DBG_ERROR, "sesdone timed out\n");
		return -ETIMEDOUT;
	}

	return ack;
}

static int bcm1161_i2c_inb(void *base, int no_ack, struct bcm1161_i2c *i2c)
{
	u8 cs = REG_I2C_CS(base);
	u8 data;

	I2C_DEBUG(DBG_TRACE, "%d\n", no_ack);

	/* Wait for controller to be done with last command */
	if (bcm1161_check_cmdbusy(base) < 0) {
		I2C_DEBUG(DBG_ERROR, "cmdbusy always high\n");
		return -EIO;
	}
	/* Initiate data read with ACK low */
	REG_I2C_CS(base) =
	    REG_I2C_CS_SDA | REG_I2C_CS_SCL |
	    REG_I2C_CS_CMDREAD | REG_I2C_CS_EN | (no_ack ? REG_I2C_CS_ACK : 0);

	/* Wait for command to be done */
	if (bcm1161_wait_sesdone(base, 1, i2c) < 0) {
		I2C_DEBUG(DBG_ERROR, "sesdone timed out, cs = 0x%02x\n", cs);
		return -ETIMEDOUT;
	}
	/* Read data */
	data = REG_I2C_DAT(base);

	udelay(5);
	return (int)data;
}

/* try_address tries to contact a chip for a number of
 * times before it gives up.
 * return values:
 * 1 chip answered
 * 0 chip did not answer
 * -x transmission error
 */
static int bcm1161_try_address(struct i2c_adapter *i2c_adap,
			       unsigned char addr, int retries,
			       struct bcm1161_i2c *i2c)
{
	int i, ret = -1;

	I2C_DEBUG(DBG_TRACE, "0x%02x, %d\n", addr, retries);

	for (i = 0; i <= retries; i++) {
		ret = bcm1161_i2c_outb(i2c_adap->algo_data, addr, i2c);
		if (ret == 1)
			break;	/* success! */
		bcm1161_i2c_stop(i2c_adap->algo_data, i2c);
		udelay(5);
		if (i == retries)	/* no success */
			break;
		bcm1161_i2c_start(i2c_adap->algo_data, i2c);
		udelay(100);
	}
	udelay(5);
	if (i) {
		I2C_DEBUG(DBG_INFO,
			  "Used %d tries to %s client at 0x%02x : %s\n",
			  i + 1, addr & 1 ? "read" : "write", addr >> 1,
			  ret == 1 ? "success" : ret == 0 ? "no ack" :
			  "failed, timeout?");
	}
	return ret;
}

static int bcm1161_sendbytes(struct i2c_adapter *i2c_adap,
			     struct i2c_msg *msg, struct bcm1161_i2c *i2c)
{
	char c;
	const char *temp = msg->buf;
	int count = msg->len;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	int retval;
	int wrcount = 0;

	while (count > 0) {
		c = *temp;
		I2C_DEBUG(DBG_TRACE2, "writing %2.2X\n", c & 0xff);
		retval = bcm1161_i2c_outb(i2c_adap->algo_data, c, i2c);

		if ((retval > 0) || (nak_ok && (retval == 0))) {
			/* ok or ignored NAK */
			count--;
			temp++;
			wrcount++;
		} else {	/* arbitration or no acknowledge */
			I2C_DEBUG(DBG_ERROR, "error %d/%d.\n", wrcount,
				  msg->len);
			i2c_errors++;
			bcm1161_i2c_stop(i2c_adap->algo_data, i2c);
			return (retval < 0) ? retval : -EFAULT;
			/* got a better one ?? */
		}
	}
	return wrcount;
}

static int bcm1161_readbytes(struct i2c_adapter *i2c_adap,
			     struct i2c_msg *msg, struct bcm1161_i2c *i2c)
{
	int inval;
	int rdcount = 0;	/* counts bytes read */
	char *temp = msg->buf;
	int count = msg->len;

	while (count > 0) {
		inval = bcm1161_i2c_inb(i2c_adap->algo_data,
					(msg->flags & I2C_M_NO_RD_ACK)
					|| (count == 1), i2c);

		if (inval >= 0) {
			I2C_DEBUG(DBG_TRACE2, "reading %2.2X\n", inval & 0xff);
			*temp = inval;
			rdcount++;
		} else {	/* read timed out */
			i2c_errors++;
			I2C_DEBUG(DBG_ERROR, "timed out.\n");
			break;
		}

		temp++;
		count--;
	}
	return rdcount;
}

/* doAddress initiates the transfer by generating the start condition (in
 * try_address) and transmits the address in the necessary format to handle
 * reads, writes as well as 10bit-addresses.
 * returns:
 *  0 everything went okay, the chip ack'ed, or IGNORE_NAK flag was set
 * -x an error occurred (like: -EREMOTEIO if the device did not answer, or
 *  -ETIMEDOUT, for example if the lines are stuck...)
 */
static int bcm1161_doAddress(struct i2c_adapter *i2c_adap,
			     struct i2c_msg *msg, struct bcm1161_i2c *i2c)
{
	unsigned short flags = msg->flags;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;

	unsigned char addr;
	int ret, retries;

	retries = nak_ok ? 0 : i2c_adap->retries;

	if ((flags & I2C_M_TEN)) {
		/* a ten bit address */
		addr = 0xf0 | ((msg->addr >> 7) & 0x03);
		I2C_DEBUG(DBG_TRACE2, "addr: %d\n", addr);
		/* try extended address code... */
		ret = bcm1161_try_address(i2c_adap, addr, retries, i2c);

		if ((ret != 1) && !nak_ok) {
			I2C_DEBUG(DBG_ERROR,
				  "died at extended address code.\n");
			return -EREMOTEIO;
		}
		/* the remaining 8 bit address */
		ret = bcm1161_i2c_outb(i2c_adap->algo_data,
				       msg->addr & 0x7f, i2c);

		if ((ret != 1) && !nak_ok) {
			/* the chip did not ack / xmission error occurred */
			I2C_DEBUG(DBG_ERROR, "died at 2nd address code.\n");
			return -EREMOTEIO;
		}

		if (flags & I2C_M_RD) {
			ret = bcm1161_i2c_repstart(i2c_adap->algo_data, i2c);

			if (ret < 0)
				return -EIO;
			/* okay, now switch into reading mode */
			addr |= 0x01;
			ret = bcm1161_try_address(i2c_adap, addr, retries, i2c);

			if ((ret != 1) && !nak_ok) {
				I2C_DEBUG(DBG_ERROR,
					  "died at extended address code.\n");
				return -EREMOTEIO;
			}
		}
	} else {		/* normal 7bit address  */
		addr = (msg->addr << 1);

		if (flags & I2C_M_RD)
			addr |= 1;

		if (flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;

		ret = bcm1161_try_address(i2c_adap, addr, retries, i2c);

		if ((ret != 1) && !nak_ok) {
			i2c_errors++;
			return -EREMOTEIO;
		}
	}

	return 0;
}

static int __bcm1161_i2c_get_client(struct device *dev, void *addrp)
{
	struct i2c_client *client = i2c_verify_client(dev);
	int addr = *(int *)addrp;

	if (client && client->addr == addr)
		return true;

	return 0;
}

static struct device *bcm1161_i2c_get_client(struct i2c_adapter *adapter,
					     int addr)
{
	return device_find_child(&adapter->dev, &addr,
				 __bcm1161_i2c_get_client);
}

/* Master tranfer function */
static int bcm1161_xfer(struct i2c_adapter *i2c_adap,
			struct i2c_msg msgs[], int num)
{
	/* Based on i2c-algo-bit */
	struct i2c_msg *pmsg;
	struct bcm1161_i2c *i2c;

	int i, ret;
	unsigned short nak_ok;
	struct i2c_host_platform_data *pdata;
	struct i2c_client *client = NULL;
	struct i2c_slave_platform_data *pcdata;
	struct device *dev = NULL;

	i2c = i2c_get_adapdata(i2c_adap);
	dev = bcm1161_i2c_get_client(i2c_adap, msgs[0].addr);

	clk_enable(i2c->clk);

	if (dev) {
		client = i2c_verify_client(dev);
		pcdata =
		    (struct i2c_slave_platform_data *)client->dev.platform_data;
		if (num && pcdata)
			setrate(i2c_adap->algo_data, i2c, pcdata->i2c_spd);
	} else {
		if (num)
			setrate(i2c_adap->algo_data, i2c, I2C_SPD_UNKNOWN);
	}
	/* Send first start */
	bcm1161_i2c_set_autosense(i2c_adap->algo_data, true);

	pdata = i2c->adapter.dev.platform_data;
	if (pdata && pdata->set_oeb)
		pdata->set_oeb(i2c->adapter.id, false);

	ret = bcm1161_i2c_start(i2c_adap->algo_data, i2c);

	if (ret < 0) {
		goto xferend;
	}
	/* Loop through all messages */
	for (i = 0; i < num; i++) {
		pmsg = &msgs[i];
		nak_ok = pmsg->flags & I2C_M_IGNORE_NAK;

		/* for debug purpose */
		i2c->addr = pmsg->addr;

		if (!(pmsg->flags & I2C_M_NOSTART)) {
			/* Send repeated start only on subsequent messages */
			if (i) {
				ret =
				    bcm1161_i2c_repstart(i2c_adap->algo_data,
							 i2c);
				if (ret < 0) {
					goto xferend;
				}
			}

			ret = bcm1161_doAddress(i2c_adap, pmsg, i2c);

			if ((ret != 0) && !nak_ok) {
				I2C_DEBUG(DBG_INFO,
					  "NAK from device addr %2.2x msg #%d\n",
					  msgs[i].addr, i);
				goto xferend;
			}
		}

		if (pmsg->flags & I2C_M_RD) {
			/* read bytes into buffer */
			ret = bcm1161_readbytes(i2c_adap, pmsg, i2c);
			I2C_DEBUG(DBG_INFO, "read %d bytes.\n", ret);

			if (ret < pmsg->len) {
				ret = (ret < 0) ? ret : -EREMOTEIO;
				goto xferend;
			}
		} else {
			/* write bytes from buffer */
			ret = bcm1161_sendbytes(i2c_adap, pmsg, i2c);
			I2C_DEBUG(DBG_INFO, "wrote %d bytes.\n", ret);
			if (ret < pmsg->len) {
				ret = (ret < 0) ? ret : -EREMOTEIO;
				goto xferend;
			}
		}
	}
xferend:
	bcm1161_i2c_stop(i2c_adap->algo_data, i2c);
	bcm1161_i2c_set_autosense(i2c_adap->algo_data, false);

	if (pdata && pdata->set_oeb)
		pdata->set_oeb(i2c->adapter.id, true);
	clk_disable(i2c->clk);

	if (ret < 0 && i2c_adap->retries)
		ret = -EAGAIN;

	return (ret < 0) ? ret : num;
}

/* Controller initialization function */
static void bcm1161_i2c_enable_controller(void *base, struct bcm1161_i2c *i2c)
{
	I2C_DEBUG(DBG_TRACE, "");

#if defined(CONFIG_ARCH_BCMRING)
	chipcHw_busInterfaceClockEnable(chipcHw_REG_BUS_CLOCK_I2CH);
#endif

	/* reset controller (EN=0) */
	REG_I2C_CS(base) = REG_I2C_CS_SDA | REG_I2C_CS_SCL;
	REG_I2C_RCM(base) = 0;	/* disable CRC */
#if defined(CONFIG_ARCH_BCMRING)
	REG_I2C_TIM(base) = REG_I2C_CLK_93K;	/* select clock */
#else
	REG_I2C_TIM(base) = REG_I2C_CLK_85K;	/* select clock */
#endif
#if defined (CONFIG_ARCH_BCM215XX)
	REG_I2C_TIM(base) = 0x6B;	/* select clock */
#endif
	REG_I2C_ISR(base) = REG_I2C_ISR_SES_DONE |
	    REG_I2C_ISR_I2CERR | REG_I2C_ISR_TXFIFOEMPTY | REG_I2C_ISR_NOACK;
	REG_I2C_IER(base) = REG_I2C_IER_INT_EN |
	    REG_I2C_IER_ERRINT_EN | REG_I2C_IER_NOACK_EN;
	REG_I2C_TOUT(base) = REG_I2C_TOUT_EN | 0x7F;	/* initialize timeout */
	REG_I2C_FCR(base) = REG_I2C_FCR_FLUSH;	/* flush fifo */
	REG_I2C_CS(base) = REG_I2C_CS_SDA | REG_I2C_CS_SCL | REG_I2C_CS_EN;
}

/* BCM1161 I2C adaptor and algorithm definitions */
static u32 bcm1161_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL |
	    I2C_FUNC_10BIT_ADDR | I2C_FUNC_PROTOCOL_MANGLING;
}

static struct i2c_algorithm bcm1161_algo = {
	.master_xfer = bcm1161_xfer,
	.functionality = bcm1161_functionality,
};

/* Module initialization function */
int __init i2c_bcm1161_probe(struct platform_device *pdev)
{
	struct bcm1161_i2c *i2c;
	int rc;
	struct resource *res;
	struct i2c_host_platform_data *pdata = pdev->dev.platform_data;
	int irq;

	dev_dbg(&pdev->dev, "<%s>\n", __func__);

	pr_info("i2c_bcm1161_probe %p\n", pdev);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "can't get device resources\n");
		return -ENOENT;
	}

	pr_info("Resource = 0x%08X\n", res->start);
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "can't get irq number\n");
		return -ENOENT;
	}
	/* Allocate memory for control sturcture */
	i2c = kzalloc(sizeof(*i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	strcpy(i2c->adapter.name, pdev->name);
	i2c->adapter.owner = THIS_MODULE;
	i2c->adapter.algo = &bcm1161_algo;
	i2c->adapter.dev.parent = &pdev->dev;
	i2c->adapter.id = pdev->id;
	i2c->adapter.nr = pdev->id;
	i2c->adapter.algo_data = (void *)res->start;
	i2c->irq = irq;
	i2c->sysCtlHeader = register_sysctl_table(gSysCtl);
	i2c->adapter.dev.platform_data = pdev->dev.platform_data;
	if (pdata) {
		i2c->adapter.retries = pdata->retries;
		i2c->adapter.timeout = pdata->timeout;
	}

	i2c->irq = irq;
	i2c->sysCtlHeader = register_sysctl_table(gSysCtl);
	i2c->adapter.dev.platform_data = pdev->dev.platform_data;
	i2c->cur_spd = I2C_SPD_UNKNOWN;

#if !I2C_POLL_COMMAND_DONE
	/* Initialize IRQ and completion queue */
	init_completion(&i2c->cmd_complete);

	rc = request_irq(irq, bcm1161_i2c_isr, IRQF_DISABLED,
			 "bcm1161-i2c", i2c);

	if (rc < 0) {
		pr_info("i2c-bcm1161: %s failed to attach interrupt, rc = %d\n",
			__FUNCTION__, rc);
		goto fail_irq;
	}
#endif

	sprintf(&i2c->i2c_name[0], "%s.%d", i2c->adapter.name, i2c->adapter.id);
	i2c->clk = clk_get(NULL, i2c->i2c_name);

	if (i2c->clk == NULL) {
		pr_info("i2c-bcm1161: %s failed to get clock %s\n",
			__FUNCTION__, i2c->i2c_name);
		goto fail_add;
	}
	clk_enable(i2c->clk);
	/* I2C */
	/* Enable controller */
	bcm1161_i2c_enable_controller((void *)res->start, i2c);
	pr_info("#################Clk = %p name = %s\n", i2c->clk,
		i2c->i2c_name);
	/* Add I2C adaptor */
	pr_info("Added i2_adapter_id = 0x%08x\n", i2c->adapter.id);
	i2c_set_adapdata(&i2c->adapter, i2c);
	rc = i2c_add_numbered_adapter(&i2c->adapter);
	if (rc < 0) {
		pr_info
		    ("i2c-bcm1161: %s failed to add adapter, rc = %d\n",
		     __FUNCTION__, rc);
		goto fail_add;
	}
#if ADD_TURNAROUND_DELAY
	i2c->i2c_bus_clk_period = bcm1161_i2c_bus_clk_period(i2c);
	pr_debug("%s:%s(): i2c_bus_clk_period[%ld]\n",
		 __FILE__, __FUNCTION__, i2c->i2c_bus_clk_period);
#endif

	clk_disable(i2c->clk);
	platform_set_drvdata(pdev, i2c);
	return rc;

fail_add:

#if !I2C_POLL_COMMAND_DONE
	free_irq(irq, 0);
fail_irq:
#endif
	i2c_del_adapter(&i2c->adapter);

	kfree(i2c);
	i2c = NULL;
	return rc;
}

/* Module exit function */
static int i2c_bcm1161_remove(struct platform_device *pdev)
{
	struct bcm1161_i2c *i2c;

	i2c = platform_get_drvdata(pdev);

	if (!i2c)
		return 0;

#if !I2C_POLL_COMMAND_DONE
	free_irq(i2c->irq, 0);
#endif
	i2c_del_adapter(&i2c->adapter);
	/* unregister sysctl table */
	if (i2c->sysCtlHeader != NULL) {
		unregister_sysctl_table(i2c->sysCtlHeader);
		i2c->sysCtlHeader = NULL;
	}

	kfree(i2c);
	i2c = NULL;
	return 0;
}

static struct platform_driver bcm1161_i2c_driver = {
	.probe = i2c_bcm1161_probe,
	.remove = i2c_bcm1161_remove,
	.driver = {
		   .name = "i2cbcm",
		   .owner = THIS_MODULE,
		   },
};

/* I2C may be needed to bring up other drivers */
static int __init bcm1161_i2c_init(void)
{
	pr_info("bcm1161_i2c_init\n");
	return platform_driver_register(&bcm1161_i2c_driver);
}

arch_initcall(bcm1161_i2c_init);

static void __exit bcm1161_i2c_exit(void)
{
	platform_driver_unregister(&bcm1161_i2c_driver);
}

module_exit(bcm1161_i2c_exit);

MODULE_AUTHOR("Broadcom Corp.");
MODULE_DESCRIPTION("I2C hardware driver for BCM1161");
MODULE_LICENSE("GPL");
