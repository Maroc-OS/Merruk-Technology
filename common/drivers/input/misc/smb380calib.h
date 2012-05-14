/*
* This software program is licensed subject to the GNU General Public License
* (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html
*
* SMB380 calibration routine based on SMB380 API
*
* Usage: calibration of SMB380/BMA150 acceleration sensor
*
*/

/*! \file smb380calib.h
    \brief This file contains all function headers for the SMB380/BMA150 calibration process

*/

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/input/misc/smb380calib.h
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

#ifndef __SMB380_CALIBRATION__
#define __SMB380_CALIBRATION__

#define SMB380_CALIBRATION_MAX_TRIES 10

/** calculates new offset in respect to acceleration data and old offset register values
  \param orientation pass orientation one axis needs to be absolute 1 the others need to be 0
  \param *offset_x takes the old offset value and modifies it to the new calculated one
  \param *offset_y takes the old offset value and modifies it to the new calculated one
  \param *offset_z takes the old offset value and modifies it to the new calculated one
 */

int smb380_calibrate(smb380acc_t, int *);

/** reads out acceleration data and averages them, measures min and max
  \param orientation pass orientation one axis needs to be absolute 1 the others need to be 0
  \param num_avg numer of samples for averaging
  \param *min returns the minimum measured value
  \param *max returns the maximum measured value
  \param *average returns the average value
 */
int smb380_read_accel_avg(int, smb380acc_t *, smb380acc_t *, smb380acc_t *);

/** verifies the accerleration values to be good enough for calibration calculations
 \param min takes the minimum measured value
  \param max takes the maximum measured value
  \param takes returns the average value
  \return 1: min,max values are in range, 0: not in range
*/
int smb380_verify_min_max(smb380acc_t, smb380acc_t, smb380acc_t);

/** smb380_calibration routine
  \param orientation pass orientation one axis needs to be absolute 1 the others need to be 0
  \param tries number of iterative passes
  \param *min, *max, *avg returns minimum, maximum and average offset
 */
int smb380_calc_new_offset(smb380acc_t, smb380acc_t, unsigned short *,
			   unsigned short *, unsigned short *);

/** overall calibration process. This function takes care about all other functions
  \param orientation input for orientation [0;0;1] for measuring the device in horizontal surface up
  \param *tries takes the number of wanted iteration steps, this pointer returns the number of calculated steps after this routine has finished
  \return 1: calibration passed 2: did not pass within N steps
*/
int smb380_store_calibration(unsigned short, unsigned short, unsigned short);

#endif /* endif __SMB380_CALIBRATION__ */
