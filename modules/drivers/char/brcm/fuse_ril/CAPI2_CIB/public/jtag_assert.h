/*********************************************************************
* 
* Copyright 2010 Broadcom Corporation.  All rights reserved.
* 
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
* 
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
***************************************************************************/

#ifdef DSP_COMMS_INCLUDED

#define JTAG_FRIENDLY_L1_ASSERT( expr, value ) { jtag_friendly_l1_assert( (expr), #expr,  __FILE__, __LINE__, value  );\
												 if (!(expr)) return; } 
#else

#define JTAG_FRIENDLY_L1_ASSERT(x, y)

#endif
