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

#ifndef _RTEM_EVENTS_H_
#define _RTEM_EVENTS_H_

typedef enum {
    E_RTEM_TEST_MNCC_SETUP_CNF_1 = 0,		// for test purpose only, this event is triggered by MNCC_SETUP_CNF, which triggers another event as well.
    E_RTEM_IR_HO_FROM_UTRAN_START,
    E_RTEM_IR_HO_FROM_UTRAN_COMPLETE,
    E_RTEM_IR_HO_FROM_UTRAN_FAIL,
    E_RTEM_IR_HO_TO_UTRAN_START,
    E_RTEM_IR_HO_TO_UTRAN_COMPLETE,
    E_RTEM_IR_HO_TO_UTRAN_FAIL,
    E_RTEM_IR_CCO_FR_UTRAN_START,
    E_RTEM_IR_CCO_FR_UTRAN_COMPLETE,
    E_RTEM_IR_CCO_FR_UTRAN_FAIL,
    E_RTEM_IR_CCO_TO_UTRAN_START,
    E_RTEM_IR_CCO_TO_UTRAN_COMPLETE,
    E_RTEM_IR_CCO_TO_UTRAN_FAIL,
    E_RTEM_IR_RESEL_FR_UTRAN_START,
    E_RTEM_TEST_MNCC_SETUP_STAR,			// for test purpose only, this event is multi-triggered by MNCC_SETUP_* (REQ/CNF).
    E_RTEM_IR_RESEL_FR_UTRAN_COMPLETE,
    E_RTEM_IR_RESEL_FR_UTRAN_FAIL,
    E_RTEM_IR_RESEL_TO_UTRAN_START,
    E_RTEM_IR_RESEL_TO_UTRAN_COMPLETE,
    E_RTEM_IR_RESEL_TO_UTRAN_FAIL,
    E_RTEM_TEST_MNCC_SETUP_CNF_2,			// for test purpose only, this event is triggered by MNCC_SETUP_CNF, which triggers another event as well.
    RTEM_EVENTS_TOTAL
} E_RTEM_EVENT_ID;

#endif  //_RTEM_EVENTS_H_
