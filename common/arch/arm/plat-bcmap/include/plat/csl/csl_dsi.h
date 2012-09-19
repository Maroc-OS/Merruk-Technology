/*****************************************************************************
*
*    (c) 2001-2009 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*   @file   csl_dsi.h
*
*   @brief  CSL DSI Controller Driver Interface
*
****************************************************************************/
/**
*
* @defgroup LCDGroup LCD Controllers
*
* @brief    LCD Controllers API
*
* @ingroup  CSLGroup
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#inlcude "csl_lcd.h"
#include "irqctrl.h"
*/

#ifndef __CSL_DSI_H__
#define __CSL_DSI_H__

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @addtogroup LCDGroup 
 * @{
 */

#define CHIP_REVISION 20

#ifndef _BRCM_8BYTE_MSG_CONSTRAINT
#define _BRCM_8BYTE_MSG_CONSTRAINT
#endif	//_BRCM_8BYTE_MSG_CONSTRAINT

//#ifndef LCD_COLUMN_UPDATE_DMA
//#define LCD_COLUMN_UPDATE_DMA
//#endif

#ifndef LCD_COLUMN_UPDATE_CPU
#define LCD_COLUMN_UPDATE_CPU
#endif

#ifdef _BRCM_8BYTE_MSG_CONSTRAINT_FIX
/**
*
*  DSI DCS Params dma buffer ( structure to define buffer for
*  params buf (Workaround for 8 byte msg constraint) )
*
*****************************************************************************/
typedef struct
{
    dma_addr_t		phys_params_base;      ///< physical address of the dma buffer
    char __iomem	*params_base;       ///< virtual address
} DSI_PARAMS_BUF_T;
#endif	//_BRCM_8BYTE_MSG_CONSTRAINT_FIX
 
/**
*
*  DSI CLK Configuration ( used for definition of HS & ESC Clocks )
*
*****************************************************************************/
typedef struct
{
    UInt32              clkIn_MHz;      ///< input clock  
    UInt32              clkInDiv;       ///< input clock divider value
} DSI_CLK_T;
 
/**
*
*  DSI D-PHY Spec Rev  
*
*****************************************************************************/
typedef enum
{
    DSI_DPHY_0_92 = 1,
} DSI_DPHY_SPEC_T;
 
 
/**
*
*  DSI BUS CONFIGURATION
*
*****************************************************************************/
typedef struct {
    UInt32              bus;            ///< DSI controller(bus) number
    UInt32              dlCount;        ///< Number of Data Lines
    DSI_DPHY_SPEC_T     dPhySpecRev;    ///< D-PHY Spec Rev
    DSI_CLK_T           escClk;         ///< ESC Clk Configuration
    DSI_CLK_T           hsBitClk;       ///< HS  Clk Configuration 
    UInt32              lpBitRate_Mbps; ///< LP Data Bit Rate, MAX=10[Mbps]
    Boolean             enaContClock;   ///< enable Continous Clock         
    Boolean             enaRxCrc;       ///< enable RX CRC         
    Boolean             enaRxEcc;       ///< enable RX ECC        
    Boolean             enaHsTxEotPkt;  ///< enable TX of EOT during HS mode         
    Boolean             enaLpTxEotPkt;  ///< enable TX of EOT during LP mode      
    Boolean             enaLpRxEotPkt;  ///< enable RX of EOT during LP mode      
} CSL_DSI_CFG_t, *pCSL_DSI_CFG;


/**
*
*  DSI Initialization CONFIGURATION
*
*****************************************************************************/
typedef struct {
    UInt32              interruptId;            ///< DSI Interrupt ID
    UInt32 				dsi_base_address;       ///< DSI Base Addresss
} CSL_DSI_INIT_CFG_t, *pCSL_DSI_INIT_CFG;



/**
*
*  DSI Replay
*
*****************************************************************************/
#define DSI_RX_TYPE_TRIG            1
#define DSI_RX_TYPE_READ_REPLY      2   
#define DSI_RX_TYPE_ERR_REPLY       4 
  
typedef struct
{
    UInt8               type;             ///< type of the reply - bit fields
    UInt8               trigger;          ///< trigger message 
    UInt8               readReplyDt;      ///< Reply DT
    UInt8               readReplySize;    ///< Reply Size   [bytes]
    UInt32              readReplyRxStat;  ///< Reply Status
    UInt8*              pReadReply;       ///< Reply payload, MAX 8 [bytes]
    UInt8               errReportDt;      ///< DsiErrRpt DT
    UInt32              errReportRxStat;  ///< DsiErrRpt STAT
    UInt16              errReport;        ///< DsiErrRpt data, bit-fields
} CSL_DSI_REPLY_t, *pCSL_DSI_REPLY; 

/**
*
*  DSI COMMAND INTERFACE
*
*****************************************************************************/
typedef struct
{
    UInt32              vc;            ///< Destination Virtual Channel
    Boolean             isLP;          ///< LP(Low Power) | HS(High Speed)
    Boolean             isLong;        ///< LONG | SHORT
    UInt32              dsiCmnd;       ///< DSI command to send
    UInt8*              msg;           ///< TX msg payload
    UInt32              msgLen;        ///< SHORT=MAX 2[bytes],LONG=MAX 8[bytes]
    Boolean             endWithBta;    ///< End command with BTA
    pCSL_DSI_REPLY      reply;         ///< if endWithBta, otherwise NULL
} CSL_DSI_CMND_t, *pCSL_DSI_CMND; 


/**
*
* TEARING Configuration
*
*****************************************************************************/
/**
*
* TE SYNC Type
*
*****************************************************************************/
typedef enum
{
    DSI_TE_NONE,                        ///< TE NotUsed
    DSI_TE_CTRLR_TRIG,                  ///< TE BTA + TE TRIG Msg
    DSI_TE_CTRLR_INPUT_0,               ///< TE Ctrlr Synced With EXT TE Sig
    DSI_TE_CTRLR_INPUT_1,               ///< TE Ctrlr Synced With EXT TE Sig
} CSL_DSI_TE_IN_TYPE_T;

#if ( defined (_ATHENA_)&& (CHIP_REVISION >= 20) ) 

#define CSL_DSI_TE_MODE_VSYNC          (0)   ///< TE MODE vsync       (mode 1)    
#define CSL_DSI_TE_MODE_VSYNC_HSYNC    (1)   ///< TE MODE vsync_hsync (mode 2)

#define CSL_DSI_TE_ACT_POL_LO          (0)   ///< Active LO ( VSYNC & HSYNC )
#define CSL_DSI_TE_ACT_POL_HI          (1)   ///< Active HI ( VSYNC & HSYNC )

/**
*
* TE Input Config
*
*****************************************************************************/
typedef struct 
{
    UInt32     te_mode;      ///< ATHENA B0 te mode ( VSYNC or VSYNC & HSYNC )
    UInt32     sync_pol;     ///< ATHENA B0 sync polarity
    UInt32     vsync_width;  ///< ATHENA B0 vsync_width [tectl_clk_count]
    UInt32     hsync_line;   ///< ATHENA B0 hsync_line
} CSL_DSI_TE_IN_CFG_t, *pCSL_DSI_TE_IN_CFG;

#endif // #if ( defined (_ATHENA_)&& (CHIP_REVISION >= 20) ) 


/**
*
* TE Config
*
*****************************************************************************/
typedef struct
{
    CSL_DSI_TE_IN_TYPE_T    teInType;   ///< DSI TE Input type
    #if ( defined (_ATHENA_)&& (CHIP_REVISION >= 20) ) 
    pCSL_DSI_TE_IN_CFG      teInCfg;    ///< DSI TE Input Configuration
    #endif    
} CSL_DSI_TE_T;



/**
*
*  COMMAND MODE DISPLAY INTERFACE
*
*****************************************************************************/
typedef struct {
    UInt32              vc;             ///< Virtual Channel
    UInt32              dsiCmnd;        ///< DSI Command DT
    UInt32              dcsCmndStart;   ///< Display MEMWR Start    Command
    UInt32              dcsCmndCont;    ///< Display MEMWR Continue Command
    Boolean             isLP;           ///< LP(Low Power) / HS(High Speed)
    CSL_LCD_CM_IN       cm_in;          ///< Color Mode In  (frame buffer)
    CSL_LCD_CM_OUT      cm_out;         ///< Color Mode Out (display)
    CSL_DSI_TE_T        teCfg;          ///< TE configuration
} CSL_DSI_CM_VC_t, *pCSL_DSI_CM_VC;



/**
*
*  @brief    Send DSI Command  
*
*  @param	 clientH        (in)  CSL DSI Client handle
*  @param	 cmnd           (in)  Command definition
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     R&W access with DSI status feedback capability
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_SendPacket ( CSL_LCD_HANDLE clientH, pCSL_DSI_CMND cmnd );


/**
*
*  @brief    Open (configure) Command Mode VC Display Interface
*
*  @param	 clientH        (in)  CSL DSI Client handle
*  @param	 dsiCmVcCfg     (in)  Command Mode VC Config
*  @param	 dsiCmVcH       (out) Command Mode VC Handle
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_OpenCmVc ( 
                CSL_LCD_HANDLE     clientH, 
                pCSL_DSI_CM_VC     dsiCmVcCfg, 
                CSL_LCD_HANDLE*    dsiCmVcH ); 

/**
*
*  @brief    Close Command Mode VC 
*
*  @param	 dsiCmVcH       (in)  Command Mode VC handle
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_CloseCmVc ( CSL_LCD_HANDLE dsiCmVcH );
              
/**
*
*  @brief    Update Command Mode Display 
*
*  @param	 dsiCmVcH       (in) Command Mode VC handle
*  @param	 req            (in) Update Request
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_UpdateCmVc ( 
                CSL_LCD_HANDLE      dsiCmVcH, 
                pCSL_LCD_UPD_REQ    req );
/**
*
*  @brief    Update Command Mode Display
*
*  @param        dsiCmVcH       (in) Command Mode VC handle
*  @param        req            (in) Update Request
*  @param  		 data_buff 		(in) data to transfer
*  @return       CSL_LCD_RES_T  (out) result
*
*  @note
*
*****************************************************************************/
CSL_LCD_RES_T CSL_DSI_CPU_UpdateCmVc (
					CSL_LCD_HANDLE  	dsiCmVcH,
					pCSL_LCD_UPD_REQ 	req,
					unsigned int* 		data_buff );
/**
*
*  @brief    Lock DSI Interface for the Client
*
*  @param	 client         (in)  DSI Client Handle
*
*  @return	 void
*
*  @note     
*
*****************************************************************************/                           
void CSL_DSI_Unlock ( CSL_LCD_HANDLE client );

/**
*
*  @brief    Release Client's DSI Lock
*
*  @param	 client         (in)  DSI Client Handle
*
*  @return	 void
*
*  @note     
*
*****************************************************************************/                           
void CSL_DSI_Lock ( CSL_LCD_HANDLE client );

/**
*
*  @brief    Register DSI Client
*
*  @param	 bus          (in)  DSI controller(bus) number
*  @param	 client       (out) DSI client handle
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     DSI Interface Must Be Open (configured)
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_OpenClient ( UInt32 bus, CSL_LCD_HANDLE* client );
              

/**
*
*  @brief    Close DSI Client Interface
*
*  @param	 client         (in)  DSI client handle
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_CloseClient ( CSL_LCD_HANDLE client );

/**
*
*  @brief    DSI Interface ULPS "Ultra Low Power State" Control
*
*  @param	 client         (in)   DSI client handle
*  @param	 on             (in)   ULPS on|off
*
*  @return	 CSL_LCD_RES_T  (out)  result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T  CSL_DSI_Ulps ( CSL_LCD_HANDLE client, Boolean on );

/**
*
*  @brief    Init DSI Interface
*
*  @param	 dsiCfg         (in)   DSI Interface Configuration
*
*  @return	 CSL_LCD_RES_T  (out)  result
*
*  @note     
*
*****************************************************************************/                           
#ifndef _BRCM_8BYTE_MSG_CONSTRAINT
CSL_LCD_RES_T CSL_DSI_Init ( pCSL_DSI_INIT_CFG initCfg, pCSL_DSI_CFG dsiCfg,void *ptr );
#else
CSL_LCD_RES_T CSL_DSI_Init( pCSL_DSI_INIT_CFG initCfg, pCSL_DSI_CFG dsiCfg);
#endif	//_BRCM_8BYTE_MSG_CONSTRAINT

/**
*
*  @brief    De-Init DSI Interface
*
*  @param	 bus            (in)   DSI Controller ID
*
*  @return	 CSL_LCD_RES_T  (out)  result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T  CSL_DSI_Close ( UInt32 bus );
/** @} */

#ifdef __cplusplus
}
#endif

#endif // __CSL_DSI_H__
