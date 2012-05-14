/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/bcm21553_cpufreq_drv.h
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

#ifndef BCM_CPUFREQ_DRV_H
#define BCM_CPUFREQ_DRV_H

struct bcm_freq_tbl {
	u32 cpu_freq;         /* in MHz */
	u32 cpu_voltage;      /* in uV  */
};

/* Helper to initialize array of above structures */
#define FTBL_INIT(freq, volt)  \
{                              \
	.cpu_freq    = freq,   \
	.cpu_voltage = volt,   \
}

struct bcm_cpu_info {
	/* Table of cpu frequencies and voltages supported for a cpu */
	struct bcm_freq_tbl *freq_tbl;
	/* Number of entries in the DVFS table */
	int num_freqs;

	/* CPU Frequency transition latency in ns */
	u32 cpu_latency;
	/* Name of cpu regulator */
	char *cpu_regulator;
	/* Name of cpu clock */
	char *cpu_clk;
	/* Name of appspll enable clock */
	char *appspll_en_clk;
	/* Index of turbo mode cpu frequency in the DVFS table. Switching
	 * to turbo mode needs special handling.
	 */
	int index_turbo;
};

/* Platform data for BCM21553 cpufreq driver */
struct bcm_cpufreq_drv_plat {
	struct bcm_cpu_info *info;
	/* Number of cpus (hence, number of entries in above table) */
	int nr_cpus;
};

#endif /* BCM_CPUFREQ_DRV_H */
