/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/char/broadcom/bcm_otp.c
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
*   @file   bcm_otp.c
*   @brief  This file defines the functions to read from BRCM SOC internal OTP
*
*****************************************************************************/

/**
*   @defgroup   BRCMOTPAPIGroup   OTP API's
*   @brief      This group defines the APIs to read from BRCM OTP
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <plat/bcm_otp.h>

static inline void otp_write_reg(u32 *base, u8 off, u32 val)
{
	u32 *temp;
	temp = (u32 *) ((u8 *) base + off);
	*temp = val;
}

static inline u32 otp_read_reg(u32 *base, u8 off)
{
	u32 *temp;
	temp = (u32 *) ((u8 *) base + off);
	return *temp;
}

struct otp_info {
	u32 *membase;
};

struct otp_info *otp;

static void assert_cmd_done(void);

static void assert_cmd_done()
{
	u32 temp;
	temp = otp_read_reg(otp->membase, OTP_STATUS);
	temp |= CMD_DONE;
	otp_write_reg(otp->membase, OTP_STATUS, temp);
}

/** @addtogroup BRCMOTPAPIGroup
	@{
*/

/**
* @brief otp_read_word: Reads one row of OTP data
*
* @param row: OTP row number to read from
* @param ecc: Whether ecc should be enabled or not
* @param rd:  pointer in which the read OTP data will be returned
*
* @return   0: Success, -EINVAL: Invalid parameter, -EFAULT: OTP device failure
*
* This function returns the data read from the given row and
* returns it on 'rd'.
* If ECC is enabled only the LSB 24 bits are valid.
* If ECC is disbaled then all the 32 bits are valid.
* In both the cases bit 31-29 indicate failed bits which are zero-ed by this
* function. For more details on OTP organization and data interpretation please
* refer the SOC Programmer's guide.
* Note that otp_probe should have been called before calling this function.
*
* Calling context: This function can be called from process,
* work queue, taskelt and interrupt context. Please be aware that
* there are udelays called from this function. Also if the OTP
* does not respond (only in theory) there is a possibility of getting
* into an infinite loop.
*/
int otp_read_word(int row, int ecc, u32 *rd)
{
	u32 temp = 0;
	u32 mask = 0;
	u32 addr;
	if ((row < 0) || (row > MAX_ROWS) || (rd == NULL))
		return -EINVAL;	/* Fail if row numbers are wrong */
	addr = (row << ADDR_SHIFT) & ADDR_MASK;
	otp_write_reg(otp->membase, OTP_ADDR, addr);
	/* Set access mode raw for all commands */
	temp = (0x0 << ACCESS_MODE_SHIFT) & ACCESS_MODE_MASK;
	otp_write_reg(otp->membase, OTP_CTRL, temp);
	temp &= ~(OTP_CMD_MASK);
	temp |= OTP_CMD_READ;
	if (ecc)
		temp |= (EN_ECC | EN_WREN);
	else
		temp &= ~(EN_ECC | EN_WREN);
	otp_write_reg(otp->membase, OTP_CTRL, temp);
	/* Done writing .. now wait */
	mask = CMD_DONE | DATA_RDY;
	temp = otp_read_reg(otp->membase, OTP_STATUS);
	while (mask != (temp & mask)) {
		udelay(2);
		temp = otp_read_reg(otp->membase, OTP_STATUS);
	}
	if (temp & ECC_COR)
		pr_info("Read Error Occured but corrected\n");
	if (temp & OTP_FAIL) {
		pr_err("Bad part found at row %d\n", row);
		return -EFAULT;
	}
	assert_cmd_done();
	*rd = otp_read_reg(otp->membase, OTP_RDATA);
	*rd &= 0x1FFFFFFF;	/* Ignore fail bits anyways */
	if (ecc)
		*rd &= RD_DATA_MASK;
	pr_info("Read data 0x%x at row %d\n", *rd, row);
	return 0;
}

/** @} */

EXPORT_SYMBOL(otp_read_word);

/** @addtogroup BRCMOTPAPIGroup
	@{
*/

/**
* @brief Probe function called to initialize the OTP Platform device
*
* @param pdev: Platform device structure
*
* @return  Returns 0 on success, -ENOMEM: mem allocation failure,
*         -1 on other errors
*
* Called during platform device initialization. This function also
* initializes and moves the OTP device to a known state, from which it will
* respond to read, write commands.
*/
static int __init otp_probe(struct platform_device *pdev)
{
	struct resource *res;
	u32 temp = 0;
	pr_info("Inside %s\n", __FUNCTION__);
	otp = (struct otp_info *)kmalloc(sizeof(struct otp_info), GFP_KERNEL);
	if (otp == NULL) {
		pr_err("Unable to allocate memory\n");
		return -ENOMEM;
	}
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {
		otp->membase = (void __iomem *)res->start;
	} else {
		return -1;
	}
	/* Reset OTP to start with */
	otp_write_reg(otp->membase, OTP_PC, RESET);
	mdelay(2);

	/* Enable OTP and set CPU mode enable */
	temp = (OTP_ENABLE | OTP_CPU_MODE_EN);

	/* Enable LOTP clock divider */
	temp |= (LOTP_CLK_DIV_EN | LOTP_CLK_DIV_RATIO);
	otp_write_reg(otp->membase, OTP_PC, temp);

	/* Enable ECC, ECC computation by HW and set continue on Fail */
	temp = (EN_ECC | EN_WREN | CONTINUE_ON_FAIL);
	otp_write_reg(otp->membase, OTP_CTRL, temp);	/* How to know if fuses were written with ECC ? */
	temp &= ~(OTP_CMD_MASK);
	temp |= OTP_CMD_INIT;
	otp_write_reg(otp->membase, OTP_CTRL, temp);

	/* Check if command execution is done */
	while (!(otp_read_reg(otp->membase, OTP_STATUS) & CMD_DONE)) {
		udelay(10);
	}

	assert_cmd_done();
	pr_info("Finished INIT of OTP module\n");
	return 0;

}

/** @} */

static int otp_remove(struct platform_device *pdev)
{
	kfree(otp);
	return 0;
}

static struct platform_driver bcm_otp_driver = {
	.probe = otp_probe,
	.remove = otp_remove,
	.driver = {
		   .name = "otp",
		   .owner = THIS_MODULE,
		   },
};

static int __init otp_init(void)
{
	return platform_driver_register(&bcm_otp_driver);
}

static void __exit otp_exit(void)
{
	return platform_driver_unregister(&bcm_otp_driver);
}

arch_initcall(otp_init);
module_exit(otp_exit);

MODULE_AUTHOR("BROADCOM");
MODULE_DESCRIPTION("BROADCOM OTP");
MODULE_LICENSE("GPL");
