/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/bcm21553_cpufreq_gov.h
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

#ifndef BCM21553_CPUFREQ_GOV_H
#define BCM21553_CPUFREQ_GOV_H

struct bcm21553_cpufreq_gov_plat {
	unsigned int freq_osuper;
	unsigned int freq_super;
	unsigned int freq_turbo;
	unsigned int freq_heigher;
	unsigned int freq_omedium;
	unsigned int freq_umedium;
	unsigned int freq_normal;
	unsigned int freq_starter;
	unsigned int freq_lower;
	unsigned int freq_ulower;
};

/* helper for cpufreq_bcm_client_get */
#define cpufreq_bcm_client_get(name)   \
	_cpufreq_bcm_client_get(name, THIS_MODULE)

#if defined(CONFIG_CPU_FREQ)
/* Returns the client id */
extern struct cpufreq_client_desc *_cpufreq_bcm_client_get(const char *name,
						   struct module *owner);
extern void cpufreq_bcm_client_put(struct cpufreq_client_desc *desc);

/* Pass the client id obtained from the client_get function
 * to the following APIs
 */
extern void cpufreq_bcm_dvfs_enable(struct cpufreq_client_desc *desc);
extern void cpufreq_bcm_dvfs_disable(struct cpufreq_client_desc *desc);
#else

#define _cpufreq_bcm_client_get(name, owner) NULL
#define cpufreq_bcm_client_put(desc)
#define cpufreq_bcm_dvfs_enable(desc)
#define cpufreq_bcm_dvfs_disable(desc)

#endif

#endif /* BCM21553_CPUFREQ_GOV_H */
