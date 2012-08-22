/*******************************************************************************
* Copyright 2010,2011 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/bcm215xx_pm.c
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

#include <linux/types.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/suspend.h>
#include <linux/wakelock.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/dma-mapping.h>
#include <linux/time.h>
#include <linux/math64.h>
#include <linux/pm_qos_params.h>
#include <asm/outercache.h>
#include <asm/hardware/cache-l2x0.h>

#include <mach/hardware.h>
#include <mach/reg_clkpwr.h>
#include <mach/reg_syscfg.h>
#include <mach/reg_uart.h>
#include <mach/reg_irq.h>
#include <mach/bcm21553_cpuidle_drv.h>

#ifdef CONFIG_ARCH_BCM21553_A0
#include <mach/reg_dmac.h>
#endif

#include <linux/broadcom/bcm_rpc.h>

#include "bcm215xx_pm.h"

struct bcm_pm_sleep *bcm_pm_sleep_buf;
EXPORT_SYMBOL(bcm_pm_sleep_buf);
struct bcm_pm_sleep *bcm_pm_sleep_buf_phys;
EXPORT_SYMBOL(bcm_pm_sleep_buf_phys);

extern int pmu_is_charger_inserted();

/* Enable/disable platform deepsleep mode */
static bool bcm_deepsleep_enable = true;
module_param_named(sleep_enable, bcm_deepsleep_enable,
		   bool, S_IRUGO | S_IWUSR | S_IWGRP);

/* Enable/disable platform pedestal mode */
static bool bcm_pedestal_enable = true;
module_param_named(pedestal_enable, bcm_pedestal_enable,
		   bool, S_IRUGO | S_IWUSR | S_IWGRP);

/* Enable/disable platform dormant mode in suspend path */
static bool bcm_dormant_enable = true;
module_param_named(dormant_enable, bcm_dormant_enable,
		   bool, S_IRUGO | S_IWUSR | S_IWGRP);

/* Enable/disable suspend */
static bool bcm_suspend_enable = true;
module_param_named(suspend_enable, bcm_suspend_enable,
		   bool, S_IRUGO | S_IWUSR | S_IWGRP);

/* Enable/disable platform dormant mode in idle path. This
 * variable is no longer used. It is here to prevent the
 * system test code from breaking.
 */
static bool bcm_idle_dormant_enable = true;
module_param_named(idledorm_enable, bcm_idle_dormant_enable,
		   bool, S_IRUGO | S_IWUSR | S_IWGRP);

static int __init early_jtag(char *p)
{
	pr_info("%s: Disabling dormant mode\n", __func__);
	bcm_dormant_enable = false;
	return 0;
}
early_param("jtag", early_jtag);

static void bcm215xx_force_sleep(void);

/*********************************************************************
 *                             DEBUG CODE                            *
 *********************************************************************/

/* Enable pm driver debug code */
#define CONFIG_BCM215XX_PM_DEBUG

#ifdef CONFIG_BCM215XX_PM_DEBUG

/****************************
 * PM  driver debug interface
 ****************************/

/* PM driver debug interface is accessed via:
 * /sys/module/bcm215xx_pm/parameters/debug
 */

/* Enable/disable debug logs */
enum {
	/* Disable all logging and low power states */
	DEBUG_DISABLE = 0U,
	/* Enable logging power related regs in suspend handler */
	DEBUG_SUSPEND_LOG_ENABLE = (1U << 0),
	/* Enable logging power related regs in idle handler */
	DEBUG_PEDESTAL_LOG_ENABLE = (1U << 1),
	/* Enable instrumentation code in suspend and idle handlers */
	DEBUG_INSTRUMENTATION_ENABLE = (1U << 2),
	/* Snapshot Athena registers before suspend */
	DEBUG_SUSPEND_SNAPSHOT = (1U << 4),
};

#define DEFAULT_LOG_LVL            \
	(DEBUG_SUSPEND_LOG_ENABLE | \
	DEBUG_INSTRUMENTATION_ENABLE)

struct debug {
	int log_lvl;
};

#define __param_check_debug(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }

#define param_check_debug(name, p) \
	__param_check_debug(name, p, debug)

static int param_set_debug(const char *val, struct kernel_param *kp);
static int param_get_debug(char *buffer, struct kernel_param *kp);

static struct debug debug = {
	.log_lvl = DEFAULT_LOG_LVL,
};
module_param_named(debug, debug, debug, S_IRUGO | S_IWUSR | S_IWGRP);

/* Helpers */
#define IS_SUSPEND_LOG_ENABLED (debug.log_lvl & DEBUG_SUSPEND_LOG_ENABLE)
#define IS_PEDESTAL_LOG_ENABLED (debug.log_lvl & DEBUG_PEDESTAL_LOG_ENABLE)
#define IS_INSTRUMENTATION_ENABLED \
	(debug.log_lvl & DEBUG_INSTRUMENTATION_ENABLE)
#define IS_SUSP_SNAPSHOT_ENABLED (debug.log_lvl & DEBUG_SUSPEND_SNAPSHOT)

/*
 * Command handlers
 */
static void cmd_force_sleep(const char *p)
{
	bcm215xx_force_sleep();
}

static void bcm_print_cntrs(void);
static void cmd_show_stats(const char *p)
{
	pr_info("log_lvl: 0x%x\n", debug.log_lvl);
	bcm_print_cntrs();
}

static void bcm_clear_hw_cntrs(void);
static void cmd_clear_stats(const char *p)
{
	bcm_clear_hw_cntrs();
}

static void cmd_set_log_lvl(const char *p)
{
	sscanf(p, "%x", &debug.log_lvl);
}

static void cmd_set_dorm_dbg(const char *p)
{
	sscanf(p, "%x", &bcm_pm_sleep_buf->dormant_sequence_dbg);
}

/* Uncomment this to take snapshot of registers before suspend */
/* #define BCM21553_SUSPEND_SNAPSHOT */
#ifdef BCM21553_SUSPEND_SNAPSHOT

/* Addresses of syscfg, gpio registers to be dumped before the system
 * enters suspend state.
 */
static u32 snapshot_reg_addr[] = {
	/* SYSCFG Regs */
	0x8880000, 0x8880004, 0x8880008, 0x888000c, 0x8880010, 0x8880014,
	0x8880018, 0x888001c, 0x8880020, 0x8880024, 0x8880028, 0x888002c,
	0x8880030, 0x8880038, 0x8880040, 0x8880044, 0x8880048, 0x8880050,
	0x8880054, 0x8880060, 0x8880064, 0x8880080, 0x8880084, 0x8880088,
	0x888008c, 0x8880090, 0x8880094, 0x8880098, 0x888009c, 0x88800a0,
	0x88800a4, 0x88800a8, 0x88800ac, 0x88800b0, 0x88800b4, 0x88800b8,
	0x88800bc, 0x88800c0, 0x88800c4, 0x88800cc, 0x88800d0, 0x88800d8,
	0x88800dc, 0x88800e0, 0x88800e4, 0x8880100, 0x8880104, 0x8880108,
	0x888010c, 0x8880110, 0x8880114, 0x8880118, 0x888011c, 0x8880120,
	0x8880124, 0x888012c, 0x8880130, 0x8880134, 0x8880138, 0x888013c,
	0x8880144, 0x888014c, 0x8880150, 0x8880154, 0x888015c, 0x8880160,
	0x8880164, 0x8880168, 0x888016c, 0x8880170, 0x8880174, 0x8880178,
	0x888017c, 0x8880180, 0x8880184, 0x888018c, 0x8880190, 0x88801a0,
	0x88801a4, 0x88801c0, 0x88801c4, 0x88801c8, 0x88801cc, 0x88801e0,
	0x88801fc, 0x8880200, 0x8880204, 0x8880220, 0x8880240, 0x8880244,
	/* GPIO Regs */
	0x88ce000, 0x88ce004, 0x88ce008, 0x88ce00c, 0x88ce010, 0x88ce014,
	0x88ce018, 0x88ce01c, 0x88ce020, 0x88ce024, 0x88ce028, 0x88ce02c,
	0x88ce030, 0x88ce034, 0x88ce038, 0x88ce03c, 0x88ce040, 0x88ce044,
	0x88ce048, 0x88ce04c, 0x88ce050, 0x88ce054, 0x88ce060, 0x88ce064,
	0x88ce068, 0x88ce06c, 0x88ce070, 0x88ce074, 0x88ce078, 0x88ce07c,
	/* CLKPWR Regs */
	0x8140000, 0x8140004, 0x8140008, 0x814000c, 0x8140010, 0x8140014,
	0x8140018, 0x814001c, 0x8140020, 0x8140024, 0x8140028, 0x814002c,
	0x8140030, 0x8140034, 0x8140038, 0x814003c, 0x8140040, 0x8140044,
	0x8140048, 0x814004c, 0x8140050, 0x8140054, 0x8140058, 0x814005c,
	0x8140060, 0x8140064, 0x8140068, 0x814006c, 0x8140070, 0x8140074,
	0x8140078, 0x814007c, 0x8140080, 0x8140084, 0x8140088, 0x814008c,
	0x8140090, 0x8140094, 0x8140098, 0x814009c, 0x81400a0, 0x81400a4,
	0x81400a8, 0x81400ac, 0x81400b0, 0x81400b4, 0x81400b8, 0x81400bc,
	0x81400d0, 0x81400d4, 0x81400d8, 0x81400dc, 0x81400e0, 0x81400e4,
	0x81400e8, 0x81400ec, 0x81400f0, 0x81400f4, 0x81400f8, 0x81400fc,
	0x8140100, 0x8140104, 0x8140108, 0x8140110, 0x8140114, 0x8140118,
	0x8140120, 0x8140128, 0x814012c, 0x8140134, 0x814013c, 0x8140140,
	0x8140148, 0x8140150, 0x8140154, 0x814015c, 0x8140160, 0x8140168,
	0x814017c, 0x8140180, 0x8140188, 0x81401a0, 0x81401a4, 0x81401a8,
	0x81401ac, 0x81401b0, 0x81401b4, 0x81401b8, 0x81401bc, 0x81401c0,
	0x81401c4, 0x81401c8, 0x81401cc, 0x81401d0, 0x81401d4, 0x81401f0,
	0x81401f4, 0x81401f8, 0x81401fc, 0x8140200, 0x8140220, 0x8140224,
	0x8140228, 0x814022c, 0x8140230, 0x8140234, 0x8140238, 0x814023c,
	0x8140240, 0x8140244, 0x8140248, 0x814024c, 0x8140250, 0x8140254,
	0x8140258, 0x814025c, 0x8140260, 0x8140264, 0x8140268, 0x814026c,
	0x8140270, 0x8140274, 0x8140278, 0x814027c, 0x8140280, 0x8140284,
	0x8140288, 0x8140298, 0x814029c, 0x81402a0, 0x81402a4, 0x81402a8,
	0x81402ac, 0x81402b0, 0x81402b4, 0x81402b8, 0x81402bc, 0x81402c0,
	0x81402c4, 0x81402c8, 0x81402cc, 0x81402d0, 0x81402d4,
};

static u32 suspend_snapshot[ARRAY_SIZE(snapshot_reg_addr)];

static void susp_snapshot(void)
{
	int i;
	int nregs;

	nregs = ARRAY_SIZE(snapshot_reg_addr);

	for (i = 0; i < nregs; i++)
		suspend_snapshot[i] = readl(io_p2v(snapshot_reg_addr[i]));
}
#endif /* BCM21553_SUSPEND_SNAPSHOT */

static void cmd_show_susp_snapshot(const char *p)
{
#ifdef BCM21553_SUSPEND_SNAPSHOT
	int i;
	int nregs;

	nregs = ARRAY_SIZE(snapshot_reg_addr);

	for (i = 0; i < nregs; i++)
		pr_info("%08x: 0x%08x\n", snapshot_reg_addr[i],
			suspend_snapshot[i]);
#else
	pr_info("%s: Suspend snapshot is not compiled in\n", __func__);
#endif
}

static void cmd_set_pm_qos(const char *p)
{
	static struct pm_qos_request_list *pm_qos_req;
	int qos;

	if (*p == 'a') {
		/* Skip past the sub command */
		while (*p != ' ' && *p != '\t')
			p++;
		/* Skip white spaces */
		while (*p == ' ' || *p == '\t')
			p++;

		sscanf(p, "%x", &qos);
		pr_info("qos: %d\n", qos);

		pm_qos_req = pm_qos_add_request(PM_QOS_CPU_DMA_LATENCY,
						qos);
	} else if (*p == 'r') {
		pm_qos_remove_request(pm_qos_req);
	} else {
		pr_info("Unknown command: %d\n", *p);
	}
}

/* List of commands supported */
enum {
	CMD_SHOW_STATS = 's',
	CMD_CLEAR_STATS = 'c',
	CMD_SET_LOG_LVL = 'l',
	CMD_SET_DORM_DBG = 'd',
	CMD_SHOW_HELP = 'h',
	CMD_FORCE_SLEEP = 'f',
	CMD_SUSP_SNAPSHOT = 'r',
	CMD_SET_PM_QOS = 'q',
};

static void cmd_show_usage(void)
{
	const char usage[] = "Usage:\n"
	  "1. echo 'cmd string' > /sys/module/bcm215xx_pm/parameters/debug\n"
	  "'cmd string' must be constructed as follows:\n"
	  "Update log level: l 0x01\n"
	  "Show stats: s\n"
	  "Clear stats: c\n"
	  "Enable/disable dormant debug: d [1/0]\n"
	  "Display usage: h\n"
	  "Force sleep: f\n\n"
	  "Take register snapshot before suspend entry: r\n"
	  "Add PM QOS request: q a <number>\n"
	  "Remove PM QOS request: q r\n"
	  "2. Print dormant buffer:\n"
	  "cat /sys/module/bcm215xx_pm/parameters/debug\n";

	pr_info("%s", usage);
}

static int param_set_debug(const char *val, struct kernel_param *kp)
{
	const char *p;

	if (!val)
		return -EINVAL;

	/* Command is only one character followed by a space. Arguments,
	 * if any, starts from offset 2 in val.
	 */
	p = &val[2];

	switch (val[0]) {
	case CMD_SHOW_STATS:
		cmd_show_stats(p);
		break;
	case CMD_CLEAR_STATS:
		cmd_clear_stats(p);
		break;
	case CMD_SET_LOG_LVL:
		cmd_set_log_lvl(p);
		break;
	case CMD_SET_DORM_DBG:
		cmd_set_dorm_dbg(p);
		break;
	case CMD_FORCE_SLEEP:
		cmd_force_sleep(p);
		break;
	case CMD_SUSP_SNAPSHOT:
		cmd_show_susp_snapshot(p);
		break;
	case CMD_SET_PM_QOS:
		cmd_set_pm_qos(p);
		break;
	case CMD_SHOW_HELP: /* Fall-through */
	default:
		cmd_show_usage();
		break;
	}
	return 0;
}

static const char *slpbuf_field_names[] = {
	__stringify(DEEPSLEEP_ALLOWED),
	__stringify(DORMANT_ALLOWED),
	__stringify(PEDESTAL_ALLOWED),
	__stringify(DEEPSLEEP_COUNT),
	__stringify(DORMANT_COUNT),
	__stringify(PEDESTAL_COUNT),
	__stringify(DORM_PEDESTAL_COUNT),
	__stringify(DORM_DEEPSLEEP_COUNT),
	__stringify(PEDESTAL_TIME),
	__stringify(DORMANT_VERIFY),
	__stringify(CHECKSUM_ENTER),
	__stringify(CHECKSUM_EXIT),
	__stringify(CHECKSUM_COMPARE),
	__stringify(PRE_DORMANT_CPSR),
	__stringify(DORMANT_SAVE_STAT),
	__stringify(DORMANT_EXIT_STAT),
	__stringify(DORMANT_SAVE_CNT),
	__stringify(DORMANT_EXIT_CNT),
	__stringify(DORMANT_INT_CNT),
	__stringify(DORMANT_FAIL_CNT),
	__stringify(DORMANT_FAIL_STAT),
	__stringify(DORMANT_SEQUENCE_DBG),
	__stringify(DORMANT_STORE_END),
	__stringify(L2CACHE_INIT_FLAG),
	__stringify(L2_EVICT_BUF),
	__stringify(DORMANT_SCRATCH1),
	__stringify(DORMANT_SCRATCH2),
	__stringify(DORMANT_SCRATCH3),
};

static int bcm_fill_reg_log(char *buffer);
static int param_get_debug(char *buffer, struct kernel_param *kp)
{
	int sz = 0;
	int i;

	u32 *ptr = (u32 *) bcm_pm_sleep_buf;

	/* Print the contents of the sleep buffer */
	sz += snprintf(buffer + sz, (SZ_4K - sz), "%-30s: %x\n",
		       slpbuf_field_names[0], bcm_deepsleep_enable);
	sz += snprintf(buffer + sz, (SZ_4K - sz), "%-30s: %x\n",
		       slpbuf_field_names[1], bcm_dormant_enable);
	for (i = 2; i < PM_NUM_SLEEP_ELEMENTS; i++) {
		sz += snprintf(buffer + sz, (SZ_4K - sz), "%-30s: %x\n",
			       slpbuf_field_names[i], ptr[i]);
	}

	sz += bcm_fill_reg_log(buffer + sz);

	return ((sz > SZ_4K) ? SZ_4K : sz);
}

#define L2X0_AUX_CTRL_RO	(L2X0_AUX_CTRL + L2CACHE_READREG_OFFSET)
#define L2X0_CTRL_RO		(L2X0_CTRL + L2CACHE_READREG_OFFSET)

static inline void bcm_print_cntrs(void)
{
	/* print the required fields from the sleep flow debug structure */
	pr_info("deepsleep_cnt: %d, dorm_cnt: %d, pedestal_cnt: %d "
		"dorm_ped_cnt: %d, dorm_slp_cnt: %d\n"
		"csum_comp: 0x%x, int_cnt: 0x%x",
		bcm_pm_sleep_buf->deepsleep_count,
		bcm_pm_sleep_buf->dormant_count,
		bcm_pm_sleep_buf->pedestal_count,
		bcm_pm_sleep_buf->dorm_pedestal_count,
		bcm_pm_sleep_buf->dorm_deepsleep_count,
		bcm_pm_sleep_buf->chksum_compare,
		bcm_pm_sleep_buf->int_cnt);
	pr_info("l210 aux ctrl: %x; l210 ctrl: %x\n",
		readl(IO_ADDRESS(BCM21553_L210_BASE) + L2X0_AUX_CTRL_RO),
		readl(IO_ADDRESS(BCM21553_L210_BASE) + L2X0_CTRL_RO));
}

/****************************************************
 * Collect dormant, pedestal and deepsleep statistics
 ****************************************************/
#define WINDOW_SIZE	256

static int moving_win_avg(u32 sample)
{
	static u32 data_store[WINDOW_SIZE + 1];
	static u32 run_sum = 0;
	static int nr_samples = 0;
	static int top;
	static int bottom = 0;
	u32 run_avg = 0;

	data_store[top] = sample;
	run_sum += sample;
	top++;
	if (top == (WINDOW_SIZE + 1))
		top = 0;

	if (nr_samples < WINDOW_SIZE)
		nr_samples++;
	else {
		if (bottom == (WINDOW_SIZE + 1))
			bottom = 0;
		run_sum -= data_store[bottom];
		bottom++;
		run_avg = run_sum / WINDOW_SIZE;
	}

	return run_avg;
}

#define AVG_INT(x)     ((x) >> FSHIFT)
#define AVG_FRAC(x)    AVG_INT(((x) & (FIXED_1-1)) * 100)

#define AVG_FREQ	(2 * HZ)
/* This exponent is for 2Hz */
#define EXP_1_2SEC	1981

#define CALC_AVG(avg, exp, n)   do {  \
	avg *= exp;                   \
	avg += n*(FIXED_1-exp);       \
	avg >>= FSHIFT;               \
	} while (0)

unsigned long calc_ped_avg(unsigned int time_ped, int print)
{
	static unsigned long timeout;
	unsigned long avg1m_ped; /* 1 minute EMA of pedestal time */

	if (!timeout)
		timeout = jiffies + AVG_FREQ;

	avg1m_ped = moving_win_avg(time_ped);
	if (time_is_before_jiffies(timeout)) {
		timeout = jiffies + AVG_FREQ;
		CALC_AVG(avg1m_ped, EXP_1_2SEC, time_ped);

		if (print)
			pr_info("ped: %10d,1m: %10lu, 1m EMA: %lu.%02lu\n",
				time_ped, avg1m_ped,
				AVG_INT(avg1m_ped),
				AVG_FRAC(avg1m_ped));
	}

	/* Clear pedestal elapsed time counter because we want instantaneous
	 * value of pedestal counter.
	 */
	writel(0x0, ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_CTRL);
	writel(0x3, ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_CTRL);

	return avg1m_ped;
}

/********************************************************
 * Log registers during suspend / idle states to identify
 * the cause of failure to enter suspend/idle states
 ********************************************************/

/* These macros are defined to have smaller names for the clkpwr module
 * regs (to silence checkpatch.pl).
 */
#define _ELAPSED_ACTIVE_TIME_LOW     \
	ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_ACTIVE_TIME_LOW
#define _ELAPSED_ACTIVE_TIME_HIGH    \
	ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_ACTIVE_TIME_HIGH

/* List of registers to log during suspend */
enum {
	AHB_CLK_GATE_MASK,
	AHB_CLK_GATE_FORCE,
	AHB_CLK_GATE_MON_RAW,
	AHB_CLK_GATE_MON,
	CLK_POWER_MODES,
	CLK_AP_POWER_MODES,
	CLK_PM_SLEEP_REQ_MON,
	CLK_PM_SLEEP_REQ_MASK,
	CLK_PM_SLEEP_REQ_FORCE,
	ELAPSED_ACTIVE_TIME_LOW,
	ELAPSED_ACTIVE_TIME_HIGH,
	UARTA_UCR,
	UARTB_UCR,
	UARTC_UCR,
	DMAC_CLK_EN,
	DMAC_CLK_MODE,
	PM_DEBUG_REGS_MAX
};

struct bcm_reg_log {
	u32 id;
	u32 reg;
	u32 addr;
	const char *name;
};

/* Helper */
#define PM_DBG_TBL(i, a) \
{ \
	.id = i, \
	.addr = a, \
	.name = (const char *)__stringify(i), \
}

static struct bcm_reg_log bcm_reg_log[] = {
	PM_DBG_TBL(AHB_CLK_GATE_MASK, ADDR_SYSCFG_AHB_CLK_GATE_MASK),
	PM_DBG_TBL(AHB_CLK_GATE_FORCE, ADDR_SYSCFG_AHB_CLK_GATE_FORCE),
	PM_DBG_TBL(AHB_CLK_GATE_MON_RAW, ADDR_SYSCFG_AHB_CLK_GATE_MONITOR_RAW),
	PM_DBG_TBL(AHB_CLK_GATE_MON, ADDR_SYSCFG_AHB_CLK_GATE_MONITOR),
	PM_DBG_TBL(CLK_POWER_MODES, ADDR_CLKPWR_CLK_POWER_MODES),
	PM_DBG_TBL(CLK_AP_POWER_MODES, ADDR_CLKPWR_CLK_AP_POWER_MODES),
	PM_DBG_TBL(CLK_PM_SLEEP_REQ_MON, ADDR_CLKPWR_CLK_PM_SLEEP_REQ_MON),
	PM_DBG_TBL(CLK_PM_SLEEP_REQ_MASK, ADDR_CLKPWR_CLK_PM_SLEEP_REQ_MASK),
	PM_DBG_TBL(CLK_PM_SLEEP_REQ_FORCE, ADDR_CLKPWR_CLK_PM_SLEEP_REQ_FORCE),
	PM_DBG_TBL(ELAPSED_ACTIVE_TIME_LOW, _ELAPSED_ACTIVE_TIME_LOW),
	PM_DBG_TBL(ELAPSED_ACTIVE_TIME_HIGH, _ELAPSED_ACTIVE_TIME_HIGH),
	PM_DBG_TBL(UARTA_UCR, ADDR_UARTA_UCR),
	PM_DBG_TBL(UARTB_UCR, ADDR_UARTB_UCR),
	PM_DBG_TBL(UARTC_UCR, ADDR_UARTC_UCR),
	PM_DBG_TBL(DMAC_CLK_EN, ADDR_SYSCFG_DMAC_AHB_CLK_EN),
	PM_DBG_TBL(DMAC_CLK_MODE, ADDR_SYSCFG_DMAC_AHB_CLK_MODE),
};

static inline void bcm_log_regs(int stage)
{
	u32 i;

	switch (stage) {
	case 1:
		for (i = 0; i < PM_DEBUG_REGS_MAX; i++)
			bcm_reg_log[i].reg = readl(bcm_reg_log[i].addr);
		break;
	default:
		break;
	}
}

static int bcm_fill_reg_log(char *buffer)
{
	int sz = 0;
	int i;

	/* Print the power management related registers saved in the
	 * pm idle function.
	 */
	for (i = 0; i < PM_DEBUG_REGS_MAX; i++) {
		sz += snprintf(buffer + sz, (SZ_4K - sz), "%-30s: %x\n",
			       bcm_reg_log[i].name, bcm_reg_log[i].reg);
	}

	return sz;
}

#ifdef CONFIG_ARM1136JFS_CP15_DEBUG
/***************************************************
 * Log CP15 registers before and after dormant state
 ***************************************************/
static const char *cp15regs[] = {
	"TLB Lockdown",
	"Primary Region Remap",
	"Normal Memory Remap",
	"Control register",
	"Auxiliary Control register",
	"Co-processor Access Control register",
	"TTBR0",
	"TTBR1",
	"TTBCR",
	"DAC",
	"DFSR",
	"IFSR",
	"FAR",
	"IFAR",
	"D Cache Lockdown",
	"I Cache Lockdown",
	"D-TCM region",
	"I-TCM region",
	"FCSE PID",
	"Context ID",
	"User R/W Thread/PID",
	"User RO Thread/PID",
	"Priv Only Thread/PID",
	"Data Mem Remap",
	"Instr Mem Remap",
	"DMA Mem Remap",
	"PP Mem Remap",
	"Performance monitor control register",
	"Cycle Counter register",
	"Count register 0",
	"Count register 1",
	"DMA identification and status register",
	"Data debug cache register",
	"Instruction debug cache register",
	"Cache Debug Control",
	"Data MicroTLB Index Register",
	"Instruction MicroTLB Index Register",
	"TLB Debug Control",
	"Main TLB VA Entry 0",
	"Main TLB PA Entry 0",
	"Main TLB Attr Entry 0",
	"Main TLB VA Entry 1",
	"Main TLB PA Entry 1",
	"Main TLB Attr Entry 1",
	"Main TLB VA Entry 2",
	"Main TLB PA Entry 2",
	"Main TLB Attr Entry 2",
	"Main TLB VA Entry 3",
	"Main TLB PA Entry 3",
	"Main TLB Attr Entry 3",
	"Main TLB VA Entry 4",
	"Main TLB PA Entry 4",
	"Main TLB Attr Entry 4",
	"Main TLB VA Entry 5",
	"Main TLB PA Entry 5",
	"Main TLB Attr Entry 5",
	"Main TLB VA Entry 6",
	"Main TLB PA Entry 6",
	"Main TLB Attr Entry 6",
	"Main TLB VA Entry 7",
	"Main TLB PA Entry 7",
	"Main TLB Attr Entry 7",
	"Dcache master valid 0",
	"Dcache master valid 1",
	"Dcache master valid 2",
	"Dcache master valid 3",
	"Data smartcache master valid 0",
	"Data smartcache master valid 1",
	"Data smartcache master valid 2",
	"Data smartcache master valid 3",
	"Icache master valid 0",
	"Icache master valid 1",
	"Icache master valid 2",
	"Icache master valid 3",
	"Instr smartcache master valid 0",
	"Instr smartcache master valid 1",
	"Instr smartcache master valid 2",
	"Instr smartcache master valid 3",
	"TLB master valid 0",
	"TLB master valid 1",
};

static int cp15reg_cnt = ARRAY_SIZE(cp15regs);
static u32 cp15regs_before_slp[ARRAY_SIZE(cp15regs)];
static u32 cp15regs_after_slp[ARRAY_SIZE(cp15regs)];

static void cp15dbg_print_buffers(void)
{
	int i;

	pr_info("CP15 register snapshot before sleep\n");
	for (i = 0; i < cp15reg_cnt; i++)
		pr_info("%-42s: 0x%08x\n", cp15regs[i], cp15regs_before_slp[i]);

	pr_info("CP15 register snapshot after sleep\n");
	for (i = 0; i < cp15reg_cnt; i++)
		pr_info("%-42s: 0x%08x\n", cp15regs[i], cp15regs_after_slp[i]);
}
#endif /* CONFIG_ARM1136JFS_CP15_DEBUG */

static inline void bcm_clear_hw_cntrs(void)
{
	/* Clear the elapsed time count registers */
	writel(0x0, ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_ACTIVE_TIME_CTRL);
	writel(0x0, ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_CTRL);
	writel(0x0, ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DEEPSLEEP_TIME_CTRL);

	/* Enabling elapsed time count for various states */
	writel(0x3, ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_ACTIVE_TIME_CTRL);
	writel(0x3, ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_CTRL);
	writel(0x3, ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DEEPSLEEP_TIME_CTRL);
}

/* Instrumentation code to log the following statistics on athena low power
 * modes:
 * 1. Count of the number of times system entered into dormant, pedestal,
 *    deepsleep modes.
 * 2. Total time spent in low power state (output of do_gettimeofday()).
 */

/* Mask for elapsed time counters in clk_pwr module */
#define ELAPSED_TIME_REG_MASK		0x0000FFFF
#define ELAPSED_TIME_HIGH_REG_SHIFT	16

/* Low power mode statistics */
struct lpm_stats {
	/* pedestal counter before pedestal entry */
	u32 ped_cnt_entry;
	/* deepsleep counter before deepsleep entry */
	u32 slp_cnt_entry;
	/* dormant counter before idle state entry */
	u32 dor_cnt_idle_entry;
	/* dormant counter before suspend state entry */
	u32 dor_cnt_slp_entry;
	/* time before entry to low power state */
	struct timeval ped_tm_entry;
};

/* Storage for stats before entering into low power state */
static struct lpm_stats lpm_stats;

static inline void instrument_idle_entry(void)
{
	u32 val;

	/* Log pedestal elapsed time counter before entering low power state */
	val = readl(ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_LOW);
	lpm_stats.ped_cnt_entry = val & ELAPSED_TIME_REG_MASK;

	val = readl(ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_HIGH);
	val = (val & ELAPSED_TIME_REG_MASK) << ELAPSED_TIME_HIGH_REG_SHIFT;
	lpm_stats.ped_cnt_entry |= val;

	/* Log dormant counter before entering low power state */
	lpm_stats.dor_cnt_idle_entry = bcm_pm_sleep_buf->dormant_count;

	/* Log current time */
	do_gettimeofday(&lpm_stats.ped_tm_entry);
}

static inline void instrument_idle_exit(void)
{
	struct timeval ped_tm_exit;
	u32 val;
	u32 ped_cnt, dor_cnt;
	/* Indicates whether system entered to low power states or not */
	bool ped, dor;

	/* Get total time spent in low power state */
	do_gettimeofday(&ped_tm_exit);

	val = (ped_tm_exit.tv_sec - lpm_stats.ped_tm_entry.tv_sec);
	val *= USEC_PER_SEC; /* Convert s to us */
	val += (ped_tm_exit.tv_usec - lpm_stats.ped_tm_entry.tv_usec);

	bcm_pm_sleep_buf->pedestal_time = val;

	/* Log pedestal elapsed time counter after exiting low power state */
	val = readl(ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_LOW);
	ped_cnt = val & ELAPSED_TIME_REG_MASK;

	val = readl(ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_PEDESTAL_TIME_HIGH);
	ped_cnt |= (val & ELAPSED_TIME_REG_MASK) << ELAPSED_TIME_HIGH_REG_SHIFT;

	/* Log dormant counter after exiting low power state */
	dor_cnt = bcm_pm_sleep_buf->dormant_count;

	/* Determine if system entered into various low power states */
	ped = (ped_cnt != lpm_stats.ped_cnt_entry) ? true : false;
	dor = (dor_cnt != lpm_stats.dor_cnt_idle_entry) ? true : false;

	/* Update the various low power state counters. Note that dormant
	 * mode counter has already got incremented in the dormant asm
	 * sequence.
	 */
	if (ped)
		bcm_pm_sleep_buf->pedestal_count++;
	if (ped && dor)
		bcm_pm_sleep_buf->dorm_pedestal_count++;
}

static inline void instrument_suspend_entry(void)
{
	u32 val;

	/*
	 * Log deepsleep elapsed time counter before entering low power state
	 */
	val = readl(ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DEEPSLEEP_TIME_LOW);
	lpm_stats.slp_cnt_entry = val & ELAPSED_TIME_REG_MASK;

	val = readl(ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DEEPSLEEP_TIME_HIGH);
	val = (val & ELAPSED_TIME_REG_MASK) << ELAPSED_TIME_HIGH_REG_SHIFT;
	lpm_stats.slp_cnt_entry |= val;

	/* Log dormant counter before entering low power state */
	lpm_stats.dor_cnt_slp_entry = bcm_pm_sleep_buf->dormant_count;
}

static inline void instrument_suspend_exit(void)
{
	u32 val;
	u32 slp_cnt, dor_cnt;
	/* Indicates whether system entered to low power states or not */
	bool slp, dor;

	/*
	 * Log deepsleep elapsed time counter after exiting low power state
	 */
	val = readl(ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DEEPSLEEP_TIME_LOW);
	slp_cnt = val & ELAPSED_TIME_REG_MASK;

	val = readl(ADDR_CLKPWR_CLK_SYSCLK_ELAPSED_DEEPSLEEP_TIME_HIGH);
	slp_cnt |= (val & ELAPSED_TIME_REG_MASK) << ELAPSED_TIME_HIGH_REG_SHIFT;

	/* Log dormant counter after exiting low power state */
	dor_cnt = bcm_pm_sleep_buf->dormant_count;

	/* Determine if system entered into various low power states */
	slp = (slp_cnt != lpm_stats.slp_cnt_entry) ? true : false;
	dor = (dor_cnt != lpm_stats.dor_cnt_slp_entry) ? true : false;

	/* Update the various low power state counters. Note that dormant
	 * mode counter has already got incremented in the dormant asm
	 * sequence.
	 */
	if (slp)
		bcm_pm_sleep_buf->deepsleep_count++;
	if (slp && dor)
		bcm_pm_sleep_buf->dorm_deepsleep_count++;
}

/****************************************
 * Force entry to suspend/pedestal states
 ****************************************/

/* This macro enables code that forces condition required for entry to the
 * dormant and deepsleep states.
 */
/* #define BCM_PM_FORCE_SLEEP */

#ifdef BCM_PM_FORCE_SLEEP
static void bcm_pm_debug_force(int stage)
{
	switch (stage) {
	default:
		break;
	}
}

static void bcm_pm_debug_restore(int stage)
{
	switch (stage) {
	default:
		break;
	}
}
#endif /* BCM_PM_FORCE_SLEEP */

#else /* CONFIG_BCM215XX_PM_DEBUG */

/* Helpers */
#define IS_SUSPEND_LOG_ENABLED (false)
#define IS_PEDESTAL_LOG_ENABLED (false)
#define IS_INSTRUMENTATION_ENABLED (false)

#define bcm_clear_hw_cntrs()
#define bcm_log_regs(stage)
#define bcm_print_cntrs()
#define instrument_idle_entry()
#define instrument_idle_exit()
#define instrument_suspend_entry()
#define instrument_suspend_exit()

#endif /* CONFIG_BCM215XX_PM_DEBUG */

/*********************************************************************
 *                     ENABLE/DISABLE SLEEP STATES                   *
 *********************************************************************/

static void bcm215xx_enable_pedestal(void)
{
	u32 reg_val;

	reg_val = readl(ADDR_CLKPWR_CLK_AP_POWER_MODES);
	reg_val |= CLK_POWER_MODES_PEDESTAL_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_AP_POWER_MODES);
}

static void bcm215xx_disable_pedestal(void)
{
	u32 reg_val;

	reg_val = readl(ADDR_CLKPWR_CLK_AP_POWER_MODES);
	reg_val &= ~CLK_POWER_MODES_PEDESTAL_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_AP_POWER_MODES);
}

static void bcm215xx_enable_deepsleep(void)
{
	u32 reg_val;

	/* Enable deep sleep for AP */
	reg_val = readl(ADDR_CLKPWR_CLK_AP_POWER_MODES);
	reg_val |= CLK_POWER_MODES_DEEPSLEEP_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_AP_POWER_MODES);
}

static void bcm215xx_disable_deepsleep(void)
{
	u32 reg_val;

	/* Disable deep sleep for AP */
	reg_val = readl(ADDR_CLKPWR_CLK_AP_POWER_MODES);
	reg_val &= ~CLK_POWER_MODES_DEEPSLEEP_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_AP_POWER_MODES);
}

static void bcm215xx_enable_dormant(void)
{
	u32 reg_val;

	/* Enable dormant mode for AP */
	reg_val = readl(ADDR_CLKPWR_CLK_APPS_DORMANT);
	reg_val |= CLK_APPS_DORMANT_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_APPS_DORMANT);
}

static void bcm215xx_disable_dormant(void)
{
	u32 reg_val;

	/* Disable dormant mode for AP */
	reg_val = readl(ADDR_CLKPWR_CLK_APPS_DORMANT);
	reg_val &= ~CLK_APPS_DORMANT_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_APPS_DORMANT);
}

/* Move athena to low power state. This function disables interrupts
 * and does not return, thereby bringing the kernel to a halt.
 */
static void bcm215xx_force_sleep(void)
{
	u32 reg_val;

	/* Disable interrupts */
	local_irq_disable();
	local_fiq_disable();

	/* Disable AHB */
	reg_val = readl(ADDR_CLKPWR_CLK_POWER_MODES);
	reg_val &= ~CLK_DEEPSLEEP_AHBOFF_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_POWER_MODES);

	/* Disable AHB-AP */
	reg_val = readl(ADDR_CLKPWR_CLK_AP_POWER_MODES);
	reg_val &= ~CLK_DEEPSLEEP_AHBOFF_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_AP_POWER_MODES);

	/* Enable deep sleep */
	reg_val = readl(ADDR_CLKPWR_CLK_POWER_MODES);
	reg_val |= CLK_POWER_MODES_DEEPSLEEP_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_POWER_MODES);

	/* Enable deep sleep-AP */
	reg_val = readl(ADDR_CLKPWR_CLK_AP_POWER_MODES);
	reg_val |= CLK_POWER_MODES_DEEPSLEEP_ENABLE;
	writel(reg_val, ADDR_CLKPWR_CLK_AP_POWER_MODES);

	writel(0x3F, ADDR_CLKPWR_CLK_PM_SLEEP_REQ_FORCE);

	while (1)
		;
}

/*********************************************************************
 *                   LINUX PM FRAMEWORK INTERFACE                    *
 *********************************************************************/

static void bcm215xx_enter_sleep(struct bcm_pm_sleep *bcm_pm_sleep)
{
#if (BCM_PM_DORMANT_PROFILING == 1)
	u32 syscfg_iocr6, gpio_iotr1;
#endif

#ifdef BCM_PM_FORCE_SLEEP
	/* Force Deepsleep mode */
	bcm_pm_debug_force(1);
#endif

#if (BCM_PM_DORMANT_PROFILING == 1)
	/* configure gpio22 output */
	syscfg_iocr6 = readl(ADDR_SYSCFG_IOCR6);
	writel(syscfg_iocr6 & (~SYSCFG_IOCR6_GPIO22_MUX), ADDR_SYSCFG_IOCR6);

	gpio_iotr1 = readl(HW_GPIO_BASE + GPIO_IOTR1_OFF);
	writel((gpio_iotr1 & (~(3 << 12))) | (2 << 12),
	       HW_GPIO_BASE + GPIO_IOTR1_OFF);
#endif

	/* Disable all sleep modes. Required modes are enabled further
	 * along.
	 */
	bcm215xx_disable_deepsleep();
	bcm215xx_disable_dormant();

	if (bcm_pm_sleep->dormant_allowed == true) {
		/* Set the high boot vector lock bit */
		writel(0x01, ADDR_CLKPWR_CLK_APPSARM_WARMBOOT_MODE);

		bcm215xx_enable_dormant();
	}

	if (bcm_pm_sleep->deepsleep_allowed == true)
		bcm215xx_enable_deepsleep();

	if (bcm_pm_sleep_buf->pedestal_allowed == false)
		bcm215xx_disable_pedestal();

#if defined(CONFIG_BRCM_FUSE_RPC_CIB)
	if (bcm_pm_sleep->deepsleep_allowed == true) {
	BcmRpc_SetApSleep(1);
	mb();
	}
#endif
	bcm215xx_sleep(bcm_pm_sleep);
	mb();

#if defined(CONFIG_BRCM_FUSE_RPC_CIB)
	if (bcm_pm_sleep->deepsleep_allowed == true)
	BcmRpc_SetApSleep(0);
#endif

#if (BCM_PM_DORMANT_PROFILING == 1)
	/* Restore SYSCFG_IOCR6 */
	writel(syscfg_iocr6, ADDR_SYSCFG_IOCR6);
	writel(gpio_iotr1, HW_GPIO_BASE + GPIO_IOTR1_OFF);
#endif

#ifdef BCM_PM_FORCE_SLEEP
	/* Restore the state before regs were forced */
	bcm_pm_debug_restore(1);
#endif

	bcm215xx_disable_deepsleep();
	bcm215xx_disable_dormant();
	bcm215xx_enable_pedestal();
}

/* This function is defined by the clock manager. The clock manager function
 * returns false if not all the clocks have been released. This function
 * returns true if all the clocks have been released and hence the platform
 * can go into pedestal state.
 *
 * The default version defined here just returns true.
 */
bool __attribute__ ((weak)) brcm_clk_is_pedestal_allowed(void)
{
	return true;
}

inline void bcm215xx_enter_idle(int cpu_state)
{
	bool allow_pedestal = true;
	u32 ori_uarta_ucr = 0, ori_uartb_ucr = 0, ori_uartc_ucr = 0;
	u32 val;

	/* Disable interrupts */
	local_irq_disable();
	local_fiq_disable();

	if (brcm_clk_is_pedestal_allowed() == false
#ifdef CONFIG_HAS_WAKELOCK
	    || has_wake_lock(WAKE_LOCK_IDLE)
#endif
	    || bcm_pedestal_enable == false
	    || cpu_state == BCM21553_STATE_C0) {
		allow_pedestal = false;
	}

	bcm_pm_sleep_buf->pedestal_allowed = allow_pedestal;

	if (allow_pedestal) {
		ori_uarta_ucr = readl(ADDR_UARTA_UCR);
		ori_uartb_ucr = readl(ADDR_UARTB_UCR);
		ori_uartc_ucr = readl(ADDR_UARTC_UCR);

		/* UARTA Power down enable */
		writel((ori_uarta_ucr | ADDR_UART_PWR_DIS), ADDR_UARTA_UCR);
		/* UARTB Power down enable */
		writel((ori_uartb_ucr | ADDR_UART_PWR_DIS), ADDR_UARTB_UCR);
		/* UARTC Power down enable */
		writel((ori_uartc_ucr | ADDR_UART_PWR_DIS), ADDR_UARTC_UCR);
	}

	if (IS_PEDESTAL_LOG_ENABLED)
		bcm_log_regs(1);
	if (IS_INSTRUMENTATION_ENABLED)
		instrument_idle_entry();

	bcm_pm_sleep_buf->deepsleep_allowed = false;
	if (cpu_state == BCM21553_STATE_C2 &&
		bcm_dormant_enable == true)
		bcm_pm_sleep_buf->dormant_allowed = true;
	else
		bcm_pm_sleep_buf->dormant_allowed = false;

	if (bcm_pm_sleep_buf->dormant_allowed == true || allow_pedestal) {
		/* Set APPS sleep bit */
		val = readl(ADDR_IRQ_IMR);
		val |= IMR_WFI_EN;
		writel(val, ADDR_IRQ_IMR);
	}

	bcm215xx_enter_sleep(bcm_pm_sleep_buf);

	if (allow_pedestal) {
		/* Restore UART[A,B,C]_UCR registers */
		writel(ori_uarta_ucr, ADDR_UARTA_UCR);
		writel(ori_uartb_ucr, ADDR_UARTB_UCR);
		writel(ori_uartc_ucr, ADDR_UARTC_UCR);
	}

	if (IS_INSTRUMENTATION_ENABLED)
		instrument_idle_exit();

	/* Enable interrupts */
	local_irq_enable();
	local_fiq_enable();
}
EXPORT_SYMBOL(bcm215xx_enter_idle);

/* CPUIdle core init is a core_initcall and this routine points
 * pm_idle_old to default_idle. PM driver init is a module_init. Hence
 * this variable is used to ensure pm_idle_old points to our idle
 * handler and also that this handler gets started much later in the
 * boot process.
 */
static bool enable_idle;

static void bcm215xx_enable_idle(void)
{
	enable_idle = true;
}

static void bcm215xx_pm_idle(void)
{
	if (!enable_idle)
		goto bcm215xx_exit_idle;

	local_irq_disable();
	local_fiq_disable();

	if (need_resched())
		goto bcm215xx_exit_idle;

	bcm215xx_enter_idle(BCM21553_STATE_C1);

bcm215xx_exit_idle:
	local_fiq_enable();
	local_irq_enable();
}

static int bcm215xx_pm_valid_state(suspend_state_t state)
{
	pr_debug("%s\n", __func__);

	switch (state) {
	case PM_SUSPEND_ON:
		return 1;
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		if (!bcm_suspend_enable) {
			pr_debug("%s: Suspend is disabled\n", __func__);
			return 0;
		}
		return 1;
	default:
		return 0;
	}
}

#ifdef CONFIG_ARCH_BCM21553_A0

/* Physical address of the ARM-DSP Audio regs */
#define AHB_AUDIO_BASE                              0x30800F00

/* Audio register offsets */
#define AHB_AUDIO_STEREOAUDMOD_R                    0x0A0
#define AHB_AUDIO_POLYAUDMOD_R                      0x060

/* AHB_AUDIO_STEREOAUDMOD_R reg bit defines */
#define AHB_AUDIO_STEREOAUDMOD_R_AUD_OUT_DMA_EN     (1 << 14)

/* AHB_AUDIO_POLYAUDMOD_R reg bit defines */
#define AHB_AUDIO_POLYAUDMOD_R_AUD_OUT_DMA_EN       (1 << 14)

/* Physical address of the ARM-DSP TL3R regs */
#define AHB_TL3R_BASE                               0x30400000

/* TL3R register offsets */
#define AHB_TL3R_TL3_A2D_ACCESS_EN_R                0x02C

static u32 ahb_audio_regs;
static u32 ahb_tl3r_regs;

static void bcm215xx_cp_sleep_fix(void);

static void bcm215xx_cp_sleep_fix(void)
{
	u32 count;
	u32 status;

	if ((readl(ADDR_DMAC_CH7CONFIG) & DMAC_CH7CONFIG_E)
	    || (readl(ADDR_DMAC_CH8CONFIG) & DMAC_CH8CONFIG_E)) {
		count = 10000000;

		while (count) {
			if (readl(ADDR_SYSCFG_AHB_CLK_GATE_MONITOR_RAW)
			    & PERIPH_AHB_CLK_GATE_MON_DMAC)
				break;

			status = readl(ADDR_DMAC_CH7CONFIG) & DMAC_CH7CONFIG_E;
			status |= readl(ADDR_DMAC_CH8CONFIG)
			    & DMAC_CH8CONFIG_E;
			if (status == 0x00)
				break;

			status = ioread16(ahb_audio_regs
					  + AHB_AUDIO_STEREOAUDMOD_R)
			    & AHB_AUDIO_STEREOAUDMOD_R_AUD_OUT_DMA_EN;
			status |= ioread16(ahb_audio_regs
					   + AHB_AUDIO_POLYAUDMOD_R)
			    & AHB_AUDIO_POLYAUDMOD_R_AUD_OUT_DMA_EN;
			if (status == 0x00)
				break;

			count--;
			if (count == 0)
				pr_debug("%s: Wait for DMAC disable failed\n",
					 __func__);
		}
	}

	status = ioread32(ahb_tl3r_regs + AHB_TL3R_TL3_A2D_ACCESS_EN_R);
	if ((status & (~0x0C)) != 8) {
		status = (status & (~0x0C)) | 8;
		iowrite32(status, ahb_tl3r_regs + AHB_TL3R_TL3_A2D_ACCESS_EN_R);
	}
}
#endif /* CONFIG_ARCH_BCM21553_A0 */

#if (BCM_PM_DORMANT_VERIFY_CRC == 1)
static void bcm_crc32_verify(void);
static void bcm_crc32_clear(void);
#endif

/* List of interrupt wakeup sources */
#define WAKEUP_SRC_GRP0	((1 << IRQ_KEYPAD) | (1 << IRQ_GPIO) | (1 << IRQ_GPT1) | (1 << IRQ_MICON))
#define WAKEUP_SRC_GRP1	(0)
#define WAKEUP_SRC_GRP2	(1 << (IRQ_IPC_C2A % 32))

#define ADDR_GPIO_GPISR0    (HW_GPIO_BASE + GPIO_GPISR0)
#define ADDR_GPIO_GPISR1    (HW_GPIO_BASE + GPIO_GPISR1)


/*+++ SEC_DEBUG to check the status of GPIOs*/
#define ADDR_GPIO_GPIPSR0    (HW_GPIO_BASE + 0x018)
#define ADDR_GPIO_GPIPSR1    (HW_GPIO_BASE + 0x01c)
/*--- SEC_DEBUG to check the status of GPIOs*/


static int bcm215xx_pm_enter(suspend_state_t state)
{
	u32 ori_imr0, ori_imr1, ori_imr2, imr;
	u32 wakeup_source_imr0 = WAKEUP_SRC_GRP0;
#ifdef CONFIG_DPRAM
	u32 wakeup_source_imr1 = ( WAKEUP_SRC_GRP1 | (1 << (IRQ_SSIPC_C2A % 32)) );
#else
	u32 wakeup_source_imr1 = WAKEUP_SRC_GRP1;
#endif
	u32 wakeup_source_imr2 = WAKEUP_SRC_GRP2;
	u32 val;

	pr_debug("%s: state:%d\n", __func__, state);

	switch (state) {
	case PM_SUSPEND_MEM:
	case PM_SUSPEND_STANDBY:

#if (BCM_PM_DORMANT_VERIFY_CRC == 1)
		bcm_crc32_clear();
#endif

#ifdef CONFIG_ARCH_BCM21553_A0
		bcm215xx_cp_sleep_fix();
		mb();
#endif
		/* Backup interrupt masks */
		ori_imr0 = readl(ADDR_IRQ_IMR);
		ori_imr1 = readl(ADDR_IRQ_IMR1);
		ori_imr2 = readl(ADDR_IRQ_IMR2);

		/* Set wakeup sources.
		 * Mask AP interrupts and enable AP wakeup source.
		 * DSP interrupt mask bits should not be modified.
		 */
		writel((wakeup_source_imr0 |
			(ori_imr0 & (~BCM21553_VALID_SRC0))), ADDR_IRQ_IMR);
		writel((wakeup_source_imr1 |
			(ori_imr1 & (~BCM21553_VALID_SRC1))), ADDR_IRQ_IMR1);
		writel((wakeup_source_imr2 |
			(ori_imr2 & (~BCM21553_VALID_SRC2))), ADDR_IRQ_IMR2);

		/* Take a snapshot of the debug registers */
		if (IS_SUSPEND_LOG_ENABLED)
			bcm_log_regs(1);
		if (IS_INSTRUMENTATION_ENABLED)
			instrument_suspend_entry();

		bcm_pm_sleep_buf->deepsleep_allowed = bcm_deepsleep_enable;
		bcm_pm_sleep_buf->dormant_allowed = bcm_dormant_enable;
		bcm_pm_sleep_buf->pedestal_allowed = bcm_pedestal_enable;

		if (bcm_pm_sleep_buf->deepsleep_allowed == true ||
		    bcm_pm_sleep_buf->dormant_allowed == true ||
		    bcm_pm_sleep_buf->pedestal_allowed == true)
		{
			/* Set APPS sleep bit */
			val = readl(ADDR_IRQ_IMR);
			val |= IMR_WFI_EN;
			writel(val, ADDR_IRQ_IMR);
		}

/*+++ SEC_DEBUG to check the status of GPIOs*/
		pr_info("%s: check gpio before sleep : gpor1(0x%08x), gpor0(0x%08x), gpips1(0x%08x), gpips0(0x%08x)\n",
			__func__, readl(ADDR_GPIO_GPOR1), readl(ADDR_GPIO_GPOR0), readl(ADDR_GPIO_GPIPSR1), readl(ADDR_GPIO_GPIPSR0));
/*--- SEC_DEBUG to check the status of GPIOs*/
		

		/* Finally go to sleep */
		if(!pmu_is_charger_inserted())
		{
			pr_info("%s: Disabling enter sleep\n", __func__);
			bcm215xx_enter_sleep(bcm_pm_sleep_buf);
		}
		/* Restore AP interrupt mask.
		 * DSP interrupt mask bits should not be modified
		 */
		imr = readl(ADDR_IRQ_IMR);
		writel(((imr & (~BCM21553_VALID_SRC0))
			| (ori_imr0 & BCM21553_VALID_SRC0)), ADDR_IRQ_IMR);
		imr = readl(ADDR_IRQ_IMR1);
		writel(((imr & (~BCM21553_VALID_SRC1))
			| (ori_imr1 & BCM21553_VALID_SRC1)), ADDR_IRQ_IMR1);
		imr = readl(ADDR_IRQ_IMR2);
		writel(((imr & (~BCM21553_VALID_SRC2))
			| (ori_imr2 & BCM21553_VALID_SRC2)), ADDR_IRQ_IMR2);

		if (IS_INSTRUMENTATION_ENABLED)
			instrument_suspend_exit();
		bcm_print_cntrs();

		/* Print the list of modules which woke up the system */
		pr_info("%s: isr0: 0x%08x; isr1: 0x%08x; isr2: 0x%08x, "
			"gpio_isr0: 0x%08x; gpio_isr1: 0x%08x\n",
			__func__, readl(ADDR_IRQ_ISR0) & WAKEUP_SRC_GRP0,
			readl(ADDR_IRQ_ISR1) & WAKEUP_SRC_GRP1,
			readl(ADDR_IRQ_ISR2) & WAKEUP_SRC_GRP2,
			readl(ADDR_GPIO_GPISR0), readl(ADDR_GPIO_GPISR1));

#if (BCM_PM_DORMANT_VERIFY_CRC == 1)
		bcm_crc32_verify();
#endif
		break;

	case PM_SUSPEND_ON:
		break;

	default:
		break;
	}
	return 0;
}

/*********************************************************************
 *                              INIT CODE                            *
 *********************************************************************/

static struct platform_suspend_ops bcm215xx_pm_ops = {
	.valid = bcm215xx_pm_valid_state,
	.enter = bcm215xx_pm_enter,
};

#if (BCM_PM_DORMANT_VERIFY_CRC == 1)
/* SRAM is used for CRC computation */
void __iomem *bcm_crc32_sram_base;
const unsigned int bcm_crc32_table[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

static void bcm_crc32_init(void)
{
	void __iomem *d;
	u32 *s;
	int i;

	bcm_crc32_sram_base = ioremap_nocache(BCM21553_SCRATCHRAM_BASE +
					      CRC_SRAM_START, CRC_SRAM_SZ);
	s = (u32 *)&bcm_crc32;
	d = bcm_crc32_sram_base;
	for (i = 0; i < bcm_crc32_sz / 4; i++) {
		iowrite32(*s, d + i * 4);
		s++;
	}
}

static void bcm_crc32_verify(void)
{
	u32 crc_entry, crc_exit;

	crc_entry = ioread32(bcm_crc32_sram_base + CRC_DORM_ENTRY);
	crc_exit = ioread32(bcm_crc32_sram_base + CRC_DORM_EXIT);

	if (crc_entry != crc_exit) {
		/* If crc_exit == 0, it means dormant state was not entered
		 * (may be due to interrupts)
		 */
		if (crc_exit != 0x00) {
			pr_info("CHECKSUM FAILED\n");
			pr_info("crc-entry: 0x%08x, crc-exit: 0x%08x\n",
				crc_entry, crc_exit);
		}
	} else {
		pr_info("CHECKSUM PASSED\n");
	}
}

static void bcm_crc32_clear(void)
{
	iowrite32(0x0, bcm_crc32_sram_base + CRC_DORM_ENTRY);
	iowrite32(0x0, bcm_crc32_sram_base + CRC_DORM_EXIT);
}
#endif  /* BCM_PM_DORMANT_VERIFY_CRC */

/*
 * Physical address of the high boot module
 */
#define HIGH_BOOT_MODULE_PHYS_ADDR      0xFFFF0000

/* Offsets of the vector locations from the high boot module base
 * for AP
 */
#define HIGH_BOOT_RST_VECTOR            0x40
#define HIGH_BOOT_UND_VECTOR            0x44
#define HIGH_BOOT_SWI_VECTOR            0x48
#define HIGH_BOOT_PBT_VECTOR            0x4C
#define HIGH_BOOT_DBT_VECTOR            0x50
#define HIGH_BOOT_IRQ_VECTOR            0x58
#define HIGH_BOOT_FIQ_VECTOR            0x5C

/* This is the default value listed in the RDB */
#define VDD_RAMP_COUNT                  0xFFF

static int __init bcm215xx_pm_init(void)
{
	void *high_boot_ptr;
	dma_addr_t bcm_sleep_buffer_handle;

	pr_info("%s\n", __func__);

	suspend_set_ops(&bcm215xx_pm_ops);
	bcm215xx_enable_idle();

	/* Enable pedestal by default */
	bcm215xx_enable_pedestal();

	/* Disable deep sleep and dormant modes by default */
	bcm215xx_disable_deepsleep();
	bcm215xx_disable_dormant();

	/* Remap high boot module physical address */
	high_boot_ptr = ioremap_nocache(HIGH_BOOT_MODULE_PHYS_ADDR, SZ_256);
	if (high_boot_ptr == NULL) {
		pr_err("%s: ioremap_nocache failed for phys addr : %u\n",
		       __func__, HIGH_BOOT_MODULE_PHYS_ADDR);
		return -ENOMEM;
	}

	/* Allocate space for sleep state store. This region should be
	 * non-cacheable.
	 */
	bcm_pm_sleep_buf = dma_alloc_coherent(NULL, SZ_1K,
					      &bcm_sleep_buffer_handle,
					      GFP_ATOMIC);
	if (!bcm_pm_sleep_buf) {
		pr_err("%s: dma_alloc_coherent for dorm buf failed\n",
			__func__);
		return -ENOMEM;
	}
	bcm_pm_sleep_buf_phys = (struct bcm_pm_sleep *)bcm_sleep_buffer_handle;
	memset(bcm_pm_sleep_buf, 0, SZ_1K);

#ifdef CONFIG_BCM21553_L2_EVCT
	bcm_pm_sleep_buf->l2_evict_buf = l2_evt_virt_buf;
#endif

	/* Ensure that dormant buffer address stored in bcm_pm_sleep_buf
	 * and bcm_pm_sleep_buf_phys gets written to the main memory.
	 */
	flush_caches();
	outer_flush_range(0, SZ_1K * 129);

	pr_info("%s: Phys Address of sleep buffer: %p\n", __func__,
		bcm_pm_sleep_buf_phys);
	pr_info("%s: Virt Address of sleep buffer: %p\n", __func__,
		bcm_pm_sleep_buf);

	bcm_pm_sleep_buf->deepsleep_allowed = bcm_deepsleep_enable;
	bcm_pm_sleep_buf->dormant_allowed = bcm_dormant_enable;
	bcm_pm_sleep_buf->verify = true;
	bcm_pm_sleep_buf->dormant_sequence_dbg = false;

	/* Load the physical addresses of the restore function
	 * and dormant mode exception handlers into the high
	 * boot module vector locations.
	 */
	iowrite32((u32) __pa(bcm215xx_dorm_wake_handler()),
		  (u32) high_boot_ptr + HIGH_BOOT_RST_VECTOR);
	iowrite32((u32) __pa(bcm215xx_dorm_exception_handler),
		  (u32) high_boot_ptr + HIGH_BOOT_UND_VECTOR);
	iowrite32((u32) __pa(bcm215xx_dorm_exception_handler),
		  (u32) high_boot_ptr + HIGH_BOOT_SWI_VECTOR);
	iowrite32((u32) __pa(bcm215xx_pabt_handler),
		  (u32) high_boot_ptr + HIGH_BOOT_PBT_VECTOR);
	iowrite32((u32) __pa(bcm215xx_dabt_handler),
		  (u32) high_boot_ptr + HIGH_BOOT_DBT_VECTOR);
	iowrite32((u32) __pa(bcm215xx_dorm_wake_handler()),
		  (u32) high_boot_ptr + HIGH_BOOT_IRQ_VECTOR);
	iowrite32((u32) __pa(bcm215xx_dorm_wake_handler()),
		  (u32) high_boot_ptr + HIGH_BOOT_FIQ_VECTOR);

	/* This mapping is no longer used */
	iounmap(high_boot_ptr);

	/* Program the ramp count */
	writel(VDD_RAMP_COUNT, ADDR_CLKPWR_CLK_APPS_RAMP_COUNT);

#ifdef CONFIG_ARCH_BCM21553_A0
	/* Map the ARM DSP audio regs (3K size) */
	ahb_audio_regs = (u32) ioremap_nocache(AHB_AUDIO_BASE, SZ_256);
	ahb_tl3r_regs = (u32) ioremap_nocache(AHB_TL3R_BASE, SZ_256);
#endif

	/* Clear the pedestal and deepsleep elapsed-time counters */
	bcm_clear_hw_cntrs();
#if (BCM_PM_DORMANT_VERIFY_CRC == 1)
	bcm_crc32_init();
#endif
	pr_info("%s: success\n", __func__);

	return 0;
}
module_init(bcm215xx_pm_init);

static int __init bcm215xx_pm_earlyinit(void)
{
	pm_idle = bcm215xx_pm_idle;
	return 0;
}
early_initcall(bcm215xx_pm_earlyinit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PM Driver for Broadcom bcm215xx chips");
