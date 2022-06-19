#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/debugfs.h>
#include <linux/mutex.h>
#include <linux/string.h>

#define LOGIN "gboucett"
#define LOGIN_LEN 8

MODULE_LICENSE("GPL");

static struct dentry *dir;
static struct mutex mutex;

static char stored[PAGE_SIZE];

static ssize_t id_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	size_t size_read;
	int retval;

	if (*ppos >= LOGIN_LEN)
		return 0;
	size_read = count < LOGIN_LEN - *ppos ? count : LOGIN_LEN - *ppos;

	retval = copy_to_user(buf, LOGIN + *ppos, size_read);

	if (retval < 0)
		return -EFAULT;
	size_read -= retval;
	*ppos += size_read;

	return size_read;
}

static ssize_t id_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char tmp[LOGIN_LEN];

	if (count != LOGIN_LEN) // Check if the user is trying to write the correct amount of bytes
		return -EINVAL; // Invalid argument

	if (copy_from_user(tmp, buf, LOGIN_LEN)) // Copy the user's data to the kernel
		return -EFAULT; // Bad address

	if (memcmp(tmp, LOGIN, LOGIN_LEN)) // Check if the user's data is correct
		return -EINVAL; // Invalid argument
	return LOGIN_LEN;
}

static ssize_t jiffies_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned long j;
	ssize_t s;

	j = get_jiffies_64();
	s = snprintf(buf, count, "%lu\n", j);
	return s > count ? count : s;
}

static ssize_t foo_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	size_t length;
	size_t size_read;
	int retval;

	retval = mutex_lock_interruptible(&mutex);
	if (retval)
		return retval;

	length = strlen(stored);
	size_read = count < length - *ppos ? count : length - *ppos;

	retval = copy_to_user(buf, stored + *ppos, size_read);
	if (retval < 0)
		return -EFAULT;
	size_read -= retval;

	*ppos += size_read;

	mutex_unlock(&mutex);

	return size_read;
}

static ssize_t foo_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int retval;

	if (count > PAGE_SIZE)
		return -EINVAL;

	retval = mutex_lock_interruptible(&mutex);
	if (retval)
	{
		mutex_unlock(&mutex);
		return retval;
	}

	if (copy_from_user(stored, buf, count))
	{
		mutex_unlock(&mutex);
		return -EFAULT;
	}

	mutex_unlock(&mutex);
	return count;
}



struct file_operations id_ops = {
	.owner = THIS_MODULE,
	.read = id_read,
	.write = id_write,
};

struct file_operations jiffies_ops = {
	.owner = THIS_MODULE,
	.read = jiffies_read,
};

struct file_operations foo_ops = {
	.owner = THIS_MODULE,
	.read = foo_read,
	.write = foo_write,
};

int mod_main(void)
{
	pr_info("Hello World!\n");

	dir = debugfs_create_dir("fortytwo", NULL);
	if (!dir || (long)dir == -ENODEV)
	{
		pr_err("Could not create debugfs directory\n");
		return -ENODEV;
	}

	debugfs_create_file("id", 0666, dir, NULL, &id_ops);
	debugfs_create_file("jiffies", 0444, dir, NULL, &jiffies_ops);
	debugfs_create_file("foo", 0644, dir, NULL, &foo_ops);

	return 0;
}

void mod_exit(void)
{
	debugfs_remove_recursive(dir);

	pr_info("Module unloaded\n");
	return;
}

module_init(mod_main);
module_exit(mod_exit);
