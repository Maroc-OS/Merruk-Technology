/* **FIXME** stubs for header files not integrated from CP (not required for AP build) */
#ifndef _DEV_SYSPARM_H_
#define _DEV_SYSPARM_H_

#include "ostypes.h"

/*
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned long UInt32;
typedef signed char Int8;
typedef signed short Int16;
typedef signed long Int32;
*/

#define SYS_IMEI_LEN            8

typedef enum
{
	IMEI_1,	// Default IMEI
	IMEI_2	// IMEI associated with second SIM for dual-SIM
} IMEI_TYPE_t;


#undef _RHEA2091_
#define tempINTERFACE_MobC00121082


#define TOTAL_BANDS					4 ///< GSM/DCS/GSM850/PCS
#define IMEI_SIZE					9
#define AGC_GAIN_SIZE          		((UInt16) 64    )    // # of AGC gain entries	//from shared.h.
#define N_MODUS                		((UInt16) 2     )   // Number of TX bands supported by DSP	//from shared.h
#define TX_DB_DAC_SIZE            	((UInt16) 128 )       // dB-to-DAC value conversion table	//from shared.h
#define N_BIAS						2
#define TXOLC_SIZE					112
#define GMSK_TX_LEVELS				20
#define N_TX_LEVELS               	((UInt16) 19  )      // # of tx power levels in version #2	//from shared.h
#define MAX_FREQS					(548 + 299 + 124) ///< GSM900 + DCS + PCS + GSM850
#define BT_DEVICE_ADDR_SIZE			6
#if (defined(_RHEA2091_)&& defined(LUTB0))
	#define				LUT_SIZE 			32	
#else
	#define 			LUT_SIZE			    	128	//Num of LUT entry
#endif 	
#define LUT_MEAS_SIZE			    32	//Num of LUT entry
#define MAX_GAIN_FREQ_CNT			5 // max # of channels to be calibrated per band for PGA gain cal
#define MAX_LNA_SEGMENTS			4 // max # of LNA segments to be calibrated per channel (default is 3 segments)
#define MAX_TX_DAC					22


typedef struct
{

 Int16 txpwr;
 UInt16 txdac;
 Int16 txscaler;

} TxDACTable;

typedef struct
{
	TxDACTable		tx_dactbl[MAX_TX_DAC];

} TxGain;

typedef struct
{
	UInt16 gain_arfcn;
	Int16 gain_error[MAX_LNA_SEGMENTS];
} FreqGainError_t;

typedef struct
{
#if (defined(_RHEA2091_) && defined(LUTB0))
	TxGain			tx_gain[TOTAL_BANDS];
	//TxLUTTable		tx_agc[TOTAL_BANDS][LUT_SIZE];	//holding cells 2-4:the DC I/Q correction, I/Q phase and gain
	UInt16 			txagc_num;
	Int16			clpc2g_vdet_min[ TOTAL_BANDS ];
	Int16			clpc2g_cal_offset[ TOTAL_BANDS ];
	Int16			clpc2g_vdet_slope[ TOTAL_BANDS ];	
#endif
	UInt16			version ;
	UInt8			imei[ IMEI_SIZE ];
	UInt8			filler1[ (IMEI_SIZE % 4) ];
	UInt8			imei_2[ IMEI_SIZE ];	// IMEI associated with second SIM for dual-SIM
	UInt8			filler4[ (IMEI_SIZE % 4) ];
	Int16			freq_dacmult;
	Int16			freq_dacshift;
	Int16			freq_daclsb;
	Int16			sysgain[ TOTAL_BANDS ][ AGC_GAIN_SIZE ];
	Int16			freq_daczero;
	UInt16			tx_db_convert[  TOTAL_BANDS ][ N_MODUS ][ TX_DB_DAC_SIZE ];
	UInt16			txolc_convert_8psk [  TOTAL_BANDS ][ N_BIAS ][ TXOLC_SIZE ];
	UInt16			txolc_convert_gmsk [  TOTAL_BANDS ][ N_BIAS ][ GMSK_TX_LEVELS ];
	// tx_pedestal, tx_base, tx_peak, tx_off, and tx_data should be in independent sysparm
	// keep the dep sysparm space allocated to these parameters for backward compatibility
	UInt8			dummy[2*(TOTAL_BANDS)*(N_MODUS)*(N_TX_LEVELS)*5- 2*(3+1+1) - MAX_FREQS -12];
				// MAX_FREQS bytes for rxlev_comp
				// 2x1 bytes for batt_ActualLowVoltReading
				// 6 bytes for bt_device_addr
	      //use 12 bytes from above for whether default cal is used or not.
	UInt8     cal_signature[2]; //hex ff, 03 for search signature
	UInt8     cal_flag[10]; //hex 64, 65, 66, 61, 75, 6c, 74, 00, 00, 00 for default
	                        //hex 63, 61, 6c, 69, 62, 72, 61, 74, 65, 64 for calibrated
	Int8			rxlev_comp[MAX_FREQS];
	UInt16			cdac;
	UInt8			bt_device_addr[BT_DEVICE_ADDR_SIZE];
	UInt16			batt_ActualLowVoltReading;
	UInt16			batt_Actual4p2VoltReading;
	UInt16			bsi_CalRefHighAdcReading;
	UInt16			bsi_CalRefLowAdcReading;

#ifdef tempINTERFACE_MobC00121082
//	UInt16          tx_agc_gain0  		[ LUT_SIZE ];
//	UInt16          tx_agc_gain1  		[ LUT_SIZE ];
	UInt16          tx_agc_gain2  		[ LUT_SIZE ];
	UInt16          tx_agc_gain3  		[ LUT_SIZE ];
	UInt16          tx_agc_gain4  		[ LUT_SIZE ];
//	UInt16          tx_agc_gain5  		[ LUT_SIZE ];
//	UInt16          tx_agc_diggain  	[ LUT_SIZE ];
#else
	UInt16          tx_agc_gain0  		[ LUT_SIZE ];
	UInt16          tx_agc_gain1  		[ LUT_SIZE ];
	UInt16          tx_agc_gain2  		[ LUT_SIZE ];
	UInt16          tx_agc_gain3  		[ LUT_SIZE ];
	UInt16          tx_agc_gain4  		[ LUT_SIZE ];
	UInt16          tx_agc_gain5  		[ LUT_SIZE ];
	UInt16          tx_agc_diggain  	[ LUT_SIZE ];
#endif
	Int16 			tx_agc_meas_hipwr0   [ TOTAL_BANDS ][ LUT_MEAS_SIZE  ];	
 	Int16 			tx_agc_meas_lopwr0   [ TOTAL_BANDS ][ LUT_MEAS_SIZE  ];
 	Int16 			tx_agc_meas_hipwr1   [ TOTAL_BANDS ][ LUT_MEAS_SIZE  ];
 	Int16 			tx_agc_meas_lopwr1   [ TOTAL_BANDS ][ LUT_MEAS_SIZE  ];	
	UInt16 lna_segment_cnt;
	UInt16 lna_segment[ MAX_LNA_SEGMENTS ];	
	UInt16 gain_idx_th[ MAX_LNA_SEGMENTS ]; // calculated from LNA segment cal points
	UInt16 gain_freq_cnt; // # of channels calibrated per band
	
	FreqGainError_t freq_gain_error_con[ TOTAL_BANDS ][ MAX_GAIN_FREQ_CNT ];
	FreqGainError_t freq_gain_error_agr[ TOTAL_BANDS ][ MAX_GAIN_FREQ_CNT ];

	Int16 xo_cal_temp;		//xo temp during calibration
	Int32 fdac_offset[4];		//freq offset of the 4 fdac values
	Int32 fdaczero_offset;	// freq offset of fdaczero value

	UInt8 rx_iip2[2][4]; // high/low band, ip,in,qp,qn

	UInt16			lcd;	// Check digit for default IMEI
	UInt16			lcd_2;	// Check digit for IMEI 2 for dual-SIM

	UInt16			checksum;
} SysCalDataDep_t;


#endif //_DEV_SYSPARM_H_