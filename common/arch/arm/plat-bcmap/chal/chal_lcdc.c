/***************************************************************************
*
* Copyright 2004 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*
****************************************************************************/
/**
*  @file   chal_lcdc.c
*
*  @brief  ATHENA LCD Controller (Z80/M68/DBI-B/DBI-C) cHAL source code file.
*
*  @note
*
****************************************************************************/

#include "plat/chal/chal_common.h"
#include "plat/chal/chal_lcdc.h"
#include "plat/rdb/brcm_rdb_syscfg.h"
#include "plat/rdb/brcm_rdb_lcdc.h"
#include "plat/rdb/brcm_rdb_util.h"

typedef struct
{
    cUInt32 baseAddr;
} CHAL_LCDC_T, *pCHAL_LCDC_T;

typedef struct
{
    cBool           init;
    CHAL_LCDC_T     lcdcCtrl[1];
} CHAL_LCDC_DRV_T, *pCHAL_LCDC_DRV_T;


//*****************************************************************************
// Local Variables
//*****************************************************************************
static CHAL_LCDC_DRV_T  lcdcDev;

//*****************************************************************************
// Local Functions
//*****************************************************************************


//SETs REGISTER BIT FIELD; VALUE IS 0 BASED
#define LCDC_REG_FIELD_SET(r,f,d) \
    (( (BRCM_REGTYPE(r))(d) << BRCM_FIELDSHIFT(r,f) ) \
    & BRCM_FIELDMASK(r,f))

//SETs REGISTER BITs Defined WITH MASK
#define LCDC_WRITE_REG_MASKED(b,r,m,d) \
    (BRCM_WRITE_REG(b,r,(BRCM_READ_REG(b,r) & (~m)) | d))

//*****************************************************************************
//
// Function Name: chal_lcdc_init
//
// Description:   Initialize LCDC Parallel (Z80/M68 & DBI) Controller
//                and software interface
//
//*****************************************************************************
CHAL_HANDLE chal_lcdc_init (cUInt32 baseAddr)
{
    pCHAL_LCDC_T pDev = NULL;

    chal_dprintf (CDBG_INFO, "chal_lcdc_init\n");

    pDev = (pCHAL_LCDC_T) &lcdcDev.lcdcCtrl[0];

    if( !lcdcDev.init )
    {
        pDev->baseAddr = baseAddr;
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, DMA, 0 );
        lcdcDev.init = TRUE;
    }

    return (CHAL_HANDLE) pDev;
}


//*****************************************************************************
//
// Function Name: chal_lcdc_set_dma
//
// Description:   Enables|Disables Controller DMA Interface
//
//*****************************************************************************
cVoid chal_lcdc_set_dma ( CHAL_HANDLE handle, cBool enable )
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if (enable)
        // Enable core DMA interface
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, DMA, (cUInt32)1 );
    else
        // Disable core DMA interface
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, DMA, (cUInt32)0 );
}


//****************************************************************************
//
//  Function Name:  chal_lcdc_set_cs
//
//  Description:    Activates Chip Select For Given Bank
//
//****************************************************************************
cVoid chal_lcdc_set_cs (CHAL_HANDLE handle, cBool cs0)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if (cs0)
        // CS0 Bank
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, SEL_LCD, (cUInt32)0 );
    else
        // CS1 Bank
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, SEL_LCD, (cUInt32)1 );
}


//****************************************************************************
//
//  Function Name:  chal_lcdc_dbib_set_speed
//
//  Description:    Configures RD&WR Timing Of DBI-B Bus Mode
//
//****************************************************************************
cVoid chal_lcdc_dbib_set_speed (CHAL_HANDLE handle, pCHAL_LCDC_DBI_SPEED speed)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, THCLK_CNT, speed->tAhbClkC );

    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBIBTR,
        DCX_HIGH  , speed->dcxHi );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBIBTR,
        DCX_LOW   , speed->dcxLo );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBIBTR,
        READ_HIGH , speed->rdHi  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBIBTR,
        READ_LOW  , speed->rdLo  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBIBTR,
        WRITE_HIGH, speed->wrHi  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBIBTR,
        WRITE_LOW , speed->wrLo  );
}


//****************************************************************************
//
//  Function Name:  chal_lcdc_dbic_set_speed
//
//  Description:    Configures RD&WR Timing For DBI-C Bus Mode
//
//****************************************************************************
cVoid chal_lcdc_dbic_set_speed (CHAL_HANDLE handle, pCHAL_LCDC_DBI_SPEED speed)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, THCLK_CNT, speed->tAhbClkC);

    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBICTR,
        DCX_HIGH  , speed->dcxHi );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBICTR,
        DCX_LOW   , speed->dcxLo );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBICTR,
        READ_HIGH , speed->rdHi  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBICTR,
        READ_LOW  , speed->rdLo  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBICTR,
        WRITE_HIGH, speed->wrHi  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_DBICTR,
        WRITE_LOW , speed->wrLo  );
}


//****************************************************************************
//
//  Function Name:  chal_lcdc_dbi_config
//
//  Description:    Configures DBI Bus Modes Of Operation
//
//****************************************************************************
cVoid chal_lcdc_dbi_config (CHAL_HANDLE handle, pCHAL_LCDC_DBI_CFG cfg)
{
    cUInt32       mask    = 0;
    cUInt32       val     = 0;
    pCHAL_LCDC_T  pDev    = (pCHAL_LCDC_T) handle;

    mask  =   LCDC_CR_DBI_B_MASK
            | LCDC_CR_DBI_C_MASK
            | LCDC_CR_DBI_C_TYPE_MASK
            | LCDC_CR_COLOR_ENDIAN_MASK
            | LCDC_CR_INPUT_COLOR_MODE_MASK
            | LCDC_CR_OUTPUT_COLOR_MODE_MASK
            | LCDC_CR_DBIC_SELECT_MASK;

    // set interface type
    if (cfg->busType == CHAL_BUS_DBI_B)
      val  = LCDC_CR_DBI_B_MASK;

    if (   (cfg->busType == CHAL_BUS_DBI_C_OPT1)
        || (cfg->busType == CHAL_BUS_DBI_C_OPT3) )
      val |= LCDC_CR_DBI_C_MASK;

    if (cfg->busType == CHAL_BUS_DBI_C_OPT3)
      val |= LCDC_CR_DBI_C_TYPE_MASK;

    if (cfg->dbicSclRouteToLcdCd )
      val |= LCDC_CR_DBIC_SELECT_MASK;

    // set endianes of input
    if( cfg->colModeInBE )
        val |= LCDC_CR_COLOR_ENDIAN_MASK;

    // set in/out color formats
    val |= LCDC_REG_FIELD_SET ( LCDC_CR, INPUT_COLOR_MODE,  cfg->colModeIn );
    val |= LCDC_REG_FIELD_SET ( LCDC_CR, OUTPUT_COLOR_MODE, cfg->colModeOut );

    LCDC_WRITE_REG_MASKED ( pDev->baseAddr, LCDC_CR, mask, val );
}


//*****************************************************************************
//
// Function Name: chal_lcdc_par_set_ce
//
// Description:   Enable|Disable 565 To 666 Color Expansion
//                For Data Written To LCDC_DATR In Z80/M68 Bus Mode
//
//*****************************************************************************
cVoid chal_lcdc_par_set_ce (CHAL_HANDLE handle, cBool enable)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if (enable)
        // Color Expansion (RGB565 to RGB666) ON,  18-bit LCD bus
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, CE, (cUInt32)1 );
    else
        // Color Expansion (RGB565 to RGB666) OFF
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, CE, (cUInt32)0 );
}

//*****************************************************************************
//
// Function Name: chal_lcdc_dbi_set_pixel_mode
//
// Description:   Enable  - data written to LCD_DATR are pixels  (col conv ON)
//                Disable - data written to LCD_DATR are generic (col conv OFF)
//
//                CAN WE AVOID ISSUE OF COMMAND WR CYCLE
//*****************************************************************************
cVoid chal_lcdc_dbi_set_pixel_mode (CHAL_HANDLE handle, cBool enable)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if (enable)
        LCDC_WRITE_REG_MASKED ( pDev->baseAddr, LCDC_CMDR,
            LCDC_CMDR_DBI_PIXEL_MODE_MASK, (cUInt32)1 );
    else
        LCDC_WRITE_REG_MASKED ( pDev->baseAddr, LCDC_CMDR,
            LCDC_CMDR_DBI_PIXEL_MODE_MASK, (cUInt32)0 );
}


//****************************************************************************
//
//  Function Name:  chal_lcdc_par_set_speed
//
//  Description:    Sets RD&WR Timing For Z80/M68 Bus
//
//****************************************************************************
cVoid chal_lcdc_par_set_speed (CHAL_HANDLE handle, pCHAL_LCDC_PAR_SPEED speed)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_RTR, HOLD , speed->rdHold  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_RTR, PULSE, speed->rdPulse );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_RTR, SETUP, speed->rdSetup );

    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_WTR, HOLD , speed->wrHold  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_WTR, PULSE, speed->wrPulse );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_WTR, SETUP, speed->wrSetup );
}

#if (CHIP_REVISION >= 20)
//****************************************************************************
//
//  Function Name:  chal_lcdc_par_set_rgb888u
//
//  Description:    Enable/Disable RGB888 Unpacked (xRGB) Mode For Legacy
//                  16-bit Interface
//
//****************************************************************************
cVoid chal_lcdc_par_set_rgb888u (CHAL_HANDLE handle, cBool enable)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if (enable)
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, UNPACKED_RGB888, (cUInt32)1 );
    else
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, UNPACKED_RGB888, (cUInt32)0 );
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_par_set_rgb888p
//
//  Description:    Enable/Disable RGB888 Packed (RGB) Mode For Legacy
//                  16-bit Interface
//
//****************************************************************************
cVoid chal_lcdc_par_set_rgb888p (CHAL_HANDLE handle, cBool enable)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if (enable)
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, PACKED_RGB888, (cUInt32)1 );
    else
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, PACKED_RGB888, (cUInt32)0 );
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_par_set_fast_mode
//
//  Description:    Enable/Disable Fast Legacy Mode
//
//****************************************************************************
cVoid chal_lcdc_par_set_fast_mode (CHAL_HANDLE handle, cBool enable)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if (enable)
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, FAST_LEGACY, (cUInt32)1 );
    else
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, FAST_LEGACY, (cUInt32)0 );
}

#endif


//****************************************************************************
//
//  Function Name:  chal_lcdc_par_config
//
//  Description:    Configures Z80/M68 Bus Modes Of Operation
//
//****************************************************************************
cVoid chal_lcdc_par_config (CHAL_HANDLE handle, pCHAL_LCDC_PAR_CFG cfg)
{
    cUInt32       mask = 0;
    cUInt32       val  = 0;
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    mask =  LCDC_CR_EIGHT_BIT_MASK
          | LCDC_CR_DBI_B_MASK
          | LCDC_CR_DBI_C_MASK
          | LCDC_CR_BYTESWAP_MASK
          | LCDC_CR_WORDSWAP_MASK
//          | LCDC_CR_UNPACKED_RGB888_MASK
          | LCDC_CR_VIDEO_CODEC_MODE_MASK;

    if (cfg->busWidth == 8)
        val |= LCDC_CR_EIGHT_BIT_MASK;
    if (cfg->byteSwap)
        val |= LCDC_CR_BYTESWAP_MASK;
    if (cfg->wordSwap)
        val |= LCDC_CR_WORDSWAP_MASK;
    if (cfg->bitSwap)
        val |= LCDC_CR_VIDEO_CODEC_MODE_MASK;

    // TODO
//  val |= LCDC_CR_UNPACKED_RGB888_MASK;

    LCDC_WRITE_REG_MASKED ( pDev->baseAddr, LCDC_CR, mask, val );
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_rd_status
//
//  Description:    Returns Content Of Status Register
//
//****************************************************************************
cUInt32 chal_lcdc_rd_status  (CHAL_HANDLE handle)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    return ( BRCM_READ_REG ( pDev->baseAddr, LCDC_STATUS ) );
}


//****************************************************************************
//
//  Function Name:  chal_lcdc_get_dma_address
//
//  Description:    Returns Controller's DMA FIFO Adress
//
//****************************************************************************
cUInt32 chal_lcdc_get_dma_address ( CHAL_HANDLE handle )
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    return (LCDC_BASE_ADDR + BRCM_REGOFS(LCDC_DATR));
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_set_te_cfg
//
//  Description:    Configures Controller TE Sync Settings
//
//****************************************************************************
cVoid chal_lcdc_set_te_cfg ( CHAL_HANDLE handle, pCHAL_LCDC_TE_CFG teCfg )
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if ( teCfg->edge == TE_EDGE_POS )
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, EDGE_SEL, (cUInt32)1 );
    else if ( teCfg->edge == TE_EDGE_NEG )
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, EDGE_SEL, (cUInt32)0 );

    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_TEDELAY, DELAY_COUNT,
        teCfg->delay_ahb_clks );
}


//****************************************************************************
//
//  Function Name:  chal_lcdc_set_te_new_frame
//
//  Description:    Set|Reset TE New Frame
//
//****************************************************************************
cVoid chal_lcdc_set_te_new_frame (CHAL_HANDLE handle, cBool enable)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if ( enable )
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, NEW_FRAME, (cUInt32)1 );
    else
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, NEW_FRAME, (cUInt32)0 );
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_set_te_enable
//
//  Description:    Enable|Disable Issue of TE synced RD/WR cycles
//
//****************************************************************************
cVoid chal_lcdc_set_te_enable (CHAL_HANDLE handle, cBool enable)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    if ( enable )
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, TEVALID, (cUInt32)1 );
    else
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, LCDC_CR, TEVALID, (cUInt32)0 );
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_wr_cmnd
//
//  Description:    Drives Data Write Cycle ('Cx' asserted on D/Cx line)
//                  On The Bus
//
//****************************************************************************
cVoid chal_lcdc_wr_cmnd (CHAL_HANDLE handle, cUInt32 command)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    BRCM_WRITE_REG ( pDev->baseAddr, LCDC_CMDR, command );
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_wr_data
//
//  Description:    Drives Data Write Cycle ('D' asserted on D/Cx line)
//                  On The Bus
//
//****************************************************************************
cVoid chal_lcdc_wr_data (CHAL_HANDLE handle, cUInt32 data)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    BRCM_WRITE_REG ( pDev->baseAddr, LCDC_DATR, data );
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_rd_data
//
//  Description:    Reads Data Value Retrived From Rd Cycle On The Bus
//
//****************************************************************************
cUInt32 chal_lcdc_rd_data (CHAL_HANDLE handle)
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    return ( BRCM_READ_REG ( pDev->baseAddr, LCDC_DATR ) );
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_par_rdreq
//
//  Description:    Drives Rd Cycle Of Given Type On Z80/M68 Bus
//
//****************************************************************************
cVoid chal_lcdc_par_rdreq ( CHAL_HANDLE handle, pCHAL_LCDC_PAR_RD_REQ rdAcc )
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    // RDB Does Not Have Field Defined
    #define LCDC_RD_CYCLE_Cx  0x00000100

    if ( rdAcc->type == PAR_RD_D )
        // RD Cycle with 'D'  state(HI) on D/Cx line
        LCDC_WRITE_REG_MASKED ( pDev->baseAddr, LCDC_RREQ,
            LCDC_RD_CYCLE_Cx, 0 );
    else
        // RD Cycle with 'Cx' state(LO) on D/Cx line
        LCDC_WRITE_REG_MASKED ( pDev->baseAddr, LCDC_RREQ,
            LCDC_RD_CYCLE_Cx, LCDC_RD_CYCLE_Cx );
}

//****************************************************************************
//
//  Function Name:  chal_lcdc_dbi_rdreq
//
//  Description:    Drives DBI Rd Cycle Of Given Type
//                  For Certain Types 8-bit Command Cycle Can be Driven Prior
//                  To Data Rd Cycle
//
//****************************************************************************
cVoid chal_lcdc_dbi_rdreq (
    CHAL_HANDLE             handle,
    pCHAL_LCDC_DBI_RD_REQ   rdAcc
    )
{
    pCHAL_LCDC_T pDev = (pCHAL_LCDC_T) handle;

    // RDB Does Not Have Field Defined
    #define LCDC_DBI_RD_CYCLE_TYPE_MASK     0x00000300
    #define LCDC_DBI_RD_CYCLE_TYPE_SHIFT    8
    #define LCDC_DBI_RD_CYCLE_RD_CMND_MASK  0x000000FF

    rdAcc->command &= LCDC_DBI_RD_CYCLE_RD_CMND_MASK;

    BRCM_WRITE_REG ( pDev->baseAddr, LCDC_RREQ,
        (cUInt32)rdAcc->type << LCDC_DBI_RD_CYCLE_TYPE_SHIFT | rdAcc->command);
}


