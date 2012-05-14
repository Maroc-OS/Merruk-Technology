/*.......................................................................................................
. COPYRIGHT (C)  SAMSUNG Electronics CO., LTD (Suwon, Korea). 2009           
. All rights are reserved. Reproduction and redistiribution in whole or 
. in part is prohibited without the written consent of the copyright owner.
. 
.   Developer:
.   Date:
.   Description:  
..........................................................................................................
*/

#if !defined(_CAMACQ_PXA950_H_)
#define _CAMACQ_PXA950_H_

/* Include */
#include "camacq_type.h"

/* Global */
#undef GLOBAL

#if !defined(_CAMACQ_API_C_)
#define GLOBAL extern
#else
#define GLOBAL
#endif

GLOBAL int CAMACQ_CAM_POWER_SET( int iflag )
#if defined(_CAMACQ_API_C_)
{
#if 0
    struct clk *axi_clk = NULL;
    struct clk *sci1_clk = NULL;
    struct clk *sci2_clk = NULL;

    CamacqTraceIN();

    sci1_clk = clk_get(NULL, "SCI1CLK");
    if (IS_ERR(sci1_clk)) 
    {
        printk(KERN_ERR "unable to get SCI1CLK\n");
        return PTR_ERR(sci1_clk);
    }

    sci2_clk = clk_get(NULL, "SCI2CLK");
    if (IS_ERR(sci2_clk)) 
    {
        printk(KERN_ERR "unable to get SCI2CLK\n");
        return PTR_ERR(sci2_clk);
    }

    if( iflag )
    {
        axi_clk = clk_get(NULL, "AXICLK");
        if (IS_ERR(axi_clk)) 
        {
        printk(KERN_ERR "unable to get AXICLK\n");
        return PTR_ERR(axi_clk);
        }

        clk_enable(axi_clk);
        clk_enable(sci1_clk);
        clk_enable(sci2_clk);

        clk_put(axi_clk);
    }
    else
    {
        clk_disable(sci1_clk);
        clk_disable(sci2_clk);
    }

    clk_put(sci1_clk);
    clk_put(sci2_clk);

    CamacqTraceOUT();

    return 0;
#else
    static struct clk *axi_clk = NULL;
	static struct clk *sci1_clk = NULL;
	static struct clk *sci2_clk = NULL;
	static struct clk *csi_tx_esc = NULL;

	if (sci1_clk == NULL) {
		sci1_clk = clk_get(NULL, "SCI1CLK");
		if (IS_ERR(sci1_clk)) {
			printk(KERN_ERR "unable to get SCI1CLK\n");
			return PTR_ERR(sci1_clk);
		};
	};

	if (sci2_clk == NULL) {
		sci2_clk = clk_get(NULL, "SCI2CLK");
		if (IS_ERR(sci2_clk)) {
			printk(KERN_ERR "unable to get SCI2CLK\n");
			return PTR_ERR(sci2_clk);
		};
	};

	if (csi_tx_esc == NULL) {
		csi_tx_esc = clk_get(NULL, "CSI_TX_ESC");
		if (IS_ERR(csi_tx_esc)) {
			printk(KERN_ERR "unable to get CSI_TX_ESC\n");
			return PTR_ERR(csi_tx_esc);
		};
	};

	if (axi_clk == NULL) {
		axi_clk = clk_get(NULL, "AXICLK");
		if (IS_ERR(axi_clk)) {
			printk(KERN_ERR "unable to get AXICLK\n");
			return PTR_ERR(axi_clk);
		};
	};

	if ( iflag) {
		/* The order of enabling matters! AXI must be the 1st one */
		clk_enable(axi_clk);
		clk_enable(csi_tx_esc);
		clk_enable(sci1_clk);
		clk_enable(sci2_clk);
	} else {
		clk_disable(csi_tx_esc);
		clk_disable(sci2_clk);
		clk_disable(sci1_clk);
		clk_disable(axi_clk);
	};

	clk_put(sci1_clk);
	clk_put(sci2_clk);
	clk_put(csi_tx_esc);
	clk_put(axi_clk);

	return 0;
#endif
}
#endif /* _CAMACQ_API_C_ */
;

#undef GLOBAL
#endif /* _CAMACQ_PXA950_H_ */

