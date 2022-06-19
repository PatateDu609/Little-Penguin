#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>

MODULE_LICENSE("GPL v2");

#define LOGIN "gboucett"
#define LOGIN_LEN 8

static ssize_t misc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
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

static ssize_t misc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
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

struct file_operations misc_fops = {
	.owner = THIS_MODULE,
	.read = misc_read,
	.write = misc_write,
};

struct miscdevice fortytwo = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fortytwo",
	.fops = &misc_fops,
};

int mod_main(void)
{
	pr_info("Hello world !\n");
	return misc_register(&fortytwo);
}

void mod_exit(void)
{
	pr_info("Cleaning up module.\n");
	misc_deregister(&fortytwo);
}

module_init(mod_main);
module_exit(mod_exit);
