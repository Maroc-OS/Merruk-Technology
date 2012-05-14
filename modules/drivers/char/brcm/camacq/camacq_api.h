#if !defined(_CAMACQ_API_H_)
#define _CAMACQ_API_H_

/* Include */
#include "camacq_type.h"
#include "camacq_ext.h"

/* Global */
#undef GLOBAL

#if !defined(_CAMACQ_API_C_)
#define GLOBAL extern
#else
#define GLOBAL
#endif

/* depend on baseband H/W */
typedef struct stCamBlock_t
{
    S32 (*CamPowerSet)( S32 iflag );        
}_stCamBlock;

/* Enumeration */
typedef enum eCameraMode_e
{
    CAMACQ_MODE_VIEWFINDER = 0,
    CAMACQ_MODE_STILL
} _eCameraMode;

typedef enum eCamacqState_e
{
    CAMACQ_STATE_UNINITIALISED = 0,
    CAMACQ_STATE_BOOT,
    CAMACQ_STATE_RUN,
    CAMACQ_STATE_PAUSE,
    CAMACQ_STATE_STOP,
    CAMACQ_STATE_FLASHGUN
} _eCamacqState;

typedef enum eCamacqSensorData_e
{
    CAMACQ_SENSORDATA_INIT = 0,
    CAMACQ_SENSORDATA_PREVIEW,
    CAMACQ_SENSORDATA_CAPTURE,
    CAMACQ_SENSORDATA_CAMCORDER,

    CAMACQ_SENSORDATA_AF_CANCEL,	
    CAMACQ_SENSORDATA_AF_ON,	
    CAMACQ_SENSORDATA_AF_OFF,	
    CAMACQ_SENSORDATA_AF_MACRO_ON,	
    CAMACQ_SENSORDATA_AF_MACRO_OFF,

    CAMACQ_SENSORDATA_AE_CENTERWEIGHTED,
    CAMACQ_SENSORDATA_AE_MATRIX,//10	
    CAMACQ_SENSORDATA_AE_SPOT,	

    CAMACQ_SENSORDATA_WB_AUTO,	
    CAMACQ_SENSORDATA_WB_CLOUDY,	
    CAMACQ_SENSORDATA_WB_DAYLIGHT,	
    CAMACQ_SENSORDATA_WB_FLUORESCENT,	
    CAMACQ_SENSORDATA_WB_INCANDESCENT,	

    CAMACQ_SENSORDATA_SATURATION_0,	
    CAMACQ_SENSORDATA_SATURATION_1,	
    CAMACQ_SENSORDATA_SATURATION_2,	
    CAMACQ_SENSORDATA_SATURATION_3,//20	
    CAMACQ_SENSORDATA_SATURATION_4,	

    CAMACQ_SENSORDATA_SHARPNESS_0,	
    CAMACQ_SENSORDATA_SHARPNESS_1,	
    CAMACQ_SENSORDATA_SHARPNESS_2,	
    CAMACQ_SENSORDATA_SHARPNESS_3,	
    CAMACQ_SENSORDATA_SHARPNESS_4,	


    CAMACQ_SENSORDATA_CONTRAST_0,	
    CAMACQ_SENSORDATA_CONTRAST_1,	
    CAMACQ_SENSORDATA_CONTRAST_2,	
    CAMACQ_SENSORDATA_CONTRAST_3,	//30
    CAMACQ_SENSORDATA_CONTRAST_4,	

    CAMACQ_SENSORDATA_EFFECT_NONE,	
    CAMACQ_SENSORDATA_EFFECT_GRAY,	
    CAMACQ_SENSORDATA_EFFECT_NEGATIVE,	
    CAMACQ_SENSORDATA_EFFECT_SEPIA,	

    CAMACQ_SENSORDATA_SCENE_NONE,
    CAMACQ_SENSORDATA_SCENE_PORTRAIT,	
    CAMACQ_SENSORDATA_SCENE_LANDSCAPE,	
    CAMACQ_SENSORDATA_SCENE_NIGHT,	
    CAMACQ_SENSORDATA_SCENE_NIGHT_DARK,//40	
    CAMACQ_SENSORDATA_SCENE_SPORTS,	
    CAMACQ_SENSORDATA_SCENE_SUNSET,	
    CAMACQ_SENSORDATA_SCENE_TEXT,	
    CAMACQ_SENSORDATA_SCENE_AGAINST_LIGHT,	
    CAMACQ_SENSORDATA_SCENE_BEACH_SNOW,	
    CAMACQ_SENSORDATA_SCENE_CANDLE,	
    CAMACQ_SENSORDATA_SCENE_DAWN,	
    CAMACQ_SENSORDATA_SCENE_FALL,	
    CAMACQ_SENSORDATA_SCENE_FIRE,	
    CAMACQ_SENSORDATA_SCENE_PARTY_INDOOR,//50	
    CAMACQ_SENSORDATA_SCENE_INDOOR,	


    CAMACQ_SENSORDATA_EXPOSURE_0,  
    CAMACQ_SENSORDATA_EXPOSURE_1,    
    CAMACQ_SENSORDATA_EXPOSURE_2,    
    CAMACQ_SENSORDATA_EXPOSURE_3,        
    CAMACQ_SENSORDATA_EXPOSURE_4,    
    CAMACQ_SENSORDATA_EXPOSURE_5,    
    CAMACQ_SENSORDATA_EXPOSURE_6,    
    CAMACQ_SENSORDATA_EXPOSURE_7, 
    CAMACQ_SENSORDATA_EXPOSURE_8,//60


    CAMACQ_SENSORDATA_BRIGHTNESS_0,  
    CAMACQ_SENSORDATA_BRIGHTNESS_1,    
    CAMACQ_SENSORDATA_BRIGHTNESS_2,        
    CAMACQ_SENSORDATA_BRIGHTNESS_3,    
    CAMACQ_SENSORDATA_BRIGHTNESS_4,    
    CAMACQ_SENSORDATA_BRIGHTNESS_5,    
    CAMACQ_SENSORDATA_BRIGHTNESS_6, 
    CAMACQ_SENSORDATA_BRIGHTNESS_7,
    CAMACQ_SENSORDATA_BRIGHTNESS_8,

    CAMACQ_SENSORDATA_SETPLL,//70
	
    CAMACQ_SENSORDATA_SIZE_5M,
    CAMACQ_SENSORDATA_SIZE_3M,
    CAMACQ_SENSORDATA_SIZE_2M,
    CAMACQ_SENSORDATA_SIZE_1M,
    CAMACQ_SENSORDATA_SIZE_VGA,
    CAMACQ_SENSORDATA_SIZE_QVGA,

    CAMACQ_SENSORDATA_QUALITY_SF,
    CAMACQ_SENSORDATA_QUALITY_F,
    CAMACQ_SENSORDATA_QUALITY_N,

    CAMACQ_SENSORDATA_CAPTURE_OUTDOOR,//80	
    CAMACQ_SENSORDATA_CAPTURE_LOWLUX,
    CAMACQ_SENSORDATA_CAPTURE_NORMAL,	
    CAMACQ_SENSORDATA_CAPTURE_NIGHTSCENE,	
    CAMACQ_SENSORDATA_CAPTURE_NIGHTSCENE_OFF,

    CAMACQ_SENSORDATA_SHADING_00,
    CAMACQ_SENSORDATA_SHADING_01,	
    CAMACQ_SENSORDATA_SHADING_10,	
    CAMACQ_SENSORDATA_SHADING_11,		

    CAMACQ_SENSORDATA_ZOOM_0,  
    CAMACQ_SENSORDATA_ZOOM_1,   //90
    CAMACQ_SENSORDATA_ZOOM_2,  
    CAMACQ_SENSORDATA_ZOOM_3,  
    CAMACQ_SENSORDATA_ZOOM_4,  
    CAMACQ_SENSORDATA_ZOOM_5,  
    CAMACQ_SENSORDATA_ZOOM_6,  
    CAMACQ_SENSORDATA_ZOOM_7,  
    CAMACQ_SENSORDATA_ZOOM_8, 

    CAMACQ_SENSORDATA_ISO_AUTO,	
    CAMACQ_SENSORDATA_ISO_50,	
    CAMACQ_SENSORDATA_ISO_100,	//100
    CAMACQ_SENSORDATA_ISO_200,	
    CAMACQ_SENSORDATA_ISO_400,		

    CAMACQ_SENSORDATA_DEFAULT_CAL
	
} _eCamacqSensorData;

/* Type Definition */
typedef struct stCamacqFmt_t 
{
    U8*         pucDesc;
    U32         uiPixelFormat;
    S32	        iBPP;           /* bits per pixel */
    
    const void*  pvRegs[CAMACQ_SENSOR_MAX];    
} _stCamacqFmt;

typedef struct stCamacqSize_t 
{
    U32         uiWidth;
    U32         uiHeight;

    const void*  pvRegs[CAMACQ_SENSOR_MAX]; 
} _stCamacqSize;

typedef struct stCamacqControl_t
{
    struct v4l2_queryctrl stV4l2QueryCtrl;
    S32 iCurVal;
    U8 ucReg;
    U8 ucMask;
    U8 ucStartBit;
} _stCamacqControl;

typedef struct stCamacqSettings_t
{
    struct stCamacqSize_t   stCamacqCurSize;
    struct stCamacqFmt_t    stCamacqCurFmt;
    enum v4l2_flash_mode    stV4l2CurFlashMode;
    struct v4l2_wb_mode     stV4l2CurWbMode;
    struct v4l2_efx         stV4l2CurEffect;
    struct v4l2_jpegcompression stV4l2CurJpegcomp;
    enum v4l2_autofocus_mode stV4l2CurAutoFocusMode;
    enum v4l2_ctrl_scene     stV4l2CurScene;
    
} _stCamacqSettings;

typedef struct stCamacqInfo_t
{
    _eCamacqState   eState;                    
} _stCamacqInfo;

struct stCamacqSensor_t;
typedef struct stCamacqAPIs_t
{
    /* APIs */
    S32 (*Detect)( struct stCamacqSensor_t *this ); /* detect this camera */
    void *(*Init)( struct stCamacqSensor_t *this ); /* one-time setup, init private fields */
    void (*CleanUp)( struct stCamacqSensor_t *this );
    S32 (*Open)( struct stCamacqSensor_t *this );
    S32 (*Close)( struct stCamacqSensor_t *this );

    S32 (*IntInit)( struct stCamacqSensor_t* this );
    S32 (*EnumPixFmt)( struct stCamacqSensor_t *this, struct v4l2_fmtdesc* pstV4l2FmtDesc );
    S32 (*EnumFrameSizes)( struct stCamacqSensor_t *this, struct v4l2_frmsizeenum* pstV4l2FrameSizeEnum );
    S32 (*TryFmt)( struct stCamacqSensor_t *this, struct v4l2_format *pstV4l2Format, _stCamacqFmt **pstRetFormat, _stCamacqSize **pstRetSize );  
    S32 (*SetFmt)( struct stCamacqSensor_t *this, struct v4l2_format* pstV4l2Format );

    S32 (*StopSensor)( struct stCamacqSensor_t *this );
    S32 (*StartSensor)( struct stCamacqSensor_t *this, struct v4l2_pix_format *pstV4l2Format, _eCameraMode eNewCameraMode, void *pvpriv);

    S32 (*QueryControl)( struct stCamacqSensor_t *this, struct v4l2_queryctrl *pstV4l2QueryCtrl );
    S32 (*SetControl)( struct stCamacqSensor_t *this, struct v4l2_control *pstV4l2Control );
    S32 (*GetControl)( struct stCamacqSensor_t *this, struct v4l2_control *pstV4l2Control );

    S32 (*CropCap)( struct stCamacqSensor_t *this, struct v4l2_cropcap *pstV4l2CropCap, void *pvpriv );
    S32 (*SetCrop)( struct stCamacqSensor_t *this, struct v4l2_crop *pstV4l2Crop, void *pvpriv );
    S32 (*GetCrop)( struct stCamacqSensor_t *this, struct v4l2_crop *pstV4l2Crop, void *pvpriv );

    S32 (*SetStrobe)( struct stCamacqSensor_t *this, struct v4l2_flash *pstV4l2Flash, void *pvpriv );
    S32 (*GetStrobe)( struct stCamacqSensor_t *this, struct v4l2_flash *pstV4l2Flash, void *pvpriv );

    S32 (*QueryMenu) ( struct stCamacqSensor_t *this, struct v4l2_querymenu *pstV4l2QueryMenu );

    S32 (*SetJpegQuality)( struct stCamacqSensor_t *this, struct v4l2_jpegcompression *pstV4l2Jpegcomp);
    S32 (*GetJpegQuality)( struct stCamacqSensor_t *this, struct v4l2_jpegcompression *pstV4l2Jpegcomp);
    S32 (*SetJpegHeader)( struct stCamacqSensor_t *this, U8 **ppcBuf );
    S32 (*SetJpegFooter)( struct stCamacqSensor_t *this, U8 *pcBuf );

    S32 (*GetCamCap)( struct stCamacqSensor_t *this, struct v4l2_camcap *pstV4l2CamCap, void *pvpriv );
    S32 (*GetActiveCam)( struct stCamacqSensor_t *this, struct v4l2_camera_status *pstV4l2CamStatus, void *pvpriv );
    S32 (*SetActiveCam)( struct stCamacqSensor_t *this, struct v4l2_camera_status *pstV4l2CamStatus, void *pvpriv );

    S32 (*SetEfx)( struct stCamacqSensor_t* this, struct v4l2_efx *pstV4l2Efx );
    S32 (*GetEfx)( struct stCamacqSensor_t* this, struct v4l2_efx *pstV4l2Efx );
    S32 (*QueryEfx)( struct stCamacqSensor_t* this, struct v4l2_query_efx *pstV4l2QueryEfx );
    
    S32 (*SetWB)( struct stCamacqSensor_t* this, struct v4l2_wb_mode *pstV4l2WB );
    S32 (*GetWB)( struct stCamacqSensor_t* this, struct v4l2_wb_mode *pstV4l2WB );
    S32 (*QueryWB)( struct stCamacqSensor_t* this, struct v4l2_query_wb_mode *pstV4l2QueryWB );
    
    S32 (*GetParm)( struct stCamacqSensor_t* this, struct v4l2_streamparm *pstV4l2StreamParm );
    S32 (*SetParm)( struct stCamacqSensor_t* this, struct v4l2_streamparm *pstV4l2StreamParm );
    
    S32 (*SetInput)( struct stCamacqSensor_t* this, int* pID );
    
    S32 (*StreamOn)( struct stCamacqSensor_t* this );
    S32 (*StreamOff)( struct stCamacqSensor_t* this );

    S32 (*WriteDirectSensorData)( struct stCamacqSensor_t* this, enum eCamacqSensorData_e eSensorData );
} _stCamacqAPIs;


//////////////////////////////////////////////////////////////////////////////////
// 
// _stCamacqSensor       * Control class
//
typedef struct stCamacqSensor_t 
{
    U8      m_szName[CAMACQ_NAME_MAX];
    U32	    m_uiResType;                  /* CAMACQ_SENSOR_MAIN, CAMACQ_SENSOR_SUB */
    U32     m_uiClock;                    /* sensor clock frequency in Mhz */
    S32     m_iYuvOrder;                  /* yuv ordering */
    S32     m_iVsyncPol;                  /* Vertical synchronization signal polarity */
    S32     m_iHsyncPol;                  /* horizontal synchronization signal polarity */
    S32     m_iSampleEdge;                /* sensor clock sample edge */
    bool    m_bFullrange;                 /* sensor yuv data flow restricted or full range? */
    bool    m_bCanAF;         
    S32     m_uiFirmwareVer;              /* sensor firmware version */

    struct i2c_client           *m_pI2cClient;          /* i2c client */

    struct stCamacqInfo_t       *m_pstCamacqInfo;       /* sensor current status or etc information */
    struct stCamacqSettings_t   *m_pstCamacqSettings;   /* sensor current settings */
    struct stCamacqControl_t    *m_pstCamacqCtrls;      /* sensor current controls */  
    
    /* APIs */
    struct stCamacqAPIs_t       *m_pstAPIs;             /* APIs of sensor common (V4L2) */
    struct stCamacqExtAPIs_t    *m_pstExtAPIs;    	    /* APIs of sensor dependency */                  
}_stCamacqSensor, *_pstCamacqSensor;

/* Member Functions (APIs) */
GLOBAL S32 CamacqAPIDetect( struct stCamacqSensor_t* this );
GLOBAL void* CamacqAPIInit( struct stCamacqSensor_t* this );
GLOBAL void CamacqAPICleanUp( struct stCamacqSensor_t *this );
GLOBAL S32 CamacqAPIOpen( struct stCamacqSensor_t *this );
GLOBAL S32 CamacqAPIClose( struct stCamacqSensor_t *this );

GLOBAL S32 CamacqAPIIntInit( struct stCamacqSensor_t* this );
GLOBAL S32 CamacqAPIEnumPixFmt( struct stCamacqSensor_t* this, struct v4l2_fmtdesc* pstV4l2FmtDesc );
GLOBAL S32 CamacqAPIEnumFrameSizes( struct stCamacqSensor_t *this, struct v4l2_frmsizeenum* pstV4l2FrameSizeEnum );

GLOBAL S32 CamacqAPITryFmt( struct stCamacqSensor_t* this, struct v4l2_format *pstV4l2Format, 
               _stCamacqFmt **pstRetFormat,  _stCamacqSize **pstRetWinSize );  
GLOBAL S32 CamacqAPISetFmt( struct stCamacqSensor_t* this, struct v4l2_format* pstV4l2Format );

GLOBAL S32 CamacqAPIStopSensor( struct stCamacqSensor_t *this );
GLOBAL S32 CamacqAPIStartSensor( struct stCamacqSensor_t *this, struct v4l2_pix_format *pstV4l2Format, _eCameraMode eNewCameraMode, void *pvpriv);

GLOBAL S32 CamacqAPIQueryControl( struct stCamacqSensor_t* this, struct v4l2_queryctrl *pstV4l2QueryCtrl );
GLOBAL S32 CamacqAPISetControl( struct stCamacqSensor_t* this, struct v4l2_control *pstV4l2Control );
GLOBAL S32 CamacqAPIGetControl( struct stCamacqSensor_t* this, struct v4l2_control *pstV4l2Control );

GLOBAL S32 CamacqAPICropCap( struct stCamacqSensor_t *this, struct v4l2_cropcap *pstV4l2CropCap, void *pvpriv );
GLOBAL S32 CamacqAPISetCrop( struct stCamacqSensor_t *this, struct v4l2_crop *pstV4l2Crop, void *pvpriv );
GLOBAL S32 CamacqAPIGetCrop( struct stCamacqSensor_t *this, struct v4l2_crop *pstV4l2Crop, void *pvpriv );

GLOBAL S32 CamacqAPISetStrobe( struct stCamacqSensor_t *this, struct v4l2_flash *pstV4l2Flash, void *pvpriv );
GLOBAL S32 CamacqAPIGetStrobe( struct stCamacqSensor_t *this, struct v4l2_flash *pstV4l2Flash, void *pvpriv );

GLOBAL S32 CamacqAPIQueryMenu( struct stCamacqSensor_t *this, struct v4l2_querymenu *pstV4l2QueryMenu );

GLOBAL S32 CamacqAPISetJpegQuality( struct stCamacqSensor_t *this, struct v4l2_jpegcompression *pstV4l2Jpegcomp );
GLOBAL S32 CamacqAPIGetJpegQuality( struct stCamacqSensor_t *this, struct v4l2_jpegcompression *pstV4l2Jpegcomp );


GLOBAL S32 CamacqAPISetJpegHeader( struct stCamacqSensor_t *this, U8 **ppcBuf );
GLOBAL S32 CamacqAPISetJpegFooter( struct stCamacqSensor_t *this, U8 *pcBuf );

GLOBAL S32 CamacqAPIGetCamCap( struct stCamacqSensor_t *this, struct v4l2_camcap *pstV4l2CamCap, void *pvpriv );
GLOBAL S32 CamacqAPIGetActiveCam( struct stCamacqSensor_t *this, struct v4l2_camera_status *pstV4l2CamStatus, void *pvpriv );
GLOBAL S32 CamacqAPISetActiveCam( struct stCamacqSensor_t *this, struct v4l2_camera_status *pstV4l2CamStatus, void *pvpriv );

GLOBAL S32 CamacqAPISetEfx( struct stCamacqSensor_t* this, struct v4l2_efx *pstV4l2Efx );
GLOBAL S32 CamacqAPIGetEfx( struct stCamacqSensor_t* this, struct v4l2_efx *pstV4l2Efx );
GLOBAL S32 CamacqAPIQueryEfx( struct stCamacqSensor_t* this, struct v4l2_query_efx *pstV4l2QueryEfx );

GLOBAL S32 CamacqAPISetWbMode( struct stCamacqSensor_t* this, struct v4l2_wb_mode *pstV4l2WbMode );
GLOBAL S32 CamacqAPIGetWbMode( struct stCamacqSensor_t* this, struct v4l2_wb_mode *pstV4l2WbMode );
GLOBAL S32 CamacqAPIQueryWbMode( struct stCamacqSensor_t* this, struct v4l2_query_wb_mode *pstV4l2QueryWbMode );

GLOBAL S32 CamacqAPIGetParm( struct stCamacqSensor_t* this, struct v4l2_streamparm *pstV4l2StreamParm );
GLOBAL S32 CamacqAPISetParm( struct stCamacqSensor_t* this, struct v4l2_streamparm *pstV4l2StreamParm );

GLOBAL S32 CamacqAPISetInput( struct stCamacqSensor_t* this, int* pID );

GLOBAL S32 CamacqAPIStreamOn( struct stCamacqSensor_t* this );
GLOBAL S32 CamacqAPIStreamOff( struct stCamacqSensor_t* this );

GLOBAL S32 CamacqAPIWriteDirectSensorData( struct stCamacqSensor_t* this, enum eCamacqSensorData_e eSensorData );


//////////////////////////////////////////////////////////////////////////////////
// 
// _stCamacqSensorManager       * Control class
// 
typedef struct stCamacqSensorManager_t
{
    _stCamacqSensor      *m_pstSensors;       
    U32                  m_uiSelectedSensor;  /* current selected sensor */
    
    /* H/W Control */
    _stCamBlock          m_stCamBlock;        /* temp, TODO : i dont know */

    S32 (*SensorSwitch)( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );
    S32 (*SensorPowerOn)( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );
    S32 (*SensorPowerOff)( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );
    S32 (*SensorReset)( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );

    /* GET SET */
    _pstCamacqSensor (*GetSensor)( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );   
    _pstCamacqSensor (*SetSensor)( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );
} _stCamacqSensorManager;


/* Member Functions */
GLOBAL S32 SensorSwitch( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );
GLOBAL S32 SensorPowerOn( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );
GLOBAL S32 SensorPowerOff( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );
GLOBAL S32 SensorReset( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );

GLOBAL struct stCamacqSensor_t* SetSensor( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );
GLOBAL struct stCamacqSensor_t* GetSensor( struct stCamacqSensorManager_t* this, U32 uiSelectedSensor );

/* Functions */
GLOBAL void CreateCamacqSensorManager( struct stCamacqSensorManager_t **ppstSensorManager );
GLOBAL void DeleteCamacqSensorManager( struct stCamacqSensorManager_t *pstSensorManager );
GLOBAL void CreateCamacqSensor( struct stCamacqSensor_t *pstSensor, int uiResType );
GLOBAL void DeleteCamacqSensor( struct stCamacqSensor_t *pstSensor );
GLOBAL void InitCamacqSensorInfo( struct stCamacqInfo_t **ppstCamacqInfo );
GLOBAL void InitCamacqSensorSettings( struct stCamacqSettings_t **ppstCamacqSettings );
GLOBAL void InitCamacqSensorControls( struct stCamacqControl_t **ppstCamacqCtrls, struct stCamacqControl_t *pstDefaultCtrls );
GLOBAL void InitCamacqAPIs( struct stCamacqAPIs_t **ppstAPIs );
GLOBAL void InitCamacqExtAPIs( struct stCamacqExtAPIs_t **ppstExtAPIs, struct stCamacqSensor_t *pstSensor );
GLOBAL struct stCamacqControl_t* CamacqFindControl( struct stCamacqSensor_t *pstSensor, S32 iId );

GLOBAL S32 CamacqRestoreCurCtrls( struct stCamacqSensor_t *pstSensor );
GLOBAL S32 CamacqSaveCurCtrls( struct stCamacqSensor_t *pstSensor );

GLOBAL S32 CamPowerSet( S32 iFlag );

/* Global Value */
GLOBAL struct stCamacqSensorManager_t   *g_pstCamacqSensorManager;
GLOBAL struct stCamacqFmt_t     g_stCamacqFmts[];
GLOBAL struct stCamacqSize_t    g_stCamacqWinSizes[];
GLOBAL struct stCamacqSize_t    g_stCamacqJpegSizes[];
GLOBAL struct stCamacqControl_t g_stCamacqControls[];

/*backup sensor settings*/
GLOBAL U32 Drv_Brightness;
GLOBAL U32 Drv_WB;
GLOBAL U32 Drv_ME;
GLOBAL U32 Drv_Effect;
GLOBAL U32 Drv_Scene;
GLOBAL U32 Drv_Iso;
GLOBAL U32 Drv_Quality;
GLOBAL U32 Drv_Size;
GLOBAL U32 Drv_Zoom;
GLOBAL U32 Drv_DTPmode;
GLOBAL S32 Drv_Mode;


/* For totoro */
typedef enum 
{
    CAM_PREVIEW = 0,
    CAM_NIGHT,
    CAM_SHUTTER_SPEED
} _eCamera_CheckExp_Mode;

typedef enum 
{
    Normal_lux = 0,
    Mid_lux,
    Low_Lux
#if defined (CONFIG_BCM_CAM_S5K5CCGX)
   ,High_Lux
#endif
} _eCamera_LUX_Mode;

GLOBAL U32 gv_checkEXPtime;
GLOBAL U32 gv_checkEXPtime2;
GLOBAL U16 gv_isoSpeedRatings; 

GLOBAL U8 gv_Nightshot_mode;

GLOBAL U8 gv_ForceSetSensor;  //0 : normal case, 1 : forcely set, 2 : forcely skip.
 enum 
{
    NORMAL_SET = 0,
    FORCELY_SET,
    FORCELY_SKIP
};
 
#undef GLOBAL
#endif /* _CAMACQ_API_H_ */

