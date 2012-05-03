/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html
 *
 *  \section intro_sec Introduction
 * BMA150 / SMB380 3-axis digital Accelerometer In Line Calibration Process
 * This software is compatible with all Bosch Sensortec SMB380 / BMA150 API >=1.1
 *
 *
 */

/*! \file smb380calib.c
    \brief This file contains all function implementatios for the SMB380/BMA150 calibration process

*/

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/input/misc/smb380calib.c
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

#include "smb380.h"
#include "smb380calib.h"

/** calculates new offset in respect to acceleration data and old offset register values
  \param orientation pass orientation one axis needs to be absolute 1 the others need to be 0
  \param *offset_x takes the old offset value and modifies it to the new calculated one
  \param *offset_y takes the old offset value and modifies it to the new calculated one
  \param *offset_z takes the old offset value and modifies it to the new calculated one
 */
int smb380_calc_new_offset(smb380acc_t orientation, smb380acc_t accel,
			   unsigned short *offset_x, unsigned short *offset_y,
			   unsigned short *offset_z)
{
	short old_offset_x, old_offset_y, old_offset_z;
	short new_offset_x, new_offset_y, new_offset_z;

	unsigned char calibrated = 0;

	old_offset_x = *offset_x;
	old_offset_y = *offset_y;
	old_offset_z = *offset_z;

	accel.x = accel.x - (orientation.x * 256);
	accel.y = accel.y - (orientation.y * 256);
	accel.z = accel.z - (orientation.z * 256);

	if ((accel.x > 4) | (accel.x < -4)) {	/* does x axis need calibration? */

		if ((accel.x < 8) && accel.x > 0)	/* check for values less than quantisation of offset register */
			new_offset_x = old_offset_x - 1;
		else if ((accel.x > -8) && (accel.x < 0))
			new_offset_x = old_offset_x + 1;
		else
			new_offset_x = old_offset_x - (accel.x / 8);	/* calculate new offset due to formula */
		if (new_offset_x < 0)	/* check for register boundary */
			new_offset_x = 0;	/* <0 ? */
		else if (new_offset_x > 1023)
			new_offset_x = 1023;	/* >1023 ? */
		*offset_x = new_offset_x;
		calibrated = 1;
	}

	if ((accel.y > 4) | (accel.y < -4)) {	/* does y axis need calibration? */
		if ((accel.y < 8) && accel.y > 0)	/* check for values less than quantisation of offset register */
			new_offset_y = old_offset_y - 1;
		else if ((accel.y > -8) && accel.y < 0)
			new_offset_y = old_offset_y + 1;
		else
			new_offset_y = old_offset_y - accel.y / 8;	/* calculate new offset due to formula */

		if (new_offset_y < 0)	/* check for register boundary */
			new_offset_y = 0;	/* <0 ? */
		else if (new_offset_y > 1023)
			new_offset_y = 1023;	/* >1023 ? */

		*offset_y = new_offset_y;
		calibrated = 1;
	}

	if ((accel.z > 4) | (accel.z < -4)) {	/* does z axis need calibration? */
		if ((accel.z < 8) && accel.z > 0)	/* check for values less than quantisation of offset register */
			new_offset_z = old_offset_z - 1;
		else if ((accel.z > -8) && accel.z < 0)
			new_offset_z = old_offset_z + 1;
		else
			new_offset_z = old_offset_z - (accel.z / 8);	/* calculate new offset due to formula */

		if (new_offset_z < 0)	/* check for register boundary */
			new_offset_z = 0;	/* <0 ? */
		else if (new_offset_z > 1023)
			new_offset_z = 1023;

		*offset_z = new_offset_z;
		calibrated = 1;
	}
	return calibrated;
}

/** reads out acceleration data and averages them, measures min and max
  \param orientation pass orientation one axis needs to be absolute 1 the others need to be 0
  \param num_avg numer of samples for averaging
  \param *min returns the minimum measured value
  \param *max returns the maximum measured value
  \param *average returns the average value
 */

int smb380_read_accel_avg(int num_avg, smb380acc_t *min, smb380acc_t *max,
			  smb380acc_t *avg)
{
	long x_avg = 0, y_avg = 0, z_avg = 0;
	int comres = 0;
	int i;
	smb380acc_t accel;	/* read accel data */

	x_avg = 0;
	y_avg = 0;
	z_avg = 0;
	max->x = -512;
	max->y = -512;
	max->z = -512;
	min->x = 512;
	min->y = 512;
	min->z = 512;

	for (i = 0; i < num_avg; i++) {
		comres += smb380_read_accel_xyz(&accel);	/* read 10 acceleration data triples */
		if (accel.x > max->x)
			max->x = accel.x;
		if (accel.x < min->x)
			min->x = accel.x;

		if (accel.y > max->y)
			max->y = accel.y;
		if (accel.y < min->y)
			min->y = accel.y;

		if (accel.z > max->z)
			max->z = accel.z;
		if (accel.z < min->z)
			min->z = accel.z;

		x_avg += accel.x;
		y_avg += accel.y;
		z_avg += accel.z;

		smb380_pause(10);
	}
	avg->x = x_avg /= num_avg;	/* calculate averages, min and max values */
	avg->y = y_avg /= num_avg;
	avg->z = z_avg /= num_avg;
	return comres;
}

/** verifies the accerleration values to be good enough for calibration calculations
 \param min takes the minimum measured value
  \param max takes the maximum measured value
  \param takes returns the average value
  \return 1: min,max values are in range, 0: not in range
*/

int smb380_verify_min_max(smb380acc_t min, smb380acc_t max, smb380acc_t avg)
{
	short dx, dy, dz;
	int ver_ok = 1;

	dx = max.x - min.x;	/* calc delta max-min */
	dy = max.y - min.y;
	dz = max.z - min.z;

	if (dx > 10 || dx < -10)
		ver_ok = 0;
	if (dy > 10 || dy < -10)
		ver_ok = 0;
	if (dz > 10 || dz < -10)
		ver_ok = 0;

	return ver_ok;
}

/** overall calibration process. This function takes care about all other functions
  \param orientation input for orientation [0;0;1] for measuring the device in horizontal surface up
  \param *tries takes the number of wanted iteration steps, this pointer returns the number of calculated steps after this routine has finished
  \return 1: calibration passed 2: did not pass within N steps
*/

int smb380_calibrate(smb380acc_t orientation, int *tries)
{

	unsigned short offset_x, offset_y, offset_z;
	unsigned short old_offset_x, old_offset_y, old_offset_z;
	int need_calibration = 0, min_max_ok = 0;
	int ltries;

	smb380acc_t min, max, avg;

	smb380_set_range(SMB380_RANGE_2G);
	smb380_set_bandwidth(SMB380_BW_25HZ);

	smb380_set_ee_w(1);

	smb380_get_offset(0, &offset_x);
	smb380_get_offset(1, &offset_y);
	smb380_get_offset(2, &offset_z);

	old_offset_x = offset_x;
	old_offset_y = offset_y;
	old_offset_z = offset_z;
	ltries = *tries;

	do {

		smb380_read_accel_avg(10, &min, &max, &avg);	/* read acceleration data min, max, avg */

		min_max_ok = smb380_verify_min_max(min, max, avg);

		/* check if calibration is needed */
		if (min_max_ok)
			need_calibration =
			    smb380_calc_new_offset(orientation, avg, &offset_x,
						   &offset_y, &offset_z);

		if (*tries == 0)	/*number of maximum tries reached? */
			break;

		if (need_calibration) {
			/* when needed calibration is updated in image */
			(*tries)--;
			smb380_set_offset(0, offset_x);
			smb380_set_offset(1, offset_y);
			smb380_set_offset(2, offset_z);
			smb380_pause(20);
		}
	} while (need_calibration || !min_max_ok);

	if (*tries > 0 && *tries < ltries) {

		if (old_offset_x != offset_x)
			smb380_set_offset_eeprom(0, offset_x);

		if (old_offset_y != offset_y)
			smb380_set_offset_eeprom(1, offset_y);

		if (old_offset_z != offset_z)
			smb380_set_offset_eeprom(2, offset_z);
	}

	smb380_set_ee_w(0);
	smb380_pause(20);
	*tries = ltries - *tries;

	return !need_calibration;
}
