/* Copyright (C)
* 2010 - Broadcom
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

/**
* @file bcm_auxadc.c
* @brief This implements the driver for the auxiliary ADC on the BCM1161.
*/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
//#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <plat/bcm_auxadc.h>
#include <linux/broadcom/types.h>

#include "linux/broadcom/bcm_reset_utils.h"

#include "../../../../modules/drivers/char/brcm/fuse_ril/CAPI2_CIB/sysinterface/hal/adc/public/hal_adc.h"

#define CONFIG_BRCM_FUSE_RIL_CIB
/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
struct auxadc_info_t {
	struct semaphore gSem;
	struct clk *clk;
	void __iomem *auxadc_base;
	u16 readmask;
	int ready;
	int start;
	u16 croff;
	u16 rdoff;
	int auxpm;
	int regoff;
	int bgoff;
};

DECLARE_WAIT_QUEUE_HEAD(gAdcReadyQueue);
static struct auxadc_info_t auxadcinfo;

/* ---- Private Variables ------------------------------------------------ */

static char banner[] __initdata = KERN_INFO "Auxiliary ADC Driver:\n";

/* ---- Private Function Prototypes -------------------------------------- */

/* ---- Functions -------------------------------------------------------- */


int auxadc_access_through_ap(int regID)
{
	int rc;
	int adcOutput;
	u32 val;

	if ((regID < 0) || (regID > 4)) {
		pr_err("%s: invalid channel number: %d\n", __func__, regID);
		return -EINVAL;
	}

	if (down_interruptible(&auxadcinfo.gSem)) {
		pr_err("%s: adc busy\n", __func__);
		return -EINTR;
	}

	if (auxadcinfo.clk)
		clk_enable(auxadcinfo.clk);

	if (auxadcinfo.auxpm) {
		val = readl(auxadcinfo.auxadc_base + auxadcinfo.croff);
		val = val & ~((auxadcinfo.regoff) | (auxadcinfo.bgoff));
		writel(val, auxadcinfo.auxadc_base + auxadcinfo.croff);
	}

	/* start the appropriate channel based on the channel number */
	val = readl(auxadcinfo.auxadc_base + auxadcinfo.croff) & ~(0x7);
	val |= (regID | auxadcinfo.start);
	writel(val, auxadcinfo.auxadc_base + auxadcinfo.croff);

	/* wait for 10 ms for conversion to complete */
	rc = interruptible_sleep_on_timeout(&gAdcReadyQueue, 10 * HZ / 1000);

	/* read the ADC, and then let other processes use it */
	if ((readl(auxadcinfo.auxadc_base + auxadcinfo.rdoff)) &(auxadcinfo.ready)) {
		/* output ready, read result */
		adcOutput = (readl(auxadcinfo.auxadc_base + auxadcinfo.rdoff))& (auxadcinfo.readmask);
	} else {		/* output not ready, return 0 */
		pr_err("ADC ready not set\n");
		adcOutput = -EIO;
	}

	if (auxadcinfo.auxpm) {
		val = readl(auxadcinfo.auxadc_base + auxadcinfo.croff);
		val = val | ((auxadcinfo.regoff) | (auxadcinfo.bgoff));
		writel(val, auxadcinfo.auxadc_base + auxadcinfo.croff);
	}

	writel(0, auxadcinfo.auxadc_base + auxadcinfo.croff);
	up(&auxadcinfo.gSem);
	val = readl(auxadcinfo.auxadc_base + auxadcinfo.croff);

	if (auxadcinfo.clk)
		clk_disable(auxadcinfo.clk);

	pr_info("Channel %d output %d register 0x%x\n", regID, adcOutput, val);

	return adcOutput;

}

/**
* @brief Provide access to specified ADC channel.
*
* @param regID - Chaneel number to access
*
* @return ADC output
*/


#ifdef CONFIG_BRCM_FUSE_RIL_CIB
extern void SYSRPC_HAL_ADC_Ctrl(HAL_ADC_Action_en_t action, HAL_ADC_ReadConfig_st_t *req, HAL_ADC_ReadConfig_st_t *rsp);
int auxadc_access_through_cp(int regID)
{
	uint16_t data;
	HAL_ADC_Cb_Result_en_t result;
	HAL_ADC_Action_en_t action;
	HAL_ADC_ReadConfig_st_t req;
	

	if ((regID < 0) || (regID > 4)) {
		pr_err("%s: invalid channel number: %d\n", __func__, regID);
		return -EINVAL;
	}
	
	action = HAL_ADC_ACTION_GET_UNALIGN;

	req.adc_basic.ch = regID;
	req.adc_basic.pAdcData = &data;
	req.adc_basic.cntxt = 0;
	req.adc_mult.mrd_cnt = 0;
	req.adc_rf_algn.trg = HAL_ADC_TRG_UN_ALGND;
	req.adc_rf_algn.dly = 1;	
	SYSRPC_HAL_ADC_Ctrl(action, &req, &req);
	result = req.adc_basic.cb_res;
	if(result != HAL_ADC_CB_PASS)
	{
		printk("auxadc_access SYSRPC_HAL_ADC_Ctrl fail, result = %d\n",result);
		return -EAGAIN;
	}
	else	
		return data;

}

int auxadc_access(int regID)
{
	if(get_ap_boot_mode() != AP_ONLY_BOOT)
		return auxadc_access_through_cp(regID);
	else
		return auxadc_access_through_ap(regID);
}
#else
int auxadc_access(int regID)
{
	return auxadc_access_through_ap(regID);
}
#endif
EXPORT_SYMBOL(auxadc_access);

/**
* @brief Called to perform module initialization when the module is loaded.
*
* @param pdev
*
* @return Null
*/
static int __devinit auxadc_probe(struct platform_device *pdev)
{
	struct resource *adc_res = platform_get_resource(pdev,
							 IORESOURCE_MEM, 0);
	struct bcm_plat_auxadc *a =
	    (struct bcm_plat_auxadc *)pdev->dev.platform_data;

	auxadcinfo.auxadc_base = (void *__iomem)adc_res->start;
	auxadcinfo.readmask = a->readmask;
	auxadcinfo.ready = a->ready;
	auxadcinfo.start = a->start;
	auxadcinfo.croff = a->croff;
	auxadcinfo.rdoff = a->rdoff;
	/* Require power management. Only bondray */
	auxadcinfo.auxpm = a->auxpm;

	if (auxadcinfo.auxpm) {
		auxadcinfo.regoff = a->regoff;
		auxadcinfo.bgoff = a->bgoff;
	}

	auxadcinfo.clk = clk_get(&pdev->dev, "auxadc_dac_ref");
	if (IS_ERR(auxadcinfo.clk)) {
		pr_err("%s: auxadc_dac_ref clk not found\n", __func__);
		auxadcinfo.clk = NULL;
	}

	sema_init(&auxadcinfo.gSem, 1);
	pr_info("%s", banner);

	return 0;
}

/**
* @brief Called to perform module uninitialization when the module is unloaded.
*
* @param pdev
*
* @return Null
*/
static int __devexit auxadc_remove(struct platform_device *pdev)
{
	return 0;
}

struct platform_driver auxadc_driver = {
	.probe = auxadc_probe,
	.remove = __devexit_p(auxadc_remove),
	.driver = {
		   .name = "bcm_auxadc",
		   }
};

/**
* @brief Called to perform module initialization when the module is loaded
*
* @return Null
*/
static int __init auxadc_init(void)
{
	platform_driver_register(&auxadc_driver);
	return 0;
}				/* auxadc_init */

/**
* @brief Called to perform module cleanup when the module is unloaded.
*
* @return
*/
static void __exit auxadc_exit(void)
{
	platform_driver_unregister(&auxadc_driver);
}				/* auxadc_exit */

/****************************************************************************/

/* Changed from module_init to fs_initcall so that AUXADC driver
 * is loaded before the any of the PMU drivers is loaded. PMU drivers
 * were also changed to fs_initcall so that they are loaded before
 * VC02 and USB drivers are loaded. THis was done because the host has to
 * read the PMU interrupts in time (< 8sec) or else the PMU
 * timeout timer (of 8sec) could expire causing the phone to shut off.
 * This was observed in cases where a battery was removed and then re inserted.
 * This action would cause a LOWBAT interrupt generated and the host has 8sec
 * to clear it before PMU goes into standby mode. If VC02 driver was loaded
 * before PMU driver, the PMU driver was getting loaded well past 8sec window
 */

fs_initcall(auxadc_init);
module_exit(auxadc_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Auxiliary ADC Driver");
