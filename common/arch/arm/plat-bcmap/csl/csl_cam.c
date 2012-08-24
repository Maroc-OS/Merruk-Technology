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

//
// File: csl_cam.c
//
//       ATHENA, HERA, RHEA CAM CSL implementation.
//

//#include "consts.h"
//#include "chip_version.h"

#include <plat/types.h>
#include <plat/osdal_os.h>
#include <plat/osdal_os_driver.h>
#include <plat/osdal_os_service.h>
//#include "pedestal.h"
#include <plat/rdb/brcm_rdb_sysmap.h>
//#include "syscfg_drv.h"
//#include <plat/csl/csl_common.h>
#include <plat/rdb/brcm_rdb_syscfg.h>
//#include "clk_drv.h"
#include <plat/csl/csl_cam.h>
#include <plat/chal/chal_common.h>
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_cam.h>
#include <mach/clkmgr.h>
#define BCM21553_CAMINTF_BASE1           0xf8440000
typedef int InterruptId_t;
#define CHIP_REVISION 20
static OSDAL_CLK_HANDLE camclk;
static OSDAL_CLK_HANDLE camsys;
static OSDAL_CLK_HANDLE camrx;
static OSDAL_CLK_HANDLE mipidsi = NULL;
/******************************************************************************
  Local macro declarations
 *****************************************************************************/
//#define ENABLE_PEDESTAL_DEEPSLEEP
 
//#define ENABLE_DEBUG
//#define ENABLE_DEBUG_REGISTER_DISPLAY
#define LOGID_SOC_CSL_CAM 100
#define DBG_L1 120
    #if ( defined(BSP_PLUS_BUILD_INCLUDED) )    
        #undef __CSLCAM_DBG_USE_LOG__
    #else
        #define __CSLCAM_DBG_USE_LOG__
    #endif
        // For compiling       
        #define __CSLCAM_DBG_USE_LOG__
    #ifndef __CSLCAM_DBG_USE_LOG__
        #define CSLCAM_DBG     Log_DebugPrintf
        #define CSLCAM_DBG_ID  LOGID_SOC_CSL_CAM
        #define DBG_OUT(x) x
    #else
        #define CSLCAM_DBG     dprintf
        #define CSLCAM_DBG_ID  DBG_L1
        #ifdef ENABLE_DEBUG
            #define DBG_OUT(x) x
        #else
           #define DBG_OUT(x) 
        #endif
    #endif 

    #ifdef ENABLE_DEBUG_REGISTER_DISPLAY
        #define DBG_OUT_REGISTER_DISPLAY(x) x
    #else
        #define DBG_OUT_REGISTER_DISPLAY(x) 
    #endif

/******************************************************************************
  Local typedefs 
 *****************************************************************************/
typedef struct
{
    Boolean                 active;
    CSL_CAM_CAPTURE_MODE_T  capture_mode;
    CSL_CAM_FRAME_st_t      frame_cfg;
    CSL_CAM_PIPELINE_st_t   pipeline_cfg;
    CSL_CAM_BUFFER_st_t     image_buffer_0;
    CSL_CAM_BUFFER_st_t     image_buffer_1;
    CSL_CAM_BUFFER_st_t     data_buffer;
    CSL_CAM_WINDOW_st_t     window_cfg;

// LISR CallBack   
    cslCamCB_t              lisr_callback;
    void *lisr_cb_userdata;
    
// Task Frame End CallBack   
    cslCamCB_t              task_callback;
    void *task_cb_userdata;

// Image buffer   
    UInt32                  bufferIndex;
    UInt32                  frame_buffer;
    UInt32                  frame_size;

// Frame Status
    UInt32                  rx_status;
    UInt32                  intr_status;
    UInt32                  raw_rx_status;
    UInt32                  raw_intr_status;
    
    UInt32                  LC_time;
    UInt32                  FE_time;
    UInt32                  FS_time;
} CSL_CAM_STATE_st_t;

typedef struct
{
    Boolean                 open;
    CSL_CAM_PORT_CHAN_T     port_chan_sel;
    CSL_CAM_STATE_st_t      state;
    CSL_CAM_DEVICE_MODE_T   dev_mode;
} CAM_HANDLE_T, *CAM_HANDLE;

typedef struct
{
    Boolean                 init;
    UInt32                  use_count;
    UInt32                  instance;
    CSL_CAM_HANDLE          currHandle;
    CSL_CAM_INTF_CFG_st_t   intf_cfg;
    CSL_CAM_PACKET_st_t     packet_cfg;
    Boolean                 int_registered;
    InterruptId_t           int_id;
    CHAL_HANDLE             chalCamH;
    CAM_HANDLE_T            camH[CAM_MAX_HANDLES]; 
} CSL_CAM_DRV_st;


typedef struct
{
    CSL_CAM_BUFFER_st_t *image0Buff;
    CSL_CAM_BUFFER_st_t *image1Buff;    
    CSL_CAM_BUFFER_st_t *dataBuff;    
} CSL_CAM_Buffers_st_t;

/******************************************************************************
 Static functions
 *****************************************************************************/
static Int32 csl_cam_set_buffer_params( CSL_CAM_HANDLE cslCamH, CSL_CAM_Buffers_st_t buffers);

/******************************************************************************
 Static data
 *****************************************************************************/

// cam driver
static CSL_CAM_DRV_st  cslCamDrv;

// handles
static void         *sClkHandle = NULL;     // clock handle

#if ( defined (_HERA_) || defined (_RHEA) )
    CHAL_HANDLE     sPadCtrlHandle = NULL;
#endif    
// csl cam Task Semaphore
static UInt32       sSemCslCamTask = NULL;              // for coordinating HISR and TASK
// csl cam Hisr
static UInt32       sHisrCslCam = NULL;
// csl cam Task
static UInt32       sCslCamTask = NULL;

#ifdef ENABLE_PEDESTAL_DEEPSLEEP            
static Int32        sCslCamPedestalId = -1;
static Int32        sCslCamDeepSleepId = -1;
#endif

/******************************************************************************
 Local Functions
 *****************************************************************************/
static CHAL_CAM_INTF_t cslCamChalIntf(CSL_CAM_INTF_T csl_val);
static CHAL_CAM_CHAN_t cslCamChalPortChan(CSL_CAM_PORT_CHAN_T csl_val);
//void cslCamFrameLisr( void );
static void cslCamFrameHisr( void );
#if 0
static void cslCamFrameTask( void );
#endif
static UInt32 cslCamFrameProcess(CSL_CAM_HANDLE cslCamH);
    #if ( defined (_HERA_) || defined (_RHEA) )
static Int32 cslCamClock(UInt32 clk_select, UInt32 freq, Boolean enable);
    #endif
 
/******************************************************************************
 Static  Functions
 *****************************************************************************/
 
//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal CHAL_CAM_INTF_t
//! \note 
//!         Parameters:   
//!             handle          (in) CSL_CAM_CHAN_SEL_t chan
//!         Return:
//!             CHAL_CAM_SUB_CHAN_t channel value
//==============================================================================
static CHAL_CAM_INTF_t cslCamChalIntf(CSL_CAM_INTF_T csl_val)
{
    CHAL_CAM_INTF_t chal_val;
    
    switch ( csl_val )      
    {
        default:
        case CSL_CAM_INTF_CCP:
            chal_val = CHAL_CAM_INTF_CCP;
            break;
        case CSL_CAM_INTF_CSI:
            chal_val = CHAL_CAM_INTF_CSI;
            break;
        case CSL_CAM_INTF_CPI:
            chal_val = CHAL_CAM_INTF_CPI;
            break;
    }
    return chal_val;
}    


//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal lane select
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PORT_CHAN_T 
//!         Return:
//!             CHAL_CAM_CHAN_t channel value
//==============================================================================
static CHAL_CAM_CHAN_t cslCamChalPortChan(CSL_CAM_PORT_CHAN_T csl_val)
{
    CHAL_CAM_CHAN_t chal_val = (CHAL_CAM_CHAN_t)0;

    if ( csl_val & CSL_CAM_PORT_CHAN_0)
    {
        chal_val |= CHAL_CAM_CHAN_0;    
    }
    if ( csl_val & CSL_CAM_PORT_CHAN_1)
    {
        chal_val |= CHAL_CAM_CHAN_1;    
    }
    return chal_val;
}    

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal lane select
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PORT_CHAN_T 
//!         Return:
//!             CHAL_CAM_CHAN_t channel value
//==============================================================================
static CHAL_CAM_LANE_SELECT_t cslCamChalLane(CSL_CAM_SELECT_t csl_val)
{
    CHAL_CAM_LANE_SELECT_t chal_val = (CHAL_CAM_LANE_SELECT_t)0;

    if (csl_val & CSL_CAM_DATA_LANE_0)
    {
        chal_val |= CHAL_CAM_DATA_LANE_0;    
    }
    if (csl_val & CSL_CAM_DATA_LANE_1)
    {
        chal_val |= CHAL_CAM_DATA_LANE_1;    
    }
    if (csl_val & CSL_CAM_CLOCK)
    {
        chal_val |= CHAL_CAM_CLK_LANE;    
    }
    return chal_val;
}    

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal lane select
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PORT_CHAN_T 
//!         Return:
//!             CHAL_CAM_CHAN_t channel value
//==============================================================================
static CHAL_CAM_LANE_STATE_t cslCamChalLane_ctrl(CSL_CAM_LANE_CONTROL_t csl_val)
{
    CHAL_CAM_LANE_STATE_t chal_val = CHAL_CAM_LANE_NOCHANGE;

    if (csl_val & CSL_CAM_LANE_EN)
    {
        chal_val |= CHAL_CAM_LANE_EN;    
    }
    if (csl_val & CSL_CAM_LANE_PD)
    {
        chal_val |= CHAL_CAM_LANE_PD;    
    }
    if (csl_val & CSL_CAM_LANE_LPEN)
    {
        chal_val |= CHAL_CAM_LANE_LPEN;    
    }
    if (csl_val & CSL_CAM_LANE_HS)
    {
        chal_val |= CHAL_CAM_LANE_HS;    
    }
    if (csl_val & CSL_CAM_LANE_TERM_EN)
    {
        chal_val |= CHAL_CAM_LANE_TERM_EN;    
    }
    if (csl_val & CSL_CAM_LANE_SYNC_MATCHING)
    {
        chal_val |= CHAL_CAM_LANE_SYNC_MATCHING;    
    }
    if (csl_val & CSL_CAM_LANE_ANALOG_BIAS)
    {
        chal_val |= CHAL_CAM_LANE_ANALOG_BIAS;    
    }
    if (csl_val & CSL_CAM_LANE_HS_RX_TIME)
    {
        chal_val |= CHAL_CAM_LANE_HS_RX_TIME;    
    }
    if (csl_val & CSL_CAM_LANE_HS_SETTLE_TIME)
    {
        chal_val |= CHAL_CAM_LANE_HS_SETTLE_TIME;    
    }
    if (csl_val & CSL_CAM_LANE_HS_TERM_TIME)
    {
        chal_val |= CHAL_CAM_LANE_HS_TERM_TIME;    
    }
    return chal_val;
}    

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal lane status
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PORT_CHAN_T 
//!         Return:
//!             CSL_CAM_LANE_CONTROL_t 
//==============================================================================
static CSL_CAM_LANE_STATUS_t cslCamChalLaneStatus(CHAL_CAM_LANE_STATE_t chal_val )
{
    CSL_CAM_LANE_STATUS_t csl_val = (CSL_CAM_LANE_STATUS_t)0;

    if (chal_val & CHAL_CAM_LANE_EN)
    {
        csl_val |= CSL_CAM_LANE_STATUS_EN;    
    }
    if (chal_val & CHAL_CAM_LANE_PD)
    {
        csl_val |= CSL_CAM_LANE_STATUS_PD;    
    }
    if (chal_val & CHAL_CAM_LANE_STATE_ERROR)
    {
        csl_val |= CSL_CAM_LANE_STATUS_ERROR;    
    }
    if (chal_val & CHAL_CAM_LANE_ULP)
    {
        csl_val |= CSL_CAM_LANE_STATUS_ULP;    
    }
    if (chal_val & CHAL_CAM_LANE_HS)
    {
        csl_val |= CSL_CAM_LANE_STATUS_HS;    
    }
    if (chal_val & CHAL_CAM_LANE_FF_ERROR)
    {
        csl_val |= CSL_CAM_LANE_STATUS_FF_ERROR;    
    }
    if (chal_val & CHAL_CAM_LANE_SYNC_MATCHING)
    {
        csl_val |= CSL_CAM_LANE_STATUS_SYNC_ERROR;    
    }
    return csl_val;
}    

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal pixel size
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PIXEL_SIZE_T 
//!         Return:
//!             CHAL_CAM_PIXEL_SIZE_t channel value
//==============================================================================
static CHAL_CAM_PIXEL_SIZE_t cslCamChalPixelSize(CSL_CAM_PIXEL_SIZE_T csl_val)
{
    CHAL_CAM_PIXEL_SIZE_t chal_val;

   switch (csl_val)
   {
        case CSL_CAM_PIXEL_6BIT:
            chal_val = CHAL_CAM_PIXEL_6BIT;
            break;
        case CSL_CAM_PIXEL_7BIT:
            chal_val = CHAL_CAM_PIXEL_7BIT;
            break;
        case CSL_CAM_PIXEL_8BIT:
            chal_val = CHAL_CAM_PIXEL_8BIT;
            break;
        case CSL_CAM_PIXEL_10BIT:
            chal_val = CHAL_CAM_PIXEL_10BIT;
            break;
        case CSL_CAM_PIXEL_12BIT:
            chal_val = CHAL_CAM_PIXEL_12BIT;
            break;
        case CSL_CAM_PIXEL_14BIT:
            chal_val = CHAL_CAM_PIXEL_14BIT;
            break;
        case CSL_CAM_PIXEL_16BIT:
            chal_val = CHAL_CAM_PIXEL_16BIT;
            break;
        default:
            chal_val = CHAL_CAM_PIXEL_NONE;
            break;
    }
    return chal_val;
}    


#if 0
/**
*  This function sets up the IOCR registers for Camera, based on Baseband Chip.
*
*/
static Int32 cslCamIocrSet( CSL_CAM_INTF_T intf, CSL_CAM_PORT_AFE_T afe_port, CSL_CAM_DEVICE_MODE_T mode )
{
    Int32 success = -1; //fail by default
#if (defined (_ATHENA_))
    UInt32 reg_val = 0;
#endif
    if (mode == CSL_CAM_INIT_MODE)
    {
#if (defined (_ATHENA_))
        success = 0;
    // Cfg IOCR's for Camera Port Receiver
        //SYSCFG_IOCR0 - IOCR0 bit I/O Configuration Register 0
        //      31  CAMCK_GPIO_MUX  0 CAMCK
        //                          1 GPIO61
        //                          Reset value is 0x0.
        //LCDD1_LCDD15_MUX  LCD/CAM/MPHI, concatenated with IOCR0 bit 25
        //  {IOCR0[28],IOCR0[25]}
        //      00      LCDD[15:1]
        //      01      reserved
        //      10      GPIO62-61, CAMD[9:0],CAMDCK,CAMHS,CAMVS
        //      11      MPHI Data[15:1]
        //      Reset value is 0x0. 
            if ( SYSCFGDRV_Config_Pin_Mux(SYSCFG_CAMCK_GPIO_MUX_CAMCK_SEL) != SYSCFG_STATUS_OK )
            {
                DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet][Error] : SYSCFGDRV_Config_Pin_Mux(): SYSCFG_CAMCK_GPIO_MUX_CAMCK_SEL: FAILED \n") );
                success = -1;
            }
            if (intf == CSL_CAM_INTF_CPI)
            {
                if ( SYSCFGDRV_Config_Pin_Mux(SYSCFG_LCD_CAM_MPHI_MUX_GPIO_CAM_SEL) != SYSCFG_STATUS_OK )
                {
                    DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet][Error] : SYSCFGDRV_Config_Pin_Mux(): SYSCFG_LCD_CSL_CAM_MPHI_MUX_GPIO_CSL_CAM_SEL: FAILED \n") );
                    success = -1;
                }
            }
        //SYSCFG_IOCR3 - IOCR3 bit I/O Configuration Register 3
        //      10  CAMD_PD     0 CAMD0-7 pulldown not selected
        //                      1 CAMD0-7 pulldown
        //                      Reset value is 0x0.
        //      09  CAMD_PU     0 CAMD0-7 pullup not selected
        //                      1 CAMD0-7 pullup
        //                      Reset value is 0x0.
        //      08  CAMHVS_PD   0 CAMHS/CAMVS pulldown not selected
        //                      1 CAMHS/CAMVS pulldown
        //                      Reset value is 0x0.
        //      07  CAMHVS_PU   0 CAMHS/CAMVS pullup not selected
        //                      1 CAMHS/CAMVS pullup
        //                      Reset value is 0x0.
        //      06  CAMDCK_PD   0 CAMDCK pulldown not selected
        //                      1 CAMDCK pulldown
        //                      Reset value is 0x0.
        //      05  CAMDCK_PU   0 CAMDCK pullup not selected
        //                      1 CAMDCK pullup
        //                      Reset value is 0x0.
        //      04  CAMCK_DIS   0 Normal operation
        //                      1 CAMCK output disabled, pulldown
        //                      Reset value is 0x0.
        reg_val = *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR3_OFFSET) ) & 
            ~(SYSCFG_IOCR3_CAMDCK_PD|SYSCFG_IOCR3_CAMDCK_PU|SYSCFG_IOCR3_CAMCK_DIS);
    
                reg_val |= SYSCFG_IOCR3_CAMDCK_PD;
        *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR3_OFFSET) ) = reg_val;
        //SYSCFG_IOCR4 - IOCR0 bit I/O Configuration Register 4
        //    14:12     Camera_Drive    Control drive strength of CAMCK/DIGMICCLK pads
        //            001 2 mA
        //            010 4 mA
        //            100 6 mA
        //            101 8 mA
        //            110 10 mA
        //            111 12 mA
        //            Reset value is 0x2.
        reg_val = *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR4_OFFSET) ) & ~(SYSCFG_IOCR4_Camera_Drive);
                reg_val |= (0x07 << 12);
        *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR4_OFFSET) ) = reg_val;
        
        
    //SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN - PERIPHERALS CAMARA INTERFACE AHB CLOCK ENABLE AND REQUEST REGISTER
    //    00    EN  Camara Interface AHB Clock Enable and Request (Read-Write)
    //          1 - clock enabled and requested
    //          0 - clock disabled and not requested
    //          Reset value is 0x1.        
    // Camera Intf Mode Select
            if ( SYSCFGDRV_Set_Periph_AHB_Clk_Item(SYSCFG_PERIPH_AHB_CLK_CAMARA_IF, SYSCFG_PERIPH_AHB_CLK_OFF) != SYSCFG_STATUS_OK )
            {
                DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet][Error] : SYSCFGDRV_Set_Periph_AHB_Clk_Item(): Clk Off FAILED \n") );
                success = -1;
            }
#endif


#if ( defined (_HERA_) || defined (_RHEA) )
        success = 0;
    #if 0
        PadCtrlConfig_t padCtrlConfig;

        if (sPadCtrlHandle == NULL)
        {
            DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet[Info] : chal_padctrl_init(): \n") );
            sPadCtrlHandle = chal_padctrl_init(PAD_CTRL_BASE_ADDR);
        }
        if (sPadCtrlHandle == NULL)
        {
            DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet][Error] : chal_padctrl_init(): FAILED \n") );
    //        return -1;
        }
            
        // Cfg IOCR's for Camera Port Receiver
        // CAMCS0: CAMCS0 Register
            //  Description :
            //  N/A
            //  Offset : 0x0000_0010
            //  Access : RW
            //  Default : 0x0000_0443
            //  Bit Field   Field Name  Default Access  Description
            //      31:11   RESERVED
            //      10:8    pinSel_CAMCS0   0x4 RW
            //          Function interface mux pin share selects
            //      7   hys_en_CAMCS0   0x0 RW
            //          Turns on hysteresis of input. Low = TTL input, High = Schmitt Trigger Input
            //      6   pdn_CAMCS0  0x1 RW
            //          High = pull-down on pad
            //      5   pup_CAMCS0  0x0 RW
            //              High = pull-up on pad
            //      4   src_CAMCS0  0x0 RW
            //          Select src rate of pad output.Low = high speed, High = slow output
            //      3   ind_CAMCS0  0x0 RW
            //          Input disable control
            //      2   sel_2_CAMCS0    0x0 RW
            //          General mode selection pin. See slect mode docs for detail information
            //      1   sel_1_CAMCS0    0x1 RW
            //          General mode selection pin. See slect mode docs for detail information
            //      0   sel_0_CAMCS0    0x1 RW
            //          General mode selection pin. See slect mode docs for detail information
            
            padCtrlConfig.PadCtrlConfigBitField.mode = 0x00;
            padCtrlConfig.PadCtrlConfigBitField.ind = 0x00;
            padCtrlConfig.PadCtrlConfigBitField.rate = 0x00;
            padCtrlConfig.PadCtrlConfigBitField.pup = 0x00;
            padCtrlConfig.PadCtrlConfigBitField.pdn = 0x00;
            padCtrlConfig.PadCtrlConfigBitField.hys = 0x00;
            padCtrlConfig.PadCtrlConfigBitField.mux = padCtrl_interface0;
    //        chal_padctrl_grant_clk(sPadCtrlHandle, padCtrl_dclk1, padCtrlConfig);
    //        chal_padctrl_grant_csi( sPadCtrlHandle, padCtrl_csi0, padCtrlConfig);
    //        chal_padctrl_grant_gpio( sPadCtrlHandle, padCtrl_csi0, padCtrlConfig);
    #endif
#endif

    }
    else if ( (mode == CSL_CAM_CLOSE_MODE)  || (mode == CSL_CAM_OPEN_MODE) ) 
    {
#if (defined (_ATHENA_))
        success = 0;
    // Cfg IOCR's for Camera Port Receiver
        //SYSCFG_IOCR0 - IOCR0 bit I/O Configuration Register 0
        //      31  CAMCK_GPIO_MUX  0 CAMCK
        //                          1 GPIO61
        //                          Reset value is 0x0.
        //LCDD1_LCDD15_MUX  LCD/CAM/MPHI, concatenated with IOCR0 bit 25
        //  {IOCR0[28],IOCR0[25]}
        //      00      LCDD[15:1]
        //      01      reserved
        //      10      GPIO62-61, CAMD[9:0],CAMDCK,CAMHS,CAMVS
        //      11      MPHI Data[15:1]
        //      Reset value is 0x0. 
            if ( SYSCFGDRV_Config_Pin_Mux(SYSCFG_CAMCK_GPIO_MUX_CAMCK_SEL) != SYSCFG_STATUS_OK )
            {
                DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet][Error] : SYSCFGDRV_Config_Pin_Mux(): SYSCFG_CAMCK_GPIO_MUX_CAMCK_SEL: FAILED \n") );
                success = -1;
            }
            if (intf == CSL_CAM_INTF_CPI)
            {
#if (CHIP_REVISION >= 20)
                // ATHENA B0
                if ( SYSCFGDRV_Config_Pin_Mux(SYSCFG_TRACEBUS_PORT_MUX_CAM_SEL) != SYSCFG_STATUS_OK )
                {
                    DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet][Error] : SYSCFGDRV_Config_Pin_Mux(): SYSCFG_TRACEBUS_PORT_MUX_CAM_SEL: FAILED \n") );
                    success = -1;
                }

             
#else        
                // ATHENA A0
                if ( SYSCFGDRV_Config_Pin_Mux(SYSCFG_LCD_CAM_MPHI_MUX_GPIO_CAM_SEL) != SYSCFG_STATUS_OK )
                {
                    DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet][Error] : SYSCFGDRV_Config_Pin_Mux(): SYSCFG_LCD_CSL_CAM_MPHI_MUX_GPIO_CSL_CAM_SEL: FAILED \n") );
                    success = -1;
                }
#endif


            }
        //SYSCFG_IOCR3 - IOCR3 bit I/O Configuration Register 3
        //      10  CAMD_PD     0 CAMD0-7 pulldown not selected
        //                      1 CAMD0-7 pulldown
        //                      Reset value is 0x0.
        //      09  CAMD_PU     0 CAMD0-7 pullup not selected
        //                      1 CAMD0-7 pullup
        //                      Reset value is 0x0.
        //      08  CAMHVS_PD   0 CAMHS/CAMVS pulldown not selected
        //                      1 CAMHS/CAMVS pulldown
        //                      Reset value is 0x0.
        //      07  CAMHVS_PU   0 CAMHS/CAMVS pullup not selected
        //                      1 CAMHS/CAMVS pullup
        //                      Reset value is 0x0.
        //      06  CAMDCK_PD   0 CAMDCK pulldown not selected
        //                      1 CAMDCK pulldown
        //                      Reset value is 0x0.
        //      05  CAMDCK_PU   0 CAMDCK pullup not selected
        //                      1 CAMDCK pullup
        //                      Reset value is 0x0.
        //      04  CAMCK_DIS   0 Normal operation
        //                      1 CAMCK output disabled, pulldown
        //                      Reset value is 0x0.
        reg_val = *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR3_OFFSET) ) & 
            ~(SYSCFG_IOCR3_CAMD_PD|SYSCFG_IOCR3_CAMD_PU|SYSCFG_IOCR3_CAMHVS_PD|SYSCFG_IOCR3_CAMHVS_PU|SYSCFG_IOCR3_CAMDCK_PD|SYSCFG_IOCR3_CAMDCK_PU|SYSCFG_IOCR3_CAMCK_DIS);
    
                reg_val |= SYSCFG_IOCR3_CAMDCK_PD;
                if (intf == CSL_CAM_INTF_CPI)
                {
                // Enable CAMD_PU,CAMHVS_PD,CAMDCK_PD
                    reg_val |= (SYSCFG_IOCR3_CAMD_PU | SYSCFG_IOCR3_CAMHVS_PD);
                }
                if (mode == CSL_CAM_CLOSE_MODE)
                {               
                    reg_val |= SYSCFG_IOCR3_CAMCK_DIS;
                }
        *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR3_OFFSET) ) = reg_val;
        //SYSCFG_IOCR4 - IOCR0 bit I/O Configuration Register 4
        //    14:12     Camera_Drive    Control drive strength of CAMCK/DIGMICCLK pads
        //            001 2 mA
        //            010 4 mA
        //            100 6 mA
        //            101 8 mA
        //            110 10 mA
        //            111 12 mA
        //            Reset value is 0x2.
        reg_val = *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR4_OFFSET) ) & ~(SYSCFG_IOCR4_Camera_Drive);
            if (mode == CSL_CAM_CLOSE_MODE)
            {               
                reg_val |= (0x01 << 12);
            }
            else
            {
                reg_val |= (0x07 << 12);
            }
        *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR4_OFFSET) ) = reg_val;
        //SYSCFG_IOCR6 - IOCR6 bit I/O Configuration Register 6
        //    30        CAM2_CAM1_B     Selects between the Camera AFE's
        //        At any point in time, only one camera can work. Dynamic switching between CAM1 and CAM2 is allowed.
        //            0 CAM1
        //            1 CAM2
        //    28:27     CSL_CAM_MODE        Selects the type of camera interface protocol. Dynamic switching between modes, will flush the previously selected FIFOs in the camera interface module.
        //            00 - Disabled
        //            01 - CCP2 Interface
        //            10 - CSI2 Interface
        //            11 - CCIR (parallel camera) Interface
        //            Reset value is 0x0.
        reg_val = *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR6_OFFSET) ) & ~(SYSCFG_IOCR6_Csi_ccp_b | SYSCFG_IOCR6_Cam_mode);
        // AFE Select
            if (afe_port == CSL_CAM_PORT_AFE_1)
            {
                reg_val |= (0x01 << 30);
            }
        // Camera Intf Mode Select
            if (mode == CSL_CAM_CLOSE_MODE)
            {               
                reg_val |= (0x00 << 27);
            }
            else
            {
                if (intf == CSL_CAM_INTF_CSI)
                {
                    reg_val |= (0x02 << 27);
                }
                else if (intf == CSL_CAM_INTF_CCP)
                {
                    reg_val |= (0x01 << 27);
                }
                else if (intf == CSL_CAM_INTF_CPI)
                {
                    reg_val |= (0x03 << 27);
                }
            }
        *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR6_OFFSET) ) = reg_val;
        
        
    //SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN - PERIPHERALS CAMARA INTERFACE AHB CLOCK ENABLE AND REQUEST REGISTER
    //    00    EN  Camara Interface AHB Clock Enable and Request (Read-Write)
    //          1 - clock enabled and requested
    //          0 - clock disabled and not requested
    //          Reset value is 0x1.        
    // Camera Intf Mode Select
        if (mode == CSL_CAM_OPEN_MODE)
        {
            if ( SYSCFGDRV_Set_Periph_AHB_Clk_Item(SYSCFG_PERIPH_AHB_CLK_CAMARA_IF, SYSCFG_PERIPH_AHB_CLK_ON) != SYSCFG_STATUS_OK )
            {
                DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet][Error] : SYSCFGDRV_Set_Periph_AHB_Clk_Item(): Clk On FAILED \n") );
                success = -1;
            }
        }
        else
        {
            if ( SYSCFGDRV_Set_Periph_AHB_Clk_Item(SYSCFG_PERIPH_AHB_CLK_CAMARA_IF, SYSCFG_PERIPH_AHB_CLK_OFF) != SYSCFG_STATUS_OK )
            {
                DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamIocrSet][Error] : SYSCFGDRV_Set_Periph_AHB_Clk_Item(): Clk Off FAILED \n") );
                success = -1;
            }
        }
#endif

#if ( defined (_HERA_) || defined (_RHEA) )
        success = 0;
/*        
    // Camera Intf Mode Select
        // CSI0_PHY_CTRL: CSI0 PHY control
        // Description :
        // N/A
        // Offset : 0x0000_0000
        // Access : RW
        // Default : 0x000C_0036        
        if (mode == CSL_CAM_OPEN_MODE)
        {
            if (afe_port == CSL_CAM_PORT_AFE_0)
            {
                *( (volatile UInt32 *)(MM_CFG_BASE_ADDR + 0x00000000) ) = 0x000C0036;
            // CSI0_AXI: CSI0_AXI Register            
                *( (volatile UInt32 *)(MM_CLK_BASE_ADDR + 0x00000228) ) = 0x00000303;
            // CSI0_LP: CSI0_LP Register            
                *( (volatile UInt32 *)(MM_CLK_BASE_ADDR + 0x00000230) ) = 0x00000303;
            }
            else
            {
                *( (volatile UInt32 *)(MM_CFG_BASE_ADDR + 0x00000000) ) = 0x800C0036;
                *( (volatile UInt32 *)(MM_CFG_BASE_ADDR + 0x00000004) ) = 0x000C0036;
            // CSI1_AXI: CSI1_AXI Register            
                *( (volatile UInt32 *)(MM_CLK_BASE_ADDR + 0x0000023C) ) = 0x00000303;
            // CSI1_LP: CSI1_LP Register            
                *( (volatile UInt32 *)(MM_CLK_BASE_ADDR + 0x00000244) ) = 0x00000303;
            }
        }     
        else 
        {
            if (afe_port == CSL_CAM_PORT_AFE_0)
            {
            // CSI0_AXI: CSI0_AXI Register            
                *( (volatile UInt32 *)(MM_CLK_BASE_ADDR + 0x00000228) ) = 0x00000302;
            // CSI0_LP: CSI0_LP Register            
                *( (volatile UInt32 *)(MM_CLK_BASE_ADDR + 0x00000230) ) = 0x00000302;
            }
            else
            {
                *( (volatile UInt32 *)(MM_CFG_BASE_ADDR + 0x00000000) ) = 0x800C0036;
                *( (volatile UInt32 *)(MM_CFG_BASE_ADDR + 0x00000004) ) = 0x000C0036;
            // CSI1_AXI: CSI1_AXI Register            
                *( (volatile UInt32 *)(MM_CLK_BASE_ADDR + 0x0000023C) ) = 0x00000302;
            // CSI1_LP: CSI1_LP Register            
                *( (volatile UInt32 *)(MM_CLK_BASE_ADDR + 0x00000244) ) = 0x00000302;
            }
         }     
*/         
#endif

    }
    else if ( (mode == CSL_CAM_STREAM_MODE) || (mode == CSL_CAM_IDLE_MODE) )
    {
        if ( (intf == CSL_CAM_INTF_CSI) || (intf == CSL_CAM_INTF_CCP) )
        {
#if (defined (_ATHENA_))
            //SYSCFG_IOCR6 - IOCR6 bit I/O Configuration Register 6
            //    30        CAM2_CAM1_B     Selects between the Camera AFE's
            //        At any point in time, only one camera can work. Dynamic switching between CAM1 and CAM2 is allowed.
            //            0 CAM1
            //            1 CAM2
            //    28:27     CSL_CAM_MODE        Selects the type of camera interface protocol. Dynamic switching between modes, will flush the previously selected FIFOs in the camera interface module.
            //            00 - Disabled
            //            01 - CCP2 Interface
            //            10 - CSI2 Interface
            //            11 - CCIR (parallel camera) Interface
            //            Reset value is 0x0.
            reg_val = *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR6_OFFSET) ) & ~(SYSCFG_IOCR6_Csi_ccp_b);
                if (afe_port == CSL_CAM_PORT_AFE_1)
                {
                    reg_val |= (0x01 << 30);
                }
            *( (volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_IOCR6_OFFSET) ) = reg_val;
#endif
        }
        success = 0;
    }
    return success;
}
#endif

/*****************************************************************************
 * NAME:    cslCamClock
 *
 * Description: this function will select the Primary or the Secondary Camera
 *
 * Parameters:  clkSel =0(DIG0) =1(DIG1)
 *              enable = 1 or disable = 0 clock
 *
 * Returns: int == 0 is success, all other values are failures
 *
 * Notes: Max clock rate = 26Mhz
 *
 ******************************************************************************/
    #if ( defined (_HERA_) || defined (_RHEA) )
static Int32 cslCamClock(UInt32 clk_select, UInt32 freq, Boolean enable)
{
    Int32 success = 0;          //pass by default
    
    
    UInt32 divider;

   /*--- Enable Camera Clk ---*/
  /* Camera clocks dclk1 & dclk2 are generated from
   * ROOT_CLK. dclk1 is 13 MHz by default
   * There is a DIG_PRE_DIV value set at 26 MHZ
   * DIG0_DIV (DCLK1) is set to 1 (div 2) by default => 13 MHz
   * DIG1_DIV (DCLK2) is set to 0 default is ?
   * If the numbers are changed then DIG_TRIG must be set again
   */

    BRCM_WRITE_REG(ROOT_CLK_BASE_ADDR,ROOT_CLK_MGR_REG_WR_ACCESS,0xA5A501);  // Enable access to registers in block
  // Is there a POLICY for this block ?

  /* Using default values */
   // ROOT CLOCK MGR 0x35001000  APB6
   // Use PRE_DIV default value  (using CRYSTAL CLOCK 0 = crystal_clk). It can be changed to PLL0 or PLL1_clk
   // BRCM_WRITE_REG( ROOT_CLK_BASE_ADDR , ROOT_PRE_DIV, 0x0);  // PRE DIV value already default set to 0 the reference clock (26 MHz)
    switch (freq)
    {
        case CSL_CAM_CLK_48MHz:
        case CSL_CAM_CLK_24MHz:
        case CSL_CAM_CLK_12MHz:
        default:
            success |= -1;
            divider = 0;
            break;
        case CSL_CAM_CLK_13MHz:
            divider = 1;
            break;
        case CSL_CAM_CLK_Disabled:
        case CSL_CAM_CLK_26MHz:
            divider = 0;
            break;
    }

    if ( BRCM_READ_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_PRE_DIV, DIGITAL_PRE_PLL_SELECT ) != 0 )
    {
        success |= -1;
    }
// Enable clock    
    if ( (success == 0) && (enable == 1) )
    {
    // Enable Clock Select 1
        if (clk_select == 1)
        {
        // Select GPIO32 to be DCLK1  ( bits 10:8 = 0x300 => DCLK2 , bits 2:0 = 3 => 8 mAmps strength
            BRCM_WRITE_REG_FIELD(PAD_CTRL_BASE_ADDR , PADCTRLREG_GPIO32, PINSEL_GPIO32, 3);
        // Disable Dig Clk1
            BRCM_WRITE_REG_FIELD( ROOT_CLK_BASE_ADDR, ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH1_CLK_EN, 0);
        // give it time to stop
            OSDAL_MDELAY( 10 * TICKS_ONE_SECOND / 1000 );
            if ( BRCM_READ_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH1_STPRSTS ) != 0 )  
            { 
                printk(KERN_INFO  "[cslCamClock][Error] : DIGITAL_CH1_STPRSTS: Clk not Stopped \n") );
                success |= -1;
            }
            BRCM_WRITE_REG( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG1_DIV, divider);
        // Enable Dig Clk 1
            BRCM_WRITE_REG_FIELD( ROOT_CLK_BASE_ADDR, ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH1_CLK_EN, 1);
        // give it time to settle
            OSDAL_MDELAY( 10 * TICKS_ONE_SECOND / 1000 );
        // Check Clock started
            if ( BRCM_READ_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH1_STPRSTS ) != 1 )  
            { 
                printk(KERN_INFO  "[cslCamClock][Error] : DIGITAL_CH1_STPRSTS: Clk not Started \n") );
                success |= -1;
            }
        }
        else
        {
        // Disable Dig Clk0
            BRCM_WRITE_REG_FIELD( ROOT_CLK_BASE_ADDR, ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH0_CLK_EN, 0);
        // give it time to stop
            OSDAL_MDELAY( 10 * TICKS_ONE_SECOND / 1000 );
            if ( BRCM_READ_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH0_STPRSTS ) != 0 )  
            { 
                printk(KERN_INFO  "[cslCamClock][Error] : DIGITAL_CH0_STPRSTS: Clk not Stopped \n") );
                success |= -1;
            }
            
        // Set Dig Clk0 Divider
            BRCM_WRITE_REG( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG0_DIV, divider);
       #if 0
            BRCM_WRITE_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_TRG, DIGITAL_CH0_TRIGGER, 1);   // Trigger channel 0
            OSDAL_MDELAY( 10 * TICKS_ONE_SECOND / 1000 );
            if ( BRCM_READ_REG_FIELD(ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_TRG, DIGITAL_CH0_TRIGGER) != 0 ) 
            {
                printk(KERN_INFO  "[cslCamClock][Error] : DIGITAL_CH0_STPRSTS: Clk Trigger not Set \n") );
                success |= -1;
            }
        #endif
        // Start Dig Clk0
            BRCM_WRITE_REG_FIELD( ROOT_CLK_BASE_ADDR, ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH0_CLK_EN, 1);
        // give it time to settle
            OSDAL_MDELAY( 10 * TICKS_ONE_SECOND / 1000 );
        // Check Clock running
            if ( BRCM_READ_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH0_STPRSTS ) != 1 )  
            { 
                printk(KERN_INFO  "[cslCamClock][Error] : DIGITAL_CH0_STPRSTS: Clk not Started \n") );
                success |= -1;
            }
        }
    }
    else if ( (success == 0) && (enable == 0) )
    {
    // Disable Dig Clk1
        if (clk_select == 1)
        {
            BRCM_WRITE_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH1_CLK_EN, 0);
        // give it time to stop
            OSDAL_MDELAY( 10 * TICKS_ONE_SECOND / 1000 );
        // Check Clock setting
            if ( BRCM_READ_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH1_STPRSTS ) != 0 )  
            { 
                success |= -1;
            }
        }
        else
        {
    // Disable Clock Select 0
            BRCM_WRITE_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH0_CLK_EN, 0);
        // give it time to settle
            OSDAL_MDELAY( 10 * TICKS_ONE_SECOND / 1000 );
        // Check Clock setting
            if ( BRCM_READ_REG_FIELD( ROOT_CLK_BASE_ADDR , ROOT_CLK_MGR_REG_DIG_CLKGATE, DIGITAL_CH0_STPRSTS ) != 0 )  
            { 
                success |= -1;
            }
        }
    }
    return success;
}            
#endif    


 /***********************************************************
 * Name: cslCamTaskInit
 * 
 * Arguments: 
 *       void
 *
 * Description: Initialize Csl Camera task and Semaphore
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
#define TASKNAME_CSLCAM "Camera task"
#define HISRNAME_CSLCAM "Camera HISR"
static Int32 cslCamTaskInit(void)
{
    Int32 success = 0;          //pass by default
    return success;
}


 /***********************************************************
 * Name: cslCamTaskDeInit
 * 
 * Arguments: 
 *       void
 *
 * Description: DeInitialize Csl Camera task and Semaphore
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
static Int32 cslCamTaskDeInit(void)
{
    Int32 success = 0;          //pass by default
    return success;
}
extern u32 chal_rbc_disp(CHAL_HANDLE handle);
extern u32 chal_rdr3_disp(CHAL_HANDLE handle);
/***********************************************************
 * Name: cslCamFrameLisr
 * 
 * Arguments: 
 *       none
 *
 * Description: Csl Camera HISR, Triggered from HISR
 * note:        Static: Triggers CSL CAM Task.
 ***********************************************************/
void cslCamFrameLisr( void )
{
    CAM_HANDLE camH;
    pCSL_CAM_BUFFER_st currentBuffer, nextBuffer;
    CSL_CAM_Buffers_st_t buffers;
	u32 jpgs;
	u32 rdr3;
    camH = cslCamDrv.currHandle;
/* Has the interrupt occured for Channel 0? */
    camH->state.raw_rx_status = csl_cam_get_rx_status( (CSL_CAM_HANDLE)camH, (CSL_CAM_RX_STATUS_t *)&camH->state.rx_status );
    camH->state.raw_intr_status = csl_cam_get_intr_status( (CSL_CAM_HANDLE)camH, (CSL_CAM_INTERRUPT_t *)&camH->state.intr_status );
    jpgs = chal_rbc_disp(cslCamDrv.chalCamH);
	//rdr3 = chal_rdr3_disp(cslCamDrv.chalCamH);
	//jpgs = rdr3;
	
    if ((camH->state.rx_status & CSL_CAM_RX_INT) && (camH->state.intr_status & CSL_CAM_INT_FRAME_START)){
	/* Just a callback to the client */
    }
    else if ((camH->state.rx_status & CSL_CAM_RX_INT) && (camH->state.intr_status & CSL_CAM_INT_FRAME_END))
    {
        if (camH->state.bufferIndex == 0) 
        {
            currentBuffer = &camH->state.image_buffer_0;
        
            if (camH->state.image_buffer_1.start_addr != NULL)
            {
                nextBuffer = &camH->state.image_buffer_1;              
            }
            else
            {
				pr_debug("Next buffer NULL 1\n");
                nextBuffer = NULL;
            }
        }
        else
        {
            currentBuffer = &camH->state.image_buffer_1;   
            nextBuffer    = &camH->state.image_buffer_0;            
        }
        
        camH->state.frame_size      = currentBuffer->size;
        camH->state.frame_buffer    = currentBuffer->start_addr;

        
    // Make sure we have a callback registered */
        if (camH->state.lisr_callback != NULL)
        {
            (*camH->state.lisr_callback)(camH->state.intr_status, camH->state.rx_status, camH->state.frame_buffer, camH->state.frame_size, camH->state.raw_intr_status, camH->state.raw_rx_status, camH->state.lisr_cb_userdata);
        }
        if ( camH->state.intr_status & (CSL_CAM_INT_FRAME_END | CSL_CAM_INT_LINE_COUNT) )
        {
            buffers.image1Buff = NULL;
            buffers.dataBuff   = NULL;
            
            if (camH->state.image_buffer_1.start_addr != NULL)
            {

//#if !( defined(_ATHENA_) && (CHIP_REVISION >= 20) )
#if 0
                // Athena B0 has HW pingpong capability.
                buffers.image0Buff = nextBuffer;                
                if (csl_cam_set_buffer_params( camH, buffers ) != 0)
                {
                    camH->state.rx_status |= CSL_CAM_RX_ERROR;
                }
#endif                
                // switch buffer index
                camH->state.bufferIndex = (camH->state.bufferIndex == 0)?1:0;                
            }
        }
       
    }
    //if ( (camH->state.task_callback != NULL) && (sHisrCslCam != NULL) )
    if ( (camH->state.task_callback != NULL) )//&& (sHisrCslCam != NULL) )
    {
	//OSSEMAPHORE_Release(sSemCslCamTask);	
 (*camH->state.task_callback)(camH->state.intr_status, camH->state.rx_status, camH->state.frame_buffer, jpgs , camH->state.raw_intr_status, camH->state.raw_rx_status, camH->state.task_cb_userdata);
    }
}

/***********************************************************
 * Name: cslCamFrameHisr
 * 
 * Arguments: 
 *       none
 *
 * Description: Csl Camera HISR, Triggered from LISR
 * note:        Static: Triggers CSL CAM Task.
 ***********************************************************/
static void cslCamFrameHisr( void )
{
    UInt32 frame_time;
    CAM_HANDLE camH;
    Boolean isr_defined = FALSE;
    Int32 sem_count;

// obtain semaphore to run Task
    camH = cslCamDrv.currHandle;

    frame_time = 0; // TIMER_GetValue(); // FIXME not sure if time is really required for processing 
    if (camH->state.FS_time == 0)
    {
        camH->state.FS_time = frame_time; 
        camH->state.FE_time = frame_time; 
        camH->state.LC_time = frame_time; 
    }

    if (camH->state.intr_status & CSL_CAM_INT_FRAME_START)
    {
        isr_defined = TRUE;
        printk(KERN_INFO  "[cslCamFrameHisr][Info]: FS: FrameRate=%d ms \n", (frame_time - camH->state.FS_time) );
        camH->state.FS_time = frame_time;
    }
    if ( camH->state.intr_status & CSL_CAM_INT_FRAME_END )
    {
        isr_defined = TRUE;
        printk(KERN_INFO  "[cslCamFrameHisr][Info]: FE: FrameRate=%d ms \n", (frame_time - camH->state.FE_time) );
        camH->state.FE_time = frame_time;
    }
    if ( camH->state.intr_status & CSL_CAM_INT_LINE_COUNT )
    {
        isr_defined = TRUE;
        printk(KERN_INFO  "[cslCamFrameHisr][Info]: LC: FrameRate=%d ms \n", (frame_time - camH->state.LC_time) );
        camH->state.LC_time = frame_time;
    }
    if ( camH->state.intr_status & CSL_CAM_INT_DATA )
    {
        isr_defined = TRUE;
        printk(KERN_INFO  "[cslCamFrameHisr][Info]: EDL: \n" );
    }
    if ( camH->state.intr_status & CHAL_CAM_INT_PKT )
    {
        isr_defined = TRUE;
        printk(KERN_INFO  "[cslCamFrameHisr][Info]: PACKET: \n" );
    }
    if ( isr_defined == FALSE )
    {
        printk(KERN_INFO  "[cslCamFrameHisr][Info]: Unknown: \n" );
//        DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( (CSL_CAM_HANDLE)camH ));
    }
    printk(KERN_INFO  "[cslCamFrameHisr][Info]: CAM_STA=0x%x CAM_ISTA=0x%x csl_rx=0x%x csl_int=0x%x time=%d \n", camH->state.raw_rx_status, camH->state.raw_intr_status, camH->state.rx_status, camH->state.intr_status, frame_time );
    if ( isr_defined == TRUE )
    {
        DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( (CSL_CAM_HANDLE)camH ));
    }
// Only Run Task if Task is Ready 
    sem_count = OSDAL_SEMA_COUNT(sSemCslCamTask);
    if (sem_count)
    {
        CSLCAM_DBG(CSLCAM_DBG_ID,  "[cslCamFrameHisr][Error]: sSemCslCamTask count != 0:  csl_rx=0x%x csl_int=0x%x time=%d \n", camH->state.rx_status, camH->state.intr_status, frame_time);
    }
    else if ( (sSemCslCamTask != NULL) && (sCslCamTask != NULL) )
    {
        printk(KERN_INFO  "[cslCamFrameHisr][Info]: sSemCslCamTask release: \n" );
        OSDAL_SENDEVENT( sSemCslCamTask ); //let task handle this state too.
    }
}
/** @} */


#if 0
/***********************************************************
 * Name: cslCamTaskEntry
 * 
 * Arguments: 
 *       none
 *
 * Description: Csl Camera Task, Called from HISR
 * note:        Static: Task Call-Back performed in this Task.
 ***********************************************************/
static void cslCamFrameTask( void )
{
    Boolean wait_event_status = TRUE;
    CAM_HANDLE camH;
    UInt32 intr_status, rx_status;
      printk("%s\n",__FUNCTION__);

    while( TRUE )
    {
      // obtain semaphore to run Task
	OSSEMAPHORE_Obtain(sSemCslCamTask,TICKS_FOREVER);
        camH = cslCamDrv.currHandle;
        if ( (camH->state.active == 1) && (cslCamDrv.intf_cfg.frame_time_out != 0) && (camH->state.task_callback != NULL) )
        {
         //   printk(KERN_INFO  "[cslCamFrameTask][Info]: sSemCslCamTask Obtain timeout=%d: \n", cslCamDrv.intf_cfg.frame_time_out );
            //wait_event_status = OSDAL_WAITEVENT_TIMEOUT( sSemCslCamTask, cslCamDrv.intf_cfg.frame_time_out );

        // Update and Clear status
            intr_status = camH->state.intr_status;
            rx_status = camH->state.rx_status;
            camH->state.intr_status = 0; 
            camH->state.rx_status = 0; 

            if ( (camH->state.active == 1) && (camH->state.task_callback != NULL) )
            {
                if (wait_event_status == FALSE) 
                {
                    intr_status |= CSL_CAM_INT_TASK_ERROR;
               //     printk(KERN_INFO  "[cslCamTaskEntry][Error] : wait_event_status FAILED \n" );
                }
                else
                {
                    if (cslCamFrameProcess( (CSL_CAM_HANDLE)camH ) == 0)
                    {
                        intr_status |= CSL_CAM_INT_TASK_ERROR;
                        printk(KERN_INFO  "[cslCamTaskEntry][Error] : cslCamFrameProcess FAILED \n" );
                    }
                }
       //Csl Task Call-back
               (*camH->state.task_callback)(intr_status, rx_status, camH->state.frame_buffer, camH->state.frame_size, camH->state.raw_intr_status, camH->state.raw_rx_status, camH->state.task_cb_userdata);
            }
        }
        else
        {
            printk(KERN_INFO  "[cslCamFrameTask][Info]: sSemCslCamTask Obtain TICKS_FOREVER: active=%d frame_time_out=%d callback=0x%x\n", camH->state.active,cslCamDrv.intf_cfg.frame_time_out,camH->state.task_callback);
            OSDAL_WAITEVENT( sSemCslCamTask );
        }
        DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( (CSL_CAM_HANDLE)camH ));
    }      
}            
#endif



/***********************************************************
 * Name: cslCamFrameProcess
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *
 * Description: Additional Frame Data Processing based on Interface and data format.
 * 
 ***********************************************************/
static UInt32 cslCamFrameProcess(CSL_CAM_HANDLE cslCamH)
{
//    Int32                       success = -1;                   //fail by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    UInt32 image_size;
#if ( defined (_ATHENA_) )
    UInt8 *src_ptr8, *dest_ptr8;
    UInt32 *src_ptr32,*dest_ptr32;
    UInt32 i;
    Boolean soi_flag,eoi_flag, sos_flag;
    UInt16 marker;
    UInt8 image_data, a5_data;
#endif

    printk(KERN_INFO  "[cslCamFrameProcess][Info] : size=0x%x ptr=0x%x\n",camH->state.frame_size,camH->state.frame_buffer );
    if ( (camH->state.frame_size == 0) || (camH->state.frame_buffer == NULL) )
    {
        printk(KERN_INFO  "[cslCamFrameProcess][Error] : size or pointer incorrect\n" );
        return 0;
    }

// All Processing from same buffer
    image_size = camH->state.frame_size;
#if ( defined (_ATHENA_) )
    src_ptr32 = (UInt32 *)camH->state.frame_buffer;
    dest_ptr32 = src_ptr32;
    src_ptr8 = (UInt8 *)src_ptr32;
    dest_ptr8 = (UInt8 *)dest_ptr32;


// FSP Check Not Working in ATHENA)
    if ( (camH->state.pipeline_cfg.decode == CSL_CAM_FSP_DEC) && ((cslCamDrv.intf_cfg.intf == CSL_CAM_INTF_CCP) || (cslCamDrv.intf_cfg.intf == CSL_CAM_INTF_CSI)) )
    {
        printk(KERN_INFO  "[cslCamFrameProcess][Info] : JPEG FSP Check: size=0x%x Rd=0x%x Wr=0x%x \n",image_size,src_ptr8,dest_ptr8 );
        soi_flag = FALSE;
        eoi_flag = FALSE;
        sos_flag =  FALSE;
        marker = 0;
        i = 0;
        while ( (i < camH->state.frame_size) && (eoi_flag == FALSE) )
        {
            image_data = *src_ptr8++;
            marker = (marker << 8) | image_data;
            if (marker == 0xFFD8)
            {
                soi_flag = TRUE;
                printk(KERN_INFO  "[cslCamFrameProcess][Info] : JPEG SOI OK \n" );
            }
            else if (marker == 0xFFDA)
            {
                sos_flag = TRUE;
                printk(KERN_INFO  "[cslCamFrameProcess][Info] : JPEG SOS OK \n" );
            }
            else if ( ((soi_flag == TRUE) && (sos_flag == TRUE)) && (marker == 0xFFD9) )
            {
                eoi_flag = TRUE;
                image_size = i;
            }
            else if ( (sos_flag == TRUE) && (marker == 0xFF00) )
            {
                a5_data = *src_ptr8;     // check A5 data
                if (a5_data == 0xA5)
                {
                    printk(KERN_INFO  "[cslCamFrameProcess][Info] : JPEG FSP A5 removed bytes=0x%x Rd=0x%x Wr=0x%x\n", i,src_ptr8,dest_ptr8 );
                    a5_data = *src_ptr8++; // throw out A5
                }
                else
                {
                    printk(KERN_INFO  "[cslCamFrameProcess][Info] : JPEG FSP FAILED: bytes=0x%x Rd=0x%x Wr=0x%x\n", i,src_ptr8,dest_ptr8 );
                }
            }
            *dest_ptr8++ = image_data;
            i++;
        }   // while ( (i < image_size) && (eoi_flag == FALSE) )
        if (eoi_flag == FALSE)
        {
            printk(KERN_INFO  "[cslCamFrameProcess][Info] : JPEG EOI FAILED: bytes=0x%x Rd=0x%x Wr=0x%x\n", i,src_ptr8,dest_ptr8 );
        }
    }
#endif
    return image_size;
}
/** @} */


/******************************************************************************
 Global Functions
 *****************************************************************************/


 /***********************************************************
 * Name: csl_cam_init
 * 
 * Arguments: 
 *       void
 *
 * Description: Routine used to initialise the csi2/ccp2/cpi driver & camera interface pins
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_init( void )
{
    Int32 success = 0;          //pass by default
    UInt32 instance;
   
// Set interface pointer
    if ( cslCamDrv.init != 1 )
    {
        memset( &cslCamDrv, 0, sizeof( CSL_CAM_DRV_st ) );
    // Set IOCR in Init state
    #if 0
        if ( cslCamIocrSet( (CSL_CAM_INTF_T)NULL, (CSL_CAM_PORT_AFE_T)NULL, CSL_CAM_INIT_MODE ) )
        {
            printk(KERN_INFO  "[csl_cam_init][Error] :  cslCamIocrSet() FAILED \n" );
            return -1;
        }
	#endif
        
    // Assign Port Channels for each handle        
        for (instance=0; instance < CAM_MAX_HANDLES; instance++)
        {
            cslCamDrv.camH[instance].port_chan_sel = (CSL_CAM_PORT_CHAN_T)(1 << instance);
        }
                
        if ( cslCamTaskInit() == 0 )
        {
        // Init CHAL
            cslCamDrv.chalCamH = (CHAL_HANDLE) chal_cam_init(BCM21553_CAMINTF_BASE1);
            if (cslCamDrv.chalCamH == NULL)
            {
                printk(KERN_INFO  "[csl_cam_init][Error] : chal_cam_init() FAILED \n" );
                success |= -1;
            }
            else
            {
                cslCamDrv.init = 1;
            #ifdef ENABLE_PEDESTAL_DEEPSLEEP            
                if ( sCslCamDeepSleepId == -1 )
                {          
                    sCslCamDeepSleepId = SLEEP_AllocId();
                }
                if ( sCslCamPedestalId == -1 )
                {          
                    sCslCamPedestalId = PEDESTAL_AllocId();
                }
            #endif
                printk(KERN_INFO  "[csl_cam_init]: Ok \n" );
            }
        }
        else
        {
            success |= -1;
            printk(KERN_INFO  "[csl_cam_init][Error] : cslCamTaskInit() FAILED \n" );
        }    
    }    
    return success;
}


 /***********************************************************
 * Name: csl_cam_exit
 * 
 * Arguments: 
 *       void
 *
 * Description: Routine used to exit the driver
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_exit( void )
{
    Int32 success = 0;  //pass by default
   
    if (cslCamDrv.init)
    {
        chal_cam_deinit(cslCamDrv.chalCamH);
    // Remove Task
    #if 0
        if ( cslCamTaskDeInit() == 0 )
        {
            printk(KERN_INFO  "[csl_cam_exit]: cslCamTaskDeInit]: Ok \n" );
        }
        else
        {
            printk(KERN_INFO  "[csl_cam_exit][ERROR]:  cslCamTaskDeInit \n" );
            success = -1;
        }
	#endif
        cslCamDrv.init = 0;
    } 
    printk(KERN_INFO  "[csl_cam_exit][Info] : exit: \n" );
    return success;
}

 /***********************************************************
 * Name: csl_cam_info
 * 
 * Arguments: 
 *       char **driver_name,
         UInt32 *version_major,
         UInt32 *version_minor
         DRIVER_FLAGS_T *flags
 *
 * Description: Routine used to get the driver info
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_info( const char **driver_name,
                        UInt32 *version_major,
                        UInt32 *version_minor )
{
    Int32 success = -1; //fail by default

   //return the driver name   
    if ( ( NULL != driver_name ) && ( NULL != version_major ) && ( NULL != version_minor ) )
    {
            *driver_name = "csl_cam";
            *version_major = 0;
            *version_minor = 1;
            success = 0;
    }

    return success;
}
                              
 /***********************************************************
 * Name: csl_cam_open
 * 
 * Arguments: 
 *       const void *params - optional parameters, defined per driver
         DRIVER_HANDLE_T *handle - returned handle to the device
 *
 * Description: Routine used to open a driver and get a handle 
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_open(  pCSL_CAM_INTF_CFG_st intfCfg, CSL_CAM_HANDLE* cslCamH )
{
    
    Int32           success = -1;                       //fail by default
    UInt32          instance = 0;
    CAM_HANDLE      camH = NULL;    
    
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_CFG_INTF_st_t      chal_cam_intf_st;
    CHAL_CAM_CFG_CNTRL_st_t     chal_cam_cfg_cntrl_st;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;

    memset(&chal_cam_cfg_cntrl_st, 0, sizeof(CHAL_CAM_CFG_CNTRL_st_t));
#if 0
#if ( defined (_ATHENA_) )
    CHAL_CLK_CFG_MIPIDSIAFE_T   afe_clk_cfg; 
    CHAL_CLK_CFG_MIPIDSI_CMI_T  mipi_dsi_cmi_clk_cfg;
#endif
#endif
    printk(KERN_INFO  "[csl_cam_open][Info] : Start! \n" );
    
// need to guard against NULL params being passed
    *cslCamH = NULL;

    if( intfCfg )
    {
        if ( cslCamDrv.init != 1 )
        {
            printk(KERN_INFO  "[csl_cam_open][Error] : Not Initiailized \n" );
            return ( success );
        }
    // Check for maximum channels already opened
        if (cslCamDrv.use_count == CAM_MAX_HANDLES)
        {
            printk(KERN_INFO  "[csl_cam_open][Error] : MAX handles allocated! \n" );
            return ( success );
        }    

    // If Channel opened, check Interface & AFE port is same
        if ( cslCamDrv.use_count && (cslCamDrv.intf_cfg.intf != intfCfg->intf) && (cslCamDrv.intf_cfg.afe_port != intfCfg->afe_port) )
        {
            printk(KERN_INFO  "[csl_cam_open][Error] : Channel opened, Interface must be same! \n" );
            return ( success );
        }    

    // Get available channel
        instance = 0;
        while ( (instance < CAM_MAX_HANDLES) && (camH == NULL) )
        {
            if ( cslCamDrv.camH[instance].open == 0 )
            { 
                camH = &cslCamDrv.camH[instance];
                cslCamDrv.camH[instance].open = 1;
                printk(KERN_INFO  "[csl_cam][Info] : csl_cam_open(): Instance=0x%x \n", instance );
            }
            instance++;
        }
    // Verify channel available
        if (camH == NULL)
        {            
            printk(KERN_INFO  "[csl_cam_open][Error] : No Channels available! \n" );
            return ( success );
        }
    // Clear state for channel selected
        memset( &camH->state, 0, sizeof( CSL_CAM_STATE_st_t ) );
        
    // If Initial Channel opening on Interface
        cslCamDrv.use_count++;
        if (cslCamDrv.use_count == 1)
        {
        // Acquire Clock Handle if not Open 
#if ( defined (_ATHENA_) )
        // Clock open was here
#endif                

        // Update Interface Info
            memcpy( &cslCamDrv.intf_cfg, intfCfg, sizeof(CSL_CAM_INTF_CFG_st_t) );

            printk(KERN_INFO  "[csl_cam][Info] : csl_cam_open(): intf=0x%x afe=0x%x chan=0x%x input=0x%x time_out=0x%x \n", 
                                cslCamDrv.intf_cfg.intf,
                                cslCamDrv.intf_cfg.afe_port,
                                camH->port_chan_sel,
                                cslCamDrv.intf_cfg.input_mode,
                                cslCamDrv.intf_cfg.frame_time_out );
#if 0
        // Set IOCR in open state
            if ( cslCamIocrSet( cslCamDrv.intf_cfg.intf, cslCamDrv.intf_cfg.afe_port, CSL_CAM_OPEN_MODE ) )
            {
                printk(KERN_INFO  "[csl_cam_open][Error] :  cslCamIocrSet() FAILED \n" );
                return -1;
            }
#endif
#if ( defined (_ATHENA_) )
	#if 0
        // Set DSI Clock Registers
            mipi_dsi_cmi_clk_cfg.speedMIPIDSI = CLK_MIPI_156MHZ;
            mipi_dsi_cmi_clk_cfg.speedCMI = 0;
            if (CLKDRV_Set_Clock(sClkHandle, CLK_MIPIDSI_CMI, (void *)&mipi_dsi_cmi_clk_cfg) != CLKDRV_STATUS_OK)
            {
                printk(KERN_INFO  "[csl_cam][Info] : csl_cam_open(): CLKDRV_Set_Clock(CLK_MIPIDSI_CMI): ERROR \n" );
            }
        // Set AFE Clock Registers
            afe_clk_cfg.dsiDiv = 4;
            afe_clk_cfg.cam2Div = 4;
            afe_clk_cfg.cam1Div = 4;
            if (CLKDRV_Set_Clock(sClkHandle, CLK_MIPIDSI_AFE, (void *)&afe_clk_cfg) != CLKDRV_STATUS_OK)
            {
                printk(KERN_INFO  "[csl_cam][Info] : csl_cam_open(): CLKDRV_Set_Clock(CLK_MIPIDSI_AFE): ERROR \n" );
            }
        // Start MIPI Clock
            if (CLKDRV_Start_Clock(sClkHandle, CLK_MIPIDSI_CMI) != CLKDRV_STATUS_OK)
            {
                printk(KERN_INFO  "[csl_cam][Info] : csl_cam_open(): CLKDRV_Start_Clock(CLK_MIPIDSI_CMI): ERROR \n" );
            }
#endif
            // Set Camera Interface Clock Speed & Enable clock
                //  2:0 clk_camrx_mode  0x1 RW
                //  clk_camrx frequency selection
                //      000 reserved
                //      001 208Mhz
                //      010 156Mhz
                //      011 104Mhz
                //      1xx reserved
                //  Note: clk_camrx_mode should be modified only when clk_camrx is disabled (clk_camrx_enable is 0)    
		OSDAL_SYSCLK_Enable(OSDAL_SYSCLK_ENABLE_CAMARA_IF);
		if(camclk == (OSDAL_CLK_HANDLE)NULL){
		}
		
		camsys = OSDAL_CLK_Open(OSDAL_CLK_CAMINTF_CAMSYS);
		if(camsys == (OSDAL_CLK_HANDLE)NULL){
		}
		
		camrx = OSDAL_CLK_Open(OSDAL_CLK_CAMINTF_CAMRX);
		if(camrx == (OSDAL_CLK_HANDLE)NULL){
		}
		if(cslCamDrv.intf_cfg.input_mode == CSL_CAM_INPUT_DUAL_LANE){
			/*
                        int val1 = 0;
                        writel(val1,io_p2v(0x0814012C));

                        val1 = 0x244;
                        writel(val1,io_p2v(0x0814013C));

                        val1 = 0x1;
                        writel(val1,io_p2v(0x0814012C)); */
			mipidsi = OSDAL_CLK_Open(OSDAL_CLK_MIPIDSI_AFE);
			OSDAL_CLK_Stop(mipidsi);
			clk_set_mipidsi_afe_div(CAM2_CLK_DIV,4);
			OSDAL_CLK_Start(mipidsi);
			OSDAL_CLK_Set_Config(camrx, OSDAL_CLK_CAMINTF_CAMRX_208MHZ, NULL); //FIXME
                } else {
			OSDAL_CLK_Set_Config(camrx, OSDAL_CLK_CAMINTF_CAMRX_104MHZ, NULL); //FIXME
		}
		OSDAL_CLK_Start(camsys);
		OSDAL_CLK_Start(camrx);
#endif  

        // Set Interface
            chal_cam_intf_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
            switch (cslCamDrv.intf_cfg.input_mode)
            {
                case CSL_CAM_INPUT_MODE_DATA_CLOCK:
                    chal_cam_intf_st.clk_mode = CHAL_CAM_DATA_CLOCK;
                    chal_cam_intf_st.input_mode = (CHAL_CAM_INPUT_MODE_t)0;
                    break;
                case CSL_CAM_INPUT_MODE_DATA_STROBE:
                    chal_cam_intf_st.clk_mode = CHAL_CAM_DATA_STROBE;
                    chal_cam_intf_st.input_mode = (CHAL_CAM_INPUT_MODE_t)0;
                    break;
                case CSL_CAM_INPUT_SINGLE_LANE:
                    chal_cam_intf_st.clk_mode = (CHAL_CAM_CLOCK_MODE_t)0;
                    chal_cam_intf_st.input_mode = CHAL_CAM_INPUT_SINGLE_LANE;
                    break;
                case CSL_CAM_INPUT_DUAL_LANE:
                    chal_cam_intf_st.clk_mode = (CHAL_CAM_CLOCK_MODE_t)0;
                    chal_cam_intf_st.input_mode = CHAL_CAM_INPUT_DUAL_LANE;
                    break;
                default:
                    chal_cam_intf_st.clk_mode = (CHAL_CAM_CLOCK_MODE_t)0;
                    chal_cam_intf_st.input_mode = (CHAL_CAM_INPUT_MODE_t)0;
                    break;
            }
        // Set CPI Configuration Pointer
            chal_cam_intf_st.p_cpi_cfg_st = cslCamDrv.intf_cfg.p_cpi_intf_st;
            chal_status |= chal_cam_cfg_intf(cslCamDrv.chalCamH, &chal_cam_intf_st);
            
        // Power Up Analog
            chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
            chal_cam_param_st.param = TRUE;
            chal_cam_param_st.select = CHAL_CAM_BANDGAP;
            chal_status |= chal_cam_set_bandgap_pwr(cslCamDrv.chalCamH, &chal_cam_param_st);
            chal_cam_param_st.select = CHAL_CAM_ANALOG;
            chal_status |= chal_cam_set_analog_pwr(cslCamDrv.chalCamH, &chal_cam_param_st);
        // Set default Controller settings                        
            chal_cam_cfg_cntrl_st.packet_timeout = 16;
            if (cslCamDrv.intf_cfg.input_mode == CHAL_CAM_INPUT_DUAL_LANE)
            {
                chal_cam_cfg_cntrl_st.packet_timeout = 32;
            }
            chal_cam_cfg_cntrl_st.line_start_suppress = FALSE;
            chal_cam_cfg_cntrl_st.burst_length = (CHAL_CAM_BURST_LENGTH_t)0;
            chal_cam_cfg_cntrl_st.burst_space = (CHAL_CAM_BURST_SPACE_t)0;

            chal_cam_cfg_cntrl_st.panic_pr = 0xC;
            chal_cam_cfg_cntrl_st.norm_pr = 0x1;
            chal_cam_cfg_cntrl_st.panic_thr = 0;
            chal_cam_cfg_cntrl_st.panic_enable = FALSE;
            chal_status |= chal_cam_cfg_cntrl(cslCamDrv.chalCamH, &chal_cam_cfg_cntrl_st);

        /* Register the CSI2/CCP2 interrupt handler */
            if (cslCamDrv.int_registered == 0)
            {
        #if defined (_ATHENA_)
	/* Interrupts over here are registered i hal_cam_core.c
 	* The HISR and LISR shall be used as processing functions
 	* Mostly LISR shall be called from the ISR
 	* HISR shall be queued up as a workqueue that does bulk of the
 	* processing */
        #endif    
        #if ( defined (_HERA_) || defined (_RHEA) )
                cslCamDrv.int_id = (InterruptId_t)MM_CSI_IRQ;
        #endif    
            	cslCamDrv.int_registered = TRUE;
            }
        // Cfg Lane Enable 
            chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
            chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
            chal_cam_param_st.lane = (CHAL_CAM_LANE_SELECT_t)(CHAL_CAM_DATA_LANE_0 | CHAL_CAM_CLK_LANE);
            if (cslCamDrv.intf_cfg.input_mode == CSL_CAM_INPUT_DUAL_LANE)
            {
                chal_cam_param_st.lane |= CHAL_CAM_DATA_LANE_1;
            }
            chal_cam_param_st.param = chal_cam_param_st.lane;
            chal_status |= chal_cam_lane_enable(cslCamDrv.chalCamH, &chal_cam_param_st);
            
        #if 0
        // Enable Clock Termination
            {
                CHAL_CAM_LANE_CNTRL_st_t    chal_cam_lane_cntrl_st;

                chal_cam_lane_cntrl_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
                chal_cam_lane_cntrl_st.chan = cslCamChalPortChan(camH->port_chan_sel);
                chal_cam_lane_cntrl_st.lane = CHAL_CAM_CLK_LANE;
                chal_cam_lane_cntrl_st.cntrl_state = CHAL_CAM_LANE_TERM_EN;
                chal_cam_lane_cntrl_st.param = 1;
                chal_status |= chal_cam_set_lane_cntrl(cslCamDrv.chalCamH, &chal_cam_lane_cntrl_st);
            }
        #endif
        }   // if (cslCamDrv.use_count == 1)

        if (chal_status == CHAL_OP_OK)
        {
            cslCamDrv.currHandle = camH;
        // success!
            success = 0;
        // assign handle
            *cslCamH = (CSL_CAM_HANDLE)camH;
        #ifdef ENABLE_PEDESTAL_DEEPSLEEP            
            SLEEP_DisableDeepSleep(sCslCamDeepSleepId);
            PEDESTAL_DisablePedestalMode(sCslCamPedestalId);
        #endif
        }
        else
        {
            printk(KERN_INFO  "[csl_cam_open][Error] :  chal_cam_xxx: FAILED \n" );
        }
    }
    else
    {
        printk(KERN_INFO  "[csl_cam_open][Error] :  params: FAILED \n" );
    }

// Return Clock Handle
    if (camH != NULL)
    {            
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( (CSL_CAM_HANDLE)camH ) );
    }
    return success;
}

 /***********************************************************
 * Name: csl_cam_close
 * 
 * Arguments: 
 *       const DRIVER_HANDLE_T handle - handle to close
 *
 * Description: Routine used to close a handle to the driver
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_close( CSL_CAM_HANDLE cslCamH )
{
    Int32 success = 0; //pass by default
    CAM_HANDLE camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;

// Decrement # handles for interface in use
    if (cslCamDrv.use_count != 0)
    {
        cslCamDrv.use_count--;
    }
// Make sure channel is stopped
    csl_cam_rx_stop(cslCamH);
// If all handles closed on this interface, close everything
    if (cslCamDrv.use_count == 0)
    {
        if ( csl_cam_reset( cslCamH, CSL_CAM_RESET_SWR ) )
        {
            printk(KERN_INFO  "[csl_cam_close][Error] :  csl_cam_reset(): CSL_CAM_RESET_SWR: FAILED \n" );
            success |= -1;
        }
        if ( csl_cam_reset( cslCamH, CSL_CAM_RESET_ARST ) )
        {
            printk(KERN_INFO  "[csl_cam_close][Error] :  csl_cam_reset(): CSL_CAM_RESET_ARST: FAILED \n" );
            success |= -1;
        }
    // Disable all Lanes  
        chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
        chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
        chal_cam_param_st.lane = (CHAL_CAM_LANE_SELECT_t)(CHAL_CAM_DATA_LANE_0 | CHAL_CAM_DATA_LANE_1 | CHAL_CAM_CLK_LANE);
        chal_cam_param_st.param = (CHAL_CAM_LANE_SELECT_t)0x0;
        if (chal_cam_lane_enable(cslCamDrv.chalCamH, &chal_cam_param_st) != CHAL_OP_OK)
        {
            printk(KERN_INFO  "[csl_cam_close][Error] : ERROR!\n" );
            success |= -1;
        }

    // Power Down Analog
        chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
        chal_cam_param_st.param = FALSE;
        chal_cam_param_st.select = CHAL_CAM_BANDGAP;
        if (chal_cam_set_analog_pwr(cslCamDrv.chalCamH, &chal_cam_param_st) != CHAL_OP_OK)
        {
            printk(KERN_INFO  "[csl_cam_close][Error] :  chal_cam_set_analog_pwr(): CHAL_CAM_ANALOG: FAILED \n" );
            success |= -1;
        }
        chal_cam_param_st.select = CHAL_CAM_ANALOG;
        if (chal_cam_set_analog_pwr(cslCamDrv.chalCamH, &chal_cam_param_st) != CHAL_OP_OK)
        {
            printk(KERN_INFO  "[csl_cam_close][Error] :  chal_cam_set_analog_pwr(): CHAL_CAM_ANALOG: FAILED \n" );
            success |= -1;
        }
            
        
        /* Have we been asked to register a callback? */
        if (cslCamDrv.int_registered == TRUE)
        {
            cslCamDrv.int_registered = FALSE;
        }
    // Set IOCR in closed state
    #if 0
        if ( cslCamIocrSet( cslCamDrv.intf_cfg.intf, cslCamDrv.intf_cfg.afe_port, CSL_CAM_CLOSE_MODE ) )
        {
            printk(KERN_INFO  "[csl_cam_close][Error] :  cslCamIocrSet() FAILED \n" );
        }
	#endif
#if ( defined (_ATHENA_) )
    // Disable Clocks
    	OSDAL_CLK_Stop(camclk);
    	OSDAL_CLK_Stop(camsys);
    	OSDAL_CLK_Stop(camrx);
	if(mipidsi != NULL)
		OSDAL_CLK_Stop(mipidsi);

	OSDAL_CLK_Close(camclk);
	OSDAL_CLK_Close(camsys);
	OSDAL_CLK_Close(camrx);
	if(mipidsi != NULL)
		OSDAL_CLK_Close(mipidsi);
	camclk = NULL;
	camrx = NULL;
	camsys = NULL;
	mipidsi = NULL;
#endif

    #ifdef ENABLE_PEDESTAL_DEEPSLEEP 
        SLEEP_EnableDeepSleep(sCslCamDeepSleepId);
        PEDESTAL_EnablePedestalMode(sCslCamPedestalId);
    #endif
    }
// close handle
    camH->open = 0;
    memset( &camH->state, 0, sizeof( CSL_CAM_STATE_st_t ) );
    OSDAL_SYSCLK_Disable(OSDAL_SYSCLK_ENABLE_CAMARA_IF);
    return success;
}


 /***********************************************************
 * Name: csl_cam_get_handle
 * 
 * Arguments: 
 *       DRIVER_HANDLE_T *handle - return current handle to the device
 *
 * Description: Routine used to get current handle opened by a driver
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_get_handle( CSL_CAM_HANDLE* cslCamH )
{
    
    Int32           success = -1;                       //fail by default
    
    *cslCamH = NULL;

    if (cslCamDrv.currHandle != NULL)
    {
    // assign handle
        *cslCamH = cslCamDrv.currHandle;
        success = 0;
    }
    else
    {
        printk(KERN_INFO  "[csl_cam_get_handle][Error] :  No Handle available \n" );
    }
    return success;
}


/***********************************************************
 * Name: csl_cam_set_intf_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *       pCSL_CAM_INTF_CNTRL_st intfCtrl - Interface configuration structure
 *
 * Description: Routine used to set Interface Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_intf_control( CSL_CAM_HANDLE cslCamH, pCSL_CAM_INTF_CNTRL_st intfCtrl )
{
    Int32                       success = -1;                   //fail by default
//    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_CFG_CNTRL_st_t     chal_cam_cfg_cntrl_st;

// Interface
    chal_cam_cfg_cntrl_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Packet timeout
    chal_cam_cfg_cntrl_st.packet_timeout = (UInt32)intfCtrl->packet_timeout;
// Line Start
    chal_cam_cfg_cntrl_st.line_start_suppress = intfCtrl->line_start_suppress;
// Burst
    chal_cam_cfg_cntrl_st.burst_length = (CHAL_CAM_BURST_LENGTH_t)intfCtrl->burst_length;
    chal_cam_cfg_cntrl_st.burst_space = (CHAL_CAM_BURST_SPACE_t)intfCtrl->burst_space;
// Panic
    chal_cam_cfg_cntrl_st.norm_pr = (UInt32)intfCtrl->norm_pr;
    chal_cam_cfg_cntrl_st.panic_pr = (UInt32)intfCtrl->panic_pr;
    chal_cam_cfg_cntrl_st.panic_thr = (UInt32)intfCtrl->panic_thr;
    chal_cam_cfg_cntrl_st.panic_enable = intfCtrl->panic_enable;
    chal_status |= chal_cam_cfg_cntrl(cslCamDrv.chalCamH, &chal_cam_cfg_cntrl_st);
    if (chal_status == CHAL_OP_OK)
    {
        success = 0;
    }
    else
    {
        printk(KERN_INFO  "[csl_cam_set_intf_control][Info] : ERROR!\n" );
    }    
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}

/***********************************************************
 * Name: csl_cam_set_input_mode
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_INTF_CFG_st intfCfg - intf input cfg
 *
 * Description: Routine used to set Interface Input mode
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_input_mode( CSL_CAM_HANDLE cslCamH, pCSL_CAM_INPUT_st inputMode )
{
    
    Int32                       success = 0;                    //pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_CFG_INTF_st_t      chal_cam_intf_st;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;

    printk(KERN_INFO  "[csl_cam_set_input_mode][Info] : mode = 0x%x\n", inputMode->input_mode );

// Update Frame Time-out
    if (inputMode->frame_time_out != 0)
    {
        cslCamDrv.intf_cfg.frame_time_out = inputMode->frame_time_out;
    }

// Interface
    chal_cam_intf_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);

// Set Interface
    switch (inputMode->input_mode)
    {
        case CSL_CAM_INPUT_MODE_DATA_CLOCK:
            chal_cam_intf_st.clk_mode = CHAL_CAM_DATA_CLOCK;
            chal_cam_intf_st.input_mode = (CHAL_CAM_INPUT_MODE_t)0;
            break;
        case CSL_CAM_INPUT_MODE_DATA_STROBE:
            chal_cam_intf_st.clk_mode = CHAL_CAM_DATA_STROBE;
            chal_cam_intf_st.input_mode = (CHAL_CAM_INPUT_MODE_t)0;
            break;
        case CSL_CAM_INPUT_SINGLE_LANE:
            chal_cam_intf_st.clk_mode = (CHAL_CAM_CLOCK_MODE_t)0;
            chal_cam_intf_st.input_mode = CHAL_CAM_INPUT_SINGLE_LANE;
            break;
        case CSL_CAM_INPUT_DUAL_LANE:
            chal_cam_intf_st.clk_mode = (CHAL_CAM_CLOCK_MODE_t)0;
            chal_cam_intf_st.input_mode = CHAL_CAM_INPUT_DUAL_LANE;
            break;
        default:
            chal_cam_intf_st.clk_mode = (CHAL_CAM_CLOCK_MODE_t)0;
            chal_cam_intf_st.input_mode = (CHAL_CAM_INPUT_MODE_t)0;
            break;
    }
// Set CPI Configuration Pointer
    chal_cam_intf_st.p_cpi_cfg_st = inputMode->p_cpi_intf_st;
    chal_status |= chal_cam_set_intf(cslCamDrv.chalCamH, &chal_cam_intf_st);

// Cfg Lane Enables 
    chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
    chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
    chal_cam_param_st.lane = (CHAL_CAM_LANE_SELECT_t)(CHAL_CAM_DATA_LANE_0 | CHAL_CAM_CLK_LANE);
    if (inputMode->input_mode == CSL_CAM_INPUT_DUAL_LANE)
    {
        chal_cam_param_st.lane |= CHAL_CAM_DATA_LANE_1;
    }
    chal_cam_param_st.param = chal_cam_param_st.lane;
    chal_status |= chal_cam_lane_enable(cslCamDrv.chalCamH, &chal_cam_param_st);

    if (chal_status != CHAL_OP_OK)
    {
        printk(KERN_INFO  "[csl_cam_set_input_mode][Info] : ERROR!\n" );
        success |= -1;
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));

    return success;
}

/***********************************************************
 * Name: csl_cam_set_input_addr
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_BUFFER_st imageAddr_0 - image_0 buffer select
 *       pCSL_CAM_BUFFER_st imageAddr_1 - image_1 buffer select
 *       pCSL_CAM_BUFFER_st dataAddr - data buffer select
 *
 * Description: Routine used to set Interface Addresses Before Rx starts
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_input_addr( CSL_CAM_HANDLE cslCamH, pCSL_CAM_BUFFER_st imageAddr_0, pCSL_CAM_BUFFER_st imageAddr_1, pCSL_CAM_BUFFER_st dataAddr )
{
    Int32                       success = 0;                    //pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CSL_CAM_Buffers_st_t buffers;



      // Set Image buffer
    if (imageAddr_0 != NULL)
    {   
        printk(KERN_INFO  "[csl_cam_set_input_addr][Info] : Image0 -> addr=0x%x, size=%d, line_stride=%d, buffer_wrap_en=%d \n", 
                           imageAddr_0->start_addr, imageAddr_0->size, imageAddr_0->line_stride, imageAddr_0->buffer_wrap_en );  
        memcpy( &camH->state.image_buffer_0, imageAddr_0, sizeof(CSL_CAM_BUFFER_st_t) );        
        if (imageAddr_1 != NULL)
        {
            memcpy( &camH->state.image_buffer_1, imageAddr_1, sizeof(CSL_CAM_BUFFER_st_t) );   
            printk(KERN_INFO  "[csl_cam_set_input_addr][Info] : Image1 -> addr=0x%x, size=%d, line_stride=%d, buffer_wrap_en=%d \n", 
                           imageAddr_1->start_addr, imageAddr_1->size, imageAddr_1->line_stride, imageAddr_1->buffer_wrap_en );  
        }
        else
        {    
            printk(KERN_INFO  "[csl_cam_set_input_addr][Info] : Image1 -> NULL \n"); 
            memset( &camH->state.image_buffer_1, 0, sizeof( CSL_CAM_BUFFER_st_t ) );            
        }
    }
    else
    {        
       printk(KERN_INFO  "[csl_cam_set_input_addr][Error] : ERROR! imageAddr_0 is NULL. No image buffers \n" );    
       success |= -1;
    }

    // Set Data Addr
    if (dataAddr != NULL)
    {
        memcpy( &camH->state.data_buffer, dataAddr, sizeof(CSL_CAM_BUFFER_st_t) );            

    }
    else
    {
        memset( &camH->state.data_buffer, 0, sizeof( CSL_CAM_BUFFER_st_t ) );
    }

   
    camH->state.bufferIndex = 0; // set current buffer index.

    buffers.image0Buff = imageAddr_0;
    buffers.image1Buff = imageAddr_1;    
    buffers.dataBuff   = dataAddr;    
    
    printk(KERN_INFO  "[csl_cam_set_input_addr][Info] : Image0 addr=0x%x, Image1 addr=0x%x, dataAddr=0x%x \n", camH->state.image_buffer_0.start_addr, camH->state.image_buffer_1.start_addr, camH->state.data_buffer.start_addr );  
    
    success |= csl_cam_set_buffer_params(cslCamH, buffers);    
    if (success)
    {
        printk(KERN_INFO  "[csl_cam_set_input_addr][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}


/***********************************************************
 * Name: csl_cam_set_buffer_params
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       CAM_SELECT_t buffer_select - buffer select
 *       pCSL_CAM_BUFFER_st bufParams - buffer configuration structure
 *
 * Description: Routine used to set Buffer Interface values every Frame
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
static Int32 csl_cam_set_buffer_params( CSL_CAM_HANDLE cslCamH, CSL_CAM_Buffers_st_t buffers)
{
    Int32                       success = -1;                   //fail by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_BUFFER_CFG_st_t    chal_cam_buffer_cfg_st;
    CHAL_CAM_BUFFER_st_t        image0, image1, data;

// Interface
    chal_cam_buffer_cfg_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_buffer_cfg_st.chan = cslCamChalPortChan(camH->port_chan_sel);

// Set Image buffer
    if (buffers.image0Buff != NULL)
    {
        image0.start_addr      = buffers.image0Buff->start_addr;  
        image0.size            = buffers.image0Buff->size;  
        image0.line_stride     = buffers.image0Buff->line_stride;          
        image0.buf_wrap_enable = buffers.image0Buff->buffer_wrap_en;
    // ACP Compatible Memory Configuration  
        if (buffers.image0Buff->mem_type == CSL_CAM_MEM_TYPE_ACP_COMPATIBLE)
        {
            image0.start_addr |= (CSL_CAM_MEM_ACP_TYPE);
        }

        chal_cam_buffer_cfg_st.buffers.image0Buff = &image0;
        if (buffers.image1Buff != NULL)
        {          
            image1.start_addr      = buffers.image1Buff->start_addr;  
            image1.size            = buffers.image1Buff->size;  
            image1.line_stride     = buffers.image1Buff->line_stride;          
            image1.buf_wrap_enable = buffers.image1Buff->buffer_wrap_en;  
        // ACP Compatible Memory Configuration  
            if (buffers.image1Buff->mem_type == CSL_CAM_MEM_TYPE_ACP_COMPATIBLE)
            {
                image1.start_addr |= (CSL_CAM_MEM_ACP_TYPE);
            }
            chal_cam_buffer_cfg_st.buffers.image1Buff = &image1;
        }
        else
        {
            chal_cam_buffer_cfg_st.buffers.image1Buff = NULL;  
        }
    }
    else
    {
       printk(KERN_INFO  "[csl_cam_set_input_addr][Error] : ERROR! imageAddr_0 is NULL. No image buffers \n" );    
       success |= -1;
       chal_cam_buffer_cfg_st.buffers.image0Buff = NULL;
       chal_cam_buffer_cfg_st.buffers.image1Buff = NULL;
       chal_cam_buffer_cfg_st.buffers.dataBuff   = NULL;
    }
        
    // Set Data Addr
    if (buffers.dataBuff != NULL)
    {       
        data.start_addr      =  buffers.dataBuff->start_addr;  
        data.size            =  buffers.dataBuff->size;  
        data.line_stride     =  buffers.dataBuff->line_stride;          
        data.buf_wrap_enable =  buffers.dataBuff->buffer_wrap_en;  
    // ACP Compatible Memory Configuration  
        if (buffers.dataBuff->mem_type == CSL_CAM_MEM_TYPE_ACP_COMPATIBLE)
        {
            data.start_addr |= (CSL_CAM_MEM_ACP_TYPE);
        }
        chal_cam_buffer_cfg_st.buffers.dataBuff = &data;

    }
    else
    {
        chal_cam_buffer_cfg_st.buffers.dataBuff = NULL;
    }
    chal_status |= chal_cam_cfg_buffer(cslCamDrv.chalCamH, &chal_cam_buffer_cfg_st);
    if (chal_status == CHAL_OP_OK)
    {
        success = 0;
    }
    else
    {
        printk(KERN_INFO  "[csl_cam_set_buffer_params][Info] : ERROR!\n" );
    }    

    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}

/***********************************************************
 * Name: csl_cam_set_frame_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_FRAME_st frameCtrl - frame interrupt configuration
 *
 * Description: Routine used to set Frame Input Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_frame_control( CSL_CAM_HANDLE cslCamH, pCSL_CAM_FRAME_st frameCtrl )
{
    Int32                       success = 0;                    //pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_FRAME_CFG_st_t     chal_cam_frame_cfg_st;

// Update handle
    memcpy( &camH->state.frame_cfg, frameCtrl, sizeof(CSL_CAM_FRAME_st_t) );
// Interface
    chal_cam_frame_cfg_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_frame_cfg_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Interrupts
    chal_cam_frame_cfg_st.interrupts = (CHAL_CAM_INTERRUPT_t)0;
    if (frameCtrl->int_enable & CSL_CAM_INT_FRAME_START)
    {
        chal_cam_frame_cfg_st.interrupts |= CHAL_CAM_INT_FRAME_START;
    }
    if (frameCtrl->int_enable & CSL_CAM_INT_FRAME_END)
    {
        chal_cam_frame_cfg_st.interrupts |= CHAL_CAM_INT_FRAME_END;
    }
#if defined (_ATHENA_)
    if (frameCtrl->int_enable & CSL_CAM_INT_LINE_START)
    {
        chal_cam_frame_cfg_st.interrupts |= CHAL_CAM_INT_LINE_START;
    }
    if (frameCtrl->int_enable & CSL_CAM_INT_LINE_END)
    {
        chal_cam_frame_cfg_st.interrupts |= CHAL_CAM_INT_LINE_END;
    }
#endif    
#if ( defined (_HERA_) || defined (_RHEA) )
    if ( (frameCtrl->int_enable & CSL_CAM_INT_LINE_START) || (frameCtrl->int_enable & CSL_CAM_INT_LINE_END) )
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_set_frame_control][Error] :  LINE_START/LINE_END not allowed!\n" );
    }

#endif    
    if (frameCtrl->int_enable & CSL_CAM_INT_LINE_COUNT)
    {
        chal_cam_frame_cfg_st.interrupts |= CHAL_CAM_INT_LINE_COUNT;
    }
// Line Count
    printk(KERN_INFO  "[csl_cam_set_frame_control][Info] :  intr=0x%x line_count=%d\n",chal_cam_frame_cfg_st.interrupts,frameCtrl->int_line_count );
    chal_cam_frame_cfg_st.line_count = (cUInt32)frameCtrl->int_line_count;
// Capture mode
    camH->state.capture_mode = frameCtrl->capture_mode;
    if (frameCtrl->capture_mode == CSL_CAM_CAPTURE_MODE_TRIGGER)
    {
        chal_cam_frame_cfg_st.mode = CHAL_CAM_SHOT_MODE_SINGLE;
    }
    else
    {
        chal_cam_frame_cfg_st.mode = CHAL_CAM_SHOT_MODE_CONTINOUS;
    }
    chal_status |= chal_cam_cfg_frame(cslCamDrv.chalCamH, &chal_cam_frame_cfg_st);
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_set_frame_control][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}

/***********************************************************
 * Name: csl_cam_set_image_type_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_IMAGE_st imageCtrl - image channel id's (virtual channels)
 *
 * Description: Routine used to set Image Capture Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_image_type_control( CSL_CAM_HANDLE cslCamH, pCSL_CAM_IMAGE_ID_st imageCtrl )
{
    Int32                       success = 0;                    //pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_IMAGE_ID_st_t      chal_cam_image_id_st;

// Interface
    chal_cam_image_id_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_image_id_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Image Channel Id's
    chal_cam_image_id_st.image_data_id_0 = (cUInt8)imageCtrl->image_data_id0;
    chal_cam_image_id_st.image_data_id_1 = (cUInt8)imageCtrl->image_data_id1;
    chal_cam_image_id_st.image_data_id_2 = (cUInt8)imageCtrl->image_data_id2;
    chal_cam_image_id_st.image_data_id_3 = (cUInt8)imageCtrl->image_data_id3;
    chal_status |= chal_cam_cfg_image_id(cslCamDrv.chalCamH, &chal_cam_image_id_st);
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_set_image_type_control][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}

/***********************************************************
 * Name: csl_cam_set_data_type_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_DATA_st dataCtrl - data channel cfg
 *
 * Description: Routine used to set Data Capture Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_data_type_control( CSL_CAM_HANDLE cslCamH, pCSL_CAM_DATA_st dataCtrl )
{
    Int32                       success = 0;                    //pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_DATA_CFG_st_t      chal_cam_data_cfg_st;

// Interface
    chal_cam_data_cfg_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_data_cfg_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Line Count
    chal_cam_data_cfg_st.line_count = (cUInt32)dataCtrl->line_count;
// Data Type
    chal_cam_data_cfg_st.data_id = dataCtrl->data_id;
// Interrupt Enable    
    chal_cam_data_cfg_st.intr_enable = (CHAL_CAM_INTERRUPT_t)0;
    if ( (dataCtrl->int_enable & CSL_CAM_INT_DATA) && (dataCtrl->int_enable & CSL_CAM_INT_FRAME_END) )
    {
        chal_cam_data_cfg_st.intr_enable |= CHAL_CAM_INT_DATA_FRAME_END;
    }
    else if (dataCtrl->int_enable & CSL_CAM_INT_DATA)
    {
        chal_cam_data_cfg_st.intr_enable |= CHAL_CAM_INT_DATA_END;
    }
        
// FSP Decoding
    chal_cam_data_cfg_st.fsp_decode_enable = dataCtrl->fsp_decode_enable;
    chal_status |= chal_cam_cfg_data(cslCamDrv.chalCamH, &chal_cam_data_cfg_st);
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_set_data_type_control][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}


/***********************************************************
 * Name: csl_cam_set_packet_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_PACKET_st packetCtrl - packet input cfg
 *
 * Description: Routine used to set Data Capture Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_packet_control( CSL_CAM_HANDLE cslCamH, pCSL_CAM_PACKET_st packetCtrl )
{
    Int32                       success = 0;                    // pass by default
    UInt32                      i;
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_PKT_CFG_st_t       chal_cam_pkt_cfg_st;

// Interface
    chal_cam_pkt_cfg_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_pkt_cfg_st.chan = cslCamChalPortChan(camH->port_chan_sel);

// Config packet captures
    for (i=0; i < MAX_PACKET_CAPTURE; i++)
    {
        chal_cam_pkt_cfg_st.instance = (CHAL_CAM_PKT_CMP_SELECT_t)(1 << i);
        chal_cam_pkt_cfg_st.cmp_enable = packetCtrl->packet_capture[i].packet_compare_enable;
        chal_cam_pkt_cfg_st.capture_enable = packetCtrl->packet_capture[i].capture_header_enable;
        if ( packetCtrl->packet_capture[i].int_enable == CSL_CAM_INT_PACKET )
        {
            chal_cam_pkt_cfg_st.intr_enable = TRUE;
        }
        else
        {
            chal_cam_pkt_cfg_st.intr_enable = FALSE;
        }
        chal_cam_pkt_cfg_st.data_id = (cUInt8)packetCtrl->packet_capture[i].data_id;
        chal_status |= chal_cam_cfg_short_pkt(cslCamDrv.chalCamH, &chal_cam_pkt_cfg_st);
    }
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_set_packet_control][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}


/***********************************************************
 * Name: csl_cam_set_lane_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_LANE_CONTROL_st laneCtrl - lane control input cfg
 *
 * Description: Routine used to set Lane Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_lane_control( CSL_CAM_HANDLE cslCamH, pCSL_CAM_LANE_CONTROL_st laneCtrl )
{
    Int32                       success = 0;                    //pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_LANE_CNTRL_st_t    chal_cam_lane_cntrl_st;

// Interface
    chal_cam_lane_cntrl_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_lane_cntrl_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Lane
    chal_cam_lane_cntrl_st.lane = cslCamChalLane(laneCtrl->lane_select);
// Lane Control
    chal_cam_lane_cntrl_st.cntrl_state = cslCamChalLane_ctrl(laneCtrl->lane_control);
// Parameter
    chal_cam_lane_cntrl_st.param = (cUInt32)laneCtrl->param;

    chal_status |= chal_cam_set_lane_cntrl(cslCamDrv.chalCamH, &chal_cam_lane_cntrl_st);
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_set_lane_control][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}


/***********************************************************
 * Name: csl_cam_set_pipeline_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_PIPELINE_st pipelineCtrl - pipeling control cfg
 *
 * Description: Routine used to set Pipeline Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_pipeline_control( CSL_CAM_HANDLE cslCamH, pCSL_CAM_PIPELINE_st pipelineCtrl )
{
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_PIPELINE_CFG_st_t  chal_cam_pipeline_st;

// Update handle
    memcpy( &camH->state.pipeline_cfg, pipelineCtrl, sizeof(CSL_CAM_PIPELINE_st_t) );
// Interface
    chal_cam_pipeline_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_pipeline_st.chan = cslCamChalPortChan(camH->port_chan_sel);

// Encode DPCM
   switch (pipelineCtrl->encode)
   {
        case CSL_CAM_10_8_DPCM_ENC:
            chal_cam_pipeline_st.enc_proc = CHAL_CAM_10_8_DPCM_ENC;
            break;
        case CSL_CAM_12_8_DPCM_ENC:
            chal_cam_pipeline_st.enc_proc = CHAL_CAM_12_8_DPCM_ENC;
            break;
        case CSL_CAM_14_8_DPCM_ENC:
            chal_cam_pipeline_st.enc_proc = CHAL_CAM_14_8_DPCM_ENC;
            break;
        default:
            chal_cam_pipeline_st.enc_proc = CHAL_CAM_ENC_NONE;
            break;
    }
//  Encode enhanced predictor
    chal_cam_pipeline_st.enc_predictor = FALSE;
    if (pipelineCtrl->enc_adv_predictor == TRUE)
    {
        chal_cam_pipeline_st.enc_predictor = TRUE;
    }
// Encode Pixel packing
    chal_cam_pipeline_st.enc_pixel_pack = cslCamChalPixelSize(pipelineCtrl->pack);
// Encode Block Size
    chal_cam_pipeline_st.enc_blk_lnth = (CHAL_CAM_ENC_BLK_LTH_t)pipelineCtrl->encode_blk_size;

// Decode DPCM
   switch (pipelineCtrl->decode)
   {
        case CSL_CAM_6_10_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_6_10_DPCM_DEC;
            break;
        case CSL_CAM_6_12_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_6_12_DPCM_DEC;
            break;
        case CSL_CAM_7_10_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_7_10_DPCM_DEC;
            break;
        case CSL_CAM_7_12_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_7_12_DPCM_DEC;
            break;
        case CSL_CAM_8_10_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_8_10_DPCM_DEC;
            break;
        case CSL_CAM_8_12_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_8_12_DPCM_DEC;
            break;
        case CSL_CAM_8_14_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_8_14_DPCM_DEC;
            break;
        case CSL_CAM_8_16_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_8_16_DPCM_DEC;
            break;
        case CSL_CAM_10_14_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_10_14_DPCM_DEC;
            break;
        case CSL_CAM_10_16_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_10_16_DPCM_DEC;
            break;
        case CSL_CAM_12_16_DPCM_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_12_16_DPCM_DEC;
            break;
        case CSL_CAM_FSP_DEC:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_FSP_DEC;
            break;            
        default:
            chal_cam_pipeline_st.dec_proc = CHAL_CAM_DEC_NONE;
            break;
    }
//  Decode enhanced predictor
    chal_cam_pipeline_st.dec_predictor = FALSE;
    if (pipelineCtrl->dec_adv_predictor == TRUE)
    {
        chal_cam_pipeline_st.dec_predictor = TRUE;
    }
// Decode Pixel unpacking
    chal_cam_pipeline_st.dec_pixel_unpack = cslCamChalPixelSize(pipelineCtrl->unpack);
   
    chal_status |= chal_cam_cfg_pipeline(cslCamDrv.chalCamH, &chal_cam_pipeline_st);
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_set_pipeline_control][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}

/***********************************************************
 * Name: csl_cam_set_pipeline_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_WINDOW_st imageWindow - window control cfg
 *
 * Description: Routine used to set Window Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_image_window( CSL_CAM_HANDLE cslCamH, pCSL_CAM_WINDOW_st imageWindow )
{
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_WINDOW_CFG_st_t    chal_cam_window_st;

// Update handle
    memcpy( &camH->state.window_cfg, imageWindow, sizeof(CSL_CAM_WINDOW_st_t) );
// Interface
    chal_cam_window_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_window_st.chan = cslCamChalPortChan(camH->port_chan_sel);

    chal_cam_window_st.enable = imageWindow->enable;
    chal_cam_window_st.field_gating = FALSE;
    chal_cam_window_st.h_start_sample = imageWindow->horizontal_start_byte;
    chal_cam_window_st.h_end_sample = imageWindow->horizontal_start_byte + imageWindow->horizontal_size_bytes;
    chal_cam_window_st.v_start_sample = imageWindow->vertical_start_line;
    chal_cam_window_st.v_end_sample = imageWindow->vertical_start_line + imageWindow->vertical_size_lines;
    chal_status |= chal_cam_cfg_window(cslCamDrv.chalCamH, &chal_cam_window_st);
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_set_image_window][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}

/***********************************************************
 * Name: csl_cam_get_buffer_status
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       CSL_CAM_SELECT_t status_select - buffer select
 *       pCSL_CAM_BUFFER_STATUS_st bufStatus - buffer status struct
 *
 * Description: Routine used to set Window Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_get_buffer_status( CSL_CAM_HANDLE cslCamH, CSL_CAM_SELECT_t status_select, pCSL_CAM_BUFFER_STATUS_st bufStatus )
{
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_BUFFER_CFG_st_t    chal_cam_buffer_st;

// Interface
    chal_cam_buffer_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_buffer_st.chan = cslCamChalPortChan(camH->port_chan_sel);
  
    chal_status |= chal_cam_get_buffer_cfg(cslCamDrv.chalCamH, &chal_cam_buffer_st);
    if (chal_status == CHAL_OP_OK)
    {
        success = 0;
    }
    bufStatus->buffer_st.start_addr = chal_cam_buffer_st.buffers.image0Buff->start_addr; 
    bufStatus->buffer_st.size = chal_cam_buffer_st.buffers.image0Buff->size; 
    bufStatus->buffer_st.line_stride = chal_cam_buffer_st.buffers.image0Buff->line_stride; 
    bufStatus->buffer_st.buffer_wrap_en = chal_cam_buffer_st.buffers.image0Buff->buf_wrap_enable; 
    bufStatus->write_ptr = chal_cam_buffer_st.write_ptr; 
    bufStatus->bytes_per_line = chal_cam_buffer_st.bytes_per_line; 
    bufStatus->lines_per_frame = chal_cam_buffer_st.lines_per_frame; 
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_get_buffer_status][Info] : ERROR!\n" );
    }
    return success;
}
    

/***********************************************************
 * Name: csl_cam_get_intr_status
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       CSL_CAM_INTERRUPT_t *intStatus - OR'd interrupt status
 *
 * Description: Routine used to get Channel Interrupt Status
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
UInt32 csl_cam_get_intr_status( CSL_CAM_HANDLE cslCamH, CSL_CAM_INTERRUPT_t *intStatus )
{
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;
    UInt32              irq_status = 0;
    UInt32              raw_status = 0;

// Interface
    chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Channel
    chal_cam_param_st.param = 0;
// Channel Intr Status    
    raw_status = chal_cam_get_chan_status(cslCamDrv.chalCamH, &chal_cam_param_st);
// Check status bits    
    if (chal_cam_param_st.param & CHAL_CAM_INT_FRAME_START)
    {
        irq_status |= CSL_CAM_INT_FRAME_START;
    }
    if (chal_cam_param_st.param & CHAL_CAM_INT_FRAME_END)
    {
        irq_status |= CSL_CAM_INT_FRAME_END;
    }
    if (chal_cam_param_st.param & CHAL_CAM_INT_LINE_START)
    {
        irq_status |= CSL_CAM_INT_LINE_START;
    }
    if (chal_cam_param_st.param & CHAL_CAM_INT_LINE_END)
    {
        irq_status |= CSL_CAM_INT_LINE_END;
    }
    if (chal_cam_param_st.param & CHAL_CAM_INT_LINE_COUNT)
    {
        irq_status |= CSL_CAM_INT_LINE_COUNT;
    }
    if ( (chal_cam_param_st.param & CHAL_CAM_INT_DATA_END) || (chal_cam_param_st.param & CHAL_CAM_INT_DATA_FRAME_END) )
    {
        irq_status |= CSL_CAM_INT_DATA;
    }
    if (chal_cam_param_st.param & CHAL_CAM_INT_PKT)
    {
        irq_status |= CSL_CAM_INT_PACKET;
    }
    if ( chal_cam_param_st.param & CHAL_CAM_INT_FRAME_ERROR )
    {
        irq_status |= CSL_CAM_INT_FRAME_ERROR;
    }
    *intStatus = (CSL_CAM_INTERRUPT_t)irq_status;
    return raw_status;
}

/***********************************************************
 * Name: csl_cam_get_rx_status
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       CSL_CAM_RX_STATUS_t *rxStatus - receiver status
 *
 * Description: Routine used to get Receiver status
 *
 * Returns: cHAL raw status
 *
 ***********************************************************/
UInt32 csl_cam_get_rx_status( CSL_CAM_HANDLE cslCamH, CSL_CAM_RX_STATUS_t *rxStatus )
{
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;
    UInt32                      rx_status = 0;
    UInt32                      raw_status = 0;

// Interface
    chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Receiver Intr Status    
    chal_cam_param_st.param = 0;
    raw_status = chal_cam_get_rx_status(cslCamDrv.chalCamH, &chal_cam_param_st);

// Check status bits    
    if (chal_cam_param_st.param & CHAL_CAM_RX_INT_CHAN_0)
    {
        rx_status |= CSL_CAM_RX_INT;
    }
    if (chal_cam_param_st.param & CHAL_CAM_RX_INT_PKT_0)
    {
        rx_status |= CSL_CAM_RX_INT_PKT;
    }
    if (chal_cam_param_st.param & CHAL_CAM_RX_INT_PKT_1)
    {
        rx_status |= CSL_CAM_RX_INT_PKT;
    }
    if ( chal_cam_param_st.param & CHAL_CAM_RX_PKT_ERROR )
    {
        rx_status |= CSL_CAM_RX_PKT_ERROR;
    }
    if (chal_cam_param_st.param & CHAL_CAM_RX_PANIC)
    {
        rx_status |= CSL_CAM_RX_PANIC;
    }
    if ( chal_cam_param_st.param & CHAL_CAM_RX_BUSY )
    {
        rx_status |= CSL_CAM_RX_BUSY;
    }
    if (chal_cam_param_st.param & CHAL_CAM_RX_CRC_ERROR)
    {
        rx_status |= CSL_CAM_RX_CRC_ERROR;
    }
    if (chal_cam_param_st.param & CHAL_CAM_RX_FIFO_ERROR)
    {
        rx_status |= CSL_CAM_RX_FIFO_ERROR;
    }
    if ( chal_cam_param_st.param & CHAL_CAM_RX_PARITY_ERROR )
    {
        rx_status |= CSL_CAM_RX_PARITY_ERROR;
    }
    if ( chal_cam_param_st.param & CHAL_CAM_RX_BIT_ERROR )
    {
        rx_status |= CSL_CAM_RX_BIT_ERROR;
    }
    if ( chal_cam_param_st.param & CHAL_CAM_RX_CLK_PRESENT )
    {
        rx_status |= CSL_CAM_RX_CLK_PRESENT;
    }
    if ( chal_cam_param_st.param & CHAL_CAM_RX_SYNCD )
    {
        rx_status |= CSL_CAM_RX_SYNCD;
    }
    *rxStatus = (CSL_CAM_RX_STATUS_t)rx_status;
    return raw_status;
}
    
/***********************************************************
 * Name: csl_cam_get_lane_status
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *       CSL_CAM_SELECT_t lane - lane select
 *       CSL_CAM_LANE_STATUS_t *laneStatus - lane status
 *
 * Description: Routine used to get Lane status
 *
 * Returns: cHAL raw status
 *
 ***********************************************************/
Int32 csl_cam_get_lane_status( CSL_CAM_HANDLE cslCamH, CSL_CAM_SELECT_t lane, CSL_CAM_LANE_STATUS_t *laneStatus )
{
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;

// Interface
    chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Lane
    chal_cam_param_st.lane = cslCamChalLane(lane);
// Lane Status    
    chal_cam_param_st.param = 0;
    chal_status |= chal_cam_get_lane_status(cslCamDrv.chalCamH, &chal_cam_param_st);
    *laneStatus = cslCamChalLaneStatus( (CHAL_CAM_LANE_STATE_t)chal_cam_param_st.param );
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_get_lane_status][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}   


/***********************************************************
 * Name: csl_cam_set_clk
 * 
 * Arguments: 
 *       pCSL_CAM_CLOCK_OUT_st clockSet - clock parameter structure 
 *       CSL_CAM_SELECT_t lane - lane select
 *       CSL_CAM_LANE_STATUS_t *laneStatus - lane status
 *
 * Description: Routine used to set clock for camera device
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_clk( pCSL_CAM_CLOCK_OUT_st clockSet )
{
    Int32                       success = 0;                    // pass by default
	int speed;

#if ( defined (_ATHENA_) )
// Acquire Clock Handle if not Open
// Clock to Camera Sensor
    if (clockSet->enable)
    {
	if(camclk == NULL)
		camclk = OSDAL_CLK_Open(OSDAL_CLK_CAM);
        printk(KERN_INFO  "[csl_cam_set_clk][Info] : csl_cam_set_clk(): enable_clock: \n" );
    // Set Camera Clock Speed & Enable clock
    	OSDAL_CLK_Stop(camclk);
        switch (clockSet->speed)
        {
            default:
            case CSL_CAM_CLK_12MHz:
                speed = OSDAL_CLK_CAM_12MHZ;
                break;
            case CSL_CAM_CLK_13MHz:
                speed = OSDAL_CLK_CAM_13MHZ;
                break;
            case CSL_CAM_CLK_24MHz:
                speed = OSDAL_CLK_CAM_24MHZ;
                break;
            case CSL_CAM_CLK_26MHz:
                speed = OSDAL_CLK_CAM_26MHZ;
                break;
            case CSL_CAM_CLK_48MHz:
                speed = OSDAL_CLK_CAM_48MHZ;
                break;
        }
	OSDAL_CLK_Set_Config(camclk,speed , NULL);
	OSDAL_CLK_Start(camclk);             
    }
    else
    {
		if(camclk == NULL)
			camclk = OSDAL_CLK_Open(OSDAL_CLK_CAM);
		printk(KERN_INFO  "[csl_cam_set_clk][Info] : csl_cam_set_clk(): disable_clock: \n" );
    // Disable Camera clock
    	OSDAL_CLK_Stop(camclk);
    }
#endif    


#if ( defined (_HERA_) || defined (_RHEA) )
// Clock to Camera Sensor
      /*--- Enable Camera Clk ---*/
      /* Camera clocks dclk1 & dclk2 are generated from
       * ROOT_CLK. The clocks are 13 MHz by default
       * There is a DIG_PRE_DIV value set at 26 MHZ
       * DIG0_DIV (dclk1) is set to 1 (div 2) by default => 13 MHz
       * DIG1_DIV (dclk2) is set to 1 (dev 2) by default => 13 MHz
       * If numbers are changed then DIG_TRIG must be set again
       *
       */
       
    success |= cslCamClock(clockSet->clock_select, clockSet->speed, clockSet->enable);
#endif
    
    return success;
}   

/***********************************************************
 * Name: csl_cam_trigger_capture
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 * Description: Routine used to Trigger capture mode
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_trigger_capture( CSL_CAM_HANDLE cslCamH )
{
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;

//*************************************************
//  TEST CODE ONLY
//*************************************************
    UInt32 frame_time;

    frame_time = 0;//TIMER_GetValue();
    camH->state.FS_time = frame_time; 
    camH->state.FE_time = frame_time; 
    camH->state.LC_time = frame_time; 
//*************************************************
//*************************************************


// Interface
    chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);

    chal_cam_param_st.param = 0x00000000;
    chal_status |= chal_cam_channel_trigger(cslCamDrv.chalCamH, &chal_cam_param_st);
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_trigger_capture][Info] : ERROR!\n" );
    }
    else
    {
    // Update Capture mode
        camH->state.capture_mode = CSL_CAM_CAPTURE_MODE_TRIGGER;
    }        
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}

/***********************************************************
 * Name: csl_cam_rx_start
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 * Description: Routine used to Start Receiver for capture mode
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_rx_start( CSL_CAM_HANDLE cslCamH )
{
	printk(KERN_INFO"csl_cam_rx_start called \n");
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;
    UInt32 sem_count;

    memset(&chal_cam_param_st, 0, sizeof(CHAL_CAM_PARAM_st_t));
// Interface
    chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);

// Verify Receiver was opened
    if (camH->open)
    {
    // Set IOCR in stream state
    #if 0
        if ( cslCamIocrSet( cslCamDrv.intf_cfg.intf, cslCamDrv.intf_cfg.afe_port, CSL_CAM_STREAM_MODE ) )
        {
            printk(KERN_INFO  "[csl_cam_rx_start][Error] :  cslCamIocrSet() FAILED \n" );
            chal_status |= CHAL_OP_FAILED;
        }
	#endif
        chal_status |= chal_cam_rx_start(cslCamDrv.chalCamH, &chal_cam_param_st);
        camH->state.active = 1;

    // Release Task Semaphore to put in proper time-out state
    	//printk(KERN_INFO"we are here 1 \n");
        if ( (sSemCslCamTask != NULL) && (sCslCamTask != NULL) )
        {
        	//printk(KERN_INFO"we are here 2 \n");
        	OSSEMAPHORE_Release(sSemCslCamTask);
	/*
            sem_count = OSDAL_SEMA_COUNT(sSemCslCamTask);
            if (sem_count == 0)
            {
            //printk(KERN_INFO"we are here 3 \n");
                printk(KERN_INFO  "[csl_cam_rx_start][Info]: sSemCslCamTask release: \n") );
                OSDAL_SENDEVENT( sSemCslCamTask ); //let task handle this state.
            }
		*/
        }
    }
    if (chal_status != CHAL_OP_OK)
    {
    	printk(KERN_INFO"Error is CHAL status \n");
        success = -1;
        printk(KERN_INFO  "[csl_cam_rx_start][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));

    camH->state.FS_time = 0;
    camH->state.FE_time = 0;
    camH->state.LC_time = 0;
	printk("Mo error in CHAL status\n");
    
    return success;
}

/***********************************************************
 * Name: csl_cam_rx_stop
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 * Description: Routine used to Stop Receiver, disable capture mode
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_rx_stop( CSL_CAM_HANDLE cslCamH )
{
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;
    UInt32 sem_count;

// Interface
    chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);

// Verify Receiver is active
    if (camH->state.active)
    {
    // Set Receiver to Inactive
        camH->state.active = 0;
        chal_cam_param_st.param = 0x00000000;
        chal_status |= chal_cam_rx_stop(cslCamDrv.chalCamH, &chal_cam_param_st);

        if ( csl_cam_reset( cslCamH, CSL_CAM_RESET_SWR ) )
        {
            printk(KERN_INFO  "[csl_cam_rx_stop][Error] :  csl_cam_reset(): CSL_CAM_RESET_SWR: FAILED \n" );
            success |= -1;
        }
        if ( csl_cam_reset( cslCamH, CSL_CAM_RESET_ARST ) )
        {
            printk(KERN_INFO  "[csl_cam_rx_stop][Error] :  csl_cam_reset(): CSL_CAM_RESET_ARST: FAILED \n" );
            success |= -1;
        }
    // Release Task Semaphore to put in proper time-out state
    #if 0 // We dont CSL Task, Hisr etc
        if ( (sSemCslCamTask != NULL) && (sCslCamTask != NULL) )
        {
            sem_count = OSDAL_SEMA_COUNT(sSemCslCamTask);
            if (sem_count == 0)
            {
                printk(KERN_INFO  "[csl_cam_rx_stop][Info]: sSemCslCamTask release: \n" );
                OSDAL_SENDEVENT( sSemCslCamTask ); //let task handle this state.
            }
        }
	#endif
    }
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_rx_stop][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}

/***********************************************************
 * Name: csl_cam_reset
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *       CSL_CAM_RESET_t mode - reset mode
 *
 * Description: Routine used to Reset Receiver
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_reset( CSL_CAM_HANDLE cslCamH, CSL_CAM_RESET_t mode )
{
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;

// Interface
    chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
    chal_cam_param_st.param = 0x00000000;
// Set reset requested
    if (mode & CSL_CAM_RESET_SWR)
    {
        chal_cam_param_st.param |= CHAL_CAM_RESET_RX;
    }
    if (mode & CSL_CAM_RESET_ARST)
    {
        chal_cam_param_st.param |= CHAL_CAM_RESET_ARST;
    }
// Reset 
    chal_status |= chal_cam_reset(cslCamDrv.chalCamH, &chal_cam_param_st);
    if (chal_status != CHAL_OP_OK)
    {
        success = -1;
        printk(KERN_INFO  "[csl_cam_reset][Info] : ERROR!\n" );
    }
    DBG_OUT_REGISTER_DISPLAY( csl_cam_register_display( cslCamH ));
    return success;
}


/***********************************************************
 * Name: csl_cam_register_callback
 * 
 * Arguments: 
 *      CSL_CAM_HANDLE cslCamH - device handle 
 *      CSL_CAM_CB_EVENT_t event - which callback to register/unregister for
 *      callback - callback function to the applcation, NULL unregisters callback
 * Description: Routine used to Set Callback function
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_register_event_callback( CSL_CAM_HANDLE cslCamH, CSL_CAM_CB_EVENT_t event, cslCamCB_t callback, void *userdata)
{
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;

// CallBack Event
    switch (event)
    {
        case CSL_CAM_LISR:
            camH->state.lisr_callback = callback;
            camH->state.lisr_cb_userdata = userdata;
            break;
        case CSL_CAM_TASK:
            camH->state.task_callback = callback;
            camH->state.task_cb_userdata = userdata;
            break;
        default:
            success = -1;
            printk(KERN_INFO  "[csl_cam_register_event_callback][Info] : ERROR! No Selection\n" );
            break;
    }
    return success;
}

/***********************************************************
 * Name: csl_cam_register_display
 * 
 * Arguments: 
 *      CSL_CAM_HANDLE cslCamH - device handle 
 * Description: Debug Routine used to display camera interface registers
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_register_display( CSL_CAM_HANDLE cslCamH )
{
    Int32                       success = 0;                    // pass by default
    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
    CHAL_CAM_STATUS_CODES       chal_status = CHAL_OP_OK;
    CHAL_CAM_PARAM_st_t         chal_cam_param_st;

// Interface
    chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
    chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
    chal_cam_param_st.param = 0x00000000;
// Display Registers for Interface/Channel chosen
    chal_status |= chal_cam_register_display(cslCamDrv.chalCamH, &chal_cam_param_st );
    if (chal_status == CHAL_OP_OK)
    {
        success = 0;
    }

    return success;
}

/********************************** End of file ******************************************/
