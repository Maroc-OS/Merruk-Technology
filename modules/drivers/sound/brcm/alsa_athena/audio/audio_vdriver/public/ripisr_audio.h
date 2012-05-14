/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/
/**
*
* @file   ripisr_audio.h
* @brief  
*
******************************************************************************/
#ifndef	__RIPISR_AUDIO_H__
#define	__RIPISR_AUDIO_H__

typedef void (*capture_request_handler_t)(VPStatQ_t reqMsg);

typedef void (*voif_request_handler_t)(UInt32 bufferindex, UInt32 sample_rate);

void register_capture_request_handler( capture_request_handler_t capture_cb );

void register_voif_request_handler( voif_request_handler_t voif_cb );

#endif