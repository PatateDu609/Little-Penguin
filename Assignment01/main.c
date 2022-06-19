#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL v2");

int init_module(void)
{
	printk(KERN_INFO "Hello world !\n");
	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Cleaning up module.\n");
}
