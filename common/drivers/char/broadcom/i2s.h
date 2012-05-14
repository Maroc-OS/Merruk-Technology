/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/char/broadcom/i2s.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/**
* @file     i2s.h
* @brief    Header file for I2S driver for BCM21xx.

****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
*/

#ifndef _I2S_H_
#define _I2S_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
	I2S_RX_LEFT_CHAN,	/**<  recording from left channel in mono mode */
	I2S_RX_RIGHT_CHAN,	/**<  recording from right channel in mono mode */
	I2S_RX_AEVERAGE		/**<  recording with (L + R)/2 in mono mode */
} I2S_RX_CHAN_t;

/* ! return status */
#define I2S_WRONG_STATE                (-1)
#define I2S_SAMPLE_RATE_NOT_SUPPORT    (-2)
#define I2S_BAD_HANDLE                 (-3)
#define I2S_NO_CB_DEFINED              (-4)
#define I2S_DMA_SETUP_ERROR            (-5)

	/** I2S state */
typedef enum {
	I2STST_NOT_INIT,		/**< Default state */
	I2STST_READY,			/**< Driver running */
	I2STST_STREAMING,		/**< Playing audio */
	I2STST_RUNNING_NO_DMA,		/**< I2S direct pass */
	I2STST_PAUSING,			/**< Attempting to pause audio stream */
	I2STST_ERROR			/**< error */
} I2S_STATE;

typedef enum {
	I2S_BLK1_ID = 0,
	I2S_BLK2_ID = 1,
	NUM_I2S_BLOCK
} I2S_BLK_ID_t;

typedef struct {
	unsigned int startAddr;
	unsigned int bufferSize;
	unsigned int bufferNum;
} I2S_Cir_t;

/** pointer to a function returning bool, with a bool input */
typedef bool(*I2sEndCb) (bool dma_ok);
typedef void *I2S_HANDLE;  /**< handle to i2s device */
/**
*
*  @brief Initializes I2S hardware
*  
*  @param 	id (in)          
*  
*  @return  I2S_HANDLE
*
*/

I2S_HANDLE I2SDRV_Init(I2S_BLK_ID_t id);
/**
* @brief    De-initialze the I2S Driver, should be the last function to call
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Cleanup(I2S_HANDLE handle);
/**
* @brief    Configure I2S Master/Slave Mode
*
* @param handle     I2S handle
* @param master     Master/Slave mode
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Set_mode(I2S_HANDLE handle, bool master);

/**
* @brief    Get the I2S device Transmit (TX) state
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
I2S_STATE I2SDRV_Get_tx_state(I2S_HANDLE handle);

/**
* @brief    Get the I2S device Receive (RX) state
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
I2S_STATE I2SDRV_Get_rx_state(I2S_HANDLE handle);

/**
* @brief    Register an end-of-transfer callback function for Transmit (TX)
*
* @param handle     I2S handle
* @param endCb      Pointer to callback function
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Register_tx_Cb(I2S_HANDLE handle, I2sEndCb endCb);

/**
* @brief    Register an end-of-transfer callback function for Receive (RX)
*
* @param handle     I2S handle
* @param endCb      Pointer to callback function
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Register_rx_Cb(I2S_HANDLE handle, I2sEndCb endCb);

/**
* @brief    Configure I2S transmit (TX) format, for Playback
*
* @param handle         I2S handle
* @param sampleRate     Sampling rate
* @param stereo         stereo/mono selection
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Set_tx_format(I2S_HANDLE handle, int sampleRate,
				   bool stereo);

/**
* @brief    Configure I2S Receive (RX) format, for Capture
*
* @param handle         I2S handle
* @param sampleRate     Sampling rate
* @param stereo         stereo/mono selection
* @param mono_from      where mono data is from. only used for mono mode
*
* @return               0 for success, i2s error status on error
*/
unsigned char I2SDRV_Set_rx_format(I2S_HANDLE handle, int sampleRate,
				   bool stereo,
				   I2S_RX_CHAN_t mono_from);

/**
* @brief     Queue a DMA buffer for Transmit (TX)
*
* @param handle     I2S handle
* @param buf        DMA buffer
* @param len        Length of buffer in 16-bit I2S words
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Queue_tx_buf(I2S_HANDLE handle,
				  unsigned short *buf,
				  unsigned int len);

/**
* @brief     Queue a DMA buffer for Receive (Rx)
*
* @param handle     I2S handle
* @param buf        DMA buffer
* @param len        Length of buffer in 16-bit I2S words
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Queue_rx_buf(I2S_HANDLE handle,
				  unsigned short *buf,
				  unsigned int len);

/**
* @brief    Start I2S Transmit (TX) in DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Start_tx(I2S_HANDLE handle);

/**
* @brief    Start I2S Receive (RX) in DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Start_rx(I2S_HANDLE handle);

/**
* @brief    Pause I2S Transmission (TX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Pause_tx(I2S_HANDLE handle);

/**
* @brief    Pause I2S Reception (RX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Pause_rx(I2S_HANDLE handle);

/**
* @brief    Resume previously 'paused' I2S Transmission (TX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Resume_tx(I2S_HANDLE handle);

/**
* @brief    Resume previously 'paused' I2S Reception (RX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Resume_rx(I2S_HANDLE handle);

/**
* @brief    Stop I2S Transmit (TX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Stop_tx(I2S_HANDLE handle);

/**
* @brief    Stop I2S Receive (RX)
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Stop_rx(I2S_HANDLE handle);

/**
* @brief    Enable/Disable I2S internal loopback for testing
*
* @param handle     I2S handle
* @param enable     enable (true/false)
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Enable_Loopback(I2S_HANDLE handle, bool enable);

unsigned short I2SDRV_Get_Data_Count(I2S_HANDLE handle,
				     unsigned char which);
unsigned int I2SDRV_Get_Data_Port(I2S_HANDLE handle);

/**
* @brief    Start I2S Receive (RX) in non DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Start_rx_noDMA(I2S_HANDLE handle);

/**
* @brief Stop I2S Receive (TX) in non DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned char I2SDRV_Stop_rx_noDMA(I2S_HANDLE handle);

/**
* @brief  Obtain I2S Handle
*
* @param id     I2S block enum that is to be initialized
*
* @return       I2S handle, NULL if error
*/
I2S_HANDLE I2SDRV_Get_handle(I2S_BLK_ID_t id);

/**
* @brief    Start I2S Receive (RX) in DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned int I2SDRV_Start_rxDMA(I2S_HANDLE handle);

/**
* @brief    Stop I2S Receive (RX) in DMA mode
*
* @param handle     I2S handle
*
* @return           0 for success, i2s error status on error
*/
unsigned int I2SDRV_Stop_rxDMA(I2S_HANDLE handle);
/**
* @brief     Queue a circular DMA buffer for Transmit (TX)
*
* @param handle     I2S handle
* @param circular        DMA buffer
*
* @return           0 for success, i2s error status on error
*/
unsigned int I2SDRV_Cir_tx_buf(I2S_HANDLE handle, I2S_Cir_t *circular);


/**
* @brief     Queue a circular DMA buffer for Receive (RX)
*
* @param handle     I2S handle
* @param circular        DMA buffer
*
* @return           0 for success, i2s error status on error
*/
unsigned int I2SDRV_Cir_rx_buf(I2S_HANDLE handle, I2S_Cir_t *circular);

#ifdef __cplusplus
}
#endif
#endif				/* _I2S_H_ */
