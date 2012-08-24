/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/gpio.c
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
*   @file   gpio.c
*   @brief  This file defines the GPIO interface
*
*****************************************************************************/
/**
*   @defgroup   GPIOAPIGroup   GPIO API's
*   @brief      This group defines the GPIO API
*
*****************************************************************************/
/*
 * GPIOLIB interface functions
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <asm-generic/bug.h>

/**
 * BCM Specfic GPIO registers
 */
#define GPIO_IOTR(x)	(0x00 + ((x) >> 4) * 4)	/**< 4 regs */
#define GPIO_GPOR(x)	(0x10 + ((x) >> 5) * 4)	/**< 2 regs */
#define GPIO_GPIPS(x)	(0x18 + ((x) >> 5) * 4)	/**< 2 regs */
#define GPIO_GPIPEN(x)	(0x20 + ((x) >> 5) * 4)	/**< 2 regs */
#define GPIO_GPIPUD(x)	(0x28 + ((x) >> 5) * 4)	/**< 2 regs */
#define GPIO_GPIMR(x)	(0x30 + ((x) >> 5) * 4)	/**< 2 regs */
#define GPIO_GPICR(x)	(0x38 + ((x) >> 5) * 4)	/**< 2 regs */
#define GPIO_GPITR(x)	(0x40 + ((x) >> 4) * 4)	/**< 4 regs */
#define GPIO_GPISR(x)	(0x50 + ((x) >> 5) * 4)	/**< 2 regs */
#define GPIO_GPDBR(x)	(0x60 + ((x) >> 3) * 4)	/**< 8 regs */

/**
 * Types of the Interrupt
 */
enum gpio_interrupt_type {
	GPIO_NO_INTERRUPT = 0x00,
	GPIO_RISING_EDGE_INTERRUPT_TRIGGER = 0x01,
	GPIO_FALLING_EDGE_INTERRUPT_TRIGGER = 0x02,
	GPIO_BOTH_EDGE_INTERRUPT_TRIGGER = 0x03,
};

/**
 * Types of GPIO mode
 */
enum gpio_io_type {
	GPIO_INPUT = 0x00,
	GPIO_INPUT_INTERRUPT = 0x01,
	GPIO_OUTPUT = 0x02,
	GPIO_UNUSED = 0x03,
};

static struct bcm_gpio_port *bcm_gpio_ports;
static DEFINE_SPINLOCK(bcm_gpio_lock);

void gpio_isr_handler(uint32_t irq, struct irq_desc *desc)
{
	uint32_t mask;
	uint32_t gpio_num;
	uint32_t loop = 0;
	struct bcm_gpio_port *port = (struct bcm_gpio_port *)get_irq_data(irq);
	for (loop = 0; loop < 2; loop++) {
		gpio_num = GPIO_TO_IRQ(loop << 5);
		mask = readl(port->base + GPIO_GPISR(loop << 5));
		for (; mask != 0; mask >>= 1, gpio_num++) {
			if ((mask & 1) == 0)
				continue;
			desc = irq_desc + gpio_num;
			BUG_ON(!(desc->handle_irq));
			desc->handle_irq(gpio_num, desc);
		}
	}
}

static void gpio_irq_ack(uint32_t irq)
{
	uint32_t val;
	uint32_t gpio = IRQ_TO_GPIO(irq);
	val = readl(bcm_gpio_ports->base + GPIO_GPICR(gpio));
	val |= (1 << (gpio & 31));
	writel(val, bcm_gpio_ports->base + GPIO_GPICR(gpio));
}

static void gpio_irq_mask(uint32_t irq)
{
	uint32_t val;
	uint32_t gpio = IRQ_TO_GPIO(irq);
	val = readl(bcm_gpio_ports->base + GPIO_GPIMR(gpio));
	val &= ~(1 << (gpio & 31));
	writel(val, bcm_gpio_ports->base + GPIO_GPIMR(gpio));
}

static void gpio_irq_unmask(uint32_t irq)
{
	uint32_t val;
	uint32_t gpio = IRQ_TO_GPIO(irq);
	val = readl(bcm_gpio_ports->base + GPIO_GPIMR(gpio));
	val |= (1 << (gpio & 31));
	writel(val, bcm_gpio_ports->base + GPIO_GPIMR(gpio));
}

static int32_t gpio_irq_set_type(uint32_t irq, uint32_t type)
{
	uint32_t gpio = IRQ_TO_GPIO(irq);
	uint32_t val, edge;

	switch (type) {
	case IRQ_TYPE_EDGE_RISING:
		edge = GPIO_RISING_EDGE_INTERRUPT_TRIGGER;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		edge = GPIO_FALLING_EDGE_INTERRUPT_TRIGGER;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		edge = GPIO_BOTH_EDGE_INTERRUPT_TRIGGER;
		break;
	default:
		return -EINVAL;
	}

	val = readl(bcm_gpio_ports->base + GPIO_GPITR(gpio));
	val &= ~(GPIO_BOTH_EDGE_INTERRUPT_TRIGGER << ((gpio & 15) << 1));
	val |= edge << ((gpio & 15) << 1);
	writel(val, bcm_gpio_ports->base + GPIO_GPITR(gpio));
	return 0;
}

static void _set_gpio_direction(struct gpio_chip *chip, unsigned offset,
				int dir)
{
	struct bcm_gpio_port *port =
	    container_of(chip, struct bcm_gpio_port, chip);
	uint32_t val;

	val = readl(port->base + GPIO_IOTR(offset));
	val &= ~(3 << ((offset & 15) << 1));
	val |= (dir & 3) << ((offset & 15) << 1);
	writel(val, port->base + GPIO_IOTR(offset));
}

static void bcm_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct bcm_gpio_port *port =
	    container_of(chip, struct bcm_gpio_port, chip);
	void __iomem *reg = port->base + GPIO_GPOR(offset);
	unsigned long flags;
	uint32_t val;

	offset &= 31;
	spin_lock_irqsave(&bcm_gpio_lock, flags);
	val = (readl(reg) & (~(1 << offset))) | (value << offset);
	writel(val, reg);
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);
}

static int bcm_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct bcm_gpio_port *port =
	    container_of(chip, struct bcm_gpio_port, chip);

	return (readl(port->base + GPIO_GPIPS(offset)) >> (offset & 31)) & 1;
}

static int bcm_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	unsigned long flags;

	spin_lock_irqsave(&bcm_gpio_lock, flags);
	_set_gpio_direction(chip, offset, GPIO_INPUT_INTERRUPT);
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);
	return 0;
}

static int bcm_gpio_direction_output(struct gpio_chip *chip,
				     unsigned offset, int value)
{
	unsigned long flags;

	spin_lock_irqsave(&bcm_gpio_lock, flags);
	_set_gpio_direction(chip, offset, GPIO_OUTPUT);
	bcm_gpio_set(chip, offset, value);
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);
	return 0;
}

/** @addtogroup GPIOAPIGroup
	@{
*/

/**
	Set the debounce value for the specified GPIO number
	@param	gpio (in) GPIO number
	@param	db_val	debounce value for the GPIO
	@return	either EINVAL or zero.

	This function sets the debounce value to the specified GPIO number.
	The supported debounce values are : <br>
	0 1 ms<br>
	1 2 ms<br>
	2 4 ms<br>
	3 8 ms<br>
	4 16 ms<br>
	5 32 ms<br>
	6 64 ms<br>
	7 128 ms<br>
	It returns 0 for success or a negative value if there is an error.
**/

int bcm_gpio_set_db_val(unsigned int gpio, unsigned int db_val)
{
	unsigned long flags;
	uint32_t val, pos;
	void __iomem *reg = bcm_gpio_ports->base + GPIO_GPDBR(gpio);

	if (!gpio_is_valid(gpio))
		return -EINVAL;

	spin_lock_irqsave(&bcm_gpio_lock, flags);
	gpio &= 31;
	val = readl(reg);

	pos = (gpio & 0x7) << 2;
	val = (val & ~(0xf << pos)) | ((db_val & 0xf) << pos);
	writel(val, reg);
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);

	return 0;
}

/**
	Enable/Disable pull up's and pull down's for specified GPIO.
	@param	gpio (in) GPIO number
	@param	enable  Boolean, true to enable.
	@return	either EINVAL or zero.

	This function is used to enable/disable the pull up/pull down
	for the specified GPIO. The input boolean parameter enable represents <br>
	0 Disable GPIOs pull up/down <br>
	1 Enable GPIOs pull up/down <br>
	It returns 0 for success or a negative value if there is an error.

**/
int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable)
{
	unsigned long flags;
	uint32_t val;
	void __iomem *reg = bcm_gpio_ports->base + GPIO_GPIPEN(gpio);
	if (!gpio_is_valid(gpio))
		return -EINVAL;
	spin_lock_irqsave(&bcm_gpio_lock, flags);
	gpio &= 31;
	val = readl(reg);
	if (enable)
		val |= (1 << gpio);
	else
		val &= ~(1 << gpio);
	writel(val, reg);
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);
	return 0;
}

/**
	Set specified GPIO pull up or set specified GPIO pull down.
	@param	gpio (in) GPIO number
	@param	up  Boolean, true to pull up, false to pull down
	@return	either EINVAL or zero.

	This function writes the GPIO specified signal (pull down/up) to the GPIO device.
	It returns 0 for success or a negative value if there is an error.
**/

int bcm_gpio_pull_up(unsigned int gpio, bool up)
{
	unsigned long flags;
	uint32_t val;
	void __iomem *reg = bcm_gpio_ports->base + GPIO_GPIPUD(gpio);
	if (!gpio_is_valid(gpio))
		return -EINVAL;
	spin_lock_irqsave(&bcm_gpio_lock, flags);
	gpio &= 31;
	val = readl(reg);
	if (up)
		val |= (1 << gpio);
	else
		val &= ~(1 << gpio);
	writel(val, reg);
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);
	return 0;
}

/**
	Get the direction(input/output) for the specified GPIO
	@param	gpio (in) GPIO number
	@return	either EINVAL or the GPIO direction

	This function gets the current direction mode set for the specified GPIO pin.
	It returns EINVAL if there is an error or successful return values are<br>
	0 - input pin without interrupt<br>
	1 - input pin with interrupt<br>
	2 - output pin<br>
	3 - not used
**/

int bcm_gpio_get_direction(unsigned int gpio)
{
	unsigned long flags;
	uint32_t val;
	void __iomem *reg = bcm_gpio_ports->base + GPIO_IOTR(gpio);

	if (!gpio_is_valid(gpio))
		return -EINVAL;
	spin_lock_irqsave(&bcm_gpio_lock, flags);
	gpio &= 15;
	val = readl(reg);
	val = (val >> (gpio << 1)) & 3;
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);
	return val;
}

/**
	Get specified GPIO IRQ type
	@param	gpio (in) GPIO number
	@return	either EINVAL or GPIO IRQ type

	This function gets the specified GPIP IRQ type.
	It returns EINVAL if there is an error or successful return GPIP IRQ type values are<br>
	0 Reserved, no interrupt triggered<br>
	1 Rising edge interrupt trigger<br>
	2 Falling edge interrupt trigger<br>
	3 Both Rising/Falling  edge interrupt trigger
**/
int bcm_gpio_get_irq_type(unsigned int gpio)
{
	unsigned long flags;
	uint32_t val;

	if (!gpio_is_valid(gpio))
		return -EINVAL;
	spin_lock_irqsave(&bcm_gpio_lock, flags);
	gpio &= 15;
	val = readl(bcm_gpio_ports->base + GPIO_GPITR(gpio));
	val = (val >> (gpio << 1)) & 3;
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);

	return val;
}

/**
	Get GPIO pull up/pull down
	@param	gpio (in) GPIO number
	@return	either EINVAL or true if GPIO is pulled up or false if its
	pulled down

	This function gets the pull up/down status of the specified GPIO number.
	It returns 0 for pull down, 1 for pull up or EINVAL on error.
**/

int bcm_gpio_get_pull_up_down(unsigned int gpio)
{
	unsigned long flags;
	uint32_t val;
	void __iomem *reg = bcm_gpio_ports->base + GPIO_GPIPUD(gpio);
	if (!gpio_is_valid(gpio))
		return -EINVAL;
	spin_lock_irqsave(&bcm_gpio_lock, flags);
	gpio &= 31;
	val = readl(reg);
	val = (val >> gpio) & 1;
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);
	return val;
}

/**
	Get the debounce value set for the specified GPIO value
	@param	gpio (in) GPIO number
	@return	either EINVAL or GPIO debounce value

	This function gets the debounce value set for the specified GPIO pin.
	It returns EINVAL if there is an error or successful return debounce values are<br>
	0 1 ms<br>
	1 2 ms<br>
	2 4 ms<br>
	3 8 ms<br>
	4 16 ms<br>
	5 32 ms<br>
	6 64 ms<br>
	7 128 ms

**/
int bcm_gpio_get_db_val(unsigned int gpio)
{
	unsigned long flags;
	uint32_t val;
	void __iomem *reg = bcm_gpio_ports->base + GPIO_GPDBR(gpio);

	if (!gpio_is_valid(gpio))
		return -EINVAL;

	spin_lock_irqsave(&bcm_gpio_lock, flags);
	gpio &= 7;
	val = readl(reg);

	val = (val >> (gpio << 2)) & 0xf;
	spin_unlock_irqrestore(&bcm_gpio_lock, flags);
	return val;
}

/** @} */

EXPORT_SYMBOL(bcm_gpio_set_db_val);
EXPORT_SYMBOL(bcm_gpio_pull_up_down_enable);
EXPORT_SYMBOL(bcm_gpio_pull_up);
EXPORT_SYMBOL(bcm_gpio_get_direction);
EXPORT_SYMBOL(bcm_gpio_get_irq_type);
EXPORT_SYMBOL(bcm_gpio_get_pull_up_down);
EXPORT_SYMBOL(bcm_gpio_get_db_val);

static struct irq_chip gpio_irq_chip = {
	.ack = gpio_irq_ack,
	.mask = gpio_irq_mask,
	.unmask = gpio_irq_unmask,
	.set_type = gpio_irq_set_type,
};

int32_t __init bcm_gpio_init(struct bcm_gpio_port *port)
{
	int32_t i;

	bcm_gpio_ports = port;
	writel(0, port->base + GPIO_GPIMR(0));
	writel(0, port->base + GPIO_GPIMR(32));
	for (i = GPIO_TO_IRQ(0); i < GPIO_TO_IRQ(ARCH_NR_GPIOS); i++) {
		set_irq_chip(i, &gpio_irq_chip);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
	}

	/* register gpio chip */
	port->chip.direction_input = bcm_gpio_direction_input;
	port->chip.direction_output = bcm_gpio_direction_output;
	port->chip.get = bcm_gpio_get;
	port->chip.set = bcm_gpio_set;
	port->chip.base = 0;
	port->chip.ngpio = 64;

	/* its a serious configuration bug when it fails */
	BUG_ON(gpiochip_add(&port->chip) < 0);

	set_irq_chained_handler(port->irq, gpio_isr_handler);
	set_irq_data(port->irq, port);
	set_irq_type(port->irq, IRQ_TYPE_LEVEL_HIGH);

	return 0;
}
