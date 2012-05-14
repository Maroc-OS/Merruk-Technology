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

#if !defined(_CAMACQ_EXT_C_)
#define _CAMACQ_EXT_C_

/* Include */
#include "camacq_ext.h"
#include "camacq_api.h"


/* Init global variable */
_stCamacqSensorRegs g_stCamacqMainSensorRegs =
{
    pvInitRegs              : CAMACQ_MAIN_REG_INIT,
    pvSleepRegs             : CAMACQ_MAIN_REG_SLEEP,
    pvWakeupRegs            : CAMACQ_MAIN_REG_WAKEUP,
    pvPreviewRegs           : CAMACQ_MAIN_REG_PREVIEW,
    pvSnapshotRegs          : CAMACQ_MAIN_REG_SNAPSHOT,

    /* ME */
    pvMeterMatrixRegs       : CAMACQ_MAIN_REG_METER_MATRIX,
    pvMeterCWRegs           : CAMACQ_MAIN_REG_METER_CW,
    pvMeterSpotRegs         : CAMACQ_MAIN_REG_METER_SPOT,
   
    /* flip */
    pvFlipNoneRegs          : CAMACQ_MAIN_REG_FLIP_NONE,
    pvFlipWaterRegs         : CAMACQ_MAIN_REG_FLIP_WATER,
    pvFlipMirrorRegs        : CAMACQ_MAIN_REG_FLIP_MIRROR,
    pvFlipWaterMirrorRegs   : CAMACQ_MAIN_REG_FLIP_WATER_MIRROR,

    /* ISO */
    pvISOAutoRegs           : CAMACQ_MAIN_REG_ISO_AUTO,
    pvISO50Regs             : CAMACQ_MAIN_REG_ISO_50,
    pvISO100Regs            : CAMACQ_MAIN_REG_ISO_100,
    pvISO200Regs            : CAMACQ_MAIN_REG_ISO_200,
    pvISO400Regs            : CAMACQ_MAIN_REG_ISO_400,
    pvISO800Regs            : CAMACQ_MAIN_REG_ISO_800,
    pvISO1600Regs           : CAMACQ_MAIN_REG_ISO_1600,
    pvISO3200Regs           : CAMACQ_MAIN_REG_ISO_3200,

    /* Scene */
    pvSceneAutoRegs         : CAMACQ_MAIN_REG_SCENE_AUTO, 
    pvSceneNightRegs        : CAMACQ_MAIN_REG_SCENE_NIGHT, 
    pvSceneNightDarkRegs        : CAMACQ_MAIN_REG_SCENE_NIGHT_DARK, 
    pvSceneLandScapeRegs    : CAMACQ_MAIN_REG_SCENE_LANDSCAPE,
    pvSceneSunSetRegs       : CAMACQ_MAIN_REG_SCENE_SUNSET,
    pvScenePortraitRegs     : CAMACQ_MAIN_REG_SCENE_PORTRAIT,
    pvSceneSunRiseRegs      : CAMACQ_MAIN_REG_SCENE_SUNRISE,
    pvSceneIndoorRegs       : CAMACQ_MAIN_REG_SCENE_INDOOR,
    pvScenePartyRegs        : CAMACQ_MAIN_REG_SCENE_PARTY,
    pvSceneSportsRegs       : CAMACQ_MAIN_REG_SCENE_SPORTS,
    pvSceneBeachRegs        : CAMACQ_MAIN_REG_SCENE_BEACH,
    pvSceneSnowRegs         : CAMACQ_MAIN_REG_SCENE_SNOW,
    pvSceneFallColorRegs    : CAMACQ_MAIN_REG_SCENE_FALLCOLOR,
    pvSceneFireWorksRegs    : CAMACQ_MAIN_REG_SCENE_FIREWORKS,
    pvSceneCandleLightRegs  : CAMACQ_MAIN_REG_SCENE_CANDLELIGHT,
    pvSceneAgainstLightRegs : CAMACQ_MAIN_REG_SCENE_AGAINSTLIGHT,
    pvSceneTextRegs         : CAMACQ_MAIN_REG_SCENE_TEXT,

    /* Brightness */
    pvBrightness_0_Regs     : CAMACQ_MAIN_REG_BRIGHTNESS_LEVEL_0,
    pvBrightness_1_Regs     : CAMACQ_MAIN_REG_BRIGHTNESS_LEVEL_1,
    pvBrightness_2_Regs     : CAMACQ_MAIN_REG_BRIGHTNESS_LEVEL_2,
    pvBrightness_3_Regs     : CAMACQ_MAIN_REG_BRIGHTNESS_LEVEL_3,
    pvBrightness_4_Regs     : CAMACQ_MAIN_REG_BRIGHTNESS_LEVEL_4,
    pvBrightness_5_Regs     : CAMACQ_MAIN_REG_BRIGHTNESS_LEVEL_5,
    pvBrightness_6_Regs     : CAMACQ_MAIN_REG_BRIGHTNESS_LEVEL_6,
    pvBrightness_7_Regs     : CAMACQ_MAIN_REG_BRIGHTNESS_LEVEL_7,
    pvBrightness_8_Regs     : CAMACQ_MAIN_REG_BRIGHTNESS_LEVEL_8,

    /* Exposure Compensation */
    pvExpCompensation_0_Regs     : CAMACQ_MAIN_REG_EXPCOMPENSATION_LEVEL_0,
    pvExpCompensation_1_Regs     : CAMACQ_MAIN_REG_EXPCOMPENSATION_LEVEL_1,
    pvExpCompensation_2_Regs     : CAMACQ_MAIN_REG_EXPCOMPENSATION_LEVEL_2,
    pvExpCompensation_3_Regs     : CAMACQ_MAIN_REG_EXPCOMPENSATION_LEVEL_3,
    pvExpCompensation_4_Regs     : CAMACQ_MAIN_REG_EXPCOMPENSATION_LEVEL_4,
    pvExpCompensation_5_Regs     : CAMACQ_MAIN_REG_EXPCOMPENSATION_LEVEL_5,
    pvExpCompensation_6_Regs     : CAMACQ_MAIN_REG_EXPCOMPENSATION_LEVEL_6,
    pvExpCompensation_7_Regs     : CAMACQ_MAIN_REG_EXPCOMPENSATION_LEVEL_7,
    pvExpCompensation_8_Regs     : CAMACQ_MAIN_REG_EXPCOMPENSATION_LEVEL_8,

    /* AF */
    pvSetAFRegs             : CAMACQ_MAIN_REG_SET_AF,   // start af
    pvOffAFRegs             : CAMACQ_MAIN_REG_OFF_AF,
    pvCheckAFRegs           : CAMACQ_MAIN_REG_CHECK_AF,
    pvResetAFRegs           : CAMACQ_MAIN_REG_RESET_AF,
    pvManualAFReg           : CAMACQ_MAIN_REG_MANUAL_AF,
    pvMacroAFReg            : CAMACQ_MAIN_REG_MACRO_AF,
    pvReturnManualAFReg     : CAMACQ_MAIN_REG_RETURN_MANUAL_AF,
    pvReturnMacroAFReg      : CAMACQ_MAIN_REG_RETURN_MACRO_AF,
    pvSetAF_NLUXRegs        : CAMACQ_MAIN_REG_SET_AF_NLUX,
    pvSetAF_LLUXRegs        : CAMACQ_MAIN_REG_SET_AF_LLUX,

    pvSetAFNormalMode1      : CAMACQ_MAIN_REG_SET_AF_NORMAL_MODE_1,
    pvSetAFNormalMode2      : CAMACQ_MAIN_REG_SET_AF_NORMAL_MODE_2,
    pvSetAFNormalMode3      : CAMACQ_MAIN_REG_SET_AF_NORMAL_MODE_3,
    pvSetAFMacroMode1       : CAMACQ_MAIN_REG_SET_AF_MACRO_MODE_1,
    pvSetAFMacroMode2       : CAMACQ_MAIN_REG_SET_AF_MACRO_MODE_2,
    pvSetAFMacroMode3       : CAMACQ_MAIN_REG_SET_AF_MACRO_MODE_3,

    /* WB */
    pvWbAutoRegs            : CAMACQ_MAIN_REG_WB_AUTO,
    pvWbDaylightRegs        : CAMACQ_MAIN_REG_WB_DAYLIGHT,
    pvWbCloudyRegs          : CAMACQ_MAIN_REG_WB_CLOUDY,
    pvWbIncandescentRegs    : CAMACQ_MAIN_REG_WB_INCANDESCENT,
    pvWbFluorescentRegs     : CAMACQ_MAIN_REG_WB_FLUORESCENT,
    pvWbTwilightRegs        : CAMACQ_MAIN_REG_WB_TWILIGHT,
    pvWbTungstenRegs        : CAMACQ_MAIN_REG_WB_TUNGSTEN,
    pvWbOffRegs             : CAMACQ_MAIN_REG_WB_OFF,
    pvWbHorizonRegs         : CAMACQ_MAIN_REG_WB_HORIZON,
    pvWbShadeRegs           : CAMACQ_MAIN_REG_WB_SHADE,
    
    /* FMT */
    pvFmtJpegRegs           : CAMACQ_MAIN_REG_FMT_JPG,
    pvFmtYUV422Regs         : CAMACQ_MAIN_REG_FMT_YUV422,

    /* Effect */
    pvEffectNoneRegs        : CAMACQ_MAIN_REG_EFFECT_NONE,
    pvEffectGrayRegs        : CAMACQ_MAIN_REG_EFFECT_GRAY,
    pvEffectNegativeRegs    : CAMACQ_MAIN_REG_EFFECT_NEGATIVE,
    pvEffectSolarizeRegs    : CAMACQ_MAIN_REG_EFFECT_SOLARIZE,
    pvEffectSepiaRegs       : CAMACQ_MAIN_REG_EFFECT_SEPIA,
    pvEffectPosterizeRegs   : CAMACQ_MAIN_REG_EFFECT_POSTERIZE,
    pvEffectWhiteBoardRegs  : CAMACQ_MAIN_REG_EFFECT_WHITEBOARD,
    pvEffectBlackBoardRegs  : CAMACQ_MAIN_REG_EFFECT_BLACKBOARD,
    pvEffectAquaRegs        : CAMACQ_MAIN_REG_EFFECT_AQUA,
    pvEffectSharpenRegs     : CAMACQ_MAIN_REG_EFFECT_SHARPEN,
    pvEffectVividRegs       : CAMACQ_MAIN_REG_EFFECT_VIVID,
    pvEffectGreenRegs       : CAMACQ_MAIN_REG_EFFECT_GREEN,
    pvEffectSketchRegs      : CAMACQ_MAIN_REG_EFFECT_SKETCH,

    /* Adjust */
    pvAdjustContrastM2          : CAMACQ_MAIN_REG_ADJUST_CONTRAST_M2,
    pvAdjustContrastM1          : CAMACQ_MAIN_REG_ADJUST_CONTRAST_M1,
    pvAdjustContrastDefault     : CAMACQ_MAIN_REG_ADJUST_CONTRAST_DEFAULT,
    pvAdjustContrastP1          : CAMACQ_MAIN_REG_ADJUST_CONTRAST_P1,
    pvAdjustContrastP2          : CAMACQ_MAIN_REG_ADJUST_CONTRAST_P2,
        
    pvAdjustSharpnessM2         : CAMACQ_MAIN_REG_ADJUST_SHARPNESS_M2,
    pvAdjustSharpnessM1         : CAMACQ_MAIN_REG_ADJUST_SHARPNESS_M1,
    pvAdjustSharpnessDefault    : CAMACQ_MAIN_REG_ADJUST_SHARPNESS_DEFAULT,
    pvAdjustSharpnessP1         : CAMACQ_MAIN_REG_ADJUST_SHARPNESS_P1,
    pvAdjustSharpnessP2         : CAMACQ_MAIN_REG_ADJUST_SHARPNESS_P2,

    pvAdjustSaturationM2        : CAMACQ_MAIN_REG_ADJUST_SATURATION_M2,
    pvAdjustSaturationM1        : CAMACQ_MAIN_REG_ADJUST_SATURATION_M1,
    pvAdjustSaturationDefault   : CAMACQ_MAIN_REG_ADJUST_SATURATION_DEFAULT,
    pvAdjustSaturationP1        : CAMACQ_MAIN_REG_ADJUST_SATURATION_P1,
    pvAdjustSaturationP2        : CAMACQ_MAIN_REG_ADJUST_SATURATION_P2,

    /* Flicker */
    pvFlickerDisabled       : CAMACQ_MAIN_REG_FLICKER_DISABLED,
    pvFlicker50Hz           : CAMACQ_MAIN_REG_FLICKER_50HZ,
    pvFlicker60Hz           : CAMACQ_MAIN_REG_FLICKER_60HZ,
    pvFlickerAuto           : CAMACQ_MAIN_REG_FLICKER_AUTO,

    pvJpegQualitySuperfine  : CAMACQ_MAIN_REG_JPEG_QUALITY_SUPERFINE,
    pvJpegQualityFine       : CAMACQ_MAIN_REG_JPEG_QUALITY_FINE,
    pvJpegQualityNormal     : CAMACQ_MAIN_REG_JPEG_QUALITY_NORMAL,   

    /* Jpeg output size */
    pvJpegOutSize5M         : CAMACQ_MAIN_REG_JPEG_5M,
    pvJpegOutSize5M_2       : CAMACQ_MAIN_REG_JPEG_5M_2,
    pvJpegOutSize4M         : CAMACQ_MAIN_REG_JPEG_W4M,
    pvJpegOutSize3M         : CAMACQ_MAIN_REG_JPEG_3M,
    pvJpegOutSize2M         : CAMACQ_MAIN_REG_JPEG_2M,
    pvJpegOutSize1_5M       : CAMACQ_MAIN_REG_JPEG_W1_5M,
    pvJpegOutSize1M         : CAMACQ_MAIN_REG_JPEG_1M,
    pvJpegOutSizeVGA        : CAMACQ_MAIN_REG_JPEG_VGA,
    pvJpegOutSizeWQVGA      : CAMACQ_MAIN_REG_JPEG_WQVGA,
    pvJpegOutSizeQVGA       : CAMACQ_MAIN_REG_JPEG_QVGA,

    /* FPS */
    pvFpsFixed_5            : CAMACQ_MAIN_REG_FPS_FIXED_5,
    pvFpsFixed_7            : CAMACQ_MAIN_REG_FPS_FIXED_7,
    pvFpsFixed_10           : CAMACQ_MAIN_REG_FPS_FIXED_10,
    pvFpsFixed_15           : CAMACQ_MAIN_REG_FPS_FIXED_15,
    pvFpsFixed_20           : CAMACQ_MAIN_REG_FPS_FIXED_20,
    pvFpsFixed_25           : CAMACQ_MAIN_REG_FPS_FIXED_25,
    pvFpsFixed_30           : CAMACQ_MAIN_REG_FPS_FIXED_30,
    pvFpsAuto_15            : CAMACQ_MAIN_REG_FPS_VAR_15,

    /* Private Control */
    pvPrivCtrlReturnPreview : CAMACQ_MAIN_REG_PRIVCTRL_RETURNPREVIEW,

   /* Camcorder control*/
    pvCamcorderPreview          : CAMACQ_MAIN_REG_CAMCORDER,
    #if defined(CONFIG_BCM_CAM_ISX005) 
    pvPllRegs              : CAMACQ_MAIN_REG_PLL,

    pvSnapshotOutdoorRegs	: CAMACQ_MAIN_REG_OUTDOORSHOT,
    pvSnapshotLowlightRegs	: CAMACQ_MAIN_REG_LOWLIGHTSHOT,
    pvSnapshotNormalRegs	: CAMACQ_MAIN_REG_SNAPSHOT,
    pvSnapshotNightsceneRegs	: CAMACQ_MAIN_REG_NIGHTSHOT,
    pvSnapshotNightsceneOffRegs    : CAMACQ_MAIN_REG_NIGHTSHOT_OFF,
    pvShandingTable_1	: CAMACQ_MAIN_REG_SHADING_1,
    pvShandingTable_2	: CAMACQ_MAIN_REG_SHADING_2,
    pvShandingTable_3	: CAMACQ_MAIN_REG_SHADING_3,
    pvShandingTable_4    : CAMACQ_MAIN_REG_SHADING_4,
    pvCalibrationDefault	: CAMACQ_MAIN_REG_DEFAULT_CAL,	
    #elif defined(CONFIG_BCM_CAM_S5K5CCGX)
    pvSnapshotOutdoorRegs	: CAMACQ_MAIN_REG_HIGHLIGHT,
    pvSnapshotLowlightRegs	: CAMACQ_MAIN_REG_LOWLIGHT,
    pvSnapshotNightsceneRegs	: CAMACQ_MAIN_REG_NIGHTSHOT,
   #endif
 	
};

#if (CAMACQ_SENSOR_MAX==2)
_stCamacqSensorRegs g_stCamacqSubSensorRegs =
{
    pvInitRegs              : CAMACQ_SUB_REG_INIT,
    pvSleepRegs             : CAMACQ_SUB_REG_SLEEP,
    pvWakeupRegs            : CAMACQ_SUB_REG_WAKEUP,
    pvPreviewRegs           : CAMACQ_SUB_REG_PREVIEW,
    pvSnapshotRegs          : CAMACQ_SUB_REG_SNAPSHOT,

    /* ME */
    pvMeterMatrixRegs       : CAMACQ_SUB_REG_METER_MATRIX,
    pvMeterCWRegs           : CAMACQ_SUB_REG_METER_CW,
    pvMeterSpotRegs         : CAMACQ_SUB_REG_METER_SPOT,
    
     /* flip */
    pvFlipNoneRegs          : CAMACQ_SUB_REG_FLIP_NONE,
    pvFlipWaterRegs         : CAMACQ_SUB_REG_FLIP_WATER,
    pvFlipMirrorRegs        : CAMACQ_SUB_REG_FLIP_MIRROR,
    pvFlipWaterMirrorRegs   : CAMACQ_SUB_REG_FLIP_WATER_MIRROR,

    /* ISO */
    pvISOAutoRegs           : CAMACQ_SUB_REG_ISO_AUTO,
    pvISO50Regs             : CAMACQ_SUB_REG_ISO_50,
    pvISO100Regs            : CAMACQ_SUB_REG_ISO_100,
    pvISO200Regs            : CAMACQ_SUB_REG_ISO_200,
    pvISO400Regs            : CAMACQ_SUB_REG_ISO_400,
    pvISO800Regs            : CAMACQ_SUB_REG_ISO_800,
    pvISO1600Regs           : CAMACQ_SUB_REG_ISO_1600,
    pvISO3200Regs           : CAMACQ_SUB_REG_ISO_3200,

    /* Scene */
    pvSceneAutoRegs         : CAMACQ_SUB_REG_SCENE_AUTO, 
    pvSceneNightRegs        : CAMACQ_SUB_REG_SCENE_NIGHT, 
    pvSceneLandScapeRegs    : CAMACQ_SUB_REG_SCENE_LANDSCAPE,
    pvSceneSunSetRegs       : CAMACQ_SUB_REG_SCENE_SUNSET,
    pvScenePortraitRegs     : CAMACQ_SUB_REG_SCENE_PORTRAIT,
    pvSceneSunRiseRegs      : CAMACQ_SUB_REG_SCENE_SUNRISE,
    pvSceneIndoorRegs       : CAMACQ_SUB_REG_SCENE_INDOOR,
    pvScenePartyRegs        : CAMACQ_SUB_REG_SCENE_PARTY,
    pvSceneSportsRegs       : CAMACQ_SUB_REG_SCENE_SPORTS,
    pvSceneBeachRegs        : CAMACQ_SUB_REG_SCENE_BEACH,
    pvSceneSnowRegs         : CAMACQ_SUB_REG_SCENE_SNOW,
    pvSceneFallColorRegs    : CAMACQ_SUB_REG_SCENE_FALLCOLOR,
    pvSceneFireWorksRegs    : CAMACQ_SUB_REG_SCENE_FIREWORKS,
    pvSceneCandleLightRegs  : CAMACQ_SUB_REG_SCENE_CANDLELIGHT,
    pvSceneAgainstLightRegs : CAMACQ_SUB_REG_SCENE_AGAINSTLIGHT,
    pvSceneTextRegs         : CAMACQ_SUB_REG_SCENE_TEXT,

    /* Brightness */
    pvBrightness_0_Regs     : CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_0,
    pvBrightness_1_Regs     : CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_1,
    pvBrightness_2_Regs     : CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_2,
    pvBrightness_3_Regs     : CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_3,
    pvBrightness_4_Regs     : CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_4,
    pvBrightness_5_Regs     : CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_5,
    pvBrightness_6_Regs     : CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_6,
    pvBrightness_7_Regs     : CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_7,
    pvBrightness_8_Regs     : CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_8,

    /* AF */
    pvSetAFRegs             : CAMACQ_SUB_REG_SET_AF,   // start af
    pvOffAFRegs             : CAMACQ_SUB_REG_OFF_AF,
    pvCheckAFRegs           : CAMACQ_SUB_REG_CHECK_AF,
    pvResetAFRegs           : CAMACQ_SUB_REG_RESET_AF,
    pvManualAFReg           : CAMACQ_SUB_REG_MANUAL_AF,
    pvMacroAFReg            : CAMACQ_SUB_REG_MACRO_AF,
    pvReturnManualAFReg     : CAMACQ_SUB_REG_RETURN_MANUAL_AF,
    pvReturnMacroAFReg      : CAMACQ_SUB_REG_RETURN_MACRO_AF,
    pvSetAF_NLUXRegs        : CAMACQ_SUB_REG_SET_AF_NLUX,
    pvSetAF_LLUXRegs        : CAMACQ_SUB_REG_SET_AF_LLUX,

    /* WB */
    pvWbAutoRegs            : CAMACQ_SUB_REG_WB_AUTO,
    pvWbDaylightRegs        : CAMACQ_SUB_REG_WB_DAYLIGHT,
    pvWbCloudyRegs          : CAMACQ_SUB_REG_WB_CLOUDY,
    pvWbIncandescentRegs    : CAMACQ_SUB_REG_WB_INCANDESCENT,
    pvWbFluorescentRegs     : CAMACQ_SUB_REG_WB_FLUORESCENT,
    pvWbTwilightRegs        : CAMACQ_SUB_REG_WB_TWILIGHT,
    pvWbTungstenRegs        : CAMACQ_SUB_REG_WB_TUNGSTEN,
    pvWbOffRegs             : CAMACQ_SUB_REG_WB_OFF,
    pvWbHorizonRegs         : CAMACQ_SUB_REG_WB_HORIZON,
    pvWbShadeRegs           : CAMACQ_SUB_REG_WB_SHADE,

    /* FMT */
    pvFmtJpegRegs           : CAMACQ_SUB_REG_FMT_JPG,
    pvFmtYUV422Regs         : CAMACQ_SUB_REG_FMT_YUV422,

    /* Effect */
    pvEffectNoneRegs        : CAMACQ_SUB_REG_EFFECT_NONE,
    pvEffectGrayRegs        : CAMACQ_SUB_REG_EFFECT_GRAY,
    pvEffectNegativeRegs    : CAMACQ_SUB_REG_EFFECT_NEGATIVE,
    pvEffectSolarizeRegs    : CAMACQ_SUB_REG_EFFECT_SOLARIZE,
    pvEffectSepiaRegs       : CAMACQ_SUB_REG_EFFECT_SEPIA,
    pvEffectPosterizeRegs   : CAMACQ_SUB_REG_EFFECT_POSTERIZE,
    pvEffectWhiteBoardRegs  : CAMACQ_SUB_REG_EFFECT_WHITEBOARD,
    pvEffectBlackBoardRegs  : CAMACQ_SUB_REG_EFFECT_BLACKBOARD,
    pvEffectAquaRegs        : CAMACQ_SUB_REG_EFFECT_AQUA,
    pvEffectSharpenRegs     : CAMACQ_SUB_REG_EFFECT_SHARPEN,
    pvEffectVividRegs       : CAMACQ_SUB_REG_EFFECT_VIVID,
    pvEffectGreenRegs       : CAMACQ_SUB_REG_EFFECT_GREEN,
    pvEffectSketchRegs      : CAMACQ_SUB_REG_EFFECT_SKETCH,

    /* Flicker */
    pvFlickerDisabled       : CAMACQ_SUB_REG_FLICKER_DISABLED,
    pvFlicker50Hz           : CAMACQ_SUB_REG_FLICKER_50HZ,
    pvFlicker60Hz           : CAMACQ_SUB_REG_FLICKER_60HZ,
    pvFlickerAuto           : CAMACQ_SUB_REG_FLICKER_AUTO,

};
#endif /* CAMACQ_SENSOR_MAX==2 */

/* EXT APIs */
/* MAIN H/W */
S32 Switch_MAIN(void)
{
    S32 iRet = 0;
    CamacqTraceIN();

    CamacqTraceOUT();
    return iRet;
}

S32 Reset_MAIN(void)
{
    S32 iRet = 0;
    CamacqTraceIN();

#if defined(GFORCE)
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_OFF ); // RF_IF1 -> MCAM_RSTN
    CamacqExtDelay(10);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_ON ); // RF_IF1 -> MCAM_RSTN
#elif defined(TOTORO)

#endif
    

    CamacqTraceOUT();
    return iRet;
}

S32 Standby_MAIN(void)
{
    S32 iRet = 0;
    CamacqTraceIN();

#if defined(GFORCE)
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO114 ), CAMACQ_OFF ); // RF_IF2 -> MCAM_STBYN
#elif defined(TOTORO)

#endif

    
    CamacqTraceOUT();
    return iRet;
}

S32 Wakeup_MAIN(void)
{
    S32 iRet = 0;
    CamacqTraceIN();
    
#if defined(GFORCE)
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO114 ), CAMACQ_OFF ); // RF_IF2 -> MCAM_STBYN
#elif defined(TOTORO)

#endif

    CamacqTraceOUT();
    return iRet;
}

S32 PowerOn_MAIN(void)
{
    S32 iRet = 0;
    _stCamacqSensor* pstSensor = NULL;
    CamacqTraceIN();

#if defined(_S5K4ECGX_EVT1_MIPI_)
    CamacqTraceDbg("_S5K4ECGX_EVT1_MIPI_ !!!!!!!");
    CamacqTraceDbg("_S5K4ECGX_EVT1_MIPI_ !!!!!!!");
    CamacqTraceDbg("_S5K4ECGX_EVT1_MIPI_ !!!!!!!");

    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO94 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO97 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO109 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO3 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO108 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO93 ), "camacq");


    // 0. reset, stdby all off 
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_MAIN_RST_OFF );    // MCAM_RSTN
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_OFF );            // MCAM_STBYN

    // 1. power up
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_MAIN_EN_ON ); // GPIO109 -> CAM_EN2 -> VCAM_CORE_1.2V
    CamacqExtDelay(10);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO3 ), CAMACQ_MAIN_EN_ON ); // GPIO3 -> CAM_EN4 -> VCAM_AVDD_2.8V	& VCAM_AF_2.8V
    CamacqExtDelay(10);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO108 ), CAMACQ_MAIN_EN_ON ); // GPIO108 -> CAM_EN -> VCAM_IO_2.8V
    CamacqExtDelay(10);

    // 2. MCLK
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 1 );
    g_pstCamacqSensorManager->m_stCamBlock.CamPowerSet( CAMACQ_ON );
    CamacqExtDelay(10);

    // 3. main stdby high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_ON ); // RF_IF2 -> MCAM_STBYN
    CamacqExtDelay(10);

    // 4. main reset high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_MAIN_RST_ON );   // RF_IF1 -> MCAM_RSTN
    CamacqExtDelay(100);  

    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO94 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO97 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO109 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO3 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO108 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO93 ));
  

#elif defined(__ISX006_SONY__)
    // 0. reset, stdby all off 
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_MAIN_RST_OFF );    // MCAM_RSTN
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_OFF );            // MCAM_STBYN

    // 1. power up
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO3 ), CAMACQ_MAIN_EN_ON ); // GPIO3 -> CAM_EN4 -> VCAM_AVDD_2.8V	& VCAM_AF_2.8V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_MAIN_EN_ON ); // GPIO109 -> CAM_EN2 -> VCAM_CORE_1.2V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO108 ), CAMACQ_MAIN_EN_ON ); // GPIO108 -> CAM_EN -> VCAM_IO_2.8V
    CamacqExtDelay(1);

    // 2. MCLK
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 1 );
    g_pstCamacqSensorManager->m_stCamBlock.CamPowerSet( CAMACQ_ON );
    CamacqExtDelay(1);

    // 3. main reset high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_MAIN_RST_ON );   // RF_IF1 -> MCAM_RSTN
    CamacqExtDelay(10);

    // 4. main stdby high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_ON ); // RF_IF2 -> MCAM_STBYN
    CamacqExtDelay(100); 

/*
    // Switch 
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO101 ), 0 ); // GPIO101 -> CAM_SW_EN -> LOW_ENABLE
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO100 ), 0 ); // GPIO100 -> CAM_SEL -> 0 is A
    CamacqExtDelay(10);

    // 0. reset, stdby all off 
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_MAIN_RST_OFF );   // RF_IF1 -> MCAM_RSTN
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO114 ), CAMACQ_OFF ); // RF_IF2 -> MCAM_STBYN
        
    // 1. power up
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_MAIN_EN_ON ); // GPIO109 -> CAM_EN4 -> VCAM_AVDD_2.8V	& VCAM_AF_2.8V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO107 ), CAMACQ_MAIN_EN_ON ); // GPIO107 -> CAM_EN2 -> VCAM_CORE_1.2V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO106 ), CAMACQ_MAIN_EN_ON ); // GPIO106 -> CAM_EN -> VCAM_IO_2.8V
    CamacqExtDelay(1);
    
    // 2. MCLK
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 1 );
    CamacqExtDelay(1);

    // 6. main reset high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_MAIN_RST_ON );   // RF_IF1 -> MCAM_RSTN
    CamacqExtDelay(10);

    // 5. main stdby high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO114 ), CAMACQ_ON ); // RF_IF2 -> MCAM_STBYN
    CamacqExtDelay(100); 
*/   
#elif defined(TEMP)
    /* Switch */ 
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO101 ), 0 ); // GPIO101 -> CAM_SW_EN -> LOW_ENABLE
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO100 ), 0 ); // GPIO100 -> CAM_SEL -> 0 is A
    CamacqExtDelay(10);

    // 0. both reset, stdby all off 
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_MAIN_RST_OFF );   // RF_IF1 -> MCAM_RSTN
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_SUB_RST_OFF );     // GPIO94 -> SCAM_RSTN
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO114 ), CAMACQ_OFF ); // RF_IF2 -> MCAM_STBYN
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_OFF ); // GPIO97 -> SCAM_STBYN
    
    // 1. both power up
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_MAIN_EN_ON ); // GPIO109 -> CAM_EN4 -> VCAM_AVDD_2.8V	& VCAM_AF_2.8V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO108 ), CAMACQ_SUB_EN_ON ); // GPIO108 -> CAM_EN3 -> VCAM_DVDD_1.5V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO107 ), CAMACQ_MAIN_EN_ON ); // GPIO107 -> CAM_EN2 -> VCAM_CORE_1.2V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO106 ), CAMACQ_MAIN_EN_ON ); // GPIO106 -> CAM_EN -> VCAM_IO_2.8V
    CamacqExtDelay(1);
    
    // 2. MCLK
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 1 );
    CamacqExtDelay(1);

    // 3. sub stbny high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_ON ); // GPIO97 -> SCAM_STBYN
    CamacqExtDelay(1);

    // 4. sub reset high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_SUB_RST_ON );     // GPIO94 -> SCAM_RSTN
    CamacqExtDelay(1);

    // 5. sub arm go & enter stdby
    pstSensor = g_pstCamacqSensorManager->GetSensor( g_pstCamacqSensorManager, CAMACQ_SENSOR_SUB );
    if( pstSensor->m_pI2cClient != 0 ) {
        CamacqExtWriteI2cLists( pstSensor->m_pI2cClient, pstSensor->m_pstExtAPIs->m_pstSensorRegs->pvSleepRegs, pstSensor->m_uiResType );
        CamacqExtDelay(10);
    }

    // 6. main reset high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_MAIN_RST_ON );   // RF_IF1 -> MCAM_RSTN
    CamacqExtDelay(10);

    // 5. main stdby high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO114 ), CAMACQ_ON ); // RF_IF2 -> MCAM_STBYN
    CamacqExtDelay(100); 
    
#elif defined(TOTORO)
    
#else

#endif /* __ISX006_SONY__ */ 
    
    CamacqTraceOUT();
    return iRet;
}

S32 PowerOff_MAIN(void)
{
    S32 iRet = 0;
    _stCamacqSensor* pstSensor = NULL;
    CamacqTraceIN();
    

#if defined(_S5K4ECGX_EVT1_MIPI_)

    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO94 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO97 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO109 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO3 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO108 ), "camacq");
    CamacqExtRequestGpio(mfp_to_gpio( MFP_PIN_GPIO93 ), "camacq");

    // reset down
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_MAIN_RST_OFF ); // GPIO94 -> MCAM_RSTN
    CamacqExtDelay(50);

    // Clock
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 0 ); // GPIO93 -> CAM_MCLK
    g_pstCamacqSensorManager->m_stCamBlock.CamPowerSet( CAMACQ_OFF );
    CamacqExtDelay(10);

    // Standby
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_OFF ); // GPIO97 MCAM_STBYN
    CamacqExtDelay(10);

    // Power off
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_MAIN_EN_OFF ); // GPIO109  VCAM_CORE_1.2V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO108 ), CAMACQ_MAIN_EN_OFF ); // GPIO108 VCAM_IO_2.8V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO3 ), CAMACQ_MAIN_EN_OFF ); // GPIO103 VCAM_AVDD_2.8V	& VCAM_AF_2.8V
    CamacqExtDelay(10);

    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO94 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO97 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO109 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO3 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO108 ));
    CamacqExtFreeGpio(mfp_to_gpio( MFP_PIN_GPIO93 ));
   
#elif defined(__ISX006_SONY__)

    // Standby down
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_OFF ); // GPIO97 MCAM_STBYN
    CamacqExtDelay(10);

    // reset down
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_MAIN_RST_OFF ); // GPIO94 -> MCAM_RSTN
    CamacqExtDelay(10);

    // Clock
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 0 ); // GPIO93 -> CAM_MCLK
    g_pstCamacqSensorManager->m_stCamBlock.CamPowerSet( CAMACQ_OFF );
    CamacqExtDelay(10);

    // Power off
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_MAIN_EN_OFF ); // GPIO109  VCAM_CORE_1.2V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO108 ), CAMACQ_MAIN_EN_OFF ); // GPIO108 VCAM_IO_2.8V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO3 ), CAMACQ_MAIN_EN_OFF ); // GPIO103 VCAM_AVDD_2.8V	& VCAM_AF_2.8V
    CamacqExtDelay(10);
/*
    // Standby
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO114 ), CAMACQ_OFF ); // RF_IF2 -> MCAM_STBYN
    CamacqExtDelay(10);

    // reset down
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_MAIN_RST_OFF ); // RF_IF1 -> MCAM_RSTN
    CamacqExtDelay(10);

    // Clock
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 0 ); // GPIO93 -> CAM_MCLK
    CamacqExtDelay(10);

    // Power
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO107 ), CAMACQ_MAIN_EN_OFF ); // GPIO107 -> CAM_EN2 -> VCAM_CORE_1.2V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO106 ), CAMACQ_MAIN_EN_OFF ); // GPIO106 -> CAM_EN -> VCAM_IO_2.8V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_MAIN_EN_OFF ); // GPIO109 -> CAM_EN4 -> VCAM_AVDD_2.8V	& VCAM_AF_2.8V
    CamacqExtDelay(10);
*/
#elif defined(TEMP)
    /* Standby */
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO114 ), CAMACQ_OFF ); // RF_IF2 -> MCAM_STBYN
    CamacqExtDelay(10);

    /* reset down */
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_MAIN_RST_OFF ); // RF_IF1 -> MCAM_RSTN
    CamacqExtDelay(10);

    /* Clock */
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 0 ); // GPIO93 -> CAM_MCLK
    CamacqExtDelay(10);

    /* Power */
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO107 ), CAMACQ_MAIN_EN_OFF ); // GPIO107 -> CAM_EN2 -> VCAM_CORE_1.2V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO106 ), CAMACQ_MAIN_EN_OFF ); // GPIO106 -> CAM_EN -> VCAM_IO_2.8V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_MAIN_EN_OFF ); // GPIO109 -> CAM_EN4 -> VCAM_AVDD_2.8V	& VCAM_AF_2.8V
    CamacqExtDelay(10);

#elif defined(TOTORO)

#endif
    CamacqTraceOUT();
    return iRet;
}

/* SUB H/W */
#if (CAMACQ_SENSOR_MAX==2)
S32 Switch_SUB(void)
{
    S32 iRet = 0;
    CamacqTraceIN();

   
    CamacqTraceOUT();
    return iRet;
}

S32 Reset_SUB(void)
{
    S32 iRet = 0;
    CamacqTraceIN();

    CamacqTraceOUT();
    return iRet;
}

S32 Standby_SUB(void)
{
    S32 iRet = 0;
    CamacqTraceIN();

       
    CamacqTraceOUT();
    return iRet;
}

S32 Wakeup_SUB(void)
{
    S32 iRet = 0;
    CamacqTraceIN();

    CamacqTraceOUT();
    return iRet;
}

S32 PowerOn_SUB(void)
{
    S32 iRet = 0;
    CamacqTraceIN();

#if defined(TEMP)
    /* Switch */
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO101 ), 0 ); // GPIO101 -> CAM_SW_EN, LOW_ENABLE
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO100 ), 1 ); // GPIO100 -> CAM_SEL, 1 is B, B is SCAM
    CamacqExtDelay(10);

    // 0. both reset, stdny all off 
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_MAIN_RST_OFF );   // RF_IF1 -> MCAM_RSTN
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_SUB_RST_OFF );     // GPIO94 -> SCAM_RSTN
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO114 ), CAMACQ_OFF ); // RF_IF2 -> MCAM_STBYN
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_OFF ); // GPIO97 -> SCAM_STBYN
    
    // 1. both power up
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_MAIN_EN_ON ); // GPIO109 -> CAM_EN4 -> VCAM_AVDD_2.8V	& VCAM_AF_2.8V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO108 ), CAMACQ_SUB_EN_ON ); // GPIO108 -> CAM_EN3 -> VCAM_DVDD_1.5V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO107 ), CAMACQ_MAIN_EN_ON ); // GPIO107 -> CAM_EN2 -> VCAM_CORE_1.2V
    CamacqExtDelay(1);
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO106 ), CAMACQ_MAIN_EN_ON ); // GPIO106 -> CAM_EN -> VCAM_IO_2.8V
    CamacqExtDelay(1);

    // 2. sub stdby high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_ON ); // GPIO97 -> SCAM_STBYN
    CamacqExtDelay(1);

    // 3. MCLK
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 1 );
    CamacqExtDelay(10);

    // 4. main reset high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO113 ), CAMACQ_MAIN_RST_ON );   // RF_IF1 -> MCAM_RSTN
    CamacqExtDelay(10);

    // 5. sub reset high
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_SUB_RST_ON );     // GPIO94 -> SCAM_RSTN
    CamacqExtDelay(1);

#endif
    CamacqTraceOUT();
    return iRet;
}

S32 PowerOff_SUB(void)
{
    S32 iRet = 0;
    CamacqTraceIN();

#if defined(TEMP)
    /* reset down */
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO94 ), CAMACQ_SUB_RST_OFF ); // GPIO94 -> SCAM_RSTN
    CamacqExtDelay(30);

    /* Clock */
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO93 ), 0 ); // GPIO93 -> CAM_MCLK
    CamacqExtDelay(10);

    /* Standby */
    // camacq_ext_standby_begin( irestype );
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO97 ), CAMACQ_OFF ); // GPIO97 -> SCAM_STBYN
    CamacqExtDelay(10);
   
    /* Power */
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO108 ), CAMACQ_SUB_EN_OFF ); // GPIO108 -> CAM_EN3 -> VCAM_DVDD1.5V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO106 ), CAMACQ_SUB_EN_OFF ); // GPIO106 -> CAM_EN -> VCAM_IO_2.8V
    gpio_direction_output( mfp_to_gpio( MFP_PIN_GPIO109 ), CAMACQ_SUB_EN_OFF ); // GPIO109 -> CAM_EN4 -> VCAM_AVDD_2.8V	
    CamacqExtDelay(10);

#endif
        
    CamacqTraceOUT();
    return iRet;
}
#endif /* CAMACQ_SENSOR_MAX==2  */

S32 CamacqExtBoot( struct stCamacqExtAPIs_t* this )
{
    S32 iRet = 0;
    _stCamacqSensor* pSensor = this->m_pstSensor;
    CamacqTraceIN();

    if( this->m_pstSensorRegs->pvInitRegs != NULL )
    {
        CamacqExtWriteI2cLists( pSensor->m_pI2cClient, this->m_pstSensorRegs->pvInitRegs, pSensor->m_uiResType );
    }
    else
    {
        iRet = -1;
         CamacqTraceErr( "pvInitRegs is NULL" );
    }
   
    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtHflip( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    CamacqTraceIN( ":piVal[%d]", *piVal );
    
    iRet = CamacqExtSetFlip( this, CAMACQ_EXT_HFLIP, *piVal );
    if(iRet < 0)
    {
        CamacqTraceErr( ":hflip failed" );
        iRet = -EINVAL;
    }

    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtVflip( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    CamacqTraceIN( ":piVal[%d]", *piVal );
    
    iRet = CamacqExtSetFlip( this, CAMACQ_EXT_VFLIP, *piVal );
    if(iRet < 0)
    {
        CamacqTraceErr(":vflip failed");
        iRet = -EINVAL;
    }

    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtSetFlip( struct stCamacqExtAPIs_t* this, U8 ucFlip, U8 ucVal )
{
    static U8 ucCurFlip = CAMACQ_EXT_NONE;
    S32 iRet = 0;
    const void* pvRegs = NULL;

    CamacqTraceIN();
    CamacqTraceDbg( " :ucFlip[%d] ucVal[%d] ", ucFlip, ucVal );

    if( ucFlip == CAMACQ_EXT_HFLIP )
    {
        ucCurFlip = (ucCurFlip & ~CAMACQ_EXT_HFLIP) + ucVal;
    }
    else
    {
        ucCurFlip = (ucCurFlip & ~CAMACQ_EXT_VFLIP) + (ucVal << 1);
    }

    switch(ucCurFlip)
    {
        case CAMACQ_EXT_NONE:
        {
            CamacqTraceDbg(" CAMACQ_EXT_NONE ");
            pvRegs = this->m_pstSensorRegs->pvFlipNoneRegs;
        }
        break;
        case CAMACQ_EXT_HFLIP:
        {
            CamacqTraceDbg(" CAMACQ_EXT_HFLIP ");
            pvRegs = this->m_pstSensorRegs->pvFlipMirrorRegs;
        }
        break;
        case CAMACQ_EXT_VFLIP:
        {
            CamacqTraceDbg(" CAMACQ_EXT_VFLIP ");
            pvRegs = this->m_pstSensorRegs->pvFlipWaterRegs;
        }
        break;
        case (CAMACQ_EXT_HFLIP + CAMACQ_EXT_VFLIP):
        {
            CamacqTraceDbg(" CAMACQ_EXT_HFLIP + CAMACQ_EXT_VFLIP ");
            pvRegs = this->m_pstSensorRegs->pvFlipWaterMirrorRegs;
        }
        break;
        default:
        {
            CamacqTraceErr( ":invalid flip mode" );
            pvRegs = NULL;
            iRet = -EINVAL; 
        }
        break;
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }

    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtExposure( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;

    CamacqTraceIN();
    CamacqTraceDbg( ":piVal[%d]", *piVal );
    
    switch( (enum v4l2_ctrl_metering_exposure)*piVal )
    {
        case V4L2_CTRL_MET_EXP_MATRIX:
        {
            CamacqTraceDbg(" V4L2_CTRL_MET_EXP_MATRIX ");
            pvRegs = this->m_pstSensorRegs->pvMeterMatrixRegs;
        }
        break;
        case V4L2_CTRL_MET_EXP_CENTERWEIGHTED:
        {
            CamacqTraceDbg(" V4L2_CTRL_MET_EXP_CENTERWEIGHTED ");
            pvRegs = this->m_pstSensorRegs->pvMeterCWRegs;
        }
        break;
        case V4L2_CTRL_MET_EXP_SPOT:
        {
            CamacqTraceDbg(" V4L2_CTRL_MET_EXP_SPOT ");
            pvRegs = this->m_pstSensorRegs->pvMeterSpotRegs;
        }
        break;
        default:
        {
            CamacqTraceErr( ":invalid psival[%d]", *piVal );
            pvRegs =NULL;
            iRet = -EINVAL;
        }
        break;
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }

    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtSensitivity( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;

    CamacqTraceIN();
    CamacqTraceDbg( ":piVal[%d]", *piVal );

#if defined(_S5K4ECGX_EVT1_MIPI_)
    /* READ 0x04E6 */
    int iRegTcDbgAutoAlgEnBits = 0;
    U8	rgucWriteRegs_1[4] = {0x00,0x2C,0x70,0x00};
    U8	rgucWriteRegs_2[4] = {0x00,0x2E,0x04,0xE6};
    U8	rgucWriteRegs_3[4] = {0x00,0x28,0x70,0x00};
    U8	rgucWriteRegs_4[4] = {0x00,0x2A,0x04,0xE6};
    U8  rgucWriteRegs[4] = {0x00, };
    U8  rgucReadData[2] = {0, };
    U16 usReadAddr = 0x0F12;

    CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_1, 4 );
    CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_2, 4 );
    CamacqExtReadI2c( this->m_pstSensor->m_pI2cClient, usReadAddr, 2, rgucReadData, 2 );

    iRegTcDbgAutoAlgEnBits = ( rgucReadData[0] << 8 );
    iRegTcDbgAutoAlgEnBits |= rgucReadData[1];

    CamacqTraceDbg( "iRegTcDbgAutoAlgEnBits : 0x%x, rgucReadData[0] : 0x%x, rgucReadData[1] : 0x%x",
            iRegTcDbgAutoAlgEnBits, rgucReadData[0], rgucReadData[1] );

    if( (enum v4l2_ctrl_sensitivity)*piVal == V4L2_CTRL_ISO_AUTO )
    {
        iRegTcDbgAutoAlgEnBits = iRegTcDbgAutoAlgEnBits | 0x20;
    }
    else
    {
        iRegTcDbgAutoAlgEnBits = iRegTcDbgAutoAlgEnBits & 0xFFDF;
    }

    CamacqTraceDbg("iRegTcDbgAutoAlgEnBits : 0x%X", iRegTcDbgAutoAlgEnBits);

    rgucWriteRegs[0] = 0x0F; rgucWriteRegs[1] = 0x12;
    rgucWriteRegs[2] = (U8)(iRegTcDbgAutoAlgEnBits >> 8);
    rgucWriteRegs[3] = (U8)iRegTcDbgAutoAlgEnBits;

    CamacqTraceDbg("rgucWriteRegs: 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
    CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_3, 4);
    CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_4, 4);
    CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs, 4 );

#endif /* _S5K4ECGX_EVT1_MIPI_ */

    
    switch( (enum v4l2_ctrl_sensitivity)*piVal )
    {
        case V4L2_CTRL_ISO_AUTO:
        {
            CamacqTraceDbg(" V4L2_CTRL_ISO_AUTO ");
            pvRegs = this->m_pstSensorRegs->pvISOAutoRegs;
        }
        break;
        case V4L2_CTRL_ISO_50:
        {
            CamacqTraceDbg(" V4L2_CTRL_ISO_50 ");
            pvRegs = this->m_pstSensorRegs->pvISO50Regs; 
        }
        break;
        case V4L2_CTRL_ISO_100:
        {
            CamacqTraceDbg(" V4L2_CTRL_ISO_100 ");
            pvRegs = this->m_pstSensorRegs->pvISO100Regs; 
        }
        break;
        case V4L2_CTRL_ISO_200:
        {
            CamacqTraceDbg(" V4L2_CTRL_ISO_200 ");
            pvRegs = this->m_pstSensorRegs->pvISO200Regs;
        }
        break;
        case V4L2_CTRL_ISO_400:
        {
            CamacqTraceDbg(" V4L2_CTRL_ISO_400 ");
            pvRegs = this->m_pstSensorRegs->pvISO400Regs;
        }
        break;
        case V4L2_CTRL_ISO_800:
        {
            CamacqTraceDbg(" V4L2_CTRL_ISO_800 ");
            pvRegs = this->m_pstSensorRegs->pvISO800Regs;

        }
        break;
        case V4L2_CTRL_ISO_1600:
        {
            CamacqTraceDbg(" V4L2_CTRL_ISO_1600 ");
            pvRegs = this->m_pstSensorRegs->pvISO1600Regs;
        }
        break;
        case V4L2_CTRL_ISO_3200:
        {
            CamacqTraceDbg(" V4L2_CTRL_ISO_3200 ");
            pvRegs = this->m_pstSensorRegs->pvISO3200Regs;
        }
        break;
        default:
        {
            CamacqTraceErr( ":invalid piVal[%d]", *piVal );
            pvRegs = NULL;
            iRet = -EINVAL;
        }
        break;
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }
     
    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtScene( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;
    CamacqTraceIN();
    CamacqTraceDbg( " :piVal[%d] ", *piVal );

#if defined(_S5K4ECGX_EVT1_MIPI_)
    /* READ 0x04E6 */
    int iRegTcDbgAutoAlgEnBits = 0;
    U8	rgucWriteRegs_1[4] = {0x00,0x2C,0x70,0x00};
    U8	rgucWriteRegs_2[4] = {0x00,0x2E,0x04,0xE6};
    U8	rgucWriteRegs_3[4] = {0x00,0x28,0x70,0x00};
    U8	rgucWriteRegs_4[4] = {0x00,0x2A,0x04,0xE6};
    U8  rgucWriteRegs[4] = {0x00, };
    U8  rgucReadData[2] = {0, };
    U16 usReadAddr = 0x0F12;
    bool bNeedChange = true;

    CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_1, 4 );
    CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_2, 4 );
    CamacqExtReadI2c( this->m_pstSensor->m_pI2cClient, usReadAddr, 2, rgucReadData, 2 );

    iRegTcDbgAutoAlgEnBits = ( rgucReadData[0] << 8 );
    iRegTcDbgAutoAlgEnBits |= rgucReadData[1];

    CamacqTraceDbg( "iRegTcDbgAutoAlgEnBits : 0x%x, rgucReadData[0] : 0x%x, rgucReadData[1] : 0x%x",
            iRegTcDbgAutoAlgEnBits, rgucReadData[0], rgucReadData[1] );

    if( ((enum v4l2_ctrl_scene)*piVal ==  V4L2_CTRL_SCENE_PARTY) || ((enum v4l2_ctrl_scene)*piVal ==  V4L2_CTRL_SCENE_INDOOR) )
    {
        iRegTcDbgAutoAlgEnBits = iRegTcDbgAutoAlgEnBits & 0xFFDF;
    }
    else if( ((enum v4l2_ctrl_scene)*piVal ==  V4L2_CTRL_SCENE_BEACH) || ((enum v4l2_ctrl_scene)*piVal ==  V4L2_CTRL_SCENE_SNOW) )
    {
        iRegTcDbgAutoAlgEnBits = iRegTcDbgAutoAlgEnBits & 0xFFDF;
    }
    else if( ((enum v4l2_ctrl_scene)*piVal ==  V4L2_CTRL_SCENE_SUNSET) )
    {
        iRegTcDbgAutoAlgEnBits = iRegTcDbgAutoAlgEnBits & 0xFFF7;
    }
    else if( ((enum v4l2_ctrl_scene)*piVal ==  V4L2_CTRL_SCENE_DUSKDAWN) )
    {
        iRegTcDbgAutoAlgEnBits = iRegTcDbgAutoAlgEnBits & 0xFFF7;
    }
    else if( ((enum v4l2_ctrl_scene)*piVal ==  V4L2_CTRL_SCENE_CANDLELIGHT) )
    {
        iRegTcDbgAutoAlgEnBits = iRegTcDbgAutoAlgEnBits & 0xFFF7;
    }
    else
    {
        bNeedChange = false;
        CamacqTraceDbg("Dont need change");
    }

    if( bNeedChange )
    {   
        CamacqTraceDbg("iRegTcDbgAutoAlgEnBits : 0x%X", iRegTcDbgAutoAlgEnBits);
        rgucWriteRegs[0] = 0x0F; rgucWriteRegs[1] = 0x12;
        rgucWriteRegs[2] = (U8)(iRegTcDbgAutoAlgEnBits >> 8);
        rgucWriteRegs[3] = (U8)iRegTcDbgAutoAlgEnBits;

        CamacqTraceDbg("rgucWriteRegs: 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
        CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_3, 4);
        CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_4, 4);
        CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs, 4 );
    }    
#endif /* _S5K4ECGX_EVT1_MIPI_ */


    switch( (enum v4l2_ctrl_scene)*piVal )
    {
        case V4L2_CTRL_SCENE_AUTO:
        {   
            CamacqTraceDbg(" V4L2_CTRL_SCENE_AUTO ");
            pvRegs = this->m_pstSensorRegs->pvSceneAutoRegs;
        }
        break;
        case V4L2_CTRL_SCENE_NIGHT:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_NIGHT ");
            pvRegs = this->m_pstSensorRegs->pvSceneNightRegs;
        }
        break;
        case V4L2_CTRL_SCENE_LANDSCAPE:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_LANDSCAPE ");
            pvRegs = this->m_pstSensorRegs->pvSceneLandScapeRegs;
        }
        break;
        case V4L2_CTRL_SCENE_SUNSET:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_SUNSET ");
            pvRegs = this->m_pstSensorRegs->pvSceneSunSetRegs;
        }
        break;
        case V4L2_CTRL_SCENE_PORTRAIT:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_PORTRAIT ");
            pvRegs = this->m_pstSensorRegs->pvScenePortraitRegs;
        }
        break;
        case V4L2_CTRL_SCENE_SUNRISE:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_SUNRISE ");
            pvRegs = this->m_pstSensorRegs->pvSceneSunRiseRegs;
        }
        break;
        case V4L2_CTRL_SCENE_INDOOR:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_INDOOR ");
            pvRegs = this->m_pstSensorRegs->pvSceneIndoorRegs;
        }
        break;
        case V4L2_CTRL_SCENE_PARTY:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_PARTY ");
            pvRegs = this->m_pstSensorRegs->pvScenePartyRegs;
        }
        break;
        case V4L2_CTRL_SCENE_SPORTS:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_SPORTS ");
            pvRegs = this->m_pstSensorRegs->pvSceneSportsRegs;
        }
        break;
        case V4L2_CTRL_SCENE_BEACH:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_BEACH ");
            pvRegs = this->m_pstSensorRegs->pvSceneBeachRegs;
        }
        break;
        case V4L2_CTRL_SCENE_SNOW:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_SNOW ");
            pvRegs = this->m_pstSensorRegs->pvSceneSnowRegs;
        }
        break;
        case V4L2_CTRL_SCENE_FALLCOLOR:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_FALLCOLOR ");
            pvRegs = this->m_pstSensorRegs->pvSceneFallColorRegs;
        }
        break;
        case V4L2_CTRL_SCENE_FIREWORKS:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_FIREWORKS ");
            pvRegs = this->m_pstSensorRegs->pvSceneFireWorksRegs;
        }
        break;
        case V4L2_CTRL_SCENE_CANDLELIGHT:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_CANDLELIGHT ");
            pvRegs = this->m_pstSensorRegs->pvSceneCandleLightRegs;
        }
        break;
        case V4L2_CTRL_SCENE_AGAINSTLIGHT:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_AGAINSTLIGHT ");
            pvRegs = this->m_pstSensorRegs->pvSceneAgainstLightRegs;
        }
        break;
        case V4L2_CTRL_SCENE_TEXT:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_TEXT ");
            pvRegs = this->m_pstSensorRegs->pvSceneTextRegs;
        }
        break;
        case V4L2_CTRL_SCENE_DUSKDAWN:
        {
            CamacqTraceDbg(" V4L2_CTRL_SCENE_DUSKDAWN ");
            pvRegs = this->m_pstSensorRegs->pvSceneSunRiseRegs; // temp
        }
        default:
        {
            CamacqTraceErr(  ":invalid psival[%d] ", *piVal );
            pvRegs = NULL;
            iRet = -EINVAL;
        }
        break;
    }

    if( pvRegs )    
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );

        // save 
        this->m_pstSensor->m_pstCamacqSettings->stV4l2CurScene = (enum v4l2_ctrl_scene)*piVal;
    }

    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtBrightness( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;
    CamacqTraceIN();
    
    CamacqTraceDbg( ":piVal[%d] ", *piVal );
    switch(*piVal)
    {
        case CAMACQ_EXT_BR_LVL_0:
        {
            CamacqTraceDbg(" CAMACQ_EXT_BR_LVL_0 ");
            pvRegs = this->m_pstSensorRegs->pvBrightness_0_Regs;
        }
        break;
        case CAMACQ_EXT_BR_LVL_1:
        {
            CamacqTraceDbg(" CAMACQ_EXT_BR_LVL_1 ");
            pvRegs = this->m_pstSensorRegs->pvBrightness_1_Regs;
        }
        break;
        case CAMACQ_EXT_BR_LVL_2:
        {
            CamacqTraceDbg(" CAMACQ_EXT_BR_LVL_2 ");
            pvRegs = this->m_pstSensorRegs->pvBrightness_2_Regs;
        }
        break;
        case CAMACQ_EXT_BR_LVL_3:
        {
            CamacqTraceDbg(" CAMACQ_EXT_BR_LVL_3 ");
            pvRegs = this->m_pstSensorRegs->pvBrightness_3_Regs;
        }
        break;
        case CAMACQ_EXT_BR_LVL_4:
        {
            CamacqTraceDbg(" CAMACQ_EXT_BR_LVL_4 ");
            pvRegs = this->m_pstSensorRegs->pvBrightness_4_Regs;
        }
        break;
        case CAMACQ_EXT_BR_LVL_5:
        {
            CamacqTraceDbg(" CAMACQ_EXT_BR_LVL_5 ");
            pvRegs = this->m_pstSensorRegs->pvBrightness_5_Regs;
        }
        break;
        case CAMACQ_EXT_BR_LVL_6:
        {
            CamacqTraceDbg(" CAMACQ_EXT_BR_LVL_6 ");
            pvRegs = this->m_pstSensorRegs->pvBrightness_6_Regs;
        }
        break;
        case CAMACQ_EXT_BR_LVL_7:
        {
            CamacqTraceDbg(" CAMACQ_EXT_BR_LVL_7 ");
            pvRegs = this->m_pstSensorRegs->pvBrightness_7_Regs;
        }
        break;
        case CAMACQ_EXT_BR_LVL_8:
        {
            CamacqTraceDbg(" CAMACQ_EXT_BR_LVL_8 ");
            pvRegs = this->m_pstSensorRegs->pvBrightness_8_Regs;
        }
        break;
        default:
        {
            CamacqTraceDbg( " :invalid psival[%d] ", *piVal );
            pvRegs = NULL;
            iRet = -EINVAL;
        }
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }

    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtExposureCompensation( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;
    CamacqTraceIN();
    
    CamacqTraceDbg( ":piVal[%d] ", *piVal );
    switch(*piVal)
    {
        case CAMACQ_EXT_EXP_COMP_LVL_0:
        {
            CamacqTraceDbg(" CAMACQ_EXT_EXP_COMP_LVL_0 ");
            pvRegs = this->m_pstSensorRegs->pvExpCompensation_0_Regs;
        }
        break;
        case CAMACQ_EXT_EXP_COMP_LVL_1:
        {
            CamacqTraceDbg(" CAMACQ_EXT_EXP_COMP_LVL_1 ");
            pvRegs = this->m_pstSensorRegs->pvExpCompensation_1_Regs;
        }
        break;
        case CAMACQ_EXT_EXP_COMP_LVL_2:
        {
            CamacqTraceDbg(" CAMACQ_EXT_EXP_COMP_LVL_2 ");
            pvRegs = this->m_pstSensorRegs->pvExpCompensation_2_Regs;
        }
        break;
        case CAMACQ_EXT_EXP_COMP_LVL_3:
        {
            CamacqTraceDbg(" CAMACQ_EXT_EXP_COMP_LVL_3 ");
            pvRegs = this->m_pstSensorRegs->pvExpCompensation_3_Regs;
        }
        break;
        case CAMACQ_EXT_EXP_COMP_LVL_4:
        {
            CamacqTraceDbg(" CAMACQ_EXT_EXP_COMP_LVL_4 ");
            pvRegs = this->m_pstSensorRegs->pvExpCompensation_4_Regs;
        }
        break;
        case CAMACQ_EXT_EXP_COMP_LVL_5:
        {
            CamacqTraceDbg(" CAMACQ_EXT_EXP_COMP_LVL_5 ");
            pvRegs = this->m_pstSensorRegs->pvExpCompensation_5_Regs;
        }
        break;
        case CAMACQ_EXT_EXP_COMP_LVL_6:
        {
            CamacqTraceDbg(" CAMACQ_EXT_EXP_COMP_LVL_6 ");
            pvRegs = this->m_pstSensorRegs->pvExpCompensation_6_Regs;
        }
        break;
        case CAMACQ_EXT_EXP_COMP_LVL_7:
        {
            CamacqTraceDbg(" CAMACQ_EXT_EXP_COMP_LVL_7 ");
            pvRegs = this->m_pstSensorRegs->pvExpCompensation_7_Regs;
        }
        break;
        case CAMACQ_EXT_EXP_COMP_LVL_8:
        {
            CamacqTraceDbg(" CAMACQ_EXT_EXP_COMP_LVL_8 ");
            pvRegs = this->m_pstSensorRegs->pvExpCompensation_8_Regs;
        }
        break;
        default:
        {
            CamacqTraceDbg( " :invalid psival[%d] ", *piVal );
            pvRegs = NULL;
            iRet = -EINVAL;
        }
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }

    CamacqTraceOUT();
    return iRet;
}


S32 CamacqExtSaturation( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;
    CamacqTraceIN();
    
    CamacqTraceDbg( ":piVal[%d] ", *piVal );
    switch(*piVal)
    {
        case CAMACQ_EXT_ADJUST_SATURATION_LVL_M2:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SATURATION_LVL_M2 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSaturationM2;
        }
        break;
        case CAMACQ_EXT_ADJUST_SATURATION_LVL_M1:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SATURATION_LVL_M1 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSaturationM1;
        }
        break;
        case CAMACQ_EXT_ADJUST_SATURATION_LVL_DEFAULT:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SATURATION_LVL_DEFAULT ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSaturationDefault;
        }
        break;
        case CAMACQ_EXT_ADJUST_SATURATION_LVL_P1:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SATURATION_LVL_P1 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSaturationP1;
        }
        break;
        case CAMACQ_EXT_ADJUST_SATURATION_LVL_P2:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SATURATION_LVL_P2 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSaturationP2;
        }
        break;
        
        default:
        {
            CamacqTraceDbg( " :invalid psival[%d] ", *piVal );
            pvRegs = NULL;
            iRet = -EINVAL;
        }
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }


    
    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtContrast( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;
    CamacqTraceIN();
    
    CamacqTraceDbg( ":piVal[%d] ", *piVal );
    
    switch(*piVal)
    {
        case CAMACQ_EXT_ADJUST_CONTRAST_LVL_M2:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_CONTRAST_LVL_M2 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustContrastM2;
        }
        break;
        case CAMACQ_EXT_ADJUST_CONTRAST_LVL_M1:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_CONTRAST_LVL_M1 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustContrastM1;
        }
        break;
        case CAMACQ_EXT_ADJUST_CONTRAST_LVL_DEFAULT:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_CONTRAST_LVL_DEFAULT ");
            pvRegs = this->m_pstSensorRegs->pvAdjustContrastDefault;
        }
        break;
        case CAMACQ_EXT_ADJUST_CONTRAST_LVL_P1:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_CONTRAST_LVL_P1 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustContrastP1;
        }
        break;
        case CAMACQ_EXT_ADJUST_CONTRAST_LVL_P2:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_CONTRAST_LVL_P2 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustContrastP2;
        }
        break;
        
        default:
        {
            CamacqTraceDbg( " :invalid psival[%d] ", *piVal );
            pvRegs = NULL;
            iRet = -EINVAL;
        }
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }
   
    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtSharpness( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;
    CamacqTraceIN();
    
    CamacqTraceDbg( ":piVal[%d] ", *piVal );
    switch(*piVal)
    {
        case CAMACQ_EXT_ADJUST_SHARPNESS_LVL_M2:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SHARPNESS_LVL_M2 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSharpnessM2;
        }
        break;
        case CAMACQ_EXT_ADJUST_SHARPNESS_LVL_M1:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SHARPNESS_LVL_M1 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSharpnessM1;
        }
        break;
        case CAMACQ_EXT_ADJUST_SHARPNESS_LVL_DEFAULT:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SHARPNESS_LVL_DEFAULT ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSharpnessDefault;
        }
        break;
        case CAMACQ_EXT_ADJUST_SHARPNESS_LVL_P1:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SHARPNESS_LVL_P1 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSharpnessP1;
        }
        break;
        case CAMACQ_EXT_ADJUST_SHARPNESS_LVL_P2:
        {
            CamacqTraceDbg(" CAMACQ_EXT_ADJUST_SHARPNESS_LVL_P2 ");
            pvRegs = this->m_pstSensorRegs->pvAdjustSharpnessP2;
        }
        break;
        
        default:
        {
            CamacqTraceDbg( " :invalid psival[%d] ", *piVal );
            pvRegs = NULL;
            iRet = -EINVAL;
        }
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }

    
    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtAntiBanding( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;
    CamacqTraceIN();
    CamacqTraceDbg( " :piVal[%d] ", *piVal );

    switch(*piVal)
    {
        case V4L2_CID_POWER_LINE_FREQUENCY_DISABLED:
        {
            CamacqTraceDbg(" V4L2_CID_POWER_LINE_FREQUENCY_DISABLED ");
            pvRegs = this->m_pstSensorRegs->pvFlickerDisabled;
        }
        break;
        case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
        {
            CamacqTraceDbg(" V4L2_CID_POWER_LINE_FREQUENCY_50HZ ");
            pvRegs = this->m_pstSensorRegs->pvFlicker50Hz;
        }
        break;
        case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
        {
            CamacqTraceDbg(" V4L2_CID_POWER_LINE_FREQUENCY_60HZ ");
            pvRegs = this->m_pstSensorRegs->pvFlicker60Hz;
        }
        break;
        case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
        {
            CamacqTraceDbg(" V4L2_CID_POWER_LINE_FREQUENCY_AUTO ");
            pvRegs = this->m_pstSensorRegs->pvFlickerAuto;
        }
        break;
        
        default:
        {
            CamacqTraceErr( " :invalid psival[%d] ", *piVal );
            pvRegs = NULL;
            iRet = -EINVAL;
        }
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }

    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtCameraPrivateControl( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    const void* pvRegs = NULL;
    CamacqTraceIN();
    CamacqTraceDbg( " :piVal[%d] ", *piVal );

    if( eMode == CAMACQ_EXT_MODE_GET )
    {
    
    }
    else if( eMode == CAMACQ_EXT_MODE_SET )
    {
        switch( *piVal )
        {
            case V4L2_PRIVATE_CTRL_RETURN_PREVIEW:
            {
                CamacqTraceDbg( "V4L2_PRIVATE_CTRL_RETURN_PREVIEW" );    
                pvRegs = this->m_pstSensorRegs->pvPrivCtrlReturnPreview;
            }
            break;
            default:
            {
                CamacqTraceErr( " :invalid psival[%d] ", *piVal );
                pvRegs = NULL;
                iRet = -EINVAL;
            }
        }
    }
    else
    {
        CamacqTraceErr("mode error : %d", eMode);
        iRet = -EINVAL;
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }

    CamacqTraceOUT();
    return iRet;
}

S32 CamacqExtAutofocus( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0;
    _stCamacqSensor* pSensor = this->m_pstSensor;
    CamacqTraceIN( ":piVal[%d]", *piVal );

    if( eMode == CAMACQ_EXT_MODE_GET )
    {
        // Check Can Support AF
        if( pSensor->m_bCanAF == false )
        {
            *piVal = V4L2_ATFCS_STT_NOT_SUPPORT;
            goto CAMACQ_EXT_OUT;
        }

#if defined(__ISX006_SONY__) 
        {
            U16 usReadAddr = 0;        
            U8 ucAFState = 0, ucAFResultFirst = 0, ucAFResultSecond = 0;
            U8 rgucWriteRegs[3] = {0, };

            usReadAddr = 0x6D76;    // AF_STATE
            CamacqExtReadI2c( pSensor->m_pI2cClient, usReadAddr, 2, &ucAFState, 1);
            CamacqTraceDbg( " AF_STATE : 0x%2x", ucAFState);

            if( ucAFState == 0x08 )
            {
                // Clear AF_LOCK_STS
                rgucWriteRegs[0] = 0x00;
                rgucWriteRegs[1] = 0xFC;
                rgucWriteRegs[2] = 0x01;
                CamacqExtWriteI2c( pSensor->m_pI2cClient, rgucWriteRegs, 3 );
                CamacqExtDelay(1);

                usReadAddr = 0x6D3A;    // AF_RESULT_First
                CamacqExtReadI2c( pSensor->m_pI2cClient, usReadAddr, 2, &ucAFResultFirst, 1);

                usReadAddr = 0x6D52;    // AF_RESULT_Second 
                CamacqExtReadI2c( pSensor->m_pI2cClient, usReadAddr, 2, &ucAFResultSecond, 1);

                CamacqTraceDbg( " AF_RESULT_First : 0x%2x, AF_RESULT_Second : 0x%2x", ucAFResultFirst, ucAFResultSecond);
                if( ucAFResultFirst == 0x01 && ucAFResultSecond == 0x01 )
                {
                *piVal = V4L2_ATFCS_STT_FOCUSED;    // AF Success
                }
                else
                {
                *piVal = V4L2_ATFCS_STT_NOT_FOCUSED; // AF fail
                }


                // OFF AF
                CamacqExtWriteI2cLists( pSensor->m_pI2cClient, this->m_pstSensorRegs->pvOffAFRegs, pSensor->m_uiResType );
            }
            else
            {
            *piVal = V4L2_ATFCS_STT_SEARCHING;
            }            
        }
#elif defined(_S5K4ECGX_EVT1_MIPI_)
        {
            // 1st check status 
            U8	rgucWriteRegs_1[4] = {0x00,0x2C,0x70,0x00};
            U8	rgucWriteRegs_2[4] = {0x00, };
            U8  rgucReadData[2] = {0x00, };
            U16 usReadData = 0x00;
            U16 usReadKey = 0x0F12;
            U32 uiLoop = 0;

            if( this->m_pstSensor->m_uiFirmwareVer == 0x0011 )
            {
                rgucWriteRegs_2[0] = 0x00; rgucWriteRegs_2[1] = 0x2E; rgucWriteRegs_2[2] = 0x2E; rgucWriteRegs_2[3] = 0xEE; 
            }
            else
            {
                rgucWriteRegs_2[0] = 0x00; rgucWriteRegs_2[1] = 0x2E; rgucWriteRegs_2[2] = 0x2E; rgucWriteRegs_2[3] = 0x06; 
            }

            do 
            {
                if( pSensor->m_pstCamacqSettings->stV4l2CurScene == V4L2_CTRL_SCENE_NIGHT )
                {
                    CamacqExtDelay(250);    // 1 frame delay
                }
                else 
                {
                    CamacqExtDelay(100);    // 1 frame delay
                }   
                
                CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_1, 4 );
                CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_2, 4 );
                CamacqExtReadI2c( this->m_pstSensor->m_pI2cClient, usReadKey, 2, rgucReadData, 2 );

                usReadData = ( rgucReadData[0] << 8 ); usReadData |= rgucReadData[1];
                CamacqTraceDbg( " 1st check status, usReadData : 0x%x", usReadData );

                switch( usReadData & 0xFF )
                {
                    case 1:
                    CamacqTraceDbg( "1st CAM_AF_PROGRESS " );
                    *piVal = V4L2_ATFCS_STT_SEARCHING;
                    break;

                    case 2:
                    CamacqTraceDbg( "1st CAM_AF_SUCCESS " );
                    *piVal = V4L2_ATFCS_STT_FOCUSED;
                    break;

                    default:                    
                    CamacqTraceDbg("1st CAM_AF_FAIL ");
                    *piVal = V4L2_ATFCS_STT_NOT_FOCUSED;
                    break;
                }            
                uiLoop++;
                CamacqTraceDbg( " 1st uiLoop : %d", uiLoop );
            }while( (*piVal == V4L2_ATFCS_STT_SEARCHING) && (uiLoop < 100) );

            if( (*piVal == V4L2_ATFCS_STT_NOT_FOCUSED) || (uiLoop >= 100) )
            {
                 *piVal = V4L2_ATFCS_STT_NOT_FOCUSED;
                CamacqTraceErr("1st CAM_AF_FAIL, goto CAMACQ_EXT_OUT ");
                goto CAMACQ_EXT_OUT;
            }

            // 2st check status
            if( this->m_pstSensor->m_uiFirmwareVer == 0x0011 )
            {
                rgucWriteRegs_2[0] = 0x00; rgucWriteRegs_2[1] = 0x2E; rgucWriteRegs_2[2] = 0x22; rgucWriteRegs_2[3] = 0x07; 
            }
            else
            {
                rgucWriteRegs_2[0] = 0x00; rgucWriteRegs_2[1] = 0x2E; rgucWriteRegs_2[2] = 0x21; rgucWriteRegs_2[3] = 0x67; 
            }

            uiLoop = 0;
            do
            {
                if( pSensor->m_pstCamacqSettings->stV4l2CurScene == V4L2_CTRL_SCENE_NIGHT )
                {
                    CamacqExtDelay(250);    // 1 frame delay
                }
                else 
                {
                    CamacqExtDelay(100);    // 1 frame delay
                }   

                CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_1, 4 );
                CamacqExtWriteI2c( this->m_pstSensor->m_pI2cClient, rgucWriteRegs_2, 4 );
                CamacqExtReadI2c( this->m_pstSensor->m_pI2cClient, usReadKey, 2, rgucReadData, 2 );

                usReadData = 0;
                usReadData = ( rgucReadData[0] << 8 ); usReadData |= rgucReadData[1];
                CamacqTraceDbg( " 2st check status, usReadData : 0x%x", usReadData );

                switch( usReadData & 0xFF )
                {
                    case 0:
                    CamacqTraceDbg( "2st CAM_AF_SUCCESS " );
                    *piVal = V4L2_ATFCS_STT_FOCUSED;
                    break;

                    default:                    
                    CamacqTraceDbg("2st CAM_AF_PROGRESS ");
                    *piVal = V4L2_ATFCS_STT_SEARCHING;
                    break;
                }    

                uiLoop++;
                CamacqTraceDbg( " 1st uiLoop : %d", uiLoop );
            } while( (*piVal == V4L2_ATFCS_STT_SEARCHING) && (uiLoop < 100) );

            if( uiLoop >= 100 )
            {
                 *piVal = V4L2_ATFCS_STT_NOT_FOCUSED;
                CamacqTraceErr("2st CAM_AF_FAIL, goto CAMACQ_EXT_OUT ");
                goto CAMACQ_EXT_OUT;
            }
        }
#else
        // lsi ??? 
        {
            U16 usReadFlag = 0x0F12;
            U8  rgucReadRegs[2] = {0, };
            U16 usAFvalue = 0;

            CamacqExtWriteI2cLists( pSensor->m_pI2cClient, this->m_pstSensorRegs->pvCheckAFRegs, pSensor->m_uiResType );
            CamacqExtDelay(1);
            CamacqExtReadI2c( pSensor->m_pI2cClient, usReadFlag, 2, rgucReadRegs, 2 );

            usAFvalue = ( rgucReadRegs[1] << 8 );
            usAFvalue |= rgucReadRegs[0];

            CamacqTraceDbg( " :rgucReadRegs[0x%x%x], usAFvalue : %x ", rgucReadRegs[0], rgucReadRegs[1], usAFvalue) ;
            switch( usAFvalue )
            {
                case CAMACQ_EXT_AF_CONTINUE:
                {
                    CamacqTraceDbg(" CAMACQ_EXT_AF_CONTINUE ");
                    *piVal = V4L2_ATFCS_STT_SEARCHING;
                }
                break;
                case CAMACQ_EXT_AF_FAILD:
                {
                    CamacqTraceDbg(" CAMACQ_EXT_AF_FAILD ");
                    *piVal = V4L2_ATFCS_STT_NOT_FOCUSED;
                }
                break;
                case CAMACQ_EXT_AF_SUCCESS:
                {
                    CamacqTraceDbg(" CAMACQ_EXT_AF_SUCCESS ");
                    *piVal = V4L2_ATFCS_STT_FOCUSED;
                }
                break;
                default:
                {
                    *piVal = V4L2_ATFCS_STT_NOT_FOCUSED;
                    iRet = -EINVAL;
                    goto CAMACQ_EXT_OUT;
                }
                break;
            }
        }
#endif /* __ISX006_SONY__ */ 
    }
    else /* CAMACQ_EXT_MODE_SET */
    {
        switch( (enum v4l2_autofocus_ctrl)(*piVal) )
        {
            case V4L2_ATFCS_CTRL_SINGLE:
            {
#if defined(_S5K4ECGX_EVT1_MIPI_)
                CamacqExtWriteI2cLists( pSensor->m_pI2cClient, this->m_pstSensorRegs->pvSetAFRegs, pSensor->m_uiResType );
                if( pSensor->m_pstCamacqSettings->stV4l2CurScene == V4L2_CTRL_SCENE_NIGHT )
                {
                    CamacqExtDelay(250);    // 1 frame delay
                }
                else 
                {
                    CamacqExtDelay(100);    // 1 frame delay
                }    

#elif defined(__ISX006_SONY__)
                CamacqExtWriteI2cLists( pSensor->m_pI2cClient, this->m_pstSensorRegs->pvSetAF_NLUXRegs, pSensor->m_uiResType );
                CamacqExtDelay(1);
                CamacqExtWriteI2cLists( pSensor->m_pI2cClient, this->m_pstSensorRegs->pvSetAFRegs, pSensor->m_uiResType );

                CamacqExtDelay(200);
#endif /* _S5K4ECGX_EVT1_MIPI_ */
            }
            break;
            
            default:
            {
                CamacqTraceErr( " :invalid (enum v4l2_autofocus_ctrl)(*piVal) : [%d]", 
                    (enum v4l2_autofocus_ctrl)(*piVal) );
                iRet = -EINVAL;
            }
        }
        

    }

CAMACQ_EXT_OUT:
    CamacqTraceOUT( ":piVal[%d]", *piVal );
    return iRet;
}

S32 CamacqExtAutofocusMode( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, S32 *piVal )
{
    S32 iRet = 0, iDelay;
    const void* pvRegs = NULL;
    _stCamacqSensor* pSensor = this->m_pstSensor;
    CamacqTraceIN( ":piVal[%d]", *piVal );
    
    switch(*piVal)
    {
        case V4L2_ATFCS_MODE_AUTO:
        case V4L2_ATFCS_MODE_NORMAL:
        case V4L2_ATFCS_MODE_INFINITY:
        case V4L2_ATFCS_MODE_FIXED:
        case V4L2_ATFCS_MODE_EDOF:
        {
            CamacqTraceDbg(" V4L2_ATFCS_MODE_AUTO(NORMAL) ");
#if defined(_S5K4ECGX_EVT1_MIPI_)
            if( pSensor->m_pstCamacqSettings->stV4l2CurScene == V4L2_CTRL_SCENE_NIGHT )
            {
                iDelay = 250;
            }
            else 
            {
                iDelay = 100;
            }
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvSetAFNormalMode1, this->m_pstSensor->m_uiResType );
            CamacqExtDelay(iDelay);
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvSetAFNormalMode2, this->m_pstSensor->m_uiResType );
            CamacqExtDelay(iDelay);
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvSetAFNormalMode3, this->m_pstSensor->m_uiResType );
            CamacqExtDelay(iDelay);
#else
            pvRegs = this->m_pstSensorRegs->pvManualAFReg;
#endif /* _S5K4ECGX_EVT1_MIPI_ */

        }
        break;
        
        case V4L2_ATFCS_MODE_MACRO:
        {
            CamacqTraceDbg(" V4L2_ATFCS_MODE_MACRO ");
#if defined(_S5K4ECGX_EVT1_MIPI_)
            if( pSensor->m_pstCamacqSettings->stV4l2CurScene == V4L2_CTRL_SCENE_NIGHT )
            {
                iDelay = 250;
            }
            else 
            {
                iDelay = 100;
            }
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvSetAFMacroMode1, this->m_pstSensor->m_uiResType );
            CamacqExtDelay(iDelay);
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvSetAFMacroMode2, this->m_pstSensor->m_uiResType );
            CamacqExtDelay(iDelay);
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvSetAFMacroMode3, this->m_pstSensor->m_uiResType );
            CamacqExtDelay(iDelay);
#else
            pvRegs = this->m_pstSensorRegs->pvMacroAFReg;
#endif /* _S5K4ECGX_EVT1_MIPI_ */

            
        }
        break;

        case V4L2_ATFCS_MODE_RET_AUTO:
        {
            CamacqTraceDbg(" V4L2_ATFCS_MODE_RET_AUTO ");
            pvRegs = this->m_pstSensorRegs->pvReturnManualAFReg;
        }
        break;
        
        case V4L2_ATFCS_MODE_RET_MACRO:
        {
            CamacqTraceDbg(" V4L2_ATFCS_MODE_RET_MACRO ");
            pvRegs = this->m_pstSensorRegs->pvReturnMacroAFReg;
        }
        break;
       
        default:
        {
            CamacqTraceErr( " :invalid psival[%d] ", *piVal );
            pvRegs = NULL;
            iRet = -EINVAL;
        }
    }

    if( pvRegs )
    {
        CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, pvRegs, this->m_pstSensor->m_uiResType );
    }

    CamacqTraceOUT( ":piVal[%d]", *piVal );
    return iRet;
}


S32 CamacqExtWhitebalance( struct stCamacqExtAPIs_t* this, _eCamacqExtMode eMode, const S8* piVal )
{
    S32 iRet = 0, iIdx;
    CamacqTraceIN();

#if 0 /* TODO : will be changed .. */
    for( iIdx = 0; iIdx < CAMACQ_EXT_WB_MAX; iIdx++ )
    {
        if( !strncmp(piVal, (S8*)&g_V4l2CamacqWBModes[iIdx], MAX_WB_NAME_LENGTH) )
        break;
    }

    CamacqTraceDbg( ":iIdx[%d]", iIdx );
    switch( iIdx )
    {
        case CAMACQ_EXT_WB_AUTO:
        {
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvWBAutoRegs, this->m_pstSensor->uiResType );
        }
        break;
        case CAMACQ_EXT_WB_DAYLIGHT:
        {
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvWBDaylightRegs, this->m_pstSensor->uiResType );
        }
        break;
        case CAMACQ_EXT_WB_TUNGSTEN:
        {
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvWBDaylightRegs, this->m_pstSensor->uiResType );
        }
        break;
        case CAMACQ_EXT_WB_FLUORESCENT:
        {
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvWBFluorescentRegs, this->m_pstSensor->uiResType );
        }
        break;
        case CAMACQ_EXT_WB_HORIZON:
        {
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvWBFluorescentRegs, this->m_pstSensor->uiResType );
        }
        break;
        case CAMACQ_EXT_WB_OFF:
        {
            CamacqExtWriteI2cLists( this->m_pstSensor->m_pI2cClient, this->m_pstSensorRegs->pvWBAutoRegs, this->m_pstSensor->uiResType );
        }
        break;
        default:
        {
            CamacqTraceErr( " :invalid value " );
            iRet = -EINVAL;
        }
        break;
    }
#endif

    CamacqTraceOUT("OUT");
    return iRet;
}



/* Ext Common API */

/* Register control API */
/**
EX) 6AA sensor read // CAMACQ_SUB_REG_DAT_SZ is 4

// read 0x7000 01DA
U8 read_address[][CAMACQ_SUB_REG_DAT_SZ]= {
0xFCFCD000,
0x002C7000,
0x002E01DA };

U16 read_flag = 0x0F12;
U8 read_data[2] = { 0, };

camacq_ext_i2c_write_reglists( pclient, read_address, CAMACQ_SENSOW_LOW );
canacq_ext_i2c_read( pclient, read_flag, read_data, 2, CAMACQ_SENSOW_LOW );

S32 camacq_ext_i2c_write( struct i2c_client* pclient, U8 * pucvalue, U8 ucnb_write )

**/
S32 CamacqExtReadI2c( struct i2c_client *pClient, U16 usAddr, U8 ucAddrSize, U8 * pucBuf, U8 ucReadSize )
{
    S32 iRet = 0;
    struct i2c_msg stMsgs[2];
    U8 ucrgAddrBuf[CAMACQ_MAIN_EXT_RD_SZ];

    // CamacqTraceDbg_v("IN");

    if( !(pClient->adapter) )
    {
        CamacqTraceErr( ":client failed" );
        iRet= -ENODEV;
        goto CAMACQ_EXT_OUT;
    }

    if( ucAddrSize == 1 )
    {
        ucrgAddrBuf[0] = (U8)(usAddr);
    }
    else if( ucAddrSize == 2 )
    {
      //  CamacqTraceErr( " AddrSize ucAddrSize[%d]", ucAddrSize );
       if(CAMACQ_MAIN_EXT_RD_SZ >=2)
       	{
        ucrgAddrBuf[0] = (U8)((usAddr&(0xff00))>>8);
       // ucrgAddrBuf[1] = (U8)(usAddr);
        ucrgAddrBuf[1] = (U8)(usAddr&(0x00ff));//BYKIM_PREVENT		
    }
    else
    {
        CamacqTraceErr( ":invalid AddrSize ucAddrSize[%d]", ucAddrSize );
            iRet= -ENODEV;
            goto CAMACQ_EXT_OUT;
        }
    }
    else
    {
        CamacqTraceErr( ":invalid AddrSize ucAddrSize[%d]", ucAddrSize );
        iRet= -ENODEV;
        goto CAMACQ_EXT_OUT;
    }

    stMsgs[0].addr = pClient->addr;
    stMsgs[0].flags = 0;
    stMsgs[0].len = ucAddrSize;
    stMsgs[0].buf = ucrgAddrBuf;

    stMsgs[1].addr = pClient->addr;
    stMsgs[1].flags = I2C_M_RD; /* read flag*/
    stMsgs[1].len = ucReadSize;
    stMsgs[1].buf = pucBuf;

    iRet = i2c_transfer( pClient->adapter, stMsgs, CAMACQ_EXT_MODE_R );
    if( iRet < 0 )
    {
        CamacqTraceErr( ":transfer failed" );
        iRet = -ENODEV;
    }
#if 0/* for debug */
    CamacqTraceDbg("  ucrgAddrBuf[0] : 0x%x ",ucrgAddrBuf[0]);
    CamacqTraceDbg("  ucrgAddrBuf[1]  : 0x%x ", ucrgAddrBuf[1] );
    CamacqTraceDbg("  pucBuf : 0x%x ", pucBuf[0]);
#endif /* for debug */

	

CAMACQ_EXT_OUT:
    // CamacqTraceDbg_v();
    return iRet;
}

S32 CamacqExtWriteI2c( struct i2c_client *pClient, U8 * pucValue, U8 ucSize )
{
    struct i2c_msg stMsg;
    S32 iRet = 0;
    // CamacqTraceDbg_v("IN");

    if( !(pClient->adapter) )
    {
        CamacqTraceErr( ":client failed" );
        iRet = -ENODEV;
        goto CAMACQ_EXT_OUT;
    }

    stMsg.addr = pClient->addr;

#if defined(TOTORO)
    stMsg.flags = pClient->flags;
#else
    stMsg.flags = I2C_M_TEN;    // Marvell ??
#endif

    stMsg.buf = pucValue;   
    stMsg.len = ucSize; 

#if 0 /* for debug */
    CamacqTraceDbg("Length[%d] : 0x%02X%02X%02X%02X\n ", stMsg.len,stMsg.buf[0] ,stMsg.buf[1] ,stMsg.buf[2] ,stMsg.buf[3]);
#endif /* for debug */

    iRet =i2c_transfer( pClient->adapter, &stMsg, CAMACQ_EXT_MODE_W );
    if(iRet < 0)
    {
        CamacqTraceErr( ":transfer failed" );
        iRet = -ENODEV;
        goto CAMACQ_EXT_OUT;
    }

CAMACQ_EXT_OUT:
    // CamacqTraceDbg_v("OUT");

    return iRet;
}

S32 CamacqExtWriteI2cLists( struct i2c_client *pClient, const void *pvArg, int iResType )
{
    S32 iRet = 0;
    S32 iNext = 0;
    U8	rgucWriteRegs[4] = {0, };
    U16	usDealy;

    CamacqTraceDbg_v("IN");

#if defined(__ISX006_SONY__) ||defined(__ISX005_SONY__)
    // check Sony sensor, temp code
  //  if( !strcmp( pClient->name, "isx006" ) )
    {
        return CamacqExtWriteI2cLists_Sony( pClient, pvArg, iResType );
    }
#endif /* __ISX006_SONY__ */
    	
	if( iResType == CAMACQ_SENSOR_MAIN )
    {
#if (CAMACQ_MAIN_BURST_MODE) 
        U8  rgucNextRegs[4] = {0, };
        U8  ucBurstCnt = 0;
        U8  rgucBurstRegs[CAMACQ_MAIN_BURST_MAX] = {0, };   
#endif /* CAMACQ_BURST_MODE */

///// init reglists variable. ///////////////////////////////////////////////////////
#if (CAMACQ_MAIN_FS_MODE)
#if (CAMACQ_MAIN_INT_MODE)
        U8* pvRegLists = CamacqExtReadFs( pvArg, CAMACQ_EXT_LEN_4BYTE_INT, iResType );
#else
        U8* pvRegLists = CamacqExtReadFs( pvArg, CAMACQ_EXT_LEN_4BYTE_ARY, iResType );
#endif /* CAMACQ_MAIN_INT_MODE */
#else // CAMACQ_MAIN_FS_MODE
#if (CAMACQ_MAIN_2BYTE_SENSOR)

#if (CAMACQ_MAIN_REG_DAT_SZ==1)
        U8 (*pvRegLists)[CAMACQ_MAIN_REG_SET_SZ] = (U8(*)[CAMACQ_MAIN_REG_SET_SZ])(pvArg);
#elif (CAMACQ_MAIN_REG_DAT_SZ==2)
        U16* pvRegLists = (U16*)(pvArg);
#endif

#else // CAMACQ_MAIN_2BYTE_SENSOR

#if (CAMACQ_MAIN_REG_DAT_SZ==2)
        U16 (*pvRegLists)[CAMACQ_MAIN_REG_SET_SZ] = (U16(*)[CAMACQ_MAIN_REG_SET_SZ])(pvArg);
#elif (CAMACQ_MAIN_REG_DAT_SZ==1) 
        U8 (*pvRegLists)[CAMACQ_MAIN_REG_SET_SZ] = (U8(*)[CAMACQ_MAIN_REG_SET_SZ])(pvArg);
#elif (CAMACQ_MAIN_REG_DAT_SZ==4)
        U32* pvRegLists = (U32*)(pvArg);
#endif 
#endif /* CAMACQ_MAIN_2BYTE_SENSOR */ 
#endif /* CAMACQ_MAIN_FS_MODE */
//// init reglists valiable. ///////////////////////////////////////////////////////

        if( pvRegLists == NULL )
            return -1;

        // start!!
        CAMACQ_MAIN_EXT_REG_GET_DATA( rgucWriteRegs, pvRegLists, iNext )
        while( !CAMACQ_MAIN_EXT_REG_IS_BTM_OF_DATA(rgucWriteRegs) )
        {
            if( CAMACQ_MAIN_EXT_REG_IS_DELAY(rgucWriteRegs) )
            {
#if (CAMACQ_MAIN_2BYTE_SENSOR)
                usDealy = rgucWriteRegs[1];
                CamacqTraceDbg(": 0x%02x%02x" , rgucWriteRegs[0], rgucWriteRegs[1]);
#else
                usDealy = (rgucWriteRegs[2]<<8 | rgucWriteRegs[3]);
                CamacqTraceDbg(": 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
#endif /* CAMACQ_MAIN_2BYTE_SENSOR */
                CamacqExtDelay(usDealy);
                iRet = 0;
            }
            else
            {
#if (CAMACQ_MAIN_BURST_MODE)
                CAMACQ_MAIN_EXT_REG_GET_DATA(rgucNextRegs, pvRegLists, iNext+1)
                if( CAMACQ_MAIN_EXT_REG_IS_CNTS(rgucWriteRegs) && CAMACQ_MAIN_EXT_REG_IS_CNTS(rgucNextRegs) )
                {
                    memset(rgucBurstRegs, 0x00, CAMACQ_MAIN_BURST_MAX);
                    // copy first 0x0f12 data
                    ucBurstCnt = 0;
                    memcpy(rgucBurstRegs + ucBurstCnt, rgucWriteRegs, sizeof(rgucWriteRegs));
                    ucBurstCnt+=sizeof(rgucWriteRegs);

                    iNext++;
                    CAMACQ_MAIN_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext) 
                    do
                    {
                        memcpy(rgucBurstRegs+ucBurstCnt, rgucWriteRegs+2, 2);
                        ucBurstCnt+=2;
                        iNext++;
                        CAMACQ_MAIN_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext) 
                        if(ucBurstCnt == CAMACQ_MAIN_BURST_MAX)
                        break;

                    } while( CAMACQ_MAIN_EXT_REG_IS_CNTS(rgucWriteRegs) );

#if (CAMACQ_MAIN_BURST_I2C_TRACE)
                    int i;
                    for( i=2 ; i<ucBurstCnt; i+=2) {
                    camacq_trace_dbg(": 0x%02X%02X%02X%02X" , rgucBurstRegs[0], rgucBurstRegs[1], rgucBurstRegs[i] , rgucBurstRegs[i+1]);
                    }
                    camacq_trace_dbg(":burst count[%d]", ucBurstCnt);
#endif /* CAMACQ_MAIN_BURST_I2C_TRACE */
					
                    iRet = CamacqExtWriteI2c( pClient, rgucBurstRegs, ucBurstCnt );
                    if(iRet < 0)
                    {
                        CamacqTraceErr( ":write failed" );
                        break;
                    }
                    continue;
                }
                else
#endif /* CAMACQ_MAIN_BURST_MODE */
                {
#if (CAMACQ_MAIN_2BYTE_SENSOR)         
                    // CamacqTraceDbg_v(": 0x%02x%02x" , rgucWriteRegs[0], rgucWriteRegs[1]);
#else
                    // CamacqTraceDbg(": 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
#endif

                    iRet = CamacqExtWriteI2c( pClient, rgucWriteRegs, CAMACQ_MAIN_REG_SET_SZ*CAMACQ_MAIN_REG_DAT_SZ );
                    if(iRet < 0)
                    {
                        CamacqTraceErr( ":write failed" );
                        break;
                    }
                } // else
            } // else
	                
            iNext++;
            CAMACQ_MAIN_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext)
        } // while

#if (CAMACQ_MAIN_FS_MODE)
        CamacqFree( pvRegLists );
#endif /* CAMACQ_MAIN_FS_MODE */

    }
#if (CAMACQ_SENSOR_MAX==2)
    else // irestype == CAMACQ_SENSOR_LOW
    {
#if (CAMACQ_SUB_BURST_MODE) 
        U8  rgucNextRegs[4] = {0, };
        U8  ucBurstCnt = 0;
        U8  rgucBurstRegs[CAMACQ_SUB_BURST_MAX] = {0, };   
#endif /* CAMACQ_BURST_MODE */

//// init reglists variable. ///////////////////////////////////////////////////////
#if (CAMACQ_SUB_FS_MODE)
#if (CAMACQ_SUB_INT_MODE)
        U8* pvRegLists = CamacqExtReadFs( pvArg, CAMACQ_EXT_LEN_4BYTE_INT, iResType );
#else
        U8* pvRegLists = CamacqExtReadFs( pvArg, CAMACQ_EXT_LEN_4BYTE_ARY, iResType );
#endif /* CAMACQ_SUB_INT_MODE */
#else // CAMACQ_SUB_FS_MODE
#if (CAMACQ_SUB_2BYTE_SENSOR)

#if (CAMACQ_SUB_REG_DAT_SZ==1)
        U8 (*pvRegLists)[CAMACQ_SUB_REG_SET_SZ] = (U8(*)[CAMACQ_SUB_REG_SET_SZ])(pvArg);
#elif (CAMACQ_SUB_REG_DAT_SZ==2)
        U16* pvRegLists = (U16*)(pvArg);
#endif

#else // CAMACQ_SUB_2BYTE_SENSOR

#if (CAMACQ_SUB_REG_DAT_SZ==2)
        U16 (*pvRegLists)[CAMACQ_SUB_REG_SET_SZ] = (U16(*)[CAMACQ_SUB_REG_SET_SZ])(pvArg);
#elif (CAMACQ_SUB_REG_DAT_SZ==1) 
        U8 (*pvRegLists)[CAMACQ_SUB_REG_SET_SZ] = (U8(*)[CAMACQ_SUB_REG_SET_SZ])(pvArg);
#elif (CAMACQ_SUB_REG_DAT_SZ==4)
        U32* pvRegLists = (U32*)(pvArg);
#endif 
#endif /* CAMACQ_SUB_2BYTE_SENSOR */ 
#endif /* CAMACQ_SUB_FS_MODE */
//// init reglists valiable. ///////////////////////////////////////////////////////

        if( pvRegLists == NULL )
            return -1;

        // start!!
        CAMACQ_SUB_EXT_REG_GET_DATA( rgucWriteRegs, pvRegLists, iNext )
        while( !CAMACQ_MAIN_EXT_REG_IS_BTM_OF_DATA(rgucWriteRegs) )
        {
            if( CAMACQ_MAIN_EXT_REG_IS_DELAY(rgucWriteRegs) )
            {
#if (CAMACQ_SUB_2BYTE_SENSOR)
                usDealy = rgucWriteRegs[1];
                CamacqTraceDbg_v(": 0x%02x%02x" , rgucWriteRegs[0], rgucWriteRegs[1]);
#else
                usDealy = (rgucWriteRegs[2]<<8 | rgucWriteRegs[3]);
                CamacqTraceDbg(": 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
#endif /* CAMACQ_SUB_2BYTE_SENSOR */
                CamacqExtDelay(usDealy);
                iRet = 0;
            }
            else
            {
#if (CAMACQ_SUB_BURST_MODE)
                CAMACQ_SUB_EXT_REG_GET_DATA(rgucNextRegs, pvRegLists, iNext+1)
                if( CAMACQ_SUB_EXT_REG_IS_CNTS(rgucWriteRegs) && CAMACQ_SUB_EXT_REG_IS_CNTS(rgucNextRegs) )
                {
                    memset(rgucBurstRegs, 0x00, CAMACQ_SUB_BURST_MAX);
                    // copy first 0x0f12 data
                    ucBurstCnt = 0;
                    memcpy(rgucBurstRegs + ucBurstCnt, rgucWriteRegs, sizeof(rgucWriteRegs));
                    ucBurstCnt+=sizeof(rgucWriteRegs);

                    iNext++;
                    CAMACQ_SUB_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext) 
                    do
                    {
                        memcpy(rgucBurstRegs+ucBurstCnt, rgucWriteRegs+2, 2);
                        ucBurstCnt+=2;
                        iNext++;
                        CAMACQ_SUB_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext) 
                        if(ucBurstCnt == CAMACQ_SUB_BURST_MAX)
                        break;

                    } while( CAMACQ_SUB_EXT_REG_IS_CNTS(rgucWriteRegs) );

#if (CAMACQ_SUB_BURST_I2C_TRACE)
                    int i;
                    for( i=2 ; i<ucBurstCnt; i+=2) {
                    camacq_trace_dbg(": 0x%02X%02X%02X%02X" , rgucBurstRegs[0], rgucBurstRegs[1], rgucBurstRegs[i] , rgucBurstRegs[i+1]);
                    }
                    camacq_trace_dbg(":burst count[%d]", ucBurstCnt);
#endif /* CAMACQ_SUB_BURST_I2C_TRACE */
					
                    iRet = CamacqExtWriteI2c( pClient, rgucBurstRegs, ucBurstCnt );
                    if(iRet < 0)
                    {
                        CamacqTraceErr( ":write failed" );
                        break;
                    }
                    continue;
                }
                else
#endif /* CAMACQ_SUB_BURST_MODE */
                {
#if (CAMACQ_SUB_2BYTE_SENSOR)         
                    CamacqTraceDbg_v(": 0x%02x%02x" , rgucWriteRegs[0], rgucWriteRegs[1]);
#else
                    CamacqTraceDbg_v(": 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
#endif

                    iRet = CamacqExtWriteI2c( pClient, rgucWriteRegs, CAMACQ_SUB_REG_SET_SZ*CAMACQ_SUB_REG_DAT_SZ );
                    if(iRet < 0)
                    {
                        CamacqTraceErr( ":write failed" );
                        break;
                    }
                } // else
            } // else
	                
            iNext++;
            CAMACQ_SUB_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext)
        } // while

#if (CAMACQ_SUB_FS_MODE)
        CamacqFree( pvRegLists );
#endif /* CAMACQ_SUB_FS_MODE */

    }
#endif /* CAMACQ_SENSOR_MAX==2 */
	
    CamacqTraceDbg( ":size[%d]", iNext );
    CamacqTraceDbg_v("OUT");

    return iRet;
}

#if defined(__ISX006_SONY__)||defined(__ISX005_SONY__)
S32 CamacqExtWriteI2cLists_Sony( struct i2c_client *pClient, const void *pvArg, int iResType )
{
    S32 iRet = 0;
    U32 uiCnt = 0;
    U8 rgucWriteRegs[4] = {0, };
    _stSonyData* pstRegLists = 0;
#if ( CAMACQ_MAIN_FS_MODE || CAMACQ_SUB_FS_MODE)
    U8* pcRegLists = 0;
    U8 ucAddrLen = 0;
    S32 iIdx = 0;
#endif
 //   CamacqTraceDbg_v("IN");

//    CamacqTraceDbg( ":IN addr : %x ", pClient->addr );

    if( iResType == CAMACQ_SENSOR_MAIN )
    {
#if (CAMACQ_MAIN_FS_MODE)
        pcRegLists = CamacqExtReadFs( (const S8*)pvArg, CAMACQ_EXT_LEN_SONY, iResType );
#else
        pstRegLists = (_stSonyData*)pvArg;
#endif
    }
#if (CAMACQ_SENSOR_MAX==2)
    else if( iResType == CAMACQ_SENSOR_SUB )
    {
#if (CAMACQ_SUB_FS_MODE)
        pcRegLists = CamacqExtReadFs( (const S8*)pvArg, CAMACQ_EXT_LEN_SONY, iResType );
#else
        pstRegLists = (_stSonyData*)pvArg;
#endif
    }
#endif /* CAMACQ_SENSOR_MAX==2 */

// debug
#if 0
    iIdx = 0;
    ucAddrLen = pcRegLists[ iIdx ];
    while( ucAddrLen != 0xFF )
    {
        CamacqTraceDbg( " : 0x%2x", pcRegLists[ iIdx++ ] );
        ucAddrLen = pcRegLists[iIdx];
    }
#endif 
       
#if (CAMACQ_MAIN_FS_MODE)
    {   
        iIdx = 0;
        ucAddrLen = pcRegLists[ iIdx++ ];
        while( ucAddrLen != 0xFF )
        {
            if( ucAddrLen == 0x02 )
            {
                rgucWriteRegs[0] = pcRegLists[iIdx++];
                rgucWriteRegs[1] = pcRegLists[iIdx++];
                rgucWriteRegs[3] = pcRegLists[iIdx++];
                rgucWriteRegs[2] = pcRegLists[iIdx++];
            }
            else if( ucAddrLen == 0x01 || ucAddrLen == 0x03 )
            {
                rgucWriteRegs[0] = pcRegLists[iIdx++];
                rgucWriteRegs[1] = pcRegLists[iIdx++];
                rgucWriteRegs[2] = pcRegLists[iIdx++];
                rgucWriteRegs[3] = 0x00;
            }
            else
            {
                CamacqTraceErr("Unexpected value!!iIdx = %d ucAddrLen = %d",iIdx,ucAddrLen);
                return iRet;   
            }

            if( ucAddrLen != 0x03 )
            {
            iRet = CamacqExtWriteI2c( pClient, rgucWriteRegs, 2+ucAddrLen );
             //  CamacqTraceDbg(":2 Length[%d] : 0x%02X%02X%02X%02X" , ucAddrLen, rgucWriteRegs[0] , 
             //                                rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
                if(iRet < 0)
                {
                    CamacqTraceErr( ":write failed" );
                    return iRet;
                }
            }
            else
            {
                CamacqExtDelay(rgucWriteRegs[2]);
                CamacqTraceDbg(" setfile delay : %d", rgucWriteRegs[2]);   
            }

            ucAddrLen = pcRegLists[ iIdx++ ];
        }
    }
    CamacqFree( pcRegLists );
#else

    if( pstRegLists == NULL )  //BYKIM_PREVENT
            return -1;

    if( pstRegLists[uiCnt].ucLen == 0x02 )
    {
        rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
        rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
#if 0         
        if( pClient->addr == 0x3D || pClient->addr == 0x3C )
        {
            rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);   
            rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
        }
        else if( pClient->addr == 0x1A )
        {   
            rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
            rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);   
        }
        else 
        {
            CamacqTraceErr( " Invalid addr!!!!!!!!!! " );
        }
#else
            rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
            rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);   
#endif
    }
    else if( pstRegLists[uiCnt].ucLen == 0x01 || pstRegLists[uiCnt].ucLen == 0x03 )
    {
        rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
        rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
        rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
        rgucWriteRegs[3] = 0x00;
    }
    else if( pstRegLists[uiCnt].ucLen == 0xFF )
    {
        rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
        rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
        rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
        rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);
    }
    else
    {
        CamacqTraceErr("Unexpected value!!");
        return iRet;
    }

    while( rgucWriteRegs[0] != 0xFF || rgucWriteRegs[1] != 0xFF || rgucWriteRegs[2] != 0xFF )
    {
        if( pstRegLists[uiCnt].ucLen != 0x03 )
        {
        iRet = CamacqExtWriteI2c( pClient, rgucWriteRegs, 2+pstRegLists[uiCnt].ucLen );
            if(iRet < 0)
            {
                CamacqTraceErr( ":write failed" );
                break;
            }
        
        // CamacqTraceDbg_v(":Length[%d] : 0x%02X%02X%02X%02X" , pstRegLists[uiCnt].ucLen, rgucWriteRegs[0] , 
        //                                rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
      //  CamacqTraceDbg(":Length[%d] : 0x%02X%02X%02X%02X" , pstRegLists[uiCnt].ucLen, rgucWriteRegs[0] , 
       //                                    rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
        }
        else // 0x03 is delay
        {
            CamacqExtDelay(rgucWriteRegs[2]);
            CamacqTraceDbg(" setfile delay : %d", rgucWriteRegs[2]);
        }        
        
        uiCnt++;
        
        if( pstRegLists[uiCnt].ucLen == 0x02 )
        {
            rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
            rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
  #if 0           
            if( pClient->addr == 0x3D || pClient->addr == 0x3C )
            {
                rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);   
                rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
            }
            else if( pClient->addr == 0x1A )
            {   
                rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
                rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);   
            }
            else 
            {
                CamacqTraceErr( " Invalid addr!!!!!!!!!! " );
            }
   #else
                rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
                rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);   
   #endif
        }
        else if(pstRegLists[uiCnt].ucLen == 0x01 || pstRegLists[uiCnt].ucLen == 0x03)
        {
            rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
            rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
            rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
            rgucWriteRegs[3] = 0x00;
        }
        else if( pstRegLists[uiCnt].ucLen == 0xFF )
        {
            rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
            rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
            rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
            rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);
        }
        else
        {
            CamacqTraceErr("Unexpected value!!");
            return iRet;
        }
    }
#endif /* CAMACQ_MAIN_FS_MODE */
   
    CamacqTraceDbg_v("OUT");
    return iRet;
}
#endif /* __ISX006_SONY__ */




S32 CamacqExtDirectlyWriteI2cLists( struct i2c_client *pClient, const void *pvArg, int iResType )
{
    S32 iRet = 0;
    S32 iNext = 0;
    U8	rgucWriteRegs[4] = {0, };
    U16	usDealy;

    CamacqTraceDbg("CamacqExtDirectlyWriteI2cLists IN");

#if defined(__ISX006_SONY__) ||defined(__ISX005_SONY__)
    {
        return CamacqExtDirectlyWriteI2cLists_Sony( pClient, pvArg, iResType );
    }
#endif /* __ISX006_SONY__ */
    	
	if( iResType == CAMACQ_SENSOR_MAIN )
    {
#if (CAMACQ_MAIN_BURST_MODE) 
        U8  rgucNextRegs[4] = {0, };
        U8  ucBurstCnt = 0;
        U8  rgucBurstRegs[CAMACQ_MAIN_BURST_MAX] = {0, };   
#endif /* CAMACQ_BURST_MODE */

///// init reglists variable. ///////////////////////////////////////////////////////
#if (CAMACQ_MAIN_2BYTE_SENSOR)

#if (CAMACQ_MAIN_REG_DAT_SZ==1)
        U8 (*pvRegLists)[CAMACQ_MAIN_REG_SET_SZ] = (U8(*)[CAMACQ_MAIN_REG_SET_SZ])(pvArg);
#elif (CAMACQ_MAIN_REG_DAT_SZ==2)
        U16* pvRegLists = (U16*)(pvArg);
#endif

#else // CAMACQ_MAIN_2BYTE_SENSOR

#if (CAMACQ_MAIN_REG_DAT_SZ==2)
        U16 (*pvRegLists)[CAMACQ_MAIN_REG_SET_SZ] = (U16(*)[CAMACQ_MAIN_REG_SET_SZ])(pvArg);
#elif (CAMACQ_MAIN_REG_DAT_SZ==1) 
        U8 (*pvRegLists)[CAMACQ_MAIN_REG_SET_SZ] = (U8(*)[CAMACQ_MAIN_REG_SET_SZ])(pvArg);
#elif (CAMACQ_MAIN_REG_DAT_SZ==4)
        U32* pvRegLists = (U32*)(pvArg);
#endif 
#endif /* CAMACQ_MAIN_2BYTE_SENSOR */ 
//// init reglists valiable. ///////////////////////////////////////////////////////

        if( pvRegLists == NULL )
            return -1;

        // start!!
        CAMACQ_MAIN_EXT_REG_GET_DATA( rgucWriteRegs, pvRegLists, iNext )
        while( !CAMACQ_MAIN_EXT_REG_IS_BTM_OF_DATA(rgucWriteRegs) )
        {
            if( CAMACQ_MAIN_EXT_REG_IS_DELAY(rgucWriteRegs) )
            {
#if (CAMACQ_MAIN_2BYTE_SENSOR)
                usDealy = rgucWriteRegs[1];
                CamacqTraceDbg(": 0x%02x%02x" , rgucWriteRegs[0], rgucWriteRegs[1]);
#else
                usDealy = (rgucWriteRegs[2]<<8 | rgucWriteRegs[3]);
                CamacqTraceDbg(": 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
#endif /* CAMACQ_MAIN_2BYTE_SENSOR */
                CamacqExtDelay(usDealy);
                iRet = 0;
            }
            else
            {
#if (CAMACQ_MAIN_BURST_MODE)
                CAMACQ_MAIN_EXT_REG_GET_DATA(rgucNextRegs, pvRegLists, iNext+1)
                if( CAMACQ_MAIN_EXT_REG_IS_CNTS(rgucWriteRegs) && CAMACQ_MAIN_EXT_REG_IS_CNTS(rgucNextRegs) )
                {
                    memset(rgucBurstRegs, 0x00, CAMACQ_MAIN_BURST_MAX);
                    // copy first 0x0f12 data
                    ucBurstCnt = 0;
                    memcpy(rgucBurstRegs + ucBurstCnt, rgucWriteRegs, sizeof(rgucWriteRegs));
                    ucBurstCnt+=sizeof(rgucWriteRegs);

                    iNext++;
                    CAMACQ_MAIN_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext) 
                    do
                    {
                        memcpy(rgucBurstRegs+ucBurstCnt, rgucWriteRegs+2, 2);
                        ucBurstCnt+=2;
                        iNext++;
                        CAMACQ_MAIN_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext) 
                        if(ucBurstCnt == CAMACQ_MAIN_BURST_MAX)
                        break;

                    } while( CAMACQ_MAIN_EXT_REG_IS_CNTS(rgucWriteRegs) );

#if (CAMACQ_MAIN_BURST_I2C_TRACE)
                    int i;
                    for( i=2 ; i<ucBurstCnt; i+=2) {
                    camacq_trace_dbg(": 0x%02X%02X%02X%02X" , rgucBurstRegs[0], rgucBurstRegs[1], rgucBurstRegs[i] , rgucBurstRegs[i+1]);
                    }
                    camacq_trace_dbg(":burst count[%d]", ucBurstCnt);
#endif /* CAMACQ_MAIN_BURST_I2C_TRACE */
					
                    iRet = CamacqExtWriteI2c( pClient, rgucBurstRegs, ucBurstCnt );
                    if(iRet < 0)
                    {
                        CamacqTraceErr( ":write failed" );
                        break;
                    }
                    continue;
                }
                else
#endif /* CAMACQ_MAIN_BURST_MODE */
                {
#if (CAMACQ_MAIN_2BYTE_SENSOR)         
                    // CamacqTraceDbg_v(": 0x%02x%02x" , rgucWriteRegs[0], rgucWriteRegs[1]);
#else
                    // CamacqTraceDbg(": 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
#endif

                    iRet = CamacqExtWriteI2c( pClient, rgucWriteRegs, CAMACQ_MAIN_REG_SET_SZ*CAMACQ_MAIN_REG_DAT_SZ );
                    if(iRet < 0)
                    {
                        CamacqTraceErr( ":write failed" );
                        break;
                    }
                } // else
            } // else
	                
            iNext++;
            CAMACQ_MAIN_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext)
        } // while
    }
#if (CAMACQ_SENSOR_MAX==2)
    else // irestype == CAMACQ_SENSOR_LOW
    {
#if (CAMACQ_SUB_BURST_MODE) 
        U8  rgucNextRegs[4] = {0, };
        U8  ucBurstCnt = 0;
        U8  rgucBurstRegs[CAMACQ_SUB_BURST_MAX] = {0, };   
#endif /* CAMACQ_BURST_MODE */

//// init reglists variable. ///////////////////////////////////////////////////////
#if (CAMACQ_SUB_2BYTE_SENSOR)

#if (CAMACQ_SUB_REG_DAT_SZ==1)
        U8 (*pvRegLists)[CAMACQ_SUB_REG_SET_SZ] = (U8(*)[CAMACQ_SUB_REG_SET_SZ])(pvArg);
#elif (CAMACQ_SUB_REG_DAT_SZ==2)
        U16* pvRegLists = (U16*)(pvArg);
#endif

#else // CAMACQ_SUB_2BYTE_SENSOR

#if (CAMACQ_SUB_REG_DAT_SZ==2)
        U16 (*pvRegLists)[CAMACQ_SUB_REG_SET_SZ] = (U16(*)[CAMACQ_SUB_REG_SET_SZ])(pvArg);
#elif (CAMACQ_SUB_REG_DAT_SZ==1) 
        U8 (*pvRegLists)[CAMACQ_SUB_REG_SET_SZ] = (U8(*)[CAMACQ_SUB_REG_SET_SZ])(pvArg);
#elif (CAMACQ_SUB_REG_DAT_SZ==4)
        U32* pvRegLists = (U32*)(pvArg);
#endif 
#endif /* CAMACQ_SUB_2BYTE_SENSOR */ 
//// init reglists valiable. ///////////////////////////////////////////////////////

        if( pvRegLists == NULL )
            return -1;

        // start!!
        CAMACQ_SUB_EXT_REG_GET_DATA( rgucWriteRegs, pvRegLists, iNext )
        while( !CAMACQ_MAIN_EXT_REG_IS_BTM_OF_DATA(rgucWriteRegs) )
        {
            if( CAMACQ_MAIN_EXT_REG_IS_DELAY(rgucWriteRegs) )
            {
#if (CAMACQ_SUB_2BYTE_SENSOR)
                usDealy = rgucWriteRegs[1];
                CamacqTraceDbg_v(": 0x%02x%02x" , rgucWriteRegs[0], rgucWriteRegs[1]);
#else
                usDealy = (rgucWriteRegs[2]<<8 | rgucWriteRegs[3]);
                CamacqTraceDbg(": 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
#endif /* CAMACQ_SUB_2BYTE_SENSOR */
                CamacqExtDelay(usDealy);
                iRet = 0;
            }
            else
            {
#if (CAMACQ_SUB_BURST_MODE)
                CAMACQ_SUB_EXT_REG_GET_DATA(rgucNextRegs, pvRegLists, iNext+1)
                if( CAMACQ_SUB_EXT_REG_IS_CNTS(rgucWriteRegs) && CAMACQ_SUB_EXT_REG_IS_CNTS(rgucNextRegs) )
                {
                    memset(rgucBurstRegs, 0x00, CAMACQ_SUB_BURST_MAX);
                    // copy first 0x0f12 data
                    ucBurstCnt = 0;
                    memcpy(rgucBurstRegs + ucBurstCnt, rgucWriteRegs, sizeof(rgucWriteRegs));
                    ucBurstCnt+=sizeof(rgucWriteRegs);

                    iNext++;
                    CAMACQ_SUB_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext) 
                    do
                    {
                        memcpy(rgucBurstRegs+ucBurstCnt, rgucWriteRegs+2, 2);
                        ucBurstCnt+=2;
                        iNext++;
                        CAMACQ_SUB_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext) 
                        if(ucBurstCnt == CAMACQ_SUB_BURST_MAX)
                        break;

                    } while( CAMACQ_SUB_EXT_REG_IS_CNTS(rgucWriteRegs) );

#if (CAMACQ_SUB_BURST_I2C_TRACE)
                    int i;
                    for( i=2 ; i<ucBurstCnt; i+=2) {
                    camacq_trace_dbg(": 0x%02X%02X%02X%02X" , rgucBurstRegs[0], rgucBurstRegs[1], rgucBurstRegs[i] , rgucBurstRegs[i+1]);
                    }
                    camacq_trace_dbg(":burst count[%d]", ucBurstCnt);
#endif /* CAMACQ_SUB_BURST_I2C_TRACE */
					
                    iRet = CamacqExtWriteI2c( pClient, rgucBurstRegs, ucBurstCnt );
                    if(iRet < 0)
                    {
                        CamacqTraceErr( ":write failed" );
                        break;
                    }
                    continue;
                }
                else
#endif /* CAMACQ_SUB_BURST_MODE */
                {
#if (CAMACQ_SUB_2BYTE_SENSOR)         
                    CamacqTraceDbg_v(": 0x%02x%02x" , rgucWriteRegs[0], rgucWriteRegs[1]);
#else
                    CamacqTraceDbg_v(": 0x%02X%02X%02X%02X" , rgucWriteRegs[0] , rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
#endif

                    iRet = CamacqExtWriteI2c( pClient, rgucWriteRegs, CAMACQ_SUB_REG_SET_SZ*CAMACQ_SUB_REG_DAT_SZ );
                    if(iRet < 0)
                    {
                        CamacqTraceErr( ":write failed" );
                        break;
                    }
                } // else
            } // else
	                
            iNext++;
            CAMACQ_SUB_EXT_REG_GET_DATA(rgucWriteRegs, pvRegLists, iNext)
        } // while


    }
#endif /* CAMACQ_SENSOR_MAX==2 */
	
    CamacqTraceDbg( ":size[%d]", iNext );
    CamacqTraceDbg_v("OUT");

    return iRet;
}


#if defined(__ISX006_SONY__)||defined(__ISX005_SONY__)
S32 CamacqExtDirectlyWriteI2cLists_Sony( struct i2c_client *pClient, const void *pvArg, int iResType )
{
    S32 iRet = 0;
    U32 uiCnt = 0;
    U8 rgucWriteRegs[4] = {0, };
    _stSonyData* pstRegLists = 0;
    
   CamacqTraceDbg("CamacqExtDirectlyWriteI2cLists_Sony IN");

//    CamacqTraceDbg( ":IN addr : %x ", pClient->addr );

    if( iResType == CAMACQ_SENSOR_MAIN )
    {
        pstRegLists = (_stSonyData*)pvArg;
    }
#if (CAMACQ_SENSOR_MAX==2)
    else if( iResType == CAMACQ_SENSOR_SUB )
    {
        pstRegLists = (_stSonyData*)pvArg;
    }
#endif /* CAMACQ_SENSOR_MAX==2 */
       
    if( pstRegLists == NULL )  //BYKIM_PREVENT
            return -1;
	
    if( pstRegLists[uiCnt].ucLen == 0x02 )
    {
        rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
        rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
        rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
        rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);   
    }
    else if( pstRegLists[uiCnt].ucLen == 0x01 || pstRegLists[uiCnt].ucLen == 0x03 )
    {
        rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
        rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
        rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
        rgucWriteRegs[3] = 0x00;
    }
    else if( pstRegLists[uiCnt].ucLen == 0xFF )
    {
        rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
        rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
        rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
        rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);
    }
    else
    {
        CamacqTraceErr("Unexpected value!!");
        return iRet;
    }

    while( rgucWriteRegs[0] != 0xFF || rgucWriteRegs[1] != 0xFF || rgucWriteRegs[2] != 0xFF )
    {
        if( pstRegLists[uiCnt].ucLen != 0x03 )
        {
        iRet = CamacqExtWriteI2c( pClient, rgucWriteRegs, 2+pstRegLists[uiCnt].ucLen );
            if(iRet < 0)
            {
                CamacqTraceErr( ":write failed" );
                break;
            }
        
        // CamacqTraceDbg_v(":Length[%d] : 0x%02X%02X%02X%02X" , pstRegLists[uiCnt].ucLen, rgucWriteRegs[0] , 
        //                                rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
      //  CamacqTraceDbg(":Length[%d] : 0x%02X%02X%02X%02X" , pstRegLists[uiCnt].ucLen, rgucWriteRegs[0] , 
       //                                    rgucWriteRegs[1], rgucWriteRegs[2] , rgucWriteRegs[3]);
        }
        else // 0x03 is delay
        {
            CamacqExtDelay(rgucWriteRegs[2]);
            CamacqTraceDbg(" setfile delay : %d", rgucWriteRegs[2]);
        }        
        
        uiCnt++;
        
        if( pstRegLists[uiCnt].ucLen == 0x02 )
        {
            rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
            rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
            rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
            rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);   
        }
        else if(pstRegLists[uiCnt].ucLen == 0x01 || pstRegLists[uiCnt].ucLen == 0x03)
        {
            rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
            rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
            rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
            rgucWriteRegs[3] = 0x00;
        }
        else if( pstRegLists[uiCnt].ucLen == 0xFF )
        {
            rgucWriteRegs[0] = (U8)((pstRegLists[uiCnt].usRegs >> 8) & 0xFF);
            rgucWriteRegs[1] = (U8)(pstRegLists[uiCnt].usRegs & 0xFF);
            rgucWriteRegs[2] = (U8)(pstRegLists[uiCnt].usData & 0xFF);
            rgucWriteRegs[3] = (U8)(pstRegLists[uiCnt].usData >> 8 & 0xFF);
        }
        else
        {
            CamacqTraceErr("Unexpected value!!");
            return iRet;
        }
    }  
    CamacqTraceDbg_v("OUT");
    return iRet;
}
#endif /* __ISX006_SONY__ */


void CamacqExtDelay( S32 iDelay )
{
    msleep( iDelay );
    CamacqTraceDbg( ": iDelay [%d]", iDelay );
}

S32 CamacqExtRequestGpio( S32 iGpio, U8* szModuleName )
{
    S32 iRet = 0;
    CamacqTraceIN();

    iRet = gpio_request( iGpio, szModuleName );
    if( iRet < 0 ) 
    {
        CamacqTraceErr( " :invalid gpio mode, iRet = [%d] ", iRet );
        iRet = -EINVAL;
        goto CAMACQ_EXT_OUT;
    }

CAMACQ_EXT_OUT:
    CamacqTraceOUT();
    return iRet;
}

void CamacqExtFreeGpio( S32 iGpio )
{
    CamacqTraceIN();
    gpio_free(iGpio);
    CamacqTraceOUT();
}

S32 CamacqExtSetGpioConfig( S32 iGpio, S32 iMux, S32 iDirection, S32 iLevel )
{
    S32 iRet = 0;
    CamacqTraceIN();

    gpio_set_value( iGpio, iMux );

    iRet = gpio_direction_output( iGpio, iLevel );
    if(iRet < 0)
    {
        CamacqTraceErr( ":invalid gpio_direction_output" );
        iRet = -EINVAL;
        goto CAMACQ_EXT_OUT;
    }

CAMACQ_EXT_OUT:
    CamacqTraceOUT();
    return iRet;
}

/* File System API */
/* Must need to release return point value by caller side */
#if (CAMACQ_MAIN_FS_MODE)
U8* CamacqExtReadFs( const S8 * szFileName, _eCamacqSelLen eLen, int iResType )
{
    U8* pucRet;
    S8 rgcPath[ CAMACQ_EXT_MAX_PATH+1 ], *pcHex;
    U8 *pucBuff, *pucCurBuffPos, *pucMovBuffPos;
    U32 uiIdx, uiCnt = 0;
    U8 ucIdy, ucHexLen;
    U8 ucSonyFlag = 0;  // 1 is addr, 2 is data, 3 is data legth
    U8 rgucSonyAddr[2] = {0, };
    U8 ucSonyLen = 0;
    U8 szSonyData[ 7 ] = {0, }; // "0xFFFF"
    S32  uiLen=0;  //BYKIM_PREVENT
#if defined(_LINUX_)
    loff_t stpos = 0;
    struct file *fd;
#else
    S32 fd;
#endif /* _LINUX_ */

#if defined(_LINUX_)
    mm_segment_t fs = get_fs();
    set_fs(get_ds());
#endif /* _LINUX_ */

    CamacqTraceDbg( ":eLen[%d]", eLen );
    switch( eLen )
    {
        case CAMACQ_EXT_LEN_2BYTE_ARY:
        case CAMACQ_EXT_LEN_4BYTE_ARY:
            ucHexLen = strlen( CAMACQ_EXT_HEX_CHAR );
        break;
        case CAMACQ_EXT_LEN_2BYTE_INT:
            ucHexLen = strlen( CAMACQ_EXT_HEX_SHORT );
        break;        
        case CAMACQ_EXT_LEN_4BYTE_INT:
            ucHexLen = strlen( CAMACQ_EXT_HEX_INT );
        break;
        case CAMACQ_EXT_LEN_SONY:
            ucHexLen = strlen( CAMACQ_EXT_HEX_SHORT );
        break;
        default:
            ucHexLen = strlen( CAMACQ_EXT_HEX_INT );
        break;
    }
    CamacqTraceDbg( " :ucHexLen[%d] ", ucHexLen );
    
    CamacqTraceDbg( " : szFileName[%s] ", szFileName );
    memset( rgcPath, 0x00, sizeof(S8)*(CAMACQ_EXT_MAX_PATH+1) );
    if( iResType == CAMACQ_SENSOR_MAIN )
        sprintf( rgcPath, CAMACQ_MAIN_PATH_SET_FILE, szFileName );
#if (CAMACQ_SENSOR_MAX==2)
    else if( iResType == CAMACQ_SENSOR_SUB )
        sprintf( rgcPath, CAMACQ_SUB_PATH_SET_FILE, szFileName );
#endif /* CAMACQ_SENSOR_MAX==2 */

    CamacqTraceDbg( " :rgcPath[%s] ", rgcPath );

    fd = CamacqOpen( rgcPath, CAMACQ_RDONLY );
    if( IS_ERR(fd) )
    {
        CamacqTraceErr( " :invalid fd[%d] ", (S32)fd );
        return NULL;
    }

    pcHex = (S8*)CamacqMalloc( sizeof(S8)*(ucHexLen+1) );
    memset( pcHex, 0x00, sizeof(S8)*(ucHexLen+1) );

#if defined(WIN32)
    camacq_seek( fd, 0, SEEK_END );
#endif /* _LINUX_ */

    uiLen = CamacqTell( fd );
    CamacqSeek( fd, 0, SEEK_SET );

    pucBuff = (U8*)CamacqMalloc( uiLen );
    pucRet = (U8*)CamacqMalloc( uiLen/2 ); 
    memset( pucBuff, 0x00, uiLen );
    memset( pucRet, 0x00, uiLen/2 );
    
    uiLen = CamacqRead( fd, pucBuff, uiLen );
    CamacqTraceDbg( ":uiLen[%d]", uiLen );

    if(uiLen<0)   //BYKIM_PREVENT
    {
        CamacqTraceErr( " :invalid uiLen[%d] ",uiLen );
        return NULL;
    }
	
    ucSonyFlag = 1;
    for( uiIdx = 0; uiIdx < uiLen; uiIdx++ )
    {
        pucCurBuffPos = pucBuff + uiIdx;
        if( CAMACQ_EXT_COMMENT_START(pucCurBuffPos) ) /* Comment Start '/*'  or '//'  */
        {
            CamacqTraceDbg( "pucCurBuffPos[0x%0x]", pucCurBuffPos );
            pucMovBuffPos = (U8*)strstr( (S8*)pucCurBuffPos, CAMACQ_EXT_COMMENT_CLOSE);
            if(pucMovBuffPos != NULL)
            {
                uiIdx += (pucMovBuffPos - pucCurBuffPos);
             }	
	    else
	     {
                CamacqTraceDbg( "There is no */ : Old style" );
                CamacqTraceDbg( "pucCurBuffPos[0x%0x]", pucCurBuffPos );
                pucCurBuffPos = pucBuff + uiIdx;
                CamacqTraceDbg( "pucCurBuffPos[0x%0x]", pucCurBuffPos );
                pucMovBuffPos = (U8*)strstr( (S8*)pucCurBuffPos, CAMACQ_EXT_START);
                if(pucMovBuffPos != NULL)
                {
                    uiIdx += (pucMovBuffPos - pucCurBuffPos);
                 }	
		 else
                 {
                        CamacqTraceDbg( " !! Error uiIdx : %d", uiIdx );
                  }
	     }	
        }
        else if( CAMACQ_EXT_HEXA_START(pucCurBuffPos) ) /* HexValue Start */
        {
            memcpy( pcHex, pucCurBuffPos, ucHexLen );
            // CamacqTraceDbg_v( " : uiIdx[ %d ], pcHex[ %s ]", uiIdx, pcHex );
            if( eLen <= CAMACQ_EXT_LEN_4BYTE_ARY )
            {
                pucRet[ uiCnt++ ] = CamacqExtAsc2Hex( pcHex + CAMACQ_EXT_HEX_HALF_LEN );
                uiIdx += ucHexLen;
            }
            else if( eLen == CAMACQ_EXT_LEN_SONY )
            {
                // addr
                if( ucSonyFlag == 1 )
                {
                    for( ucIdy = 0; ucIdy < ( ucHexLen-CAMACQ_EXT_HEX_HALF_LEN ); ucIdy+=2 )
                    {
                        rgucSonyAddr[ ucIdy / 2 ] = CamacqExtAsc2Hex( pcHex + CAMACQ_EXT_HEX_HALF_LEN + ucIdy );
                        CamacqTraceDbg_v( " : rgucSonyAddr[ %d ] = %x ", ucIdy / 2, rgucSonyAddr[ ucIdy / 2 ] );
                    }
                    uiIdx += ucHexLen;
                    ucSonyFlag = 2;
                }
                // data
                else if( ucSonyFlag == 2 )
                {
                    memcpy( szSonyData, pcHex, ucHexLen );
                    CamacqTraceDbg_v( " szSonyData : %s", szSonyData );
                    uiIdx += CAMACQ_EXT_HEX_HALF_LEN;
                    ucSonyFlag = 3;
                }
                // length
                else if( ucSonyFlag == 3 )
                {
                    ucSonyLen = CamacqExtAsc2Hex( pcHex + CAMACQ_EXT_HEX_HALF_LEN );

                    CamacqTraceDbg_v( " ucSonyLen : %d", ucSonyLen );

                    // 1. save length
                    pucRet[ uiCnt++ ] = ucSonyLen;

                    // 2. save addr
                    memcpy( pucRet+uiCnt, rgucSonyAddr, sizeof(rgucSonyAddr) );
                    uiCnt += sizeof(rgucSonyAddr);

                    // 3. save data
                    if( (ucSonyLen == 1 ) ||( ucSonyLen == 3 ))
                    {
                        // pucRet[ uiCnt++ ] = 0x00;   // padding
                        pucRet[ uiCnt++ ] = CamacqExtAsc2Hex( szSonyData + CAMACQ_EXT_HEX_HALF_LEN );
                    }
                    else if( ucSonyLen == 2 )
                    {
                        for( ucIdy = 0; ucIdy < ucSonyLen; ucIdy++ )
                        {
                            pucRet[ uiCnt++ ] = CamacqExtAsc2Hex( szSonyData + CAMACQ_EXT_HEX_HALF_LEN + (ucIdy * 2) );
                        }
                    }
                    else if( ucSonyLen == 0xFF )
                    {
                        break;
                    }
                    else
                    {
                        CamacqTraceDbg( " !! Error ucSonyLen : %d", ucSonyLen );
                    }
                    
                    uiIdx += CAMACQ_EXT_HEX_HALF_LEN;
                    ucSonyFlag = 1;
                }
            }
            else
            {
                for( ucIdy = 0; ucIdy < ( ucHexLen-CAMACQ_EXT_HEX_HALF_LEN ); ucIdy+=2 )
                {
                    pucRet[ uiCnt++ ] = CamacqExtAsc2Hex( pcHex + CAMACQ_EXT_HEX_HALF_LEN + ucIdy );
                }
                uiIdx += ucHexLen;
            }
        }
    }

    CamacqFree( pucBuff );
    CamacqFree( pcHex );
#if defined(_LINUX_)
    set_fs( fs );
#endif /* _LINUX_ */

    return pucRet;
}

U8  CamacqExtAsc2Hex( S8 *pcAscii )
{
    U8 ucRetHex = 0, ucCurHex = 0;
    U8 ucIdx;

    for( ucIdx = 0; ucIdx < CAMACQ_EXT_HEX_HALF_LEN; ucIdx++ )
    {
        if(pcAscii[ucIdx] >= CAMACQ_EXT_a && pcAscii[ucIdx] <= CAMACQ_EXT_f)
        {
            ucCurHex = (pcAscii[ucIdx]-CAMACQ_EXT_a+10)*(CAMACQ_EXT_HEX_WIDTH);
        }
        else if(pcAscii[ucIdx] >= CAMACQ_EXT_A && pcAscii[ucIdx] <= CAMACQ_EXT_F)
        {
            ucCurHex = (pcAscii[ucIdx]-CAMACQ_EXT_A+10)*(CAMACQ_EXT_HEX_WIDTH);
        }
        else if(pcAscii[ucIdx] >= CAMACQ_EXT_0 && pcAscii[ucIdx] <= CAMACQ_EXT_9)
        {
            ucCurHex = (pcAscii[ucIdx]-CAMACQ_EXT_0)*(CAMACQ_EXT_HEX_WIDTH);
        }

        if(ucIdx != 0)
            ucCurHex /= CAMACQ_EXT_HEX_WIDTH;

        ucRetHex += ucCurHex;
    }

    return ucRetHex;
}
#endif

#undef _CAMACQ_EXT_C_
#endif /* _CAMACQ_EXT_C_ */

