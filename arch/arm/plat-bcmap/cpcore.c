/*
 *	arch/arm/plat-brcm/cpcore.c Interface for accessing the CP crash
 * 				 dump from the system's previous life.
 * 	Heavily borrowed from fs/proc/vmcore.c
 *	Created by: Hariprasad Nellitheertha (hari@in.ibm.com)
 *	Copyright (C) IBM Corporation, 2004. All rights reserved
 *
 */

#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/user.h>
#include <linux/elf.h>
#include <linux/elfcore.h>
#include <linux/highmem.h>
#include <linux/bootmem.h>
#include <linux/init.h>
#include <linux/crash_dump.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <asm/io.h>


struct cpcore {
	struct list_head list;
	unsigned long long paddr;
	unsigned long long size;
	loff_t offset;
}; 

/* List representing chunks of contiguous memory areas and their offsets in
 * cpcore file.
 */
static LIST_HEAD(cpcore_list);

/* Total size of cpcore file. */
static unsigned long long cpcore_size;
static unsigned long long cp_base_addr;

static struct proc_dir_entry *proc_cpcore = NULL;

/* Reads a page from the oldmem device from given offset. */
static ssize_t read_from_oldmem(char *buf, size_t count,
				u64 *ppos, int userbuf)
{
	unsigned long pfn, offset;
	size_t nr_bytes;
	ssize_t read = 0, tmp;

	if (!count)
		return 0;

	offset = (unsigned long)(*ppos % PAGE_SIZE);
	pfn = (unsigned long)(*ppos / PAGE_SIZE);

	do {
		if (count > (PAGE_SIZE - offset))
			nr_bytes = PAGE_SIZE - offset;
		else
			nr_bytes = count;

		tmp = copy_oldmem_page(pfn, buf, nr_bytes, offset, userbuf);
		if (tmp < 0)
			return tmp;
		*ppos += nr_bytes;
		count -= nr_bytes;
		buf += nr_bytes;
		read += nr_bytes;
		++pfn;
		offset = 0;
	} while (count);

	return read;
}

/* Maps cpcore file offset to respective physical address in memroy. */
static u64 map_offset_to_paddr(loff_t offset, struct list_head *vc_list,
                                        struct cpcore **m_ptr)
{
        struct cpcore *m;
        u64 paddr;

        list_for_each_entry(m, vc_list, list) {
                u64 start, end;
                start = m->offset;
                end = m->offset + m->size - 1;
                if (offset >= start && offset <= end) {
                        paddr = m->paddr + offset - start;
                        *m_ptr = m;
                        return paddr;
                }
        }
        *m_ptr = NULL;
        return 0;
}

static ssize_t read_cpcore(struct file *file, char __user *buffer,
				size_t buflen, loff_t *fpos)
{
	ssize_t acc = 0, tmp;
	size_t tsz;
	u64 start, nr_bytes;
	struct cpcore *curr_m = NULL;

	if (buflen == 0 || *fpos >= cpcore_size)
		return 0;

	/* trim buflen to not go beyond EOF */
	if (buflen > cpcore_size - *fpos)
		buflen = cpcore_size - *fpos;

	if(*fpos < 0)
		*fpos = 0;

        start = map_offset_to_paddr(*fpos, &cpcore_list, &curr_m);
        if (!curr_m)
                return -EINVAL;
        if ((tsz = (PAGE_SIZE - (start & ~PAGE_MASK))) > buflen)
                tsz = buflen;

	/* Calculate left bytes in current memory segment. */
	nr_bytes = (curr_m->size - (start - curr_m->paddr));
	if (tsz > nr_bytes)
		tsz = nr_bytes;

	while (buflen) {
		tmp = read_from_oldmem(buffer, tsz, &start, 1);
		if (tmp < 0)
			return tmp;
		buflen -= tsz;
		*fpos += tsz;
		buffer += tsz;
		acc += tsz;
		if (start >= (curr_m->paddr + curr_m->size)) {
			return acc;
		}

		if ((tsz = (PAGE_SIZE - (start & ~PAGE_MASK))) > buflen)
			tsz = buflen;
		/* Calculate left bytes in current memory segment. */
		nr_bytes = (curr_m->size - (start - curr_m->paddr));
		if (tsz > nr_bytes)
			tsz = nr_bytes;
	}

	return acc;
}


static const struct file_operations proc_cpcore_operations = {
	.read		= read_cpcore,
};

static struct cpcore* __init get_new_element(void)
{
	return kzalloc(sizeof(struct cpcore), GFP_KERNEL);
}

static int __init get_cpcore(void)
{
	struct cpcore *new;

	new = get_new_element();

	if (!new) {
		return -ENOMEM;
	}

	new->paddr = cp_base_addr; // CP_BASE_ADDR;
	new->size = cpcore_size;
	new->offset = 0;
	
	list_add_tail(&new->list, &cpcore_list);

	return 0;
}

/* Parse the CP core base address. */
static int __init setup_cpbase(char *arg)
{
        char *end,*p;

        if (!arg)
                return -EINVAL;

        p = strstr(arg, "@");
        *p = '\0'; p += 1;

        cp_base_addr = memparse(p, &end);
        cpcore_size = memparse(arg, &end);

        return end > arg ? 0 : -EINVAL;
}
early_param("cpbase", setup_cpbase);

/* Init function for cpcore module. */
static int __init cpcore_init(void)
{

	get_cpcore();

	proc_cpcore = proc_create("cpcore", S_IRUSR, NULL, &proc_cpcore_operations);
	if (proc_cpcore) {
		printk("Creating cpcore proc entry!!!!\n");
		proc_cpcore->size = cpcore_size;
	}
	else {
		printk("cpcore creation failed!!!\n");
	}

	return 0;
}
module_init(cpcore_init)
