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

#if !defined(_CAMACQ_S5K6AAFX13_H_)
#define _CAMACQ_S5K6AAFX13_H_

/* Include */
#include "camacq_api.h"

/* Global */
#undef GLOBAL

#if !defined(_CAMACQ_API_C_)
#define GLOBAL extern
#else
#define GLOBAL
#endif /* _CAMACQ_API_C_ */

/* Include */
#if defined(WIN32)
#include "cmmfile.h"
#endif /* WIN32 */

/* Definition */

#define CAMACQ_SUB_CHIP_IDENT   V4L2_IDENT_S5K6AAFX

#define CAMACQ_SUB_NAME         "s5k6aafx13" 
#define CAMACQ_SUB_I2C_ID       0x3c	// 0x78
#define CAMACQ_SUB_RES_TYPE		CAMACQ_SENSOR_LOW   // sub sensor

#define CAMACQ_SUB_ISPROBED     0
#define CAMACQ_SUB_CLOCK        0               
#define CAMACQ_SUB_YUVORDER     0
#define CAMACQ_SUB_V_SYNCPOL    0
#define CAMACQ_SUB_H_SYNCPOL    0
#define CAMACQ_SUB_SAMPLE_EDGE  0
#define CAMACQ_SUB_FULL_RANGE   0

#define CAMACQ_SUB_RST 
#define CAMACQ_SUB_RST_MUX 
#define CAMACQ_SUB_EN 
#define CAMACQ_SUB_EN_MUX 

#define CAMACQ_SUB_RST_ON           1
#define CAMACQ_SUB_RST_OFF          0
#define CAMACQ_SUB_EN_ON            1
#define CAMACQ_SUB_EN_OFF           0
#define CAMACQ_SUB_STANDBY_BEGIN    0
#define CAMACQ_SUB_STANDBY_END	    1

#define CAMACQ_SUB_POWER_ON         CamacqSubPowerOn();
#define CAMACQ_SUB_POWER_OFF        CamacqSubPowerOff();    

#define CAMACQ_SUB_2BYTE_SENSOR 0
#define CAMACQ_SUB_AF 0
#define CAMACQ_SUB_INT_MODE 1   // 0xAABBCCDD is INT_MODE, 0xAA, 0xBB, 0xCC, 0xDD is ARRAY MODE
#define CAMACQ_SUB_FS_MODE 0
#define CAMACQ_SUB_PATH_SET_FILE "/sdcard/sensor/sub/%s.dat"

#if (CAMACQ_SUB_2BYTE_SENSOR)	
#define CAMACQ_SUB_BURST_MODE 0
#else
#define CAMACQ_SUB_BURST_MODE 0
#endif /* CAMACQ_SUB_2BYTE_SENSOR */

#define CAMACQ_SUB_BURST_I2C_TRACE 0
#define CAMACQ_SUB_BURST_MAX 100

#define CAMACQ_SUB_REG_FLAG_CNTS 	0x0F12
#define CAMACQ_SUB_REG_DELAY 		0xFFFF       
#define CAMACQ_SUB_BTM_OF_DATA 		0xFFFFFFFF
#define CAMACQ_SUB_END_MARKER 		0xFF
#define CAMACQ_SUB_REG_SET_SZ 1	// {0xFFFFFFFF} is 1, {0xFFFF,0xFFFF} is 2, {0xFF,0XFF} is 2, {0xFF,0xFF,0xFF,0xFF} is 4, {0xFFFF} is 1
#define CAMACQ_SUB_REG_DAT_SZ 4 // {0xFFFFFFFF} is 4, {0xFFFF,0xFFFF} is 2, {0xFF,0XFF} is 1, {0xFF,0xFF,0xFF,0xFF} is 1, {0xFFFF} is 2

#define CAMACQ_SUB_FRATE_MIN  5
#define CAMACQ_SUB_FRATE_MAX  30

// MACRO FUNCTIONS BEGIN ////////////////////////////////////////////////////////////
#if (CAMACQ_SUB_2BYTE_SENSOR)
#define CAMACQ_SUB_EXT_RD_SZ 1
#else
#define CAMACQ_SUB_EXT_RD_SZ 2
#endif /* CAMACQ_SUB_2BYTE_SENSOR */

/* write reglists macros */
#if CAMACQ_SUB_2BYTE_SENSOR
#define CAMACQ_SUB_EXT_REG_IS_BTM_OF_DATA(A)		(((A[0]==CAMACQ_SUB_END_MARKER) && (A[1]==CAMACQ_SUB_END_MARKER))? 1:0)
#define CAMACQ_SUB_EXT_REG_IS_DELAY(A)				((A[0]==CAMACQ_SUB_REG_DELAY)? 1:0)

#if (CAMACQ_SUB_FS_MODE==1)
#define CAMACQ_SUB_EXT_REG_GET_DATA(dest,srce,idx)\
memcpy(dest, &(srce[idx*CAMACQ_SUB_REG_DAT_SZ*CAMACQ_SUB_REG_SET_SZ]), CAMACQ_SUB_REG_DAT_SZ*CAMACQ_SUB_REG_SET_SZ);
#elif (CAMACQ_SUB_REG_DAT_SZ==1)
#define CAMACQ_SUB_EXT_REG_GET_DATA(dest,srce,idx)	dest[0] = (srce[idx][0] & 0xFF); dest[1] = (srce[idx][1] & 0xFF);
#elif (CAMACQ_SUB_REG_DAT_SZ==2)
#define CAMACQ_SUB_EXT_REG_GET_DATA(dest,srce,idx)	dest[0] = ((U8)(srce[idx] >> 8) & 0xFF); dest[1] = ((U8)(srce[idx]) & 0xFF);
#endif

#else // CAMACQ_SUB_2BYTE_SENSOR

#define CAMACQ_SUB_EXT_REG_IS_BTM_OF_DATA(A)		(((A[0]==CAMACQ_SUB_END_MARKER) && (A[1]==CAMACQ_SUB_END_MARKER) && \
(A[2]==CAMACQ_SUB_END_MARKER) && (A[3]==CAMACQ_SUB_END_MARKER))? 1:0)
#define CAMACQ_SUB_EXT_REG_IS_DELAY(A)				(((A[0]==((CAMACQ_SUB_REG_DELAY>>8) & 0xFF)) && (A[1]==(CAMACQ_SUB_REG_DELAY & 0xFF)))? 1:0)
#define CAMACQ_SUB_EXT_REG_IS_CNTS(A)				(((A[0]==((CAMACQ_SUB_REG_FLAG_CNTS>>8) & 0xFF)) && (A[1]==(CAMACQ_SUB_REG_FLAG_CNTS & 0xFF)))? 1:0)

#if (CAMACQ_SUB_FS_MODE==1)
#define CAMACQ_SUB_EXT_REG_GET_DATA(dest,srce,idx)\
memcpy(dest, &(srce[idx*CAMACQ_SUB_REG_DAT_SZ*CAMACQ_SUB_REG_SET_SZ]), CAMACQ_SUB_REG_DAT_SZ*CAMACQ_SUB_REG_SET_SZ);
#elif (CAMACQ_SUB_REG_DAT_SZ==2)
#define CAMACQ_SUB_EXT_REG_GET_DATA(dest,srce,idx)	dest[0]=((srce[idx][0] >> 8) & 0xFF); dest[1]=(srce[idx][0] & 0xFF); \
dest[2]=((srce[idx][1] >> 8) & 0xFF); dest[3]=(srce[idx][1] & 0xFF);
#elif (CAMACQ_SUB_REG_DAT_SZ==1)
#define CAMACQ_SUB_EXT_REG_GET_DATA(dest,srce,idx)	dest[0]=srce[idx][0]; dest[1]=srce[idx][1]; \
dest[2]=srce[idx][2]; dest[3]=srce[idx][3];
#elif (CAMACQ_SUB_REG_DAT_SZ==4)
#define CAMACQ_SUB_EXT_REG_GET_DATA(dest,srce,idx)	dest[0] = ((U8)(srce[idx] >> 24) & 0xFF); dest[1] = ((U8)(srce[idx] >> 16) & 0xFF); \
dest[2] = ((U8)(srce[idx] >> 8) & 0xFF); dest[3] = ((U8)(srce[idx]) & 0xFF);			
#endif
#endif /* CAMACQ_SUB_2BYTE_SENSOR */
// MACRO FUNCTIONS END ///////////////////////////////////////////////////////////

#if( CAMACQ_SUB_FS_MODE )
#define CAMACQ_SUB_REG_INIT     "reg_sub_init"
#define CAMACQ_SUB_REG_SLEEP    "reg_sub_sleep"
#define CAMACQ_SUB_REG_WAKEUP   "reg_sub_wakeup"
#else
#define CAMACQ_SUB_REG_INIT     reg_sub_init
#define CAMACQ_SUB_REG_SLEEP    reg_sub_sleep
#define CAMACQ_SUB_REG_WAKEUP   reg_sub_wakeup
#endif

// #define CAMACQ_SUB_REG_INIT reg_sub_init
#define CAMACQ_SUB_REG_PREVIEW reg_sub_preview
#define CAMACQ_SUB_REG_SNAPSHOT reg_sub_snapshot
#define CAMACQ_SUB_REG_MIDLIGHT reg_sub_midlight
#define CAMACQ_SUB_REG_LOWLIGHT reg_sub_lowlight
#define CAMACQ_SUB_REG_NIGHTSHOT_ON reg_sub_nightshot_on
#define CAMACQ_SUB_REG_NIGHTSHOT_OFF reg_sub_nightshot_off
#define CAMACQ_SUB_REG_NIGHTSHOT reg_sub_nightshot

#define CAMACQ_SUB_REG_WB_AUTO reg_sub_wb_auto
#define CAMACQ_SUB_REG_WB_DAYLIGHT reg_sub_wb_daylight
#define CAMACQ_SUB_REG_WB_CLOUDY reg_sub_wb_cloudy
#define CAMACQ_SUB_REG_WB_INCANDESCENT reg_sub_wb_incandescent
#define CAMACQ_SUB_REG_WB_FLUORESCENT reg_sub_wb_fluorescent
#define CAMACQ_SUB_REG_WB_TWILIGHT reg_sub_wb_twilight
#define CAMACQ_SUB_REG_WB_TUNGSTEN reg_sub_wb_tungsten
#define CAMACQ_SUB_REG_WB_OFF reg_sub_wb_off
#define CAMACQ_SUB_REG_WB_HORIZON reg_sub_wb_horizon
#define CAMACQ_SUB_REG_WB_SHADE reg_sub_wb_shade

#define CAMACQ_SUB_REG_EFFECT_NONE reg_sub_effect_none
#define CAMACQ_SUB_REG_EFFECT_GRAY reg_sub_effect_gray // mono, blackwhite
#define CAMACQ_SUB_REG_EFFECT_NEGATIVE reg_sub_effect_negative
#define CAMACQ_SUB_REG_EFFECT_SOLARIZE reg_sub_effect_solarize
#define CAMACQ_SUB_REG_EFFECT_SEPIA reg_sub_effect_sepia
#define CAMACQ_SUB_REG_EFFECT_POSTERIZE reg_sub_effect_posterize
#define CAMACQ_SUB_REG_EFFECT_WHITEBOARD reg_sub_effect_whiteboard
#define CAMACQ_SUB_REG_EFFECT_BLACKBOARD reg_sub_effect_blackboard
#define CAMACQ_SUB_REG_EFFECT_AQUA reg_sub_effect_aqua
#define CAMACQ_SUB_REG_EFFECT_SHARPEN  reg_sub_effect_sharpen
#define CAMACQ_SUB_REG_EFFECT_VIVID  reg_sub_effect_vivid
#define CAMACQ_SUB_REG_EFFECT_GREEN reg_sub_effect_green
#define CAMACQ_SUB_REG_EFFECT_SKETCH reg_sub_effect_sketch

#define CAMACQ_SUB_REG_METER_MATRIX reg_sub_meter_matrix
#define CAMACQ_SUB_REG_METER_CW reg_sub_meter_cw
#define CAMACQ_SUB_REG_METER_SPOT reg_sub_meter_spot
#define CAMACQ_SUB_REG_FLIP_NONE reg_sub_flip_none
#define CAMACQ_SUB_REG_FLIP_WATER reg_sub_flip_water
#define CAMACQ_SUB_REG_FLIP_MIRROR reg_sub_flip_mirror
#define CAMACQ_SUB_REG_FLIP_WATER_MIRROR reg_sub_flip_water_mirror
#define CAMACQ_SUB_REG_FPS_FIXED_7 reg_sub_fps_fixed_7
#define CAMACQ_SUB_REG_FPS_FIXED_10 reg_sub_fps_fixed_10
#define CAMACQ_SUB_REG_FPS_FIXED_15 reg_sub_fps_fixed_15
#define CAMACQ_SUB_REG_FPS_VAR_15 reg_sub_fps_var_15

#define CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_0 reg_sub_brightness_level_0
#define CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_1 reg_sub_brightness_level_1
#define CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_2 reg_sub_brightness_level_2
#define CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_3 reg_sub_brightness_level_3
#define CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_4 reg_sub_brightness_level_4
#define CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_5 reg_sub_brightness_level_5
#define CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_6 reg_sub_brightness_level_6
#define CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_7 reg_sub_brightness_level_7
#define CAMACQ_SUB_REG_BRIGHTNESS_LEVEL_8 reg_sub_brightness_level_8

#define CAMACQ_SUB_REG_SET_AF reg_sub_set_af  // start af
#define CAMACQ_SUB_REG_OFF_AF reg_sub_off_af
#define CAMACQ_SUB_REG_CHECK_AF reg_sub_check_af
#define CAMACQ_SUB_REG_RESET_AF reg_sub_reset_af
#define CAMACQ_SUB_REG_MANUAL_AF reg_sub_manual_af    // normal_af
#define CAMACQ_SUB_REG_MACRO_AF reg_sub_macro_af
#define CAMACQ_SUB_REG_RETURN_MANUAL_AF reg_sub_return_manual_af
#define CAMACQ_SUB_REG_RETURN_MACRO_AF reg_sub_return_macro_af
#define CAMACQ_SUB_REG_SET_AF_NLUX reg_sub_set_af_nlux
#define CAMACQ_SUB_REG_SET_AF_LLUX reg_sub_set_af_llux

#define CAMACQ_SUB_REG_ISO_AUTO reg_sub_iso_auto
#define CAMACQ_SUB_REG_ISO_50 reg_sub_iso_50
#define CAMACQ_SUB_REG_ISO_100 reg_sub_iso_100
#define CAMACQ_SUB_REG_ISO_200 reg_sub_iso_200
#define CAMACQ_SUB_REG_ISO_400 reg_sub_iso_400
#define CAMACQ_SUB_REG_ISO_800 reg_sub_iso_800
#define CAMACQ_SUB_REG_ISO_1600 reg_sub_iso_1600
#define CAMACQ_SUB_REG_ISO_3200 reg_sub_iso_3200

#define CAMACQ_SUB_REG_SCENE_AUTO reg_sub_scene_auto  // auto, off
#define CAMACQ_SUB_REG_SCENE_NIGHT reg_sub_scene_night
#define CAMACQ_SUB_REG_SCENE_LANDSCAPE reg_sub_scene_landscape
#define CAMACQ_SUB_REG_SCENE_SUNSET reg_sub_scene_sunset
#define CAMACQ_SUB_REG_SCENE_PORTRAIT reg_sub_scene_portrait
#define CAMACQ_SUB_REG_SCENE_SUNRISE reg_sub_scene_sunrise    // dawn
#define CAMACQ_SUB_REG_SCENE_INDOOR reg_sub_scene_indoor
#define CAMACQ_SUB_REG_SCENE_PARTY reg_sub_scene_party
#define CAMACQ_SUB_REG_SCENE_SPORTS reg_sub_scene_sports
#define CAMACQ_SUB_REG_SCENE_BEACH reg_sub_scene_beach
#define CAMACQ_SUB_REG_SCENE_SNOW reg_sub_scene_snow
#define CAMACQ_SUB_REG_SCENE_FALLCOLOR reg_sub_scene_fallcolor
#define CAMACQ_SUB_REG_SCENE_FIREWORKS reg_sub_scene_fireworks
#define CAMACQ_SUB_REG_SCENE_CANDLELIGHT reg_sub_scene_candlelight
#define CAMACQ_SUB_REG_SCENE_AGAINSTLIGHT reg_sub_scene_againstlight  // backlight
#define CAMACQ_SUB_REG_SCENE_TEXT reg_sub_scene_text

#define CAMACQ_SUB_REG_QQVGA reg_sub_qqvga
#define CAMACQ_SUB_REG_QCIF reg_sub_qcif
#define CAMACQ_SUB_REG_QVGA reg_sub_qvga
#define CAMACQ_SUB_REG_WQVGA reg_sub_wqvga
#define CAMACQ_SUB_REG_CIF reg_sub_cif
#define CAMACQ_SUB_REG_VGA reg_sub_vga	
#define CAMACQ_SUB_REG_WVGA reg_sub_wvga
#define CAMACQ_SUB_REG_SVGA reg_sub_svga
#define CAMACQ_SUB_REG_SXGA reg_sub_sxga
#define CAMACQ_SUB_REG_QXGA reg_sub_qxga
#define CAMACQ_SUB_REG_FULL_YUV reg_sub_full_yuv
#define CAMACQ_SUB_REG_CROP_YUV reg_sub_crop_yuv

#define CAMACQ_SUB_REG_FLICKER_DISABLED reg_sub_flicker_disabled
#define CAMACQ_SUB_REG_FLICKER_50HZ reg_sub_flicker_50hz
#define CAMACQ_SUB_REG_FLICKER_60HZ reg_sub_flicker_60hz
#define CAMACQ_SUB_REG_FLICKER_AUTO reg_sub_flicker_auto

// NEW
#define CAMACQ_SUB_REG_FMT_YUV422 	reg_sub_fmt_yuv422
#define CAMACQ_SUB_REG_FMT_JPG		reg_sub_fmt_jpg

#define CAMACQ_SUB_REG_QVGA_V		reg_sub_qvga_v
#define CAMACQ_SUB_REG_HALF_VGA_V	reg_sub_half_vga_v
#define CAMACQ_SUB_REG_HALF_VGA		reg_sub_half_vga	
#define CAMACQ_SUB_REG_VGA_V		reg_sub_vga_v			
#define CAMACQ_SUB_REG_5M			reg_sub_5M
#define CAMACQ_SUB_REG_1080P		reg_sub_1080P
#define CAMACQ_SUB_REG_720P			reg_sub_720P

/* Enumeration */

/* Type Definition */

/* Global Value */
GLOBAL const U32 reg_sub_sleep[]
#if defined(_CAMACQ_API_C_)
={
//********,********************************/
0xFCFCD000,
//********,********************************/

0x00100001,	// Re0xet
0x10300000,	// Clear ho0xt interrupt 0xo main will wait
0x00140001,	// ARM go

// p100	// W,ait100mSec
0xFFFF0064,

0x00287000,
0x002A0AEA,
0x0F12001E,
0x0F12001E,
0x002A01F0,
0x0F120000,
0x0F120001,

// p100
0xFFFF0064,

// MIPI ULPM-Enter
0x0028D000,
0x002AB0B0,
0x0f120001,

// p100
0xFFFF0064,

0x002AB0B4,
0x0f120000,
0x002AB0A0,	
0x0f120031,
0x002AB0A0,	
0x0f120010,
0x002AB0A0,	
0x0f120000,

0x00287000,
0x002A022E,
0x0F120001,
0x0F120001,


CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_wakeup[]
#if defined(_CAMACQ_API_C_)
={
0xFCFCD000,

0x0028D000,
0x002A003A,
0x0F120000,
0x002AB0B0,
0x0F120000, // escape ULPS mode

0x00287000,
0x002A022E,
0x0F120000,
0x0F120001, // #REG_TC_GP_SleepModeChanged

0x0028D000,
0x002A1000,
0x0F120001,

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;


GLOBAL const U32 reg_sub_init[]
#if defined(_CAMACQ_API_C_)
={
//********,********************************/
0xFCFCD000,
//********,********************************/

0x00100001,	// Re0xet
0x10300000,	// Clear ho0xt interrupt 0xo main will wait
0x00140001,	// ARM go

// p100	// W,ait100mSec
0xFFFF0064,

// Start T,&P part
0x00287000,
0x002A1D60,
0x0F12B570,
0x0F124936,
0x0F124836,
0x0F122205,
0x0F12F000,
0x0F12FA4E,
0x0F124935,
0x0F122002,
0x0F1283C8,
0x0F122001,
0x0F123120,
0x0F128088,
0x0F124933,
0x0F120200,
0x0F128008,
0x0F124933,
0x0F128048,
0x0F124933,
0x0F124833,
0x0F122204,
0x0F12F000,
0x0F12FA3E,
0x0F124932,
0x0F124833,
0x0F122206,
0x0F12F000,
0x0F12FA39,
0x0F124932,
0x0F124832,
0x0F122207,
0x0F12F000,
0x0F12FA34,
0x0F124931,
0x0F124832,
0x0F122208,
0x0F12F000,
0x0F12FA2F,
0x0F124931,
0x0F124831,
0x0F122209,
0x0F12F000,
0x0F12FA2A,
0x0F124930,
0x0F124831,
0x0F12220A,
0x0F12F000,
0x0F12FA25,
0x0F124930,
0x0F124830,
0x0F12220B,
0x0F12F000,
0x0F12FA20,
0x0F12482F,
0x0F124930,
0x0F126108,
0x0F124830,
0x0F1239FF,
0x0F123901,
0x0F126748,
0x0F12482F,
0x0F121C0A,
0x0F1232C0,
0x0F126390,
0x0F12482E,
0x0F126708,
0x0F12491A,
0x0F12482D,
0x0F123108,
0x0F1260C1,
0x0F126882,
0x0F121A51,
0x0F128201,
0x0F124C2B,
0x0F122607,
0x0F126821,
0x0F120736,
0x0F1242B1,
0x0F12DA05,
0x0F124829,
0x0F1222D8,
0x0F121C05,
0x0F12F000,
0x0F12FA09,
0x0F126025,
0x0F1268A1,
0x0F1242B1,
0x0F12DA07,
0x0F124825,
0x0F122224,
0x0F123824,
0x0F12F000,
0x0F12FA00,
0x0F124822,
0x0F123824,
0x0F1260A0,
0x0F124D22,
0x0F126D29,
0x0F1242B1,
0x0F12DA07,
0x0F12481F,
0x0F12228F,
0x0F1200D2,
0x0F1230D8,
0x0F121C04,
0x0F12F000,
0x0F12F9F2,
0x0F12652C,
0x0F12BC70,
0x0F12BC08,
0x0F124718,
0x0F12218B,
0x0F127000,
0x0F12127B,
0x0F120000,
0x0F120398,
0x0F127000,
0x0F121376,
0x0F127000,
0x0F122370,
0x0F127000,
0x0F121F0D,
0x0F127000,
0x0F12890D,
0x0F120000,
0x0F121F2F,
0x0F127000,
0x0F1227A9,
0x0F120000,
0x0F121FE1,
0x0F127000,
0x0F1227C5,
0x0F120000,
0x0F122043,
0x0F127000,
0x0F12285F,
0x0F120000,
0x0F122003,
0x0F127000,
0x0F1228FF,
0x0F120000,
0x0F1220CD,
0x0F127000,
0x0F126181,
0x0F120000,
0x0F1220EF,
0x0F127000,
0x0F126663,
0x0F120000,
0x0F122123,
0x0F127000,
0x0F120100,
0x0F127000,
0x0F121EC1,
0x0F127000,
0x0F121EAD,
0x0F127000,
0x0F121F79,
0x0F127000,
0x0F1204AC,
0x0F127000,
0x0F1206CC,
0x0F127000,
0x0F1223A4,
0x0F127000,
0x0F120704,
0x0F127000,
0x0F12B510,
0x0F12F000,
0x0F12F9B9,
0x0F1248C3,
0x0F1249C3,
0x0F128800,
0x0F128048,
0x0F12BC10,
0x0F12BC08,
0x0F124718,
0x0F12B5F8,
0x0F121C06,
0x0F124DC0,
0x0F1268AC,
0x0F121C30,
0x0F12F000,
0x0F12F9B3,
0x0F1268A9,
0x0F124ABC,
0x0F1242A1,
0x0F12D003,
0x0F124BBD,
0x0F128A1B,
0x0F123301,
0x0F128013,
0x0F128813,
0x0F121C14,
0x0F122B00,
0x0F12D00F,
0x0F122201,
0x0F124281,
0x0F12D003,
0x0F128C2F,
0x0F1242B9,
0x0F12D300,
0x0F122200,
0x0F1260AE,
0x0F122A00,
0x0F12D003,
0x0F128C28,
0x0F1242B0,
0x0F12D800,
0x0F121C30,
0x0F121E59,
0x0F128021,
0x0F12BCF8,
0x0F12BC08,
0x0F124718,
0x0F12B510,
0x0F121C04,
0x0F1248AF,
0x0F12F000,
0x0F12F997,
0x0F124AAD,
0x0F124BAE,
0x0F128811,
0x0F12885B,
0x0F128852,
0x0F124359,
0x0F121889,
0x0F124288,
0x0F12D800,
0x0F121C08,
0x0F126020,
0x0F12E7C5,
0x0F12B570,
0x0F121C05,
0x0F12F000,
0x0F12F98F,
0x0F1249A5,
0x0F128989,
0x0F124348,
0x0F120200,
0x0F120C00,
0x0F122101,
0x0F120349,
0x0F12F000,
0x0F12F98E,
0x0F121C04,
0x0F12489F,
0x0F128F80,
0x0F12F000,
0x0F12F991,
0x0F121C01,
0x0F1220FF,
0x0F1243C0,
0x0F12F000,
0x0F12F994,
0x0F12F000,
0x0F12F998,
0x0F121C01,
0x0F124898,
0x0F128840,
0x0F124360,
0x0F120200,
0x0F120C00,
0x0F12F000,
0x0F12F97A,
0x0F126028,
0x0F12BC70,
0x0F12BC08,
0x0F124718,
0x0F12B5F1,
0x0F12B082,
0x0F124D96,
0x0F124E91,
0x0F1288A8,
0x0F121C2C,
0x0F123420,
0x0F124F90,
0x0F122800,
0x0F12D018,
0x0F12F000,
0x0F12F988,
0x0F129001,
0x0F129802,
0x0F126B39,
0x0F120200,
0x0F12F000,
0x0F12F974,
0x0F12F000,
0x0F12F978,
0x0F129901,
0x0F12F000,
0x0F12F95F,
0x0F128020,
0x0F128871,
0x0F120200,
0x0F12F000,
0x0F12F96A,
0x0F120400,
0x0F120C00,
0x0F1221FF,
0x0F123101,
0x0F12F000,
0x0F12F97A,
0x0F128020,
0x0F1288E8,
0x0F122800,
0x0F12D00A,
0x0F124980,
0x0F128820,
0x0F123128,
0x0F12F000,
0x0F12F979,
0x0F128D38,
0x0F128871,
0x0F124348,
0x0F120200,
0x0F120C00,
0x0F128538,
0x0F12BCFE,
0x0F12BC08,
0x0F124718,
0x0F12B510,
0x0F121C04,
0x0F12F000,
0x0F12F974,
0x0F126821,
0x0F120409,
0x0F120C09,
0x0F121A40,
0x0F124976,
0x0F126849,
0x0F124281,
0x0F12D800,
0x0F121C08,
0x0F12F000,
0x0F12F971,
0x0F126020,
0x0F12E75B,
0x0F12B570,
0x0F126801,
0x0F12040D,
0x0F120C2D,
0x0F126844,
0x0F12486F,
0x0F128981,
0x0F121C28,
0x0F12F000,
0x0F12F927,
0x0F128060,
0x0F124970,
0x0F1269C9,
0x0F12F000,
0x0F12F968,
0x0F121C01,
0x0F1280A0,
0x0F120228,
0x0F12F000,
0x0F12F92D,
0x0F120400,
0x0F120C00,
0x0F128020,
0x0F12496B,
0x0F122300,
0x0F125EC9,
0x0F124288,
0x0F12DA02,
0x0F1220FF,
0x0F123001,
0x0F128020,
0x0F12E797,
0x0F12B5F8,
0x0F121C04,
0x0F124867,
0x0F124E65,
0x0F127800,
0x0F126AB7,
0x0F122800,
0x0F12D100,
0x0F126A37,
0x0F12495D,
0x0F122800,
0x0F12688D,
0x0F12D100,
0x0F12684D,
0x0F124859,
0x0F128841,
0x0F126820,
0x0F120200,
0x0F12F000,
0x0F12F94B,
0x0F128DF1, 
0x0F12434F,
0x0F120A3A,
0x0F124282,
0x0F12D30C,
0x0F124D5C,
0x0F1226FF,
0x0F128829,
0x0F123601,
0x0F1243B1,
0x0F128029,
0x0F12F000,
0x0F12F944,
0x0F126020,
0x0F128828,
0x0F124330,
0x0F128028,
0x0F12E73B,
0x0F121C0A,
0x0F12436A,
0x0F120A12,
0x0F124282,
0x0F12D304,
0x0F120200,
0x0F12F000,
0x0F12F8F3,
0x0F126020,
0x0F12E7F4,
0x0F126025,
0x0F12E7F2,
0x0F12B410,
0x0F124848,
0x0F124950,
0x0F1289C0,
0x0F122316,
0x0F125ECC,
0x0F121C02,
0x0F1242A0,
0x0F12DC00,
0x0F121C22,
0x0F1282CA,
0x0F122318,
0x0F125ECA,
0x0F124290,
0x0F12DC00,
0x0F121C10,
0x0F128308,
0x0F12BC10,
0x0F124770,
0x0F12B570,
0x0F121C06,
0x0F124C45,
0x0F122501,
0x0F128820,
0x0F1202AD,
0x0F1243A8,
0x0F128020,
0x0F12F000,
0x0F12F91E,
0x0F126030,
0x0F12F7FF,
0x0F12FFE0,
0x0F128820,
0x0F124328,
0x0F128020,
0x0F12E741,
0x0F12B570,
0x0F124C3D,
0x0F122501,
0x0F128820,
0x0F1202ED,
0x0F1243A8,
0x0F128020,
0x0F12F000,
0x0F12F916,
0x0F12F7FF,
0x0F12FFD1,
0x0F128820,
0x0F124328,
0x0F128020,
0x0F12E732,
0x0F12230D,
0x0F12071B,
0x0F1218C3,
0x0F128818,
0x0F122A00,
0x0F12D001,
0x0F124308,
0x0F12E000,
0x0F124388,
0x0F128018,
0x0F124770,
0x0F12B570,
0x0F122402,
0x0F124932,
0x0F128809,
0x0F12078A,
0x0F12D500,
0x0F122406,
0x0F122900,
0x0F12D01F,
0x0F121C02,
0x0F12207D,
0x0F1200C0,
0x0F122600,
0x0F124D2D,
0x0F122A00,
0x0F12D019,
0x0F122101,
0x0F128229,
0x0F12F000,
0x0F12F8F9,
0x0F122200,
0x0F122101,
0x0F12482A,
0x0F120309,
0x0F12F7FF,
0x0F12FFDB,
0x0F122008,
0x0F124304,
0x0F121C21,
0x0F124C26,
0x0F122200,
0x0F123C14,
0x0F121C20,
0x0F12F7FF,
0x0F12FFD2,
0x0F122200,
0x0F122121,
0x0F121C20,
0x0F12F7FF,
0x0F12FFCD,
0x0F12802E,
0x0F12E6FD,
0x0F12822E,
0x0F120789,
0x0F120FC9,
0x0F120089,
0x0F12223B,
0x0F124311,
0x0F128029,
0x0F12F000,
0x0F12F8DA,
0x0F12E7F4,
0x0F12B510,
0x0F12491B,
0x0F128FC8,
0x0F122800,
0x0F12D007,
0x0F122000,
0x0F1287C8,
0x0F128F88,
0x0F124C19,
0x0F122800,
0x0F12D002,
0x0F122008,
0x0F128020,
0x0F12E689,
0x0F124815,
0x0F123060,
0x0F128900,
0x0F122800,
0x0F12D103,
0x0F124814,
0x0F122101,
0x0F12F000,
0x0F12F8CA,
0x0F122010,
0x0F128020,
0x0F12E7F2,
0x0F120000,
0x0F121376,
0x0F127000,
0x0F122370,
0x0F127000,
0x0F1214D8,
0x0F127000,
0x0F12235C,
0x0F127000,
0x0F12F4B0,
0x0F120000,
0x0F121554,
0x0F127000,
0x0F121AB8,
0x0F127000,
0x0F120080,
0x0F127000,
0x0F12046C,
0x0F127000,
0x0F120468,
0x0F127000,
0x0F121100,
0x0F12D000,
0x0F12198C,
0x0F127000,
0x0F120AC4,
0x0F127000,
0x0F12B0A0,
0x0F12D000,
0x0F12B0B4,
0x0F120000,
0x0F1201B8,
0x0F127000,
0x0F12044E,
0x0F127000,
0x0F120450,
0x0F127000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F129CE7,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12F004,
0x0F12E51F,
0x0F129FB8,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F1214C1,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F1227E1,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F1288DF,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F12275D,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F121ED3,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F1227C5,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12F004,
0x0F12E51F,
0x0F12A144,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F121F87,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F1227A9,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F121ECB,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F1228FF,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F1226F9,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F124027,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F129F03,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12F004,
0x0F12E51F,
0x0F129D9C,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F12285F,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F126181,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F126663,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F1285D9,
0x0F120000,
0x0F124778,
0x0F1246C0,
0x0F12C000,
0x0F12E59F,
0x0F12FF1C,
0x0F12E12F,
0x0F122001,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F12E848,
0x0F120001,
0x0F12E848,
0x0F120001,
0x0F120500,
0x0F120064,
0x0F120002,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,  
// End T&P, part
  
// Start t,uning part
  
//	Write ,memory example:
//	m70003,000 10CE
//	Write ,HW regi0xter example:
//	1004 5,678
  
0xF4540001, 
0xF4180050,
0xF43E0010,

0x002A0E3A,
0x0F1202C2,
  
// Analog ,Settings
0x002A112A,
0x0F120000,
0x002A1132,
0x0F120000,
0x002A113E,
0x0F120000,
0x002A115C,
0x0F120000,
0x002A1164,
0x0F120000,
0x002A1174,
0x0F120000,
0x002A1178,
0x0F120000,
  
0x002A077A,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
  
0x002A07A2,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
  
0x002A07B6,
0x0F120000,
0x0F120002,
0x0F120004,
0x0F120004,
0x0F120005,
0x0F120005,

//EDS Check Code
0x00287000,
0x002A0132,
0x0F12AAAA,

//////////////////////////                                
//3. AE & AE weight                                       
//////////////////////////                                
                                                          
0x002A1000,   //#TVAR_ae_BrAve                             
0x0F120038,                                                          
0x002A0474,                                                
0x0F120112,   //#lt_uLimitHigh              
0x0F1200EF,   //#lt_uLimitLow               
                                                          
                                                          
0x002A1006,                                                 
0x0F12001F,    //#ae_StatMode                               
                                                          
0x002A108E,    //#SARR_IllumType                            
0x0F1200C7,                                                 
0x0F1200F7,                                                 
0x0F120107,                                                 
0x0F120142,                                                 
0x0F12017A,                                                 
0x0F1201A0,                                                 
0x0F1201B6,                                                 
                                                          
0x0F120100,    //#SARR_IllumTypeF	// 0112                   
0x0F120100,	   // 0122                                         
0x0F120100,	   // 0136                                         
0x0F120100,	   // 00F6                                         
0x0F120100,	   // 0100                                         
0x0F120100,	   // 00FE                                         
0x0F120100,	   // 0100                                         
                                                             
0x002A0488,                                                 
0x0F12410a,   //33.3m  //#lt_uMaxExp1                       
0x0F120000,                                                 
0x0F12A316,   //#lt_uMaxExp2                                
0x0F120000,                                                 
0x002A2360,   //#AWBBTune_EVT4_uMaxExp3                     
0x0F12F424,                                                 
0x0F120000,                                                 
                                                          
0x002A0490,   //#lt_uCapMaxExp1                             
0x0F12410a,   //33.3m                                       
0x0F120000,                                                 
0x0F12A316,   //#lt_uCapMaxExp2                             
0x0F120000,                                                 
0x002A2364,   //#AWBBTune_EVT4_uCapMaxExp3                  
0x0F12F424,                                                 
0x0F120000,                                                 
                                                          
0x002A0498,                                                 
0x0F120210,   //#lt_uMaxAnGain1       700lux shutter        
0x0F120310,   //#lt_uMaxAnGain2 	                          
0x002A2368,     	                                          
0x0F120690,	 //#AWBBTune_EVT4_uMaxAnGain3                   
0x002A049C,                                                 
0x0F120100,	 //#lt_uMaxDigGain                              
                                                          
0x002A235c,                                                 
0x0F120002,  //#AWBBTune_EVT4_uMinCoarse                    
0x0F120090,  //#AWBBTune_EVT4_uMinFine                      
                                                          
0x002A100e, //#ae_WeightTbl_16                              
0x0F120101,                                                 
0x0F120101,                                                 
0x0F120101,                                                 
0x0F120101,                                                 
0x0F120201,                                                 
0x0F120202,                                                 
0x0F120202,                                                 
0x0F120102,                                                 
0x0F120201,                                                 
0x0F120303,                                                 
0x0F120303,                                                 
0x0F120102,                                                 
0x0F120201,                                                 
0x0F120303,                                                 
0x0F120303,                                                 
0x0F120102,                                                 
0x0F120201,                                                 
0x0F120303,                                                 
0x0F120303,                                                 
0x0F120102,                                                 
0x0F120201,                                                 
0x0F120303,                                                 
0x0F120303,                                                 
0x0F120102,                                                 
0x0F120201,                                                 
0x0F120202,                                                 
0x0F120202,                                                 
0x0F120102,                                                 
0x0F120101,                                                 
0x0F120101,                                                 
0x0F120101,                                                 
0x0F120101,                                                 
                                                                                                               
0x002A0f4c, //brightness                                    
0x0F1202b0, //                                              
0x002A0f52, //                                              
0x0F1202f0, //                                              

//Shading Gas

0x002A0754,  //#TVAR_ash_pGAS
0x0F12247C,
0x0F127000,

//	param_,0xtart	TVAR_a0xh_pGAS 
0x002A247C,                      
0x0F120182,	//TVAR_a0xh_pGAS[0]  
0x0F120154,	//TVAR_a0xh_pGAS[1]  
0x0F120109,	//TVAR_a0xh_pGAS[2]  
0x0F1200CE,	//TVAR_a0xh_pGAS[3]  
0x0F1200AA,	//TVAR_a0xh_pGAS[4]  
0x0F120094,	//TVAR_a0xh_pGAS[5]  
0x0F12008E,	//TVAR_a0xh_pGAS[6]  
0x0F120094,	//TVAR_a0xh_pGAS[7]  
0x0F1200A9,	//TVAR_a0xh_pGAS[8]  
0x0F1200CF,	//TVAR_a0xh_pGAS[9]  
0x0F120108,	//TVAR_a0xh_pGAS[10] 
0x0F120154,	//TVAR_a0xh_pGAS[11] 
0x0F120187,	//TVAR_a0xh_pGAS[12] 
0x0F12015D,	//TVAR_a0xh_pGAS[13] 
0x0F120115,	//TVAR_a0xh_pGAS[14] 
0x0F1200C9,	//TVAR_a0xh_pGAS[15] 
0x0F120096,	//TVAR_a0xh_pGAS[16] 
0x0F120076,	//TVAR_a0xh_pGAS[17] 
0x0F120064,	//TVAR_a0xh_pGAS[18] 
0x0F12005E,	//TVAR_a0xh_pGAS[19] 
0x0F120063,	//TVAR_a0xh_pGAS[20] 
0x0F120076,	//TVAR_a0xh_pGAS[21] 
0x0F120097,	//TVAR_a0xh_pGAS[22] 
0x0F1200CA,	//TVAR_a0xh_pGAS[23] 
0x0F120117,	//TVAR_a0xh_pGAS[24] 
0x0F120162,	//TVAR_a0xh_pGAS[25] 
0x0F120128,	//TVAR_a0xh_pGAS[26] 
0x0F1200DB,	//TVAR_a0xh_pGAS[27] 
0x0F120095,	//TVAR_a0xh_pGAS[28] 
0x0F120068,	//TVAR_a0xh_pGAS[29] 
0x0F12004B,	//TVAR_a0xh_pGAS[30] 
0x0F12003A,	//TVAR_a0xh_pGAS[31] 
0x0F120034,	//TVAR_a0xh_pGAS[32] 
0x0F12003A,	//TVAR_a0xh_pGAS[33] 
0x0F12004B,	//TVAR_a0xh_pGAS[34] 
0x0F120069,	//TVAR_a0xh_pGAS[35] 
0x0F120096,	//TVAR_a0xh_pGAS[36] 
0x0F1200DD,	//TVAR_a0xh_pGAS[37] 
0x0F12012D,	//TVAR_a0xh_pGAS[38] 
0x0F120102,	//TVAR_a0xh_pGAS[39] 
0x0F1200B7,	//TVAR_a0xh_pGAS[40] 
0x0F120076,	//TVAR_a0xh_pGAS[41] 
0x0F12004C,	//TVAR_a0xh_pGAS[42] 
0x0F120030,	//TVAR_a0xh_pGAS[43] 
0x0F12001D,	//TVAR_a0xh_pGAS[44] 
0x0F120017,	//TVAR_a0xh_pGAS[45] 
0x0F12001D,	//TVAR_a0xh_pGAS[46] 
0x0F120030,	//TVAR_a0xh_pGAS[47] 
0x0F12004D,	//TVAR_a0xh_pGAS[48] 
0x0F120077,	//TVAR_a0xh_pGAS[49] 
0x0F1200B7,	//TVAR_a0xh_pGAS[50] 
0x0F120105,	//TVAR_a0xh_pGAS[51] 
0x0F1200E9,	//TVAR_a0xh_pGAS[52] 
0x0F1200A3,	//TVAR_a0xh_pGAS[53] 
0x0F120066,	//TVAR_a0xh_pGAS[54] 
0x0F12003D,	//TVAR_a0xh_pGAS[55] 
0x0F12001F,	//TVAR_a0xh_pGAS[56] 
0x0F12000B,	//TVAR_a0xh_pGAS[57] 
0x0F120005,	//TVAR_a0xh_pGAS[58] 
0x0F12000C,	//TVAR_a0xh_pGAS[59] 
0x0F120020,	//TVAR_a0xh_pGAS[60] 
0x0F12003E,	//TVAR_a0xh_pGAS[61] 
0x0F120067,	//TVAR_a0xh_pGAS[62] 
0x0F1200A4,	//TVAR_a0xh_pGAS[63] 
0x0F1200EF,	//TVAR_a0xh_pGAS[64] 
0x0F1200E4,	//TVAR_a0xh_pGAS[65] 
0x0F12009E,	//TVAR_a0xh_pGAS[66] 
0x0F120063,	//TVAR_a0xh_pGAS[67] 
0x0F120039,	//TVAR_a0xh_pGAS[68] 
0x0F12001B,	//TVAR_a0xh_pGAS[69] 
0x0F120007,	//TVAR_a0xh_pGAS[70] 
0x0F120000,	//TVAR_a0xh_pGAS[71] 
0x0F120007,	//TVAR_a0xh_pGAS[72] 
0x0F12001C,	//TVAR_a0xh_pGAS[73] 
0x0F12003B,	//TVAR_a0xh_pGAS[74] 
0x0F120064,	//TVAR_a0xh_pGAS[75] 
0x0F1200A1,	//TVAR_a0xh_pGAS[76] 
0x0F1200EA,	//TVAR_a0xh_pGAS[77] 
0x0F1200F0,	//TVAR_a0xh_pGAS[78] 
0x0F1200A9,	//TVAR_a0xh_pGAS[79] 
0x0F12006C,	//TVAR_a0xh_pGAS[80] 
0x0F120042,	//TVAR_a0xh_pGAS[81] 
0x0F120024,	//TVAR_a0xh_pGAS[82] 
0x0F120010,	//TVAR_a0xh_pGAS[83] 
0x0F120009,	//TVAR_a0xh_pGAS[84] 
0x0F120011,	//TVAR_a0xh_pGAS[85] 
0x0F120025,	//TVAR_a0xh_pGAS[86] 
0x0F120044,	//TVAR_a0xh_pGAS[87] 
0x0F12006F,	//TVAR_a0xh_pGAS[88] 
0x0F1200AE,	//TVAR_a0xh_pGAS[89] 
0x0F1200FA,	//TVAR_a0xh_pGAS[90] 
0x0F12010F,	//TVAR_a0xh_pGAS[91] 
0x0F1200C3,	//TVAR_a0xh_pGAS[92] 
0x0F120082,	//TVAR_a0xh_pGAS[93] 
0x0F120057,	//TVAR_a0xh_pGAS[94] 
0x0F120039,	//TVAR_a0xh_pGAS[95] 
0x0F120025,	//TVAR_a0xh_pGAS[96] 
0x0F12001F,	//TVAR_a0xh_pGAS[97] 
0x0F120026,	//TVAR_a0xh_pGAS[98] 
0x0F12003A,	//TVAR_a0xh_pGAS[99] 
0x0F120059,	//TVAR_a0xh_pGAS[100]
0x0F120085,	//TVAR_a0xh_pGAS[101]
0x0F1200C9,	//TVAR_a0xh_pGAS[102]
0x0F12011A,	//TVAR_a0xh_pGAS[103]
0x0F120140,	//TVAR_a0xh_pGAS[104]
0x0F1200EF,	//TVAR_a0xh_pGAS[105]
0x0F1200A7,	//TVAR_a0xh_pGAS[106]
0x0F120077,	//TVAR_a0xh_pGAS[107]
0x0F120058,	//TVAR_a0xh_pGAS[108]
0x0F120046,	//TVAR_a0xh_pGAS[109]
0x0F12003F,	//TVAR_a0xh_pGAS[110]
0x0F120046,	//TVAR_a0xh_pGAS[111]
0x0F12005A,	//TVAR_a0xh_pGAS[112]
0x0F12007A,	//TVAR_a0xh_pGAS[113]
0x0F1200AB,	//TVAR_a0xh_pGAS[114]
0x0F1200F7,	//TVAR_a0xh_pGAS[115]
0x0F12014C,	//TVAR_a0xh_pGAS[116]
0x0F12017D,	//TVAR_a0xh_pGAS[117]
0x0F120130,	//TVAR_a0xh_pGAS[118]
0x0F1200E0,	//TVAR_a0xh_pGAS[119]
0x0F1200A9,	//TVAR_a0xh_pGAS[120]
0x0F120086,	//TVAR_a0xh_pGAS[121]
0x0F120072,	//TVAR_a0xh_pGAS[122]
0x0F12006D,	//TVAR_a0xh_pGAS[123]
0x0F120074,	//TVAR_a0xh_pGAS[124]
0x0F120089,	//TVAR_a0xh_pGAS[125]
0x0F1200AD,	//TVAR_a0xh_pGAS[126]
0x0F1200E5,	//TVAR_a0xh_pGAS[127]
0x0F120137,	//TVAR_a0xh_pGAS[128]
0x0F120187,	//TVAR_a0xh_pGAS[129]
0x0F12019D,	//TVAR_a0xh_pGAS[130]
0x0F12016B,	//TVAR_a0xh_pGAS[131]
0x0F12011C,	//TVAR_a0xh_pGAS[132]
0x0F1200DF,	//TVAR_a0xh_pGAS[133]
0x0F1200B7,	//TVAR_a0xh_pGAS[134]
0x0F1200A1,	//TVAR_a0xh_pGAS[135]
0x0F12009B,	//TVAR_a0xh_pGAS[136]
0x0F1200A2,	//TVAR_a0xh_pGAS[137]
0x0F1200BB,	//TVAR_a0xh_pGAS[138]
0x0F1200E4,	//TVAR_a0xh_pGAS[139]
0x0F120123,	//TVAR_a0xh_pGAS[140]
0x0F120174,	//TVAR_a0xh_pGAS[141]
0x0F1201A5,	//TVAR_a0xh_pGAS[142]
0x0F120148,	//TVAR_a0xh_pGAS[143]
0x0F12011C,	//TVAR_a0xh_pGAS[144]
0x0F1200D9,	//TVAR_a0xh_pGAS[145]
0x0F1200A8,	//TVAR_a0xh_pGAS[146]
0x0F120088,	//TVAR_a0xh_pGAS[147]
0x0F120076,	//TVAR_a0xh_pGAS[148]
0x0F120070,	//TVAR_a0xh_pGAS[149]
0x0F120074,	//TVAR_a0xh_pGAS[150]
0x0F120083,	//TVAR_a0xh_pGAS[151]
0x0F1200A0,	//TVAR_a0xh_pGAS[152]
0x0F1200CE,	//TVAR_a0xh_pGAS[153]
0x0F12010B,	//TVAR_a0xh_pGAS[154]
0x0F120138,	//TVAR_a0xh_pGAS[155]
0x0F120121,	//TVAR_a0xh_pGAS[156]
0x0F1200E0,	//TVAR_a0xh_pGAS[157]
0x0F12009F,	//TVAR_a0xh_pGAS[158]
0x0F120076,	//TVAR_a0xh_pGAS[159]
0x0F12005C,	//TVAR_a0xh_pGAS[160]
0x0F12004D,	//TVAR_a0xh_pGAS[161]
0x0F120049,	//TVAR_a0xh_pGAS[162]
0x0F12004C,	//TVAR_a0xh_pGAS[163]
0x0F120059,	//TVAR_a0xh_pGAS[164]
0x0F120071,	//TVAR_a0xh_pGAS[165]
0x0F120099,	//TVAR_a0xh_pGAS[166]
0x0F1200D7,	//TVAR_a0xh_pGAS[167]
0x0F120114,	//TVAR_a0xh_pGAS[168]
0x0F1200F4,	//TVAR_a0xh_pGAS[169]
0x0F1200B0,	//TVAR_a0xh_pGAS[170]
0x0F120075,	//TVAR_a0xh_pGAS[171]
0x0F120051,	//TVAR_a0xh_pGAS[172]
0x0F12003B,	//TVAR_a0xh_pGAS[173]
0x0F12002D,	//TVAR_a0xh_pGAS[174]
0x0F120029,	//TVAR_a0xh_pGAS[175]
0x0F12002C,	//TVAR_a0xh_pGAS[176]
0x0F120038,	//TVAR_a0xh_pGAS[177]
0x0F12004D,	//TVAR_a0xh_pGAS[178]
0x0F120070,	//TVAR_a0xh_pGAS[179]
0x0F1200A8,	//TVAR_a0xh_pGAS[180]
0x0F1200EB,	//TVAR_a0xh_pGAS[181]
0x0F1200D1,	//TVAR_a0xh_pGAS[182]
0x0F120090,	//TVAR_a0xh_pGAS[183]
0x0F12005B,	//TVAR_a0xh_pGAS[184]
0x0F12003B,	//TVAR_a0xh_pGAS[185]
0x0F120025,	//TVAR_a0xh_pGAS[186]
0x0F120017,	//TVAR_a0xh_pGAS[187]
0x0F120012,	//TVAR_a0xh_pGAS[188]
0x0F120016,	//TVAR_a0xh_pGAS[189]
0x0F120023,	//TVAR_a0xh_pGAS[190]
0x0F120037,	//TVAR_a0xh_pGAS[191]
0x0F120056,	//TVAR_a0xh_pGAS[192]
0x0F12008A,	//TVAR_a0xh_pGAS[193]
0x0F1200C9,	//TVAR_a0xh_pGAS[194]
0x0F1200BC,	//TVAR_a0xh_pGAS[195]
0x0F12007F,	//TVAR_a0xh_pGAS[196]
0x0F12004D,	//TVAR_a0xh_pGAS[197]
0x0F12002D,	//TVAR_a0xh_pGAS[198]
0x0F120017,	//TVAR_a0xh_pGAS[199]
0x0F120008,	//TVAR_a0xh_pGAS[200]
0x0F120003,	//TVAR_a0xh_pGAS[201]
0x0F120008,	//TVAR_a0xh_pGAS[202]
0x0F120016,	//TVAR_a0xh_pGAS[203]
0x0F12002B,	//TVAR_a0xh_pGAS[204]
0x0F120049,	//TVAR_a0xh_pGAS[205]
0x0F120079,	//TVAR_a0xh_pGAS[206]
0x0F1200B7,	//TVAR_a0xh_pGAS[207]
0x0F1200B7,	//TVAR_a0xh_pGAS[208]
0x0F12007B,	//TVAR_a0xh_pGAS[209]
0x0F12004A,	//TVAR_a0xh_pGAS[210]
0x0F12002B,	//TVAR_a0xh_pGAS[211]
0x0F120014,	//TVAR_a0xh_pGAS[212]
0x0F120005,	//TVAR_a0xh_pGAS[213]
0x0F120000,	//TVAR_a0xh_pGAS[214]
0x0F120005,	//TVAR_a0xh_pGAS[215]
0x0F120014,	//TVAR_a0xh_pGAS[216]
0x0F12002A,	//TVAR_a0xh_pGAS[217]
0x0F120049,	//TVAR_a0xh_pGAS[218]
0x0F120078,	//TVAR_a0xh_pGAS[219]
0x0F1200B4,	//TVAR_a0xh_pGAS[220]
0x0F1200C1,	//TVAR_a0xh_pGAS[221]
0x0F120084,	//TVAR_a0xh_pGAS[222]
0x0F120052,	//TVAR_a0xh_pGAS[223]
0x0F120032,	//TVAR_a0xh_pGAS[224]
0x0F12001C,	//TVAR_a0xh_pGAS[225]
0x0F12000D,	//TVAR_a0xh_pGAS[226]
0x0F120008,	//TVAR_a0xh_pGAS[227]
0x0F12000D,	//TVAR_a0xh_pGAS[228]
0x0F12001C,	//TVAR_a0xh_pGAS[229]
0x0F120033,	//TVAR_a0xh_pGAS[230]
0x0F120052,	//TVAR_a0xh_pGAS[231]
0x0F120083,	//TVAR_a0xh_pGAS[232]
0x0F1200C1,	//TVAR_a0xh_pGAS[233]
0x0F1200DA,	//TVAR_a0xh_pGAS[234]
0x0F120099,	//TVAR_a0xh_pGAS[235]
0x0F120064,	//TVAR_a0xh_pGAS[236]
0x0F120043,	//TVAR_a0xh_pGAS[237]
0x0F12002D,	//TVAR_a0xh_pGAS[238]
0x0F12001F,	//TVAR_a0xh_pGAS[239]
0x0F12001A,	//TVAR_a0xh_pGAS[240]
0x0F12001F,	//TVAR_a0xh_pGAS[241]
0x0F12002E,	//TVAR_a0xh_pGAS[242]
0x0F120045,	//TVAR_a0xh_pGAS[243]
0x0F120065,	//TVAR_a0xh_pGAS[244]
0x0F12009A,	//TVAR_a0xh_pGAS[245]
0x0F1200DC,	//TVAR_a0xh_pGAS[246]
0x0F120103,	//TVAR_a0xh_pGAS[247]
0x0F1200BF,	//TVAR_a0xh_pGAS[248]
0x0F120083,	//TVAR_a0xh_pGAS[249]
0x0F12005E,	//TVAR_a0xh_pGAS[250]
0x0F120047,	//TVAR_a0xh_pGAS[251]
0x0F12003A,	//TVAR_a0xh_pGAS[252]
0x0F120035,	//TVAR_a0xh_pGAS[253]
0x0F12003A,	//TVAR_a0xh_pGAS[254]
0x0F120048,	//TVAR_a0xh_pGAS[255]
0x0F120060,	//TVAR_a0xh_pGAS[256]
0x0F120085,	//TVAR_a0xh_pGAS[257]
0x0F1200C0,	//TVAR_a0xh_pGAS[258]
0x0F120107,	//TVAR_a0xh_pGAS[259]
0x0F120137,	//TVAR_a0xh_pGAS[260]
0x0F1200F5,	//TVAR_a0xh_pGAS[261]
0x0F1200B3,	//TVAR_a0xh_pGAS[262]
0x0F120087,	//TVAR_a0xh_pGAS[263]
0x0F12006C,	//TVAR_a0xh_pGAS[264]
0x0F12005E,	//TVAR_a0xh_pGAS[265]
0x0F12005A,	//TVAR_a0xh_pGAS[266]
0x0F12005F,	//TVAR_a0xh_pGAS[267]
0x0F12006E,	//TVAR_a0xh_pGAS[268]
0x0F12008A,	//TVAR_a0xh_pGAS[269]
0x0F1200B5,	//TVAR_a0xh_pGAS[270]
0x0F1200F7,	//TVAR_a0xh_pGAS[271]
0x0F120139,	//TVAR_a0xh_pGAS[272]
0x0F120157,	//TVAR_a0xh_pGAS[273]
0x0F120129,	//TVAR_a0xh_pGAS[274]
0x0F1200E5,	//TVAR_a0xh_pGAS[275]
0x0F1200B5,	//TVAR_a0xh_pGAS[276]
0x0F120094,	//TVAR_a0xh_pGAS[277]
0x0F120083,	//TVAR_a0xh_pGAS[278]
0x0F12007E,	//TVAR_a0xh_pGAS[279]
0x0F120084,	//TVAR_a0xh_pGAS[280]
0x0F120098,	//TVAR_a0xh_pGAS[281]
0x0F1200B8,	//TVAR_a0xh_pGAS[282]
0x0F1200EA,	//TVAR_a0xh_pGAS[283]
0x0F12012B,	//TVAR_a0xh_pGAS[284]
0x0F120156,	//TVAR_a0xh_pGAS[285]
0x0F120156,	//TVAR_a0xh_pGAS[286]
0x0F120126,	//TVAR_a0xh_pGAS[287]
0x0F1200E0,	//TVAR_a0xh_pGAS[288]
0x0F1200AA,	//TVAR_a0xh_pGAS[289]
0x0F120089,	//TVAR_a0xh_pGAS[290]
0x0F120076,	//TVAR_a0xh_pGAS[291]
0x0F120070,	//TVAR_a0xh_pGAS[292]
0x0F120077,	//TVAR_a0xh_pGAS[293]
0x0F12008B,	//TVAR_a0xh_pGAS[294]
0x0F1200AE,	//TVAR_a0xh_pGAS[295]
0x0F1200E3,	//TVAR_a0xh_pGAS[296]
0x0F12012B,	//TVAR_a0xh_pGAS[297]
0x0F12015E,	//TVAR_a0xh_pGAS[298]
0x0F120130,	//TVAR_a0xh_pGAS[299]
0x0F1200EB,	//TVAR_a0xh_pGAS[300]
0x0F1200A7,	//TVAR_a0xh_pGAS[301]
0x0F12007A,	//TVAR_a0xh_pGAS[302]
0x0F12005E,	//TVAR_a0xh_pGAS[303]
0x0F12004E,	//TVAR_a0xh_pGAS[304]
0x0F12004A,	//TVAR_a0xh_pGAS[305]
0x0F12004F,	//TVAR_a0xh_pGAS[306]
0x0F120060,	//TVAR_a0xh_pGAS[307]
0x0F12007D,	//TVAR_a0xh_pGAS[308]
0x0F1200AB,	//TVAR_a0xh_pGAS[309]
0x0F1200F2,	//TVAR_a0xh_pGAS[310]
0x0F120137,	//TVAR_a0xh_pGAS[311]
0x0F120103,	//TVAR_a0xh_pGAS[312]
0x0F1200BB,	//TVAR_a0xh_pGAS[313]
0x0F12007D,	//TVAR_a0xh_pGAS[314]
0x0F120056,	//TVAR_a0xh_pGAS[315]
0x0F12003E,	//TVAR_a0xh_pGAS[316]
0x0F12002F,	//TVAR_a0xh_pGAS[317]
0x0F12002A,	//TVAR_a0xh_pGAS[318]
0x0F12002F,	//TVAR_a0xh_pGAS[319]
0x0F12003F,	//TVAR_a0xh_pGAS[320]
0x0F120058,	//TVAR_a0xh_pGAS[321]
0x0F12007F,	//TVAR_a0xh_pGAS[322]
0x0F1200BD,	//TVAR_a0xh_pGAS[323]
0x0F120106,	//TVAR_a0xh_pGAS[324]
0x0F1200E0,	//TVAR_a0xh_pGAS[325]
0x0F12009C,	//TVAR_a0xh_pGAS[326]
0x0F120064,	//TVAR_a0xh_pGAS[327]
0x0F120041,	//TVAR_a0xh_pGAS[328]
0x0F120028,	//TVAR_a0xh_pGAS[329]
0x0F120019,	//TVAR_a0xh_pGAS[330]
0x0F120013,	//TVAR_a0xh_pGAS[331]
0x0F120018,	//TVAR_a0xh_pGAS[332]
0x0F120028,	//TVAR_a0xh_pGAS[333]
0x0F120040,	//TVAR_a0xh_pGAS[334]
0x0F120063,	//TVAR_a0xh_pGAS[335]
0x0F12009A,	//TVAR_a0xh_pGAS[336]
0x0F1200DE,	//TVAR_a0xh_pGAS[337]
0x0F1200CB,	//TVAR_a0xh_pGAS[338]
0x0F12008B,	//TVAR_a0xh_pGAS[339]
0x0F120056,	//TVAR_a0xh_pGAS[340]
0x0F120034,	//TVAR_a0xh_pGAS[341]
0x0F12001B,	//TVAR_a0xh_pGAS[342]
0x0F12000A,	//TVAR_a0xh_pGAS[343]
0x0F120004,	//TVAR_a0xh_pGAS[344]
0x0F120009,	//TVAR_a0xh_pGAS[345]
0x0F120019,	//TVAR_a0xh_pGAS[346]
0x0F120031,	//TVAR_a0xh_pGAS[347]
0x0F120052,	//TVAR_a0xh_pGAS[348]
0x0F120085,	//TVAR_a0xh_pGAS[349]
0x0F1200C5,	//TVAR_a0xh_pGAS[350]
0x0F1200C6,	//TVAR_a0xh_pGAS[351]
0x0F120087,	//TVAR_a0xh_pGAS[352]
0x0F120054,	//TVAR_a0xh_pGAS[353]
0x0F120031,	//TVAR_a0xh_pGAS[354]
0x0F120017,	//TVAR_a0xh_pGAS[355]
0x0F120006,	//TVAR_a0xh_pGAS[356]
0x0F120000,	//TVAR_a0xh_pGAS[357]
0x0F120005,	//TVAR_a0xh_pGAS[358]
0x0F120014,	//TVAR_a0xh_pGAS[359]
0x0F12002B,	//TVAR_a0xh_pGAS[360]
0x0F12004B,	//TVAR_a0xh_pGAS[361]
0x0F12007C,	//TVAR_a0xh_pGAS[362]
0x0F1200BA,	//TVAR_a0xh_pGAS[363]
0x0F1200D1,	//TVAR_a0xh_pGAS[364]
0x0F120090,	//TVAR_a0xh_pGAS[365]
0x0F12005B,	//TVAR_a0xh_pGAS[366]
0x0F120038,	//TVAR_a0xh_pGAS[367]
0x0F12001F,	//TVAR_a0xh_pGAS[368]
0x0F12000D,	//TVAR_a0xh_pGAS[369]
0x0F120007,	//TVAR_a0xh_pGAS[370]
0x0F12000B,	//TVAR_a0xh_pGAS[371]
0x0F120019,	//TVAR_a0xh_pGAS[372]
0x0F120030,	//TVAR_a0xh_pGAS[373]
0x0F12004F,	//TVAR_a0xh_pGAS[374]
0x0F120080,	//TVAR_a0xh_pGAS[375]
0x0F1200C0,	//TVAR_a0xh_pGAS[376]
0x0F1200E9,	//TVAR_a0xh_pGAS[377]
0x0F1200A5,	//TVAR_a0xh_pGAS[378]
0x0F12006D,	//TVAR_a0xh_pGAS[379]
0x0F120048,	//TVAR_a0xh_pGAS[380]
0x0F12002F,	//TVAR_a0xh_pGAS[381]
0x0F12001E,	//TVAR_a0xh_pGAS[382]
0x0F120017,	//TVAR_a0xh_pGAS[383]
0x0F12001A,	//TVAR_a0xh_pGAS[384]
0x0F120027,	//TVAR_a0xh_pGAS[385]
0x0F12003D,	//TVAR_a0xh_pGAS[386]
0x0F12005D,	//TVAR_a0xh_pGAS[387]
0x0F120092,	//TVAR_a0xh_pGAS[388]
0x0F1200D4,	//TVAR_a0xh_pGAS[389]
0x0F120112,	//TVAR_a0xh_pGAS[390]
0x0F1200CB,	//TVAR_a0xh_pGAS[391]
0x0F12008C,	//TVAR_a0xh_pGAS[392]
0x0F120063,	//TVAR_a0xh_pGAS[393]
0x0F120049,	//TVAR_a0xh_pGAS[394]
0x0F120038,	//TVAR_a0xh_pGAS[395]
0x0F120031,	//TVAR_a0xh_pGAS[396]
0x0F120033,	//TVAR_a0xh_pGAS[397]
0x0F12003F,	//TVAR_a0xh_pGAS[398]
0x0F120055,	//TVAR_a0xh_pGAS[399]
0x0F120079,	//TVAR_a0xh_pGAS[400]
0x0F1200B3,	//TVAR_a0xh_pGAS[401]
0x0F1200F8,	//TVAR_a0xh_pGAS[402]
0x0F120148,	//TVAR_a0xh_pGAS[403]
0x0F120102,	//TVAR_a0xh_pGAS[404]
0x0F1200BC,	//TVAR_a0xh_pGAS[405]
0x0F12008D,	//TVAR_a0xh_pGAS[406]
0x0F12006E,	//TVAR_a0xh_pGAS[407]
0x0F12005C,	//TVAR_a0xh_pGAS[408]
0x0F120055,	//TVAR_a0xh_pGAS[409]
0x0F120058,	//TVAR_a0xh_pGAS[410]
0x0F120064,	//TVAR_a0xh_pGAS[411]
0x0F12007E,	//TVAR_a0xh_pGAS[412]
0x0F1200A6,	//TVAR_a0xh_pGAS[413]
0x0F1200E5,	//TVAR_a0xh_pGAS[414]
0x0F120123,	//TVAR_a0xh_pGAS[415]
0x0F120166,	//TVAR_a0xh_pGAS[416]
0x0F120136,	//TVAR_a0xh_pGAS[417]
0x0F1200F1,	//TVAR_a0xh_pGAS[418]
0x0F1200BB,	//TVAR_a0xh_pGAS[419]
0x0F120097,	//TVAR_a0xh_pGAS[420]
0x0F120083,	//TVAR_a0xh_pGAS[421]
0x0F12007A,	//TVAR_a0xh_pGAS[422]
0x0F12007D,	//TVAR_a0xh_pGAS[423]
0x0F12008D,	//TVAR_a0xh_pGAS[424]
0x0F1200AB,	//TVAR_a0xh_pGAS[425]
0x0F1200D8,	//TVAR_a0xh_pGAS[426]
0x0F120115,	//TVAR_a0xh_pGAS[427]
0x0F12013F,	//TVAR_a0xh_pGAS[428]
0x0F120104,	//TVAR_a0xh_pGAS[429]
0x0F1200E4,	//TVAR_a0xh_pGAS[430]
0x0F1200AD,	//TVAR_a0xh_pGAS[431]
0x0F120085,	//TVAR_a0xh_pGAS[432]
0x0F12006B,	//TVAR_a0xh_pGAS[433]
0x0F12005E,	//TVAR_a0xh_pGAS[434]
0x0F12005B,	//TVAR_a0xh_pGAS[435]
0x0F120062,	//TVAR_a0xh_pGAS[436]
0x0F120073,	//TVAR_a0xh_pGAS[437]
0x0F120091,	//TVAR_a0xh_pGAS[438]
0x0F1200BF,	//TVAR_a0xh_pGAS[439]
0x0F1200FE,	//TVAR_a0xh_pGAS[440]
0x0F120126,	//TVAR_a0xh_pGAS[441]
0x0F1200E6,	//TVAR_a0xh_pGAS[442]
0x0F1200B3,	//TVAR_a0xh_pGAS[443]
0x0F12007E,	//TVAR_a0xh_pGAS[444]
0x0F12005C,	//TVAR_a0xh_pGAS[445]
0x0F120048,	//TVAR_a0xh_pGAS[446]
0x0F12003E,	//TVAR_a0xh_pGAS[447]
0x0F12003C,	//TVAR_a0xh_pGAS[448]
0x0F120042,	//TVAR_a0xh_pGAS[449]
0x0F120051,	//TVAR_a0xh_pGAS[450]
0x0F12006A,	//TVAR_a0xh_pGAS[451]
0x0F120091,	//TVAR_a0xh_pGAS[452]
0x0F1200CE,	//TVAR_a0xh_pGAS[453]
0x0F120107,	//TVAR_a0xh_pGAS[454]
0x0F1200C0,	//TVAR_a0xh_pGAS[455]
0x0F120089,	//TVAR_a0xh_pGAS[456]
0x0F12005A,	//TVAR_a0xh_pGAS[457]
0x0F12003F,	//TVAR_a0xh_pGAS[458]
0x0F12002E,	//TVAR_a0xh_pGAS[459]
0x0F120024,	//TVAR_a0xh_pGAS[460]
0x0F120022,	//TVAR_a0xh_pGAS[461]
0x0F120028,	//TVAR_a0xh_pGAS[462]
0x0F120035,	//TVAR_a0xh_pGAS[463]
0x0F12004A,	//TVAR_a0xh_pGAS[464]
0x0F12006A,	//TVAR_a0xh_pGAS[465]
0x0F12009F,	//TVAR_a0xh_pGAS[466]
0x0F1200DB,	//TVAR_a0xh_pGAS[467]
0x0F1200A2,	//TVAR_a0xh_pGAS[468]
0x0F12006F,	//TVAR_a0xh_pGAS[469]
0x0F120045,	//TVAR_a0xh_pGAS[470]
0x0F12002D,	//TVAR_a0xh_pGAS[471]
0x0F12001C,	//TVAR_a0xh_pGAS[472]
0x0F120012,	//TVAR_a0xh_pGAS[473]
0x0F120010,	//TVAR_a0xh_pGAS[474]
0x0F120015,	//TVAR_a0xh_pGAS[475]
0x0F120022,	//TVAR_a0xh_pGAS[476]
0x0F120035,	//TVAR_a0xh_pGAS[477]
0x0F120050,	//TVAR_a0xh_pGAS[478]
0x0F12007E,	//TVAR_a0xh_pGAS[479]
0x0F1200B6,	//TVAR_a0xh_pGAS[480]
0x0F120090,	//TVAR_a0xh_pGAS[481]
0x0F120061,	//TVAR_a0xh_pGAS[482]
0x0F12003A,	//TVAR_a0xh_pGAS[483]
0x0F120022,	//TVAR_a0xh_pGAS[484]
0x0F120012,	//TVAR_a0xh_pGAS[485]
0x0F120007,	//TVAR_a0xh_pGAS[486]
0x0F120003,	//TVAR_a0xh_pGAS[487]
0x0F120008,	//TVAR_a0xh_pGAS[488]
0x0F120015,	//TVAR_a0xh_pGAS[489]
0x0F120027,	//TVAR_a0xh_pGAS[490]
0x0F120040,	//TVAR_a0xh_pGAS[491]
0x0F120069,	//TVAR_a0xh_pGAS[492]
0x0F12009D,	//TVAR_a0xh_pGAS[493]
0x0F12008C,	//TVAR_a0xh_pGAS[494]
0x0F12005E,	//TVAR_a0xh_pGAS[495]
0x0F120039,	//TVAR_a0xh_pGAS[496]
0x0F120021,	//TVAR_a0xh_pGAS[497]
0x0F120010,	//TVAR_a0xh_pGAS[498]
0x0F120004,	//TVAR_a0xh_pGAS[499]
0x0F120000,	//TVAR_a0xh_pGAS[500]
0x0F120005,	//TVAR_a0xh_pGAS[501]
0x0F120010,	//TVAR_a0xh_pGAS[502]
0x0F120022,	//TVAR_a0xh_pGAS[503]
0x0F12003A,	//TVAR_a0xh_pGAS[504]
0x0F120061,	//TVAR_a0xh_pGAS[505]
0x0F120093,	//TVAR_a0xh_pGAS[506]
0x0F120096,	//TVAR_a0xh_pGAS[507]
0x0F120067,	//TVAR_a0xh_pGAS[508]
0x0F120040,	//TVAR_a0xh_pGAS[509]
0x0F120028,	//TVAR_a0xh_pGAS[510]
0x0F120017,	//TVAR_a0xh_pGAS[511]
0x0F12000B,	//TVAR_a0xh_pGAS[512]
0x0F120006,	//TVAR_a0xh_pGAS[513]
0x0F12000A,	//TVAR_a0xh_pGAS[514]
0x0F120014,	//TVAR_a0xh_pGAS[515]
0x0F120025,	//TVAR_a0xh_pGAS[516]
0x0F12003C,	//TVAR_a0xh_pGAS[517]
0x0F120064,	//TVAR_a0xh_pGAS[518]
0x0F120097,	//TVAR_a0xh_pGAS[519]
0x0F1200AC,	//TVAR_a0xh_pGAS[520]
0x0F12007A,	//TVAR_a0xh_pGAS[521]
0x0F120050,	//TVAR_a0xh_pGAS[522]
0x0F120036,	//TVAR_a0xh_pGAS[523]
0x0F120025,	//TVAR_a0xh_pGAS[524]
0x0F120019,	//TVAR_a0xh_pGAS[525]
0x0F120015,	//TVAR_a0xh_pGAS[526]
0x0F120017,	//TVAR_a0xh_pGAS[527]
0x0F120021,	//TVAR_a0xh_pGAS[528]
0x0F120030,	//TVAR_a0xh_pGAS[529]
0x0F120049,	//TVAR_a0xh_pGAS[530]
0x0F120073,	//TVAR_a0xh_pGAS[531]
0x0F1200A8,	//TVAR_a0xh_pGAS[532]
0x0F1200D2,	//TVAR_a0xh_pGAS[533]
0x0F12009B,	//TVAR_a0xh_pGAS[534]
0x0F12006B,	//TVAR_a0xh_pGAS[535]
0x0F12004E,	//TVAR_a0xh_pGAS[536]
0x0F12003C,	//TVAR_a0xh_pGAS[537]
0x0F120031,	//TVAR_a0xh_pGAS[538]
0x0F12002B,	//TVAR_a0xh_pGAS[539]
0x0F12002E,	//TVAR_a0xh_pGAS[540]
0x0F120036,	//TVAR_a0xh_pGAS[541]
0x0F120046,	//TVAR_a0xh_pGAS[542]
0x0F120062,	//TVAR_a0xh_pGAS[543]
0x0F120092,	//TVAR_a0xh_pGAS[544]
0x0F1200C8,	//TVAR_a0xh_pGAS[545]
0x0F120102,	//TVAR_a0xh_pGAS[546]
0x0F1200CE,	//TVAR_a0xh_pGAS[547]
0x0F120097,	//TVAR_a0xh_pGAS[548]
0x0F120073,	//TVAR_a0xh_pGAS[549]
0x0F12005D,	//TVAR_a0xh_pGAS[550]
0x0F120050,	//TVAR_a0xh_pGAS[551]
0x0F12004C,	//TVAR_a0xh_pGAS[552]
0x0F12004E,	//TVAR_a0xh_pGAS[553]
0x0F120057,	//TVAR_a0xh_pGAS[554]
0x0F12006A,	//TVAR_a0xh_pGAS[555]
0x0F12008B,	//TVAR_a0xh_pGAS[556]
0x0F1200BF,	//TVAR_a0xh_pGAS[557]
0x0F1200F0,	//TVAR_a0xh_pGAS[558]
0x0F12011C,	//TVAR_a0xh_pGAS[559]
0x0F1200F9,	//TVAR_a0xh_pGAS[560]
0x0F1200C1,	//TVAR_a0xh_pGAS[561]
0x0F120098,	//TVAR_a0xh_pGAS[562]
0x0F12007D,	//TVAR_a0xh_pGAS[563]
0x0F12006E,	//TVAR_a0xh_pGAS[564]
0x0F120069,	//TVAR_a0xh_pGAS[565]
0x0F12006C,	//TVAR_a0xh_pGAS[566]
0x0F120079,	//TVAR_a0xh_pGAS[567]
0x0F120090,	//TVAR_a0xh_pGAS[568]
0x0F1200B5,	//TVAR_a0xh_pGAS[569]
0x0F1200E7,	//TVAR_a0xh_pGAS[570]
0x0F120105,	//TVAR_a0xh_pGAS[571]

//	param_,0xtart	TVAR_a0xh_AwbA0xhCord
0x002A0704,
0x0F1200ED,	//TVAR_a0xh_AwbA0xhCord[0]
0x0F120124,	//TVAR_a0xh_AwbA0xhCord[1]
0x0F12012B,	//TVAR_a0xh_AwbA0xhCord[2]
0x0F12014A,	//TVAR_a0xh_AwbA0xhCord[3]
0x0F120190,	//TVAR_a0xh_AwbA0xhCord[4]
0x0F1201B2,	//TVAR_a0xh_AwbA0xhCord[5]
0x0F1201C4,	//TVAR_a0xh_AwbA0xhCord[6]

  
//	param_,start	TVAR_ash_GASAlpha
0x002A0712,
0x0F12012B,	//TVAR_a0xh_GASAlpha[0]
0x0F120106,	//TVAR_a0xh_GASAlpha[1]
0x0F12010A,	//TVAR_a0xh_GASAlpha[2]
0x0F1200EE,	//TVAR_a0xh_GASAlpha[3]
0x0F12011b,	//TVAR_a0xh_GASAlpha[4]
0x0F1200FC,	//TVAR_a0xh_GASAlpha[5]
0x0F1200FE,	//TVAR_a0xh_GASAlpha[6]
0x0F120100,	//TVAR_a0xh_GASAlpha[7]
0x0F12011B,	//TVAR_a0xh_GASAlpha[8]
0x0F120107,	//TVAR_a0xh_GASAlpha[9]
0x0F120109,	//TVAR_a0xh_GASAlpha[10]
0x0F1200FF,	//TVAR_a0xh_GASAlpha[11]
0x0F1200DB,	//TVAR_a0xh_GASAlpha[12]
0x0F1200FF,	//TVAR_a0xh_GASAlpha[13]
0x0F120100,	//TVAR_a0xh_GASAlpha[14]
0x0F1200FB,	//TVAR_a0xh_GASAlpha[15]
0x0F120100,	//TVAR_a0xh_GASAlpha[16]
0x0F120103,	//TVAR_a0xh_GASAlpha[17]
0x0F120101,	//TVAR_a0xh_GASAlpha[18]
0x0F12010C,	//TVAR_a0xh_GASAlpha[19]
0x0F120110,	//TVAR_a0xh_GASAlpha[20]
0x0F1200FD,	//TVAR_a0xh_GASAlpha[21]
0x0F1200FD,	//TVAR_a0xh_GASAlpha[22]
0x0F120109,	//TVAR_a0xh_GASAlpha[23]
0x0F1200D4,	//TVAR_a0xh_GASAlpha[24]
0x0F1200FB,	//TVAR_a0xh_GASAlpha[25]
0x0F1200F8,	//TVAR_a0xh_GASAlpha[26]
0x0F120107,	//TVAR_a0xh_GASAlpha[27]
//	param_,end	TVAR_ash_GASAlpha
  
//	param_,0xtart	TVAR_ash_GASOutdoorAlpha
0x0F12010a,	//TVAR_a0xh_GASOutdoorAlpha[0]
0x0F120100,	//TVAR_a0xh_GASOutdoorAlpha[1]
0x0F120100,	//TVAR_a0xh_GASOutdoorAlpha[2]
0x0F1200FC,	//TVAR_a0xh_GASOutdoorAlpha[3]
//	param_,end	TVAR_ash_GASOutdoorAlpha
  
//0x002A075A,
//0x0F120000,
//0x0F120280,
//0x0F120200,
//0x0F12000E,
//0x0F12000F,

//Gamma      //                              
//////////////////////////                      
                                                
//	param_start	SARR_usGammaLutRGBIndoor        
0x002A04C8,                                       
0x0F120000,                                       
0x0F120001,                                       
0x0F120007,                                       
0x0F120010,                                       
0x0F120066,                                       
0x0F1200F1,                                       
0x0F120154,                                       
0x0F120190,                                       
0x0F1201E4,                                       
0x0F12022C,                                       
0x0F1202AC,                                       
0x0F12030C,                                       
0x0F12035C,                                       
0x0F1203A4,                                       
0x0F1203D2,                                       
0x0F1203E8,                                       
                                                
0x0F120000,                                       
0x0F120001,                                       
0x0F120007,                                       
0x0F120010,                                       
0x0F120066,                                       
0x0F1200F1,                                       
0x0F120154,                                       
0x0F120190,                                       
0x0F1201E4,                                       
0x0F12022C,                                       
0x0F1202AC,                                       
0x0F12030C,                                       
0x0F12035C,                                       
0x0F1203A4,                                       
0x0F1203D2,                                       
0x0F1203E8,                                       
                                                
0x0F120000,                                       
0x0F120001,                                       
0x0F120007,                                       
0x0F120010,                                       
0x0F120066,                                       
0x0F1200F1,                                       
0x0F120154,                                       
0x0F120190,                                       
0x0F1201E4,                                       
0x0F12022C,                                       
0x0F1202AC,                                       
0x0F12030C,                                       
0x0F12035C,                                       
0x0F1203A4,                                       
0x0F1203D2,                                       
0x0F1203E8,                                       
//	param_end	SARR_usGammaLutRGBIndoor          

//AWB 
//////////////////////////

//	param_start	awbb_IndoorGrZones_m_BGrid
0x002A0C50,
0x0F1203B8,  //03B8  //03BA     
0x0F1203C8,  //03C8  //03E3    
0x0F120384,  //0384  //039A    
0x0F1203D0,  //03D0  //03DB    
0x0F12035E,  //035E  //037B    
0x0F1203CC,  //03CC  //03CD    
0x0F12033E,  //033E  //035D    
0x0F1203B2,  //03B2  //03B2    
0x0F120322,  //0322  //0342    
0x0F120396,  //0396  //0397    
0x0F12030C,  //030C  //0325    
0x0F120380,  //0380  //0380    
0x0F1202F8,  //02F8  //030E    
0x0F120368,  //0368  //0369    
0x0F1202DC,  //02DC  //02F8    
0x0F120352,  //034A  //034B    
0x0F1202C2,  //02C2  //02DE    
0x0F12033C,  //0336  //0336    
0x0F1202AE,  //02AE  //02BF    
0x0F12032A,  //031E  //031F    
0x0F12029A,  //029A  //02A6    
0x0F12031C,  //0306  //0306    
0x0F12028C,  //028C  //028D    
0x0F12030A,  //02F4  //02F4    
0x0F12027C,  //027C  //027D    
0x0F1202FC,  //02DC  //02DD    
0x0F120264,  //0264  //026C    
0x0F1202EC,  //02C2  //02C2    
0x0F120252,  //0252  //025C    
0x0F1202DE,  //02AE  //02AE    
0x0F120246,  //0246  //024F    
0x0F1202CC,  //029C  //029D    
0x0F12023C,  //023C  //0245    
0x0F1202C2,  //028A  //028B    
0x0F12022E,  //022E  //023E    
0x0F1202B4,  //027E  //027F    
0x0F120222,  //0222  //0235    
0x0F1202A8,  //0272  //0272    
0x0F120212,  //0212  //022B    
0x0F12029C,  //0266  //0267    
0x0F120202,  //0202  //0220    
0x0F120292,  //025A  //025B    
0x0F1201FA,  //01FA  //0218    
0x0F120288,  //0250  //0250    
0x0F1201EC,  //01EC  //020E    
0x0F12027E,  //0246  //0246    
0x0F1201E6,  //01E6  //0206    
0x0F120272,  //023C  //023D    
0x0F1201DC,  //01DC  //01FB    
0x0F120264,  //0234  //0234    
0x0F1201D4,  //01D4  //01F1    
0x0F120256,  //0228  //0229    
0x0F1201CE,  //01CE  //01E7    
0x0F120248,  //0220  //0220    
0x0F1201C6,  //01C6  //01DF    
0x0F12023E,  //0216  //0216    
0x0F1201C0,  //01C0  //01D2    
0x0F12022E,  //020C  //020D    
0x0F1201BE,  //01BE  //01C7    
0x0F120222,  //01FC  //01FD    
0x0F1201C4,  //01C4  //01C4    
0x0F12020E,  //01EE  //01EE    
0x0F1201D0,  //01D0  //01D1     
0x0F1201E0,  //01E0  //01E1      
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
//	param_end	awbb_IndoorGrZones_m_BGrid
0x0F120004,  //#awbb_IndoorGrZones_m_GridStep
0x0F120000,              
0x002A0CF8,              
0x0F12010F,  //#awbb_IndoorGrZones_m_Boffs
0x0F120000,              
//	param_start	awbb_LowBrGrZones_m_BGrid
0x002A0D84,              
0x0F120406,              
0x0F120467,              
0x0F120371,              
0x0F1204B0,              
0x0F1202E5,              
0x0F120481,              
0x0F120298,              
0x0F12042E,              
0x0F120260,              
0x0F1203DE,              
0x0F12022F,              
0x0F120391,              
0x0F120201,              
0x0F12034D,              
0x0F1201DA,              
0x0F120310,              
0x0F1201B3,              
0x0F1202D4,              
0x0F12018F,              
0x0F120297,              
0x0F120181,              
0x0F120271,              
0x0F120181,              
0x0F12022A,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
0x0F120000,              
//	param_end	awbb_LowBrGrZones_m_BGrid
0x0F120006,  //#awbb_LowBrGrZones_m_GridStep
0x0F120000,              
0x002A0DF0,              
0x0F120081,  //#awbb_LowBrGrZones_m_Boffs
0x0F120000,              
//	param_start	awbb_OutdoorGrZones_m_BGrid
0x002A0D08,              
0x0F120256, //awbb_OutdoorGrZones_m_BGrid[0] 
0x0F12026C, //awbb_OutdoorGrZones_m_BGrid[1] 
0x0F120240, //awbb_OutdoorGrZones_m_BGrid[2] 
0x0F120278, //awbb_OutdoorGrZones_m_BGrid[3] 
0x0F120234, //awbb_OutdoorGrZones_m_BGrid[4] 
0x0F120282, //awbb_OutdoorGrZones_m_BGrid[5] 
0x0F12022A, //awbb_OutdoorGrZones_m_BGrid[6] 
0x0F12028A, //awbb_OutdoorGrZones_m_BGrid[7] 
0x0F120220, //awbb_OutdoorGrZones_m_BGrid[8] 
0x0F12028E, //awbb_OutdoorGrZones_m_BGrid[9] 
0x0F120218, //awbb_OutdoorGrZones_m_BGrid[10]
0x0F120292, //awbb_OutdoorGrZones_m_BGrid[11]
0x0F120210, //awbb_OutdoorGrZones_m_BGrid[12]
0x0F120294, //awbb_OutdoorGrZones_m_BGrid[13]
0x0F12020C, //awbb_OutdoorGrZones_m_BGrid[14]
0x0F120294, //awbb_OutdoorGrZones_m_BGrid[15]
0x0F120208, //awbb_OutdoorGrZones_m_BGrid[16]
0x0F120292, //awbb_OutdoorGrZones_m_BGrid[17]
0x0F120206, //awbb_OutdoorGrZones_m_BGrid[18]
0x0F12028E, //awbb_OutdoorGrZones_m_BGrid[19]
0x0F120204, //awbb_OutdoorGrZones_m_BGrid[20]
0x0F120288, //awbb_OutdoorGrZones_m_BGrid[21]
0x0F120202, //awbb_OutdoorGrZones_m_BGrid[22]
0x0F120284, //awbb_OutdoorGrZones_m_BGrid[23]
0x0F120202, //awbb_OutdoorGrZones_m_BGrid[24]
0x0F12027E, //awbb_OutdoorGrZones_m_BGrid[25]
0x0F120202, //awbb_OutdoorGrZones_m_BGrid[26]
0x0F120276, //awbb_OutdoorGrZones_m_BGrid[27]
0x0F120202, //awbb_OutdoorGrZones_m_BGrid[28]
0x0F12026E, //awbb_OutdoorGrZones_m_BGrid[29]
0x0F120204, //awbb_OutdoorGrZones_m_BGrid[30]
0x0F120268, //awbb_OutdoorGrZones_m_BGrid[31]
0x0F12020A, //awbb_OutdoorGrZones_m_BGrid[32]
0x0F120260, //awbb_OutdoorGrZones_m_BGrid[33]
0x0F120214,	//awbb_OutdoorGrZones_m_BGrid[34]
0x0F120254,	//awbb_OutdoorGrZones_m_BGrid[35]
0x0F12022A,	//awbb_OutdoorGrZones_m_BGrid[36]
0x0F120238,	//awbb_OutdoorGrZones_m_BGrid[37]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[38]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[39]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[40]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[41]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[42]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[43]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[44]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[45]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[46]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[47]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[48]
0x0F120000,	//awbb_OutdoorGrZones_m_BGrid[49]
//	param_end	awbb_OutdoorGrZones_m_BGrid
0x0F120003,  //#awbb_OutdoorGrZones_m_GridStep
0x0F120000,

0x002A0D70,          
0x0F120013, //0011,//awbb_OutdoorGrZones_m_GridSz

0x002A0D74,
0x0F12021e, //2F, //awbb_OutdoorGrZones_m_Boffs
0x0F120000,
0x002A0E00,
0x0F12034A, //awbb_CrclLowT_R_c
0x0F120000,
0x0F120176,  //awbb_CrclLowT_B_c
0x0F120000,
0x0F1271B8, //awbb_CrclLowT_Rad_c
0x0F120000,
0x002A0E1A,
0x0F12012F,
0x0F120120,

// awbb_LowTempRB
0x002A0E68,
0x0F1204F2,
          
0x002A0D78,
0x0F120020,	//AWB min.
          
0x002A0D80,
0x0F1200E0,	//AWB Max.
          
0x002A0E40, //awbb_Use_Filters
0x0F120061,	//AWB option
          
0x002A0EE4,
0x0F120003,  //awbb_OutdoorFltrSz
          
0x002A0E3C,
0x0F120001,  //awbb_Use_InvalidOutDoor
0x002A0F3A,
0x0F12024C,  //awbb_OutdoorWP_r	//0240  //0218  // 0226
0x0F120290,  //awbb_OutdoorWP_b	//2B2  // 0280
          
0x002A0E46,
0x0F120Bb8,	//awbb_SunnyBr	  //
0x0F120096,	//awbb_Sunny_NBzone  //
0x0F120BB7,	//awbb_CloudyBr
0x0F120096,	//awbb_Sunny_NBzone  //
          
0x002A0E5E,
0x0F12071A,  //awbb_GamutWidthThr1
0x0F1203A4,
          
0x002A0E50,
0x0F12001B,  //awbb_MacbethGamut_WidthZone
0x0F12000E,
0x0F120008,
0x0F120004,
       
0x002A0E36,
0x0F120001,  //awbb_ByPass_LowTempMode
//////////////////
//AWB etc
//////////////////

0x002A0E3A,
0x0F1202C2,  //awbb_Alpha_Comp_Mode
         
0x002A0F12,
0x0F1202C9,  //awbb_GLocusR
0x0F12033F,  //awbb_GLocusB
        
0x002A0E1A,
0x0F120138,  //awbb_IntcR
         
0x002A236c,
0x0F120000,  //AWBBTune_EVT4_uInitPostToleranceCnt


//////////////////
//AWB Start Point
//////////////////

0x002A0c48, // #awbb_GainsInit 
0x0F12053c, // R Gain
0x0F120400, //400,
0x0F12055c, // B Gain


//////////////////////////
//8. Grid Correction      //
//////////////////////////

0x002A0E42,
0x0F120002,
          
0x002A0EE0,
0x0F1200B5,  //awbb_GridCoeff_R_2 
0x0F1200B5,  //awbb_GridCoeff_B_2 
0x002A0ED0,  //
0x0F120EC8,  //awbb_GridConst_2[0]
0x0F121022,  //awbb_GridConst_2[1]
0x0F1210BB,  //awbb_GridConst_2[2]
0x0F1210C9,  //awbb_GridConst_2[3]
0x0F121149,  //awbb_GridConst_2[4]
0x0F1211FD,  //awbb_GridConst_2[5]
0x0F1200B8,  //awbb_GridCoeff_R_1 
0x0F1200B2,  //awbb_GridCoeff_B_1 
0x002A0ECA,  //
0x0F12029A,  //awbb_GridConst_1[0]
0x0F120344,  //awbb_GridConst_1[1]
0x0F1203FB,  //awbb_GridConst_1[2]


0x002A0E82,
0x0F12ffd0,  //awbb_GridCorr_R[0][0] //d65
0x0F12fffd,  //awbb_GridCorr_R[0][1]  //d50
0x0F120000,  //awbb_GridCorr_R[0][2]  //cw
0x0F120000,  //awbb_GridCorr_R[0][3]  //ww
0x0F120000,  //awbb_GridCorr_R[0][4]  //a 
0x0F120080,  //awbb_GridCorr_R[0][5] //h

0x0F12ffd0,  //awbb_GridCorr_R[1][0]
0x0F12fffd,  //fffa awbb_GridCorr_R[1][1]
0x0F120000,  //awbb_GridCorr_R[1][2]
0x0F120000,  //awbb_GridCorr_R[1][3]
0x0F120000,  //awbb_GridCorr_R[1][4]
0x0F120080,  //awbb_GridCorr_R[1][5]

0x0F12ffd0,  //awbb_GridCorr_R[2][0]
0x0F12fffd,  //awbb_GridCorr_R[2][1]
0x0F120000,  //awbb_GridCorr_R[2][2]
0x0F120000,  //awbb_GridCorr_R[2][3]
0x0F120000,  //awbb_GridCorr_R[2][4]
0x0F120080,  //awbb_GridCorr_R[2][5]
           
0x0F120000, //awbb_GridCorr_B[0][0]
0x0F12fff0, //awbb_GridCorr_B[0][1]
0x0F12fffa, //awbb_GridCorr_B[0][2]
0x0F12fffa, //awbb_GridCorr_B[0][3]                                    
0x0F12ffba, //awbb_GridCorr_B[0][4]                                    
0x0F12fe80, //awbb_GridCorr_B[0][5]                                    
                          
0x0F120000, //awbb_GridCorr_B[1][0]
0x0F12fff0, //awbb_GridCorr_B[1][1]
0x0F12fffa, //awbb_GridCorr_B[1][2]
0x0F12fffa, //awbb_GridCorr_B[1][3]
0x0F12ffba, //awbb_GridCorr_B[1][4]
0x0F12fe80, //awbb_GridCorr_B[1][5]

0x0F120000, //awbb_GridCorr_B[2][0]
0x0F12fff0, //awbb_GridCorr_B[2][1]
0x0F12fffa, //awbb_GridCorr_B[2][2]
0x0F12fffa, //awbb_GridCorr_B[2][3]
0x0F12ffba, //awbb_GridCorr_B[2][4]
0x0F12fe80, //awbb_GridCorr_B[2][5]


//////////////////////////
//9. CCM      //
//////////////////////////

0x002A06D4,
0x0F122380,  //TVAR_wbt_pOutdoorCcm
0x0F127000,
0x002A06CC,
0x0F1223A4,  //TVAR_wbt_pBaseCcms
0x0F127000,
0x002A06E8,
0x0F1223A4,
0x0F127000,
0x0F1223C8,
0x0F127000,
0x0F1223EC,
0x0F127000,
0x0F122410,
0x0F127000,
0x0F122434,
0x0F127000,
0x0F122458,
0x0F127000,


0x002A06DA,
0x0F1200BF,	//SARR_AwbCcmCord[0]  2   700006DA
0x0F1200E6,	//SARR_AwbCcmCord[1]  2   700006DC
0x0F1200F2,	//SARR_AwbCcmCord[2]  2   700006DE
0x0F120143,	//SARR_AwbCcmCord[3]  2   700006E0
0x0F120178,	//SARR_AwbCcmCord[4]  2   700006E2
0x0F1201A3,	//SARR_AwbCcmCord[5]

//	param_start	TVAR_wbt_pBaseCcms
0x002A23A4,
0x0F1201F0, //01F0 //01FA	  	//H//23A4
0x0F12FFB9, //FFB9 //FFB9 
0x0F12FFFD, //FFFD //FFF8 
0x0F1200CC, //00CC //00EC 
0x0F120104, //0104 //00EB 
0x0F12FF3E, //FF3E //FF33 
0x0F12FF4D, //FF4D //FF23 
0x0F1201B4, //01B4 //01AB 
0x0F12FF53, //FF53 //FF81 
0x0F12FF0E, //FF0E //FF0D 
0x0F12016A, //016A //0169 
0x0F1200DF, //00DF //00DE 
0x0F12FFD9, //FFD9 //FFEF 
0x0F12FFAB, //FFAB //FFCA 
0x0F12017E, //017E //014D 
0x0F1201C1, //01C1 //01C3 
0x0F12FF7B, //FF7B //FF7E 
0x0F12016D, //016D //016F      
        
//  ///  /           
0x0F1201F0, //01FA //01FA    //A//23C8
0x0F12FFB9, //FFB9 //FFB9
0x0F12FFFD, //FFF8 //FFF8
0x0F1200CC, //0116 //00EC
0x0F120104, //00BD //00EB
0x0F12FF3E, //FF38 //FF33
0x0F12FF4D, //FF23 //FF23
0x0F1201B4, //01AB //01AB
0x0F12FF53, //FF81 //FF81
0x0F12FF0E, //FF0D //FF0D
0x0F12016A, //0169 //0169
0x0F1200DF, //00DE //00DE
0x0F12FFD9, //FFEF //FFEF
0x0F12FFAB, //FFCA //FFCA
0x0F12017E, //014D //014D
0x0F1201C1, //01C3 //01C3
0x0F12FF7B, //FF7E //FF7E
0x0F12016D, //016F //016F
//  ///  /        
0x0F1201F0, //01FA	//WW//23EC
0x0F12FFB9, //FFB9
0x0F12FFFD, //FFF8
0x0F1200CC, //00EC
0x0F120104, //00EB
0x0F12FF3E, //FF33
0x0F12FF4D, //FF23
0x0F1201B4, //01AB
0x0F12FF53, //FF81
0x0F12FF0E, //FF0D
0x0F12016A, //0169
0x0F1200DF, //00DE
0x0F12FFD9, //FFEF
0x0F12FFAB, //FFCA
0x0F12017E, //014D
0x0F1201C1, //01C3
0x0F12FF7B, //FF7E
0x0F12016D, //016F
//  ///  /        
0x0F1201E0, //01B7	//CW//2410
0x0F12FFB9, //FFD0
0x0F12000C, //0019
0x0F1200E0, //00C5
0x0F1200E1, //00E3
0x0F12FF4C, //FF5D
0x0F12FF33, //FF3E
0x0F12019C, //0186
0x0F12FF85, //FF8C
0x0F12FF0E, //FF0E
0x0F12016A, //016A
0x0F1200DF, //00DF
0x0F12FFEE, //FFEE
0x0F12FFC9, //FFC9
0x0F12014C, //014C
0x0F1201C1, //01C1
0x0F12FF7C, //FF7C
0x0F12016D, //016D
//  ///  /        
//  ///  /        
0x0F120200, //01E0, //01B7	//D50//2434
0x0F12FFA4, //FFB9, //FFD0
0x0F12FFFF, //000C, //0019
0x0F1200E1, //00CB, //00C5
0x0F1200E0, //00F3, //00E3
0x0F12FF4C, //FF4C, //FF5D
0x0F12FF66, //ff20, //FF33, //FF3E
0x0F1201AB, //01bb, //019C, //0186
0x0F12FF43, //ff79, //FF85, //FF8C
0x0F12FF0E, //FF0E, //FF0E
0x0F12016A, //016A, //016A
0x0F1200DF, //00DF, //00DF
0x0F12FFE8, //FFEE, //FFEE
0x0F12FFC1, //FFC9, //FFC9
0x0F12015A, //014C, //014C
0x0F1201C1, //01C1, //01C1
0x0F12FF7C, //FF7C, //FF7C
0x0F12016D, //016D, //016D
//  ///  /        
0x0F1201E3, //01E4, //01EA, //0202, //01E2, //01E0, //01B7	//D65//2458
0x0F12FFC6, //FFAB, //FF98, //FFA2, //FFB8, //FFB9, //FFD0
0x0F12FFF0, //0000, //FFF0, //FFFD, //000B, //000C, //0019
0x0F1200CB, //00C7, //00BE, //00CC, //00CB, //00CB, //00C5
0x0F1200F3, //00EF, //00E5, //00F4, //00F3, //00F3, //00E3
0x0F12FF4C, //FF48, //FF3E, //FF4D, //FF4C, //FF4C, //FF5D
0x0F12FF5D, //FF5D, //FF5B, //FF68, //FF66, //ff20, //FF33, //FF3E
0x0F1201BE, //01BE, //019F, //01AD, //01AB, //01bb, //019C, //0186
0x0F12FF36, //FF36, //FF38, //FF45, //FF43, //ff79, //FF85, //FF8C
0x0F12FF07, //FF07, //FF01, //FF0F, //FF0E, //FF0E, //FF0E
0x0F120162, //0162, //015C, //016B, //016A, //016A, //016A
0x0F1200D8, //00D8, //00D2, //00E0, //00DF, //00DF, //00DF
0x0F12FFD7, //FFD7, //FFD7, //FFE7, //FFE8, //FFEE, //FFEE
0x0F12FFB1, //FFB1, //FFB1, //FFC0, //FFC1, //FFC9, //FFC9
0x0F12014A, //014A, //014A, //0159, //015A, //014C, //014C
0x0F1201B4, //01B4, //01AE, //01BF, //01C1, //01C1, //01C1
0x0F12FF6F, //FF6F, //FF6A, //FF7A, //FF7C, //FF7C, //FF7C
0x0F120160, //0160, //015B, //016B, //016D, //016D, //016D
//	param_end	TVAR_wbt_pBaseCcms

//	param_start	TVAR_wbt_pOutdoorCcm
0x002A2380,
0x0F1201B7, //01B9, //01DD, //TVAR_wbt_pOutdoorCcm[0]  ¾ß¿Ü¾ó±¼»ö°¨       ÇÏ´Ã
0x0F12FFBC, //FFBC, //FFA8, //TVAR_wbt_pOutdoorCcm[1]
0x0F120011, //0012, //0008, //TVAR_wbt_pOutdoorCcm[2]

0x0F1200FC, //00FB, //00FD, //TVAR_wbt_pOutdoorCcm[3]
0x0F12013B, //013B, //0141, //TVAR_wbt_pOutdoorCcm[4]
0x0F12FF84, //FF82, //FF74, //TVAR_wbt_pOutdoorCcm[5]

0x0F12FED9, //FED6, //FEC6, //TVAR_wbt_pOutdoorCcm[6]
0x0F1201E6, //01E6, //01F6, //TVAR_wbt_pOutdoorCcm[7]
0x0F12FF16, //FF13, //FF05, //TVAR_wbt_pOutdoorCcm[8]

0x0F12FF4A, //FF48, //FF38, //TVAR_wbt_pOutdoorCcm[9]
0x0F120179, //0179, //017F, //TVAR_wbt_pOutdoorCcm[10]
0x0F12014F, //014F, //0153, //TVAR_wbt_pOutdoorCcm[11]

0x0F12FFCA, //FFC2 //FFCB, //FFCC, //TVAR_wbt_pOutdoorCcm[12]
0x0F12FFA3, //FF99 //FFA4, //FFA6, //TVAR_wbt_pOutdoorCcm[13]
0x0F120209, //0219 //0207, //020A, //TVAR_wbt_pOutdoorCcm[14]
  
0x0F120172, //0174, //017E, //TVAR_wbt_pOutdoorCcm[15]
0x0F12FF51, //FF51, //FF45, //TVAR_wbt_pOutdoorCcm[16]
0x0F12019B, //019E, //01AA, //TVAR_wbt_pOutdoorCcm[17]
//	param_end	TVAR_wbt_pOutdoorCcm

//////////////////////////
//10. AFIT      //
//////////////////////////

//	param_start	SARR_uNormBrInDoor
0x002A07E8,
0x0F120016,	//000A	//SARR_uNormBrInDoor[0]
0x0F120028,	//0019	//SARR_uNormBrInDoor[1]
0x0F120096,	//0096	//SARR_uNormBrInDoor[2]
0x0F1201F4,	//01F4	//SARR_uNormBrInDoor[3]
0x0F1207D0,	//07D0	//SARR_uNormBrInDoor[4]
//	param_end	SARR_uNormBrInDoor
//	param_start	afit_uNoiseIndInDoor
0x002A07D0,
0x0F120030,	//afit_uNoiseIndInDoor[0]
0x0F120046,	//afit_uNoiseIndInDoor[1]
0x0F120088,	//afit_uNoiseIndInDoor[2]
0x0F120205,	//afit_uNoiseIndInDoor[3]
0x0F1202BC,	//afit_uNoiseIndInDoor[4]
//	param_end	afit_uNoiseIndInDoor


0x002A07E6,
0x0F120000,	//afit_bUseNoiseInd 
//param_start	TVAR_afit_pBaseVals
0x002A0828,
0x0F120008, //TVAR_afit_pBaseVals[0]     70000828  //BRIGHTNESS              22                                                  
0x0F120014,	//TVAR_afit_pBaseVals[1]     7000082A  //CONTRAST                                                                  
0x0F120000,	//TVAR_afit_pBaseVals[2]     7000082C  //SATURATION                                                                
0x0F120000,	//TVAR_afit_pBaseVals[3]     7000082E  //SHARP_BLUR                                                                
0x0F120000,	//TVAR_afit_pBaseVals[4]     70000830  //GLAMOUR                                                                   
0x0F1203FF,	//TVAR_afit_pBaseVals[5]     70000832  //Disparity_iSatSat                                                         
0x0F12003a,	//TVAR_afit_pBaseVals[6]     70000834  //Denoise1_iYDenThreshLow                                                   
0x0F120028,	//TVAR_afit_pBaseVals[7]     70000836  //Denoise1_iYDenThreshLow_Bin                                               
0x0F120050,	//TVAR_afit_pBaseVals[8]     70000838  //Denoise1_iYDenThreshHigh                                                  
0x0F1200FF,	//TVAR_afit_pBaseVals[9]     7000083A  //Denoise1_iYDenThreshHigh_Bin                                              
0x0F120129,	//TVAR_afit_pBaseVals[10]    7000083C  //Denoise1_iLowWWideThresh                                                  
0x0F12000A,	//TVAR_afit_pBaseVals[11]    7000083E  //Denoise1_iHighWWideThresh                                                 
0x0F120028,	//TVAR_afit_pBaseVals[12]    70000840  //Denoise1_iLowWideThresh                                                   
0x0F120028,	//TVAR_afit_pBaseVals[13]    70000842  //Denoise1_iHighWideThresh                                                  
0x0F1203FF,	//TVAR_afit_pBaseVals[14]    70000844  //Denoise1_iSatSat                                                          
0x0F1203FF,	//TVAR_afit_pBaseVals[15]    70000846  //Demosaic4_iHystGrayLow                                                    
0x0F120000,	//TVAR_afit_pBaseVals[16]    70000848  //Demosaic4_iHystGrayHigh                                                   
0x0F120344,	//TVAR_afit_pBaseVals[17]    7000084A  //UVDenoise_iYLowThresh                                                     
0x0F12033A,	//TVAR_afit_pBaseVals[18]    7000084C  //UVDenoise_iYHighThresh                                                    
0x0F1203FF,	//TVAR_afit_pBaseVals[19]    7000084E  //UVDenoise_iUVLowThresh                                                    
0x0F1203FF,	//TVAR_afit_pBaseVals[20]    70000850  //UVDenoise_iUVHighThresh                                                   
0x0F12000A,	//TVAR_afit_pBaseVals[21]    70000852  //DSMix1_iLowLimit_Wide                                                     
0x0F120032,	//TVAR_afit_pBaseVals[22]    70000854  //DSMix1_iLowLimit_Wide_Bin                                                 
0x0F12001E,	//TVAR_afit_pBaseVals[23]    70000856  //DSMix1_iHighLimit_Wide                                                    
0x0F120032,	//TVAR_afit_pBaseVals[24]    70000858  //DSMix1_iHighLimit_Wide_Bin                                                
0x0F120032,	//TVAR_afit_pBaseVals[25]    7000085A  //DSMix1_iLowLimit_Fine                                                     
0x0F120032,	//TVAR_afit_pBaseVals[26]    7000085C  //DSMix1_iLowLimit_Fine_Bin                                                 
0x0F120010,	//TVAR_afit_pBaseVals[27]    7000085E  //DSMix1_iHighLimit_Fine                                                    
0x0F120032,	//TVAR_afit_pBaseVals[28]    70000860  //DSMix1_iHighLimit_Fine_Bin                                                
0x0F120106,	//TVAR_afit_pBaseVals[29]    70000862  //DSMix1_iRGBOffset                                                         
0x0F12006F,	//TVAR_afit_pBaseVals[30]    70000864  //DSMix1_iDemClamp                                                          
0x0F120C0F,	//TVAR_afit_pBaseVals[31]    70000866  //"Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                          
0x0F120C0F,	//TVAR_afit_pBaseVals[32]    70000868  //"Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                       
0x0F120303,	//TVAR_afit_pBaseVals[33]    7000086A  //"Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"        
0x0F120303,	//TVAR_afit_pBaseVals[34]    7000086C  //Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin            
0x0F12140A,	//TVAR_afit_pBaseVals[35]    7000086E  //"Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                       
0x0F12140A,	//TVAR_afit_pBaseVals[36]    70000870  //"Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                         
0x0F122828,	//TVAR_afit_pBaseVals[37]    70000872  //"Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"               
0x0F120000,	//TVAR_afit_pBaseVals[38]    70000874  //"Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                       
0x0F12020A,	//TVAR_afit_pBaseVals[39]    70000876  //"Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                        
0x0F120480,	//TVAR_afit_pBaseVals[40]    70000878  //"Denoise1_iRadialLimitDenoise1_iLWBNoise"                                 
0x0F120E08,	//TVAR_afit_pBaseVals[41]    7000087A  //"Denoise1_iWideDenoise1_iWideWide"                                        
0x0F12030A,	//TVAR_afit_pBaseVals[42]    7000087C  //"Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                             
0x0F120A03,	//TVAR_afit_pBaseVals[43]    7000087E  //"UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                               
0x0F120A11,	//TVAR_afit_pBaseVals[44]    70000880  //"DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                         
0x0F12000F,	//TVAR_afit_pBaseVals[45]    70000882  //"DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                        
0x0F120500,	//TVAR_afit_pBaseVals[46]    70000884  //"DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                       
0x0F120914,	//TVAR_afit_pBaseVals[47]    70000886  //"DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                          
0x0F120012,	//TVAR_afit_pBaseVals[48]    70000888  //"DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                        
0x0F120000,	//TVAR_afit_pBaseVals[49]    7000088A  //"DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                           
0x0F120005,	//TVAR_afit_pBaseVals[50]    7000088C  //"DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                         
0x0F120000,	//TVAR_afit_pBaseVals[51]    7000088E  //"Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                        
0x0F120000,	//TVAR_afit_pBaseVals[52]    70000890  //"Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"                
0x0F120000,	//TVAR_afit_pBaseVals[53]    70000892  //"Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"                      
0x0F120000,	//TVAR_afit_pBaseVals[54]    70000894  //"Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                        
0x0F120A00,	//TVAR_afit_pBaseVals[55]    70000896  //"Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                        
0x0F12000A,	//TVAR_afit_pBaseVals[56]    70000898  //"Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                        
0x0F120180, //014C,	//TVAR_afit_pBaseVals[57]    7000089A  //"CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                              
0x0F12014D,	//TVAR_afit_pBaseVals[58]    7000089C  //"RGBGamma2_iLinearityRGBGamma2_bLinearity"                                
0x0F120100,	//TVAR_afit_pBaseVals[59]    7000089E  //"RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                              
0x0F128020,	//TVAR_afit_pBaseVals[60]    700008A0  //"byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                                  
0x0F120180,	//TVAR_afit_pBaseVals[61]    700008A2  //"RGB2YUV_iSaturationRGB2YUV_bGainOffset"                                  
0x0F120013, //11 15, //18, //001a, //05, //A,	//TVAR_afit_pBaseVals[62]    700008A4  //RGB2YUV_iYOffset                                                          

0x0F120000,	//TVAR_afit_pBaseVals[63]    700008A6            //BRIGHTNESS               40                                                 
0x0F120028,	//TVAR_afit_pBaseVals[64]    700008A8            //CONTRAST                                                                  
0x0F120000,	//TVAR_afit_pBaseVals[65]    700008AA            //SATURATION                                                                
0x0F120000,	//TVAR_afit_pBaseVals[66]    700008AC            //SHARP_BLUR                                                                
0x0F120000,	//TVAR_afit_pBaseVals[67]    700008AE            //GLAMOUR                                                                   
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[68]    700008B0    //Disparity_iSatSat                                                         
0x0F12000E,	//0020	//TVAR_afit_pBaseVals[69]    700008B2    //Denoise1_iYDenThreshLow                                                   
0x0F12000E,	//000E	//TVAR_afit_pBaseVals[70]    700008B4    //Denoise1_iYDenThreshLow_Bin                                               
0x0F120050,	//0080	//TVAR_afit_pBaseVals[71]    700008B6    //Denoise1_iYDenThreshHigh                                                  
0x0F1200FF,	//00FF	//TVAR_afit_pBaseVals[72]    700008B8    //Denoise1_iYDenThreshHigh_Bin                                              
0x0F120129,	//0129	//TVAR_afit_pBaseVals[73]    700008BA    //Denoise1_iLowWWideThresh                                                  
0x0F12000A,	//000A	//TVAR_afit_pBaseVals[74]    700008BC    //Denoise1_iHighWWideThresh                                                 
0x0F120028,	//0028	//TVAR_afit_pBaseVals[75]    700008BE    //Denoise1_iLowWideThresh                                                   
0x0F120028,	//0028	//TVAR_afit_pBaseVals[76]    700008C0    //Denoise1_iHighWideThresh                                                  
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[77]    700008C2    //Denoise1_iSatSat                                                          
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[78]    700008C4    //Demosaic4_iHystGrayLow                                                    
0x0F120000,	//0000	//TVAR_afit_pBaseVals[79]    700008C6    //Demosaic4_iHystGrayHigh                                                   
0x0F120114,	//0014	//TVAR_afit_pBaseVals[80]    700008C8    //UVDenoise_iYLowThresh                                                     
0x0F12020A,	//000A	//TVAR_afit_pBaseVals[81]    700008CA    //UVDenoise_iYHighThresh                                                    
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[82]    700008CC    //UVDenoise_iUVLowThresh                                                    
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[83]    700008CE    //UVDenoise_iUVHighThresh                                                   
0x0F120018,	//000a	//TVAR_afit_pBaseVals[84]    700008D0    //DSMix1_iLowLimit_Wide                                                     
0x0F120032,	//0000	//TVAR_afit_pBaseVals[85]    700008D2    //DSMix1_iLowLimit_Wide_Bin                                                 
0x0F12000A,	//0014	//TVAR_afit_pBaseVals[86]    700008D4    //DSMix1_iHighLimit_Wide                                                    
0x0F120032,	//0032	//TVAR_afit_pBaseVals[87]    700008D6    //DSMix1_iHighLimit_Wide_Bin                                                
0x0F120028,	//0000	//TVAR_afit_pBaseVals[88]    700008D8    //DSMix1_iLowLimit_Fine                                                     
0x0F120032,	//0000	//TVAR_afit_pBaseVals[89]    700008DA    //DSMix1_iLowLimit_Fine_Bin                                                 
0x0F120010,	//00A0	//TVAR_afit_pBaseVals[90]    700008DC    //DSMix1_iHighLimit_Fine                                                    
0x0F120032,	//0000	//TVAR_afit_pBaseVals[91]    700008DE    //DSMix1_iHighLimit_Fine_Bin                                                
0x0F120106,	//0106	//TVAR_afit_pBaseVals[92]    700008E0    //DSMix1_iRGBOffset                                                         
0x0F12006F,	//006F	//TVAR_afit_pBaseVals[93]    700008E2    //DSMix1_iDemClamp                                                          
0x0F12050F,	//050F	//TVAR_afit_pBaseVals[94]    700008E4    //"Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                          
0x0F120A0F,	//0A0F	//TVAR_afit_pBaseVals[95]    700008E6    //"Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                       
0x0F120203,	//0203	//TVAR_afit_pBaseVals[96]    700008E8    //"Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"        
0x0F120303,	//0203	//TVAR_afit_pBaseVals[97]    700008EA    //Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin            
0x0F12140A,	//140A	//TVAR_afit_pBaseVals[98]    700008EC    //"Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                       
0x0F12140A,	//140A	//TVAR_afit_pBaseVals[99]    700008EE    //"Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                         
0x0F122828,	//2828	//TVAR_afit_pBaseVals[100]   700008F0    //"Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"               
0x0F120000,	//0000	//TVAR_afit_pBaseVals[101]   700008F2    //"Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                       
0x0F12020A,	//020A	//TVAR_afit_pBaseVals[102]   700008F4    //"Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                        
0x0F120480,	//0480	//TVAR_afit_pBaseVals[103]   700008F6    //"Denoise1_iRadialLimitDenoise1_iLWBNoise"                                 
0x0F120E08,	//0E08	//TVAR_afit_pBaseVals[104]   700008F8    //"Denoise1_iWideDenoise1_iWideWide"                                        
0x0F12030A,	//020A	//TVAR_afit_pBaseVals[105]   700008FA    //"Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                             
0x0F121403,	//0A03	//TVAR_afit_pBaseVals[106]   700008FC    //"UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                               
0x0F120A11,	//0A11	//TVAR_afit_pBaseVals[107]   700008FE    //"DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                         
0x0F120A0F,	//0A0F	//TVAR_afit_pBaseVals[108]   70000900    //"DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                        
0x0F12050A,	//050A	//TVAR_afit_pBaseVals[109]   70000902    //"DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                       
0x0F12101E,	//101E	//TVAR_afit_pBaseVals[110]   70000904    //"DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                          
0x0F12101E,	//101E	//TVAR_afit_pBaseVals[111]   70000906    //"DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                        
0x0F120A08,	//3030	//TVAR_afit_pBaseVals[112]   70000908    //"DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                           
0x0F120005,	//0005	//TVAR_afit_pBaseVals[113]   7000090A    //"DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                         
0x0F120400,	//0400	//TVAR_afit_pBaseVals[114]   7000090C    //"Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                        
0x0F120400,	//0400	//TVAR_afit_pBaseVals[115]   7000090E    //"Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"                
0x0F120000,	//0000	//TVAR_afit_pBaseVals[116]   70000910    //"Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"                      
0x0F120000,	//0000	//TVAR_afit_pBaseVals[117]   70000912    //"Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                        
0x0F120A00,	//0A00	//TVAR_afit_pBaseVals[118]   70000914    //"Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                        
0x0F12000A,	//100A	//TVAR_afit_pBaseVals[119]   70000916    //"Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                        
0x0F120180,	//TVAR_afit_pBaseVals[120]   70000918            //"CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                              
0x0F120154,	//TVAR_afit_pBaseVals[121]   7000091A            //"RGBGamma2_iLinearityRGBGamma2_bLinearity"                                
0x0F120100,	//TVAR_afit_pBaseVals[122]   7000091C            //"RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                              
0x0F128020,	//TVAR_afit_pBaseVals[123]   7000091E            //"byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                                  
0x0F120180,	//TVAR_afit_pBaseVals[124]   70000920            //"RGB2YUV_iSaturationRGB2YUV_bGainOffset"                                  
0x0F12000d,	//TVAR_afit_pBaseVals[125]   70000922            //RGB2YUV_iYOffset                                                          

0x0F120000,	//TVAR_afit_pBaseVals[126]   70000924            //BRIGHTNESS                    150                                            
0x0F120024,	//TVAR_afit_pBaseVals[127]   70000926            //CONTRAST                                                                  
0x0F120000,	//TVAR_afit_pBaseVals[128]   70000928            //SATURATION                                                                
0x0F120000,	//TVAR_afit_pBaseVals[129]   7000092A            //SHARP_BLUR                                                                
0x0F120000,	//TVAR_afit_pBaseVals[130]   7000092C            //GLAMOUR                                                                   
0x0F1203FF,	//TVAR_afit_pBaseVals[131]   7000092E    //Disparity_iSatSat                                                         
0x0F12000E,	//TVAR_afit_pBaseVals[132]   70000930    //Denoise1_iYDenThreshLow                                                   
0x0F120006,	//0006	//TVAR_afit_pBaseVals[133]   70000932    //Denoise1_iYDenThreshLow_Bin                                               
0x0F120050,	//0064	//TVAR_afit_pBaseVals[134]   70000934    //Denoise1_iYDenThreshHigh                                                  
0x0F120050,	//0050	//TVAR_afit_pBaseVals[135]   70000936    //Denoise1_iYDenThreshHigh_Bin                                              
0x0F120002,	//0002	//TVAR_afit_pBaseVals[136]   70000938    //Denoise1_iLowWWideThresh                                                  
0x0F12000A,	//000A	//TVAR_afit_pBaseVals[137]   7000093A    //Denoise1_iHighWWideThresh                                                 
0x0F12000A,	//000A	//TVAR_afit_pBaseVals[138]   7000093C    //Denoise1_iLowWideThresh                                                   
0x0F12000A,	//000A	//TVAR_afit_pBaseVals[139]   7000093E    //Denoise1_iHighWideThresh                                                  
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[140]   70000940    //Denoise1_iSatSat                                                          
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[141]   70000942    //Demosaic4_iHystGrayLow                                                    
0x0F120000,	//0000	//TVAR_afit_pBaseVals[142]   70000944    //Demosaic4_iHystGrayHigh                                                   
0x0F120014,	//0014	//TVAR_afit_pBaseVals[143]   70000946    //UVDenoise_iYLowThresh                                                     
0x0F12000A,	//000A	//TVAR_afit_pBaseVals[144]   70000948    //UVDenoise_iYHighThresh                                                    
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[145]   7000094A    //UVDenoise_iUVLowThresh                                                    
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[146]   7000094C    //UVDenoise_iUVHighThresh                                                   
0x0F12001C,	//000a	//TVAR_afit_pBaseVals[147]   7000094E    //DSMix1_iLowLimit_Wide                                                     
0x0F120032,	//0032	//TVAR_afit_pBaseVals[148]   70000950    //DSMix1_iLowLimit_Wide_Bin                                                 
0x0F12000A,	//0014	//TVAR_afit_pBaseVals[149]   70000952    //DSMix1_iHighLimit_Wide                                                    
0x0F120032,	//0032	//TVAR_afit_pBaseVals[150]   70000954    //DSMix1_iHighLimit_Wide_Bin                                                
0x0F120028,	//0050	//TVAR_afit_pBaseVals[151]   70000956    //DSMix1_iLowLimit_Fine                                                     
0x0F120032,	//0032	//TVAR_afit_pBaseVals[152]   70000958    //DSMix1_iLowLimit_Fine_Bin                                                 
0x0F120010,	//0010	//TVAR_afit_pBaseVals[153]   7000095A    //DSMix1_iHighLimit_Fine                                                    
0x0F120032,	//0032	//TVAR_afit_pBaseVals[154]   7000095C    //DSMix1_iHighLimit_Fine_Bin                                                
0x0F120106,	//0106	//TVAR_afit_pBaseVals[155]   7000095E    //DSMix1_iRGBOffset                                                         
0x0F12006F,	//006F	//TVAR_afit_pBaseVals[156]   70000960    //DSMix1_iDemClamp                                                          
0x0F120205,	//020A	//TVAR_afit_pBaseVals[157]   70000962    //"Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                          
0x0F120505,	//050A	//TVAR_afit_pBaseVals[158]   70000964    //"Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                       
0x0F120101,	//0101	//TVAR_afit_pBaseVals[159]   70000966    //"Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"        
0x0F120202,	//0102	//TVAR_afit_pBaseVals[160]   70000968    //Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin            
0x0F12140A,	//140A	//TVAR_afit_pBaseVals[161]   7000096A    //"Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                       
0x0F12140A,	//140A	//TVAR_afit_pBaseVals[162]   7000096C    //"Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                         
0x0F122828,	//2828	//TVAR_afit_pBaseVals[163]   7000096E    //"Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"               
0x0F120606,	//0606	//TVAR_afit_pBaseVals[164]   70000970    //"Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                       
0x0F120205,	//0205	//TVAR_afit_pBaseVals[165]   70000972    //"Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                        
0x0F120480,	//0480	//TVAR_afit_pBaseVals[166]   70000974    //"Denoise1_iRadialLimitDenoise1_iLWBNoise"                                 
0x0F12000A,	//000F	//TVAR_afit_pBaseVals[167]   70000976    //"Denoise1_iWideDenoise1_iWideWide"                                        
0x0F120005,	//0005	//TVAR_afit_pBaseVals[168]   70000978    //"Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                             
0x0F121903,	//1903	//TVAR_afit_pBaseVals[169]   7000097A    //"UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                               
0x0F121911,	//1911	//TVAR_afit_pBaseVals[170]   7000097C    //"DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                         
0x0F120A0F,	//0A0F	//TVAR_afit_pBaseVals[171]   7000097E    //"DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                        
0x0F12050A,	//050A	//TVAR_afit_pBaseVals[172]   70000980    //"DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                       
0x0F12200f,	//2028	//TVAR_afit_pBaseVals[173]   70000982    //"DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                          
0x0F122028,	//2028	//TVAR_afit_pBaseVals[174]   70000984    //"DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                        
0x0F120A08,	//2000	//TVAR_afit_pBaseVals[175]   70000986    //"DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                           
0x0F120007,	//0007	//TVAR_afit_pBaseVals[176]   70000988    //"DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                         
0x0F120403,	//0403	//TVAR_afit_pBaseVals[177]   7000098A    //"Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                        
0x0F120402,	//0402	//TVAR_afit_pBaseVals[178]   7000098C    //"Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"                
0x0F120000,	//0000	//TVAR_afit_pBaseVals[179]   7000098E    //"Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"                      
0x0F120203,	//0203	//TVAR_afit_pBaseVals[180]   70000990    //"Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                        
0x0F120000,	//0000	//TVAR_afit_pBaseVals[181]   70000992    //"Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                        
0x0F120006,	//1006	//TVAR_afit_pBaseVals[182]   70000994    //"Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                        
0x0F120180,	//TVAR_afit_pBaseVals[183]   70000996            //"CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                              
0x0F120173,	//TVAR_afit_pBaseVals[184]   70000998            //"RGBGamma2_iLinearityRGBGamma2_bLinearity"                                
0x0F120100,	//TVAR_afit_pBaseVals[185]   7000099A            //"RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                              
0x0F128032,	//TVAR_afit_pBaseVals[186]   7000099C            //"byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                                  
0x0F120180,	//TVAR_afit_pBaseVals[187]   7000099E            //"RGB2YUV_iSaturationRGB2YUV_bGainOffset"                                  
0x0F120000,	//TVAR_afit_pBaseVals[188]   700009A0            //RGB2YUV_iYOffset                                                          

0x0F120000,	//TVAR_afit_pBaseVals[189]   700009A2            //BRIGHTNESS                                500                                
0x0F120014,	//TVAR_afit_pBaseVals[190]   700009A4            //CONTRAST                                                                  
0x0F120000,	//TVAR_afit_pBaseVals[191]   700009A6            //SATURATION                                                                
0x0F120000,	//TVAR_afit_pBaseVals[192]   700009A8            //SHARP_BLUR                                                                
0x0F120000,	//TVAR_afit_pBaseVals[193]   700009AA            //GLAMOUR                                                                   
0x0F1203FF,	//TVAR_afit_pBaseVals[194]   700009AC    //Disparity_iSatSat                                                         
0x0F12000E,	//TVAR_afit_pBaseVals[195]   700009AE    //Denoise1_iYDenThreshLow                                                   
0x0F120006,	//0006	//TVAR_afit_pBaseVals[196]   700009B0    //Denoise1_iYDenThreshLow_Bin                                               
0x0F120048,	//0064	//TVAR_afit_pBaseVals[197]   700009B2    //Denoise1_iYDenThreshHigh                                                  
0x0F120050,	//0050	//TVAR_afit_pBaseVals[198]   700009B4    //Denoise1_iYDenThreshHigh_Bin                                              
0x0F120002,	//0002	//TVAR_afit_pBaseVals[199]   700009B6    //Denoise1_iLowWWideThresh                                                  
0x0F12000A,	//000A	//TVAR_afit_pBaseVals[200]   700009B8    //Denoise1_iHighWWideThresh                                                 
0x0F12000A,	//000A	//TVAR_afit_pBaseVals[201]   700009BA    //Denoise1_iLowWideThresh                                                   
0x0F12000A,	//000A	//TVAR_afit_pBaseVals[202]   700009BC    //Denoise1_iHighWideThresh                                                  
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[203]   700009BE    //Denoise1_iSatSat                                                          
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[204]   700009C0    //Demosaic4_iHystGrayLow                                                    
0x0F120000,	//0000	//TVAR_afit_pBaseVals[205]   700009C2    //Demosaic4_iHystGrayHigh                                                   
0x0F120014,	//0014	//TVAR_afit_pBaseVals[206]   700009C4    //UVDenoise_iYLowThresh                                                     
0x0F120032,	//000A	//TVAR_afit_pBaseVals[207]   700009C6    //UVDenoise_iYHighThresh                                                    
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[208]   700009C8    //UVDenoise_iUVLowThresh                                                    
0x0F1203FF,	//03FF	//TVAR_afit_pBaseVals[209]   700009CA    //UVDenoise_iUVHighThresh                                                   
0x0F12001C,	//000a	//TVAR_afit_pBaseVals[210]   700009CC    //DSMix1_iLowLimit_Wide                                                     
0x0F120032,	//0032	//TVAR_afit_pBaseVals[211]   700009CE    //DSMix1_iLowLimit_Wide_Bin                                                 
0x0F12000A,	//0014	//TVAR_afit_pBaseVals[212]   700009D0    //DSMix1_iHighLimit_Wide                                                    
0x0F120032,	//0032	//TVAR_afit_pBaseVals[213]   700009D2    //DSMix1_iHighLimit_Wide_Bin                                                
0x0F120028,	//0050	//TVAR_afit_pBaseVals[214]   700009D4    //DSMix1_iLowLimit_Fine                                                     
0x0F120032,	//0032	//TVAR_afit_pBaseVals[215]   700009D6    //DSMix1_iLowLimit_Fine_Bin                                                 
0x0F120010,	//0010	//TVAR_afit_pBaseVals[216]   700009D8    //DSMix1_iHighLimit_Fine                                                    
0x0F120032,	//0032	//TVAR_afit_pBaseVals[217]   700009DA    //DSMix1_iHighLimit_Fine_Bin                                                
0x0F120106,	//0106	//TVAR_afit_pBaseVals[218]   700009DC    //DSMix1_iRGBOffset                                                         
0x0F12006F,	//006F	//TVAR_afit_pBaseVals[219]   700009DE    //DSMix1_iDemClamp                                                          
0x0F120205,	//0205	//TVAR_afit_pBaseVals[220]   700009E0    //"Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                          
0x0F120505,	//0505	//TVAR_afit_pBaseVals[221]   700009E2    //"Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                       
0x0F120101,	//0101	//TVAR_afit_pBaseVals[222]   700009E4    //"Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"        
0x0F120202,	//0102	//TVAR_afit_pBaseVals[223]   700009E6    //Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin            
0x0F12140A,	//140A	//TVAR_afit_pBaseVals[224]   700009E8    //"Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                       
0x0F12140A,	//140A	//TVAR_afit_pBaseVals[225]   700009EA    //"Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                         
0x0F122828,	//2828	//TVAR_afit_pBaseVals[226]   700009EC    //"Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"               
0x0F120606,	//0606	//TVAR_afit_pBaseVals[227]   700009EE    //"Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                       
0x0F120205,	//0205	//TVAR_afit_pBaseVals[228]   700009F0    //"Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                        
0x0F120480,	//0480	//TVAR_afit_pBaseVals[229]   700009F2    //"Denoise1_iRadialLimitDenoise1_iLWBNoise"                                 
0x0F12000A,	//000F	//TVAR_afit_pBaseVals[230]   700009F4    //"Denoise1_iWideDenoise1_iWideWide"                                        
0x0F120005,	//0005	//TVAR_afit_pBaseVals[231]   700009F6    //"Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                             
0x0F121903,	//1903	//TVAR_afit_pBaseVals[232]   700009F8    //"UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                               
0x0F121911,	//1911	//TVAR_afit_pBaseVals[233]   700009FA    //"DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                         
0x0F120A0F,	//0A0F	//TVAR_afit_pBaseVals[234]   700009FC    //"DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                        
0x0F12050A,	//050A	//TVAR_afit_pBaseVals[235]   700009FE    //"DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                       
0x0F12201a,	//2028	//TVAR_afit_pBaseVals[236]   70000A00    //"DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                          
0x0F122020,	//2028	//TVAR_afit_pBaseVals[237]   70000A02    //"DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                        
0x0F120A08,	//2000	//TVAR_afit_pBaseVals[238]   70000A04    //"DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                           
0x0F120007,	//0007	//TVAR_afit_pBaseVals[239]   70000A06    //"DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                         
0x0F120403,	//0403	//TVAR_afit_pBaseVals[240]   70000A08    //"Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                        
0x0F120402,	//0402	//TVAR_afit_pBaseVals[241]   70000A0A    //"Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"                
0x0F120000,	//0000	//TVAR_afit_pBaseVals[242]   70000A0C    //"Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"                      
0x0F120203,	//0203	//TVAR_afit_pBaseVals[243]   70000A0E    //"Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                        
0x0F120000,	//0000	//TVAR_afit_pBaseVals[244]   70000A10    //"Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                        
0x0F120006,	//1006	//TVAR_afit_pBaseVals[245]   70000A12    //"Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                        
0x0F120180,	//TVAR_afit_pBaseVals[246]   70000A14            //"CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                              
0x0F120180,	//TVAR_afit_pBaseVals[247]   70000A16            //"RGBGamma2_iLinearityRGBGamma2_bLinearity"                                
0x0F120100,	//TVAR_afit_pBaseVals[248]   70000A18            //"RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                              
0x0F12803c,	//TVAR_afit_pBaseVals[249]   70000A1A            //"byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                                  
0x0F120180,	//TVAR_afit_pBaseVals[250]   70000A1C            //"RGB2YUV_iSaturationRGB2YUV_bGainOffset"                                  
0x0F120000,	//TVAR_afit_pBaseVals[251]   70000A1E            //RGB2YUV_iYOffset  

0x0F120000,	//TVAR_afit_pBaseVals[252]   70000A20                //BRIGHTNESS                                                                
0x0F120000,	//TVAR_afit_pBaseVals[253]   70000A22                //CONTRAST                                                                  
0x0F120000,	//TVAR_afit_pBaseVals[254]   70000A24                //SATURATION                                                                
0x0F120014,	//TVAR_afit_pBaseVals[255]   70000A26                //SHARP_BLUR                                                                
0x0F120000,	//TVAR_afit_pBaseVals[256]   70000A28                //GLAMOUR                                                                   
0x0F1203FF,	//TVAR_afit_pBaseVals[257]   70000A2A                //Disparity_iSatSat                                                         
0x0F12000E,	//TVAR_afit_pBaseVals[258]   70000A2C                //Denoise1_iYDenThreshLow                                                   
0x0F120006,	//TVAR_afit_pBaseVals[259]   70000A2E                //Denoise1_iYDenThreshLow_Bin                                               
0x0F120020,	//TVAR_afit_pBaseVals[260]   70000A30                //Denoise1_iYDenThreshHigh                                                  
0x0F120050,	//TVAR_afit_pBaseVals[261]   70000A32                //Denoise1_iYDenThreshHigh_Bin                                              
0x0F120002,	//TVAR_afit_pBaseVals[262]   70000A34                //Denoise1_iLowWWideThresh                                                  
0x0F12000A,	//TVAR_afit_pBaseVals[263]   70000A36                //Denoise1_iHighWWideThresh                                                 
0x0F12000A,	//TVAR_afit_pBaseVals[264]   70000A38                //Denoise1_iLowWideThresh                                                   
0x0F12000A,	//TVAR_afit_pBaseVals[265]   70000A3A                //Denoise1_iHighWideThresh                                                  
0x0F1203FF,	//TVAR_afit_pBaseVals[266]   70000A3C                //Denoise1_iSatSat                                                          
0x0F1203FF,	//TVAR_afit_pBaseVals[267]   70000A3E                //Demosaic4_iHystGrayLow                                                    
0x0F120000,	//TVAR_afit_pBaseVals[268]   70000A40                //Demosaic4_iHystGrayHigh                                                   
0x0F120000,	//TVAR_afit_pBaseVals[269]   70000A42                //UVDenoise_iYLowThresh                                                     
0x0F120000,	//TVAR_afit_pBaseVals[270]   70000A44                //UVDenoise_iYHighThresh                                                    
0x0F1203FF,	//TVAR_afit_pBaseVals[271]   70000A46                //UVDenoise_iUVLowThresh                                                    
0x0F1203FF,	//TVAR_afit_pBaseVals[272]   70000A48                //UVDenoise_iUVHighThresh                                                   
0x0F120014,	//TVAR_afit_pBaseVals[273]   70000A4A                //DSMix1_iLowLimit_Wide                                                     
0x0F120032,	//TVAR_afit_pBaseVals[274]   70000A4C                //DSMix1_iLowLimit_Wide_Bin                                                 
0x0F120000,	//TVAR_afit_pBaseVals[275]   70000A4E                //DSMix1_iHighLimit_Wide                                                    
0x0F120032,	//TVAR_afit_pBaseVals[276]   70000A50                //DSMix1_iHighLimit_Wide_Bin                                                
0x0F120020,	//TVAR_afit_pBaseVals[277]   70000A52                //DSMix1_iLowLimit_Fine                                                     
0x0F120032,	//TVAR_afit_pBaseVals[278]   70000A54                //DSMix1_iLowLimit_Fine_Bin                                                 
0x0F120000,	//TVAR_afit_pBaseVals[279]   70000A56                //DSMix1_iHighLimit_Fine                                                    
0x0F120032,	//TVAR_afit_pBaseVals[280]   70000A58                //DSMix1_iHighLimit_Fine_Bin                                                
0x0F120106,	//TVAR_afit_pBaseVals[281]   70000A5A                //DSMix1_iRGBOffset                                                         
0x0F12006F,	//TVAR_afit_pBaseVals[282]   70000A5C                //DSMix1_iDemClamp                                                          
0x0F120202,	//TVAR_afit_pBaseVals[283]   70000A5E                //"Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                          
0x0F120502,	//TVAR_afit_pBaseVals[284]   70000A60                //"Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                       
0x0F120101,	//TVAR_afit_pBaseVals[285]   70000A62                //"Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"        
0x0F120202,	//TVAR_afit_pBaseVals[286]   70000A64                //Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin            
0x0F12140A,	//TVAR_afit_pBaseVals[287]   70000A66                //"Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                       
0x0F12140A,	//TVAR_afit_pBaseVals[288]   70000A68                //"Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                         
0x0F122828,	//TVAR_afit_pBaseVals[289]   70000A6A                //"Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"               
0x0F120606,	//TVAR_afit_pBaseVals[290]   70000A6C                //"Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                       
0x0F120205,	//TVAR_afit_pBaseVals[291]   70000A6E                //"Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                        
0x0F120880,	//TVAR_afit_pBaseVals[292]   70000A70                //"Denoise1_iRadialLimitDenoise1_iLWBNoise"                                 
0x0F12000F,	//TVAR_afit_pBaseVals[293]   70000A72                //"Denoise1_iWideDenoise1_iWideWide"                                        
0x0F120005,	//TVAR_afit_pBaseVals[294]   70000A74                //"Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                             
0x0F121903,	//TVAR_afit_pBaseVals[295]   70000A76                //"UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                               
0x0F121911, //0D11 1911	//TVAR_afit_pBaseVals[296]   70000A78                //"DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                         
0x0F120A0F,	//TVAR_afit_pBaseVals[297]   70000A7A                //"DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                        
0x0F12050A,	//TVAR_afit_pBaseVals[298]   70000A7C                //"DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                       
0x0F122020, //10 20	//TVAR_afit_pBaseVals[299]   70000A7E                //"DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                          
0x0F122020, //1A 20	//TVAR_afit_pBaseVals[300]   70000A80                //"DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                        
0x0F120A08,	//0A08 TVAR_afit_pBaseVals[301]   70000A82                //"DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                           
0x0F120007,	//0007 TVAR_afit_pBaseVals[302]   70000A84                //"DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                         
0x0F120408,	//TVAR_afit_pBaseVals[303]   70000A86                //"Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                        
0x0F120406,	//TVAR_afit_pBaseVals[304]   70000A88                //"Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"                
0x0F120000,	//TVAR_afit_pBaseVals[305]   70000A8A                //"Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"                      
0x0F120608,	//TVAR_afit_pBaseVals[306]   70000A8C                //"Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                        
0x0F120000,	//TVAR_afit_pBaseVals[307]   70000A8E                //"Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                        
0x0F120006,	//TVAR_afit_pBaseVals[308]   70000A90                //"Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                        
0x0F120180,	//TVAR_afit_pBaseVals[309]   70000A92 //180 173 164  //"CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                              
0x0F120180,	//TVAR_afit_pBaseVals[310]   70000A94 //Linearity    //"RGBGamma2_iLinearityRGBGamma2_bLinearity"                                
0x0F120100,	//TVAR_afit_pBaseVals[311]   70000A96                //"RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                              
0x0F128064,	//40 TVAR_afit_pBaseVals[312]   70000A98                //"byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                                  
0x0F120180,	//TVAR_afit_pBaseVals[313]   70000A9A                //"RGB2YUV_iSaturationRGB2YUV_bGainOffset"                                  
0x0F120000,	//TVAR_afit_pBaseVals[314]   70000A9C                //RGB2YUV_iYOffset                                                            
//	param_end	TVAR_afit_pBaseVals
//	param_start	afit_pConstBaseVals
0x0F1200FF,	//afit_pConstBaseVals[0]                            //Denoise1_iUVDenThreshLow                             
0x0F1200FF,	//afit_pConstBaseVals[1]                            //Denoise1_iUVDenThreshHigh                            
0x0F120800,	//afit_pConstBaseVals[2]                            //Denoise1_sensor_width                                
0x0F120600,	//afit_pConstBaseVals[3]                            //Denoise1_sensor_height                               
0x0F120000,	//afit_pConstBaseVals[4]                            //Denoise1_start_x                                     
0x0F120000,	//afit_pConstBaseVals[5]                            //Denoise1_start_y                                     
0x0F120000,	//afit_pConstBaseVals[6]                            //"Denoise1_iYDenSmoothDenoise1_iWSharp  "             
0x0F120300,	//afit_pConstBaseVals[7]                            //"Denoise1_iWWSharp Denoise1_iRadialTune  "           
0x0F120002,	//afit_pConstBaseVals[8]                            //"Denoise1_iOutputBrightnessDenoise1_binning_x  "     
0x0F120400,	//afit_pConstBaseVals[9]                            //"Denoise1_binning_yDemosaic4_iFDeriv  "              
0x0F120106,	//afit_pConstBaseVals[10]                           //"Demosaic4_iFDerivNeiDemosaic4_iSDeriv  "            
0x0F120005,	//afit_pConstBaseVals[11]                           //"Demosaic4_iSDerivNeiDemosaic4_iEnhancerG  "         
0x0F120000,	//afit_pConstBaseVals[12]                           //"Demosaic4_iEnhancerRBDemosaic4_iEnhancerV  "        
0x0F120703,	//afit_pConstBaseVals[13]                           //"Demosaic4_iDecisionThreshDemosaic4_iDesatThresh"    
0x0F120000,	//afit_pConstBaseVals[14]                           //Demosaic4_iBypassSelect                              
0x0F12FFD6,	//afit_pConstBaseVals[15]
0x0F1253C1,	//afit_pConstBaseVals[16]//hys off : 4341
0x0F12E1FE,	//afit_pConstBaseVals[17]//mixer on :E0FA
0x0F120001,	//afit_pConstBaseVals[18]
//	param_end	afit_pConstBaseVals
 
//////////////////////////
//Flicker, CLK //
//////////////////////////

// End tuning part
0x10000001,	// Set host interrupt so main start run
0xFFFF0064, // Wait10mSec

//////////////////////////
// Anti-Flicker //
//////////////////////////

// End user init script
0x002A0400,                                  
0x0F12007F,  //REG_TC_DBG_AutoAlgEnBits default : 007F Manual : 005F

0x002A0B2A,                                  
0x0F120001,   //AFC_Default60Hz; 60Hz:01 50Hz:00

//0x002A03DC, // Manual setting ½Ã ÇÊ¿ä                    
//0x0F120001, //REG_SF_USER_FlickerQuant  60Hz:02 50Hz:01  
//0x0F120001,  //REG_SF_USER_FlickerQuantChanged           

//clk Setting

0x002A01B8, 
0x0F126590, //26Mhz
0x0F120000, 
0x002A01C6, 
0x0F120001, 
0x0F120001,  //1 MIPI configurations (mipi clk enable)
0x002A01CC, 
0x0F121b58,  //1 system clk 26Mhz
0x0F1236B0, //REG_TC_IPRM_MinOutRate4KHz_0
0x0F1236B0, //REG_TC_IPRM_MinOutRate4KHz_0
0x0F121b58, 
0x0F1236B0, 
0x0F1236B0, 
0x002A01E0, 
0x0F120001, 
// p100      ,
0xFFFF0064,

//////////,/////////////////////////////////
//PREVIEW ,CONFIGURATION 0 (SXGA, YUV, 15fps)

0x002A0242,                                 
0x0F120280,	//1280                          
0x0F1201e0,	//1024                          
0x0F120005,	//YUV                           
0x002A024E,                                 
0x0F120000,                                 
0x002A0248,                                 
0x0F1236B0, //REG_1TC_PCFG_usMaxOut4KHzRate	
0x0F1236B0, //REG_1TC_PCFG_usMaxOut4KHzRate	
0x0F120052, // PVI Mask                     
0x002A0252,                                 
0x0F120002,                                 
0x002A0250,                                 
0x0F120002,                                 
0x002A0254,

0x0F12014D, // 0x29a = 15fps, 0x14d = 30fps                                  

0x0F120000,   

0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,
0x0F120000,                              

///////////////////////////////////////////
//PREVIEW CONFIGURATION 1 (VGA, YUV, 30fps)

0x0F120280,	//640
0x0F1201E0,	//480
// 0x0F120140,	//320
// 0x0F1200F0,	//240
0x0F120005,	//YUV
0x002A0274,
0x0F120001,
0x002A026E,
0x0F1236B0, //REG_1TC_PCFG_usMaxOut4KHzRate		//PCLK max
0x0F1236B0, //REG_1TC_PCFG_usMaxOut4KHzRate		//PCLK min
0x0F120052, // PVI Mask
0x002A0278,
0x0F120001,
0x002A0276,
0x0F120002,
0x002A027A,

0x0F12014D, // 0x29a = 15fps, 0x14d = 30fps 

0x0F120000,

//PREVIEW 

0x002A03B6,
0x0F120000,
0x002A03FA,
0x0F120001, // Lane ???
0x0F1200C3,
0x0F120001,

0x002A021C,
0x0F120001,
0x002A0220,
0x0F120001,
0x002A01F8,
0x0F120001,
0x002A021E,
0x0F120001,
0x002A01F0,
0x0F120001,
0x0F120001,

// p500 //   ,
0xFFFF01F4,

// MIPI RX.. type setting gated ?? or continus ??
// 0x0028D000,
// 0x002AB0B2,
// 0x0F120020,


CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA PREVIEW(640*480 / FULL)
//==========================================================
GLOBAL const U32 reg_sub_preview[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// SNAPSHOT
//==========================================================

GLOBAL const U32 reg_sub_snapshot[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// MIDLIGHT SNAPSHOT =======> Flash Low light snapshot
//==========================================================
GLOBAL const U32 reg_sub_midlight[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// LOWLIGHT SNAPSHOT
//==========================================================
GLOBAL const U32 reg_sub_lowlight[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;



/*****************************************************************/
/*********************** N I G H T  M O D E **********************/
/*****************************************************************/

//==========================================================
// CAMERA NIGHTMODE ON
//==========================================================
GLOBAL const U32 reg_sub_nightshot_on[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA NIGHTMODE OFF
//==========================================================
GLOBAL const U32 reg_sub_nightshot_off[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// NIGHT-MODE SNAPSHOT
//==========================================================
GLOBAL const U32 reg_sub_nightshot[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_AUTO (1/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_auto[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_DAYLIGHT (2/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_daylight[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_CLOUDY_DAYLIGHT (3/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_cloudy[]
#if defined(_CAMACQ_API_C_)
={


CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_INCANDESCENT (4/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_incandescent[]
#if defined(_CAMACQ_API_C_)
={


CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_FLUORESCENT (5/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_fluorescent[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_TWILIGHT (6/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_twilight[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_TUNGSTEN (7/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_tungsten[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_OFF (8/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_off[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_OFF (9/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_horizon[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_WB_OFF (10/10)
//==========================================================
GLOBAL const U32 reg_sub_wb_shade[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_OFF (1/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_none[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_MONO (2/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_gray[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_NEGATIVE (3/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_negative[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_SOLARIZE (4/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_solarize[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_SEPIA (5/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_sepia[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_POSTERIZE (6/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_posterize[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_WHITEBOARD (7/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_whiteboard[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_BLACKBOARD (8/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_blackboard[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_AQUA (9/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_aqua[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_AQUA (10/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_sharpen[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_AQUA (11/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_vivid[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_AQUA (12/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_green[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_EFFECT_AQUA (13/13)
//==========================================================
GLOBAL const U32 reg_sub_effect_sketch[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_AEC_MATRIX_METERING (2/2)
//==========================================================
GLOBAL const U32 reg_sub_meter_matrix[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_AEC_CENTERWEIGHTED_METERING (2/2)
//==========================================================
GLOBAL const U32 reg_sub_meter_cw[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_AEC_SPOT_METERING (1/2)
//==========================================================
GLOBAL const U32 reg_sub_meter_spot[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_AEC_FRAME_AVERAGE (2/2)
//==========================================================
GLOBAL const U32 reg_sub_meter_frame[]
#if defined(_CAMACQ_API_C_)
={

0x7700, 
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_FLIP_NONE (1/4)
//==========================================================
GLOBAL const U32 reg_sub_flip_none[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_FLIP_VERTICAL (2/4)
//==========================================================
GLOBAL const U32 reg_sub_flip_water[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_FLIP_HORIZ (3/4)
//==========================================================
GLOBAL const U32 reg_sub_flip_mirror[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_FLIP_SYMMETRIC (4/4)
//==========================================================
GLOBAL const U32 reg_sub_flip_water_mirror[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_FRAMERATE_7FPS
//==========================================================
GLOBAL const U32 reg_sub_fps_fixed_7[]
#if defined(_CAMACQ_API_C_)
={


CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_FRAMERATE_10FPS
//==========================================================
GLOBAL const U32 reg_sub_fps_fixed_10[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_FRAMERATE_15FPS
//==========================================================
GLOBAL const U32 reg_sub_fps_fixed_15[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_FRAMERATE_AUTO_MAX15(5~15fps)
//==========================================================
GLOBAL const U32 reg_sub_fps_var_15[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_BRIGHTNESS_LEVEL1 (1/9) : ï¿½ï¿½ï¿½ï¿½ 
//==========================================================
GLOBAL const U32 reg_sub_brightness_level_0[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_BRIGHTNESS_LEVEL1 (2/9)
//==========================================================
GLOBAL const U32 reg_sub_brightness_level_1[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_BRIGHTNESS_LEVEL1 (3/9)
//==========================================================
GLOBAL const U32 reg_sub_brightness_level_2[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_BRIGHTNESS_LEVEL1 (4/9)
//==========================================================
GLOBAL const U32 reg_sub_brightness_level_3[]
#if defined(_CAMACQ_API_C_)
={


CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_BRIGHTNESS_LEVEL1 (5/9)
//==========================================================
GLOBAL const U32 reg_sub_brightness_level_4[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_BRIGHTNESS_LEVEL1 (6/9)
//==========================================================
GLOBAL const U32 reg_sub_brightness_level_5[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_BRIGHTNESS_LEVEL1 (7/9)
//==========================================================
GLOBAL const U32 reg_sub_brightness_level_6[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_BRIGHTNESS_LEVEL1 (8/9)
//==========================================================
GLOBAL const U32 reg_sub_brightness_level_7[]
#if defined(_CAMACQ_API_C_)
={


CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_BRIGHTNESS_LEVEL1 (9/9)
//==========================================================
GLOBAL const U32 reg_sub_brightness_level_8[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_AF
//==========================================================
GLOBAL const U32 reg_sub_set_af[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_off_af[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_check_af[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_reset_af []
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_manual_af []
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_macro_af []
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_return_manual_af []
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_return_macro_af []
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_set_af_nlux []
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_set_af_llux []
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_ISO_AUTO
//==========================================================
GLOBAL const U32 reg_sub_iso_auto[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_ISO_50
//==========================================================
GLOBAL const U32 reg_sub_iso_50[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_ISO_100
//==========================================================
GLOBAL const U32 reg_sub_iso_100[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_ISO_200
//==========================================================
GLOBAL const U32 reg_sub_iso_200[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_ISO_400
//==========================================================
GLOBAL const U32 reg_sub_iso_400[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_ISO_800
//==========================================================
GLOBAL const U32 reg_sub_iso_800[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_ISO_1600
//==========================================================
GLOBAL const U32 reg_sub_iso_1600[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_ISO_1600
//==========================================================
GLOBAL const U32 reg_sub_iso_3200[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_AUTO (OFF)
//==========================================================
GLOBAL const U32 reg_sub_scene_auto[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_NIGHT
//==========================================================
GLOBAL const U32 reg_sub_scene_night[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_LANDSCAPE
//==========================================================
GLOBAL const U32 reg_sub_scene_landscape[]
#if defined(_CAMACQ_API_C_)
={      

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_SUNSET
//==========================================================
GLOBAL const U32 reg_sub_scene_sunset[]
#if defined(_CAMACQ_API_C_)
={      

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_PORTRAIT
//==========================================================
GLOBAL const U32 reg_sub_scene_portrait[]
#if defined(_CAMACQ_API_C_)
={ 
CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_SUNRISE
//==========================================================
GLOBAL const U32 reg_sub_scene_sunrise[]
#if defined(_CAMACQ_API_C_)
={      

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_INDOOR
//==========================================================
GLOBAL const U32 reg_sub_scene_indoor[]
#if defined(_CAMACQ_API_C_)
={      

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_PARTY
//==========================================================
GLOBAL const U32 reg_sub_scene_party[]
#if defined(_CAMACQ_API_C_)
={      

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_SPORTS
//==========================================================
GLOBAL const U32 reg_sub_scene_sports[]
#if defined(_CAMACQ_API_C_)
={  

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_BEACH
//==========================================================
GLOBAL const U32 reg_sub_scene_beach[]
#if defined(_CAMACQ_API_C_)
={  

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_SNOW
//==========================================================
GLOBAL const U32 reg_sub_scene_snow[]
#if defined(_CAMACQ_API_C_)
={  

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_SCENE_FALLCOLOR
//==========================================================
GLOBAL const U32 reg_sub_scene_fallcolor[]
#if defined(_CAMACQ_API_C_)
={  

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_FIREWORKS
//==========================================================
GLOBAL const U32 reg_sub_scene_fireworks[]
#if defined(_CAMACQ_API_C_)
={  

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_CANDLELIGHT
//==========================================================
GLOBAL const U32 reg_sub_scene_candlelight[]
#if defined(_CAMACQ_API_C_)
={  

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_AGAINSTLIGHT
//==========================================================
GLOBAL const U32 reg_sub_scene_againstlight[]
#if defined(_CAMACQ_API_C_)
={  

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_SCENE_TEXT
//==========================================================
GLOBAL const U32 reg_sub_scene_text[]
#if defined(_CAMACQ_API_C_)
={

CAMACQ_SUB_BTM_OF_DATA,
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_output_qqvga
//==========================================================
GLOBAL const U32 reg_sub_qqvga[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_output_qcif
//==========================================================
GLOBAL const U32 reg_sub_qcif[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_output_qvga
//==========================================================
GLOBAL const U32 reg_sub_qvga[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_output_wqvga
//==========================================================
GLOBAL const U32 reg_sub_wqvga[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_output_cif
//==========================================================
GLOBAL const U32 reg_sub_cif[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_output_vga
//==========================================================
GLOBAL const U32 reg_sub_vga[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_output_wvga
//==========================================================
GLOBAL const U32 reg_sub_wvga[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


//==========================================================
// CAMERA_output_svga
//==========================================================
GLOBAL const U32 reg_sub_svga[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_output_sxga
//==========================================================
GLOBAL const U32 reg_sub_sxga[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_output_qxga
//==========================================================
GLOBAL const U32 reg_sub_qxga[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_output_full_yuv
//==========================================================
GLOBAL const U32 reg_sub_full_yuv[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

//==========================================================
// CAMERA_output_cropped_yuv
//==========================================================
GLOBAL const U32 reg_sub_crop_yuv[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

// NEW
GLOBAL const U32 reg_sub_fmt_yuv422[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


GLOBAL const U32 reg_sub_fmt_jpg[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


GLOBAL const U32 reg_sub_qvga_v[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


GLOBAL const U32 reg_sub_half_vga_v[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


GLOBAL const U32 reg_sub_half_vga[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


GLOBAL const U32 reg_sub_vga_v[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


GLOBAL const U32 reg_sub_5M[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;


GLOBAL const U32 reg_sub_1080P[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_720P[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_flicker_disabled[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_flicker_50hz[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_flicker_60hz[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

GLOBAL const U32 reg_sub_flicker_auto[]
#if defined(_CAMACQ_API_C_)
={
CAMACQ_SUB_BTM_OF_DATA
}
#endif /* _CAMACQ_API_C_ */
;

#undef GLOBAL

#endif /* _CAMACQ_S5K6AAFX13_H_ */
