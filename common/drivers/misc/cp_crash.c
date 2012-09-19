/*****************************************************************************
*  Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/* 
* This file adds a panic notifier task and is run when there is a system panic.
* It sends a message to CP to crash it if CP has not crashed yet. 
*/

#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/proc_fs.h>

#include <linux/broadcom/ipcinterface.h>

/* Prototypes */
static int cp_proc_read(char *, char **, off_t, int, int *, void *);
static int do_cp_crash(struct notifier_block *, unsigned long , void *);

static struct work_struct proc_removal_work;

/* External */
extern void ProcessCPCrashedDump(struct work_struct *); 
extern int IpcCPCrashCheck(void);
extern void ipcs_get_ipc_state(int *state);

/* Globals */
struct proc_dir_entry *cp_dump_proc;

static int cp_proc_read(char *buffer, char **start, off_t offset,
			       int count, int *peof, void *dat)
{
	struct mtd_info *mtd = NULL;
	int wr_sz;
	int file_length = CP_DUMP_LEN;
	int file_offset = KPANIC_CP_DUMP_OFFSET;
	void *cp_buf;
	int ret = 0;
	int len = 0;

	if (!count) 
		return 0;

	if ((offset + count) > file_length) 
		return 0;

	mtd = get_mtd_device_nm(CONFIG_APANIC_PLABEL);
	wr_sz = mtd->writesize;
	cp_buf = kmalloc(wr_sz, GFP_ATOMIC);
	memset(cp_buf, 0xff, wr_sz);

	/* We only support reading a maximum of a flash page */
	if (count > wr_sz)
		count = wr_sz;

	ret = mtd->read(mtd, file_offset + offset, wr_sz, &len, cp_buf);
	if (ret) 
		printk(KERN_CRIT "%s- MTD read failed, ret:%dn", __func__, ret);
	else 
		memcpy(buffer, cp_buf, wr_sz);
	
	*start = count;

	if ((offset + count) == file_length)
		*peof = 1;
	
	kfree(cp_buf);

	return count;
}

static void cp_crash_remove_proc_work(struct work_struct *work)
{
	remove_proc_entry("cp_dump", NULL);
}

static int cp_proc_write(struct file *file, const char __user *buffer,
				unsigned long count, void *data)
{
	schedule_work (&proc_removal_work);
	return count;
}

static int do_cp_crash(struct notifier_block *this, unsigned long event,
			void *ptr)
{
    int ipc_state;

#ifdef CONFIG_PREEMPT
	/* Ensure that cond_resched() won't try to preempt anybody */
	add_preempt_count(PREEMPT_ACTIVE);
#endif

    ipcs_get_ipc_state(&ipc_state);

    if(!ipc_state)
        goto out;

	if (!IpcCPCrashCheck())
		IPCCP_SetCPCrashedStatus(IPC_AP_ASSERT);
	ProcessCPCrashedDump(NULL);	

out:
#ifdef CONFIG_PREEMPT
	sub_preempt_count(PREEMPT_ACTIVE);
#endif

	return NOTIFY_DONE;
}

static struct notifier_block cp_crash_blk = {
	.notifier_call	= do_cp_crash
};

static void mtd_notify_add(struct mtd_info *mtd)
{
	int wr_sz, len;
	int file_offset = KPANIC_CP_DUMP_OFFSET;
	char cp_string[] = "COMMS_PROCESSOR crash summary";
	char str[30] = "";
	void *cp_buf;
	int ret;

	if (strcmp(mtd->name, CONFIG_APANIC_PLABEL))
		return;

	wr_sz = mtd->writesize;
	cp_buf = kmalloc(wr_sz, GFP_KERNEL);
	memset(cp_buf, 0xff, wr_sz);

	ret = mtd->read(mtd, file_offset, wr_sz, &len, cp_buf);
	if (!ret) {
		strncpy(str, cp_buf+19, strlen(cp_string));
		if (strcasecmp(str, cp_string)) {
			printk(KERN_CRIT "CP crash dump not found. \n");
			kfree(cp_buf);
			return;
		}
		else 
			printk(KERN_CRIT "CP crash dump found!. \n");			
	}
	else {
		printk(KERN_CRIT "%s- MTD read failed, ret:%dn", __func__, ret);
		kfree(cp_buf);
		return;
	}

	cp_dump_proc = create_proc_entry("cp_dump",
					 S_IFREG | S_IRUGO, NULL);
				
	if (cp_dump_proc == NULL) {
		printk(KERN_ERR "%s: failed creating procfile for CP dump \n", __func__);
		return;
	}
	
	cp_dump_proc->read_proc = cp_proc_read;
	cp_dump_proc->write_proc = cp_proc_write;
	cp_dump_proc->size = CP_DUMP_LEN;

}
static struct mtd_notifier mtd_panic_notifier = {
	.add	= mtd_notify_add,
};

static int __init cp_crash_init (void)
{
	int ret = 0;

	register_mtd_user(&mtd_panic_notifier);
  	atomic_notifier_chain_register(&panic_notifier_list, &cp_crash_blk);
	INIT_WORK(&proc_removal_work, cp_crash_remove_proc_work);

	printk(KERN_INFO "CP crash driver initialized!\n");
	return ret;
}

static void __exit cp_crash_exit(void)
{
	remove_proc_entry("cp_dump", NULL);
	
}

module_init (cp_crash_init);
module_exit (cp_crash_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
