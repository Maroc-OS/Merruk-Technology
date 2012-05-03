/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/mfd/max8986-core.c
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
*   @file   max8986-core.c
*
*   @brief  Core/Protocol driver for Maxim Max8986 PMU Chip
*
****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/mfd/core.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/mfd/max8986/max8986.h>
#include <mach/gpio.h>
#define MAX8986_INT_BASE                MAX8986_PM_REG_INT1
#define MAX8986_INT_MASK_BASE           MAX8986_PM_REG_INTMSK1
#define MAX8986_MUIC_INT_MASK_BASE      MAX8986_MUIC_REG_INTMSK1
#define MAX8986_MUIC_INT_BASE           MAX8986_MUIC_REG_INT1
#define MAX8986_MASK_ALL_INTS           0xFF

#define IRQ_TO_REG_INX(irq)             (irq / 8)
#define IRQ_TO_REG_BIT(irq)             (irq % 8)

static struct max8986 *max8986_info;

static struct max_pmu_irq *max8986_find_irq_handler(struct max8986 *max8986,
	int irq)
{
	struct max_pmu_irq  *p = NULL;
	struct max_pmu_irq  *ret = NULL;

	list_for_each_entry(p, &max8986->irq_handlers, node) {
		if (p->irq == irq) {
			ret = p;
			break;
		}
	}

	return ret;
}

int max8986_request_irq(struct max8986 *max8986, int irq, bool enable_irq,
	void (*handler) (int, void *), void *data)
{
	struct max_pmu_irq  *irq_info;

	pr_debug("%s\n", __func__);

	if (irq < 0 || irq >= MAX8986_TOTAL_IRQ || !handler ||
		(irq == MAX8986_IRQID_INT3_MUIC))
		return -EINVAL;

	if (WARN_ON(max8986_find_irq_handler(max8986, irq)))
		return -EBUSY;

	irq_info = kzalloc(sizeof(struct max_pmu_irq), GFP_KERNEL);
	if (!irq_info)
		return -ENOMEM;

	irq_info->handler = handler;
	irq_info->data = data;
	irq_info->irq_enabled = enable_irq;
	irq_info->irq = irq;

	mutex_lock(&max8986->list_lock);
	list_add(&irq_info->node, &max8986->irq_handlers);
	mutex_unlock(&max8986->list_lock);

	enable_irq ? max8986_enable_irq(max8986, irq) : \
		max8986_disable_irq(max8986, irq);

	return 0;
}
EXPORT_SYMBOL_GPL(max8986_request_irq);

int max8986_free_irq(struct max8986 *max8986, int irq)
{
	struct max_pmu_irq  *irq_info;

	pr_debug("%s\n", __func__);

	if (irq < 0 || irq >= MAX8986_TOTAL_IRQ ||
		(irq == MAX8986_IRQID_INT3_MUIC)) {
		return -EINVAL;
	}

	irq_info = max8986_find_irq_handler(max8986, irq);
	if (irq_info) {
		mutex_lock(&max8986->list_lock);
		list_del(&irq_info->node);
		mutex_unlock(&max8986->list_lock);
		kfree(irq_info);
	}

	/* disalbe IRQ as there is no handler */
	max8986_disable_irq(max8986, irq);
	return 0;
}
EXPORT_SYMBOL_GPL(max8986_free_irq);

int max8986_disable_irq(struct max8986 *max8986, int irq)
{
	int reg_inx;
	int st;
	int reg;
	u8 reg_val,mask;
	struct max_pmu_irq *handler;

	if (irq < 0 || irq >= MAX8986_TOTAL_IRQ ||
		(irq == MAX8986_IRQID_INT3_MUIC)) {
		return -EINVAL;
	}

	reg_inx = IRQ_TO_REG_INX(irq);
	if (reg_inx < MAX8986_NUM_INT_REG)
	{
		reg = reg_inx + MAX8986_INT_MASK_BASE;
		max8986->read_dev(max8986, reg, &mask);
		mask |= (1 << IRQ_TO_REG_BIT(irq));
	} else
	{
		if (max8986->muic_int_enable > 0) {
			mutex_lock(&max8986->muic_int_lock);
			max8986->muic_int_enable--;
			mutex_unlock(&max8986->muic_int_lock);
		}
		reg = (reg_inx - MAX8986_NUM_INT_REG)
			+ MAX8986_MUIC_INT_MASK_BASE;
		max8986->read_dev(max8986, reg, &mask);
		mask &= ~(1 << IRQ_TO_REG_BIT(irq));
	}


	st = max8986->write_dev(max8986, reg, mask);

	/* If no other MUIC sub-interrupt is enable the disable main MUIC int */
	if (max8986->muic_int_enable == 0)
	{
		printk(KERN_INFO "Disabling MUIC.....\n");
		st = max8986->read_dev(max8986, MAX8986_PM_REG_INTMSK3, &reg_val);
		reg_val |= (1 << IRQ_TO_REG_BIT(
				MAX8986_IRQID_INT3_MUIC));

		st |= max8986->write_dev(max8986, MAX8986_PM_REG_INTMSK3, reg_val);
		if (st < 0)
			return st;
	}

	handler = max8986_find_irq_handler(max8986, irq);
	if (handler)
		handler->irq_enabled = false;

	return 0;
}
EXPORT_SYMBOL_GPL(max8986_disable_irq);

int max8986_enable_irq(struct max8986 *max8986, int irq)
{
	int reg_inx;
	int st;
	int reg;
	u8 reg_val,mask;
	struct max_pmu_irq *handler;

	if (irq < 0 || irq >= MAX8986_TOTAL_IRQ ||
		(irq == MAX8986_IRQID_INT3_MUIC)) {
		return -EINVAL;
	}

	reg_inx = IRQ_TO_REG_INX(irq);

	if (reg_inx < MAX8986_NUM_INT_REG)
	{
		reg = reg_inx + MAX8986_INT_MASK_BASE;
		max8986->read_dev(max8986, reg, &mask);
		mask &= ~(1 << IRQ_TO_REG_BIT(irq));
	}
	else
	{
		reg = (reg_inx - MAX8986_NUM_INT_REG)
			+ MAX8986_MUIC_INT_MASK_BASE;
		max8986->read_dev(max8986, reg, &mask);
		mask |= (1 << IRQ_TO_REG_BIT(irq));
		if(max8986->muic_int_enable == 0)
		{
			printk(KERN_INFO "Enabling MUIC.....\n");
			st = max8986->read_dev(max8986, MAX8986_PM_REG_INTMSK3,
				&reg_val);
			reg_val &= ~(1 << IRQ_TO_REG_BIT(MAX8986_IRQID_INT3_MUIC));
			st |= max8986->write_dev(max8986,
				MAX8986_PM_REG_INTMSK3, reg_val);
			if (st) {
				pr_err("%s: register read/write error\n",
					__func__);
				return st;
			}
		}
		mutex_lock(&max8986->muic_int_lock);
		max8986->muic_int_enable++;
		mutex_unlock(&max8986->muic_int_lock);
	}

	st = max8986->write_dev(max8986, reg, mask);

	handler = max8986_find_irq_handler(max8986, irq);
	if (!handler)
		pr_err("WARNING: %s: enabling pmu irq \
			without registering handler!!!\n", __func__);
	else
		handler->irq_enabled = true;

	return st;
}
EXPORT_SYMBOL_GPL(max8986_enable_irq);

int max8986_register_ioctl_handler(struct max8986 *max8986, u8 sub_dev_id,
	pmu_subdev_ioctl_handler handler, void *pri_data)
{
	pr_debug("%s\n", __func__);

	if (sub_dev_id >= MAX8986_SUBDEV_MAX || !handler)
		return -EINVAL;

	if (max8986->ioctl_handler[sub_dev_id].handler) {
		pr_info("%s: handler already registered - id = %u\n",
			__func__ , sub_dev_id);
		return -EINVAL;
	}

	max8986->ioctl_handler[sub_dev_id].handler = handler;
	max8986->ioctl_handler[sub_dev_id].pri_data = pri_data;

	return 0;
}
EXPORT_SYMBOL_GPL(max8986_register_ioctl_handler);

void max8986_shutdown(struct max8986 *max8986)
{
	int ret = 0;
	u8 hostact;

	pr_debug("%s\n", __func__);

	ret = max8986->read_dev(max8986, MAX8986_PM_REG_HOSTACT, &hostact);
	if (!ret) {
		hostact |= MAX8986_HOSTACT_HOSTDICOFF;
		max8986->write_dev(max8986, MAX8986_PM_REG_HOSTACT, hostact);
	}
}
EXPORT_SYMBOL(max8986_shutdown);

static void max8986_power_off(void)
{
	pr_debug("%s\n", __func__);

	if (max8986_info)
		max8986_shutdown(max8986_info);
}

static void max8986_irq_workq(struct work_struct *work)
{
	struct max8986 *max8986 = container_of(work, struct max8986, work);
	u8 intStatus[MAX8986_NUM_INT_REG + MAX8986_NUM_MUIC_INT_REG];
	struct max_pmu_irq  *handler;
	int int_state;
	pr_debug("%s\n", __func__);

	do {
	/* Read all interrupt status registers.
	 * All interrupt status registers are R&C
	 */
	if (max8986->read_mul_dev(max8986, MAX8986_INT_BASE,
		MAX8986_NUM_INT_REG, intStatus)) {
		pr_err("%s: pmu int reg read error\n", __func__);
		return;
	}
	if (max8986->read_mul_dev(max8986, MAX8986_MUIC_INT_BASE,
		MAX8986_NUM_MUIC_INT_REG,
		(intStatus+MAX8986_NUM_INT_REG))) {
		pr_err("%s: pmu int reg read error\n", __func__);
		return;
	}

	mutex_lock(&max8986->list_lock);
	list_for_each_entry(handler, &max8986->irq_handlers, node)
	{
		if (handler->irq_enabled &&
			(intStatus[IRQ_TO_REG_INX(handler->irq)] &
				(1 << IRQ_TO_REG_BIT(handler->irq)))) {
			handler->handler(handler->irq, handler->data);
		}
	}
		
	mutex_unlock(&max8986->list_lock);
		/*
		* PMU interrupt is GPIO based and is edge trigerred interrupt
		* (GPIO lib supports only edge triggerred interrupt for now).
		* Since there are three interrupt status regs in the PMU,
		* following scenario can occur and lock out the PMU from
		* generating any more interrupts:
		* 1. One interrupt bit in INT1 and INT2 regs. PMU asserts
		* the interrupt line low.
		* 2. ISR schedules workqueue to handle the interrupt.
		* 3. Worker thread reads INT1, which clears INT1 bits.
		* 4. Worker thread begins read of INT2. At the same time,
		* another interrupt bit in INT1 gets set.
		* 5. Worker thread completes reading INT2 and INT3 and
		* completes interrupt handling and returns.
		* 6. But the PMU INT line is still asserted because of the
		* interrupt bit that got set in step 3 above.
		* Since no interrupt was recognized by Linux, this bit never
		* gets cleared and no more interrutps are recevied from the
		* PMU.
		*
		* So to fix the issue, after processing all the interrrupts,
		* the interrupt line status is read. If the line is still
		* asserted, we again read all the three status regs and handle
		* the interrupts. This is repeated until the INT line is no
		* longer asserted.
		*/
		int_state = gpio_get_value(IRQ_TO_GPIO(max8986->irq));
		if( int_state==0 )
			pr_debug("%s: PMU INT line is still asserted\n", __func__);

	}while (int_state == 0); 
	pr_debug("%s: end\n", __func__);
}

static irqreturn_t max8986_irq(int irq, void *dev_id)
{
	struct max8986 *max8986 = dev_id;

	pr_info("%s\n", __func__);

	if (queue_work(max8986->pmu_workqueue, &max8986->work) == 0) {
		pr_info("%s: Work previously queued\n", __func__);
	}

	return IRQ_HANDLED;
}

static int max8986_client_dev_register(struct max8986 *max8986,
	const char *name)
{
	struct mfd_cell cell = { };
	int ret;

	cell.name = name;
	ret = mfd_add_devices(
		&max8986->max8986_cl[MAX8986_SLAVE_CORE].client->dev,
		-1, &cell, 1, NULL, 0);
	return ret;
}

static int max8986_find_slave(struct max8986 *max8986, int slave)
{
	int i;

	for (i = 0; i < MAX8986_NUM_SLAVES; i++)
		if (max8986->max8986_cl[i].addr == slave)
			return i;

	pr_err("%s: wrong slave id %d\n", __func__, slave);
	return -EINVAL;
}

static int max8986_read_pmu_register(struct max8986 *max8986, u32 command,
	u8 *reg_val)
{
	s32 data = 0;
	int i;
	u8 slave = MAX8986_REG_SLAVE(command);
	u8 reg = MAX8986_REG_DECODE(command);

	mutex_lock(&max8986->i2c_rw_lock);
	i = max8986_find_slave(max8986, slave);

	if (i >= 0)
		data = i2c_smbus_read_byte_data(max8986->max8986_cl[i].client,
			reg);
	mutex_unlock(&max8986->i2c_rw_lock);

	if (data < 0) {
		pr_err("%s: pmu i2c read error: ret : %d\n", __func__,
			*reg_val);
		return data;
	}
	*reg_val = (u8)data;
	pr_debug("%s: data read %x\n", __func__, *reg_val);

	return 0;
}

static int max8986_write_pmu_register(struct max8986 *max8986, u32 command,
	u8 val)
{
	int ret = -EINVAL;
	int i;
	u8 slave = MAX8986_REG_SLAVE(command);
	u8 reg = MAX8986_REG_DECODE(command);

	mutex_lock(&max8986->i2c_rw_lock);
	i = max8986_find_slave(max8986, slave);

	if (i >= 0)
		ret =  i2c_smbus_write_byte_data(max8986->max8986_cl[i].
			client, reg, val);
	mutex_unlock(&max8986->i2c_rw_lock);

	pr_debug("%s: slave: %x reg: %x index : %d\n", __func__, slave, reg, i);

	return ret;
}

static int max8986_read_pmu_mul_register(struct max8986 *max8986, u32 command,
	u32 length, u8 *val)
{
	int ret = -EINVAL;
	int index;
	u8 slave = MAX8986_REG_SLAVE(command);
	u8 reg = MAX8986_REG_DECODE(command);

	mutex_lock(&max8986->i2c_rw_lock);
	index = max8986_find_slave(max8986, slave);

	pr_debug("%s: slave: %x reg: %x\n", __func__, slave, reg);

	if (index >= 0) {
		ret = i2c_smbus_read_i2c_block_data(
			max8986->max8986_cl[index].client, reg,	length, val);
		if (ret < length)
			return -EIO;
	}
	mutex_unlock(&max8986->i2c_rw_lock);

	return 0;
}

static int max8986_write_pmu_mul_register(struct max8986 *max8986, u32 command,
	u32 length, u8 *val)
{
	int ret = -EINVAL;
	int index;
	u8 slave = MAX8986_REG_SLAVE(command);
	u8 reg = MAX8986_REG_DECODE(command);

	mutex_lock(&max8986->i2c_rw_lock);
	index = max8986_find_slave(max8986, slave);
	if (index >= 0) {
		ret = i2c_smbus_write_i2c_block_data(
			max8986->max8986_cl[index].client, reg, length, val);
	}
	mutex_unlock(&max8986->i2c_rw_lock);
	return ret;
}

static void max8986_init_chip(struct max8986 *max8986)
{
	int i;
	u8 reg_val;

	max8986->read_dev(max8986, MAX8986_PM_REG_CSRCTRL1, &reg_val);
	if (MAX8986_ENABLE_DVS & max8986->flags) {
		reg_val |= MAX8986_CSRCTRL1_CSR_DVS_EN;

		if(max8986->pdata->csr_lpm_volt != -1)
		{
			reg_val &= ~MAX8986_CSRCTRL1_CSR_VOUT_L_MASK;
			reg_val |= max8986->pdata->csr_lpm_volt & MAX8986_CSRCTRL1_CSR_VOUT_L_MASK;
		}

		if(max8986->pdata->csr_nm_volt != -1)
			max8986->write_dev(max8986, MAX8986_PM_REG_CSRCTRL10,
				(u8)max8986->pdata->csr_nm_volt);
		if(max8986->pdata->csr_nm2_volt != -1)
			max8986->write_dev(max8986, MAX8986_PM_REG_CSRCTRL3,
				(u8)max8986->pdata->csr_nm2_volt);

	}
	else
	{
		reg_val &= ~MAX8986_CSRCTRL1_CSR_DVS_EN;
	}
	reg_val &= ~MAX8986_CSRCTRL1_CSROKACT;
	max8986->write_dev(max8986, MAX8986_PM_REG_CSRCTRL1, reg_val);

	/* mask all interrupts */
	for (i = MAX8986_PM_REG_INTMSK1; i <= MAX8986_PM_REG_INTMSK3; i++)
		max8986->write_dev(max8986, i, MAX8986_MASK_ALL_INTS);

	for (i = MAX8986_MUIC_REG_INTMSK1; i <= MAX8986_MUIC_REG_INTMSK3; i++)
		max8986->write_dev(max8986, i, MAX8986_MASK_ALL_INTS);

	/* read & clear all interrupts */
	for (i = 0; i < MAX8986_NUM_INT_REG; i++)
		max8986->read_dev(max8986, MAX8986_PM_REG_INT1 + i, &reg_val);

	for (i = 0; i < MAX8986_NUM_MUIC_INT_REG; i++)
		max8986->read_dev(max8986, MAX8986_MUIC_REG_INT1 + i, &reg_val);

	max8986->write_dev(max8986, MAX8986_PM_REG_A8_A2_LDOCTRL,0x73);
	
}

static int max8986_open(struct inode *inode, struct file *file)
{
	pr_debug("%s\n", __func__);
	file->private_data = PDE(inode)->data;

	return 0;
}

int max8986_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static int max8986_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	struct max8986 *max8986 =  file->private_data;
	pmu_reg reg;
	int ret = 0, i;
	u8 reg_val;

	if (!max8986)
		return -ENOTTY;

	switch (cmd) {
	case BCM_PMU_IOCTL_ENABLE_INTS:
		/* Clear all latched interrupts if any */
		for (i = 0; i < MAX8986_NUM_INT_REG ; i++) {
			if (max8986->read_dev(max8986, MAX8986_INT_BASE + i,
				&reg_val)) {
				return -EFAULT;
			}
			pr_debug("int register 0x%x = 0x%x\n",
				(MAX8986_INT_BASE + i), reg_val);
		}
		enable_irq(max8986->irq);
		break;

	case BCM_PMU_IOCTL_DISABLE_INTS:
		disable_irq_nosync(max8986->irq);
		break;

	case BCM_PMU_IOCTL_READ_REG:
		if (copy_from_user(&reg, (pmu_reg *)arg, sizeof(pmu_reg)) != 0)
			return -EFAULT;
		ret = max8986->read_dev(max8986, reg.reg,&reg_val);
		reg.val = reg_val;
		if (ret < 0)
			return -EFAULT;

		if (copy_to_user((pmu_reg *)arg, &reg, sizeof(pmu_reg)) != 0)
			return -EFAULT;
		break;

	case BCM_PMU_IOCTL_WRITE_REG:
		if (copy_from_user(&reg, (pmu_reg *)arg, sizeof(pmu_reg)) != 0)
			return -EFAULT;
		ret =  max8986->write_dev(max8986, reg.reg, (u8)reg.val);
		break;

	case BCM_PMU_IOCTL_SET_VOLTAGE:
	case BCM_PMU_IOCTL_GET_VOLTAGE:
	case BCM_PMU_IOCTL_GET_REGULATOR_STATE:
	case BCM_PMU_IOCTL_SET_REGULATOR_STATE:
	case BCM_PMU_IOCTL_ACTIVATESIM:
	case BCM_PMU_IOCTL_DEACTIVATESIM:
		if (max8986->ioctl_handler[MAX8986_SUBDEV_REGULATOR].handler)
			return
			max8986->ioctl_handler[MAX8986_SUBDEV_REGULATOR].
			handler(cmd, arg,
		max8986->ioctl_handler[MAX8986_SUBDEV_REGULATOR].pri_data);
		else
			return -ENOTTY;

	case BCM_PMU_IOCTL_START_CHARGING:
	case BCM_PMU_IOCTL_STOP_CHARGING:
	case BCM_PMU_IOCTL_SET_CHARGING_CURRENT:
	case BCM_PMU_IOCTL_GET_CHARGING_CURRENT:
		if (max8986->ioctl_handler[MAX8986_SUBDEV_POWER].handler)
			return
			max8986->ioctl_handler[MAX8986_SUBDEV_POWER].handler(
			cmd, arg,
			max8986->ioctl_handler[MAX8986_SUBDEV_POWER].pri_data);
		else
			return -ENOTTY;

	case BCM_PMU_IOCTL_POWERONOFF:
		max8986_shutdown(max8986);
		break;

	default:
		pr_err("%s: unsupported cmd\n", __func__);
		ret = -ENOTTY;
	}
	return ret;
}

#define MAX_USER_INPUT_LEN      100
#define MAX_REGS_READ_WRITE     10

enum pmu_debug_ops {
	PMUDBG_READ_REG = 0UL,
	PMUDBG_WRITE_REG,
};

struct pmu_debug {
	int read_write;
	int len;
	int addr;
	u8 val[MAX_REGS_READ_WRITE];
};

static void max8986_dbg_usage(void)
{
	printk(KERN_INFO "Usage:\n");
	printk(KERN_INFO "Read a register: echo 0x0800 > /proc/pmu0\n");
	printk(KERN_INFO
		"Read multiple regs: echo 0x0800 -c 10 > /proc/pmu0\n");
	printk(KERN_INFO
		"Write multiple regs: echo 0x0800 0xFF 0xFF > /proc/pmu0\n");
	printk(KERN_INFO
		"Write single reg: echo 0x0800 0xFF > /proc/pmu0\n");
	printk(KERN_INFO "Max number of regs in single write is :%d\n",
		MAX_REGS_READ_WRITE);
	printk(KERN_INFO "Register address is encoded as follows:\n");
	printk(KERN_INFO "0xSSRR, SS: i2c slave addr, RR: register addr\n");
}

static int max8986_dbg_parse_args(char *cmd, struct pmu_debug *dbg)
{
	char *tok;                 /* used to separate tokens             */
	const char ct[] = " \t";   /* space or tab delimits the tokens    */
	bool count_flag = false;   /* whether -c option is present or not */
	int tok_count = 0;         /* total number of tokens parsed       */
	int i = 0;

	dbg->len        = 0;

	/* parse the input string */
	while ((tok = strsep(&cmd, ct)) != NULL) {
		pr_debug("token: %s\n", tok);

		/* first token is always address */
		if (tok_count == 0) {
			sscanf(tok, "%x", &dbg->addr);
		} else if (strnicmp(tok, "-c", 2) == 0) {
			/* the next token will be number of regs to read */
			tok = strsep(&cmd, ct);
			if (tok == NULL)
				return -EINVAL;

			tok_count++;
			sscanf(tok, "%d", &dbg->len);
			count_flag = true;
			break;
		} else {
			int val;

			/* this is a value to be written to the pmu register */
			sscanf(tok, "%x", &val);
			if (i < MAX_REGS_READ_WRITE) {
				dbg->val[i] = val;
				i++;
			}
		}

		tok_count++;
	}

	/* decide whether it is a read or write operation based on the
	 * value of tok_count and count_flag.
	 * tok_count = 0: no inputs, invalid case.
	 * tok_count = 1: only reg address is given, so do a read.
	 * tok_count > 1, count_flag = false: reg address and atleast one
	 *     value is present, so do a write operation.
	 * tok_count > 1, count_flag = true: to a multiple reg read operation.
	 */
	switch (tok_count) {
	case 0:
		return -EINVAL;
	case 1:
		dbg->read_write = PMUDBG_READ_REG;
		dbg->len = 1;
		break;
	default:
		if (count_flag == true) {
			dbg->read_write = PMUDBG_READ_REG;
		} else {
			dbg->read_write = PMUDBG_WRITE_REG;
			dbg->len = i;
		}
	}

	return 0;
}

static ssize_t max8986_write(struct file *file, const char __user *buffer,
	size_t len, loff_t *offset)
{
	struct max8986 *max8986 = file->private_data;
	struct pmu_debug dbg;
	char cmd[MAX_USER_INPUT_LEN];
	int ret, i;

	pr_debug("%s\n", __func__);

	if (!max8986) {
		pr_err("%s: driver not initialized\n", __func__);
		return -EINVAL;
	}

	if (len > MAX_USER_INPUT_LEN)
		len = MAX_USER_INPUT_LEN;

	if (copy_from_user(cmd, buffer, len)) {
		pr_err("%s: copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	/* chop of '\n' introduced by echo at the end of the input */
	if (cmd[len - 1] == '\n')
		cmd[len - 1] = '\0';

	if (max8986_dbg_parse_args(cmd, &dbg) < 0) {
		max8986_dbg_usage();
		return -EINVAL;
	}

	pr_debug("operation: %s\n", (dbg.read_write == PMUDBG_READ_REG) ?
		"read" : "write");
	pr_debug("address  : 0x%x\n", dbg.addr);
	pr_debug("length   : %d\n", dbg.len);

	if (dbg.read_write == PMUDBG_READ_REG) {
		ret = max8986->read_mul_dev(max8986, dbg.addr, dbg.len,
			dbg.val);
		if (ret < 0) {
			pr_err("%s: pmu reg read failed\n", __func__);
			return -EFAULT;
		}

		for (i = 0; i < dbg.len; i++, dbg.addr++)
			printk(KERN_INFO "[%x] = 0x%02x\n", dbg.addr,
				dbg.val[i]);
	} else {
		ret = max8986->write_mul_dev(max8986, dbg.addr, dbg.len,
			dbg.val);
		if (ret < 0) {
			pr_err("%s: pmu reg write failed\n", __func__);
			return -EFAULT;
		}
	}

	*offset += len;

	return len;
}

static const struct file_operations max8986_pmu_ops = {
	.open = max8986_open,
	.ioctl = max8986_ioctl,
	.write = max8986_write,
	.release = max8986_release,
	.owner = THIS_MODULE,
};

static u16 max8986_slave[] = {
	MAX8986_RTC_I2C_ADDRESS,
	MAX8986_MUIC_I2C_ADDRESS,
	MAX8986_AUDIO_I2C_ADDRESS
};

static int __devinit max8986_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int                             ret, i;
	struct max8986                  *max8986;
	struct max8986_platform_data    *pdata = client->dev.platform_data;

	pr_info("%s: client name %s\n", __func__, client->name);

	max8986 = kzalloc(sizeof(struct max8986), GFP_KERNEL);
	if (!max8986) {
		ret = -ENOMEM;
		goto err_alloc_mem;
	}

	/* Create all sub device client structure */
	for (i = 0; i < MAX8986_NUM_SLAVES; i++) {
		if (i == 0) {
			max8986->max8986_cl[i].client = client;
			max8986->max8986_cl[i].addr = client->addr;
		} else {
			max8986->max8986_cl[i].addr = max8986_slave[i-1];
			max8986->max8986_cl[i].client =
			  i2c_new_dummy(client->adapter, max8986_slave[i-1]);
			if (!max8986->max8986_cl[i].client) {
				pr_err("%s : failed to create new i2c slave \
					with addr 0x%x\n", __func__,
					max8986->max8986_cl[i].addr);
				ret = -ENOMEM;
				goto err_slave_reg;
			}
			max8986->max8986_cl[i].client->dev.platform_data =
				pdata;
		}
	}

	i2c_set_clientdata(client, max8986);

	max8986->pdata = pdata;
	max8986->irq = client->irq;
	max8986->flags = pdata->flags;
	INIT_LIST_HEAD(&max8986->irq_handlers);
	INIT_WORK(&max8986->work, max8986_irq_workq);

	max8986->pmu_workqueue = create_workqueue("pmu_events");
	if (!max8986->pmu_workqueue) {
		ret = -ENOMEM;
		pr_err("%s: failed to create work queue", __func__);
		goto err_create_workq;
	}

	mutex_init(&max8986->list_lock);
	mutex_init(&max8986->i2c_rw_lock);
	mutex_init(&max8986->muic_int_lock);

	max8986->read_dev =  max8986_read_pmu_register;
	max8986->write_dev =  max8986_write_pmu_register;
	max8986->read_mul_dev = max8986_read_pmu_mul_register;
	max8986->write_mul_dev = max8986_write_pmu_mul_register;

	max8986_info = max8986;

	max8986_init_chip(max8986);

	/* Do platform specific inits */
	if (pdata->pmu_event_cb)
		pdata->pmu_event_cb(PMU_EVENT_INIT_PLATFORM, 0);
	if (pdata->max8986_sysparms)
		pdata->max8986_sysparms(max8986);

	ret = request_irq(client->irq,
		max8986_irq,
		(IRQF_DISABLED | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND),
		"max8986_irq",
		max8986);
	if (ret < 0) {
		pr_err("%s: failed to register max8986 pmu irq: %d\n",
			__func__, ret);
		goto err_request_irq;
	}

	/* Register battery charging device	*/
	if(max8986->flags & MAX8986_USE_POWER)
		max8986_client_dev_register(max8986, "max8986-power");
	/* Register regulator device */
	if(max8986->flags & MAX8986_USE_REGULATORS)
		max8986_client_dev_register(max8986, "max8986-regulator");
	/* Register RTC device */
	if(max8986->flags & MAX8986_USE_RTC)
		max8986_client_dev_register(max8986, "max8986-rtc");
	/* Register PONKEY device */
	if(max8986->flags & MAX8986_USE_PONKEY)
		max8986_client_dev_register(max8986, "max8986-ponkey");
	/* Register AUDIO device */
	if(max8986->flags & MAX8986_ENABLE_AUDIO)
		max8986_client_dev_register(max8986, "max8986-audio");
	/* Register MUIC device */
	if(max8986->flags & MAX8986_ENABLE_MUIC)
		max8986_client_dev_register(max8986, "max8986-muic");

	if(max8986->flags & MAX8986_REGISTER_POWER_OFF) {
		pr_info("%s: registering pm_power_off function\n", __func__);
		pm_power_off = max8986_power_off;
	}

	disable_irq(client->irq);
	proc_create_data("pmu0", S_IRWXUGO, NULL, &max8986_pmu_ops, max8986);
	enable_irq(client->irq);

	max8986->read_dev(max8986, MAX8986_PM_REG_ID, (u8 *)&ret);
	pr_info("%s success. pmu id : %d\n", __func__, ret);

	return 0;

err_request_irq:
	mutex_destroy(max8986->list_lock);
	mutex_destroy(max8986->i2c_rw_lock);
	mutex_destroy(max8986->muic_int_lock);
	max8986_info = NULL;
	destroy_workqueue(max8986->pmu_workqueue);
err_create_workq:
err_slave_reg:
	for (i = 0; i < MAX8986_NUM_SLAVES; i++)
		if (max8986->max8986_cl[i].client)
			i2c_unregister_device(max8986->max8986_cl[i].client);

	kfree(max8986);
err_alloc_mem:
	return ret;
}

static int __devexit max8986_remove(struct i2c_client *client)
{
	struct max8986 *max8986;
	int i;

	max8986 = i2c_get_clientdata(client);
	free_irq(client->irq, max8986);
	mfd_remove_devices(&client->dev);
	mutex_destroy(max8986->list_lock);
	mutex_destroy(max8986->i2c_rw_lock);
	mutex_destroy(max8986->muic_int_lock);

	pm_power_off = NULL;
	max8986_info = NULL;

	for (i = 0; i < MAX8986_NUM_SLAVES; i++)
		if (max8986->max8986_cl[i].client)
			i2c_unregister_device(max8986->max8986_cl[i].client);

	destroy_workqueue(max8986->pmu_workqueue);
	remove_proc_entry("pmu0", NULL);
	kfree(max8986);

	return 0;
}

static struct i2c_device_id max8986_idtable[] = {
	{"max8986", 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, pmu_idtable);
static struct i2c_driver max8986_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "max8986"
	},
	.id_table = max8986_idtable,
	.probe = max8986_probe,
	.remove = __devexit_p(max8986_remove),
};

static int __init max8986_init(void)
{
	return i2c_add_driver(&max8986_driver);
}

static void __exit max8986_exit(void)
{
	i2c_del_driver(&max8986_driver);
}

subsys_initcall(max8986_init);
module_exit(max8986_exit);

MODULE_DESCRIPTION("Core/Protocol driver for Maxim PMU Chip");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TKG");
