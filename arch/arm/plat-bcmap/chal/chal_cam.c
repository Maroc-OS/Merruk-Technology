/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
//============================================================================
//!
//! \file   chal_cam.c
//! \brief  CAM cHAL layer
//! \note  
//!
//============================================================================
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <plat/osdal_os.h>
#include <plat/chal/chal_common_os.h>
#include <plat/chal/chal_cam.h>
#include <plat/rdb/brcm_rdb_sysmap.h>
#define CHIP_REVISION 20
#if defined(_ATHENA_) && (CHIP_REVISION==20)
#include <plat/rdb/brcm_rdb_camintf.h>
#else
#include <plat/rdb/brcm_rdb_cam.h>
#endif
#include <plat/rdb/brcm_rdb_util.h>

//===========================================================================
// local macro declarations
//
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
    #define DBG_OUT(x) x
#else
   #define DBG_OUT(x) 
#endif
//#define  DISPLAY_CHAL_CAM_STATUS    

#define MIN_FRAME_RATE  100                // Minimum Camera Video Frame Rate (10Fps)

typedef struct chal_cam_s {
    cBool   init;
    UInt32	baseAddr;                       // base address of registers
} chal_cam_t, *p_chal_cam_t;

//*****************************************************************************
// Local Variables
//*****************************************************************************
static chal_cam_t cam_dev[1] = 
{ 
    {0}
};

//*****************************************************************************
// Local Functions
//*****************************************************************************
//SETs REGISTER BIT FIELD; VALUE IS 0 BASED 
#define CAM_REG_FIELD_SET(r,f,d)        \
    ( ((BRCM_REGTYPE(r))(d) << BRCM_FIELDSHIFT(r,f) ) & BRCM_FIELDMASK(r,f) )
//GETs REGISTER BIT FIELD; VALUE IS 0 BASED 
#define CAM_REG_FIELD_GET(r,f,d)        \
    ( ((BRCM_REGTYPE(r))(d) & BRCM_FIELDMASK(r,f) ) >> BRCM_FIELDSHIFT(r,f) )

//GETs REGISTER BITs Defined WITH MASK
#define CAM_REG_WRITE_MASKED(b,r,m,d)   \
    ( BRCM_WRITE_REG(b,r,(BRCM_READ_REG(b,r) & (~m)) | d) )

//SETs REGISTER BITs Defined WITH MASK
#define CAM_REG_WRITE_MASKED(b,r,m,d)   \
    ( BRCM_WRITE_REG(b,r,(BRCM_READ_REG(b,r) & (~m)) | d) )

// ---- Private Function Prototypes -----------------------------------------
 static UInt32 chal_cam_wait_value(CHAL_HANDLE handle, CHAL_CAM_INTF_t intf, UInt32 timeout, UInt32 *reg_addr, UInt32 reg_mask, UInt32 value);

// ---- Functions -----------------------------------------------------------

u32 chal_rbc_disp(CHAL_HANDLE handle)
{
	chal_cam_t *pCamDevice = (chal_cam_t *)handle;
	u32 rbcx = 0;
	rbcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RBC0 );
	//printk("RBC0 0x%x\n",rbcx);
	return rbcx;
}
EXPORT_SYMBOL(chal_rbc_disp);

u32 chal_rdr3_disp(CHAL_HANDLE handle)
{
	chal_cam_t *pCamDevice = (chal_cam_t *)handle;
	u32 rdr3 = 0;
	rdr3 = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RDR3);
	//pr_debug("RBC0 0x%x\n",rbcx);
	return rdr3;
}
EXPORT_SYMBOL(chal_rdr3_disp);


CHAL_CAM_STATUS_CODES chal_cam_register_display(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 rc, rcx, rpcx;
    cUInt32 rs, rdls, rsx, rsax, reax, rwpx, rbcx, rlsx;
    cUInt32 rgsp, tren, lpds, pcp, rp;
    cUInt32 thssto, thsset, thsckto, srst;
    cUInt32 rdr1,rdr2,rdr3,rdr4;
    cUInt32 rdsax, rdeax, rdsx, dtovx;
    cUInt32 cpis, cpir, cpif, cpiw;
    cUInt32 cpiwvc, cpiwvs, cpiwhc, cpiwhs;


// Register Output
    if (param->intf == CHAL_CAM_INTF_CSI)
    {
    // CSI2 Receiver control
        rc = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RC );
    // CSI2 Receiver status
        rs = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RS );
    // CSI2 Receiver status
        rdls = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RDLS );
        printk(KERN_INFO "chal_cam_status(): CSI2RC=0x%x CSI2RS=0x%x CSI2RDLS=0x%x \r\n", rc, rs, rdls);

    // CSI2 Receiver control
        rgsp = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RGSP );
    // CSI2 Receiver status
        tren = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2TREN );
    // CSI2 Receiver status
        lpds = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2LPDS );
    // CSI2 Channel 0 control
        pcp = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2PCP );
        printk(KERN_INFO "chal_cam_status(): CSI2RGSPC=0x%x CSI2TREN=0x%x CSI2LPDS=0x%x CSI2PCP=0x%x \r\n", rgsp, tren, lpds, pcp);

    // CSI2 Receiver Sync Timeout
        thssto = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2THSSTO );
    // CSI2 Receiver HS Settle MaskTimer
        thsset = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2THSSET );
    // CSI2 Receiver HS Clock Timeout
        thsckto = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2THSCKTO );
    // CSI2 Receiver Analog Reset
        srst = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2SRST );
        printk(KERN_INFO "chal_cam_status(): CSI2THSSTO=0x%x CSI2THSSET=0x%x CSI2THSCKTO=0x%x CSI2SRST=0x%x \r\n", thssto, thsset, thsckto, srst);
    
        if (param->chan & CHAL_CAM_CHAN_0)
        {
       // CSI2 Channel 0 control
            rcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RC0 );
            rsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RS0 );
            printk(KERN_INFO "chal_cam_status(): CSI2RC0=0x%x CSI2RS0=0x%x \r\n", rcx, rsx);
    
            rsax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RSA0 );
            reax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2REA0 );
            rwpx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RWP0 );
            rbcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RBC0 );
            rlsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RLS0 );
            printk(KERN_INFO "chal_cam_status(): CSI2RSA0=0x%x CSI2REA0=0x%x CSI2RWP0=0x%x CSI2RBC0=0x%x CSI2RLS0=0x%x \r\n", rsax, reax, rwpx, rbcx, rlsx);
    
            rdsax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RDSA0 );
            rdeax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RDEA0 );
            rdsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RDS0 );
            dtovx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2DTOV0 );
            printk(KERN_INFO "chal_cam_status(): CSI2RDSA0=0x%x CSI2RDEA0=0x%x CSI2RDS0=0x%x CSI2DTOV0=0x%x \r\n", rdsax, rdeax, rdsx, dtovx);
        }
        if (param->chan & CHAL_CAM_CHAN_1)
        {
        // CSI2 Channel control
            rcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RC1 );
            rsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RS1 );
            printk(KERN_INFO "chal_cam_status(): CSI2RC1=0x%x CSI2RS1=0x%x \r\n", rcx, rsx);
    
            rsax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RSA1 );
            reax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2REA1 );
            rwpx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RWP1 );
            rbcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RBC1 );
            rlsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RLS1 );
            dtovx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2DTOV1 );
            printk(KERN_INFO "chal_cam_status(): CSI2RSA1=0x%x CSI2REA1=0x%x CSI2RWP1=0x%x CSI2RBC1=0x%x CSI2RLS1=0x%x CSI2DTOV1=0x%x \r\n", rsax, reax, rwpx, rbcx, rlsx, dtovx);
    
        // CSI2 Channel 0 status
            rdsax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RDSA1 );
        // CSI2 Channel 0 status
            rdeax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RDEA1 );
        // CSI2 Channel 0 status
            rdsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CSI2RDS1 );
            printk(KERN_INFO "chal_cam_status(): CSI2RDSA1=0x%x CSI2RDEA1=0x%x CSI2RDS1=0x%x \r\n", rdsax, rdeax, rdsx);
        }

    }
    else if ( (param->intf == CHAL_CAM_INTF_CCP) || (param->intf == CHAL_CAM_INTF_CPI) )
    {
    // CCP2 Receiver control
        rc = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RC );
    // CCP2 Receiver status
        rs = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RS );
    // CCP2 Receiver Panic
        rp = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RP );
        printk(KERN_INFO "CCP2RC=0x%x CCP2RS=0x%x CCP2RP=0x%x  \r\n", rc, rs, rp);

        if (param->chan & CHAL_CAM_CHAN_0)
        {
        // CCP2 Channel 0 control
            rcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RC0 );
        // CCP2 Channel 0 status
            rsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RS0 );
        // CCP2 Pipeline Control status
            rpcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RPC0 );
            printk(KERN_INFO "CCP Rx0: CCP2RC0=0x%x CCP2RS0=0x%x CCP2RPC0=0x%x \r\n", rcx, rsx, rpcx);

            rsax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RSA0 );
            reax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2REA0 );
            rwpx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RWP0 );
            rbcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RBC0 );
            rlsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RLS0 );
            printk(KERN_INFO "CCP Rx0: CCP2RSA0=0x%x CCP2REA0=0x%x CCP2RWP0=0x%x CCP2RBC0=0x%x CCP2RLS0=0x%x \r\n", rsax, reax, rwpx, rbcx, rlsx);
    
            rdsax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDSA0 );
            rdeax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDEA0 );
            rdsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDS0 );
            printk(KERN_INFO "CCP Rx0: CCP2RDSA0=0x%x CCP2RDEA0=0x%x CCP2RDS0=0x%x \r\n", rdsax, rdeax, rdsx);
        }
        if (param->chan & CHAL_CAM_CHAN_1)
        {
        // CCP2 Channel 1 control
            rcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RC1 );
        // CCP2 Channel 1 status
            rsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RS1 );
        // CCP2 Pipeline Control status
            rpcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RPC1 );
            printk(KERN_INFO "CCP Rx1: CCP2RC1=0x%x CCP2RS1=0x%x CCP2RPC1=0x%x \r\n", rcx, rsx, rpcx);

            rsax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RSA1 );
            reax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2REA1 );
            rwpx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RWP1 );
            rbcx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RBC1 );
            rlsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RLS1 );
            printk(KERN_INFO "CCP Rx1: CCP2RSA1=0x%x CCP2REA1=0x%x CCP2RWP1=0x%x CCP2RBC1=0x%x CCP2RLS1=0x%x \r\n", rsax, reax, rwpx, rbcx, rlsx);
    
            rdsax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDSA1 );
            rdeax = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDEA1 );
            rdsx = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDS1 );
            printk(KERN_INFO "CCP Rx1: CCP2RDSA1=0x%x CCP2RDEA1=0x%x CCP2RDS1=0x%x \r\n", rdsax, rdeax, rdsx);
        }
        if (param->intf == CHAL_CAM_INTF_CPI)  
        {
        // CPI control status
            cpis = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CPIS );
        // CPI Resync status
            cpir = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CPIR );
        // CPI Framing Status
            cpif = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CPIF );
        // CPI Windowing status
            cpiw = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CPIW );
            printk(KERN_INFO "\nCPI Status: CPIS=0x%x CPIR=0x%x CPIF=0x%x CPIW=0x%x \r\n\n", cpis, cpir, cpif, cpiw);
        // CPI Windowing status
            cpiwvc = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CPIWVC );
            cpiwvs = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CPIWVS );
            cpiwhc = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CPIWHC );
            cpiwhs = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CPIWHS );
            printk(KERN_INFO "CPI Window: CPIWVC=0x%x CPIWVS=0x%x CPIWHC=0x%x CPIWHS=0x%x \r\n", cpiwvc, cpiwvs, cpiwhc, cpiwhs);
        }

    // CCP2 Debug 1 status
        rdr1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDR1 );
    // CCP2 Debug 2 status
        rdr2 = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDR2 );
    // CCP2 Debug 3 status
        rdr3 = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDR3 );
    // CCP2 Debug 4 status
        rdr4 = BRCM_READ_REG ( pCamDevice->baseAddr, CAMINTF_CCP2RDR4 );
        printk(KERN_INFO "CAM_CCP2RDR1=0x%x CAM_CCP2RDR2=0x%x CAM_CCP2RDR3=0x%x CAM_CCP2RDR4=0x%x \r\n", rdr1, rdr2, rdr3, rdr4);
        
    }
    return chal_status;
}


//***************************************************************************
/**
*       chal_cam_wait_value, wait for mode change
*/
static UInt32 chal_cam_wait_value(CHAL_HANDLE handle, CHAL_CAM_INTF_t intf, UInt32 timeout, UInt32 *reg_addr, UInt32 reg_mask, UInt32 value)
{
    UInt32 register_value;

    register_value = *reg_addr & reg_mask;
    while ( (timeout != 0) && (register_value != value) )
    {
        CHAL_DELAY_MS(1);        
        register_value = *reg_addr & reg_mask;
        timeout--;
    } 

    if (timeout == 0)
    {
        DBG_OUT( chal_dprintf(CDBG_INFO, "chal_cam_wait_value(): ERROR: addr = 0x%x value_read=0x%x value_set=0x%x \r\n", reg_addr,register_value,value) );
    }
    else
    {
        register_value = 0x00000000;
    }
    return register_value;
}


//==============================================================================
//! \brief 
//!		   	Initialize CHAL CAM for the passed CAM instance
//! \note 
//!        	Parameters:   
//!				baseAddr, mapped address of this CAM instance
//!        	Return:      
//!				CHAL handle for this CAM instance
//==============================================================================
CHAL_HANDLE chal_cam_init(cUInt32 baseAddr)
{
	chal_cam_t *pCamDevice=NULL;

    printk(KERN_INFO "chal_cam_init\n");

// Check for Null handle
#if 0
    if (baseAddr != CAM_BASE_ADDR)
    {
        chal_dprintf(CDBG_ERRO, "ERROR: chal_cam_init: bad base addr\n");
        baseAddr = CAM_BASE_ADDR;
    }
#endif    
// We need Virtual address for linux 

// Assign handle
    pCamDevice = &cam_dev[0];

// Check if init already
    if( pCamDevice->init != TRUE )
    {
        pCamDevice->baseAddr = baseAddr;
        pCamDevice->init = TRUE;
    }
    else
    {
        DBG_OUT( chal_dprintf(CDBG_INFO, "chal_cam_init: already initialized\n") );
    }         
    
    DBG_OUT( chal_dprintf(CDBG_INFO, "chal_cam_init: Register base=0x%x \n", pCamDevice->baseAddr) );
	printk(KERN_INFO"Returning correct CHAL device\n");
	return (CHAL_HANDLE)pCamDevice;
}


//==============================================================================
//! \brief 
//!		   	De-Initialize CHAL CAM for the passed CAM instance
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
cVoid chal_cam_deinit(CHAL_HANDLE handle)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;

    if (pCamDevice == NULL)
    {
        DBG_OUT( chal_dprintf(CDBG_INFO, "chal_cam_deinit: already NULL handle\n") );
    }
    else
    {
        pCamDevice->baseAddr = NULL;
        pCamDevice->init = FALSE;
    }
}

//==============================================================================
//! \brief 
//!		   	Configure CAM module Control Settings
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_intf(CHAL_HANDLE handle, CHAL_CAM_CFG_INTF_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    cInt32 data_shift;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    CHAL_CAM_CPI_CNTRL_CFG_st_t*    p_cpi_cfg;

// Set CPI Configuration Pointer
    printk(KERN_INFO"CHAL handle 0x%x\n",pCamDevice);
    p_cpi_cfg = (CHAL_CAM_CPI_CNTRL_CFG_st_t*)cfg->p_cpi_cfg_st; 

    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
        printk(KERN_INFO "chal_cam_cfg_intf: CHAL_CAM_INTF_CSI \n");	
    // Reset Cam 
    printk(KERN_INFO"CHAL Cam base addr 0x%x offset 0x%x\n",pCamDevice->baseAddr, CAMINTF_CSI2RC0_OFFSET);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC0,0x00000000);
    printk(KERN_INFO"CHAL Cam base addr 0x%x offset 0x%x\n",pCamDevice->baseAddr, CAMINTF_CSI2RC0_OFFSET);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RPC0,0x00000000);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2DTOV0,0x00000000);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC1,0x00000000);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RPC1,0x00000000);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2DTOV1,0x00000000);
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC);
        reg |= (CAMINTF_CSI2RC_BPD_MASK|CAMINTF_CSI2RC_APD_MASK);
        reg &= ~(CAMINTF_CSI2RC_GEN_MASK||CAMINTF_CSI2RC_LENC_MASK|CAMINTF_CSI2RC_LEN1_MASK|CAMINTF_CSI2RC_LEN2_MASK);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC,reg);
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2SRST,SRST,1);
    // Init Rx
            if (cfg->input_mode == CHAL_CAM_INPUT_DUAL_LANE)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSET,HSMC,32);
            }
            else
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSET,HSMC,16);
            }
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC);
        // Default Receiver Priority
            reg |= (0x0C << CAMINTF_CSI2RC_RPP_SHIFT);
        // Default Receiver BandGap
            reg |= (0x07 << CAMINTF_CSI2RC_PTATADJ_SHIFT);
            reg |= (0x07 << CAMINTF_CSI2RC_CTATADJ_SHIFT);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC,reg);
        // Termination Override
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2TREN,0x00000000);
        // Termination Delay
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSET,TD,0);
        // Sync Timeout
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSTO,THSSTO,0);
        // HS Clock Timeout
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSCKTO,THSCKTO,0);
        // Set Image Override
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2DTOV0,IMEN,1);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2DTOV0,IMDT,0x00);
    }
    else
    {
        printk(KERN_INFO "chal_cam_cfg_intf: CHAL_CAM_INTF_CCP/CHAL_CAM_INTF_CPI \n");	
    // Reset Cam 
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC0,0x00000000);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RPC0,0x00000000);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC1,0x00000000);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RPC1,0x00000000);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIS,0x00000000);
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC);
        reg |= (CAMINTF_CCP2RC_BPD_MASK|CAMINTF_CCP2RC_APD_MASK|CAMINTF_CCP2RC_SWR_MASK); //haipeng
        reg &= ~(CAMINTF_CCP2RC_RXEN_MASK|CAMINTF_CCP2RC_CLKM_MASK|CAMINTF_CCP2RC_INMP_MASK|CAMINTF_CCP2RC_TREN_MASK);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC,reg);

		//Add a little bit delay for protection
		udelay(1000);
		BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,SWR,1);
    // Init Rx
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC);
        // Disable Analog Power
            reg |= (CAMINTF_CCP2RC_BPD_MASK|CAMINTF_CCP2RC_APD_MASK);
        // Set Cam Control Register Defaults
            // Default Receiver Analog Levels
                reg &= ~(CAMINTF_CCP2RC_PTATADJ_MASK|CAMINTF_CCP2RC_CTATADJ_MASK|CAMINTF_CCP2RC_CLAC_MASK|CAMINTF_CCP2RC_DLAC_MASK|CAMINTF_CCP2RC_CLK_IDR_MASK|CAMINTF_CCP2RC_DATA_IDR_MASK);
                reg |= (0x07 << CAMINTF_CCP2RC_PTATADJ_SHIFT);
                reg |= (0x07 << CAMINTF_CCP2RC_CTATADJ_SHIFT);
                reg |= (0x07 << CAMINTF_CCP2RC_CLAC_SHIFT);
                reg |= (0x07 << CAMINTF_CCP2RC_DLAC_SHIFT);
                reg |= (0x06 << CAMINTF_CCP2RC_CLK_IDR_SHIFT);
                reg |= (0x06 << CAMINTF_CCP2RC_DATA_IDR_SHIFT);
        // CCP Mode
            if (cfg->intf == CHAL_CAM_INTF_CCP)
            {
                printk(KERN_INFO "chal_cam_cfg_intf:  CCP Mode\n");	
            // Clk Mode
                if (cfg->clk_mode == CHAL_CAM_DATA_CLOCK)
                {
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RC,CLKM,1);
                }
            // Enable Termination
                reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RC,TREN,1);
            }
            else
        // CPI Mode
            {
                reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RC,INMP,1);
                printk(KERN_INFO "chal_cam_cfg_intf:  CPI Mode\n");	
            }
    // Update Cam Control Register
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC,reg);

    // Default Receiver Priority
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RP);
            reg &= ~(CAMINTF_CCP2RP_RPT_MASK | CAMINTF_CCP2RP_RRP_MASK | CAMINTF_CCP2RP_RNP_MASK);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RP,RPT,0x00);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RP,RRP,0x0C);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RP,RNP,0x01);
        reg = BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RP,reg);
    // CPI Mode
        if ( (cfg->intf == CHAL_CAM_INTF_CPI) && (p_cpi_cfg != NULL) )
        {
        // Reset CPI Peripheral        
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIS,0xC0000000);
        // Enable CPI Peripheral        
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,ENB,0x01);
       
            if ( p_cpi_cfg != NULL )
            {
            // Set CPIF Register
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CPIF);
                // Sync Mode
                    reg &= ~(CAMINTF_CPIF_SMODE_MASK);
                    if (p_cpi_cfg->sync_mode == CHAL_CAM_SYNC_EMBEDDED)
                    {
                        reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,SMODE,1);
                    }
                // VSync Mode
                    reg &= ~(CAMINTF_CPIF_VMODE_MASK);
                    switch (p_cpi_cfg->vsync_signal)
                    {
                        case CHAL_CAM_SYNC_START_DATA:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,VMODE,1);
                            break;
                        case CHAL_CAM_SYNC_DEFINES_SYNC:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,VMODE,2);
                            break;
                        case CHAL_CAM_SYNC_IGNORED:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,VMODE,3);
                            break;
                        case CHAL_CAM_SYNC_DEFINES_ACTIVE:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,VMODE,0);
                            break;
                    }
                // HSync Mode
                    reg &= ~(CAMINTF_CPIF_HMODE_MASK);
                    switch (p_cpi_cfg->hsync_signal)
                    {
                        case CHAL_CAM_SYNC_START_DATA:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,HMODE,1);
                            break;
                        case CHAL_CAM_SYNC_DEFINES_SYNC:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,HMODE,2);
                            break;
                        case CHAL_CAM_SYNC_IGNORED:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,HMODE,3);
                            break;
                        case CHAL_CAM_SYNC_DEFINES_ACTIVE:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,HMODE,0);
                            break;
                    }
                // Field Mode
                    reg &= ~(CAMINTF_CPIF_FMODE_MASK);
                    switch (p_cpi_cfg->fmode)
                    {
                        case CHAL_CAM_FIELD_EMBEDDED:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,FMODE,1);
                            break;
                        case CHAL_CAM_FIELD_EVEN:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,FMODE,2);
                            break;
                        case CHAL_CAM_FIELD_ODD:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,FMODE,3);
                            break;
                        case CHAL_CAM_FIELD_H_V:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,FMODE,0);
                            break;
                    }
            // Update CPIF Control Register
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIF,reg);
        
            // Set CPIR Register
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CPIR);
                // Input Bit Shift (+)=Left shift, (-)=Right shift
                    reg &= ~(CAMINTF_CPIR_BITSHIFT_MASK);
                    if (p_cpi_cfg->data_shift > 0)
                    {
                        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_intf:  Left Data Shift Not Allowed:\n");	
                        chal_status |= CHAL_OP_INVALID;
                        data_shift = 0;
                    }
                    else
                    {
                        data_shift = 0 - p_cpi_cfg->data_shift;
                    }
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITSHIFT,data_shift);
                        
                // Input Bit Width
                    reg &= ~(CAMINTF_CPIR_BITWIDTH_MASK);
                    switch (p_cpi_cfg->bit_width)
                    {
                        case CHAL_CAM_PIXEL_10BIT:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,1);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,1);
                            break;
                        case CHAL_CAM_PIXEL_12BIT:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,2);
                            break;
                        case CHAL_CAM_PIXEL_16BIT:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,3);
                            break;
                        case CHAL_CAM_PIXEL_8BIT:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,0);
                            break;
                    }
                // VSync Trigger
                    reg &= ~(CAMINTF_CPIR_VSRM_MASK | CAMINTF_CPIR_VSAL_MASK);
                    switch (p_cpi_cfg->vsync_trigger)
                    {
                        case CHAL_CAM_SYNC_EDGE_NEG:
                        case CHAL_CAM_SYNC_ACTIVE_LOW:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,VSRM,1);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,VSAL,1);
                            break;
                        case CHAL_CAM_SYNC_EDGE_POS:
                        case CHAL_CAM_SYNC_ACTIVE_HIGH:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,VSRM,0);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,VSAL,0);
                           break;
                    }
                // HSync Trigger
                    reg &= ~(CAMINTF_CPIR_HSRM_MASK | CAMINTF_CPIR_HSAL_MASK);
                    switch (p_cpi_cfg->hsync_trigger)
                    {
                        case CHAL_CAM_SYNC_EDGE_NEG:
                        case CHAL_CAM_SYNC_ACTIVE_LOW:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,HSRM,1);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,HSAL,1);
                            break;
                        case CHAL_CAM_SYNC_EDGE_POS:
                        case CHAL_CAM_SYNC_ACTIVE_HIGH:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,HSRM,0);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,HSAL,0);
                            break;
                    }
                // Clock Edge
                    reg &= ~(CAMINTF_CPIR_DSRM_MASK);
                    switch (p_cpi_cfg->clock_edge)
                    {
                        case CHAL_CAM_SYNC_EDGE_NEG:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,DSRM,1);
                            break;
                        case CHAL_CAM_SYNC_EDGE_POS:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,DSRM,0);
                            break;
                    }
            // Update CPIR Control Register
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIR,reg);
            }
            else
        // CPI Mode NULL POINTER
            {
                DBG_OUT( chal_dprintf( CDBG_ERRO, "chal_cam_cfg_intf:  CPI Mode NULL CFG pointer\n") );	
            }
        }
    }
    return chal_status;
}    
    
    

//==============================================================================
//! \brief 
//!		   	Configure CAM module Control Settings
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_cntrl(CHAL_HANDLE handle, CHAL_CAM_CFG_CNTRL_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;


    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
        printk(KERN_INFO "chal_cam_cfg_cntrl: CHAL_CAM_INTF_CSI \n");	
    // Sync Timeout
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSTO,THSSTO,cfg->packet_timeout);
    }
    else if (cfg->intf == CHAL_CAM_INTF_CCP)
    {
        printk(KERN_INFO "chal_cam_cfg_cntrl: CHAL_CAM_INTF_CCP/CHAL_CAM_INTF_CPI \n" );	
    // Panic Priority
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RP,RRP,cfg->panic_pr);
    // Normal Priority
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RP,RNP,cfg->norm_pr);
    // Panic Threshold
        if (cfg->panic_enable)
        {
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RP,RPT,cfg->panic_thr);
        }
        else
        {
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RP,RPT,0x00);
        }
    }
    return chal_status;
}    


//==============================================================================
//! \brief 
//!		   	Configure CAM Channel pipeline
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_pipeline(CHAL_HANDLE handle, CHAL_CAM_PIPELINE_CFG_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    
    printk(KERN_INFO "chal_cam_cfg_pipeline\n");	

    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
    // Set CSI2RPCx
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            printk(KERN_INFO "chal_cam_cfg_pipeline:  CHAL_CAM_INTF_CSI: CHAL_CAM_CHAN_0\n");
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RPC0);
        }
        else
        {
            printk(KERN_INFO "chal_cam_cfg_pipeline:  CHAL_CAM_INTF_CSI: CHAL_CAM_CHAN_1\n" );
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RPC1);
        }
        // Encode Block Length
            reg &= ~(CAMINTF_CSI2RPC0_EBL_MASK);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,EBL,cfg->enc_blk_lnth);
        // Encode Advanced Predictor
            reg &= ~(CAMINTF_CSI2RPC0_EAP_MASK);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,EAP,cfg->enc_predictor);
        // Encode Processing
            reg &= ~(CAMINTF_CSI2RPC0_EP_MASK);
            switch (cfg->enc_proc)
            {
                case CHAL_CAM_10_8_DPCM_ENC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,EP,3);
                    break;
                case CHAL_CAM_12_8_DPCM_ENC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,EP,4);
                    break;
                default:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,EP,0);
                    break;
            }
        // Encode Pixel packing
            switch (cfg->enc_pixel_pack)
            {
                case CHAL_CAM_PIXEL_10BIT:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,EP,1);
                    break;
                case CHAL_CAM_PIXEL_12BIT:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,EP,2);
                    break;
                case CHAL_CAM_PIXEL_8BIT:
                default:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,EP,0);
                    break;
            }
        // Decode Advanced Predictor
            reg &= ~(CAMINTF_CSI2RPC0_DAP_MASK);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DAP,cfg->dec_predictor);
        // Decode Processing
            reg &= ~(CAMINTF_CSI2RPC0_DP_MASK);
            switch (cfg->dec_proc)
            {
                case CHAL_CAM_6_10_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,3);
                    break;
                case CHAL_CAM_6_12_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,4);
                    break;
                case CHAL_CAM_7_10_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,5);
                    break;
                case CHAL_CAM_7_12_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,6);
                    break;
                case CHAL_CAM_8_10_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,7);
                    break;
                case CHAL_CAM_8_12_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,8);
                    break;
                case CHAL_CAM_FSP_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,9);
                    break;
                default:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,0);
                    break;
            }
        // Decode Pixel Unpacking
            switch (cfg->dec_pixel_unpack)
            {
                case CHAL_CAM_PIXEL_10BIT:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,1);
                    break;
                case CHAL_CAM_PIXEL_12BIT:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,2);
                    break;
                case CHAL_CAM_PIXEL_8BIT:
                default:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2RPC0,DP,0);
                    break;
            }
        // Update CSI2RPCx
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RPC0,reg);
        }
        else
        {
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RPC1,reg);
        }
    }
    else if (cfg->intf == CHAL_CAM_INTF_CCP)
    {
    // Set CCP2RPCx
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            printk(KERN_INFO "chal_cam_cfg_pipeline:  CHAL_CAM_INTF_CCP: CHAL_CAM_CHAN_0\n");
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RPC0);
        }
        else
        {
            printk(KERN_INFO "chal_cam_cfg_pipeline:  CHAL_CAM_INTF_CCP: CHAL_CAM_CHAN_1\n");
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RPC1);
        }
        // Encode Block Length
            reg &= ~(CAMINTF_CCP2RPC0_EBL_MASK);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,EBL,cfg->enc_blk_lnth);
        // Encode Advanced Predictor
            reg &= ~(CAMINTF_CCP2RPC0_EAP_MASK);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,EAP,cfg->enc_predictor);
        // Encode Processing
            reg &= ~(CAMINTF_CCP2RPC0_EP_MASK);
            switch (cfg->enc_proc)
            {
                case CHAL_CAM_10_8_DPCM_ENC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,EP,4);
                    break;
                case CHAL_CAM_12_8_DPCM_ENC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,EP,5);
                    break;
                default:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,EP,0);
                    break;
            }
        // Encode Pixel packing
            switch (cfg->enc_pixel_pack)
            {
                case CHAL_CAM_PIXEL_16BIT:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,EP,1);
                    break;
                case CHAL_CAM_PIXEL_10BIT:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,EP,2);
                    break;
                case CHAL_CAM_PIXEL_8BIT:
                default:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,EP,0);
                    break;
            }
        // Decode Advanced Predictor
            reg &= ~(CAMINTF_CCP2RPC0_DAP_MASK);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DAP,cfg->dec_predictor);
        // Decode Processing
            reg &= ~(CAMINTF_CCP2RPC0_DP_MASK);
            switch (cfg->dec_proc)
            {
                case CHAL_CAM_6_10_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,3);
                    break;
                case CHAL_CAM_6_12_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,4);
                    break;
                case CHAL_CAM_7_10_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,5);
                    break;
                case CHAL_CAM_7_12_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,6);
                    break;
                case CHAL_CAM_8_10_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,7);
                    break;
                case CHAL_CAM_8_12_DPCM_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,8);
                    break;
                case CHAL_CAM_FSP_DEC:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,9);
                    break;
                default:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,0);
                    break;
            }
        // Decode Pixel Unpacking
            switch (cfg->dec_pixel_unpack)
            {
                case CHAL_CAM_PIXEL_10BIT:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,1);
                    break;
                case CHAL_CAM_PIXEL_12BIT:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,2);
                    break;
                case CHAL_CAM_PIXEL_8BIT:
                default:
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RPC0,DP,0);
                    break;
            }
        // Update CCP2RPCx
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RPC0,reg);
        }
        else
        {
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RPC1,reg);
        }
    }
    else if (cfg->intf == CHAL_CAM_INTF_CPI)
    {
        printk(KERN_INFO "chal_cam_cfg_pipeline:  CHAL_CAM_INTF_CPI\n");	
    // Set CCP2RPC0
        printk(KERN_INFO "chal_cam_cfg_pipeline:  CHAL_CAM_INTF_CCP: CHAL_CAM_CHAN_0\n");
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RPC0);
        // Encode Block Length = 0
            reg &= ~(CAMINTF_CCP2RPC0_EBL_MASK);
        // Encode Advanced Predictor
            reg &= ~(CAMINTF_CCP2RPC0_EAP_MASK);
        // Encode Processing = 8-bit Pixel
            reg &= ~(CAMINTF_CCP2RPC0_EP_MASK);
        // Decode Advanced Predictor
            reg &= ~(CAMINTF_CCP2RPC0_DAP_MASK);
        // Decode Processing = 8-bit pixel
            reg &= ~(CAMINTF_CCP2RPC0_DP_MASK);
        // Update CCP2RPC0
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RPC0,reg);
    }
    else
    {
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_pipeline:  Interface Undefined\n");	
        chal_status = CHAL_OP_INVALID;
    }
    return chal_status;
}        


//==============================================================================
//! \brief 
//!		   	Configure CAM Channel x for Receive data
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_buffer(CHAL_HANDLE handle, CHAL_CAM_BUFFER_CFG_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;

// CSI Interface
    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
    // Channel 0
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
        // Image Update
            if (cfg->buffers.image0Buff != NULL)
            {
                printk(KERN_INFO "chal_cam_cfg_buffer:  CHAL_CAM_INTF_CSI: CHAL_CAM_CHAN_0: CHAL_CAM_IMAGE\n");
            // Image Start Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RSA0,RSA,cfg->buffers.image0Buff->start_addr);
            // Image End Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2REA0,REA,(cfg->buffers.image0Buff->start_addr + cfg->buffers.image0Buff->size));
            // Image line stride
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RLS0,RLS,cfg->buffers.image0Buff->line_stride);
            }
            
        // Data Update
            if (cfg->buffers.dataBuff != NULL)
            {
            // Data Start Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RDSA0,RDSA,cfg->buffers.dataBuff->start_addr);
            // Data End Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RDEA0,RDEA,(cfg->buffers.dataBuff->start_addr + cfg->buffers.dataBuff->size));
            // Data line stride
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RDS1,RDS,cfg->buffers.dataBuff->line_stride);
            }
        }
        else
        {
    // Channel 1
        // Image Update
            if (cfg->buffers.image0Buff != NULL)
            {
                printk(KERN_INFO "chal_cam_cfg_buffer:  CHAL_CAM_INTF_CSI: CHAL_CAM_CHAN_1: CHAL_CAM_IMAGE\n");
            // Image Start Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RSA1,RSA,cfg->buffers.image0Buff->start_addr);
            // Image End Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2REA1,REA,(cfg->buffers.image0Buff->start_addr + cfg->buffers.image0Buff->size));
            // Image line stride
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RLS1,RLS,cfg->buffers.image0Buff->line_stride);
            }
            
        // Data Update
            if (cfg->buffers.dataBuff != NULL)
            {
            // Data Start Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RDSA1,RDSA,cfg->buffers.dataBuff->start_addr);
            // Data End Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RDEA1,RDEA,(cfg->buffers.dataBuff->start_addr + cfg->buffers.dataBuff->size));
            // Data line stride
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RDS1,RDS,cfg->buffers.dataBuff->line_stride);
            }
        }
    }
// CCP/CPI Interface
    else if ( (cfg->intf == CHAL_CAM_INTF_CCP) || (cfg->intf == CHAL_CAM_INTF_CPI) )
    {
    // Channel 0
        if ( (cfg->chan == CHAL_CAM_CHAN_0) || (cfg->intf == CHAL_CAM_INTF_CPI) )
        {
        // Image Update
            if (cfg->buffers.image0Buff != NULL)
            {
            // Image Start Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RSA0,SA,cfg->buffers.image0Buff->start_addr);
            // Image End Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2REA0,EA,(cfg->buffers.image0Buff->start_addr + cfg->buffers.image0Buff->size));
            // Image line stride
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RLS0,LSTR,cfg->buffers.image0Buff->line_stride);
            }
        // Data Update
            if (cfg->buffers.dataBuff != NULL)
            {
            // Data Start Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RDSA0,DSA,cfg->buffers.dataBuff->start_addr);
            // Data End Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RDEA0,DEA,(cfg->buffers.dataBuff->start_addr + cfg->buffers.dataBuff->size));
            // Data line stride
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RDS0,DSTR,cfg->buffers.dataBuff->line_stride);
            }
        }
        else
        {
    // Channel 1
        // Image Update
            if (cfg->buffers.image0Buff != NULL)
            {
            // Image Start Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RSA1,SA,cfg->buffers.image0Buff->start_addr);
            // Image End Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2REA1,EA,(cfg->buffers.image0Buff->start_addr + cfg->buffers.image0Buff->size));
            // Image line stride
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RLS1,LSTR,cfg->buffers.image0Buff->line_stride);
            }
        // Data Update
            if (cfg->buffers.dataBuff != NULL)
            {
            // Data Start Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RDSA1,DSA,cfg->buffers.dataBuff->start_addr);
            // Data End Addr
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RDEA1,DEA,(cfg->buffers.dataBuff->start_addr + cfg->buffers.dataBuff->size));
            // Data line stride
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RDS1,DSTR,cfg->buffers.dataBuff->line_stride);
            }
        }
    }
    else
    {
        printk( "ERROR:  chal_cam_cfg_buffer:  Interface Undefined\n");	
        chal_status |= CHAL_OP_INVALID;
    }
// No Buffer Wrap
    if (cfg->buffers.image0Buff != NULL)
    {
        if (cfg->buffers.image0Buff->buf_wrap_enable)
        {
            printk("ERROR:  chal_cam_cfg_buffer:  buf_wrap_enable Undefined\n");	
            chal_status |= CHAL_OP_INVALID;
        }
    }
    else {
        return CHAL_OP_INVALID_PARAMETER;
    }
    
#if (CHIP_REVISION >= 20)
    if (cfg->buffers.image1Buff != NULL )
    {
        UInt32 offsetAddr = cfg->buffers.image1Buff->start_addr - cfg->buffers.image0Buff->start_addr;
        printk(KERN_INFO "chal_cam_cfg_buffer:  PingPong buffer configuration offsetaddr 0x%x chal status %d\n",offsetAddr,chal_status);
        
        if (cfg->buffers.image1Buff->start_addr < cfg->buffers.image0Buff->start_addr)
        {
            printk("ERROR:  chal_cam_cfg_buffer: image1.start_addr(0x%x) is less than image0.start_addr(0x%x)\n", cfg->buffers.image1Buff->start_addr, cfg->buffers.image0Buff->start_addr);
            chal_status |= CHAL_OP_INVALID_PARAMETER;            
        }
        
        if ( offsetAddr & 0x3FF )
        {
            printk(KERN_INFO"ERROR:  chal_cam_cfg_buffer: offset(0x%x) is not 10 bit aligned \n", offsetAddr);
            chal_status |= CHAL_OP_INVALID_PARAMETER;                        
        }

        offsetAddr >>= 10;
        if (cfg->intf == CHAL_CAM_INTF_CSI)
        {            
            printk(KERN_INFO "chal_cam_cfg_buffer:  Configure CSI2 PingPong buffer offset(0x%x)\n", offsetAddr);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PPMGMT,PP_OFFSET_ADDR,offsetAddr);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PPMGMT,FORCE_HW_RDY,1);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PPMGMT,CAPTURE_EN,0);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PPMGMT,PINGPON_EN,1); 
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PPMGMT,CAPTURE_EN,1);                           
            
            //printk(KERN_INFO "chal_cam_cfg_buffer:  CAM_CSI2PPMGMT: 0x%0X\n", *(UInt32 *)0x08440400);
            
        }
        else
        {
            // CPI and CCP2
            printk(KERN_INFO "chal_cam_cfg_buffer:  Configure CCP2 PingPong buffer offset(0x%x)\n", offsetAddr);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2PPMGMT,PP_OFFSET_ADDR,offsetAddr);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2PPMGMT,FORCE_HW_RDY,1);             
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2PPMGMT,CAPTURE_EN,0);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2PPMGMT,PINGPONG_EN,1); 
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2PPMGMT,CAPTURE_EN,1);                           
            
           // printk(KERN_INFO "chal_cam_cfg_buffer:  CAM_CCP2PPMGMT: 0x%0X\n", *(UInt32 *)0x08440B00 );
        
        }
    }
    else
    {
	printk("Disabling HW ping-pong\n");
         // Disable pingpong buffer for single buffer usage.
         BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2PPMGMT,CAPTURE_EN,0);
         BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2PPMGMT,PINGPONG_EN,0); 
         BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PPMGMT,PINGPON_EN,0); 
         BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PPMGMT,CAPTURE_EN,0);         
    }
#endif
    
    return chal_status;
}    


//==============================================================================
//! \brief 
//!		   	Configure CAM Channel x for Receive data
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_data(CHAL_HANDLE handle, CHAL_CAM_DATA_CFG_st_t* cfg)
{
//    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;

    chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_data:  Not Available\n");	
    chal_status |= CHAL_OP_INVALID;
    return chal_status;
}    


//==============================================================================
//! \brief 
//!		   	Get CAM x Configuration for Receive Data
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (out) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_get_buffer_cfg(CHAL_HANDLE handle, CHAL_CAM_BUFFER_CFG_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    UInt32 bytes;
    
    printk(KERN_INFO "chal_cam_get_buffer_cfg\n");	

// CSI Interface
    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
    // Channel 0
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            if (cfg->buffers.image0Buff != NULL)
            {
            // Image case
                    printk(KERN_INFO "chal_cam_get_cfg_data:  Chan 0: CHAL_CAM_IMAGE\n" );
                // Image Start Addr
                    cfg->buffers.image0Buff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RSA0);
                // Image End Addr
                    cfg->buffers.image0Buff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2REA0) - cfg->buffers.image0Buff->start_addr;
                // Image line stride
                    cfg->buffers.image0Buff->line_stride = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RLS0);
                // Image buffer wrap enable
                   cfg->buffers.image0Buff->buf_wrap_enable = 0;
                // Image write pointer
                    cfg->write_ptr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RWP0);
                // Image bytes/line
                    bytes = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RBC0);
                    cfg->bytes_per_line = cfg->buffers.image0Buff->line_stride;
                    cfg->lines_per_frame = cfg->buffers.image0Buff->size/cfg->buffers.image0Buff->line_stride;
                    if (cfg->buffers.image0Buff->line_stride != 0)
                    {
                        cfg->lines_per_frame = cfg->buffers.image0Buff->size/cfg->buffers.image0Buff->line_stride;
                        if (bytes != 0)
                        {                            
                            cfg->lines_per_frame = bytes/cfg->buffers.image0Buff->line_stride;
                        }
                    }
             }

               if (cfg->buffers.dataBuff != NULL)   
               {
            // Data case
                    printk(KERN_INFO "chal_cam_get_cfg_data: Chan 0: CHAL_CAM_DATA\n" );
                // Data Start Addr
                    cfg->buffers.dataBuff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDSA0);
                // Data End Addr
                    cfg->buffers.dataBuff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDEA0) - cfg->buffers.dataBuff->start_addr;
                // line stride
                   cfg->buffers.dataBuff->line_stride = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDS0);
                // Data buffer wrap enable
                    cfg->buffers.dataBuff->buf_wrap_enable = 0;
                // Data write pointer
                    cfg->write_ptr = cfg->buffers.dataBuff->start_addr;
                // bytes/line
                    cfg->bytes_per_line = cfg->buffers.dataBuff->line_stride;
                // lines/frame
                    cfg->lines_per_frame = cfg->buffers.dataBuff->size;
                    if (cfg->buffers.dataBuff->line_stride != 0)
                    {
                        cfg->lines_per_frame = cfg->buffers.dataBuff->size/cfg->buffers.dataBuff->line_stride;
                    }
                 
            }
        }
    // Channel 1
        else
        {
               if (cfg->buffers.image0Buff != NULL)  
            {
                    printk(KERN_INFO "chal_cam_get_cfg_data: Chan 1: CHAL_CAM_IMAGE\n");
                // Image Start Addr
                    cfg->buffers.image0Buff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RSA1);
                // Image End Addr
                    cfg->buffers.image0Buff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2REA1) - cfg->buffers.image0Buff->start_addr;
                // Image line stride
                    cfg->buffers.image0Buff->line_stride = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RLS1);
                // Image buffer wrap enable
                    cfg->buffers.image0Buff->buf_wrap_enable = 0;
                // Image write pointer
                    cfg->write_ptr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RWP1);
                // Image bytes/line
                    bytes = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RBC1);
                    cfg->bytes_per_line = cfg->buffers.image0Buff->line_stride;
                    cfg->lines_per_frame = cfg->buffers.image0Buff->size/cfg->buffers.image0Buff->line_stride;
                    if (cfg->buffers.image0Buff->line_stride != 0)
                    {
                        cfg->lines_per_frame = cfg->buffers.image0Buff->size/cfg->buffers.image0Buff->line_stride;
                        if (bytes != 0)
                        {                            
                            cfg->lines_per_frame = bytes/cfg->buffers.image0Buff->line_stride;
                        }
                    }
               }
               
               if (cfg->buffers.dataBuff != NULL)  
               {
                    printk(KERN_INFO "chal_cam_get_cfg_data: Chan 1: CHAL_CAM_DATA\n");
                // Data Start Addr
                    cfg->buffers.dataBuff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDSA1);
                // Data End Addr
                    cfg->buffers.dataBuff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDEA1) - cfg->buffers.dataBuff->start_addr;
                // line stride
                    cfg->buffers.dataBuff->line_stride = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDS1);
                // Data buffer wrap enable
                    cfg->buffers.dataBuff->buf_wrap_enable = 0;
                // Data write pointer
                    cfg->write_ptr = cfg->buffers.dataBuff->start_addr;
                // bytes/line
                    cfg->bytes_per_line = cfg->buffers.dataBuff->line_stride;
                // lines/frame
                    cfg->lines_per_frame = cfg->buffers.dataBuff->size;
                    if (cfg->buffers.dataBuff->line_stride != 0)
                    {
                        cfg->lines_per_frame = cfg->buffers.dataBuff->size/cfg->buffers.dataBuff->line_stride;
                    }
            }
        }
    }
// CCP/CPI Interface
    else if ( (cfg->intf == CHAL_CAM_INTF_CCP) || (cfg->intf == CHAL_CAM_INTF_CPI) )
    {
    // Channel 0
        if ( (cfg->chan == CHAL_CAM_CHAN_0) || (cfg->intf == CHAL_CAM_INTF_CPI) )
        {
                if (cfg->buffers.image0Buff != NULL)  
            {
                    printk(KERN_INFO "chal_cam_get_cfg_data:  Chan 0: CHAL_CAM_IMAGE\n");
                // Image Start Addr
                    cfg->buffers.image0Buff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RSA0);
                // Image End Addr
                    cfg->buffers.image0Buff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2REA0) - cfg->buffers.image0Buff->start_addr;
                // Image line stride
                    cfg->buffers.image0Buff->line_stride = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RLS0);
                // Image buffer wrap enable
                    cfg->buffers.image0Buff->buf_wrap_enable = 0;
                // Image write pointer
                    cfg->write_ptr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RWP0);
                // Image bytes/line
                    bytes = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RBC0);
                    cfg->bytes_per_line = cfg->buffers.image0Buff->line_stride;
                    cfg->lines_per_frame = cfg->buffers.image0Buff->size/cfg->buffers.image0Buff->line_stride;
                    if (cfg->buffers.image0Buff->line_stride != 0)
                    {
                        cfg->lines_per_frame = cfg->buffers.image0Buff->size/cfg->buffers.image0Buff->line_stride;
                        if (bytes != 0)
                        {                            
                            cfg->lines_per_frame = bytes/cfg->buffers.image0Buff->line_stride;
                        }
                    }
           }
                
           if (cfg->buffers.image0Buff != NULL)  
           {
                    printk(KERN_INFO "chal_cam_get_cfg_data: Chan 0: CHAL_CAM_DATA\n" );
                // Data Start Addr
                    cfg->buffers.dataBuff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RDSA0);
                // Data End Addr
                    cfg->buffers.dataBuff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RDEA0) - cfg->buffers.dataBuff->start_addr;
                // line stride
                    cfg->buffers.dataBuff->line_stride = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RDS0);
                // Data buffer wrap enable
                    cfg->buffers.dataBuff->buf_wrap_enable = 0;
                // Data write pointer
                    cfg->write_ptr = cfg->buffers.dataBuff->start_addr;
                // bytes/line
                    cfg->bytes_per_line = cfg->buffers.dataBuff->line_stride;
                // lines/frame
                    cfg->lines_per_frame = cfg->buffers.dataBuff->size;
                    if (cfg->buffers.dataBuff->line_stride != 0)
                    {
                        cfg->lines_per_frame = cfg->buffers.dataBuff->size/cfg->buffers.dataBuff->line_stride;
                    }
            }
        }
    // Channel 1
        else
        {
                if (cfg->buffers.image0Buff != NULL)  
            {
                    printk(KERN_INFO "chal_cam_get_cfg_data:  Chan 1: CHAL_CAM_IMAGE\n");
                // Image Start Addr
                    cfg->buffers.image0Buff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RSA1);
                // Image End Addr
                    cfg->buffers.image0Buff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2REA1) - cfg->buffers.image0Buff->start_addr;
                // Image line stride
                    cfg->buffers.image0Buff->line_stride = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RLS1);
                // Image buffer wrap enable
                    cfg->buffers.image0Buff->buf_wrap_enable = 0;
                // Image write pointer
                    cfg->write_ptr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RWP1);
                // Image bytes/line
                    bytes = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RBC1);
                    cfg->bytes_per_line = cfg->buffers.image0Buff->line_stride;
                    cfg->lines_per_frame = cfg->buffers.image0Buff->size/cfg->buffers.image0Buff->line_stride;
                    if (cfg->buffers.image0Buff->line_stride != 0)
                    {
                        cfg->lines_per_frame = cfg->buffers.image0Buff->size/cfg->buffers.image0Buff->line_stride;
                        if (bytes != 0)
                        {                            
                            cfg->lines_per_frame = bytes/cfg->buffers.image0Buff->line_stride;
                        }
                    }
                }

                
                if (cfg->buffers.dataBuff != NULL)  
                {
                    printk(KERN_INFO "chal_cam_get_cfg_data: Chan 1: CHAL_CAM_DATA\n");
                // Data Start Addr
                    cfg->buffers.dataBuff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RDSA1);
                // Data End Addr
                    cfg->buffers.dataBuff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RDEA1) - cfg->buffers.dataBuff->start_addr;
                // line stride
                    cfg->buffers.dataBuff->line_stride = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RDS1);
                // Data buffer wrap enable
                    cfg->buffers.dataBuff->buf_wrap_enable = 0;
                // Data write pointer
                    cfg->write_ptr = cfg->buffers.dataBuff->start_addr;
                // bytes/line
                    cfg->bytes_per_line = cfg->buffers.dataBuff->line_stride;
                // lines/frame
                    cfg->lines_per_frame = cfg->buffers.dataBuff->size;
                    if (cfg->buffers.dataBuff->line_stride != 0)
                    {
                        cfg->lines_per_frame = cfg->buffers.dataBuff->size/cfg->buffers.dataBuff->line_stride;
                    }
            }
        }
    }            
    else
    {
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_get_buffer_cfg:  Interface Undefined\n");	
        chal_status |= CHAL_OP_INVALID;
    }
    return chal_status;
}    


//==============================================================================
//! \brief 
//!		   	Configure CAM Channel x for Receive
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_frame(CHAL_HANDLE handle, CHAL_CAM_FRAME_CFG_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    
    printk(KERN_INFO "chal_cam_cfg_frame\n");	

// CSI Interface
    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
    // Get Channel Control Register
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC0);
            printk(KERN_INFO "chal_cam_cfg_frame:  CHAL_CAM_INTF_CSI: CHAL_CAM_CHAN_0 reg=0x%x\n",reg );
        }
        else
        {
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC1);
            printk(KERN_INFO "chal_cam_cfg_frame:  CHAL_CAM_INTF_CSI: CHAL_CAM_CHAN_1 reg=0x%x\n",reg );
        }
            reg &= ~(CAMINTF_CSI2RC0_FSIE_MASK | CAMINTF_CSI2RC0_FEIE_MASK | CAMINTF_CSI2RC0_LSIE_MASK | CAMINTF_CSI2RC0_LEIE_MASK | CAMINTF_CSI2RC0_LCIE_MASK | CAMINTF_CSI2RC0_GSPIE_MASK);
            // Interrupt Setting
                if (cfg->interrupts & CHAL_CAM_INT_FRAME_START)
                {
                    reg |= (1 << CAMINTF_CSI2RC0_FSIE_SHIFT);
                }
                if (cfg->interrupts & CHAL_CAM_INT_FRAME_END)
                {
                    reg |= (1 << CAMINTF_CSI2RC0_FEIE_SHIFT);
                }
                if (cfg->interrupts & CHAL_CAM_INT_LINE_START)
                {
                    reg |= (1 << CAMINTF_CSI2RC0_LSIE_SHIFT);
                }
                if (cfg->interrupts & CHAL_CAM_INT_LINE_END)
                {
                    reg |= (1 << CAMINTF_CSI2RC0_LEIE_SHIFT);
                }
                if (cfg->interrupts & CHAL_CAM_INT_LINE_COUNT)
                {
                    reg |= (cfg->line_count << CAMINTF_CSI2RC0_LCIE_SHIFT);
                }
                if (cfg->interrupts & CHAL_CAM_INT_PKT)
                {
                    reg |= (1 << CAMINTF_CSI2RC0_GSPIE_SHIFT);
                }
            // Frame Capture Setting
            #if 0
                if (cfg->mode == CHAL_CAM_SHOT_MODE_SINGLE)
                {
                    reg |= (1 << CAM_ICTL_FCM_SHIFT);
                }
            #endif
    // Update Channel Control Register
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            reg = BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC0,reg);
        }
        else
        {
            reg = BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC1,reg);
        }
    }
// CCP/CPI Interface
    else if ( (cfg->intf == CHAL_CAM_INTF_CCP) || (cfg->intf == CHAL_CAM_INTF_CPI) )
    {
    // Set CCP2RCx
        if ( (cfg->chan == CHAL_CAM_CHAN_0) || (cfg->intf == CHAL_CAM_INTF_CPI) )
        {
            printk(KERN_INFO "chal_cam_cfg_frame:  CHAL_CAM_INTF_CCP: CHAL_CAM_CHAN_0: intr=0x%x line_count=%d \n",cfg->interrupts,cfg->line_count );
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC0);
        }
        else
        {
            printk(KERN_INFO "chal_cam_cfg_frame:  CHAL_CAM_INTF_CCP: CHAL_CAM_CHAN_1\n" );
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC1);
        }
            // Interrupt Setting
                reg &= ~(CAMINTF_CCP2RC0_FSIE_MASK | CAMINTF_CCP2RC0_FEIE_MASK | CAMINTF_CCP2RC0_LSIE_MASK | CAMINTF_CCP2RC0_LEIE_MASK | CAMINTF_CCP2RC0_LCIE_MASK);
                if (cfg->interrupts & CHAL_CAM_INT_FRAME_START)
                {
                    reg |= (1 << CAMINTF_CCP2RC0_FSIE_SHIFT);
                }
                if (cfg->interrupts & CHAL_CAM_INT_FRAME_END)
                {
                    reg |= (1 << CAMINTF_CCP2RC0_FEIE_SHIFT);
                }
                if (cfg->interrupts & CHAL_CAM_INT_LINE_START)
                {
                    reg |= (1 << CAMINTF_CCP2RC0_LSIE_SHIFT);
                }
                if (cfg->interrupts & CHAL_CAM_INT_LINE_END)
                {
                    reg |= (1 << CAMINTF_CCP2RC0_LEIE_SHIFT);
                }
                if (cfg->interrupts & CHAL_CAM_INT_LINE_COUNT)
                {
                    reg |= (cfg->line_count << CAMINTF_CCP2RC0_LCIE_SHIFT);
                }
            // Capture Mode
                reg &= ~(CAMINTF_CCP2RC0_FCM_MASK);
                if (cfg->mode == CHAL_CAM_SHOT_MODE_SINGLE)
                {
                    reg |= (1 << CAMINTF_CCP2RC0_FCM_SHIFT);
                }
    // Update CCP2RPx
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC0,reg);
        }
        else
        {
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC1,reg);
        }

      // CPI Settings             
        if (cfg->intf == CHAL_CAM_INTF_CPI)
        {
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CPIS);
        // Capture Mode
            reg &= ~(CAMINTF_CPIS_SHOT_MASK);
            if (cfg->mode == CHAL_CAM_SHOT_MODE_SINGLE)
            {
                reg |= (1 << CAMINTF_CPIS_SHOT_SHIFT);
            }
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIS,reg);
        }
    }
    else
    {
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_frame:  Interface Undefined\n");	
        chal_status = CHAL_OP_INVALID;
    }

    return chal_status;
}    



//==============================================================================
//! \brief 
//!		   	Configure CAM Image Id's for Receive
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_image_id(CHAL_HANDLE handle, CHAL_CAM_IMAGE_ID_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;

// CSI Interface
    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
    // Get Channel Data Type Override Register
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            printk(KERN_INFO "chal_cam_cfg_image_id:  CHAL_CAM_INTF_CSI: CHAL_CAM_CHAN_0\n" );
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2DTOV0);
        }
        else
        {
            printk(KERN_INFO "chal_cam_cfg_image_id:  CHAL_CAM_INTF_CSI: CHAL_CAM_CHAN_1\n" );
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2DTOV1);
        }
            reg &= ~(CAMINTF_CSI2DTOV0_IMEN_MASK | CAMINTF_CSI2DTOV0_IMDT_MASK);
            if (cfg->image_data_id_0 & CHAL_CAM_DATA_TYPE_MASK)
            {
                reg |= ( ((cfg->image_data_id_0 & CHAL_CAM_DATA_TYPE_MASK) >> CHAL_CAM_DATA_TYPE_SHIFT) << CAMINTF_CSI2DTOV0_IMDT_SHIFT );
                reg |= (1 << CAMINTF_CSI2DTOV0_IMEN_SHIFT);
            }
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
        // Set Image Override
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2DTOV0,reg);
        // Set Virtual Channel
           // reg = (cfg->image_data_id_0 & CHAL_CAM_VIRTUAL_CHANNEL_MASK) >> CHAL_CAM_VIRTUAL_CHANNEL_SHIFT;
            //BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC0,VC,reg);
        }
        else
        {
        // Set Image Override
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2DTOV1,reg);
        // Set Virtual Channel
            //reg = (cfg->image_data_id_0 & CHAL_CAM_VIRTUAL_CHANNEL_MASK) >> CHAL_CAM_VIRTUAL_CHANNEL_SHIFT;
            //BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC1,VC,reg);
        }
    }
// CCP Interface
    else 
    {
    // Set Logical Channel Number
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            printk(KERN_INFO "chal_cam_cfg_image_id:  CHAL_CAM_INTF_CCP: CHAL_CAM_CHAN_0\n" );
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC0);
        }
        else
        {
            printk(KERN_INFO "chal_cam_cfg_image_id:  CHAL_CAM_INTF_CCP: CHAL_CAM_CHAN_1\n" );
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC1);
        }
    // Set the Logical Channel #
            reg |= ( ((cfg->image_data_id_0 & CHAL_CAM_DATA_TYPE_MASK) >> CHAL_CAM_DATA_TYPE_SHIFT) << CAMINTF_CCP2RC0_LCN_SHIFT );
    // Update Logical Channel Number
        if (cfg->chan == CHAL_CAM_CHAN_0)
        {
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC0,reg);
        }
        else
        {
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC1,reg);
        }
    }
    return chal_status;
}    
    
    


//==============================================================================
//! \brief 
//!		   	Configure CAM Channel x windowing
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_window(CHAL_HANDLE handle, CHAL_CAM_WINDOW_CFG_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    
    printk(KERN_INFO "chal_cam_cfg_window\n" );	

// Windowing CPI Interface Only
    if (cfg->intf == CHAL_CAM_INTF_CPI)
    {
    // De-Activate Peripheral PipeLine, Enable CPI Peripheral for register writes
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CPIS);
        reg &= ~(CAMINTF_CPIS_ACT_MASK);
        reg |= (1 << CAMINTF_CPIS_ENB_SHIFT);      
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIS,reg);
        reg = chal_cam_wait_value(handle, cfg->intf, MIN_FRAME_RATE, (UInt32 *)(pCamDevice->baseAddr+CAMINTF_CPIS_OFFSET), (UInt32)CAMINTF_CPIS_DACT_MASK, (UInt32)0x00000000);
        if ( reg != 0 )
        {
            chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_window: CAM_CPIS_DACT: CAMINTF_CPIS_OFFSET=0x%x\n", reg);	
        }

    // Field Gating
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIW,FGATE,cfg->field_gating);

    // Windowing
        if (cfg->enable)
        {    
        // Set Horizontal Window
            reg = CAM_REG_FIELD_SET(CAMINTF_CPIWHC,HFRONT,cfg->h_start_sample);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIWHC,HACTIVE,cfg->h_end_sample);
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIWHC,reg);
        // Set Vertical Window
            reg = CAM_REG_FIELD_SET(CAMINTF_CPIWVC,VFRONT,cfg->v_start_sample);
            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIWVC,VACTIVE,cfg->v_end_sample);
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIWVC,reg);
        // Enable Windowing
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIW,ENB,1);
        }
        else
        {    
        // Disable Windowing
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIW,ENB,0);
            reg = 0;
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIWHC,reg);
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIWVC,reg);
        }
    }
    else
// CSI,CCP2 Interface
    {
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_window:  Operation Invalid\n");	
        chal_status = CHAL_OP_INVALID;
    }
    return chal_status;
}    


//==============================================================================
//! \brief 
//!		   	 Configure CAM module AFE Controller
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_afe_cntrl(CHAL_HANDLE handle, CHAL_CAM_AFE_CFG_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg;

    switch (cfg->intf)
    {
    // CSI Set Analog Power
        case CHAL_CAM_INTF_CSI:
            printk(KERN_INFO "chal_cam_cfg_afe_cntrl:  CHAL_CAM_INTF_CSI = %d\n", cfg->bandgap_bias );	
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC);
            reg &= ~(CAMINTF_CSI2RC_PTATADJ_MASK | CAMINTF_CSI2RC_CTATADJ_MASK);
            // Analog Bandgap Bias Control
                reg |= (cfg->bandgap_bias << CAMINTF_CSI2RC_PTATADJ_SHIFT);
                reg |= (cfg->bandgap_bias << CAMINTF_CSI2RC_CTATADJ_SHIFT);
                if (cfg->data_lane_disable == 1)
                {
                    reg &= ~(CAMINTF_CSI2RC_LEN1_MASK | CAMINTF_CSI2RC_LEN2_MASK);
                }
            reg = BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC,reg);
            break;
    // CCP Set Analog Power
        case CHAL_CAM_INTF_CCP:
            printk(KERN_INFO "chal_cam_cfg_afe_cntrl:  CHAL_CAM_INTF_CCP = %d\n", cfg->bandgap_bias );	
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC);
            reg &= ~(CAMINTF_CCP2RC_PTATADJ_MASK | CAMINTF_CCP2RC_CTATADJ_MASK);
            // Analog Bandgap Bias Control
                reg |= (cfg->bandgap_bias << CAMINTF_CCP2RC_PTATADJ_SHIFT);
                reg |= (cfg->bandgap_bias << CAMINTF_CCP2RC_CTATADJ_SHIFT);
            reg = BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC,reg);
            break;
    // CPI Set Analog Power
        case CHAL_CAM_INTF_CPI:
        default:
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC);
            reg |= (CAMINTF_CCP2RC_APD_MASK |  CAMINTF_CCP2RC_BPD_MASK);
            reg = BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC,reg);
            printk(KERN_INFO "chal_cam_cfg_afe_cntrl:  CPI Interface\n" );	
            break;
    }
    return chal_status;
}    



//==============================================================================
//! \brief 
//!		   	CAM module Set Analog Power
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       (unused)
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            (unused)
//!				param.select    (in) cfg select             
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_set_analog_pwr(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg;
    
    printk(KERN_INFO "chal_cam_set_analog_pwr = %d\n", param->param );	


    switch (param->intf)
    {
    // CSI Set Analog Power
        case CHAL_CAM_INTF_CSI:
            printk(KERN_INFO "chal_cam_set_analog_pwr:  CHAL_CAM_INTF_CSI = %d\n", param->param );	
        // if Power OFF, Check if channels enabled        
            if (param->param == FALSE)
            {
            // if Power OFF, Check if channels enabled        
                if ( (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LENC) == 1) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LEN1) == 1) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LEN2) == 1))
                {
                    printk(KERN_INFO "chal_cam_set_analog_pwr: OFF: Channels Enabled\n" );	
                    chal_status |= CHAL_OP_WRONG_ORDER;
                }
            // Reset Analog Receiver
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2SRST,ARST,1);
            // Analog Power Down
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,APD,1);
                if ( (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC0,CHEN)) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC1,CHEN)) )
                {
                    printk(KERN_INFO "chal_cam_set_analog_pwr: OFF: Channels Enabled\n");	
                }
            }
            else
            {
            // Analog Power Up
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,APD,0);
            // Reset Analog Receiver
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2SRST,ARST,1);
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2SRST,ARST,0);
            }
            break;
    // CCP Set Analog Power
        case CHAL_CAM_INTF_CCP:
            printk(KERN_INFO "chal_cam_set_analog_pwr:  CHAL_CAM_INTF_CCP = %d\n", param->param );	
        // if Power OFF, Check if channels enabled        
            if (param->param == FALSE)
            {
            // if Power OFF, Check if channels enabled        
                if (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,RXEN) == 1)
                {
                    printk(KERN_INFO "chal_cam_set_analog_pwr: OFF: Channels Enabled\n");	
                    chal_status |= CHAL_OP_WRONG_ORDER;
                }
            // Reset Analog Receiver
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,ARST,1);
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,ARST,0);
            // Analog Power Down
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,APD,1);
                if ( (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC0,CHEN)) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC1,CHEN)) )
                {
                    printk(KERN_INFO "chal_cam_set_analog_pwr: OFF: Channels Enabled\n");	
                }
            }
            else
            {
            // Analog Power Up
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,APD,0);
            // Reset Analog Receiver
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,ARST,1);
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,ARST,0);
            }
            break;
    // CPI Set Analog Power
        case CHAL_CAM_INTF_CPI:
        default:
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC);
            reg |= (CAMINTF_CCP2RC_APD_MASK |  CAMINTF_CCP2RC_BPD_MASK);
            reg = BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC,reg);
            printk(KERN_INFO "chal_cam_set_analog_pwr:  CPI Interface\n");	
            break;
    }
    return chal_status;
}    



//==============================================================================
//! \brief 
//!		   	CAM module Set Bandgap Power
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       (unused)
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            (unused)
//!				param.select    (in) cfg select             (unused)
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (in) 0=OFF  1=ON
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_set_bandgap_pwr(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg;
    
    printk(KERN_INFO "chal_cam_set_bandgap_pwr = %d\n", param->param);	

// Set Bandgap Power
    switch (param->intf)
    {
    // CSI Set Bandgap Power
        case CHAL_CAM_INTF_CSI:
            printk(KERN_INFO "chal_cam_set_bandgap_pwr:  CHAL_CAM_INTF_CSI = %d\n", param->param);	
        // if Power OFF, Check if channels enabled        
            if (param->param == FALSE)
            {
            // if Power OFF, Check if channels enabled        
                if ( (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LENC) == 1) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LEN1) == 1) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LEN2) == 1))
                {
                    printk(KERN_INFO "chal_cam_set_bandgap_pwr: OFF: Channels Enabled\n" );	
                    chal_status |= CHAL_OP_WRONG_ORDER;
                }
            // Bandgap Power Down
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,BPD,1);
            }
            else
            {
            // Bandgap Power Up
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,BPD,0);
            }
            break;
    // CCP Set Bandgap Power
        case CHAL_CAM_INTF_CCP:
            printk(KERN_INFO "chal_cam_set_bandgap_pwr:  CHAL_CAM_INTF_CCP = %d\n", param->param );	
        // if Power OFF, Check if channels enabled        
            if (param->param == FALSE)
            {
            // if Power OFF, Check if channels enabled        
                if (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,RXEN) == 1)
                {
                    printk(KERN_INFO "chal_cam_set_bandgap_pwr: OFF: Channels Enabled\n" );	
                    chal_status |= CHAL_OP_WRONG_ORDER;
                }
            // Bandgap Power Down
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,BPD,1);
            }
            else
            {
            // Bandgap Power Up
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,BPD,0);
            }
            break;
    // CPI Set Bandgap Power
        case CHAL_CAM_INTF_CPI:
        default:
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC);
            reg |= (CAMINTF_CCP2RC_APD_MASK |  CAMINTF_CCP2RC_BPD_MASK);
            reg = BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC,reg);
            printk(KERN_INFO "chal_cam_set_bandgap_pwr:  CPI Interface\n" );	
            break;
    }
    return chal_status;
}    



//==============================================================================
//! \brief 
//!		   	CAM module Set Lane Enable
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            
//!				param.select    (in) cfg select             (unused)
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (in) if (CHAL_CAM_LANE_SELECT_t == 1) Lane = ON, else Lane = OFF
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_lane_enable(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg;
    
    printk(KERN_INFO "chal_cam_set_lane: lane=0x%x enable=0x%x\n", param->lane, param->param);	
// CSI2 Interface
    if (param->intf == CHAL_CAM_INTF_CSI)
    {
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC);
    // Lane 0
        if (param->lane & CHAL_CAM_DATA_LANE_0)
        {
        // Enable Data Lane 0 for CSI mode
            reg &= ~(CAMINTF_CSI2RC_LEN1_MASK);
            if (param->param & CHAL_CAM_DATA_LANE_0)
            {
                reg |= (1 << CAMINTF_CSI2RC_LEN1_SHIFT);
            }
        }
    // Lane 1
        if (param->lane & CHAL_CAM_DATA_LANE_1)
        {
        // Enable Data Lane 1 for CSI mode
            reg &= ~(CAMINTF_CSI2RC_LEN2_MASK);
            if (param->param & CHAL_CAM_DATA_LANE_1)
            {
                reg |= (1 << CAMINTF_CSI2RC_LEN2_SHIFT);
            }
        }
    // Clock
        if (param->lane & CHAL_CAM_CLK_LANE)
        {
        // Enable Clock Lane for CSI mode
            reg &= ~(CAMINTF_CSI2RC_LENC_MASK);
            if (param->param & CHAL_CAM_CLK_LANE)
            {
                reg |= (1 << CAMINTF_CSI2RC_LENC_SHIFT);
            }
        }
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC,reg);
    }
    else
// CCP2 Interface
    {
        printk(KERN_INFO "chal_cam_set_lane:  CHAL_CAM_INTF_CCP No Action\n" );	
    }    
    return chal_status;
}    

//==============================================================================
//! \brief 
//!		   	CAM module Set Lane Control
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select
//!				param.chan      (in) unused
//!				param.lane      (in) lane select
//!				param.param     (in) if (CHAL_CAM_LANE_SELECT_t == 1) Lane = ON, else Lane = OFF
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_set_lane_cntrl(CHAL_HANDLE handle, CHAL_CAM_LANE_CNTRL_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg;
    
    printk(KERN_INFO "chal_cam_set_lane: lane=0x%x enable=0x%x\n", cfg->lane, cfg->param );	

// CSI2 Interface
    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
    // Analog Bias
        if (cfg->cntrl_state & CHAL_CAM_LANE_ANALOG_BIAS)
        {
        // Lane 0  Analog Bias
            if (cfg->lane & CHAL_CAM_DATA_LANE_0)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PCP,D0_ACTRL,cfg->param);
            }
        // Lane 1 Analog Bias
            if (cfg->lane & CHAL_CAM_DATA_LANE_1)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PCP,D1_ACTRL,cfg->param);
            }
        // Clock Analog Bias
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2PCP,CLK_ACTRL,cfg->param);
            }
        }
    // Force Termination Enable
        else if (cfg->cntrl_state & CHAL_CAM_LANE_TERM_EN)
        {
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2TREN);
        // Lane 0 Termination
            if (cfg->lane & CHAL_CAM_DATA_LANE_0)
            {
                reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2TREN,TROV1,1);
                if (cfg->param)
                {
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2TREN,TREN1,1);
                }
                else
                {
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2TREN,TREN1,0);
                }
            }
        // Lane 1 Termination
            if (cfg->lane & CHAL_CAM_DATA_LANE_1)
            {
                reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2TREN,TROV2,1);
                if (cfg->param)
                {
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2TREN,TREN2,1);
                }
                else
                {
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2TREN,TREN2,0);
                }
            }
        // Clock Termination
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
                reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2TREN,TROVC,1);
                if (cfg->param)
                {
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2TREN,TRENC,1);
                }
                else
                {
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CSI2TREN,TRENC,0);
                }
            }
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2TREN,reg);
        }
    // High Speed Data Reception Delay Time
        else if (cfg->cntrl_state & CHAL_CAM_LANE_HS_RX_TIME)
        {
        // SOT Sync Timeout
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSTO,THSSTO,cfg->param);
        }
    // High Speed Settle Delay Time
        else if (cfg->cntrl_state & CHAL_CAM_LANE_HS_SETTLE_TIME)
        {
        // HS Settle
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSET,HSMC,cfg->param);
        }
    // High Speed Settle Delay Time
        else if (cfg->cntrl_state & CHAL_CAM_LANE_HS_TERM_TIME)
        {
        // HS Termination
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSET,TD,cfg->param);
        }
    // High Speed Clock Timeout
        else if (cfg->cntrl_state & CHAL_CAM_LANE_HS_CLK_TIME)
        {
        // HS Termination
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSCKTO,THSCKTO,cfg->param);
        }
        else
        {
            chal_status = CHAL_OP_INVALID;
        }
    }
    else
// CCP2 Interface
    {
    // Analog Bias
        if (cfg->cntrl_state & CHAL_CAM_LANE_ANALOG_BIAS)
        {
        // Lane 0  Analog Bias
            if (cfg->lane & (CHAL_CAM_DATA_LANE_0 |CHAL_CAM_DATA_LANE_1))
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,DLAC,cfg->param);
            }
        // Clock Analog Bias
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,CLAC,cfg->param);
            }
        }
    // Force Termination Enable
        else if (cfg->cntrl_state & CHAL_CAM_LANE_TERM_EN)
        {
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,TREN,cfg->param);
        }
        else
        {
            chal_status = CHAL_OP_INVALID;
        }
    }    
    return chal_status;
}    





//==============================================================================
//! \brief 
//!		   	Set CAM module Input Intf
//! \note 
//!        	Parameters:   
//!				handle  (in) this CAM instance
//!				cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_set_intf(CHAL_HANDLE handle, CHAL_CAM_CFG_INTF_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    cInt32 data_shift;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    CHAL_CAM_CPI_CNTRL_CFG_st_t*    p_cpi_cfg;

// Set CPI Configuration Pointer
    p_cpi_cfg = (CHAL_CAM_CPI_CNTRL_CFG_st_t*)cfg->p_cpi_cfg_st; 

    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
        printk(KERN_INFO "chal_cam_set_intf: CHAL_CAM_INTF_CSI \n" );	
    // Set Rx
            if (cfg->input_mode == CHAL_CAM_INPUT_DUAL_LANE)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSET,HSMC,32);
            }
            else
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2THSSET,HSMC,16);
            }
        // Termination Override
            //BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2TREN,0x00000000);
    }
    else
    {
        printk(KERN_INFO "chal_cam_set_intf: CHAL_CAM_INTF_CCP/CHAL_CAM_INTF_CPI \n" );	
    // Set Rx
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC);
        // CCP Mode
            if (cfg->intf == CHAL_CAM_INTF_CCP)
            {
                printk(KERN_INFO "chal_cam_set_intf:  CCP Mode\n" );	
                reg &= ~(CAMINTF_CCP2RC_INMP_MASK);
            // Clk Mode
                if (cfg->clk_mode == CHAL_CAM_DATA_CLOCK)
                {
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RC,CLKM,1);
                }
            // Enable Termination
                //reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RC,TREN,1);
            }
            else
        // CPI Mode
            {
                reg |= CAM_REG_FIELD_SET(CAMINTF_CCP2RC,INMP,1);
                printk(KERN_INFO "chal_cam_set_intf:  CPI Mode\n" );	
            }
    // Update Cam Control Register
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC,reg);

    // CPI Mode
        if ( (cfg->intf == CHAL_CAM_INTF_CPI) && (p_cpi_cfg != NULL) )
        {
        // Reset CPI Peripheral        
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIS,0xC0000000);
        // Enable CPI Peripheral        
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,ENB,0x01);
       
            // Set CPIF Register
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CPIF);
                // Sync Mode
                    reg &= ~(CAMINTF_CPIF_SMODE_MASK);
                    if (p_cpi_cfg->sync_mode == CHAL_CAM_SYNC_EMBEDDED)
                    {
                        reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,SMODE,1);
                    }
                // VSync Mode
                    reg &= ~(CAMINTF_CPIF_VMODE_MASK);
                    switch (p_cpi_cfg->vsync_signal)
                    {
                        case CHAL_CAM_SYNC_START_DATA:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,VMODE,1);
                            break;
                        case CHAL_CAM_SYNC_DEFINES_SYNC:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,VMODE,2);
                            break;
                        case CHAL_CAM_SYNC_IGNORED:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,VMODE,3);
                            break;
                        case CHAL_CAM_SYNC_DEFINES_ACTIVE:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,VMODE,0);
                            break;
                    }
                // HSync Mode
                    reg &= ~(CAMINTF_CPIF_HMODE_MASK);
                    switch (p_cpi_cfg->hsync_signal)
                    {
                        case CHAL_CAM_SYNC_START_DATA:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,HMODE,1);
                            break;
                        case CHAL_CAM_SYNC_DEFINES_SYNC:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,HMODE,2);
                            break;
                        case CHAL_CAM_SYNC_IGNORED:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,HMODE,3);
                            break;
                        case CHAL_CAM_SYNC_DEFINES_ACTIVE:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,HMODE,0);
                            break;
                    }
                // Field Mode
                    reg &= ~(CAMINTF_CPIF_FMODE_MASK);
                    switch (p_cpi_cfg->fmode)
                    {
                        case CHAL_CAM_FIELD_EMBEDDED:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,FMODE,1);
                            break;
                        case CHAL_CAM_FIELD_EVEN:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,FMODE,2);
                            break;
                        case CHAL_CAM_FIELD_ODD:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,FMODE,3);
                            break;
                        case CHAL_CAM_FIELD_H_V:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIF,FMODE,0);
                            break;
                    }
            // Update CPIF Control Register
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIF,reg);
        
            // Set CPIR Register
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CPIR);
                // Input Bit Shift (+)=Left shift, (-)=Right shift
                    reg &= ~(CAMINTF_CPIR_BITSHIFT_MASK);
                    if (p_cpi_cfg->data_shift > 0)
                    {
                        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_set_intf:  Left Data Shift Not Allowed:\n");	
                        chal_status |= CHAL_OP_INVALID;
                        data_shift = 0;
                    }
                    else
                    {
                        data_shift = 0 - p_cpi_cfg->data_shift;
                    }
                    reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITSHIFT,data_shift);
                        
                // Input Bit Width
                    reg &= ~(CAMINTF_CPIR_BITWIDTH_MASK);
                    switch (p_cpi_cfg->bit_width)
                    {
                        case CHAL_CAM_PIXEL_10BIT:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,1);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,1);
                            break;
                        case CHAL_CAM_PIXEL_12BIT:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,2);
                            break;
                        case CHAL_CAM_PIXEL_16BIT:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,3);
                            break;
                        case CHAL_CAM_PIXEL_8BIT:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,BITWIDTH,0);
                            break;
                    }
                // VSync Trigger
                    reg &= ~(CAMINTF_CPIR_VSRM_MASK | CAMINTF_CPIR_VSAL_MASK);
                    switch (p_cpi_cfg->vsync_trigger)
                    {
                        case CHAL_CAM_SYNC_EDGE_NEG:
                        case CHAL_CAM_SYNC_ACTIVE_LOW:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,VSRM,1);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,VSAL,1);
                            break;
                        case CHAL_CAM_SYNC_EDGE_POS:
                        case CHAL_CAM_SYNC_ACTIVE_HIGH:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,VSRM,0);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,VSAL,0);
                           break;
                    }
                // HSync Trigger
                    reg &= ~(CAMINTF_CPIR_HSRM_MASK | CAMINTF_CPIR_HSAL_MASK);
                    switch (p_cpi_cfg->hsync_trigger)
                    {
                        case CHAL_CAM_SYNC_EDGE_NEG:
                        case CHAL_CAM_SYNC_ACTIVE_LOW:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,HSRM,1);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,HSAL,1);
                            break;
                        case CHAL_CAM_SYNC_EDGE_POS:
                        case CHAL_CAM_SYNC_ACTIVE_HIGH:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,HSRM,0);
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,HSAL,0);
                            break;
                    }
                // Clock Edge
                    reg &= ~(CAMINTF_CPIR_DSRM_MASK);
                    switch (p_cpi_cfg->clock_edge)
                    {
                        case CHAL_CAM_SYNC_EDGE_NEG:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,DSRM,1);
                            break;
                        case CHAL_CAM_SYNC_EDGE_POS:
                        default:
                            reg |= CAM_REG_FIELD_SET(CAMINTF_CPIR,DSRM,0);
                            break;
                    }
            // Update CPIR Control Register
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIR,reg);
        }
    }
    return chal_status;
}    


//==============================================================================
//! \brief 
//!		   	CAM module Configure Short Packet Capture
//! \note 
//!        	Parameters:   
//!				handle      (in) this CAM instance
//!				cfg         (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_short_pkt(CHAL_HANDLE handle, CHAL_CAM_PKT_CFG_st_t* cfg)
{
//    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;

    printk(KERN_INFO "chal_cam_cfg_short_pkt: instance=%d \n",cfg->instance );	

    chal_status = CHAL_OP_INVALID;

    return chal_status;
}    

//==============================================================================
//! \brief 
//!		   	CAM module Get Short Packet
//! \note 
//!        	Parameters:   
//!				handle      (in) this CAM instance
//!				short_pkt   (out) packet struct to fill in
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_get_short_pkt(CHAL_HANDLE handle, CHAL_CAM_PKT_st_t* short_pkt)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;


// CSI2 Interface
    if (short_pkt->intf == CHAL_CAM_INTF_CSI)
    {
        printk(KERN_INFO "chal_cam_get_short_pkt: CSI Interface\n" );	
        short_pkt->word_count = BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RGSP,DATA);
        short_pkt->data_id = (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RGSP,VC) << CHAL_CAM_VIRTUAL_CHANNEL_SHIFT) | 
                                    (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RGSP,DT) << CHAL_CAM_DATA_TYPE_SHIFT);
        short_pkt->ecc = 0x00;
    }
// CCP/CPI Interface
    else 
    {
        chal_status = CHAL_OP_FAILED;
        //chal_dprintf( CDBG_INFO, "chal_cam_get_short_pkt: FAILED: No Packet Available\n");	
    }

    return chal_status;
}    


//==============================================================================
//! \brief 
//!		   	CAM module Get Receiver status
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       (unused)
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            (unused)
//!				param.select    (in) cfg select             (unused)
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (in) OR'd Interrupt and Error Status's for receiver
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
UInt32 chal_cam_get_rx_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    
    switch (param->intf)
    {
    // CSI Status Register
        case CHAL_CAM_INTF_CSI:
        // Get Status Register       
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS);
        // Clear status          
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS,reg);
            param->param = 0;
//            printk(KERN_INFO "chal_cam_get_rx_status:  CSI2RS=0x%x\n", reg );	
        // Check status bits	
            if (reg & CAMINTF_CSI2RS_IS0_MASK)
            {
                param->param |= CHAL_CAM_RX_INT_CHAN_0;
            }
            if (reg & CAMINTF_CSI2RS_IS1_MASK)
            {
                param->param |= CHAL_CAM_RX_INT_CHAN_1;
            }
            if (reg & (CAMINTF_CSI2RS_OEB_MASK | CAMINTF_CSI2RS_PED_MASK))
            {
                param->param |= CHAL_CAM_RX_PKT_ERROR;
            }
            if (reg & CAMINTF_CSI2RS_OFP_MASK)
            {
                param->param |= CHAL_CAM_RX_PANIC;
            }
            if (reg & CAMINTF_CSI2RS_OFF_MASK)
            {
                param->param |= CHAL_CAM_RX_FIFO_ERROR;
            }
            if (reg & CAMINTF_CSI2RS_GEF_MASK)
            {
                param->param |= CHAL_CAM_RX_ERROR;
            }
            break;

    // CCP/CPI Status Register
        default:
        case CHAL_CAM_INTF_CCP:
        case CHAL_CAM_INTF_CPI:
        // Get Status Register       
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS);
        // Clear status          
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS,reg);
            param->param = 0;
        // Check status bits	
            if (reg & CAMINTF_CCP2RS_IS0_MASK)
            {
                param->param |= CHAL_CAM_RX_INT_CHAN_0;
            }
            if (reg & CAMINTF_CCP2RS_IS1_MASK)
            {
                param->param |= CHAL_CAM_RX_INT_CHAN_1;
            }
            if (reg & CAMINTF_CCP2RS_OEB_MASK)
            {
                param->param |= CHAL_CAM_RX_BUSY;
            }
            if (reg & (CAMINTF_CCP2RS_IDSO_MASK | CAMINTF_CCP2RS_FSC_MASK | CAMINTF_CCP2RS_SSC_MASK | CAMINTF_CCP2RS_SYN_MASK))
            {
                param->param |= CHAL_CAM_RX_PKT_ERROR;
            }
            if (reg & CAMINTF_CCP2RS_OFP_MASK)
            {
                param->param |= CHAL_CAM_RX_PANIC;
            }
            if (reg & (CAMINTF_CCP2RS_IFO_MASK | CAMINTF_CCP2RS_OFO_MASK))
            {
                param->param |= CHAL_CAM_RX_FIFO_ERROR;
            }
            if (reg & CAMINTF_CCP2RS_GEF_MASK)
            {
                param->param |= CHAL_CAM_RX_ERROR;
//                printk(KERN_INFO "CHAL_CAM_RX_GLOBAL_ERROR:  CAM_CPC2RS=0x%x\n", reg );
            }
            break;
    }
    return reg;
}


//==============================================================================
//! \brief 
//!		   	CAM module Get Receiver Lane status
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select
//!				param.chan      (in) unused
//!				param.lane      (in) lane select: CLK, Lane 1-2
//!				param.cntrl_state (in) unused
//!				param.param     (in) OR'd Lane Status's for receiver
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_get_lane_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    
    printk(KERN_INFO "chal_cam_get_lane_status:  0x%x\n", param->lane );	

    switch (param->intf)
    {
    // CSI Status Register
        case CHAL_CAM_INTF_CSI:
        // Get Status Register       
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDLS);
            param->param = 0;
            printk(KERN_INFO "chal_cam_get_lane_status:  CHAL_CAM_INTF_CSI=%d CAM_CSI2RDLS=0x%x\n", param->lane, reg );	
            if (param->lane == CHAL_CAM_CLK_LANE)
            {
            // Enabled
                if ( BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LENC) == 1 )
                {
                    param->param |= CHAL_CAM_LANE_EN;
                }
            // Check Clock status bits	
                if (reg & CAMINTF_CSI2RDLS_ULPSC_MASK)
                {
                    param->param |= CHAL_CAM_LANE_ULP;
                }
                if ( reg & (CAMINTF_CSI2RDLS_SOTEC_MASK|CAMINTF_CSI2RDLS_SOTSEC_MASK|CAMINTF_CSI2RDLS_ESEC_MASK|CAMINTF_CSI2RDLS_EEEC_MASK|CAMINTF_CSI2RDLS_CEC_MASK) )
                {
                    param->param |= CHAL_CAM_LANE_STATE_ERROR;
                }
                reg &= (CAMINTF_CSI2RDLS_ULPSC_MASK|CAMINTF_CSI2RDLS_SOTEC_MASK|CAMINTF_CSI2RDLS_SOTSEC_MASK|CAMINTF_CSI2RDLS_ESEC_MASK|CAMINTF_CSI2RDLS_EEEC_MASK|CAMINTF_CSI2RDLS_CEC_MASK);
            }
            else if (param->lane == CHAL_CAM_DATA_LANE_0)
            {
            // Enabled
                if ( BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LEN1) == 1 )
                {
                    param->param |= CHAL_CAM_LANE_EN;
                }
            // Check Lane 1 status bits	
                if (reg & CAMINTF_CSI2RDLS_ULPS1_MASK)
                {
                    param->param |= CHAL_CAM_LANE_ULP;
                }
                if ( reg & (CAMINTF_CSI2RDLS_SOTE1_MASK|CAMINTF_CSI2RDLS_SOTSE1_MASK|CAMINTF_CSI2RDLS_ESE1_MASK|CAMINTF_CSI2RDLS_EEE1_MASK|CAMINTF_CSI2RDLS_CE1_MASK) )
                {
                    param->param |= CHAL_CAM_LANE_STATE_ERROR;
                }
                reg &= (CAMINTF_CSI2RDLS_ULPS1_MASK|CAMINTF_CSI2RDLS_SOTE1_MASK|CAMINTF_CSI2RDLS_SOTSE1_MASK|CAMINTF_CSI2RDLS_ESE1_MASK|CAMINTF_CSI2RDLS_EEE1_MASK|CAMINTF_CSI2RDLS_CE1_MASK);
            }
            else if (param->lane == CHAL_CAM_DATA_LANE_1)
            {
            // Enabled
                if ( BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LEN2) == 1 )
                {
                    param->param |= CHAL_CAM_LANE_EN;
                }
            // Check Lane 2 status bits	
                if (reg & CAMINTF_CSI2RDLS_ULPS2_MASK)
                {
                    param->param |= CHAL_CAM_LANE_ULP;
                }
                if ( reg & (CAMINTF_CSI2RDLS_SOTE2_MASK|CAMINTF_CSI2RDLS_SOTSE2_MASK|CAMINTF_CSI2RDLS_ESE2_MASK|CAMINTF_CSI2RDLS_EEE2_MASK|CAMINTF_CSI2RDLS_CE2_MASK) )
                {
                    param->param |= CHAL_CAM_LANE_STATE_ERROR;
                }
                reg &= (CAMINTF_CSI2RDLS_ULPS2_MASK|CAMINTF_CSI2RDLS_SOTE2_MASK|CAMINTF_CSI2RDLS_SOTSE2_MASK|CAMINTF_CSI2RDLS_ESE2_MASK|CAMINTF_CSI2RDLS_EEE2_MASK|CAMINTF_CSI2RDLS_CE2_MASK);
            }
        // Clear status          
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDLS,reg);
            break;

    // CCP/CPI Status Register
        case CHAL_CAM_INTF_CCP:
        case CHAL_CAM_INTF_CPI:
        default:
            printk(KERN_INFO "chal_cam_get_lane_status:  Undefined Interface\n" );	
            chal_status = CHAL_OP_INVALID;
            break;
    }
    return chal_status;
}    


//==============================================================================
//! \brief 
//!		   	CAM module Get Receiver Channel status
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       (unused)
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            (unused)
//!				param.select    (in) cfg select             (unused)
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (in) OR'd channel interrupt status, interrupts then cleared
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
UInt32 chal_cam_get_chan_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;


    switch (param->intf)
    {
    // CSI Status Register
        case CHAL_CAM_INTF_CSI:
        // Get Status Register       
            if (param->chan == CHAL_CAM_CHAN_0)
            {
            // Read status
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS0);
            // Clear status
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS0,reg);
            }
            else
            {
            // Read status
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS1);
            // Clear status
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS1,reg);
            }
            param->param = 0;
//            printk(KERN_INFO "chal_cam_get_chan_status:  CHAL_CAM_INTF_CSI=%d CAM_CSI2RSx=0x%x\n", param->chan, reg );	
        // Check status bits	
            if (reg & CAMINTF_CSI2RS0_FSI_MASK)
            {
                param->param |= CHAL_CAM_INT_FRAME_START;
            }
            if (reg & CAMINTF_CSI2RS0_FEI_MASK)
            {
                param->param |= CHAL_CAM_INT_FRAME_END;
            }
            if (reg & CAMINTF_CSI2RS0_LSI_MASK)
            {
                param->param |= CHAL_CAM_INT_LINE_START;
            }
            if (reg & CAMINTF_CSI2RS0_LEI_MASK)
            {
                param->param |= CHAL_CAM_INT_LINE_END;
            }
            if (reg & CAMINTF_CSI2RS0_LCI_MASK)
            {
                param->param |= CHAL_CAM_INT_LINE_COUNT;
            }
            if (reg & CAMINTF_CSI2RS0_GSPI_MASK)
            {
                param->param |= CHAL_CAM_INT_PKT;
            }
            if ( reg & (CAMINTF_CSI2RS0_CRCE_MASK|CAMINTF_CSI2RS0_IBO_MASK|CAMINTF_CSI2RS0_DBO_MASK) )
            {
                param->param |= CHAL_CAM_INT_FRAME_ERROR;
                chal_dprintf( CDBG_ERRO, "FRAME ERROR: CAM_CSI2RSx=0x%x\n", reg);	
            }
            break;

    // CCP/CPI Status Register
        default:
        case CHAL_CAM_INTF_CCP:
        case CHAL_CAM_INTF_CPI:
        // Get Status Register       
            if ( (param->chan == CHAL_CAM_CHAN_0) || (param->intf == CHAL_CAM_INTF_CPI) )
            {
            // Read status
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS0);
            // Clear status
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS0,reg);
            }
            else
            {
            // Read status
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS1);
            // Clear status
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS1,reg);
            }
            param->param = 0;
//            printk(KERN_INFO "chal_cam_get_chan_status:  CHAL_CAM_INTF_CCP=%d CAM_CCP2RSx=0x%x\n", param->chan, reg );	
        // Check status bits	
            if (reg & CAMINTF_CCP2RS0_FSI_MASK)
            {
                param->param |= CHAL_CAM_INT_FRAME_START;
            }
            if (reg & CAMINTF_CCP2RS0_FEI_MASK)
            {
                param->param |= CHAL_CAM_INT_FRAME_END;
            }
            if (reg & CAMINTF_CCP2RS0_LSI_MASK)
            {
                param->param |= CHAL_CAM_INT_LINE_START;
            }
            if (reg & CAMINTF_CCP2RS0_LEI_MASK)
            {
                param->param |= CHAL_CAM_INT_LINE_END;
            }
            if (reg & CAMINTF_CCP2RS0_LCI_MASK)
            {
                param->param |= CHAL_CAM_INT_LINE_COUNT;
            }
//            if ( (reg & CAM_CCP2RS0_CRCE) || (reg & CAM_CCP2RS0_IBO) || (reg & CAM_CCP2RS0_DBO) || (reg & CAM_CCP2RS0_UDF) || (reg & CAM_CCP2RS0_UMT))
//            if ( (reg & CAM_CCP2RS0_CRCE) || (reg & CAM_CCP2RS0_UDF) || (reg & CAM_CCP2RS0_UMT))
            if (reg &  (CAMINTF_CCP2RS0_UDF_MASK | CAMINTF_CCP2RS0_UMT_MASK) )
            {
                param->param |= CHAL_CAM_INT_FRAME_ERROR;
                chal_dprintf( CDBG_ERRO, "FRAME ERROR: CAM_CCP2RSx=0x%x\n", reg);	
            }
            break;
    }
    return reg;
}    



//==============================================================================
//! \brief 
//!		   	CAM module Receiver Reset
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       (unused)
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            (unused)
//!				param.select    (in) cfg select             (unused)
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_reset(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;

    switch (param->intf)
    {
    // CSI Reset
        case CHAL_CAM_INTF_CSI:
            printk(KERN_INFO "chal_cam_reset:  CHAL_CAM_INTF_CSI: param=0x%x\n", param->param );
        // Software Reset
            if (param->param & CHAL_CAM_RESET_RX)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2SRST,SRST,1);
            }
        // Analog Reset
            if (param->param & CHAL_CAM_RESET_ARST)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2SRST,ARST,1);
                CHAL_DELAY_MS(1);        
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2SRST,ARST,0);
            }
            break;     
       
    // CCP & CPI Reset
        case CHAL_CAM_INTF_CCP:
        case CHAL_CAM_INTF_CPI:
            printk(KERN_INFO "chal_cam_reset:  CHAL_CAM_INTF_CCP/CHAL_CAM_INTF_CPI: param=0x%x\n", param->param );
        // Software Reset
            if (param->param & CHAL_CAM_RESET_RX)
            {
            //    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,SWR,1); //haipeng
            }
        // Analog Reset
            if (param->param & CHAL_CAM_RESET_ARST)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,ARST,1);
                CHAL_DELAY_MS(1);        
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,ARST,0);
            }
        // Image/Data Pointers
            if (param->param & (CHAL_CAM_RESET_IMAGE | CHAL_CAM_RESET_DATA) )
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,RSYN,1);
            }

        // CPI Interface
            if (param->intf & CHAL_CAM_INTF_CPI)
            {
            // CPI Channel Reset
                printk(KERN_INFO "chal_cam_reset:  CHAL_CAM_INTF_CPI: CAM_CPIS_ACT\n" );
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,ACT,0);
                reg = chal_cam_wait_value(handle, CHAL_CAM_INTF_CPI, MIN_FRAME_RATE, (UInt32 *)(pCamDevice->baseAddr+CAMINTF_CPIS_OFFSET), (UInt32)CAMINTF_CPIS_DACT_MASK, (UInt32)0x00000000);
                if ( reg != 0 )
                {
                    chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_reset: CAM_CPIS_DACT: CAMINTF_CPIS=0x%x\n", reg);	
                }
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIS,0);
           //     BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,SWR,1); //haipeng
            }
            break;     
        default:
            printk(KERN_INFO "chal_cam_reset:  Undefined Interface\n");	
            chal_status = CHAL_OP_INVALID;
            break;
    }
    return chal_status;
}    


//==============================================================================
//! \brief 
//!		   	CAM module Enable Frame Trigger
//! \note 
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       (unused)
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            (unused)
//!				param.select    (in) cfg select             (unused)
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_channel_trigger(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg;

// CSI2 Interface
    if (param->intf == CHAL_CAM_INTF_CSI)
    {
        printk(KERN_INFO "chal_cam_channel_trigger:  CSI Trigger Unavailable\n" );	
    }
    else
    {
        reg = 1;
        if (param->chan & CHAL_CAM_CHAN_0)
        {                
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC0,TRIG,reg);
        }
        else
        {
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC1,TRIG,reg);
        }   
    }
    return chal_status;
}


//==============================================================================
//! \brief 
//!		   	CAM module Receiver Start
//! \note 
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       (unused)
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            (unused)
//!				param.select    (in) cfg select             (unused)
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_rx_start(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    
    switch (param->intf)
    {
    // CSI Interface
        case CHAL_CAM_INTF_CSI:
            printk(KERN_INFO "chal_cam_rx_start:  CHAL_CAM_INTF_CSI: chan=0x%x \n", param->chan );
        // Channel Enable
            if (param->chan & CHAL_CAM_CHAN_0)
            {                
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC0,CHEN,1);
            }
            else
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC1,CHEN,1);
            } 
        // Enable Clock Lane
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,LENC,1);
              
        // Receiver Enable
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2RC,GEN,1);
            break;

    // CCP Interface
        case CHAL_CAM_INTF_CCP:
            printk(KERN_INFO "chal_cam_rx_start:  CHAL_CAM_INTF_CCP: chan=0x%x \n", param->chan );
        // Channel Enable
            if (param->chan & CHAL_CAM_CHAN_0)
            {                
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC0,CHEN,1);
            }
            else
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC1,CHEN,1);
            }   
        // Receiver Enable
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,RXEN,1);
            break;

    // CPI Interface
        case CHAL_CAM_INTF_CPI:
            printk(KERN_INFO "chal_cam_rx_start:  CHAL_CAM_INTF_CPI: chan=0x%x \n", param->chan, param->lane );
        // CCP Channel 0 Enable
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC0,CHEN,1);
        // CPI Channel Enable
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,ENB,1);
        // Receiver Enable
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,RXEN,1);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,ACT,1);
            break;

        default:
            printk(KERN_INFO "chal_cam_rx_start:  Undefined Interface\n" );	
            chal_status = CHAL_OP_INVALID;
            break;
    }
    return chal_status;
}    



//==============================================================================
//! \brief 
//!		   	CAM module Receiver Stop
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       (unused)
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            (unused)
//!				param.select    (in) cfg select             (unused)
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_rx_stop(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg;
	cUInt32 count=0;
    
    printk(KERN_INFO "chal_cam_rx_stop:   \n" );

    switch (param->intf)
    {
    // CSI Interface
        case CHAL_CAM_INTF_CSI:
        // halt & disable Channel interrupts
            if (param->chan & CHAL_CAM_CHAN_0)
            {                
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC0);
            }
            else
            {
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC1);
            }   
            // halt & disable interrupts
                reg &= ~(CAMINTF_CSI2RC0_FSIE_MASK|CAMINTF_CSI2RC0_FEIE_MASK|CAMINTF_CSI2RC0_LSIE_MASK|CAMINTF_CSI2RC0_LEIE_MASK|CAMINTF_CSI2RC0_GSPIE_MASK|CAMINTF_CSI2RC0_LCIE_MASK);
            // halt channel
                reg &= ~(CAMINTF_CSI2RC0_CHEN_MASK);
            if (param->chan & CHAL_CAM_CHAN_0)
            {                
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC0,reg);
            // Clear Interrupts
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS0);
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS0,reg);
            }
            else
            {
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC1,reg);
            // Clear Interrupts
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS1);
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS1,reg);
            }   
        // Disable Receiver & Clock Lane
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC);
            reg &= ~(CAMINTF_CSI2RC_LENC_MASK|CAMINTF_CSI2RC_GEN_MASK);
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RC,reg);
        // Reset Receiver
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CSI2SRST,SRST,1);
        // Clear Receiver Status
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS);
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RS,reg);
        // Clear Receiver Lane Status
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDLS);
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CSI2RDLS,reg);
            break;

    // CCP, CPI Interface
        case CHAL_CAM_INTF_CPI:
        case CHAL_CAM_INTF_CCP:

        // Disable Channel 
            if ( (param->chan & CHAL_CAM_CHAN_0) || (param->intf & CHAL_CAM_INTF_CPI) )
            {                
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC0);
            // disable interrupts
                reg &= ~(CAMINTF_CCP2RC0_FSIE_MASK|CAMINTF_CCP2RC0_FEIE_MASK|CAMINTF_CCP2RC0_LSIE_MASK|CAMINTF_CCP2RC0_LEIE_MASK|CAMINTF_CCP2RC0_LCIE_MASK);
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC0,reg);

            // CPI interface
                if (param->intf & CHAL_CAM_INTF_CPI)
                {
                // CPI Channel Disable
                    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,ACT,0);
                    reg = chal_cam_wait_value(handle, CHAL_CAM_INTF_CPI, MIN_FRAME_RATE, (UInt32 *)(pCamDevice->baseAddr+CAMINTF_CPIS_OFFSET), (UInt32)CAMINTF_CPIS_DACT_MASK, (UInt32)0x00000000);
                    if ( reg != 0 )
                    {
                        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_rx_stop: CAM_CPIS_DACT: CAMINTF_CPIS=0x%x\n", reg);	
                    }
                    BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CPIS,0x00000000);
                }

            // wait for Channel Ready
                reg = chal_cam_wait_value(handle, CHAL_CAM_INTF_CCP, MIN_FRAME_RATE, (UInt32 *)(pCamDevice->baseAddr+CAMINTF_CCP2RS0_OFFSET), (UInt32)CAMINTF_CCP2RS0_CHB_MASK, (UInt32)0x00000000);
                if ( reg != 0 )
                {
                // Reset Receiver
              //      BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,SWR,1);
                    chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_rx_stop: CAMINTF_CCP2RS0_CHB: CAMINTF_CCP2RS0=0x%x\n", reg);	
                }
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC0,0x00000000);
            // Clear status
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS0);
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS0,reg);
            }
            else
            {
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC1);
            // disable interrupts
                reg &= ~(CAMINTF_CCP2RC0_FSIE_MASK|CAMINTF_CCP2RC0_FEIE_MASK|CAMINTF_CCP2RC0_LSIE_MASK|CAMINTF_CCP2RC0_LEIE_MASK|CAMINTF_CCP2RC0_LCIE_MASK);
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC1,reg);
            // wait for Channel Ready
                reg = chal_cam_wait_value(handle, CHAL_CAM_INTF_CCP, MIN_FRAME_RATE, (UInt32 *)(pCamDevice->baseAddr+CAMINTF_CCP2RS1_OFFSET), (UInt32)CAMINTF_CCP2RS1_CHB_MASK, (UInt32)0x00000000);
                if ( reg != 0 )
                {
                // Reset Receiver
                //    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,SWR,1);
                    chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_rx_stop: CAMINTF_CCP2RS1_CHB: CAMINTF_CCP2RS1=0x%x\n", reg);	
                }
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RC1,0x00000000);
            // Clear status
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS1);
                BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS1,reg);
            }   
        // Receiver Disable
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,RXEN,0);
        // Clear Receiver status
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS);
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAMINTF_CCP2RS,reg);
			BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CCP2RC,SWR,1);
			
			while(BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,ACT)!=0)
			{
				count++;
				BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,ACT,0);
				if(count>=50)
				{
					printk("count not reset ACT bit !!!!!!!\n");
					break;
				}
			}
			for(count=0;count<1000;count++)
			{
				BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,ENB,0);
				if(BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAMINTF_CPIS,CAPT)==0)
					break;
			}

            break;

        default:
            printk(KERN_INFO "chal_cam_rx_stop:  Undefined Interface\n" );	
            chal_status = CHAL_OP_INVALID;
            break;
    }
    return chal_status;
}    

    

//==============================================================================
//! \brief 
//!		   	CAM module Bytes Written
//! \note 
//!        	Parameters:   
//!				handle          (in) this CAM instance
//!				param.intf      (in) interface select       (unused)
//!				param.chan      (in) channel select         (unused)
//!				param.lane      (in) lane select            (unused)
//!				param.select    (in) cfg select             (unused)
//!				param.virt_chan (in) virtual channel select (unused)
//!				param.param     (out) bytes written
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_rx_bytes_written(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg = 0;

    switch (param->intf)
    {
    // CSI Interface
        case CHAL_CAM_INTF_CSI:
            printk(KERN_INFO "chal_cam_rx_bytes_written:  CHAL_CAM_INTF_CSI: chan=0x%x \n", param->chan );
        // Channel 
            if (param->chan & CHAL_CAM_CHAN_0)
            {                
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RBC0);
            }
            else
            {
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CSI2RBC1);
            }   
            break;

    // CCP, CPI Interface
        case CHAL_CAM_INTF_CPI:
        case CHAL_CAM_INTF_CCP:
            printk(KERN_INFO "chal_cam_rx_bytes_written:  CHAL_CAM_INTF_CCP: chan=0x%x \n", param->chan );
        // Channel 
            if ( (param->chan & CHAL_CAM_CHAN_0) || (param->intf & CHAL_CAM_INTF_CPI) )
            {                
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RBC0);
            }
            else
            {
                reg = BRCM_READ_REG(pCamDevice->baseAddr,CAMINTF_CCP2RBC1);
            }   
            break;

        default:
            printk(KERN_INFO "chal_cam_rx_bytes_written:  Undefined Interface\n" );
            chal_status = CHAL_OP_INVALID;
            break;
    }
    param->param = reg;
    return chal_status;
}    



