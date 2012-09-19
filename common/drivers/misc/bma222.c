/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2010 Bosch Sensortec GmbH
 * All Rights Reserved
*/


/*************************************************************************************************/
/* EasyCASE ) */
/* EasyCASE ( 100012
   File Name For Doxy */
/*! \file bma222.c
    \brief Sensor Driver for BMA222 Triaxial acceleration sensor */
/* EasyCASE ) */
/* EasyCASE ( 247
   Includes */
/* EasyCASE ( 912
   Standard includes */
//#include "API.h"
/* EasyCASE ) */
/* EasyCASE ( 100013
   Module includes */
#include <linux/bma222.h>
/* EasyCASE ) */
/* EasyCASE ) */
/* EasyCASE ( 63
   Local Declarations and Definitions */
/* EasyCASE ( 322
   Protected Data Interfaces (PDIs) */
/* EasyCASE ) */
/* EasyCASE ( 251
   #Define Constants */
/* user defined code to be added here ... */

//Example....
//#define YOUR_H_DEFINE         /**< <Description for YOUR_H_DEFINE> */
/* EasyCASE ) */
/* EasyCASE ( 222
   ENUM Definitions */
/* EasyCASE < */
/* user defined code to be added here ... */

//Example...
//typedef enum {
//E_YOURDATA1, /**< <DOXY Comment for E_YOURDATA1> */
//E_YOURDATA2  /**< <DOXY Comment for E_YOURDATA2> */
//}te_YourData;
/* EasyCASE > */
/* EasyCASE ) */
/* EasyCASE ( 217
   Variable Definitions */
/* user defined code to be added here ... */

//Example.....
//char v_Variable_u8r /**\brief <Description for v_Variable_u8r>*/


bma222_t * p_bma222;
/* EasyCASE ) */
/* EasyCASE ( 223
   Constant- and Table-Definitions in ROM/EEPROM */
/* user defined code to be added here ... */

//Example....
//char const C_ConstData /**\brief <Description for C_ConstData> */
/* EasyCASE ) */
/* EasyCASE ( 252
   Functions Prototype Declaration */
/* user defined code to be added here ... */
//Example...
//void Local_1(void);       /**\brief <Description for Local_1 function> */
/* EasyCASE ) */
/* EasyCASE ) */
/* EasyCASE ( 70
   API-Interface functions */
/* EasyCASE ( 100017
   bma222_init */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief API Initialization routine
 *
 *
 *
 *
 *  \param *bma222 pointer to bma222 structured type
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_init(bma222_t *bma222)
   {
   int comres= C_Zero_U8X ;
   unsigned char data;
   
   p_bma222 = bma222;                                                                                                                                                      /* assign bma222 ptr */
   p_bma222->dev_addr = BMA222_I2C_ADDR;                                                   /* preset bma222 I2C_addr */
   comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_CHIP_ID__REG, &data, 1);     /* read Chip Id */
   
   p_bma222->chip_id = data ;                                          /* get bitslice */
   
   comres += p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_ML_VERSION__REG, &data, 1); /* read Version reg */
   p_bma222->ml_version = bma222_GET_BITSLICE(data, bma222_ML_VERSION);                            /* get ML Version */
   p_bma222->al_version = bma222_GET_BITSLICE(data, bma222_AL_VERSION);                            /* get AL Version */
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100018
   bma222_soft_reset */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  Perform soft reset of bma222 via bus command
 *
 *
 *
 *
 *  \param None
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_soft_reset(void)
   {
   int comres = C_Zero_U8X ;
   unsigned char data = bma222_EN_SOFT_RESET_VALUE ;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SOFT_RESET__REG,&data ,C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100019
   bma222_write_reg */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API writes given data to the register
 *
 *
 *
 *
 *  \param       unsigned char addr  --> Address of the register to be written
 *               unsigned char *data --> Data to be written
 *               unsigned char len   --> Length of the data
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_write_reg(unsigned char addr, unsigned char *data, unsigned char len)
   {
   int comres = C_Zero_U8X ;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, addr, data, len);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100020
   bma222_read_reg */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API reads data from the given register
 *
 *
 *
 *
 *  \param       unsigned char addr  --> Address of the register to be read
 *               unsigned char *data --> Read data will be placed in the pointer to variable.
 *               unsigned char len   --> Length of the data
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_read_reg(unsigned char addr, unsigned char *data, unsigned char len)
   {
   int comres = C_Zero_U8X ;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, addr, data, len);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100021
   bma222_set_mode */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API sets the function mode of the sensor
 *
 *
 *
 *
 *  \param       unsigned char mode  --> value to be set in mode register.
 *                        mode --> 0 --> Normal mode
 *                        mode --> 1 --> Low Power
 *                        mode --> 2 --> Suspend
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_mode(unsigned char Mode)
   {
   int comres=C_Zero_U8X ;
   unsigned char data1;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (Mode < C_Three_U8X)
         {
         comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_LOW_POWER__REG, &data1, C_One_U8X );
         switch (Mode)
            {
            case bma222_MODE_NORMAL:
               data1  = bma222_SET_BITSLICE(data1, bma222_EN_LOW_POWER, C_Zero_U8X);
               data1  = bma222_SET_BITSLICE(data1, bma222_EN_SUSPEND, C_Zero_U8X);
               break;
            case bma222_MODE_LOWPOWER:
               data1  = bma222_SET_BITSLICE(data1, bma222_EN_LOW_POWER, C_One_U8X);
               data1  = bma222_SET_BITSLICE(data1, bma222_EN_SUSPEND, C_Zero_U8X);
               break;
            case bma222_MODE_SUSPEND:
               data1  = bma222_SET_BITSLICE(data1, bma222_EN_LOW_POWER, C_Zero_U8X);
               data1  = bma222_SET_BITSLICE(data1, bma222_EN_SUSPEND, C_One_U8X);
               break;
            default:
               break;
            }
         comres += p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_LOW_POWER__REG, &data1, C_One_U8X);
         p_bma222->mode = Mode;
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100022
   bma222_get_mode */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API gets the function mode of the sensor
 *
 *
 *
 *
 *  \param       unsigned char *mode  --> value read from the mode register.
 *                        mode --> 00 --> Normal mode
 *                        mode --> 01 --> Low Power
 *                        mode --> 10 --> Suspend
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_mode(unsigned char * Mode )
   {
   int comres= C_Zero_U8X;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_LOW_POWER__REG, Mode, C_One_U8X );
      *Mode  = (*Mode) >> C_Six_U8X;
      /* EasyCASE - */
      p_bma222->mode = *Mode;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100044
   bma222_set_range */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the g-range
 *
 *
 *
 *
 *  \param Possible Values :
 *               unsigned char Range
 *                              0           +/-2 g
 *                              1           +/-4 g
 *                              2           +/-8 g
 *                              3           +/-16 g
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
unsigned char bma222_set_range(unsigned char Range)
   {
   int comres=C_Zero_U8X ;
   unsigned char data1;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (Range < C_Four_U8X)
         {
         comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_RANGE_SEL_REG, &data1, C_One_U8X );
         switch (Range)
            {
            case C_Zero_U8X:
               data1  = bma222_SET_BITSLICE(data1, bma222_RANGE_SEL, C_Zero_U8X);
               break;
            case C_One_U8X:
               data1  = bma222_SET_BITSLICE(data1, bma222_RANGE_SEL, C_Five_U8X);
               break;
            case C_Two_U8X:
               data1  = bma222_SET_BITSLICE(data1, bma222_RANGE_SEL, C_Eight_U8X);
               break;
            case C_Three_U8X:
               data1  = bma222_SET_BITSLICE(data1, bma222_RANGE_SEL, C_Twelve_U8X);
               break;
            default:
               break;
            }
         comres += p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_RANGE_SEL_REG, &data1, C_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100024
   bma222_get_range */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the g-range currently set
 *
 *
 *
 *
 *  \param Possible Values :
 *               unsigned char *Range
 *                              0101           +/-4 g
 *                              1000           +/-8 g
 *                              1100           +/-16 g
 *                              others         +/-2 g
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_range(unsigned char * Range )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_RANGE_SEL__REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_RANGE_SEL);
      *Range = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100025
   bma222_set_bandwidth */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the bandwidth
 *
 *
 *
 *
 *  \param Possible Values :
 *               unsigned char BW
 *
 *               bw       Selected Bandwidth[Hz]       Tupdate [?s]
 *              =================================================
 *               0            7.81                       64000
 *               1            15.63                      32000
 *               2            31.25                      16000
 *               3            62.50                      8000
 *               4            125                        4000
 *               5            250                        2000
 *               6            500                        1000
 *               7            1000                       500
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_bandwidth(unsigned char BW)
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   int Bandwidth ;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (BW < C_Eight_U8X)
         {
         switch (BW)
            {
            case C_Zero_U8X:
               Bandwidth = bma222_BW_7_81HZ;
               
               /*  7.81 Hz      64000 uS   */
               break;
            case C_One_U8X:
               Bandwidth = bma222_BW_15_63HZ;
               
               /*  15.63 Hz     32000 uS   */
               break;
            case C_Two_U8X:
               Bandwidth = bma222_BW_31_25HZ;
               
               /*  31.25 Hz     16000 uS   */
               break;
            case C_Three_U8X:
               Bandwidth = bma222_BW_62_50HZ;
               
               /*  62.50 Hz     8000 uS   */
               break;
            case C_Four_U8X:
               Bandwidth = bma222_BW_125HZ;
               
               /*  125 Hz       4000 uS   */
               break;
            case C_Five_U8X:
               Bandwidth = bma222_BW_250HZ;
               
               /*  250 Hz       2000 uS   */
               break;
            case C_Six_U8X:
               Bandwidth = bma222_BW_500HZ;
               
               /*  500 Hz       1000 uS   */
               break;
            case C_Seven_U8X:
               Bandwidth = bma222_BW_1000HZ;
               
               /*  1000 Hz      500 uS   */
               break;
            default:
               break;
            }
         comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_BANDWIDTH__REG, &data, C_One_U8X );
         data = bma222_SET_BITSLICE(data, bma222_BANDWIDTH, Bandwidth );
         comres += p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_BANDWIDTH__REG, &data, C_One_U8X );
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100026
   bma222_get_bandwidth */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the bandwidth
 *
 *
 *
 *
 *  \param Possible Values :
 *               unsigned char *BW
 *
 *               bw       Selected Bandwidth[Hz]       Tupdate [?s]
 *              =================================================
 *               0            7.81                       64000
 *               1            15.63                      32000
 *               2            31.25                      16000
 *               3            62.50                      8000
 *               4            125                        4000
 *               5            250                        2000
 *               6            500                        1000
 *               7            1000                       500
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_bandwidth(unsigned char *BW)
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_BANDWIDTH__REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_BANDWIDTH);
      if (data <= C_Eight_U8X)
         {
         *BW = C_Zero_U8X;
         }
      else
         {
         if (data >= 0x0F)
            {
            *BW = C_Seven_U8X;
            }
         else
            {
            *BW = data - C_Eight_U8X;
            }
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100027
   bma222_read_accel_xyz */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API gets the X,Y and Z-axis acceleration data readout
 *
 *
 *
 *
 *  \param    *acc pointer to \ref bma222acc_t structure for x,y,z data readout
 *          \note data will be read by multi-byte protocol into a 6 byte structure
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_read_accel_xyz(bma222acc_t * acc)
   {
   int comres;
   unsigned char data[6];
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, BMA222_ACC_X8_LSB__REG, data, 6);
      acc->x = bma222_GET_BITSLICE(data[0],BMA222_ACC_X8_LSB)| (bma222_GET_BITSLICE(data[1],BMA222_ACC_X_MSB)<<(BMA222_ACC_X8_LSB__LEN));
      acc->x = acc->x << (sizeof(short)*8-(BMA222_ACC_X8_LSB__LEN + BMA222_ACC_X_MSB__LEN));
      acc->x = acc->x >> (sizeof(short)*8-(BMA222_ACC_X8_LSB__LEN + BMA222_ACC_X_MSB__LEN));
      
      acc->y = bma222_GET_BITSLICE(data[2],BMA222_ACC_Y8_LSB)| (bma222_GET_BITSLICE(data[3],BMA222_ACC_Y_MSB)<<(BMA222_ACC_Y8_LSB__LEN ));
      acc->y = acc->y << (sizeof(short)*8-(BMA222_ACC_Y8_LSB__LEN + BMA222_ACC_Y_MSB__LEN));
      acc->y = acc->y >> (sizeof(short)*8-(BMA222_ACC_Y8_LSB__LEN + BMA222_ACC_Y_MSB__LEN));
      
      acc->z = bma222_GET_BITSLICE(data[4],BMA222_ACC_Z8_LSB)| (bma222_GET_BITSLICE(data[5],BMA222_ACC_Z_MSB)<<(BMA222_ACC_Z8_LSB__LEN));
      acc->z = acc->z << (sizeof(short)*8-(BMA222_ACC_Z8_LSB__LEN + BMA222_ACC_Z_MSB__LEN));
      acc->z = acc->z >> (sizeof(short)*8-(BMA222_ACC_Z8_LSB__LEN + BMA222_ACC_Z_MSB__LEN));
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100028
   bma222_read_accel_x */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API gets the X axis acceleration data
 *
 *
 *
 *
 *  \param  *a_x pointer for 16 bit 2's compliment data output (LSB aligned)
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_read_accel_x(short *a_x)
   {
   int comres;
   unsigned char data[2];
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, BMA222_ACC_X8_LSB__REG, data, 2);
      *a_x = bma222_GET_BITSLICE(data[0],BMA222_ACC_X8_LSB)| (bma222_GET_BITSLICE(data[1],BMA222_ACC_X_MSB)<<(BMA222_ACC_X8_LSB__LEN));
      *a_x = *a_x << (sizeof(short)*8-(BMA222_ACC_X8_LSB__LEN + BMA222_ACC_X_MSB__LEN));
      *a_x = *a_x >> (sizeof(short)*8-(BMA222_ACC_X8_LSB__LEN + BMA222_ACC_X_MSB__LEN));
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100029
   bma222_read_accel_y */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API gets the Y axis acceleration data
 *
 *
 *
 *
 *  \param  *a_y pointer for 16 bit 2's compliment data output (LSB aligned)
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_read_accel_y(short *a_y)
   {
   int comres;
   unsigned char data[2];
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, BMA222_ACC_Y8_LSB__REG, data, 2);
      *a_y = bma222_GET_BITSLICE(data[0],BMA222_ACC_Y8_LSB)| (bma222_GET_BITSLICE(data[1],BMA222_ACC_Y_MSB)<<(BMA222_ACC_Y8_LSB__LEN ));
      *a_y = *a_y << (sizeof(short)*8-(BMA222_ACC_Y8_LSB__LEN + BMA222_ACC_Y_MSB__LEN));
      *a_y = *a_y >> (sizeof(short)*8-(BMA222_ACC_Y8_LSB__LEN + BMA222_ACC_Y_MSB__LEN));
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100030
   bma222_read_accel_z */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API gets the Z axis acceleration data
 *
 *
 *
 *
 *  \param  *a_z pointer for 16 bit 2's compliment data output (LSB aligned)
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_read_accel_z(short *a_z)
   {
   int comres;
   unsigned char data[2];
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, BMA222_ACC_Z8_LSB__REG, data, 2);
      *a_z = bma222_GET_BITSLICE(data[0],BMA222_ACC_Z8_LSB)| (bma222_GET_BITSLICE(data[1],BMA222_ACC_Z_MSB)<<(BMA222_ACC_Z8_LSB__LEN));
      *a_z = *a_z << (sizeof(short)*8-(BMA222_ACC_Z8_LSB__LEN + BMA222_ACC_Z_MSB__LEN));
      *a_z = *a_z >> (sizeof(short)*8-(BMA222_ACC_Z8_LSB__LEN + BMA222_ACC_Z_MSB__LEN));
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100032
   bma222_reset_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API resets the interrupt
 *
 *
 *
 *
 *  \param  none
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_reset_interrupt(void)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_RESET_LATCHED__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_INT_RESET_LATCHED, C_One_U8X );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_INT_RESET_LATCHED__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100053
   bma222_get_interruptstatus1 */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the interrupt status1 register
 *
 *
 *
 *
 *  \param unsigned char *intstatus
 *                       Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_interruptstatus1(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS1_REG, &data, C_One_U8X );
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100061
   bma222_get_interruptstatus2 */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the interrupt status2 register
 *
 *
 *
 *
 *  \param      unsigned char *intstatus
 *                       Interrupt status will be stored to pointer to a variable
 *
 *
 *
 *  \return     result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_interruptstatus2(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS2_REG, &data, C_One_U8X );
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100065
   bma222_get_Low_G_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the Low G interrupt
 *
 *
 *
 *
 *  \param unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_Low_G_interrupt(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS1_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_LOWG_INT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100067
   bma222_get_High_G_Interrupt */
/* Compiler Switch if applicable
#ifdef

#endif
*/
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the High G interrupt
 *
 *
 *
 *
 *  \param unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_High_G_Interrupt(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS1_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_HIGHG_INT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100069
   bma222_get_slope_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the slope interrupt
 *
 *
 *
 *
 *  \param unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_slope_interrupt(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS1_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_SLOPE_INT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100071
   bma222_get_double_tap_interrupt */
/* Compiler Switch if applicable
#ifdef

#endif
*/
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the double tap interrupt
 *
 *
 *
 *
 *  \param unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_double_tap_interrupt(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS1_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_DOUBLE_TAP_INT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100073
   bma222_get_single_tap_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the single tap interrupt
 *
 *
 *
 *
 *  \param unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_single_tap_interrupt(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS1_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_SINGLE_TAP_INT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100075
   bma222_get_orient_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the orient interrupt
 *
 *
 *
 *
 *  \param unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_orient_interrupt(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS1_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_ORIENT_INT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100079
   bma222_get_flat_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the flat interrupt
 *
 *
 *
 *
 *  \param unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_flat_interrupt(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS1_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_FLAT_INT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100083
   bma222_get_data_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the data interrupt
 *
 *
 *
 *
 *  \param unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_data_interrupt(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS2_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_DATA_INT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100087
   bma222_get_slope_first */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the slope first
 *
 *
 *
 *
 *  \param
 *            unsigned char param
 *                        0 -- > Slope First X
 *                        1 -- > Slope First Y
 *                        2 -- > Slope First Z
 *
 *            unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_slope_first(unsigned char param,unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_TAP_SLOPE_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_SLOPE_FIRST_X);
            *intstatus = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_TAP_SLOPE_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_SLOPE_FIRST_Y);
            *intstatus = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_TAP_SLOPE_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_SLOPE_FIRST_Z);
            *intstatus = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100096
   bma222_get_slope_sign */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the slope sign
 *
 *
 *
 *
 *  \param
 *            unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_slope_sign(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_TAP_SLOPE_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_SLOPE_SIGN_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100097
   bma222_get_tap_first */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the tap first
 *
 *
 *
 *
 *  \param
 *            unsigned char param
 *                        0 -- > Tap First X
 *                        1 -- > Tap First Y
 *                        2 -- > Tap First Z
 *
 *            unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_tap_first(unsigned char param,unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_TAP_SLOPE_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_TAP_FIRST_X);
            *intstatus = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_TAP_SLOPE_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_TAP_FIRST_Y);
            *intstatus = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_TAP_SLOPE_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_TAP_FIRST_Z);
            *intstatus = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100101
   bma222_get_tap_sign */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the tap sign
 *
 *
 *
 *
 *  \param
 *            unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_tap_sign(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_TAP_SLOPE_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_TAP_SIGN_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100102
   bma222_get_HIGH_first */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the HIGH first
 *
 *
 *
 *
 *  \param
 *            unsigned char param
 *                        0 -- > High g First X
 *                        1 -- > High g First Y
 *                        2 -- > High g First Z
 *
 *            unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_HIGH_first(unsigned char param,unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_ORIENT_HIGH_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_HIGHG_FIRST_X);
            *intstatus = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_ORIENT_HIGH_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_HIGHG_FIRST_Y);
            *intstatus = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_ORIENT_HIGH_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_HIGHG_FIRST_Z);
            *intstatus = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100106
   bma222_get_HIGH_sign */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the HIGH sign
 *
 *
 *
 *
 *  \param
 *
 *            unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_HIGH_sign(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_ORIENT_HIGH_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_HIGHG_SIGN_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100107
   bma222_get_orient_status */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the orient status
 *
 *
 *
 *
 *  \param
 *
 *            unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_orient_status(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_ORIENT_HIGH_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_ORIENT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100108
   bma222_get_orient_flat_status */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the orient flat status
 *
 *
 *
 *
 *  \param
 *
 *            unsigned char *intstatus
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_orient_flat_status(unsigned char *intstatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_ORIENT_HIGH_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_FLAT_S);
      *intstatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100113
   bma222_get_sleep_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the sleep duration
 *
 *
 *
 *
 *  \param
 *
 *            unsigned char *sleep
 *              Interrupt status will be stored to pointer to a variable
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_sleep_duration(unsigned char *sleep )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_MODE_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_SLEEP_DUR);
      *sleep = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100122
   bma222_set_sleep_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the sleep duration
 *
 *
 *
 *
 *  \param              unsigned char sleepdur
 *
 *
 *                     sleepdur
 *                      0000 --> 0.5ms
 *                      0001 --> 0.5ms
 *                      0010 --> 0.5ms
 *                      0011 --> 0.5ms
 *                      0100 --> 0.5ms
 *                      0101 --> 0.5ms
 *                      0110 --> 1ms
 *                      0111 --> 2ms
 *                      1000 --> 4ms
 *                      1001 --> 6ms
 *                      1010 --> 10ms
 *                      1011 --> 25ms
 *                      1100 --> 50ms
 *                      1101 --> 100ms
 *                      1110 --> 500ms
 *                      1111 --> 1s
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_sleep_duration(unsigned char sleepdur )
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SLEEP_DUR__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_SLEEP_DUR, sleepdur );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_SLEEP_DUR__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100130
   bma222_set_suspend */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets/Resets the Suspend mode
 *
 *
 *
 *
 *  \param
 *                 unsigned char State:
 *                             0 -- > Disable suspend mode
 *                             1 -- > Enable suspend mode
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return     result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_suspend(unsigned char state)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SUSPEND__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_EN_SUSPEND, state );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SUSPEND__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100428
   bma222_get_suspend */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API gets the suspend mode register bit
 *
 *
 *
 *
 *  \param unsigned char *status
 *                  Read status will be placed in pointer to a variable
 *
 *
 *  \return  result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_suspend(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_MODE_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_EN_SUSPEND);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100126
   bma222_set_lowpower */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets/resets the Lowpower register bit
 *
 *
 *
 *
 *  \param
 *
 *             unsigned char state:
 *                     State:
 *                             0 -- > Resets the Low power register bit
 *                             1 -- > Sets the Low power regsiter bit
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_lowpower(unsigned char state)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_LOW_POWER__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_EN_LOW_POWER, state );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_LOW_POWER__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100432
   bma222_get_lowpower_en */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the lowpower register bit status
 *
 *
 *
 *
 *  \param unsigned char *status
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_lowpower_en(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_MODE_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_EN_LOW_POWER);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100134
   bma222_set_low_noise_ctrl */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief   This API sets the Low noise ctrl bit
 *
 *
 *
 *
 *  \param
 *              unsigned char state
 *                          State:
 *                             0 -- > Disable
 *                             1 -- > Enable
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_low_noise_ctrl(unsigned char state)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_LOW_NOISE__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_EN_LOW_NOISE, state );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_LOW_NOISE__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100436
   bma222_get_low_noise_ctrl */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API gets the low noise ctrl register bit
 *
 *
 *
 *
 *  \param unsigned char *status
 *                      0 -- > Disabled
 *                      1 -- > Enabled
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_low_noise_ctrl(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_LOW_NOISE_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_EN_LOW_NOISE);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100138
   bma222_set_shadow_disable */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API disables the shadow procedure
 *
 *
 *
 *
 *  \param    None
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_shadow_disable(unsigned char state)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_DIS_SHADOW_PROC__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_DIS_SHADOW_PROC, state );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_DIS_SHADOW_PROC__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100440
   bma222_get_shadow_disable */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the Shadow disable bit status
 *
 *
 *
 *
 *  \param unsigned char *status
 *                    1 --> Shadow procedure disabled
 *                    0 --> Shadow procedure Not disabled
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_shadow_disable(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_DATA_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_DIS_SHADOW_PROC);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100142
   bma222_set_unfilt_acc */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the unfitl_acc
 *                      Unfiltered accelatration data are stored in Data register
 *
 *
 *
 *  \param    None
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_unfilt_acc(unsigned char state)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_UNFILT_ACC__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_EN_UNFILT_ACC, state );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_UNFILT_ACC__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100442
   bma222_get_unfilt_acc */
/* Compiler Switch if applicable
#ifdef

#endif
*/
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the Unfilt_acc register bit status.
 *
 *
 *
 *
 *  \param              unsigned char *status
 *                              0 --> Filtered accel data are stored in the data register
 *                              1 --> UnFiltered accel data are stored in the data register
 *
 *  \return     result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_unfilt_acc(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_DATA_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_EN_UNFILT_ACC);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100146
   bma222_set_enable_slope_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API Enables the slope interrupt
 *
 *
 *
 *
 *  \param
 *                     State:
 *                             0 -- > enable slope x
 *                             1 -- > enable slope y
 *                             2 -- > enable slope z
 *                             3 -- > enable slope XYZ
 *
 *
 *
 *
 *
 *
 *  \return             result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_enable_slope_interrupt(unsigned char slope)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char state;
   state = 0x01;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (slope)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SLOPE_X_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_SLOPE_X_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SLOPE_X_INT__REG, &data, C_One_U8X);
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SLOPE_Y_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_SLOPE_Y_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SLOPE_Y_INT__REG, &data, C_One_U8X);
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SLOPE_Z_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_SLOPE_Z_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SLOPE_Z_INT__REG, &data, C_One_U8X);
            break;
         case C_Three_U8X:
            state = 0x07;
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SLOPE_XYZ_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_SLOPE_XYZ_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SLOPE_XYZ_INT__REG, &data, C_One_U8X);
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100446
   bma222_get_enable_slope_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the status of the slope interrupt
 *
 *
 *
 *
 *  \param
 *              unsigned char param
 *                                      0 -- > EN_SLOPE_X
 *                                      1 -- > EN_SLOPE_Y
 *                                      2 -- > EN_SLOPE_Z
 *
 *              unsigned char *staus
 *                                      status of the slope interrupt
 *
 *
 *
 *
 *  \return       result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_enable_slope_interrupt(unsigned char param,unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE1_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_SLOPE_X_INT);
            *status = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE1_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_SLOPE_Y_INT);
            *status = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE1_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_SLOPE_Z_INT);
            *status = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100165
   bma222_set_enable_tap_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API enables the double tap, single tap , orient and flat interrupt
 *
 *
 *
 *
 *  \param
 *                     State:
 *                             0 -- > enable double tap interrupt
 *                             1 -- > enable single tap interrupt
 *                             2 -- > enable orient interrupt
 *                             3 -- > enable flat interrupt
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_enable_tap_interrupt(unsigned char tapinterrupt)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char state;
   state = C_One_U8X;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (tapinterrupt)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_DOUBLE_TAP_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_DOUBLE_TAP_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_DOUBLE_TAP_INT__REG, &data, C_One_U8X);
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SINGLE_TAP_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_SINGLE_TAP_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SINGLE_TAP_INT__REG, &data, C_One_U8X);
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_ORIENT_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_ORIENT_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_ORIENT_INT__REG, &data, C_One_U8X);
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_FLAT_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_FLAT_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_FLAT_INT__REG, &data, C_One_U8X);
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100454
   bma222_get_enable_tap_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the status of the tap double tap , single tap,orient and flat interrupt
 *
 *
 *
 *
 *  \param unsigned char param
 *                       0 --> Double Tap
 *                       1 --> Single Tap
 *                       2 --> Orient Interrupt
 *                       3 --> Flat interrupt
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_enable_tap_interrupt(unsigned char param,unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE1_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_DOUBLE_TAP_INT);
            *status = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE1_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_SINGLE_TAP_INT);
            *status = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE1_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_ORIENT_INT);
            *status = data;
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE1_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_FLAT_INT);
            *status = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100169
   bma222_set_enable_high_g_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the HIGH G interrupt enable bit
 *
 *
 *
 *
 *  \param
 *                     State:
 *                             0 -- > enable high x
 *                             1 -- > enable high y
 *                             2 -- > enable high z
 *                             3 -- > enable high XYZ
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_enable_high_g_interrupt(unsigned char highinterrupt)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char state;
   state = 0x01;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (highinterrupt)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_HIGHG_X_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_HIGHG_X_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_HIGHG_X_INT__REG, &data, C_One_U8X);
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_HIGHG_Y_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_HIGHG_Y_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_HIGHG_Y_INT__REG, &data, C_One_U8X);
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_HIGHG_Z_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_HIGHG_Z_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_HIGHG_Z_INT__REG, &data, C_One_U8X);
            break;
         case C_Three_U8X:
            state = 0x07;
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_HIGHG_XYZ_INT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_HIGHG_XYZ_INT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_HIGHG_XYZ_INT__REG, &data, C_One_U8X);
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100461
   bma222_get_enable_high_g_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the High G interrupt status bit
 *
 *
 *
 *
 *  \param
 *                     State:
 *                             0 -- > enable high x
 *                             1 -- > enable high y
 *                             2 -- > enable high z
 *                             3 -- > enable high XYZ
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_enable_high_g_interrupt(unsigned char param,unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE2_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_HIGHG_X_INT);
            *status = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE2_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_HIGHG_Y_INT);
            *status = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE2_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_HIGHG_Z_INT);
            *status = data;
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE2_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_HIGHG_XYZ_INT);
            *status = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100173
   bma222_set_enable_low_g_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API enables the low g interrupt
 *
 *
 *
 *
 *  \param              None
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return     result of communication reoutines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_enable_low_g_interrupt(void)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char state;
   state= 0x01;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_LOWG_INT__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_EN_LOWG_INT, state );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_LOWG_INT__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100465
   bma222_get_enable_low_g_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API get the status of low g interrupt
 *
 *
 *
 *
 *  \param unsigned char *status
 *
 *
 *
 *  \return     Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_enable_low_g_interrupt(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE2_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_EN_LOWG_INT);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100179
   bma222_set_enable_data_interrupt */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API enables the data interrupt
 *
 *
 *
 *
 *  \param None
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_enable_data_interrupt(void)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char state;
   state= 0x01;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_NEW_DATA_INT__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_EN_NEW_DATA_INT, state );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_NEW_DATA_INT__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100469
   bma222_get_enable_data_interrupt */
/* Compiler Switch if applicable
#ifdef

#endif
*/
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the enable status of the data interrupt
 *
 *
 *
 *
 *  \param unsigned char *status
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_enable_data_interrupt(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE2_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_EN_NEW_DATA_INT);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100175
   bma222_set_int1_pad_sel */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the  INTL pad select register
 *
 *
 *
 *
 *  \param
 *               unsigned char intlsel
 *
 *                             0 -- > int1_low
 *                             1 -- > int1_High
 *                             2 -- > int1_slope
 *                             3 -- > int1_d_tap
 *                             4 -- > int1_s_tap
 *                             5 -- > int1_orient
 *                             6 -- > int1_flat
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_int1_pad_sel(unsigned char int1sel)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char state;
   state = 0x01;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (int1sel)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_LOWG__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT1_PAD_LOWG, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_LOWG__REG, &data, C_One_U8X);
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_HIGHG__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT1_PAD_HIGHG, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_HIGHG__REG, &data, C_One_U8X);
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_SLOPE__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT1_PAD_SLOPE, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_SLOPE__REG, &data, C_One_U8X);
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_DB_TAP__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT1_PAD_DB_TAP, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_DB_TAP__REG, &data, C_One_U8X);
            break;
         case C_Four_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_SNG_TAP__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT1_PAD_SNG_TAP, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_SNG_TAP__REG, &data, C_One_U8X);
            break;
         case C_Five_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_ORIENT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT1_PAD_ORIENT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_ORIENT__REG, &data, C_One_U8X);
            break;
         case C_Six_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_FLAT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT1_PAD_FLAT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_FLAT__REG, &data, C_One_U8X);
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100473
   bma222_get_int1_pad_sel */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the INTL pad sel register status
 *
 *
 *
 *
 *  \param
 *                   unsigned char intlsel
 *
 *                             0 -- > int1_low
 *                             1 -- > int1_High
 *                             2 -- > int1_slope
 *                             3 -- > int1_d_tap
 *                             4 -- > int1_s_tap
 *                             5 -- > int1_orient
 *                             6 -- > int1_flat
 *
 *              unsigned char *status
 *                             0 --> Disabled
 *                             1 --> Enabled
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_int1_pad_sel(unsigned char param,unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT1_PAD_LOWG);
            *status = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT1_PAD_HIGHG);
            *status = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT1_PAD_SLOPE);
            *status = data;
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT1_PAD_DB_TAP);
            *status = data;
            break;
         case C_Four_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT1_PAD_SNG_TAP);
            *status = data;
            break;
         case C_Five_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT1_PAD_ORIENT);
            *status = data;
            break;
         case C_Six_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT1_PAD_FLAT);
            *status = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100199
   bma222_set_int_data_sel */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the int1 & int2 register
 *
 *
 *
 *
 *  \param
 *                     intsel
 *                             0 -- > int1_data
 *                             1 -- > int2_data
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_int_data_sel(unsigned char intsel)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char state;
   state = C_One_U8X;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (intsel)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_NEWDATA__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT1_PAD_NEWDATA, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT1_PAD_NEWDATA__REG, &data, C_One_U8X);
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_NEWDATA__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT2_PAD_NEWDATA, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_NEWDATA__REG, &data, C_One_U8X);
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100484
   bma222_get_int_data_sel */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the status of int data sel
 *
 *
 *
 *
 *  \param unsigned char param
 *                      0 -- > INT1_PAD_DATA
 *                      1 -- > INT2_PAD_DATA
 *         unsigned char *status
 *                      0 -- > Disbled
 *                      1 -- > Enabled
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_int_data_sel(unsigned char param,unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_DATA_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT1_PAD_NEWDATA);
            *status = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_DATA_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT2_PAD_NEWDATA);
            *status = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100203
   bma222_set_int2_pad_sel */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API enables the int2_pad sel
 *
 *
 *
 *
 *  \param
 *              unsigned char int2sel
 *                    int2sel:
 *                             0 -- > int2_low
 *                             1 -- > int2_High
 *                             2 -- > int2_slope
 *                             3 -- > int2_d_tap
 *                             4 -- > int2_s_tap
 *                             5 -- > int2_orient
 *                             6 -- > int2_flat
 *
 *
 *
 *  \return Result of communication routine
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_int2_pad_sel(unsigned char int2sel)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char state;
   state = C_One_U8X;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (int2sel)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_LOWG__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT2_PAD_LOWG, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_LOWG__REG, &data, C_One_U8X);
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_HIGHG__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT2_PAD_HIGHG, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_HIGHG__REG, &data, C_One_U8X);
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_SLOPE__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT2_PAD_SLOPE, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_SLOPE__REG, &data, C_One_U8X);
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_DB_TAP__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT2_PAD_DB_TAP, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_DB_TAP__REG, &data, C_One_U8X);
            break;
         case C_Four_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_SNG_TAP__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT2_PAD_SNG_TAP, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_SNG_TAP__REG, &data, C_One_U8X);
            break;
         case C_Five_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_ORIENT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT2_PAD_ORIENT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_ORIENT__REG, &data, C_One_U8X);
            break;
         case C_Six_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_FLAT__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_INT2_PAD_FLAT, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_INT2_PAD_FLAT__REG, &data, C_One_U8X);
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100488
   bma222_get_int2_pad_sel */
/* Compiler Switch if applicable
#ifdef

#endif
*/
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the status of int2 pad sel
 *
 *
 *
 *
 *  \param
 *               unsigned char int2sel
 *                    int2sel:
 *                             0 -- > int2_low
 *                             1 -- > int2_High
 *                             2 -- > int2_slope
 *                             3 -- > int2_d_tap
 *                             4 -- > int2_s_tap
 *                             5 -- > int2_orient
 *                             6 -- > int2_flat
 *
 *             unsigned char *status
 *                             0 -- > Disabled
 *                             1 -- > Enabled
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_int2_pad_sel(unsigned char param,unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT2_PAD_LOWG);
            *status = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT2_PAD_HIGHG);
            *status = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT2_PAD_SLOPE);
            *status = data;
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT2_PAD_DB_TAP);
            *status = data;
            break;
         case C_Four_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT2_PAD_SNG_TAP);
            *status = data;
            break;
         case C_Five_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT2_PAD_ORIENT);
            *status = data;
            break;
         case C_Six_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_SEL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_INT2_PAD_FLAT);
            *status = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100207
   bma222_set_int_src */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API enables the int src register
 *
 *
 *
 *
 *  \param
 *                    unsigned char intsrc
 *                     State:
 *                             0 -- > int_src_low
 *                             1 -- > int_src_High
 *                             2 -- > int_src_slope
 *                             3 -- > int_src_tap
 *                             4 -- > int_src_data
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_int_src(unsigned char intsrc)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char state;
   state = C_One_U8X;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (intsrc)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_LOWG__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_UNFILT_INT_SRC_LOWG, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_LOWG__REG, &data, C_One_U8X);
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_HIGHG__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_UNFILT_INT_SRC_HIGHG, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_HIGHG__REG, &data, C_One_U8X);
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_SLOPE__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_UNFILT_INT_SRC_SLOPE, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_SLOPE__REG, &data, C_One_U8X);
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_TAP__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_UNFILT_INT_SRC_TAP, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_TAP__REG, &data, C_One_U8X);
            break;
         case C_Four_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_DATA__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_UNFILT_INT_SRC_DATA, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_UNFILT_INT_SRC_DATA__REG, &data, C_One_U8X);
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100492
   bma222_get_int_src */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the status of int src register
 *
 *
 *
 *
 *  \param
 *                    unsigned char intsrc
 *                     State:
 *                             0 -- > int_src_low
 *                             1 -- > int_src_High
 *                             2 -- > int_src_slope
 *                             3 -- > int_src_tap
 *                             4 -- > int_src_data
 *
 *                   unsigned char *status
 *                               0 -- > Disbaled
 *                               1 -- > Enabled
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_int_src(unsigned char param,unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_SRC_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_UNFILT_INT_SRC_LOWG);
            *status = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_SRC_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_UNFILT_INT_SRC_HIGHG);
            *status = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_SRC_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_UNFILT_INT_SRC_SLOPE);
            *status = data;
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_SRC_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_UNFILT_INT_SRC_TAP);
            *status = data;
            break;
         case C_Four_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_SRC_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_UNFILT_INT_SRC_DATA);
            *status = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100211
   bma222_set_int_set */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the int set
 *
 *
 *
 *
 *  \param
 *                     intset:
 *                             0 -- > int1_lvl
 *                             1 -- > int1_od
 *                             2 -- > int2_lvl
 *                             3 -- > int2_od
 *
 *
 *
 *
 *                   state:
 *                            Reset --> 0 -- > Writes zero
 *                             set -- > 1 -- > Writes one
 *                                                      Reset value
 *             int1_lvl INT1 active level selection           1
 *             int2_lvl INT2 active level selection           1
 *             int1_od  INT1 output type selection            0
 *             int2_od  INT2 output type selection            0
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_int_set(unsigned char intset,unsigned char state)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (intset)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_ACTIVE_LEVEL__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_INT1_PAD_ACTIVE_LEVEL, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_ACTIVE_LEVEL__REG, &data, C_One_U8X);
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_OUTPUT_TYPE__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_INT1_PAD_OUTPUT_TYPE, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_INT1_PAD_OUTPUT_TYPE__REG, &data, C_One_U8X);
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_ACTIVE_LEVEL__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_INT2_PAD_ACTIVE_LEVEL, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_ACTIVE_LEVEL__REG, &data, C_One_U8X);
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_OUTPUT_TYPE__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_INT2_PAD_OUTPUT_TYPE, state );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_INT2_PAD_OUTPUT_TYPE__REG, &data, C_One_U8X);
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100496
   bma222_get_int_set */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the int set status of the register
 *
 *
 *
 *
 *  \param
 *                     intset:
 *                             0 -- > int1_lvl
 *                             1 -- > int1_od
 *                             2 -- > int2_lvl
 *                             3 -- > int2_od
 *
 *
 *
 *
 *                   state:
 *                            Reset --> 0 -- > Writes zero
 *                             set -- > 1 -- > Writes one
 *                                                      Reset value
 *             int1_lvl INT1 active level selection           1
 *             int2_lvl INT2 active level selection           1
 *             int1_od  INT1 output type selection            0
 *             int2_od  INT2 output type selection            0
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_int_set(unsigned char param,unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_SET_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_INT1_PAD_ACTIVE_LEVEL);
            *status = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_SET_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_INT1_PAD_OUTPUT_TYPE);
            *status = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_SET_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_INT2_PAD_ACTIVE_LEVEL);
            *status = data;
            break;
         case C_Three_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_SET_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_INT2_PAD_OUTPUT_TYPE);
            *status = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100117
   bma222_get_mode_ctrl */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the status of mode control register
 *
 *
 *
 *
 *  \param           unsigned char *mode
 *
 *
 *
 *  \return     result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_mode_ctrl(unsigned char *mode)
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_MODE_CTRL_REG, &data, C_One_U8X );
      *mode = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100219
   bma222_set_low_g_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the Low g duration in ms
 *
 *
 *
 *
 *  \param
 *                     duration:
 *                             0000 0000 -- > 0 ms
 *                             0000 0001 -- > 2 ms ( one LSB = 2 ms)
 *                             0000 0010 -- > 4 ms
 *                             0000 0011 -- > 6 ms
 *                             0000 0101 -- > 8 ms
 *                             0000 0110 -- > 10 ms
 *                             0000 0111 -- > 12 ms
 *
 *
 *                             1111 1111 ---> 512 ms
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_low_g_duration(unsigned char duration)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_LOWG_DUR__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_LOWG_DUR, duration );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_LOWG_DUR__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100500
   bma222_get_low_g_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the low g duration register value
 *
 *
 *
 *
 *  \param
 *              unsigned char *status
 *                     status:
 *                             0000 0000 -- > 0 ms
 *                             0000 0001 -- > 2 ms ( one LSB = 2 ms)
 *                             0000 0010 -- > 4 ms
 *                             0000 0011 -- > 6 ms
 *                             0000 0101 -- > 8 ms
 *                             0000 0110 -- > 10 ms
 *                             0000 0111 -- > 12 ms
 *
 *
 *                             1111 1111 ---> 512 ms
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_low_g_duration(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_LOW_DURN_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_LOWG_DUR);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100223
   bma222_set_low_g_threshold */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the low g threshold
 *
 *
 *
 *
 *  \param
 *           unsigned char threshold
 *                     threshold: "00110000" --> Reset Value
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_low_g_threshold(unsigned char threshold)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_LOWG_THRES__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_LOWG_THRES, threshold );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_LOWG_THRES__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100504
   bma222_get_low_g_threshold */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the low g threshold value
 *
 *
 *
 *
 *  \param unsigned char *status
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_low_g_threshold(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_LOW_THRES_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_LOWG_THRES);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100227
   bma222_set_high_g_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the high g duration
 *
 *
 *
 *
 *  \param
 *                     duration:
 *                             0000 0000 -- > 0 ms
 *                             0000 0001 -- > 2 ms ( one LSB = 2 ms)
 *                             0000 0010 -- > 4 ms
 *                             0000 0011 -- > 6 ms
 *                             0000 0101 -- > 8 ms
 *                             0000 0110 -- > 10 ms
 *                             0000 0111 -- > 12 ms
 *
 *
 *                             1111 1111 ---> 512 ms
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_high_g_duration(unsigned char duration)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_HIGHG_DUR__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_HIGHG_DUR, duration );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_HIGHG_DUR__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100508
   bma222_get_high_g_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the high g duration register value
 *
 *
 *
 *
 *  \param
 *              unsigned char *status
 *                     status:
 *                             0000 0000 -- > 0 ms
 *                             0000 0001 -- > 2 ms ( one LSB = 2 ms)
 *                             0000 0010 -- > 4 ms
 *                             0000 0011 -- > 6 ms
 *                             0000 0101 -- > 8 ms
 *                             0000 0110 -- > 10 ms
 *                             0000 0111 -- > 12 ms
 *
 *
 *                             1111 1111 ---> 512 ms
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_high_g_duration(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_HIGH_DURN_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_HIGHG_DUR);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100228
   bma222_set_high_g_threshold */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the high g threshold
 *
 *
 *
 *
 *  \param
  *           unsigned char threshold
 *                     threshold: "11000000" --> Reset Value
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_high_g_threshold(unsigned char threshold)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_HIGHG_THRES__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_HIGHG_THRES, threshold );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_HIGHG_THRES__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100512
   bma222_get_high_g_threshold */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the high g threshold value
 *
 *
 *
 *
 *  \param unsigned char *status
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_high_g_threshold(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_HIGH_THRES_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_HIGHG_THRES);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100232
   bma222_set_slope_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the slope duration
 *
 *
 *
 *
 *  \param
 *                    unsigned char duration
 *
 *                     duration: "00010100" --> reset value
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_slope_duration(unsigned char duration)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SLOPE_DUR__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_SLOPE_DUR, duration );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_SLOPE_DUR__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100516
   bma222_get_slope_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the slope duration
 *
 *
 *
 *
 *  \param      unsigned char *status
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_slope_duration(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SLOPE_DURN_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_SLOPE_DUR);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100233
   bma222_set_slope_threshold */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the slope threshold
 *
 *
 *
 *
 *  \param
 *                      unsigned char threshold
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_slope_threshold(unsigned char threshold)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SLOPE_THRES__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_SLOPE_THRES, threshold );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_SLOPE_THRES__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100520
   bma222_get_slope_threshold */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the slope threshold
 *
 *
 *
 *
 *  \param              unsigned char *status
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_slope_threshold(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SLOPE_THRES_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_SLOPE_THRES);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100245
   bma222_set_tap_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the tap duration
 *
 *
 *
 *
 *  \param
 *          unsigned chat duration
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_tap_duration(unsigned char duration)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_DUR__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_TAP_DUR, duration );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_TAP_DUR__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100524
   bma222_get_tap_duration */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the Tap durations
 *
 *
 *
 *
 *  \param  unsigned chat *status
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_tap_duration(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_PARAM_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_TAP_DUR);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100251
   bma222_set_tap_shock */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the tap shock duration
 *
 *
 *
 *
 *  \param
 *                  unsigned char setval
 *                             0 -- > 50 ms
 *                             1 -- > 75 ms
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_tap_shock(unsigned char setval)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_SHOCK_DURN__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_TAP_SHOCK_DURN, setval );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_TAP_SHOCK_DURN__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100528
   bma222_get_tap_shock */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the tap shock duration
 *
 *
 *
 *
 *  \param       unsigned char *status
 *                      0 -- > 50 ms
 *                      1 -- > 75 ms
 *
 *
 *  \return     result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_tap_shock(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_PARAM_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_TAP_SHOCK_DURN);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100253
   bma222_set_tap_quiet */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the tap quiet duration
 *
 *
 *
 *
 *  \param
 *             unsigned char duration:
 *                             0 -- > 30 ms
 *                             1 -- > 20 ms
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_tap_quiet_duration(unsigned char duration)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_QUIET_DURN__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_TAP_QUIET_DURN, duration );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_TAP_QUIET_DURN__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100532
   bma222_get_tap_quiet */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the tap quiet duration
 *
 *
 *
 *
 *  \param
 *             unsigned char *status
 *                             0 -- > 30 ms
 *                             1 -- > 20 ms
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_tap_quiet(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_PARAM_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_TAP_QUIET_DURN);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100257
   bma222_set_tap_threshold */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the tap threshold
 *
 *
 *
 *
 *  \param
 *             unsigned char threshold:
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_tap_threshold(unsigned char threshold)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_THRES__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_TAP_THRES, threshold );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_TAP_THRES__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100536
   bma222_get_tap_threshold */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the tap threshold
 *
 *
 *
 *
 *  \param
 *              unsigned char threshold
 *                             "00000"
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_tap_threshold(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_THRES_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_TAP_THRES);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100261
   bma222_set_tap_samp */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the tap sample period
 *
 *
 *
 *
 *  \param unsigned char samp
 *                     samp:
 *         tap_samp             Samples
 *              00                2
 *              01                4
 *              10                8
 *              11                16
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_tap_samp(unsigned char samp)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_SAMPLES__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_TAP_SAMPLES, samp );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_TAP_SAMPLES__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100540
   bma222_get_tap_samp */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the tap sample period
 *
 *
 *
 *
 *  \param unsigned char samp
 *                     samp:
 *         tap_samp             Samples
 *              00                2
 *              01                4
 *              10                8
 *              11                16
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_tap_samp(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_TAP_THRES_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_TAP_SAMPLES);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100265
   bma222_set_orient_mode */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the orient mode
 *
 *
 *
 *
 *  \param
 *              unsigned char mode
 *               Orient mode  Orientation mode
 *                     00       Symmetrical
 *                     01       High asymmetrical
 *                     10       Low asymmetrical
 *                     11       Symmetrical
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_orient_mode(unsigned char mode)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_ORIENT_MODE__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_ORIENT_MODE, mode );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_ORIENT_MODE__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100544
   bma222_get_orient_mode */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the orient mode
 *
 *
 *
 *
 *  \param
 *              unsigned char mode
 *               Orient mode  Orientation mode
 *                     00       Symmetrical
 *                     01       High asymmetrical
 *                     10       Low asymmetrical
 *                     11       Symmetrical
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_orient_mode(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_ORIENT_PARAM_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_ORIENT_MODE);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100267
   bma222_set_orient_blocking */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the orient blocking
 *
 *
 *
 *
 *  \param
 *             unsigned char orientblk
 *
 *
 *
 *      orient_blocking                       Conditions
 *              00                              No blocking
 *              01                              Theta blocking
 *              10                              Theta blocking
 *                                                      OR
 *                                      slope of an acceleration in any axis is > 0.2g
 *
 *              11                     The orient value is not stable for at least 100 ms
 *                                                      OR
 *                                              Theta blocking
 *                                                      OR
 *                                        slope of an acceleration in any axis is > 0.4g
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_orient_blocking(unsigned char orientblk)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_ORIENT_BLOCK__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_ORIENT_BLOCK, orientblk );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_ORIENT_BLOCK__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100548
   bma222_get_orient_blocking */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the orient blocking
 *
 *
 *
 *
 *  \param
 *             unsigned char orientblk
 *
 *
 *
 *      orient_blocking                       Conditions
 *              00                              No blocking
 *              01                              Theta blocking
 *              10                              Theta blocking
 *                                                      OR
 *                                      slope of an acceleration in any axis is > 0.2g
 *
 *              11                     The orient value is not stable for at least 100 ms
 *                                                      OR
 *                                              Theta blocking
 *                                                      OR
 *                                        slope of an acceleration in any axis is > 0.4g
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_orient_blocking(unsigned char *orientblk )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_ORIENT_PARAM_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_ORIENT_BLOCK);
      *orientblk = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100269
   bma222_set_orient_hyst */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the orient hysteresis
 *
 *
 *
 *
 *  \param           unsigned char orienthyst
 *                     orienthyst:
 *                            Orientation hysteresis
 *                                     Reset value -- > "001"
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_orient_hyst(unsigned char orienthyst)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_ORIENT_HYST__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_ORIENT_HYST, orienthyst );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_ORIENT_HYST__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100552
   bma222_get_orient_hyst */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the orient hysteresis
 *
 *
 *
 *
 *  \param           unsigned char *status
 *                     orienthyst:
 *                            Orientation hysteresis
 *                                     Reset value -- > "001"
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_orient_hyst(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_ORIENT_PARAM_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_ORIENT_HYST);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100273
   bma222_set_theta_blocking */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the theta blocking
 *
 *
 *
 *
 *  \param
 *               unsigned char thetablk
 *                      theatblk:
 *                             "001000" -- > Reset Value
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_theta_blocking(unsigned char thetablk)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_THETA_BLOCK__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_THETA_BLOCK, thetablk );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_THETA_BLOCK__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100556
   bma222_get_theta_blocking */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the theta blocking
 *
 *
 *
 *
 *  \param
 *               unsigned char *status
 *                      theatblk:
 *                             "001000" -- > Reset Value
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_theta_blocking(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_THETA_BLOCK_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_THETA_BLOCK);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100281
   bma222_set_orient_ex */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the orient ex
 *
 *
 *
 *
 *  \param      unsigned char orientex
 *                     orientex:
 *                             If the register orient_ex is set to ,1.,
 *      the axes X and Z are swapped and axis Y is inverted in all orientation calculations.
 *
 *                             Reset value will be       "0"
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_orient_ex(unsigned char orientex)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_ORIENT_AXIS__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_ORIENT_AXIS, orientex );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_ORIENT_AXIS__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100560
   bma222_get_orient_ex */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the orient ex
 *
 *
 *
 *
 *  \param      unsigned char *status
 *                     orientex:
 *                             If the register orient_ex is set to ,1.,
 *      the axes X and Z are swapped and axis Y is inverted in all orientation calculations.
 *
 *                             Reset value will be       "0"
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_orient_ex(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_THETA_BLOCK_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_ORIENT_AXIS);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100277
   bma222_set_theta_flat */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the theta flat
 *
 *
 *
 *
 *  \param
 *                      unsigned char thetaflat
 *                     "001000" Reset value
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_theta_flat(unsigned char thetaflat)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_THETA_FLAT__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_THETA_FLAT, thetaflat );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_THETA_FLAT__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100564
   bma222_get_theta_flat */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the theta flat
 *
 *
 *
 *
 *  \param
 *                      unsigned char *status
 *                     "001000" Reset value
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_theta_flat(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_THETA_FLAT_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_THETA_FLAT);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100418
   bma222_set_flat_hold_time */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets flat hold time
 *
 *
 *
 *
 *  \param
 *             unsigned char holdtime
 *                      flat holdtime   time(s)
 *                        00             0
 *                        01             0.512
 *                        10             1.024
 *                        11             2.048
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_flat_hold_time(unsigned char holdtime)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_FLAT_HOLD_TIME__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_FLAT_HOLD_TIME, holdtime );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_FLAT_HOLD_TIME__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100422
   bma222_get_flat_hold_time */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets flat hold time
 *
 *
 *
 *
 *  \param
 *             unsigned char *holdtime
 *                      flat holdtime   time(s)
 *                        00             0
 *                        01             0.512
 *                        10             1.024
 *                        11             2.048
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_flat_hold_time(unsigned char *holdtime )
   {
   int comres= C_Zero_U8X;
   unsigned char data1;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_FLAT_HOLD_TIME_REG, &data1, C_One_U8X );
      data1  = bma222_GET_BITSLICE(data1, bma222_FLAT_HOLD_TIME);
      *holdtime = data1 ;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100285
   bma222_get_low_power_state */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the lowpower state
 *
 *
 *
 *
 *  \param unsigned char *Lowpower
 *
 *              state :
 *                      0 -- >  sleep period
 *                      1 -- >  otherwise
 *                              After reset it will be set to "0"
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_low_power_state(unsigned char *Lowpower )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_STATUS_LOW_POWER_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_LOW_POWER_MODE_S);
      *Lowpower = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100289
   bma222_set_selftest_st */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the selftest st
 *
 *
 *
 *
 *  \param
 *              unsigned char selftest
 *
 *                             00       No self-test active
 *                             01       Self-test active for X axis
 *                             10       Self-test active for Y axis
 *                             11       Self-test active for Z axis
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_selftest_st(unsigned char selftest)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SELF_TEST__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_EN_SELF_TEST, selftest );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SELF_TEST__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100568
   bma222_get_selftest_st */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the selftest st
 *
 *
 *
 *
 *  \param
 *              unsigned char selftest
 *
 *                             00       No self-test active
 *                             01       Self-test active for X axis
 *                             10       Self-test active for Y axis
 *                             11       Self-test active for Z axis
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_selftest_st(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SELF_TEST_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_EN_SELF_TEST);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100293
   bma222_set_selftest_stn */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief
 *
 *
 *
 *
 *  \param
 *             unsigned char stn
 *
 *                         If stn is at ,0., a positive self-test deviation is performed
 *                         If stn is at ,1., a negative self-test deviation is performed
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_selftest_stn(unsigned char stn)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_NEG_SELF_TEST__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_NEG_SELF_TEST, stn );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_NEG_SELF_TEST__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100572
   bma222_get_selftest_stn */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief
 *
 *
 *
 *
 *  \param
 *
 *                         If stn is at ,0., a positive self-test deviation is performed
 *                         If stn is at ,1., a negative self-test deviation is performed
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_selftest_stn(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SELF_TEST_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_NEG_SELF_TEST);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100297
   bma222_set_selftest_st_amp */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the selftest st amp
 *
 *
 *
 *
 *  \param
 *              unsigned char stamp
 *
 *           Self-test amplitude is control through st_amp state;
 *
 *              st_amp<2:0>                     vst_amp amplitude
 *                                                  (ratio of vreg)
 *              000                                      3/10
 *              001                                      4/10
 *              010                                      5/10
 *              011                                      6/10
 *              100                                      7/10
 *              101                                      8/10
 *              110                                      9/10
 *              111                                      1
 *
 *
 *
 *
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_selftest_st_amp(unsigned char stamp)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SELF_TEST_AMP__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_SELF_TEST_AMP, stamp );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_SELF_TEST_AMP__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100576
   bma222_get_selftest_st_amp */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the selftest st amp
 *
 *
 *
 *
 *  \param
 *              unsigned char *status
 *
 *           Self-test amplitude is control through st_amp state;
 *
 *              st_amp<2:0>                     vst_amp amplitude
 *                                                  (ratio of vreg)
 *              000                                      3/10
 *              001                                      4/10
 *              010                                      5/10
 *              011                                      6/10
 *              100                                      7/10
 *              101                                      8/10
 *              110                                      9/10
 *              111                                      1
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_selftest_st_amp(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SELF_TEST_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_SELF_TEST_AMP);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100038
   bma222_set_ee_w */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the ee_w bit
 *
 *
 *
 *
 *  \param
 *              unsigned char selftest
 *
 *                      eew 0 = lock EEPROM
 *                      eew 1 = unlock Setting part of EEPROM
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_ee_w(unsigned char eew)
   {
   unsigned char data;
         int comres;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR ;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_UNLOCK_EE_WRITE_SETTING__REG,&data, C_One_U8X);
      data = bma222_SET_BITSLICE(data, bma222_UNLOCK_EE_WRITE_SETTING, eew);
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_UNLOCK_EE_WRITE_SETTING__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100039
   bma222_get_ee_w */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the status of the ee_w bit
 *
 *
 *
 *
 *  \param
 *              unsigned char selftest
 *
 *                      eew 0 = lock EEPROM
 *                      eew 1 = unlock Setting part of EEPROM
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_ee_w(unsigned char *eew)
   {
   int comres;
   if (p_bma222==C_Zero_U8X)
      {
      comres =E_SMB_NULL_PTR ;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_UNLOCK_EE_WRITE_SETTING__REG,eew, C_One_U8X);
      *eew = bma222_GET_BITSLICE(*eew, bma222_UNLOCK_EE_WRITE_SETTING);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100305
   bma222_set_ee_prog_trig */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the ee_prog_trig bit
 *
 *
 *
 *
 *  \param none
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_ee_prog_trig(void)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char eeprog;
   eeprog = 0x01;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_START_EE_WRITE_SETTING__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_START_EE_WRITE_SETTING, eeprog );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_START_EE_WRITE_SETTING__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100309
   bma222_get_eeprom_writing_status */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the EEPROM writing status
 *
 *
 *
 *
 *  \param unsigned char *eewrite
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_eeprom_writing_status(unsigned char *eewrite )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EEPROM_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_EE_WRITE_SETTING_S);
      *eewrite = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100313
   bma222_set_update_image */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the Update image in EEPROM control
 *
 *
 *
 *
 *  \param
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_update_image(void)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char update;
   update = C_One_U8X;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_UPDATE_IMAGE__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_UPDATE_IMAGE, update );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_UPDATE_IMAGE__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100321
   bma222_set_3wire_spi */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the 3wite SPI
 *
 *
 *
 *
 *  \param      None
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \   return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_3wire_spi(void)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char update;
   update = 0x01;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SPI_MODE_3__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_EN_SPI_MODE_3, update );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SPI_MODE_3__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100580
   bma222_get_3wire_spi */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the 3wire SPI
 *
 *
 *
 *
 *  \param  unsigned char *status
 *
 *
 *
 *  \return  result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_3wire_spi(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SERIAL_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_EN_SPI_MODE_3);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100325
   bma222_set_i2c_wdt_timer */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the i2c Watch dog timer
 *
 *
 *
 *
 *  \param
 *                     duration:
 *                             0 -- > 1 ms
 *                             1 -- > 50 ms
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_i2c_wdt_timer(unsigned char timedly)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_I2C_WATCHDOG_PERIOD__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_I2C_WATCHDOG_PERIOD, timedly );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_I2C_WATCHDOG_PERIOD__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100584
   bma222_get_i2c_wdt_timer */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the i2c Watch dog timer
 *
 *
 *
 *
 *  \param              unsigned char *status
 *
 *                             0 -- > 1 ms
 *                             1 -- > 50 ms
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_i2c_wdt_timer(unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_SERIAL_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_I2C_WATCHDOG_PERIOD);
      *status = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100339
   bma222_set_hp_en */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the hp_en bit
 *
 *
 *
 *
 *  \param
 *                   unsigned char param
 *                             0 -- >  hp_x_en
 *                             1 -- >  hp_y_en
 *                             2 -- >  hp_z_en
 *                   unsigned char hpval
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_hp_en(unsigned char param,unsigned char hpval)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SLOW_COMP_X__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_SLOW_COMP_X, hpval );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SLOW_COMP_X__REG, &data, C_One_U8X);
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SLOW_COMP_Y__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_SLOW_COMP_Y, hpval );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SLOW_COMP_Y__REG, &data, C_One_U8X);
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_SLOW_COMP_Z__REG, &data, C_One_U8X );
            data = bma222_SET_BITSLICE(data, bma222_EN_SLOW_COMP_Z, hpval );
            comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_SLOW_COMP_Z__REG, &data, C_One_U8X);
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100597
   bma222_get_hp_en */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the hp_en bit
 *
 *
 *
 *
 *  \param
 *                   unsigned char param
 *                             0 -- >  hp_x_en
 *                             1 -- >  hp_y_en
 *                             2 -- >  hp_z_en
 *                   unsigned char *status
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_hp_en(unsigned char param,unsigned char *status )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (param)
         {
         case C_Zero_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_CTRL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_SLOW_COMP_X);
            *status = data;
            break;
         case C_One_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_CTRL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_SLOW_COMP_Y);
            *status = data;
            break;
         case C_Two_U8X:
            comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_CTRL_REG, &data, C_One_U8X );
            data = bma222_GET_BITSLICE(data, bma222_EN_SLOW_COMP_Z);
            *status = data;
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100343
   bma222_get_cal_ready */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API get the status of the cal_rdy bit
 *
 *
 *
 *
 *  \param              unsigned char *calrdy
 *                           calredy reset value "0"
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_cal_ready(unsigned char *calrdy )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_FAST_COMP_RDY_S);
      *calrdy = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100347
   bma222_set_cal_trigger */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief  This API sets the cal trigger
 *
 *
 *
 *
 *  \param
 *                     duration:
 *                             00 -- > None
 *                             01 -- > X
 *                             10 -- > Y
 *                             11 -- > Z
 *
 *
 *
 *
 *
 *
 *  \return     Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_cal_trigger(unsigned char caltrigger)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EN_FAST_COMP__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_EN_FAST_COMP, caltrigger );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_EN_FAST_COMP__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100351
   bma222_set_offset_reset */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset reset bit
 *
 *
 *
 *
 *  \param              None
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return     Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_reset(void)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   unsigned char offsetreset;
   offsetreset = C_One_U8X;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_RESET_OFFSET_REGS__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_RESET_OFFSET_REGS, offsetreset );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_RESET_OFFSET_REGS__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100355
   bma222_set_offset_cutoff */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset cutoff
 *
 *
 *
 *
 *  \param      unsigned char offsetcutoff
 *
 *
 *                           cut_off              OffsetPer
 *                             0        -- >           8
 *                             1        -- >           16
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_cutoff(unsigned char offsetcutoff)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_COMP_CUTOFF__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_COMP_CUTOFF, offsetcutoff );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_COMP_CUTOFF__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100359
   bma222_get_offset_cutoff */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the offset cutoff
 *
 *
 *
 *
 *  \param      unsigned char offsetcutoff
 *
 *
 *                           cut_off              OffsetPer
 *                             0        -- >           8
 *                             1        -- >           16
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_cutoff(unsigned char *cutoff )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_PARAMS_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_COMP_CUTOFF);
      *cutoff = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100363
   bma222_set_offset_target_x */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset target_x
 *
 *
 *
 *
 *  \param unsigned char offsettarget
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_target_x(unsigned char offsettarget)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_COMP_TARGET_OFFSET_X__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_COMP_TARGET_OFFSET_X, offsettarget );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_COMP_TARGET_OFFSET_X__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100369
   bma222_get_offset_target_x */
/* Compiler Switch if applicable
#ifdef

#endif
*/
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the offset_target_x
 *
 *
 *
 *
 *  \param unsigned char *offsettarget
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_target_x(unsigned char *offsettarget )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_PARAMS_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_COMP_TARGET_OFFSET_X);
      *offsettarget = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100368
   bma222_set_offset_target_y */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_target_y
 *
 *
 *
 *
 *  \param unsigned char offsettarget
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_target_y(unsigned char offsettarget)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_COMP_TARGET_OFFSET_Y__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_COMP_TARGET_OFFSET_Y, offsettarget );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_COMP_TARGET_OFFSET_Y__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100364
   bma222_get_offset_target_y */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the offset_target_y
 *
 *
 *
 *
 *  \param unsigned char *offsettarget
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_target_y(unsigned char *offsettarget )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_PARAMS_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_COMP_TARGET_OFFSET_Y);
      *offsettarget = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100371
   bma222_set_offset_target_z */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_target_z
 *
 *
 *
 *
 *  \param unsigned char offsettarget
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_target_z(unsigned char offsettarget)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_COMP_TARGET_OFFSET_Z__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_COMP_TARGET_OFFSET_Z, offsettarget );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_COMP_TARGET_OFFSET_Z__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100372
   bma222_get_offset_target_z */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the offset_target_z
 *
 *
 *
 *
 *  \param unsigned char *offsettarget
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_target_z(unsigned char *offsettarget )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_PARAMS_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_COMP_TARGET_OFFSET_Z);
      *offsettarget = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100376
   bma222_set_offset_filt_x */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_filt_x
 *
 *
 *
 *
 *  \param
 *                    unsigned char offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_filt_x(unsigned char offsetfilt)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      data =  offsetfilt;
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_OFFSET_FILT_X_REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100377
   bma222_get_offset_filt_x */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the offset_filt_x
 *
 *
 *
 *
 *  \param
 *                    unsigned char *offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_filt_x(unsigned char *offsetfilt )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_FILT_X_REG, &data, C_One_U8X );
      *offsetfilt = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100381
   bma222_set_offset_filt_y */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_filt_y
 *
 *
 *
 *
 *  \param
 *                    unsigned char offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_filt_y(unsigned char offsetfilt)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      data =  offsetfilt;
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_OFFSET_FILT_Y_REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100382
   bma222_get_offset_filt_y */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the offset_filt_x
 *
 *
 *
 *
 *  \param
 *                    unsigned char *offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_filt_y(unsigned char *offsetfilt )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_FILT_Y_REG, &data, C_One_U8X );
      *offsetfilt = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100384
   bma222_set_offset_filt_z */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_filt_z
 *
 *
 *
 *
 *  \param
 *                    unsigned char offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_filt_z(unsigned char offsetfilt)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      data =  offsetfilt;
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_OFFSET_FILT_Z_REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100385
   bma222_get_offset_filt_z */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_filt_z
 *
 *
 *
 *
 *  \param
 *                    unsigned char *offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_filt_z(unsigned char *offsetfilt )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_FILT_Z_REG, &data, C_One_U8X );
      *offsetfilt = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100389
   bma222_set_offset_unfilt_x */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_unfilt_x
 *
 *
 *
 *
 *  \param              unsigned char offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_unfilt_x(unsigned char offsetfilt)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      data =  offsetfilt;
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_OFFSET_UNFILT_X_REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100390
   bma222_get_offset_unfilt_x */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the offset_unfilt_x
 *
 *
 *
 *
 *  \param              unsigned char *offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_unfilt_x(unsigned char *offsetfilt )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_UNFILT_X_REG, &data, C_One_U8X );
      *offsetfilt = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100391
   bma222_set_offset_unfilt_y */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_unfilt_y
 *
 *
 *
 *
 *  \param              unsigned char offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_unfilt_y(unsigned char offsetfilt)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      data =  offsetfilt;
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_OFFSET_UNFILT_Y_REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100392
   bma222_get_offset_unfilt_y */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the offset_unfilt_y
 *
 *
 *
 *
 *  \param              unsigned char offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_unfilt_y(unsigned char *offsetfilt )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_UNFILT_Y_REG, &data, C_One_U8X );
      *offsetfilt = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100393
   bma222_set_offset_unfilt_z */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_unfilt_z
 *
 *
 *
 *
 *  \param              unsigned char offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_offset_unfilt_z(unsigned char offsetfilt)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      data =  offsetfilt;
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_OFFSET_UNFILT_Z_REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100394
   bma222_get_offset_unfilt_z */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the offset_unfilt_z
 *
 *
 *
 *
 *  \param              unsigned char offsetfilt
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_offset_unfilt_z(unsigned char *offsetfilt )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_OFFSET_UNFILT_Z_REG, &data, C_One_U8X );
      *offsetfilt = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100034
   bma222_set_Int_Mode */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the set_Int_Mode
 *
 *
 *
 *
 *  \param              unsigned char Mode
 *
 *               0000            non-latched
 *               0001            temporary, 250 ms
 *               0010            temporary, 500 ms
 *               0011            temporary, 1 s
 *               0100            temporary, 2 s
 *               0101            temporary, 4 s
 *               0110            temporary, 8 s
 *               0111            latched
 *               1000            non-latched
 *               1001            temporary, 500 行
 *               1010            temporary, 500 行
 *               1011            temporary, 1 ms
 *               1100            temporary, 12.5 ms
 *               1101            temporary, 25 ms
 *               1110            temporary, 50 ms
 *               1111            latched
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_Int_Mode(unsigned char Mode )
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_MODE_SEL__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_INT_MODE_SEL, Mode );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_INT_MODE_SEL__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100035
   bma222_get_Int_Mode */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets the set_Int_Mode
 *
 *
 *
 *
 *  \param              unsigned char *Mode
 *
 *               0000            non-latched
 *               0001            temporary, 250 ms
 *               0010            temporary, 500 ms
 *               0011            temporary, 1 s
 *               0100            temporary, 2 s
 *               0101            temporary, 4 s
 *               0110            temporary, 8 s
 *               0111            latched
 *               1000            non-latched
 *               1001            temporary, 500 行
 *               1010            temporary, 500 行
 *               1011            temporary, 1 ms
 *               1100            temporary, 12.5 ms
 *               1101            temporary, 25 ms
 *               1110            temporary, 50 ms
 *               1111            latched
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_Int_Mode(unsigned char * Mode )
   {
   int comres= C_Zero_U8X;
   unsigned char data1;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_MODE_SEL__REG, &data1, C_One_U8X );
      data1  = bma222_GET_BITSLICE(data1, bma222_INT_MODE_SEL);
      *Mode = data1 ;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100036
   bma222_set_Int_Enable */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the offset_filt_z
 *
 *
 *
 *
 *  \param
 *                   unsigned char InterruptType
 *
 *                      0      Low G Interrupt Enable
 *                      1      High G X Interrupt Enable
 *                      2      High G Y Interrupt Enable
 *                      3      High G Z Interrupt Enable
 *                      4      New Data Interrupt Enable
 *                      5      Slope X Interrupt Enable
 *                      6      Slope Y Interrupt Enable
 *                      7      Slope Z Interrupt Enable
 *                      8      Single Tap Interrupt Enable
 *                      9      Double tap Interrupt Enable
 *                      10     Orient Interrupt Enable
 *                      11     Flat Interrupt Enableunsigned char offsetfilt
 *
 *                    unsigned char value
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_Int_Enable(unsigned char InterruptType , unsigned char value )
   {
   int comres=C_Zero_U8X;
   unsigned char data1,data2;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE1_REG, &data1, C_One_U8X );
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE2_REG, &data2, C_One_U8X );
      
      value = value & C_One_U8X;
      switch (InterruptType)
         {
         case C_Zero_U8X:
            /* Low G Interrupt  */
            data2 = bma222_SET_BITSLICE(data2, bma222_EN_LOWG_INT, value );
            break;
         case C_One_U8X:
            /* High G X Interrupt */
            /* EasyCASE - */
            data2 = bma222_SET_BITSLICE(data2, bma222_EN_HIGHG_X_INT, value );
            break;
         case C_Two_U8X:
            /* High G Y Interrupt */
            /* EasyCASE - */
            data2 = bma222_SET_BITSLICE(data2, bma222_EN_HIGHG_Y_INT, value );
            break;
         case C_Three_U8X:
            /* High G Z Interrupt */
            /* EasyCASE - */
            data2 = bma222_SET_BITSLICE(data2, bma222_EN_HIGHG_Z_INT, value );
            break;
         case C_Four_U8X:
            /* New Data Interrupt  */
            /* EasyCASE - */
            data2 = bma222_SET_BITSLICE(data2, bma222_EN_NEW_DATA_INT, value );
            break;
         case C_Five_U8X:
            /* Slope X Interrupt */
            /* EasyCASE - */
            data1 = bma222_SET_BITSLICE(data1, bma222_EN_SLOPE_X_INT, value );
            break;
         case C_Six_U8X:
            /* Slope Y Interrupt */
            /* EasyCASE - */
            data1 = bma222_SET_BITSLICE(data1, bma222_EN_SLOPE_Y_INT, value );
            break;
         case C_Seven_U8X:
            /* Slope Z Interrupt */
            /* EasyCASE - */
            data1 = bma222_SET_BITSLICE(data1, bma222_EN_SLOPE_Z_INT, value );
            break;
         case C_Eight_U8X:
            /* Single Tap Interrupt */
            /* EasyCASE - */
            data1 = bma222_SET_BITSLICE(data1, bma222_EN_SINGLE_TAP_INT, value );
            break;
         case C_Nine_U8X:
            /* Double Tap Interrupt */
            /* EasyCASE - */
            data1 = bma222_SET_BITSLICE(data1, bma222_EN_DOUBLE_TAP_INT, value );
            break;
         case C_Ten_U8X:
            /* Orient Interrupt  */
            /* EasyCASE - */
            data1 = bma222_SET_BITSLICE(data1, bma222_EN_ORIENT_INT, value );
            break;
         case C_Eleven_U8X:
            /* Flat Interrupt */
            /* EasyCASE - */
            data1 = bma222_SET_BITSLICE(data1, bma222_EN_FLAT_INT, value );
            break;
         default:
            break;
         }
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE1_REG, &data1, C_One_U8X );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_INT_ENABLE2_REG, &data2, C_One_U8X );
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100037
   bma222_write_ee */
/* EasyCASE F */
/** write byte to bma222 EEPROM
   \param addr address to write to (image addresses are automatically extended to EEPROM space)
   \param data byte content to write
   \return result of bus communication function
   \note this function will first write to the image and then trigger the copy of this byte from image
    to the EEPROM by writting to the EEPROM byte
*/
int bma222_write_ee(unsigned char addr, unsigned char data)
   {
   int comres;
   unsigned char ee_busy;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (p_bma222->delay_msec == C_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = bma222_get_ee_w( & ee_busy );
         if (ee_busy)
            {
            comres = E_EEPROM_BUSY;
            }
         else
            {
            comres += p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, addr, &data, C_One_U8X );
                addr += bma222_EEP_OFFSET;
                comres += p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, addr, &data, C_One_U8X );
                p_bma222->delay_msec( bma222_EE_W_DELAY );
            }
         }
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100237
   bma222_set_low_hy */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the Low hysteresis
 *
 *
 *
 *
 *  \param
 *              unsigned char hysval:
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_low_hy(unsigned char hysval)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_LOWG_HYST__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_LOWG_HYST, hysval );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_LOWG_HYST__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100239
   bma222_set_high_hy */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the high hysteresis
 *
 *
 *
 *
 *  \param
 *              unsigned char hysval:
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_high_hy(unsigned char hysval)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_HIGHG_HYST__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_HIGHG_HYST, hysval );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_HIGHG_HYST__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100241
   bma222_set_low_mode */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API sets the Low mode
 *
 *
 *
 *
 *  \param
 *              unsigned char state:
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_set_low_mode(unsigned char state)
   {
   int comres=C_Zero_U8X;
   unsigned char data;
   if (p_bma222==C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_LOWG_INT_MODE__REG, &data, C_One_U8X );
      data = bma222_SET_BITSLICE(data, bma222_LOWG_INT_MODE, state );
      comres = p_bma222->bma222_BUS_WRITE_FUNC(p_bma222->dev_addr, bma222_LOWG_INT_MODE__REG, &data, C_One_U8X);
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ( 100317
   bma222_get_update_image_status */
/* EasyCASE F */
/*******************************************************************************
 * Description: *//**\brief This API gets image update status
 *
 *
 *
 *
 *  \param unsigned char *imagestatus
 *
 *
 *
 *  \return Result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma222_get_update_image_status(unsigned char *imagestatus )
   {
   int comres = C_Zero_U8X ;
   unsigned char data;
   if (p_bma222 == C_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma222->bma222_BUS_READ_FUNC(p_bma222->dev_addr, bma222_EEPROM_CTRL_REG, &data, C_One_U8X );
      data = bma222_GET_BITSLICE(data, bma222_UPDATE_IMAGE);
      *imagestatus = data;
      }
   return comres;
   }
/* EasyCASE ) */
/* EasyCASE ) */
/* EasyCASE ( 253
   Local functions */
/* EasyCASE ) */
/* EasyCASE ) */
