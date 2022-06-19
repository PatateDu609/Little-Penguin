#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mount.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/nsproxy.h>
#include <../fs/mount.h>

static struct proc_dir_entry *mymounts;

MODULE_LICENSE("GPL");

static ssize_t mymounts_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	char *kbuf;
	struct mount *mnt;
	char raw_path[256];
	int ret;
	int i;

	kbuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	i = 0;
	ret = 0;
	list_for_each_entry(mnt, &current->nsproxy->mnt_ns->list, mnt_list) {
		if (strcmp(mnt->mnt_devname, "none") == 0)
			continue;
		if (strcmp(mnt->mnt_mountpoint->d_name.name, "/") == 0)
			continue;

		memset(raw_path, 0, 256);

		if (strcmp(mnt->mnt_devname, "/dev/root") == 0)
			i += snprintf(kbuf + i, PAGE_SIZE - i, "%-8s\t%s\n",
					"root",
					dentry_path_raw(mnt->mnt_mountpoint, raw_path, 256));
		else
			i += snprintf(kbuf + i, PAGE_SIZE - i, "%-8s\t%s\n",
					mnt->mnt_mountpoint->d_name.name,
					dentry_path_raw(mnt->mnt_mountpoint, raw_path, 256));
	}

	if (i == 0)
		i += snprintf(kbuf + i, PAGE_SIZE - i, "No mounted filesystems\n");
	ret = simple_read_from_buffer(buf, count, ppos, kbuf, i);
	kfree(kbuf);
	return ret;
}

static const struct proc_ops mymounts_proc_ops = {
	.proc_read = mymounts_read,
};

int mod_main(void)
{
	mymounts = proc_create("mymounts", 0444, NULL, &mymounts_proc_ops);
	if (!mymounts)
		return -ENOMEM;

	printk(KERN_INFO "/proc/mymounts created successfully!\n");

	return 0;
}

void mod_exit(void)
{
	proc_remove(mymounts);
	printk(KERN_INFO "Goodbye, world!\n");
}

module_init(mod_main);
module_exit(mod_exit);
